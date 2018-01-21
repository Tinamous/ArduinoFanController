#include <SparkFunCCS811.h>
#include "customTypes.h"

#include <FastLED.h>
//#include <WiFi101.h>
//#include <Adafruit_MQTT.h>
//#include <Adafruit_MQTT_Client.h>
//#include <Adafruit_MQTT_FONA.h>



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

// User selected speed to set the fans to.
int pwmSpeed = 255;
int fanMode = 3; // Fan mode. 0=Off, 1=Low, 2=Medium, 3=High 

// State of the master power selection.
bool master_power = false;

// running LED Index, by "Hour" (0 top, 11 at 11 o'clock...)
int redHourIndex = 0;
int lastRedHourIndex = 0;

int ledBrightness = 128;

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

// Visualisation settings
// temperature
// 0 = relative, 1 = absolute.
int temperatureMode = 0;
// relative mode, +/- range of referece
// absolute mode, reference is minimm
int temperatureReference = 21;


// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Switch off the 12V to the fans (TODO: Pull down resistor).
  pinMode (master_power_pin, OUTPUT);
  digitalWrite(master_power_pin, LOW);

  SetupNeopixels();

  //Initialize serial:
  Serial.begin(9600);
  delay(5000);

  setFanBackground(1, CRGB::Yellow);
  setFanBackground(2, CRGB::Yellow);
  setFanBackground(3, CRGB::Yellow);
  setFanBackground(4, CRGB::Yellow);

  setupTemperatureDisplayRange();
  
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

CRGB ledsSetColor = CRGB::Red;
  
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
        break;
      case '-':
        temperature -=0.25;
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
}

// ==========================================================
// Neopixel handling
// ==========================================================

void SetLedsOnOff(bool newState) {
  ledsEnabled = newState;
  
  if (ledsEnabled) {
    Serial.println("All LEDs enabled.");
  } else {
    Serial.println("All LEDs off.");
  }
}

// Loop handler to update the Neopixels (i.e. LED leds + possible others)
void handleNeopixels() {
  updateFansLeds();
  //updateStrip1Leds();
  //updateStrip2Leds();
  endLedUpdate();

  FastLED.show(); 
}

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
// 255: Automatic
// DisplayMode fanModes[] = {DisplayMode::Temperature, DisplayMode::Ignore, DisplayMode::Ignore, DisplayMode::Ignore};
int fanModes[] = {1, 0, 0, 0};

// Converts from a clock 'hour' position (0..12) (0==12 to make range lookup easier) to a led id (0..15, well 4..15)
// This assumes the fan is placed so the wire exits top right....
int fanOuterLedLookup[] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14};

// Convert the scale 0..11 to an "hour" position on the display.
int normalisedHours[] = {7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6};

void updateFansLeds() {
  int maxFan = 1;
  for (int fanId = 1; fanId <=maxFan; fanId++) {
    Serial.print("Setting fan: ");
    Serial.print(fanId);
    Serial.println();
    showOuterValue(fanId);
    showNoseValue(fanId);
  }
}

void updateStrip1Leds(){}
void updateStrip2Leds(){}

void endLedUpdate() {
   lastRedHourIndex = redHourIndex;
   redHourIndex++;

    // run around the outer ring
    // is the "hour" index rather than led id.
    if (redHourIndex >= 12) {
      redHourIndex = 0;
    }

    // Override any settings made to the LEDs to switch them off.
    if (!ledsEnabled) {
      FastLED.setBrightness( 0 );
    } else {
      FastLED.setBrightness( ledBrightness );
    }
}

// --------------------------------------------------------

// Show value on fan's outer LEDs.
// fanId: 1..4
void showOuterValue(int fanId) {
  Serial.print("Setting fan outer. Fan: ");
  Serial.println(fanId);

  int fanMode = fanModes[fanId-1];
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
// 255: Automatic

  switch (fanMode) {
    case 0:
      // No action. Manual control.
      break;
    case 1:
      showTemperature(fanId);
      break;
    case 2: 
      showHumidity(fanId);
      break;
    case 3:
      showPressure(fanId);
      break;
    case 4:
      showAirQuality(fanId);
      break;
    case 5:
      showTime(fanId);
      break;
    case 6: // circling
      showRunningClock(fanId);
      break;
    case 7:
      // TODO: Use all fans
      showRunningClock(fanId);
      break;
    case 8:
      showDust(fanId);
      break;
    case 9:
      showTimer(fanId);
      break;
    case 255:
      showAutomatic(fanId);
      break;
  }
}

