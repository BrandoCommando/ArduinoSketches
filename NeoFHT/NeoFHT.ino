//------------------------------------------------------------------------------
//
//  APC Arduino Masterclass - Project #16 - Mono/Stereo Audio Spectrum Analyser
//
//  Using:
//     * ArduinoFHT library from OpenMusicLabs
//     * WS182B 150 RGB LED 5m strip
//     * Arduino Uno R3
//
//-------------------------------------------------------------------------------

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LIN_OUT 1
#define FHT_N 128 // set to 256 point fht

#include <FHT.h> // include the library

#define USE_FHT    true
#define AIN_LEFT   0x44 // ADC4
#define AIN_RIGHT  0x45 // ADC5
#define PIXEL_COUNT 50
#define DISPLAY_SIZE 6
#define PIXEL_OFFSET 1
#define PIXEL_PIN 7
#define DC_OFFSET 0
#define NOISE 0
#define RAW_HISTORY 60
#define BUTTON_PIN 3
#define SPECTRUM_REFRESH 100

unsigned long startTime, endTime, oldTime, lastPress, lastStart;
int samples[DISPLAY_SIZE];
int displaySize = DISPLAY_SIZE;
int pixelWidth = PIXEL_COUNT / DISPLAY_SIZE;
int cycle = 0;
int rawHistory[RAW_HISTORY];
int lvl = 10, peak = 0, rawCount, minLvlAvg, maxLvlAvg;
int displayMode = 0, patternCycle = 0;
uint16_t patternLoop = 0;
byte leftGood = 0, rightGood = 0;
uint8_t sampleSide = 0;
uint8_t sampleRange = 10;
uint8_t last13 = 0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
uint32_t black = strip.Color(0, 0, 0);
uint32_t white = strip.Color(255, 255, 255);
uint32_t red = strip.Color(200, 0, 0);
uint32_t green = strip.Color(0, 255, 0);

void setup() {
  memset(samples, 0, sizeof(samples));
  strip.begin();
//  strip.setBrightness(20);
//  strip.setPixelColor(0, red);
  strip.show(); // Initialize all pixels to 'off'

  if(USE_FHT)
  {
    //TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe7; // set the adc to free running mode
    ADMUX = AIN_LEFT; // use adc5
    DIDR0 = 0x20; // turn off the digital input for adc5
    ADMUX = AIN_RIGHT;
    DIDR0 = 0x20;
  }
  pinMode(BUTTON_PIN, INPUT);
}

void loop()
{
  startTime = millis();
  if(digitalRead(BUTTON_PIN) == HIGH)
  {
    if(lastPress - startTime > 1000)
    {
      patternLoop = 0;
      lastPress = startTime;
      displayMode++;
    }
  }
  switch(displayMode)
  {
      case 0: runPatterns(); break;
      case 1: runSpectrum(); break;
      case 2: runAllOn(); break;
      case 3: runAllOff(); break;
      default: displayMode = 0; break;
  }
  endTime = millis();
}

void runAllOn()
{
  if(patternLoop == 0)
  {
    patternLoop = 1;
    strip.setBrightness(64);
    for(int i=0; i<PIXEL_COUNT; i++)
      strip.setPixelColor(i, white);
  }
}

void runAllOff()
{
  if(patternLoop == 0)
  {
    patternLoop = 1;
    for(int i=0; i<PIXEL_COUNT; i++)
      strip.setPixelColor(i, black);
  }
}

void runSpectrum()
{
  if(lastStart == 0)
    lastStart = startTime;
  else if(lastStart - startTime > SPECTRUM_REFRESH)
    lastStart = startTime;
  else return;
  sampleInput();
  if(USE_FHT)
    sampleFix();
  drawSpectrum();
//  delay(100);
}

void runPatterns()
{
  int wait = 100;
  if(lastStart == 0)
    lastStart = startTime;
  else if(lastStart - startTime >= wait)
  {
    patternLoop++;
    lastStart = startTime;
  } else return;
  bool complete = false;
  switch(patternCycle)
  {
    case 0: complete = colorWipe(red); break;
    case 1: complete = colorWipe(green); break;
    case 2: complete = colorWipe(white); break;
    default: patternCycle = 0; break;
  }
  delay(50);
  if(complete) {
    patternLoop = lastStart = 0;
    patternCycle++;
  }
}

