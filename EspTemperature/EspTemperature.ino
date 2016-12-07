#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"

#define PIN_TEMP 2
#define DEBUG
#define ENABLE_EEPROM

#define TYPE_HTML 1
#define TYPE_TEXT 2
#define TYPE_JSON 3

const char* myhostname = "esp-temp";
const char* ssid = "Bowles10";
const char* password = "RainBowles";

DHT dht(PIN_TEMP, DHT22);
ESP8266WebServer server(80);

void serverResponse(String msg, int code = 200, String type = "text/html")
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "text/html", msg);
}

void sendHeader()
{
  server.sendContent("<html><head><title>");
  server.sendContent(myhostname);
  server.sendContent("</title></head><body>");
}
void sendFooter()
{
  server.sendContent("</body></html>");
}

void sendTempData(int type = TYPE_HTML)
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(true);

  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  if(type==TYPE_JSON)
  {
    server.send(200, "application/json");
    server.sendContent("{\"temperature\":\"");
    server.sendContent(String(t));
    server.sendContent(",\"humidity\":");
    server.sendContent(String(h));
    server.sendContent("}");
  } else if(type==TYPE_TEXT)
  {
    server.send(200, "text/plain");
    server.sendContent(String(t));
    server.sendContent(" ");
    server.sendContent(String(h));
  } else {
    server.send(200, "text/html");
    sendHeader();
    server.sendContent("<dl><dt>Temperature</dt><dd>");
    server.sendContent(String(t));
    server.sendContent("</dd><dt>Humidity</dt><dd>");
    server.sendContent(String(h));
    server.sendContent("</dd></dl>");
    sendFooter();
  }
}
void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while(!Serial);
  #endif
  Serialprintln("EspTemperature Startup");

  Serialprint("Connecting [");
  Serialprint(myhostname);
  Serialprint("] to [");
  Serialprint(ssid);
  Serialprint("::");
  Serialprint(password);
  Serialprintln("]");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  if(WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi failed");
    return;
  }

  MDNS.begin(myhostname);
  server.on("/", HTTP_GET, [](){
    if(server.hasArg("type")&&server.arg("type")=="json")
      sendTempData(TYPE_JSON);
    else if(server.hasArg("type")&&server.arg("type")=="text")
      sendTempData(TYPE_TEXT);
    else
      sendTempData(TYPE_HTML);
  });

  Serialprint("Connected! Starting server on ");
  Serialprint(WiFi.localIP().toString());
  Serialprintln("!");

  server.begin();
  MDNS.addService("http", "tcp", 80);

  Serialprint("Started server! Starting DHT22...");

  dht.begin();

  Serialprintln("Done!");
}

void loop() {
  server.handleClient();
  delay(1);
}

int Serialprint(String s)
{
  #ifdef DEBUG
  return Serial.print(s);
  #else
  return 0;
  #endif
}
int Serialprint(int s)
{
  #ifdef DEBUG
  return Serial.print(s);
  #else
  return 0;
  #endif
}
int Serialprintln(String s)
{
  #ifdef DEBUG
  return Serial.println(s);
  #else
  return 0;
  #endif
}
int Serialprintln(int s)
{
  #ifdef DEBUG
  return Serial.println(s);
  #else
  return 0;
  #endif
}