// Use the fans "nose" to show a value. Uses all 4 LEDs.
// Doesn't cycle.
void showNoseValue(int fanId) {
  Serial.print("Setting nose color. Fan: ");
  Serial.println(fanId);

  int fanMode = fanModes[fanId-1];

  switch (fanMode) {
    case 0:
      // No action. Manual control.
      break;
    case 1:
      showNoseTemperature(fanId);
      break;
    case 2:
      showNoseHumidity(fanId);
       case 3:
      showPressure(fanId);
      break;
    case 4:
      showAirQuality(fanId);
      break;
    case 5:
      setNoseColor(fanId, CRGB::Green);
      break;
    case 6: // circling
      setNoseColor(fanId, CRGB::Green);
      break;
    case 7:
      setNoseColor(fanId, CRGB::Green);
      break;
    case 8:
      showNoseDust(fanId);
      break;
    case 9:
      showNoseTimer(fanId);
      break;
    case 255:
      showNoseAutomatic(fanId);
      break;
    default: 
      // Blue nose: Not implemented
      setNoseColor(fanId, CRGB::Blue);
      break;
  }
}

// Sensor display range settings.
displayRange_t temperatureRange;
displayRange_t humidityRange;
displayRange_t pressureRange;
displayRange_t airQualityRange;
displayRange_t dustRange;

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

// Desired temperature range display
float idealTemperature = 20;

float idealTemperatureRangeLow = idealTemperature- 1; 
float idealTemperatureRangeHigh = idealTemperature + 1;

// +/- 5 segments on the display (6 hour ignored).
float minTemperature = idealTemperature-2.5; // each segment worth 0.5 C
float maxTemperature = idealTemperature + 2.5;

int temperatureFactor = 10;

// ----------------------------------
// 1: Temperature display
// ----------------------------------

void showTemperature(int fanId) {
  // Map the desired value onto the fan surround.
  // *10 to avoid float usage
  
  mapToFan(fanId, 
    temperature * temperatureRange.factor, 
    temperatureRange);

    /*
    idealTemperature * temperatureFactor,
    (int)(idealTemperatureRangeLow  * temperatureFactor), 
    (int)(idealTemperatureRangeHigh  * temperatureFactor), 
    (int)(minTemperature * temperatureFactor),
    (int)(maxTemperature * temperatureFactor)
    );
    */
}

void showNoseTemperature(int fanId) {
  // if temperature is outside +/- 1 of the ideal temperature 
  // then show the nose error.
  int difference = temperature - idealTemperature;

  Serial.print("Difference: ");
  Serial.print(difference);

  // Cold
  if (temperature < idealTemperatureRangeLow) {
    setNoseColor(fanId, CRGB::Blue);
    return;  
  } 

  // Hot
  if (temperature > idealTemperatureRangeHigh ) {
    setNoseColor(fanId, CRGB::Red);
    return;
  } 
    
  setNoseColor(fanId, CRGB::Green);
}
// ----------------------------------

// ----------------------------------
// 2: Humidity Display
// ----------------------------------

void showHumidity(int fanId) {

}

void showNoseHumidity(int fanId) {

}

// ----------------------------------
// 3: Pressure Display
// ----------------------------------

void showPressure(int fanId) {
  
}

void showNosePressure(int fanId) {
  
}

// ----------------------------------
// 4: Air Quality Display
// ----------------------------------

void showAirQuality(int fanId) {
  
}

void showNoseAirQuality(int fanId) {
  
}

// ----------------------------------
// 5: Time / clock
// ----------------------------------

void showTime(int fanId) {
  //
}

// ----------------------------------
// 6: Circle (single fan)
// ----------------------------------

void showRunningClock(int fanId) {
  setLedByHour(fanId, lastRedHourIndex, CRGB::Blue);
  setLedByHour(fanId, redHourIndex, CRGB::Red);
}

// ----------------------------------
// 7: Circle (all fans)
// ----------------------------------

void showAllFansRunningClock(int fanId) {
  //
}

// ----------------------------------
// 8:Dust Display
// ----------------------------------

void showDust(int fanId) {
  
}

void showNoseDust(int fanId) {

}

// ----------------------------------
// 9: Timer
// ----------------------------------

void showTimer(int fanId) {
  
}

void showNoseTimer(int fanId) {
  
}

// ----------------------------------
// 10: Pomodoro. Work (25min) + Play (5 min)
// ----------------------------------

// ----------------------------------
// 11: Pomodoro Work only (0..25 mins)
// ----------------------------------
// TODO: Link to the play fan.

// ----------------------------------
// 12: Pomodoro Play (0..5 mins)
// ----------------------------------
// TODO: Link to the work fan.

// ----------------------------------
// 255: Automatic
// ----------------------------------
// Automatic - show which ever measurement needs attemption
void showAutomatic(int fanId) {
  
}

void showNoseAutomatic(int fanId) {
  // Color code the nose to indicate which parameter.
}

// ----------------------------------



// Hot
DEFINE_GRADIENT_PALETTE( green_to_red_palette ) {
    0, 0x00, 0xFF, 0x00,
  255, 0xFF, 0x00, 0x00};
  
CRGBPalette16 rgPalette = green_to_red_palette;

// Cold
DEFINE_GRADIENT_PALETTE( green_to_blue_palette ) {
    0,   0, 0,  0,
   95, 255, 255, 0,
  255, 255, 0,0};




