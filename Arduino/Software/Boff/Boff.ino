#include <MQTTClient.h>
#include <system.h>
#include <WiFi101.h>
#include <RTCZero.h>
#include <FastLED.h>
#include <SparkFunCCS811.h>
#include "customTypes.h"


// Global fan info settings.
// used by LEDs and fan control
// as well as other places (e.g. setting fan speed).
// Fans 1,2,3, 4 and 5 , indexed as 0..4
fanInfo_t fanInfos[5];

// Not used in the fan box.
//int switch_pins[] = {A1, A2};
//int switch_leds[] = {A3, A3};
// Only on prototype PCB. Needs to be pulled to GND
//int switch_enable = A5;

// 0: Ignore - manual
// 1: Temperature 
// 2: Humidity
// 3: Pressure
// 4: Air Quality
// 5: Clock
// 6: circle (single fan)
// 7: circle (all fans)
// 8: Dust
// 10: Pomodoro (work + Play)
// 11: Pomodoro Work
// 12: Pomodoro Play
// 13: Fixed Color
// 14: lightLevel
// 15: SelectedFanSpeed (0..11)
// 16: FanSpeed
// 17: WiFiStrength
// 18: OnOff
// 19: MqttFeed
// 255: Automatic
// TODO: Load this from EEPROM or something.
// Let it be settable via MQTT/Alexa/////
DisplayMode fanDisplayModes[] = {
  DisplayMode::Temperature, 
  DisplayMode::Humidity, 
  DisplayMode::AirQuality, 
  DisplayMode::WiFiStrength};

// running LED Index, by "Hour" (0 top, 11 at 11 o'clock...)
int redHourIndex = 0;
int lastRedHourIndex = 0;
CRGB ledsSetColor = CRGB::Red;

// How bright to make the LEDs.
int ledBrightness = 32;

//#define NUM_LEDS 24
// 4 Fans, 16 LEDs per fan = 64
// 2 1M strips of LEDs, 120 LEDS per M = 240
// 2 1M strips of LEDs, 90 LEDS per M = 180
// 64 + 240 = 304
#define NUM_LEDS 64
// ech fan has 16ish...
CRGB leds[NUM_LEDS];

// If the LEDs are enabled (false = LEDs off - dark)
bool ledsEnabled = true;

// --------------------------------
// Sensor values (used across the application).
// --------------------------------
// BME 280 (or 680)
// Guess at appropriate values whilst not available to be read.
float humidity = 50;
float temperature = 22;
float pressure = 1015.2;

// TODO: 680 toc/air quality

// CCS811 values.
long ccs811DataUsableAfter;
unsigned int ccsBaseline;
unsigned int tVOC = 0;
unsigned int eCO2 = 400;

int light = 34;

// measured rssi.
int rssi;

float voltage = 0.0;

// =============================================

// External MQTT feeds (0..3).
// These should be mapped to the fan they are displayed on
// or the other way around (i.e. feed[2] of interest, 
// used fan[2] to show that.
String mqttFeedsTopic[4] = {"/Radiation/cpm", "", "", ""};

// TODO: The feed value should be normalised into 23 steps with
// 0 = desired value. +11 high, -11 low.
// If the value never goes below the desited valus
// then still use +/-11 just the -1..-11 is never displayed.
// For on/off, anything > than 0 is on.
int mqttFeedsValue[4] = {0,0,0,0};

RTCZero rtc;

// Sensor display range settings.
displayRange_t temperatureRange;
displayRange_t humidityRange;
displayRange_t pressureRange;
displayRange_t airQualityRange;
displayRange_t dustRange;
displayRange_t wifiDisplayRange;

