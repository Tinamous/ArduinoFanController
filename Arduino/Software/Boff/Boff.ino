#include <Adafruit_TCS34725.h>
#include <Adafruit_BME680.h>
#include <bme680.h>
#include <bme680_defs.h>
#include <SparkFunCCS811.h>

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
int switch_pins[] = {A1, A2};
int switch_leds[] = {A3, A4};

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
// 100: Fancy
// 255: Automatic
// TODO: Load this from EEPROM or something.
// Let it be settable via MQTT/Alexa/////
DisplayMode fanDisplayModes[] = {
  DisplayMode::Temperature, 
  DisplayMode::Humidity, 
  DisplayMode::AirQuality, 
  DisplayMode::Clock};

// running LED Index, by "Hour" (0 top, 11 at 11 o'clock...)
int redHourIndex = 0;
int lastRedHourIndex = 0;
CRGB ledsSetColor;

// How bright to make the LEDs.
int ledBrightnessPercent = 20;

//#define NUM_LEDS 24
// 4 Fans, 16 LEDs per fan = 64
// 2 1M strips of LEDs, 120 LEDS per M = 240
// 2 1M strips of LEDs, 90 LEDS per M = 180
// 64 + 240 = 304
// 64 (Fans) + 120 + 60-18 (42) (1 stip + 18 LEDs short of 1/2 srtip).
// = 64 + 162 = 226
// + about 74 from the top pointing set
//#define NUM_LEDS 226 + 74
// each fan has 16ish...
// Wooden fan...
#define NUM_LEDS 64
CRGB leds[NUM_LEDS];

// ------------------------------------
// Sensor values (defined here so they 
// can be used across the application).
// -------------------------------------
// What sensors are attached.
bool hasBme280 = false;
bool hasBme680 = false;
bool hasCCS811 = false;
bool hasLightSensor = false;

// BME 280 (or 680)
// Guess at appropriate values whilst not available to be read.
float humidity = 50;
float temperature = 22;
float pressure = 1015.2;
int sensorSource = 0; // 0: Fake, 1: 280, 2: 680

// BME680 specific. toc/air quality 
float gas_resistance;

// CCS811 values.
long ccs811DataUsableAfter;
unsigned int ccsBaseline;
unsigned int tVOC = 0;
unsigned int eCO2 = 400;
uint8_t ccsLastStatusError;

// Light sensor
uint16_t lightLevelLux = 20;
uint16_t redLightLevel = 0;
uint16_t greenLightLevel = 0;
uint16_t blueLightLevel = 0;
uint16_t clearLightLevel = 0;
uint16_t colorTemperature = 0;

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

// =============================================
// Switches
// =============================================
// Switch debounce handling.
volatile bool handle_switch1_pressed = false;
volatile bool handle_switch2_pressed = false;


// LED 1 Nose Green: Fans and Neopixel setup done.
// LED 2 Nose Green: Serial port wait done.
// LED 3 Nose Green: WiFi done.
// LED 4 Nose Green: MQTT done.
// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  setupFans();
  setupNeopixels();
  // Artificial delay so that the first nose
  // isn't instandly green
  delay(5000);
  
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
  rtc.setTime(04, 40, 20);
  rtc.setDate(20, 02, 2018);
  printCurrentDateTime();
  delay(2000);

  setupMqtt();
  showSetupStageComplete(4);

  setupSwitches();
  
  Serial.println("Boff version 0.2.3");
  Serial.println("------------------------------------------");

  // Switch the Switch LED on to indicate we're ready...
  setSwitchLEDs(HIGH);
}

