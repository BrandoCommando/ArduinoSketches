#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIXEL_PIN 2
#define PIXEL_COUNT 150
#define DEFAULT_STEPS 10
#define DEFAULT_REST 25
#define RGBW
#define DEBUG
#define ENABLE_EEPROM

#ifdef ENABLE_EEPROM
#include <EEPROM.h>
#endif

const char* ssid = "Bowles10";
const char* password = "RainBowles";
const char* myhostname = "EspLightStrip";
const char* typeHtml = "text/html";

int bright;
int wait = 20;
uint16_t pause = 150;
bool bconn = false;
uint32_t start;
uint8_t lastCycle = 1;
int lastPixi = -1;

uint8_t smode = 2;

#ifdef OLD_WIFI_SERVER
WiFiServer server(80);
#else
ESP8266WebServer server(80);
#endif

#ifdef RGBW
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
uint32_t white = strip.Color(0,0,0,255);
uint32_t black = strip.Color(0,0,0,0);
uint32_t red = strip.Color(255,0,0,0);
uint32_t green = strip.Color(0,255,0,0);
uint32_t blue = strip.Color(0,0,255,0);
#else
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
uint32_t white = strip.Color(255,255,255);
uint32_t black = strip.Color(0,0,0);
uint32_t red = strip.Color(255,0,0);
uint32_t green = strip.Color(0,255,0);
uint32_t blue = strip.Color(0,0,255);
#endif
uint32_t color1 = red;
uint32_t color2 = green;
uint32_t color3 = black;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  start = millis();
  bright = 100;
  #ifdef DEBUG
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Arduino booting!");
  #endif
  #ifdef ENABLE_EEPROM
  EEPROM.begin(512);
  byte b1 = EEPROM.read(0);
  if(b1!=32)
  {
    EEPROM.write(0, 32);
    saveColors();
    EEPROM.write(10, bright);
    EEPROM.write(11, wait);
    writeConfig2(12, pause); // 2 bytes
    EEPROM.write(14, smode);
    EEPROM.commit();
  } else {
    color1 = readConfig(color1, 1);
    color2 = readConfig(color2, 5);
    color3 = readConfig(color3, 9);
    bright = EEPROM.read(10);
    wait = EEPROM.read(11);
    pause = readConfig(pause, 12); // 2 bytes
    smode = EEPROM.read(14);
  }
  #endif
  strip.begin();
  strip.setBrightness(bright);
  setColor(color1);
