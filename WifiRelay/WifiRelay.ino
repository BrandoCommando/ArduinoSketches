#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define ACTOR_COUNT 4
#define MEMORY_OFFSET 0
#define ENABLE_MEMORY 1
#define HOST_MEM_START 50

const uint16_t relays[]= { 12, 13, 14, 16 };
const uint16_t buttons[] = { 5, 4, 0, 2 };
const char* ssid = "Bowles10";
const char* password = "RainBowles";

String myhostname = "ESP8266v3";

uint16_t lasts[] = { LOW, LOW, LOW, LOW };
uint16_t modes[] = { 2, 2, 2, 2 };

WiFiServer server(80);

int readMem(int pos, int defReturn)
{
  if(!ENABLE_MEMORY) return defReturn;
  int mem = EEPROM.read(MEMORY_OFFSET + pos);
  if(mem==255) return defReturn;
  return mem;
}
void writeMem(int pos, int val)
{
  if(!ENABLE_MEMORY) return;
  EEPROM.write(MEMORY_OFFSET + pos, val);
}
void setMode(int pos, int newmode)
{
  modes[pos] = newmode;
  if(ENABLE_MEMORY)
    writeMem(pos, newmode);
}
void sendSwitch(int pos, int val)
{
  if(modes[pos] == 0)
    digitalWrite(relays[pos], LOW);
  else if(modes[pos] == 1)
    digitalWrite(relays[pos], HIGH);
  else if(modes[pos] == 2)
    digitalWrite(relays[pos], val);
}
String readHostname() {
  if(!ENABLE_MEMORY) return myhostname;
  int hlen = EEPROM.read(HOST_MEM_START);
  if(hlen==255) return myhostname;
  for(int pos=0; pos<hlen; pos++)
  {
    char c = EEPROM.read(HOST_MEM_START + 1 + pos);
    if(c==255) return myhostname;
    if(pos==0) myhostname = "";
    myhostname += c;
  }
  return myhostname;
}
void changeHostname(String newhost)
{
  if(ENABLE_MEMORY)
  {
    EEPROM.write(HOST_MEM_START, newhost.length());
    for(int pos=0; pos<newhost.length(); pos++)
      EEPROM.write(HOST_MEM_START + 1 + pos, newhost.charAt(pos));
  }
  myhostname = newhost;
  WiFi.hostname(newhost);
}

void sendClientMessage(WiFiClient client, String msg, int code = 200)
{
  String s = "HTTP/1.1 ";
  s += code;
  if(code == 200)
    s += " OK";
  else if(code == 500)
    s += " Server Error";
  else
    s += " Unknown";
  s += "\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += msg;
  s += "</html>";
  client.flush();
  client.print(s);
}
String getStatusHTML()
{
  String ret = "<table><tr><td>Actor</td><td>Pin (Relay)</td><td>Pin (Switch)</td><td>Mode</td><td>Output</td></tr>";
  for(int pi=0; pi<ACTOR_COUNT; pi++)
  {
    int relay = relays[pi];
    int btn = buttons[pi];
    int num = pi + 1;
    int val = (modes[pi] == 2) ? digitalRead(pi) : modes[pi];
    char* row;
    sprintf(row, "<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>",
              num,
              relay,
              btn,
              modes[pi],
              val);
    ret += row;
  }
  ret += "</table>";
  return ret;
}

void setup() {
  delay(10);
  int pi;
  if(ENABLE_MEMORY)
  {
    for(pi=0;pi<ACTOR_COUNT;pi++)
    {
      pinMode(buttons[pi], INPUT);
      pinMode(relays[pi], OUTPUT);
      modes[pi] = readMem(pi, 2);
      int val = digitalRead(buttons[pi]);
      sendSwitch(pi, val);
    }
    readHostname();
  } else {
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
  }
  WiFi.hostname(myhostname);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.begin();
  if(ENABLE_MEMORY)
  {
    for(pi=0;pi<ACTOR_COUNT;pi++)
      digitalWrite(relays[pi], HIGH);
    delay(250);
    for(pi=0;pi<ACTOR_COUNT;pi++)
      digitalWrite(relays[pi], LOW);
  }
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
  String msg = "";
  if(req.indexOf("/host")>-1)
  {
    String newhost = req.substring(req.indexOf("/host")+5);
    if(newhost.startsWith("/"))
      newhost = newhost.substring(1);
    if(newhost.indexOf("/")>-1)
      newhost = newhost.substring(0,newhost.indexOf("/"));
    changeHostname(newhost);
    sendClientMessage(client, "Hostname changed to " + newhost);
    return;
  }
  if(req.indexOf("/off")>-1)
    setMode(pini, val = LOW);
  else if(req.indexOf("/on")>-1)
    setMode(pini, val = HIGH);
  else if(req.indexOf("/auto")>-1)
  {
    setMode(pini, 2);
    val = digitalRead(buttons[pini]);
  }
  else if(req.indexOf("/status")>-1)
  {
    sendClientMessage(client, getStatusHTML());
  }
  else
  {
    sendClientMessage(client, "Invalid Command", 500);
    return;
  }
  
  msg += "Relay ";
  msg += pin;
  msg += " is now ";
  msg += val;
  
  sendSwitch(pini, val);

  sendClientMessage(client, msg);
  delay(1);
}

