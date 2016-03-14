#include <ESP8266WiFi.h>

#define ACTOR_COUNT 4

const uint16_t relays[]= { 12, 13, 14, 16 };
const uint16_t buttons[] = { 5, 4, 0, 2 };
const char* ssid = "Bowles10";
const char* password = "RainBowles";

uint16_t lasts[] = { LOW, LOW, LOW, LOW };
uint16_t modes[] = { 2, 2, 2, 2 };

WiFiServer server(80);

void setup() {
  delay(10);
  int pi;
  for(pi=0;pi<ACTOR_COUNT;pi++)
  {
    pinMode(buttons[pi], INPUT);
    pinMode(relays[pi], OUTPUT);
    digitalWrite(relays[pi], HIGH);
  }
  delay(500);
  for(pi=0;pi<ACTOR_COUNT;pi++)
    digitalWrite(relays[pi], LOW);
  delay(500);
  WiFi.hostname(F("ESP8266"));
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.begin();
  for(pi=0;pi<ACTOR_COUNT;pi++)
    digitalWrite(relays[pi], HIGH);
  delay(250);
  for(pi=0;pi<ACTOR_COUNT;pi++)
    digitalWrite(relays[pi], LOW);
}

void loop() {
  int pi;
  WiFiClient client = server.available();
  if(!client) return;
  while(!client.available()) {
    for(pi=0;pi<ACTOR_COUNT;pi++)
    {
      int val = digitalRead(buttons[pi]);
      if(val == lasts[pi]) continue;
      lasts[pi] = val;
      if(modes[pi]==2)
        digitalWrite(relays[pi], val);
    }
    delay(1);
  }
  String req = client.readStringUntil('\r');
  int val = LOW;
  int pin = 0;
  int pini = 0;
  for(int pi=0;pi<ACTOR_COUNT;pi++)
  {
    int pc = pi + 1;
    String check = "/" + (String)pc + "/";
    if(req.indexOf(check)>-1)
    {
      pin = relays[pi];
      pini = pi;
      break;
    }
  }
  if(req.indexOf("/off")>-1)
    val = modes[pin] = LOW;
  else if(req.indexOf("/on")>-1)
    val = modes[pin] = HIGH;
  else if(req.indexOf("/auto")>-1)
  {
    modes[pin] = 2;
    val = digitalRead(buttons[pini]);
  }
  else {
    client.stop();
    return;
  }
  
  digitalWrite(pin, val);

  client.flush();

  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nRelay ";
  s += pin;
  s += " is now ";
  s += val ? "high" : "low";
  s += "</html>";

  client.print(s);
  delay(1);
}