// LED 1 Nose Green: Fans and Neopixel setup done.
// LED 2 Nose Green: Serial port wait done.
// LED 3 Nose Green: WiFi done.
// LED 4 Nose Green: MQTT done.
// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Small (diagnostic) reboot delay
  delay(1000);

  setupFans();
  setupNeopixels();
  showSetupStageComplete(1);
  delay(1000);
   
  //Initialize serial:
  Serial.begin(9600);
  serialConnectDelay();
  Serial.println("Serial setup complete");
  showSetupStageComplete(2);

  // Setup the display ranges used to show values
  // on the fans.
  temperatureRange = setupTemperatureDisplayRange();
  humidityRange = setupHumidityDisplayRange();
  pressureRange = setupPressureDisplayRange();
  airQualityRange = setupAirQualityDisplayRange();
  wifiDisplayRange = setupWiFiDisplayRange();

  setupSensors();

  setupWiFi();
  showSetupStageComplete(3);

  // TODO: Get time from NTP server
  rtc.begin();
  //rtc.setTime(04, 40, 20);
  //rtc.setDate(21, 01, 2018);
  printCurrentDateTime();
  delay(2000);

  setupMqtt();
  showSetupStageComplete(4);
  
  Serial.println("Boff version 0.2.1");
  Serial.println("------------------------------------------");
}

void serialConnectDelay() {
  for (int i = 0; i<10; i++) {
    Serial.print("Serial wait ");
    Serial.print(i+1);
    Serial.println("......");
    delay(1000);
  }
}

void printCurrentDateTime() {
  // Print date...
  print2digits(rtc.getDay());
  Serial.print("/");
  print2digits(rtc.getMonth());
  Serial.print("/");
  print2digits(rtc.getYear());
  Serial.print(" ");

  // ...and time
  print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());

  Serial.println();
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}

// ==============================================================
// Loop functions
// ==============================================================

// Profiling variables.
unsigned long loop_start;
unsigned long loop_took;

void loop() {
  loop_start = millis();
  digitalWrite(LED_BUILTIN, HIGH); // D6 used for input for dust sensor when fitted.
  delay(100);

  sensorsLoop();
  fansLoop();
  readInput();
  handleNeopixels();

  mqttLoop();
  printHeader();
  printInfo();

  digitalWrite(LED_BUILTIN, LOW);    
  delay(100);

  loop_took = millis() - loop_start;
  //Serial.print("Loop took: ");
  //Serial.print(loop_took);
  //Serial.println("ms");
}

// Loop handler to update the Neopixels (i.e. LED leds + possible others)
// code for this is in the displayLeds file.
void handleNeopixels() {
  updateFansLeds();
  updateStrip1Leds();
  updateStrip2Leds();
  endLedUpdate();

  FastLED.show(); 
}

unsigned long lastPrintInfo = 0;
unsigned long lastHeaderPrinted = 0;
void printInfo() {
  // Print only once per...
  if (lastPrintInfo + 500 > millis()) {
    return;
  }

  Serial.print(temperature);
  Serial.print("\t");
  Serial.print(humidity);
  Serial.print("\t");
  Serial.print(pressure);
  Serial.print("\t");
  Serial.print(eCO2);
  Serial.print("\t");
  Serial.print(tVOC);
  Serial.print("\t");
  Serial.print(light);
  Serial.print("\t");  
  Serial.print(rssi);
  Serial.print("\t");
  Serial.print(voltage);
  Serial.print("\t");
  // Assume all fans have the same set speed.
  Serial.print(fanInfos[0].speedSet);
  Serial.print("\t[");
  for (int fanId=0; fanId<4;fanId++) {
    Serial.print(fanInfos[fanId].computedRpm);
    Serial.print("\t");
  }
  Serial.print("]\t[");
  for (int fanId=0; fanId<4;fanId++) {
    Serial.print(fanDisplayModes[fanId]);
    Serial.print("\t");
  }
  Serial.print("]\t");
  Serial.print(ledBrightness);
  Serial.print("\t");
  Serial.println();

  lastPrintInfo = millis();
}

