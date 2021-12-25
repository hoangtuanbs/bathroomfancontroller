#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <splash.h>
#include "debug.h"
#include "eeprom.h"
#include "device.h"
#include "webserver.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only

#include <TaskScheduler.h>
#include <DHT_U.h>
#include <DHT.h>

#if defined(ARDUINO_ARCH_ESP32)
#define LED_BUILTIN 23 // esp32 dev2 kit does not have LED
#endif

Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Scheduler _scheduler;

bool _fanRequest = false;
bool _fanRunning = false;
bool _automaticFan = true;

int _relayPin = RELAY_PIN;
int _dhtPin = DHT_PIN;
DHT _dhtSensor(_dhtPin, DHTTYPE);

int _dhtReadIntervalMs = 6000 * TASK_MILLISECOND;
int _printDebugIntervalMs = 6000 * TASK_MILLISECOND;
int _processFanIntervalMs = 1000 * TASK_MILLISECOND;
int _operateFanIntervalMs = 60 * TASK_SECOND;
int _fanOnInterval = 1 * TASK_MINUTE;
int _fanOffInterval = 10 * TASK_MINUTE;

int _maxHumidity;
int _minTemperature;
int _maxTemperature;

// Reboot device everyweek
unsigned long _processRebootHour = 7 * 24 * TASK_HOUR;

float _dhtHumidity;
float _dhtTemperature;

Task _taskReadDht(_dhtReadIntervalMs, TASK_FOREVER, &readDht, &_scheduler, false);
Task _taskPrintDebug(_printDebugIntervalMs, TASK_FOREVER, &printDebug, &_scheduler, false);
Task _taskDisplayScreen(_printDebugIntervalMs, TASK_FOREVER, &displayScreen, &_scheduler, false);

Task _taskRebootDevice(_processRebootHour, TASK_FOREVER, &processReboot, &_scheduler, false);

void _processFanOn();
void _processFanOff();
Task _taskProcessFan(_processFanIntervalMs, TASK_FOREVER, &_processFanOn, &_scheduler, false);
Task _taskOperateFan(_operateFanIntervalMs, TASK_FOREVER, &operateFan, &_scheduler, false);

void loadDefaultData()
{
    if (loadRomData())
    {
        _PL("Rom data loaded.");
        _maxHumidity = eepRom.MaxHumidity;
        _minTemperature = eepRom.MinTemperature;
        _maxTemperature = eepRom.MaxTemperature;

        _fanOnInterval = eepRom.FanOnInterval * TASK_MINUTE;
        _fanOffInterval = eepRom.FanOffInterval * TASK_MINUTE;
        _automaticFan = eepRom.AutomaticFan;
    }
}

void handleOnConnect()
{
    _PL("Handling connection");
    _webServer.send(200, "text/html", displayPage(_dhtHumidity, _dhtTemperature, _fanRunning));
}

void handleEdit() {
  /*LED1status = HIGH;
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status));*/
}

void handlePostRom()
{
    String postBody = _webServer.arg("plain");
    _PL(postBody);

    DynamicJsonDocument doc(512);
    DeserializationError derror = deserializeJson(doc, postBody);
    if (derror)
    {
        // if the file didn't open, print an error:
        _PP(F("Error parsing JSON "));
        _PL(derror.c_str());

        String msg = derror.c_str();

        _webServer.send(400, F("text/html"),
                "Error in parsin json body! <br>" + msg);

    } else
    {
        JsonObject postObj = doc.as<JsonObject>();

        if (_webServer.method() == HTTP_POST)
        {
            bool error = false;
            if (postObj.containsKey("max_hum") && postObj["max_hum"].is<int>())
            {
                int val = postObj["max_hum"].as<int>();
                if (val < 100 && val > 0) eepRom.MaxHumidity = val;
                else error = true;
            }

            if (postObj.containsKey("min_temp") && postObj["min_temp"].is<int>())
            {
                int val = postObj["min_temp"].as<int>();
                if (val < 30 && val > 0) eepRom.MinTemperature = val;
                else error = true;
            }

            if (postObj.containsKey("max_temp") && postObj["max_temp"].is<int>())
            {
                int val = postObj["max_temp"].as<int>();
                if (val < 100 && val > 0) eepRom.MaxTemperature = val;
                else error = true;
            }

            if (postObj.containsKey("fan_on") && postObj["fan_on"].is<int>())
            {
                int val = postObj["fan_on"].as<int>();
                if (val < 100 && val > 0) eepRom.FanOnInterval = val;
                else error = true;
            }

            if (postObj.containsKey("fan_off") && postObj["fan_off"].is<int>())
            {
                int val = postObj["fan_off"].as<int>();
                if (val < 100 && val > 0) eepRom.FanOffInterval = val;
                else error = true;
            }

            if (postObj.containsKey("auto_mod") && postObj["auto_mod"].is<bool>())
            {
                bool val = postObj["auto_mod"].as<bool>();
                eepRom.FanOffInterval = val;
            }

            if (error)
            {
                _webServer.send(400, F("text/html"),
                        "Error in parsing json data! <br>");
            } else
            {
                _webServer.send(200, F("text/html"), "OK");
            }
        }
    }
}

