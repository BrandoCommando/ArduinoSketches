#include <ESP8266WiFi.h>
#include "DHT.h"

#define PIN_TEMP 2

String myhostname = "ESP_DHT22";
const char* ssid = "Bowles10";
const char* password = "RainBowles";

DHT dht(PIN_TEMP, DHT22);
WiFiServer server(80);

void beginResponse(WiFiClient client, String code = "200 OK", String type = "text/html")
{
  Serial.print("Sending ");
  Serial.print(code);
  Serial.println(" response");
  client.print("HTTP/1.1 ");
  client.print(code);
  client.print("\r\nContent-Type: ");
  client.print(type);
  client.print("\r\n\r\n");
  if(type=="text/html")
  {
    client.print("<!DOCTYPE HTML>\r\n<html>\r\n<head><title>");
    client.print(myhostname);
    client.print("</title></head><body>");
  }
}
void sendClientMessage(WiFiClient client, String msg, int code = 200)
{
  String codeS = (String)code;
  if(code==200) codeS += " OK";
  else if(code==400) codeS += " Not Found";
  else if(code==500) codeS += " Server Error";
  else codeS += " Unknown";
  beginResponse(client, codeS);
  client.print(msg);
  client.print("\r\n</body></html>");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("EspTemperature Startup");

  Serial.print("Connecting [");
  Serial.print(myhostname);
  Serial.print("] to [");
  Serial.print(ssid);
  Serial.print("::");
  Serial.print(password);
  Serial.println("]");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("Connected! Starting server on ");
  Serial.print(WiFi.localIP().toString());
  Serial.println("!");

  server.begin();

  Serial.print("Started server! Starting DHT22...");

  dht.begin();

  Serial.println("Done!");
}

void loop() {
  WiFiClient client = server.available();
  if(!client) return;
  while(!client.available()) {
    delay(1);
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature(true);

  if(isnan(h)&&isnan(t))
  {
    sendClientMessage(client, "Invalid reading", 500);
    return;
  } else if(isnan(h)) {
    sendClientMessage(client, "Invalid humidity (temp = " + (String)t + ")", 500);
    return;
  } else if(isnan(t)) {
    sendClientMessage(client, "Invalid temperature (humidity = " + (String)h + ")", 500);
    return;
  }

   beginResponse(client, "200 OK", "text/plain");

   client.print(t);
   client.print(" ");
   client.print(h);
}