

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only

#include <TaskScheduler.h>
#include <DHT_U.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define GLOBAL_SOFTWARE_VERSION "0.1.1"
#define _DEBUG_
//#define _TEST_

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#define _PM(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#define _PM(a)
#endif

#if defined(ARDUINO_ARCH_ESP32)
#define LED_BUILTIN 23 // esp32 dev2 kit does not have LED
#endif

Scheduler _scheduler;

bool _pinStatus = false;
int _relayPin = D0;
int _dhtPin = D3;
DHT _dhtSensor(_dhtPin, DHTTYPE);

int _dhtReadIntervalMs = 5000;
int _printDebugIntervalMs = 5000;

float _dhtHumidity;
float _dhtTemperature;

Task _taskReadDht(_dhtReadIntervalMs, TASK_FOREVER, &readDht, &_scheduler, false);
Task _taskPrintDebug(_printDebugIntervalMs, TASK_FOREVER, &printDebug, &_scheduler, false);

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
    pinMode(_dhtPin, INPUT);

    _taskReadDht.setOnEnable(NULL);
    _taskPrintDebug.setOnEnable(NULL);
}

void loop()
{
    _scheduler.execute();
}

inline void turnFan(bool on = true)
{
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
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
    _PL(_dhtHumidity);
    _PP("%, Temp");
    _PL(_dhtTemperature);
    _PL("C, Fan Status ");
    _PL(_pinStatus);
}
