#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define ACTOR_COUNT 2
#define MEMORY_OFFSET 1
#define ENABLE_MEMORY 1
#define HOST_MEM_START 50
#define DEBUG_MODE 1
#define ENABLE_BUTTONS 0

const uint16_t relays[]= { 2, 0 };
const uint16_t buttons[] = { 3, 1 };
const char* ssid = "Bowles10";
const char* password = "RainBowles";

String myhostname = "EspRelay";

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
void Serial_println(String txt)
{
  if(DEBUG_MODE)
    Serial.println(txt);
}
void Serial_print(String txt)
{
  if(DEBUG_MODE)
    Serial.print(txt);
}
void writeMem(int pos, int val)
{
  if(!ENABLE_MEMORY) return;
  EEPROM.write(MEMORY_OFFSET + pos, val);
  EEPROM.commit();
}
void startMem()
{
  EEPROM.begin(512);
  int flag = EEPROM.read(0);
  if(flag != 55)
  {
    Serial_print("First run (");
    Serial_print(String(flag));
    Serial_println(")!");
    for(int pi=0;pi<ACTOR_COUNT;pi++)
      writeMem(pi, 2);
    EEPROM.write(0, 55);
    EEPROM.commit();
  } else Serial_println("Not first run!");
}
void setMode(int pos, int newmode)
{
  if(pos < 0 || pos >= ACTOR_COUNT)
  {
    for(int pi=0;pi<ACTOR_COUNT;pi++)
      setMode(pi, newmode);
    return;
  }
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

void beginResponse(WiFiClient client, String code = "200 OK", String type = "text/html")
{
  Serial_print("Sending ");
  Serial_print(code);
  Serial_println(" response");
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
void getStatusHTML(WiFiClient client)
{
  beginResponse(client);
  client.println("<table><tr><td>Actor</td><td>Pin (Relay)</td>");
  if(ENABLE_BUTTONS)
    client.println("<td>Pin (Switch)</td>");
  client.println("<td>Mode</td><td>Output</td></tr>");
  for(int pi=0; pi<ACTOR_COUNT; pi++)
  {
    int relay = relays[pi];
    int num = pi + 1;
    int btv = 0;
    if(ENABLE_BUTTONS)
      btv = digitalRead(pi);
    int val = (modes[pi] == 2) ? btv : modes[pi];
    char* row;
    client.print("<tr><td>");
    client.print(num);
    client.print("</td><td>");
    client.print(relay);
    if(ENABLE_BUTTONS)
    {
      int btn = buttons[pi];
      client.print("</td><td>");
      client.print(btn);
    }
    client.print("</td><td>");
    client.print(modes[pi]);
    client.print("</td><td>");
    client.print(val);
    client.println("</td></tr>");
  }
  client.println("</table>");
}

void setup() {
  if(DEBUG_MODE)
  {
    Serial.begin(115200);
    delay(10);
    Serial.println("Arduino booting!");
  }
  int pi;
  if(ENABLE_MEMORY)
  {
    startMem();
    Serial_println("Memory mode enabled!");
    for(pi=0;pi<ACTOR_COUNT;pi++)
    {
      pinMode(relays[pi], OUTPUT);
      if(ENABLE_BUTTONS)
      {
        pinMode(buttons[pi], INPUT);
        lasts[pi] = digitalRead(buttons[pi]);
      } else lasts[pi] = 0;
      modes[pi] = readMem(pi, 2);
      sendSwitch(pi, lasts[pi]);
    }
    readHostname();
  } else {
    for(pi=0;pi<ACTOR_COUNT;pi++)
    {
      if(ENABLE_BUTTONS)
      {
        pinMode(buttons[pi], INPUT);
        lasts[pi] = digitalRead(buttons[pi]);
      } else lasts[pi] = 0;
      pinMode(relays[pi], OUTPUT);
      digitalWrite(relays[pi], HIGH);
    }
    delay(500);
    for(pi=0;pi<ACTOR_COUNT;pi++)
      digitalWrite(relays[pi], LOW);
    delay(500);
  }
  if(DEBUG_MODE)
  {
    Serial.print("Connecting [");
    Serial.print(myhostname);
    Serial.print("] to [");
    Serial.print(ssid);
    Serial.print("::");
    Serial.print(password);
    Serial.println("]");
  }
  WiFi.begin(ssid, password);
  WiFi.hostname(myhostname);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial_println("Connected! Starting server on " + WiFi.localIP().toString() + "!");

  WiFiClient client;
  String url = "/esp.php?pt=80&ip=" + WiFi.localIP().toString();
  Serial_println("Hitting " + url + " from host home.brandroid.org");
  if(!client.connect("home.brandroid.org", 80))
    Serial_println("Unable to connect to home.brandroid.org");
  else
    client.print(String("GET " + url + " HTTP/1.1\r\nHost: home.brandroid.org\r\nUser-Agent: EspRelay\r\nConnection: close\r\n\r\n"));

  while(client.connected())
  {
    String line = client.readStringUntil('\n');
    Serial_println(line);
    if(line == "\r")
      break;
  }
  String resp = client.readStringUntil('\n');
  Serial_println("Response:");
  Serial_println(resp);
  
  if(DEBUG_MODE)
    WiFi.printDiag(Serial);

  server.begin();
  if(!ENABLE_MEMORY)
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
  if(ENABLE_BUTTONS)
  for(pi=0;pi<ACTOR_COUNT;pi++)
  {
    int val = digitalRead(buttons[pi]);
    if(val == lasts[pi]) continue;
    if(DEBUG_MODE)
    {
      Serial.print("Button #");
      Serial.print(pi+1);
      Serial.print(" changed to ");
      Serial.println(val);
    }
    lasts[pi] = val;
    sendSwitch(pi, lasts[pi]);
  }
  WiFiClient client = server.available();
  if(!client) return;
  while(!client.available()) {
    delay(1);
  }
  String req = client.readStringUntil('\r');
  String method = req.substring(0, req.indexOf(" "));
  String prot = req.substring(req.lastIndexOf(" ")+1);
  req = req.substring(req.indexOf(" ")+1,req.lastIndexOf(" "));
  Serial_print("Request received: ");
  Serial_println(req);
  int val = LOW;
  int pin = 0;
  int pini = ACTOR_COUNT;
  for(int pi=0;pi<ACTOR_COUNT;pi++)
  {
    int pc = pi + 1;
    String check = "/" + (String)pc;
    if(req.indexOf(check)>-1)
    {
      pini = pi;
      break;
    }
  }
  if(pini<5)
    pin = relays[pini];
  String msg = "";
  client.flush();
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
    if(pini < ACTOR_COUNT && ENABLE_BUTTONS)
      val = digitalRead(buttons[pini]);
  }
  else if(req.indexOf("/status")>-1)
  {
    getStatusHTML(client);
    return;
  }
  else
  {
    sendClientMessage(client, "Invalid Command", 400);
    return;
  }

  if(pini < ACTOR_COUNT)
  {
    msg += "Relay #";
    msg += pini;
    msg += " is ";
  } else {
    msg += "All relays are ";
  }
  msg += "now ";
  msg += val;

  if(pini < ACTOR_COUNT && modes[pini] == 2)
    msg += " (auto)";
  
  sendSwitch(pini, val);

  sendClientMessage(client, msg);
  delay(1);
}