//  colorWipe(color1, wait);
//  colorWipe(color2, wait);
//  if(color3 != black)
//    colorWipe(color3, wait);
  
  #ifdef DEBUG
  Serial.print("Connecting [");
  Serial.print(myhostname);
  Serial.print("] to [");
  Serial.print(ssid);
  Serial.print("::");
  Serial.print(password);
  Serial.println("]");
  #endif
  delay(10);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED) {

  #ifdef DEBUG
  Serial.println("Connected! Starting server on " + WiFi.localIP().toString() + "!");
  #endif

  #ifndef OLD_WIFI_SERVER
  MDNS.begin(myhostname);
  server.on("/off", HTTP_GET, [](){
    #ifdef DEBUG
    Serial.println("Turning off.");
    #endif
    smode = 1;
    #ifdef ENABLE_EEPROM
    EEPROM.write(14, smode);
    EEPROM.commit();
    #endif
    serverRedirect("/?msg=All+turned+off");
  });
  server.on("/on", HTTP_GET, [](){
    #ifdef DEBUG
    Serial.println("Turning on.");
    #endif
    smode = 0;
    #ifdef ENABLE_EEPROM
    EEPROM.write(14, smode);
    EEPROM.commit();
    #endif
    serverRedirect("/?msg=All+turned+on");
  });
  server.on("/wipe", HTTP_GET, [](){
    smode = 2;
    #ifdef ENABLE_EEPROM
    EEPROM.write(14, smode);
    EEPROM.commit();
    #endif
    serverRedirect("/?msg=Wipe+mode+enabled");
  });
  server.on("/twinkle", HTTP_GET, [](){
    smode = 3;
    #ifdef ENABLE_EEPROM
    EEPROM.write(14, smode);
    EEPROM.commit();
    #endif
    serverRedirect("/?msg=Twinkling+mode+enabled");
  });
  server.on("/", HTTP_GET, []() {
    #ifdef DEBUG
    Serial.println("Home requested. Looking for arguments.");
    #endif
    String msg = checkForArgs();
    server.send(200, typeHtml);
    sendHead();
    sendModes();
    sendForm();
    if(msg!="")
      server.sendContent("<div class=\"alert alert-success\">"+msg+"</div>");
    sendFooter();
  });
  server.onNotFound([](){
    serverResponse(400, "Content not found");
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
  #endif

  bconn = true;

  #ifdef DEBUG
  Serial.println("Ready to go!");
  #endif
  } else {
    Serial.println("WiFi failed");
  }
}
void sendHead()
{
  server.sendContent("<html lang=\"en\"><head><title>" + String(myhostname) + "</title>");
  server.sendContent("<meta name=\"viewport\" content=\"width=device-width\">");
  server.sendContent("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" crossorigin=\"anonymous\">");
  server.sendContent("</head><body><div class=\"container\">");
}
void sendModes()
{
  String success = "success";
  String normal = "default";
  server.sendContent("<div class=\"btn-group\">");
  server.sendContent("<a class=\"btn btn-primary\" href=\"/\">Refresh</a>");
  server.sendContent("<a class=\"btn btn-"+(smode==1?success:normal)+"\" href=\"/off\">Turn off</a>");
  server.sendContent("<a class=\"btn btn-"+(smode==0?success:normal)+"\" href=\"/on\">Turn on</a>");
  server.sendContent("<a class=\"btn btn-"+(smode==2?success:normal)+"\" href=\"/wipe\">Wipe Mode</a>");
  server.sendContent("<a class=\"btn btn-"+(smode==3?success:normal)+"\" href=\"/twinkle\">Twinkle Mode</a>");
  server.sendContent("</div>");
}
void sendFooter()
{
  server.sendContent("</div></body></html>");
}
void sendForm()
{
  server.sendContent("<form method=\"GET\" action=\"/\"><table>");
  server.sendContent("<tr><td><label for=\"color1\">Main Color:</label></td><td><input type=\"color\" id=\"color1\" name=\"color1\" style=\"height:34px\" value=\""+toHex(color1)+"\" /></td></tr>");
  server.sendContent("<tr><td><label for=\"color2\">Secondary Color:</label></td><td><input type=\"color\" id=\"color2\" name=\"color2\" style=\"height:34px\" value=\""+toHex(color2)+"\" /></td></tr>");
  server.sendContent("<tr><td><label for=\"color3\">Accent Color:</label></td><td><input type=\"color\" id=\"color3\" name=\"color3\" style=\"height:34px\" value=\""+toHex(color3)+"\" /></td></tr>");
  server.sendContent("<tr><td><label for=\"bright\">Brightness:</label></td><td><input name=\"bright\" type=\"number\" value=\""+String(bright)+"\" /></td></tr>");
  server.sendContent("<tr><td><label for=\"wait\">Short Wait:</label></td><td><input name=\"wait\" type=\"number\" value=\""+String(wait)+"\" /></td></tr>");
  server.sendContent("<tr><td><label for=\"pause\">Long Pause:</label></td><td><input name=\"pause\" type=\"number\" value=\""+String(pause)+"\" /></td></tr>");
  server.sendContent("</table><input type=\"submit\" />");
  server.sendContent("</form>");
}

String toHex(uint32_t num)
{
  String ret = String(num, HEX);
  if(ret.indexOf("#")>-1)
    ret = ret.substring(1);
  while(ret.length()<6)
    ret = "00" + ret;
  return "#" + ret;
}

#ifndef OLD_WIFI_SERVER
String checkForArgs()
{
  String msg;
  if(server.hasArg("color1")&&server.arg("color1")!="")
  {
    uint32_t tmp;
    if(server.arg("color1").substring(0,1)=="#")
      tmp = strtol(&server.arg("color1")[1],NULL,16);
    else
      tmp = server.arg("color1").toInt();
    if(tmp!=color1)
    {
      color1 = tmp;
      msg += "color 1 changed to ";
      msg += toHex(color1);
      msg += "<br>";
    }
  }
  if(server.hasArg("color2")&&server.arg("color2")!="")
  {
    uint32_t tmp;
    if(server.arg("color2").substring(0,1)=="#")
      tmp = strtol(&server.arg("color2")[1],NULL,16);
    else
      tmp = server.arg("color2").toInt();
    if(tmp!=color2)
    {
      color2 = tmp;
      msg += "color 2 changed to ";
      msg += toHex(color2);
      msg += "<br>";
    }
  }
  if(server.hasArg("color3")&&server.arg("color3")!="")
  {
    uint32_t tmp;
    if(server.arg("color3").substring(0,1)=="#")
      tmp = strtol(&server.arg("color3")[1],NULL,16);
    else
      tmp = server.arg("color3").toInt();
    if(tmp!=color3)
    {
      color3 = tmp;
      msg += "color 3 changed to ";
      msg += toHex(color3);
      msg += "<br>";
    }
  }
  if(server.hasArg("bright"))
  {
    if(server.arg("bright").length()>0)
    {
      int tmp = server.arg("bright").toInt();
      if((server.arg("bright")=="0"||tmp!=0)&&tmp!=bright)
      {
        bright = tmp;
        msg += "brightness changed to ";
        msg += bright;
        msg += "<br>";
        #ifdef ENABLE_EEPROM
        EEPROM.write(10, bright);
        #endif
      }
    }
  }
  if(server.hasArg("wait")&&server.arg("wait").length()>0)
  {
    int tmp = server.arg("wait").toInt();
    if(tmp > 10 && tmp != wait)
    {
      wait = tmp;
      msg += "short wait set to ";
      msg += wait;
      msg += "<br>";
      #ifdef ENABLE_EEPROM
      EEPROM.write(11, wait);
      #endif
    }
  }
  if(server.hasArg("pause")&&server.arg("pause").length()>0)
  {
    int tmp = server.arg("pause").toInt();
    if(tmp>0&&tmp!=pause)
    {
      pause = tmp;
      msg += "long pause set to ";
      msg += pause;
      msg += "<br>";
      #ifdef ENABLE_EEPROM
      writeConfig2(12, pause);
      #endif
    }
  }
  #ifdef ENABLE_EEPROM
  if(msg.length()>0)
  {
    saveColors();
    EEPROM.commit();
  }
  #endif
  if(server.hasArg("msg"))
    msg += server.arg("msg") + "<br>";
  return msg;
}
#endif

unsigned long hex2int(const char* a, unsigned int len)
{
   int i;
   unsigned long val = 0;

   for(i=0;i<len;i++)
      if(a[i] <= 57)
       val += (a[i]-48)*(1<<(4*(len-1-i)));
      else
       val += (a[i]-55)*(1<<(4*(len-1-i)));
   return val;
}

void serverRedirect(String url)
{
  #ifndef OLD_WIFI_SERRVER
  server.sendHeader("Connection", "close");
  server.sendHeader("Location", url);
  server.send(302);
  #endif
}
void serverResponse(String message)
{
  #ifndef OLD_WIFI_SERVER
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", message);
  #endif
}
void serverResponse(int code, const char* message)
{
  #ifndef OLD_WIFI_SERVER
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(code, "text/html", message);
  #endif
}

#ifdef ENABLE_EEPROM

void writeConfig2(uint16_t val, int pos)
{
  EEPROM.write(pos, (val >> 8) & 0xFF);
  EEPROM.write(pos, val & 0xFF);
}
void writeConfig(uint32_t val, int pos)
{
  byte b1 = (val >> 24) & 0xFF;
  byte b2 = (val >> 16) & 0xFF;
  byte b3 = (val >> 8) & 0xFF;
  byte b4 = val & 0xFF;
  EEPROM.write(pos, b1);
  EEPROM.write(pos + 1, b2);
  EEPROM.write(pos + 2, b3);
  EEPROM.write(pos + 3, b4);
}
uint16_t readConfig(uint16_t defReturn, int pos)
{
  byte b1 = EEPROM.read(pos);
  byte b2 = EEPROM.read(pos+1);
  return (b1 << 8) + b2;
}
uint32_t readConfig(uint32_t defReturn, int pos)
{
  byte b1 = EEPROM.read(pos);
  byte b2 = EEPROM.read(pos+1);
  byte b3 = EEPROM.read(pos+2);
  byte b4 = EEPROM.read(pos+3);
  return (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
}
void saveColors()
{
  writeConfig(color1, 1);
  writeConfig(color2, 5);
  writeConfig(color3, 9);
}
#endif

void setColor(uint32_t c)
{
  for(int i=0;i<PIXEL_COUNT;i++)
    strip.setPixelColor(i, c);
  strip.show();
}

void stripShow()
{
//  for(int i = 0; i< 8; i++)
    strip.show();
}

void fadeBrightness(int brightness, int steps = DEFAULT_STEPS, int rest = DEFAULT_REST)
{
  if(bright == brightness) { setBrightness(brightness); return; }
  if(abs(brightness-bright) < steps)
    steps = abs(brightness-bright);
  int ss = (brightness - bright) / steps;
  #ifdef DEBUG
  Serial.print("Fading brightness from ");
  Serial.print(bright);
  Serial.print(" to ");
  Serial.print(brightness);
  Serial.print(": ");
  Serial.print(steps);
  Serial.print(" :: ");
  Serial.println(ss);
  #endif
  for(int x = 1; x <= steps; x++)
  {
    int b = bright + (x * ss);
    setBrightness(b);
    delay(rest);
  }
  bright = brightness;
}
void setBrightness(int brightness)
{
  if(brightness < 0)
  {
    strip.setBrightness(1);
    setColor(white);
  } else {
    strip.setBrightness(brightness);
    strip.show();
  }
}
void fadeTo2(uint32_t c, int brightness, int steps = DEFAULT_STEPS, int rest = DEFAULT_REST)
{ 
  int r1 = (color1 >> 16) & 255;
  int g1 = (color1 >>  8) & 255;
  int b1 = (color1 >>  0) & 255;
  int r2 = (c >> 16) & 255;
  int g2 = (c >>  8) & 255;
  int b2 = (c >>  0) & 255;
  float rs = (((float)r2 - (float)r1) / (float)steps);
  float gs = (((float)g2 - (float)g1) / (float)steps);
  float bs = (((float)b2 - (float)b1) / (float)steps);
}
void fadeTo(uint32_t c, int steps = DEFAULT_STEPS, int rest = DEFAULT_REST)
{
  int r1 = (color1 >> 16) & 255;
  int g1 = (color1 >>  8) & 255;
  int b1 = (color1 >>  0) & 255;
  int r2 = (c >> 16) & 255;
  int g2 = (c >>  8) & 255;
  int b2 = (c >>  0) & 255;
  float rs = (((float)r2 - (float)r1) / (float)steps);
  float gs = (((float)g2 - (float)g1) / (float)steps);
  float bs = (((float)b2 - (float)b1) / (float)steps);
  for(int i=0;i<steps;i++)
  {
    int r = rs * i;
    int g = gs * i;
    int b = bs * i;
    int newcolor = strip.Color(r,g,b);
    setColor(newcolor);
    delay(rest);
  }
  color1 = c;
}
void beginResponse(WiFiClient client, String code = "200 OK", String type = "text/html")
{
  #ifdef DEBUG
  Serial.print(F("Sending "));
  Serial.print(code);
  Serial.println(" response");
  #endif
  client.print(F("HTTP/1.1 "));
  client.print(code);
  client.print(F("\r\nContent-Type: "));
  client.print(type);
  client.print(F("\r\n\r\n"));
  if(type=="text/html")
  {
    client.print(F("<!DOCTYPE HTML>\r\n<html>\r\n<head><title>"));
    client.print(myhostname);
    client.print(F("</title></head><body>"));
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

bool wipeMode()
{
  int cycle = (millis() / wait) % (3 * numPixels());
  if(cycle==lastCycle) return true;
  uint32_t color;
  int pass = floor(cycle / numPixels());
  int pixi = cycle % numPixels();
  if(pass==2)
    color = color3;
  else if(pass==1)
    color = color2;
  else if(pass==0)
    color = color1;
  if(lastPixi<pixi)
    for(int i=lastPixi+1;i<=pixi;i++)
      strip.setPixelColor(i, color);
  else
    strip.setPixelColor(pixi, color);
  strip.show();
  lastPixi = pixi;
  return true;
}

void loop() {
  switch(smode){
    case 0: // All on
      setColor(color1);
      break;
    case 1: // All off
      setColor(black);
      break;
    case 2: // Christmas chase
      if(!wipeMode()) return;
      break;
    case 3: // Twinkle
      if(!twinkle(color1, color2, color3)) return;
      break;
  }
  doEvents();
}

int numPixels() { return strip.numPixels(); }

bool delay2(int t) {
  if(!bconn) {
    delay(t);
    return true;
  }
  uint32_t start=millis();
  while(millis()-start<t)
  {
    if(bconn && !doEvents()) return false;
    delay(1);
  }
}
bool doEvents() {
  bool ret = true;
#ifdef OLD_WIFI_SERVER
  WiFiClient client = server.available();
  if(!client) return true;
  if(!client.available()) return true;
  String req = client.readStringUntil('\r');
  req = req.substring(req.indexOf(" ")+1,req.lastIndexOf(" "));
  #ifdef DEBUG
  Serial.print("Request received: ");
  Serial.println(req);
  #endif
  client.flush();
  if(req.indexOf("/off")>-1)
  {
    fadeBrightness(0);
    bright = 0;
    smode = 1;
    ret = false;
  }
  else if(req.indexOf("/on")>-1)
  {
    setColor(color1);
    fadeBrightness(100);
    bright = 100;
    smode = 0;
    ret = false;
  }
  else if(req.indexOf("/rgb/")>-1)
  {
    int pos = 5;
    String sR = req.substring(pos,req.indexOf("/",pos));
    pos = 6 + sR.length();
    String sG = req.substring(pos,req.indexOf("/",pos));
    pos = 7 + sR.length() + sG.length();
    String sB = req.substring(pos);
    pos = 8 + sR.length() + sG.length() + sB.length();
    if(sB.indexOf("/")>-1)
      sB = sB.substring(0,sB.indexOf("/"));
    #ifdef DEBUG
    Serial.print("Setting color to rgb(");
    Serial.print(sR);
    Serial.print(",");
    Serial.print(sG);
    Serial.print(",");
    Serial.print(sB);
    Serial.println(")");
    #endif
    color1 = strip.Color(sR.toInt(), sG.toInt(), sB.toInt());
    #ifdef ENABLE_EEPROM
    saveColors();
    EEPROM.commit();
    #endif
    setColor(color1);
    smode = 0;
    ret = false;
  }
  else if(req.indexOf("/rgbw/")>-1)
  {
    int pos = 5;
    String sR = req.substring(pos,req.indexOf("/",pos));
    pos = 6 + sR.length();
    String sG = req.substring(pos,req.indexOf("/",pos));
    pos = 7 + sR.length() + sG.length();
    String sB = req.substring(pos,req.indexOf("/",pos));
    pos = 8 + sR.length() + sG.length() + sB.length();
    String sW = req.substring(pos);
    if(sW.indexOf("/")>0)
      sW = sW.substring(0,sW.indexOf("/"));
    color1 = strip.Color(sR.toInt(), sG.toInt(), sB.toInt(), sW.toInt());
    #ifdef ENABLE_EEPROM
    saveColors();
    EEPROM.commit();
    #endif
    setColor(color1);
    smode = 0;
    ret = false;
  }
  else if(req.indexOf(F("/color/"))>-1)
  {
    String sC = req.substring(7);
    if(sC.indexOf("/")>-1)
      sC = sC.substring(0,sC.indexOf("/"));
    color1 = sC.toInt();
    if(req.indexOf("/", 8 + sC.length()) > -1)
    {
      String sC2 = req.substring(req.indexOf("/", 8 + sC.length())+1);
      if(sC2.indexOf("/")>-1)
        sC2 = sC2.substring(0,sC2.indexOf("/"));
      if(sC2.length()>0)
        color2 = sC2.toInt();
      if(req.indexOf("/", 9 + sC.length() + sC2.length()) > -1)
      {
        String sC3 = req.substring(req.indexOf("/", 9 + sC.length() + sC2.length()) + 1);
        if(sC3.indexOf("/") > -1)
          sC3 = sC3.substring(0, sC3.indexOf("/"));
        if(sC3.length() > 0)
          color3 = sC3.toInt();
      }
    }
    #ifdef DEBUG
    Serial.print("Setting colors to [");
    Serial.print(color1, HEX);
    Serial.print(", ");
    Serial.print(color2, HEX);
    Serial.print(", ");
    Serial.print(color3, HEX);
    Serial.println("]");
    #endif
    #ifdef ENABLE_EEPROM
    saveColors();
    EEPROM.commit();
    #endif
  }
  else if(req.indexOf(F("/bright/"))>-1)
  {
    String sB = req.substring(8);
    if(sB.indexOf("/")>-1)
      sB = sB.substring(0,sB.indexOf("/"));
    int b = sB.toInt();
    #ifdef DEBUG
    Serial.print("Setting brightness to [");
    Serial.print(b);
    Serial.println("]");
    #endif
    fadeBrightness(b);
    bright = b;
  }
  else if(req.indexOf(F("/demo"))>-1)
  {
    bright = 100;
    smode = 2;
    ret = false;
  }
  else if(req.indexOf(F("/twinkle"))>-1)
  {
    bright = 100;
    smode = 3;
    ret = false;
  }
  else if(req.indexOf(F("/red/"))>-1)
  {
    #ifdef DEBUG
    Serial.println("Setting color to red");
    #endif
    setColor(red);
    color1 = red;
    ret = false;
  } else {
    sendClientMessage(client, "Invalid Command", 400);
    return ret;
  }
  sendClientMessage(client, "Done");
#else
  server.handleClient();
#endif
  return ret;
}

bool twinkle(uint32_t c1, uint32_t c2, uint32_t cstay) {
  uint32_t passed = millis() - start;
  //int cycle = (passed / pause) % 2;
  int cycle = (millis() / pause) % 2;
  if(cycle==lastCycle) return true;
  for(int pos=0;pos<numPixels();pos+=3){
    setPixelColor(pos+1, cycle==0?c1:c2);
    setPixelColor(pos+2, cycle==0?c2:c1);
    setPixelColor(pos, cstay);
 }
 stripShow();
 lastCycle = cycle;
 return true;
}

uint32_t stripColor(uint16_t r, uint16_t g, uint16_t b)
{
  return strip.Color(r,g,b);
}
uint32_t stripColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
  return strip.Color(r,g,b,w);
}

void fadein(uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t wait)
{
  for (uint8_t x = 0; x < len; x++)
  {
    float m = (float)x / (float)len;
    uint16_t c = stripColor(
                   r * m, g * m, b * m
                 );
    for (uint16_t i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, c);
    }
    stripShow();
    delay2(wait);
  }
}

void fadeout(uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t wait)
{
  for (uint8_t x = len - 1; x >= 0; x--)
  {
    float m = (float)x / (float)len;
    uint16_t c = stripColor(
                   r * m, g * m, b * m
                 );
    for (uint16_t i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, c);
    }
    stripShow();
    delay2(wait);
  }
}

void trail(uint8_t r, uint8_t g, uint8_t b, uint8_t len, uint8_t wait)
{
  for (uint16_t i = 0; i < numPixels(); i++)
  {
    for (uint16_t j = 0; j < len; j++)
    {
      uint16_t k = i - j;
      if (k < 0) k += numPixels();
      float x  = (1.0 / (float)len) * ((float)j + 1.0);
      if (x == 0) continue;
      uint32_t c = stripColor(r / x, g / x, b / x);
      setPixelColor(k, c);
    }
    if (i >= len)
      setPixelColor(i - len, black);
    else
      setPixelColor((i - len) + numPixels(), black);
    stripShow();
    delay2(wait);
  }
  for (uint16_t i = numPixels() - len; i < numPixels(); i++)
    setPixelColor(i, black);
  stripShow();
}

void trinkle(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < numPixels(); i++) {
    setPixelColor(i, c);
    stripShow();
    delay2(wait);
    setPixelColor(i, stripColor(0, 0, 0));
    stripShow();
  }
}