bool rainbow() {
  uint16_t i, j = patternLoop;
  if(j >= 256) return true;
  
//  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
//    delay(wait);
//  }
  return false;
}

void colorAll(uint32_t c) {
  for(int i = 0; i < PIXEL_COUNT; i++)
    strip.setPixelColor(i, c);
  strip.show();
}

// Fill the dots one after the other with a color
bool colorWipe(uint32_t c) {
  if(patternLoop == 0)
    colorAll(black);
  if(patternLoop >= PIXEL_COUNT) return true;
  strip.setPixelColor(patternLoop, c);
  strip.show();
  return false;
}

void drawSample(int x, byte y)
{
  uint32_t color;
  cycle = x % 2;
  if(cycle == 0)
    color = strip.Color(y * 50, 0, 0);
  else if(cycle == 1)
    color = strip.Color(0, y * 50, 0);
  else
    color = strip.Color(0, 0, y * 50);
//  if(cycle++ >= 3) cycle = 0;
  for(int i = x; i < x + pixelWidth; i++)
    strip.setPixelColor(PIXEL_OFFSET + i, color);
}
void drawSpectrum ()
{
  for(int x=0; x<DISPLAY_SIZE; x++)
  {
    byte s = samples[x];
    drawSample(x,s);
  }
  strip.show();
//  delay(50);
}

int readInput(uint8_t adc)
{
  ADMUX = adc;
  while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA = 0xf5;
  byte m = ADCL; // fetch adc data
  byte j = ADCH;
  int k = (j << 8) | m; // form into an int
  k -= 0x0200; // form into a signed int
  k <<= 6; // form into a 16b signed int
  ADCSRA = 0xf5;
  return k;
}

void sampleInput() {
  if(USE_FHT)
    sampleInputFHT();
  else
    sampleInputRaw();
}
void sampleInputRaw() {
  int n, k, x, minLvl, maxLvl;
  n = (analogRead(4) + analogRead(5)) / 2;
  n = abs(n - 512 - DC_OFFSET);
  n = (n <= NOISE) ? 0 : (n - NOISE);
  lvl = ((lvl * 7) + n) >> 3;
  k = sampleRange * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);
  if(k <= 0L) k = 0;
  else if(k > sampleRange) k = sampleRange;
  if(k > peak) peak = k;
  if((maxLvl - minLvl) < sampleRange) maxLvl = minLvl + sampleRange;
  for(x = 0; x < displaySize; x++)
    samples[x] = k;
  rawHistory[rawCount] = n;
  if(++rawCount >= RAW_HISTORY) rawCount = 0;
  minLvl = maxLvl = rawHistory[0];
  for(x=1; x<RAW_HISTORY; x++) {
    if(rawHistory[x] < minLvl) minLvl = rawHistory[x];
    else if(rawHistory[x] > maxLvl) maxLvl = rawHistory[x];
  }
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6;
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6;
}
void sampleInputFHT() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int x=0; x < FHT_N; x++) {
    int left = readInput(AIN_LEFT);
    int right = readInput(AIN_RIGHT);
    if(leftGood == 0 && left > 0)
      leftGood = 1;
    if(rightGood == 0 && right > 0)
      rightGood = 1;
    if(leftGood == 1 && rightGood == 1)
      fht_input[x] = (left + right) / 2;
    else if(leftGood == 1)
      fht_input[x] = left;
    else if(rightGood == 1)
      fht_input[x] = right;
//    fht_input[x] = readInput(AIN_LEFT);
//    fht_input[x] = (readInput(AIN_LEFT) + readInput(AIN_RIGHT)) / 2; // put real data into bins
/*
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[x] = k; // put real data into bins
      */
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_lin();
  sei();
}

void sampleFix() {
  int newPos; 
  float fhtCount = FHT_N/2;
  float tempY;
  float groupSize = fhtCount / DISPLAY_SIZE;
  float sum, avg;
  for (int x=0; x < displaySize; x++) {
    sum = 0.0;
    for(int y = x * groupSize; y < (x + 1) * groupSize; y++)
      sum += fht_lin_out[y];
    avg = sum / groupSize;
    samples[x] = ((avg/(FHT_N*2))*sampleRange); // single channel full 16 LED high
  }
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
