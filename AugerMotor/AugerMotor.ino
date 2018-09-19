#define SERVO_PIN 11
#define STEPPER_STEP_PIN 12
#define STEPPER_DIR_PIN 13
#define STEPPER_ENABLE_PIN 11
#define SCALE_PIN_DOUT A3
#define SCALE_PIN_SCK A2
#define SWITCH_PIN 7
#define SLOW_PIN 6 // Reset/Tare when Switch is off
#define ENCODER_A_PIN 10
#define ENCODER_B_PIN 9
#define ENCODER_BTN_PIN 8
#define FULL_PHASE 1000
#define FORWARD_PHASE 750
#define SCALE_UPDATE_FREQ 250
#define SCALE_SMOOTHING 10
#define SCALE_MIN_WAIT 50

//#define DISABLE_SCALE
#define ENABLE_SERVO
//#define ENABLE_STEPPER
//#define LOW_POWER_MANUAL
#define DISABLE_SERIAL

//#define MENU_USERAM
#define MAX_DEPTH 3

#include <EEPROM.h>
#include <menu.h>
#include <LiquidCrystal_I2C.h>//F. Malpartida LCD's driver
#include <menuIO/lcdOut.h>
#ifndef DISABLE_SERIAL
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#endif
#include <menuIO/keyIn.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>

#ifdef ENABLE_SERVO
#include <Servo.h>
#endif

