#include <RTCZero.h>

#include <SparkFunCCS811.h>
#include "customTypes.h"

#include <FastLED.h>

// Pin Mappins
int fan_pwm_pins[] = { 2, 3, 4, 5, 10}; // Was 6, now pin D10 - MISO
int fan_tach_pins[] = { 0, 1, 8, 9, 7 }; // D8 = MOSI, D9 = SCK

int dust_sensor_pin = 6;
int switch_pins[] = {A1, A2};
int switch_leds[] = {A3, A3};
// Only on prototype PCB. Needs to be pulled to GND
int switch_enable = A5;

int master_power_pin = 13; // RX

// Settings.
int fan_pulse_count[] = {0,0,0,0,0};
int fan_computed_rpm[] = {0,0,0,0,0};
int fan_speed_set[] = {0,0,0,0,0};


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
// 255: Automatic
// TODO: Load this from EEPROM or something.
// Let it be settable via MQTT/Alexa/////
DisplayMode fanDisplayModes[] = {DisplayMode::Humidity, DisplayMode::Ignore, DisplayMode::Ignore, DisplayMode::Ignore};

// User selected speed to set the fans to.
int pwmSpeed = 255;
int fanMode = 3; // Fan mode. 0=Off, 1=Low, 2=Medium, 3=High 

// State of the master power selection.
bool master_power = false;

// running LED Index, by "Hour" (0 top, 11 at 11 o'clock...)
int redHourIndex = 0;
int lastRedHourIndex = 0;
CRGB ledsSetColor = CRGB::Red;
int ledBrightness = 200;

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

// Fake values...
bool hasBme280 = false;
bool hasBme680 = false;
bool hasCCS811 = false;

// BME 280 (or 680)
// Guess at appropriate values whilst not available to be read.
float humidity = 40;
float temperature = 22;

// CCS811
long ccs811DataUsableAfter;
unsigned int ccsBaseline;
unsigned int tVOC = 0;
unsigned int eCO2 = 0;

RTCZero rtc;

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Switch off the 12V to the fans (TODO: Pull down resistor).
  pinMode (master_power_pin, OUTPUT);
  digitalWrite(master_power_pin, LOW);

  SetupNeopixels();

  setFanBackground(1, CRGB::Yellow);
  setFanBackground(2, CRGB::Yellow);
  setFanBackground(3, CRGB::Yellow);
  setFanBackground(4, CRGB::Yellow);

  //Initialize serial:
  Serial.begin(9600);
  delay(5000);

  setupTemperatureDisplayRange();
  setupHumidityDisplayRange();

  rtc.begin();
  //rtc.setTime(04, 40, 20);
  //rtc.setDate(21, 01, 2018);
  
  Serial.println("Fan LED Tester...");
  Serial.println("");
}


void SetupNeopixels() {

  for (int i=0; i< NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  
  // 15 puts it on A0.
  // 6 - D6 - as used by protoboard at prsent.
  FastLED.addLeds<NEOPIXEL, 15>(leds, NUM_LEDS); 
  Serial.println("Neopixels setup...");
  FastLED.show(); 
}

// ==============================================================
// Loop functions
// ==============================================================

int loopCounter = 0;
long lastAirMonitor = 0;

// the loop function runs over and over again forever
void loop() {
  loopCounter++;
  digitalWrite(LED_BUILTIN, HIGH); // D6 used for input for dust sensor when fitted.
  delay(50);

  readInput();
  handleNeopixels();
  
  digitalWrite(LED_BUILTIN, LOW);    
  delay(500);
}

// Loop handler to update the Neopixels (i.e. LED leds + possible others)
// code for this is in the displayLeds file.
void handleNeopixels() {
  updateFansLeds();
  //updateStrip1Leds();
  //updateStrip2Leds();
  endLedUpdate();

  FastLED.show(); 
}
  
// ==============================================================
// User input
// ==============================================================
void readInput() {
  int selectedFanId = 1;
 
  if (Serial.available()) {
    char instruction = Serial.read();

    switch (instruction) {
      case '0':
        setLed(selectedFanId, 0, ledsSetColor);
        break;
      case '1':
        setLed(selectedFanId, 1, ledsSetColor);
        break;
      case '2':
        setLed(selectedFanId, 2, ledsSetColor);
        break;
      case '3':
        setLed(selectedFanId, 3, ledsSetColor);
        break;
      case '4':
        setLed(selectedFanId, 4, ledsSetColor);
        break;
      case '5':
        setLed(selectedFanId, 5, ledsSetColor);
        break;
      case '6':
        setLed(selectedFanId, 6, ledsSetColor);
        break;
      case '7':
        setLed(selectedFanId, 7, ledsSetColor);
        break;
      case '8':
        setLed(selectedFanId, 8, ledsSetColor);
        break;
      case '9':
        setLed(selectedFanId, 9, ledsSetColor);
        break;
      case 'a':
        setLed(selectedFanId, 10, ledsSetColor);
        break;
      case 'b':
        setLed(selectedFanId, 11, ledsSetColor);
        break;
      case 'c':
        setLed(selectedFanId, 12, ledsSetColor);
        break;
      case 'd':
        setLed(selectedFanId, 13, ledsSetColor);
        break;
      case 'e':
        setLed(selectedFanId, 14, ledsSetColor);
        break;
      case 'f':
        setLed(selectedFanId, 15, ledsSetColor); // Should be the last LED on the first fan (16 LEDs, 0..15)
        break;
      case 'g': // First LED on next fan... 
        setLed(selectedFanId, 16, ledsSetColor);
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
        break;
      case '-':
        temperature -=0.25;
        humidity -=2;
        break;
      default:
        Serial.println("Unknown instruction. Select: 0..F, t, h, p, q");
        Serial.println("0..f - HEX Led Index");
        Serial.println("t - Select [t]emperature fan");
        Serial.println("h - Select [h]umidity fan");
        Serial.println("p - Select [p]ressure fan");
        Serial.println("q - Select air [q]uality fan");
        Serial.println("o - all LEDs [o]ff)");
        break;
    }
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(", Humidity: ");
  Serial.print(humidity);
  Serial.println();
}


// ==========================================================
// Range setup
// ==========================================================

// Sensor display range settings.
displayRange_t temperatureRange;
displayRange_t humidityRange;
displayRange_t pressureRange;
displayRange_t airQualityRange;
displayRange_t dustRange;

// Setup parameters for temperature display
void setupTemperatureDisplayRange() {
  float idealValue = 20;
  int factor = 10;
  
  temperatureRange.idealValue = idealValue * factor;
  
  temperatureRange.idealRangeLow = (idealValue - 1) * factor; 
  temperatureRange.idealRangeHigh = (idealValue + 1) * factor; 

  // +/- 6 segments on the display
  temperatureRange.minValue = (idealValue - 2.5) * factor;  // each segment worth 0.5 C
  temperatureRange.maxValue = (idealValue + 2.5) * factor; 

  temperatureRange.factor = factor;
}

// Setup parameters for temperature display
void setupHumidityDisplayRange() {
  float idealValue = 55;
  int factor = 1;
  
  humidityRange.idealValue = idealValue;
  
  humidityRange.idealRangeLow = 50; 
  humidityRange.idealRangeHigh = 60; 

  // +/- 6 segments on the display
  humidityRange.minValue = 30;
  humidityRange.maxValue = 70; 

  humidityRange.factor = factor;
}


