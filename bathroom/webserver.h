#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "eeprom.h"
#include "debug.h"

const char* ssid = "Bathroom";  // Enter SSID here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer _webServer(80);

void handleOnConnect();
void handleEdit();
String displayPage(float humidity, float temperature, bool running = false);

void setupWebserver()
{
    WiFi.softAP(ssid);
    WiFi.softAPConfig(local_ip, gateway, subnet);

    _webServer.begin();
}

String displayPage(float humidity, float temperature, bool running)
{
    String ptr = "<!DOCTYPE html> <html>\n"
            "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"
            "<title>Fan Control</title>\n"
            "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"
            "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n"
            ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n"
            ".button-on {background-color: #1abc9c;}\n"
            ".button-on:active {background-color: #16a085;}\n"
            ".button-off {background-color: #34495e;}\n"
            ".button-off:active {background-color: #2c3e50;}\n"
            "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<h1>Bathroom Fan Control</h1>\n";

    if(humidity)
    {
        ptr +="<p>Humidity: ";
        ptr.concat(humidity);
        ptr +=" %</p>\n";
    }

    if(temperature)
    {
        ptr +="<p>Temperature: ";
        ptr.concat(temperature);
        ptr +=" %</p>\n";
    }

    if(running)
    {
        ptr +="<p>Fan : ON</p><a class=\"button button-off\" href=\"/fan\">OFF</a>\n";
    }
    else
    {
        ptr +="<p>Fan: OFF</p><a class=\"button button-on\" href=\"/fan\">ON</a>\n";
    }
    ptr +="</body>\n";
    ptr +="</html>\n";
    return ptr;
}