encoderIn<ENCODER_A_PIN,ENCODER_B_PIN> encoder;//simple quad encoder driver
encoderInStream<ENCODER_A_PIN,ENCODER_B_PIN> encStream(encoder,4);// simple quad encoder fake Stream
keyMap encBtn_map[]={{-ENCODER_BTN_PIN,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
keyIn<1> encButton(encBtn_map);//1 is the number of keys
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Servo motor;  // create servo object to control a servo
// twelve servo objects can be created on most boards

const char* units[] = {"g", "oz"};
const PROGMEM char blank_lcd[] = { "                    " };
float scaleFactor = 696.0;
#ifdef ENABLE_STEPPER
const unsigned int full_speed_sleep = 480;
const float normSpeed = .8f;
const float slowSpeed = .2f;
unsigned long lastStep = 0;
bool lastBit = 0;
#endif
#ifdef NEW_STATUS
String lastStatus;
long lastStatusUpdate = 0.0f;
#endif
unsigned long lastReadMillis = 0;
unsigned long lastRead[SCALE_SMOOTHING];
int lastRI = 0;
unsigned long lastUpdate = 0;
float lastWeight = 0.0f;
float tareVal = 0.0f;
int selUnit = 0;
int middle = 0;    // variable to store the servo position
unsigned long phase = 0;
unsigned long starts;
float grams = 25;
int pockets = 24;
int pocketNumber = 0;
float pounds = 5;
int lastMode = 0;
byte isEnabled = 1;
int scale_gain = 1;

void setScaleGain(int gain)
{
  if(gain == 128) scale_gain = 1;
  else if(gain == 64) scale_gain = 3;
  else if(gain == 32) scale_gain = 2;
  else if(gain > 0 && gain <= 3) scale_gain = gain;
  digitalWrite(SCALE_PIN_SCK, LOW);
  readLongScale();
}
bool readyToScale() { return digitalRead(SCALE_PIN_DOUT) == LOW; }
long readLongScale() {
  #ifndef DISABLE_SERIAL
  Serial.print("Reading");
  #endif
  while (!readyToScale()) {
    #ifndef DISABLE_SERIAL
    Serial.print(".");
    #endif
    yield();
  }

  #ifndef DISABLE_SERIAL
  Serial.print(":");
  #endif
  
  byte data[3];
  for(byte j=3;j--;) {
    data[j] = shiftIn(SCALE_PIN_DOUT, SCALE_PIN_SCK, MSBFIRST);
    #ifndef DISABLE_SERIAL
    Serial.print(data[j]);
    Serial.print(".");
    #endif
  }

  for(int i=0; i < scale_gain; i++) {
    digitalWrite(SCALE_PIN_SCK, HIGH);
    digitalWrite(SCALE_PIN_SCK, LOW);
  }

  data[2] ^= 0x80;

  #ifndef DISABLE_SERIAL
  uint32_t ret = ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
  Serial.println(ret);
  return ret;
  #else
  return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
  #endif
}

void calcWeight() {
  grams = 28.35 * (pounds * 16.0f / pockets);
}
result updateWeight(eventMask e,navNode& nav, prompt &item) {
  #ifndef DISABLE_SERIAL
  Serial.print("Event: ");
  Serial.println(e);
  #endif
  if(e == Menu::exitEvent)
    saveMemory();
  calcWeight();
  #ifndef DISABLE_SERIAL
  Serial.print("New Weight: ");
  Serial.println(grams);
  #endif
  return proceed;
}
result doManualWeigh() {
  
}
result doAutoWeigh() {
  return proceed;
}
result updateFactor(eventMask e, navNode& nav, prompt &item) {
  return updateWeight(e, nav, item);
}
result updateGain(eventMask e, navNode& nav, prompt &item) {
  setScaleGain(scale_gain);
  return proceed;
}
result nextPocket(eventMask e, navNode& nav, prompt &item) {
  pocketNumber++;
  updatePocket(nav);
  return doAutoWeigh();
}
result idle(menuOut& o,idleEvent e) {
  switch(e) {
    case idleStart:lcd.off();isEnabled=0;break;
    case idling:o.print("suspended...");break;
    case idleEnd:lcd.on();isEnabled=1;break;
  }
  return proceed;
}
CHOOSE(selUnit, unitMenu, "Unit:", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,VALUE("Grams", 0, Menu::doNothing, Menu::noEvent)
  ,VALUE("Ounces", 1, Menu::doNothing, Menu::noEvent)
  );
MENU(configMenu, "Settings", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,SUBMENU(unitMenu)
  ,FIELD(pockets, "Pockets:", "", 0, 96, 4, 1, updateWeight, Menu::exitEvent | Menu::updateEvent, Menu::noStyle)
  ,FIELD(scaleFactor, "Factor:", "", -2000, 500, 100, 1, updateFactor, Menu::exitEvent | Menu::updateEvent, Menu::noStyle)
  ,FIELD(scale_gain, "Gain:", "", 1, 3, 1, 1, updateGain, Menu::exitEvent, Menu::noStyle)
  ,EXIT("<Back")
  );
/*
MENU(autoMenu, "Auto-Weigh", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,OP("Next", nextPocket, Menu::anyEvent)
  //,EDIT("Pocket #", autoPocket, nums, Menu::doNothing, Menu::noEvent, Menu::wrapStyle)
  ,EXIT("<Back")
  );
*/
MENU(mainMenu, "Weigh Something!", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(pounds, "Total", "lb", 0, 50, 1, 0.1, updateWeight, Menu::exitEvent | Menu::updateEvent, Menu::noStyle)
//  ,SUBMENU(autoMenu)
  ,FIELD(grams, "Each", "g", 0, 1000, 10, 1, Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,SUBMENU(configMenu)
  ,EXIT("<EXIT")
  );

#ifndef DISABLE_SERIAL
Menu::serialIn serial(Serial);
Menu::menuIn* inputsList[] = {&encStream, &encButton, &serial};
Menu::chainStream<3> menu_in(inputsList);
#else
Menu::menuIn* inputsList[] = {&encStream, &encButton};
Menu::chainStream<2> menu_in(inputsList);
#endif

//MENU_OUTPUTS(out,MAX_DEPTH,SERIAL_OUT(Serial),NONE);
MENU_OUTPUTS(menu_out, MAX_DEPTH, LCD_OUT(lcd,{0,0,20,3}), NONE);

NAVROOT(nav,mainMenu,MAX_DEPTH,menu_in,menu_out);
//*/
void updatePocket(navNode& n) {
  if(nav.active().enabled==Menu::enabledStatus)
    nav.active().disable();
  else nav.active().enable();
}
void saveMemory() {
  int mind = 10;
  EEPROM.put(mind++, (byte)60);
  EEPROM.put(mind, pounds);
  mind += 8;
  EEPROM.put(mind, pockets);
  mind += 4;
  EEPROM.put(mind, scaleFactor);
  #ifndef DISABLE_SERIAL
  Serial.print("Data saved (");
  Serial.print(mind);
  Serial.println(").");
  #endif
}
void loadMemory() {
  int mind = 10;
  byte test;
  EEPROM.get(mind, test);
  mind++;
  if(test != 60)
  {
    #ifndef DISABLE_SERIAL
    Serial.print("Memory has not been initialized (");
    Serial.print(test);
    Serial.println("). Reverting to defaults.");
    #endif
    saveMemory();
    return;
  }
  EEPROM.get(mind, pounds);
  if(isnan(pounds)||pounds<=0) pounds = 5.0;
  mind += 8;
  EEPROM.get(mind, pockets);
  if(isnan(pockets)||pockets<=0) pockets = 24;
  mind += 4;
  EEPROM.get(mind, scaleFactor);
  if(isnan(scaleFactor)||scaleFactor==0) scaleFactor = -19.8;
  #ifndef DISABLE_SERIAL
  Serial.print("Scale Factor: ");
  Serial.println(scaleFactor);
  #endif
}
void setup() {
  #ifndef DISABLE_SERIAL
  Serial.begin(74880);
  while(!Serial);
  #endif
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(SLOW_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
  loadMemory();
  encoder.begin();
  lcd.begin(20,4);
  #ifdef ENABLE_STEPPER
  pinMode(STEPPER_STEP_PIN, OUTPUT);
  pinMode(STEPPER_DIR_PIN, OUTPUT);
  pinMode(STEPPER_ENABLE_PIN, OUTPUT);
  digitalWrite(STEPPER_ENABLE_PIN, HIGH);
  #endif
  nav.showTitle=false;
  nav.idleTask=idle;
  lcd.setCursor(0, 0);
  lcd.print("PushWeigh");
  lcd.setCursor(0, 2);
  lcd.print("Starting Scale");
  pinMode(SCALE_PIN_SCK, OUTPUT);
  pinMode(SCALE_PIN_DOUT, INPUT);
  calcWeight();
  fillScaleBuffer();
  #ifndef DISABLE_SCALE
  tareVal = getLongWeight();
  #endif
  #ifdef ENABLE_SERVO
  motor.attach(SERVO_PIN);
  middle = motor.read();
  if(middle==0) middle = 90;
//  motor.write(90);
  motor.detach();
  #endif
  lcd.setCursor(0, 2);
  lcd.print("Ready!");
  #ifndef DISABLE_SERIAL
  Serial.println("Ready!");
  #endif
  starts = millis();
}
int getMode() {
  int switched = digitalRead(SWITCH_PIN) == LOW ? 1 : 0;
  int slow = digitalRead(SLOW_PIN) == LOW ? 1 : 0;
  String sl = "Status: ";
  if(slow&&!switched) {
    sl += "Resetting...";
    tareVal = getLongWeight();
  } else {
    if(!switched) {
//      sl += "Ready!";
      return 0;
    } else {
      sl += "Spinning";
      if(slow)
        sl += " (Slow)";
      #ifdef LOW_POWER_MANUAL
      return slow ? 2 : 1;
      #endif
    }
  }
//  #ifdef DISABLE_SCALE
  while(sl.length() < 20)
    sl += " ";
  lcd.setCursor(0, 3);
  lcd.print(sl);
//  #endif
  return (switched ? (slow ? 2 : 1) : 0);
}

long getLongWeight() // Average Reading
{
  long ttl = 0;
  for(int i=0;i<SCALE_SMOOTHING;i++)
    ttl += lastRead[i];
  return ttl / SCALE_SMOOTHING;
}
float getLastWeight()
{
  return (getLongWeight() - tareVal) / scaleFactor;
}

/*
int printFloatToLCD(float f, byte points)
{
  int wholes = floor(f);
  int tenths = floor((f - wholes) * pow(10, points));
  lcd.print(wholes);
  lcd.print(".");
  lcd.print(tenths);
  if(wholes > 1000)
    return 4+points;
  if(wholes > 100)
    return 3+points;
  if(wholes > 10)
    return 2+points;
  if(wholes >= 0)
    return 1+points;
  if(wholes < -1000)
    return 5+points;
  if(wholes < -100)
    return 4+points;
  if(wholes < -10)
    return 3+points;
  return 2+points;
} //*/

int getFloatLength(float f)
{
  if(f>=1000) return 7;
  if(f>=100) return 6;
  if(f>=10) return 5;
  if(f>=0) return 4;
  if(f<=-1000) return 8;
  if(f<=-100) return 7;
  if(f<=-10) return 6;
  return 5;
}
int getUnitPoints(byte unit)
{
  if(unit==0) return 1;
  return 2;
}
float getConvertedWeight(float grams, byte unit)
{
  if(unit==1) return grams / 28.35;
  return grams;
}
int printWeightToLCD(float grams, byte unit, bool plus)
{
  if(plus&&grams>0) lcd.print("+");
  float weight = getConvertedWeight(grams, unit);
  lcd.print(weight);
  //printFloatToLCD(getConvertedWeight(grams, unit), getUnitPoints(unit));
  lcd.print(units[unit]);
  return getFloatLength(weight) + (unit==0?1:2) + (plus&&grams>0?1:0);
  //return len + (unit==0?1:2) + (plus&&grams>0?1:0);
}

void loop() {
  int mode = getMode();
  #ifdef LOW_POWER_MANUAL
  if(mode > 0)
  {
    if(lastMode==0)
    {
      digitalWrite(SCALE_PIN_SCK, LOW);
      digitalWrite(SCALE_PIN_SCK, HIGH);
      nav.idleOn();
      lcd.off();
      lastMode = mode;
      delay(50);
      return;
    }
    lastMode = mode;
    runMotor();
    return;
  } else if(mode==0&&lastMode>0)
  {
    digitalWrite(SCALE_PIN_SCK, LOW);
    nav.idleOff();
    lcd.on();
    delay(50);
    lastMode = mode;
    return;
  }
  #endif
  nav.poll();
  if(!isEnabled) {
    if(digitalRead(SWITCH_PIN)==LOW||
      digitalRead(SLOW_PIN)==LOW)
    {
      nav.idleOff();
    }
    return;
  }
  #ifndef DISABLE_SCALE
  if(mode==0) {
    checkScale();
  }
  #endif
  runMotor();
}
void fillScaleBuffer() {
  for(int i=0;i<SCALE_SMOOTHING;i++)
    lastRead[i] = readLongScale();
}
void checkScale() {
  unsigned long now = millis();
  if(now - lastReadMillis <= SCALE_MIN_WAIT) return;
  lastReadMillis = now;
  lastRead[lastRI] = readLongScale();
  lastRI = (lastRI + 1) % SCALE_SMOOTHING;
  lastWeight = getLastWeight();
  if(millis() - lastUpdate <= SCALE_UPDATE_FREQ) return;
  lcd.setCursor(0, 3);
  lcd.setCursor(0, 3);
//      float left = grams - lastWeight;
  int written = 3;
  written += printWeightToLCD(lastWeight, selUnit, false);
  lcd.print(" : ");
  written += printWeightToLCD(grams - lastWeight, selUnit, true);
  while(++written<19)
    lcd.print(" ");
  lcd.flush();
  /*
  lcd.print(lastWeight);
  lcd.print(" ");
  lcd.print(units[selUnit]);
  lcd.print(" : ");
  float left = grams - lastWeight;
  if(left > 0) lcd.print("+");
  lcd.print(left);
  lcd.print("   ");
  */
  lastUpdate = millis();
}
void runMotor() {
  #ifdef ENABLE_SERVO
  runServo();
  #endif
  #ifdef ENABLE_STEPPER
  runStepper();
  #endif
}
void runServo() {
  int mode = getMode();
  if(mode==0&&lastMode!=0) { // stopped
    motor.detach();
  }
  lastMode = mode;
  if(mode==0) return;
  phase = (millis() - starts);
  if(phase > FULL_PHASE)
  {
    starts = millis();
    phase = 0;
  }
  if(!motor.attached()) motor.attach(SERVO_PIN);
  if(mode==1) {
    if(phase < FORWARD_PHASE) // forward
      motor.write(middle+80);
    else
      motor.write(middle-80);
//    delay(10);
    return;
  } else if(mode==2) { // slow
    if(phase < FORWARD_PHASE)
      motor.write(middle+10);
    else
      motor.write(middle-80);
//    delay(10);
  }
}
#ifdef ENABLE_STEPPER
void runStepper() {
  int mode = getMode();
  if(mode==0&&lastMode!=0) {
    digitalWrite(STEPPER_ENABLE_PIN, HIGH);
  } else if(mode>0&&lastMode==0) {
    digitalWrite(STEPPER_ENABLE_PIN, LOW);
  }
  lastMode = mode;
  if(mode == 0) return;
  long newphase = (millis() - starts) % FULL_PHASE;
  if((newphase > FORWARD_PHASE) != (phase > FORWARD_PHASE))
  {
    if(newphase < FORWARD_PHASE) digitalWrite(STEPPER_DIR_PIN, LOW);
    else digitalWrite(STEPPER_DIR_PIN, HIGH);
  }
  phase = newphase;
  takeStep(mode);
}
void takeStep(int mode) {
  unsigned long now = micros();
  float sleep = full_speed_sleep / max(1.0f, ((mode == 1) ? normSpeed : slowSpeed));
  if(now > lastStep + sleep)
  {
    lastStep = now;
    lastBit = !lastBit;
    digitalWrite(STEPPER_STEP_PIN, lastBit);
  }
}
#endif

#ifdef NEW_STATUS
void setStatus(String line)
{
  if(lastStatusUpdate - millis() > SCALE_UPDATE_FREQ)
  {
    lastStatusUpdate = millis();
    lastStatus = line;
    lcd.setCursor(0, 3);
    lcd.print(line);
  }
}
#endif