void printHeader() {
  // Print only once per n samples.
  if (lastHeaderPrinted + 20000 > millis()) {
    return;
  }
  Serial.print("T/°C: ");
  Serial.print("\t");
  Serial.print("RH /%: ");
  Serial.print("\t");
  Serial.print("BP: ");
  Serial.print("\t");
  Serial.print("eCO2: ");
  Serial.print("\t");
  Serial.print("TVOC: ");
  Serial.print("\t");
  Serial.print("Light: ");
  Serial.print("\t");
  Serial.print("RSSI: ");
  Serial.print("\t");
  Serial.print("V in:");
  Serial.print("\t");
  Serial.print("Speed: ");
  Serial.print("\t");
  Serial.print("RPMs: ");
  Serial.print("\t\t\t\t");
  Serial.print("\t");
  Serial.print("Display Mode: ");
  Serial.print("\t\t\t\t");
  Serial.print("Brightness: ");
  Serial.print("\t");
  Serial.println();

  lastHeaderPrinted = millis();
}

int selectedFanId = 1;
  
// ==============================================================
// User input
// ==============================================================
void readInput() {

  if (Serial.available()) {
    char instruction = Serial.read();

    switch (instruction) {
      case '0':
        setFansSpeed(0);
        break;
      case '1':
        setFansSpeed(1);
        break;
      case '2':
        setFansSpeed(7);
        break;
      case '3':
        setFansSpeed(11);
        break;
      case 't': // temperature fan
        Serial.println("Fan 1 selected.");
        selectedFanId = 1;
        setFanBackground(selectedFanId, CRGB::Blue);
        break;
      case 'h': // humidity fan
        Serial.println("Fan 2 selected.");
        selectedFanId = 2;
        setFanBackground(selectedFanId, CRGB::Blue);
        break;
      case 'p': // pressure fan
        Serial.println("Fan 3 selected.");
        selectedFanId = 3;
        setFanBackground(selectedFanId, CRGB::Blue);
        break;
      case 'q': // air quality fan
        Serial.println("Fan 4 selected.");
        selectedFanId = 4;
        setFanBackground(selectedFanId, CRGB::Blue);
        break;
      case 'o': // air quality fan
        SetLedsOnOff(!ledsEnabled);      
        break;
      case ',':
        ledsSetColor = CRGB::Red; 
        break;
      case '.':
        ledsSetColor = CRGB::Green;
        break;
      case '+':
        temperature +=0.25;
        humidity +=2;
        eCO2 +=100;
        pressure +=25;
        break;
      case '-':
        temperature -=0.25;
        humidity -=2;
        eCO2 -=100;
        pressure -=25;
        break;
      case 'm':
        setPower(true);
        break;
      case 'n':
        setPower(false);       
        break;
      case '>':
        ledBrightness+= 5;
        if (ledBrightness > 255){
          ledBrightness = 255;
        }
        break;
      case '<':
        ledBrightness-= 5;
        if (ledBrightness <= 0){
          ledBrightness = 0;
        }
        break;
      default:
        Serial.println("Unknown instruction. Select: 0..3, t, h, p, q, o, +/-, m, n, <, >");
        Serial.println("0..3 - Fan speed (0, 1, 7, 11)");
        Serial.println("t - Select [t]emperature fan");
        Serial.println("h - Select [h]umidity fan");
        Serial.println("p - Select [p]ressure fan");
        Serial.println("q - Select air [q]uality fan");
        Serial.println("o - all LEDs [o]ff");
        Serial.println("+/- - increase/decrease faked values");
        Serial.println("m - [m]aster power on");
        Serial.println("n - [m]aster power off");
        Serial.println("> - brighter");
        Serial.println("< - dimmer");
        break;
    }

    updateFanSpeeds();
    //printVariables();
  }
}