void setup()
{
#if defined(_DEBUG_) || defined(_TEST_)
    Serial.begin(115200);
    _PL("Bathroom Controller");
    _PP("Version: ");
    _PL(GLOBAL_SOFTWARE_VERSION);
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(_relayPin, OUTPUT);
    _dhtSensor.begin();

    loadDefaultData();

    _taskReadDht.enable();
    _taskPrintDebug.enable();
    _taskDisplayScreen.enable();
    _taskProcessFan.enable();
    _taskOperateFan.enable();

    if  (!_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        _PL(F("SSD1306 allocation failed"));
    }

    _webServer.on("/", handleOnConnect);
    _webServer.on("/edit", handleEdit);
    _webServer.on("/rom", handlePostRom);
    setupWebserver();

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    _display.display();
    _display.clearDisplay();
}

void loop()
{
    _scheduler.execute();
    _webServer.handleClient();
}

inline void turnFan(bool on = true)
{
    if (on)
    {
        _PL("Fan is Running");
    } else
    {
        _PL("Fan is Stopped");
    }

    _fanRunning = on;
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
    digitalWrite(_relayPin, on ? HIGH : LOW);
}

inline void readDht()
{
    _dhtHumidity = _dhtSensor.readHumidity();
    _dhtTemperature = _dhtSensor.readTemperature();
}

inline void printDebug()
{
    _PP(millis());
    _PP(": Humidity ");
    _PP(_dhtHumidity);
    _PP(" %, Temp ");
    _PP(_dhtTemperature);
    _PP(" C, Fan Status ");
    _PL(_fanRunning);
}

void displayScreen()
{
    _display.clearDisplay();
    _display.setTextSize(2);      // Normal 1:1 pixel scale
    _display.setTextColor(SSD1306_WHITE); // Draw white text
    _display.setCursor(2, 0);     // Start at top-left corner
    _display.cp437(true);         // Use full 256 char 'Code Page 437' font
    _display.print((int)_dhtHumidity);
    _display.print(" %");

    _display.setCursor(2, 18);     // Start at left corner
    _display.cp437(true);         // Use full 256 char 'Code Page 437' font
    _display.print((int)_dhtTemperature);
    _display.print(" C");

    if (_fanRunning)
    {
        _display.fillCircle(105, 15, 10, SSD1306_WHITE);
        delay(1);
    }

    _display.display();
}

void operateFan()
{
    if (_automaticFan)
    {
        // Operate fan when temperature and humidity is high
        if (_dhtHumidity > _maxHumidity && _dhtTemperature > _minTemperature || _dhtTemperature > _maxTemperature)
        {
            turnFan(true);
            return;
        }

        // Don't operate fan when temperature is too low
        if (_dhtTemperature < _minTemperature)
        {
            turnFan(false);
            return;
        }
    }

    // Operate fan as indicate by timer
    turnFan(_fanRequest);
}

void processReboot()
{
    ESP.restart();
}

void _processFanOn()
{
    _PL("Request Fan Running");
    _fanRequest = true;
    _taskProcessFan.setInterval(_fanOnInterval);
    _taskProcessFan.setCallback(&_processFanOff);
}

void _processFanOff()
{
    _PL("Request Fan Stopping");
    _fanRequest = false;
    _taskProcessFan.setInterval(_fanOffInterval);
    _taskProcessFan.setCallback(&_processFanOn);
}