void serialConnectDelay() {
  for (int i = 0; i<5; i++) {
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

void setupSwitches() {
  
  for (int i=0; i<2; i++) {
      pinMode(switch_leds[i], OUTPUT);
      digitalWrite(switch_leds[i], LOW);
      pinMode(switch_pins[i], INPUT_PULLUP);
  }

  attachInterrupt(switch_pins[0], switch1_pressed, FALLING); 
  attachInterrupt(switch_pins[1], switch2_pressed, FALLING); 
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

  sensorsLoop();
  fansLoop();
  readInput();
  ledsLoop();
  handleSwitches();
  mqttLoop();
  
  printHeader();
  printInfo();

  digitalWrite(LED_BUILTIN, LOW);    
  // Minimum delay, otherwise WiFi/MQTT processing
  // doesn't happen and we keep disconnecting.
  delay(20);

  fanSpeedDelay();

  

  loop_took = millis() - loop_start;
  //Serial.print("Loop took: ");
  //Serial.print(loop_took);
  //Serial.println("ms");
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
  Serial.print(gas_resistance);
  Serial.print("\t");
  Serial.print(lightLevelLux);
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
  Serial.print(ledBrightnessPercent);
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
  Serial.print("Gas R'");
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

// ==============================================================
// Switches (Optional)
// ==============================================================
void handleSwitches() {
  // Interrupt from switch 1...
  if (handle_switch1_pressed) {

    Serial.println("Switch 1 Interrupt handling");

    // Ensure we have atleast 200ms delay before checking if the switch is still pressed.
    delay(200);

    // If the switch is now high, ignore it, probably a bounde.
    if (digitalRead(switch_pins[0])) {
      handle_switch1_pressed = false;
      return;
    }
    setSwitchLEDs(LOW);

    if (isMasterPowerEnabled()) {
      publishTinamousStatus("Switch pressed, switching off the fans");
    } else {
      publishTinamousStatus("Switch pressed, switching on the fans");
    }

    // Power the fans on gently
    setFansSpeed(10);
    setPower(!isMasterPowerEnabled());
    delay(2000);
    
    // Set all the fans to 100%
    setFansSpeed(100);
    
    // Erm....
    if (isMasterPowerEnabled()) {
      leds[0] = CRGB::Red; 
      leds[1] = CRGB::Red; 
      leds[2] = CRGB::Red; 
      leds[3] = CRGB::Red; 
    } else {
      leds[0] = CRGB::Green; 
      leds[1] = CRGB::Green; 
      leds[2] = CRGB::Green; 
      leds[3] = CRGB::Green; 
    }

    // wait for Switch1 and two to go high again.
    // to ensure the user has released it 
    while(!digitalRead(switch_pins[0])) { }
    
    handle_switch1_pressed = false;
    setSwitchLEDs(HIGH);
  }

  if (handle_switch2_pressed) {

    Serial.println("Switch 2 Interrupt handling");

    // Ensure we have atleast 200ms delay before checking if the switch is still pressed.
    delay(200);

    // If the switch is now high, ignore it, probably a bounde.
    if (digitalRead(switch_pins[1])) {
      handle_switch1_pressed = false;
      return ;
    }

    setSwitchLEDs(LOW);

    // Do switch 2 stuff here....
    
    while(!digitalRead(switch_pins[1])) { }
    handle_switch2_pressed = false;
    setSwitchLEDs(HIGH);
  }
}

void setSwitchLEDs(bool state) {
  for (int i=0; i<2; i++) {
      digitalWrite(switch_leds[i], state);
  }
}
 
// ==============================================================
// User input
// ==============================================================

int selectedFanId = 1;

void readInput() {

  if (Serial.available()) {
    char instruction = Serial.read();

    switch (instruction) {
      case '0':
        setFansSpeed(0);
        break;
      case '1':
        setFansSpeed(10);
        break;
      case '2':
        setFansSpeed(60);
        break;
      case '3':
        setFansSpeed(100);
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
        ledBrightnessPercent+= 5;
        if (ledBrightnessPercent > 100){
          ledBrightnessPercent = 100;
        }
        break;
      case '<':
        ledBrightnessPercent-= 5;
        if (ledBrightnessPercent <= 0){
          ledBrightnessPercent = 0;
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
  setFansSpeed(0);
  ledBrightnessPercent = 0;
  publishTinamousStatus("Sleep mode activated.");
}

void wakeNow() {
  Serial.println("Wake!"); 

  // TODO: Wake on previous speed or 
  // have a desired wake speed for the fans.
  setFansSpeed(0);
  setPower(1);
  delay(2000);
  setFansSpeed(100);
  
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

  // Ideal would really be 0, however with at least
  // 1 person observing it's likely to be a low but not 
  // zero value so will make the display weird. hence
  // set "idealValue" to about a normal level for 
  // one person.
  float idealValue = 400;
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

void switch1_pressed() {
  handle_switch1_pressed = true;
}

void switch2_pressed() {
  handle_switch2_pressed = true;
}