void printVariables() {
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(", Humidity: ");
  Serial.print(humidity);
  Serial.print(", Pressure: ");
  Serial.print(pressure);
  Serial.print(", eCO2: ");
  Serial.print(eCO2);
  Serial.print(", fan speed: ");
  Serial.print(fanInfos[selectedFanId-1].speedSet);
  Serial.println();
}

// General

void sleepNow() {
  Serial.println("Sleep!"); 
  setPower(0);
    // setFansSpeed(0); // leave as is for wake mode.
  SetLedsOnOff(false);
  publishTinamousStatus("Sleep mode activated.");
}

void wakeNow() {
  Serial.println("Wake!"); 
  setPower(1);
  // setFansSpeed(11);?
  SetLedsOnOff(true);
  publishTinamousStatus("Waking up.");
}


// ==========================================================
// Display parameters setup
// ==========================================================

// Setup parameters for temperature display
displayRange_t setupTemperatureDisplayRange() {
  float idealValue = 22;
  int factor = 10;

  displayRange_t range;
  range.idealValue = idealValue * factor;
  
  range.idealRangeLow = (idealValue - 1) * factor; 
  range.idealRangeHigh = (idealValue + 1) * factor; 

  // +/- 6 segments on the display
  range.minValue = (idealValue - 2.5) * factor;  // each segment worth 0.5 C
  range.maxValue = (idealValue + 2.5) * factor; 

  range.factor = factor;
  range.fanSpeedAboveIdeal[0] = 22;
  range.fanSpeedAboveIdeal[1] = 24;
  range.fanSpeedAboveIdeal[2] = 25;
  range.fanSpeedBelowIdeal[0] = 18;
  range.fanSpeedBelowIdeal[1] = 18;
  range.fanSpeedBelowIdeal[2] = 18;
  return range;
}

// Setup parameters for humidity display
displayRange_t setupHumidityDisplayRange() {
  
  float idealValue = 55;
  int factor = 1;

  displayRange_t range;
  range.idealValue = idealValue;

  // 40-60% is "optimal"
  range.idealRangeLow = idealValue - 5; 
  range.idealRangeHigh = idealValue + 5; 

  // this needs to be symmetrical either wide of 
  // the ideal value (at-least until the display 
  // can support it.)
  range.minValue = idealValue - 45; // 10%
  range.maxValue = idealValue + 45; // 100%

  range.factor = factor;
  return range;
}

displayRange_t setupPressureDisplayRange() {
  float idealValue = 1015;
  int factor = 1;

  displayRange_t range;
  range.idealValue = idealValue;
  
  range.idealRangeLow = 1000; 
  range.idealRangeHigh = 1030; 

  // Hack for the -ve value to balance
  // the display.
  range.minValue = 900;
  range.maxValue = 1100;

  range.factor = factor;
  return range;
}

// Using eCO2 as air quality...
// Setup parameters for air quality display
// this is different to temp/humidity in that
// it's only the upper range that matters.
displayRange_t setupAirQualityDisplayRange() {
  float idealValue = 0;
  int factor = 1;

  displayRange_t range;
  range.idealValue = idealValue;
  
  range.idealRangeLow = -1000; 
  range.idealRangeHigh = 1000; 

  // Hack for the -ve value to balance
  // the display.
  range.minValue = -2000;
  range.maxValue = 2000; 

  range.factor = factor;
  return range;
}

// WiFi range...
// 0 to -120 (0 = best).
// map to +/-60.
displayRange_t setupWiFiDisplayRange() {
  float idealValue = 0;
  int factor = 1;

  displayRange_t range;
  range.idealValue = idealValue;

  // Anything 20-60 (i.e. -40 to 0)
  range.idealRangeLow = 20;  // i.e. RSSI = -40 (value - 60)
  range.idealRangeHigh = 60;  // i.e. RSSI = 0

  // Hack for the -ve value to balance
  // the display.
  range.minValue = -60;
  range.maxValue = 60;  // 

  range.factor = factor;
  // todo: offset? (e.g. +60 here).
  return range;
}