void setPixelColor(int pos, uint32_t color)
{
//  for(int i = 0; i < 8; i++)
    strip.setPixelColor(pos, color);
}
void setStripColor(uint32_t color)
{
  for(int i = 0; i < strip.numPixels(); i++)
    strip.setPixelColor(i, color);
  strip.show();
}

// Fill the dots one after the other with a color
bool colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < numPixels(); i++) {
    setPixelColor(i, c);
    stripShow();
    if(!delay2(wait)) return false;
  }
  return true;
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < numPixels(); i++) {
      setPixelColor(i, Wheel((i + j) & 255));
    }
    stripShow();
    delay2(wait);
  }
}

// Sstriply different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < numPixels(); i++) {
      setPixelColor(i, Wheel(((i * 256 / numPixels()) + j) & 255));
    }
    stripShow();
    delay2(wait);
  }
}

//Theatre-style crawling strips.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < numPixels(); i = i + 3) {
        setPixelColor(i + q, c);  //turn every third pixel on
      }
      stripShow();

      delay2(wait);

      for (int i = 0; i < numPixels(); i = i + 3) {
        setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling strips with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < numPixels(); i = i + 3) {
        setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      stripShow();

      delay2(wait);

      for (int i = 0; i < numPixels(); i = i + 3) {
        setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return stripColor(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return stripColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return stripColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}
