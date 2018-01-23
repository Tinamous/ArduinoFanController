#include <SparkFunCCS811.h>

#include <FastLED.h>
//#include <WiFi101.h>
//#include <Adafruit_MQTT.h>
//#include <Adafruit_MQTT_Client.h>
//#include <Adafruit_MQTT_FONA.h>

// 15 puts it on A0 as used on PCB
// 6 - D6 - as used by functional box at present....
int neopixelPins = 15; // proto & +PCB
//int neopixelPins = 6; // box

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

// Switch debounce handling.
volatile bool handle_switch1_pressed = false;
volatile bool handle_switch2_pressed = false;

int redLedIndex = 4;
int lastRedLedIndex = 4;
//#define NUM_LEDS 24
#define NUM_LEDS 16
// ech fan has 16ish...
CRGB leds[NUM_LEDS];

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

CCS811 myCCS811(CCS811_ADDR);

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
  // NB: This is Dust sensor, so if fitted we need to stop using this.
  //pinMode(LED_BUILTIN, OUTPUT); // NB: This is Dust sensor.

  // Switch off the 12V to the fans (TODO: Pull down resistor).
  pinMode (master_power_pin, OUTPUT);
  digitalWrite(master_power_pin, LOW);

  setupFans();
  
  setupSwitches();

  SetupDustSensor();

  SetupNeopixels();

  //Initialize serial:
  Serial.begin(9600);

  delay(5000);

  setupBME280();
  
  setupBME680();
  
  setupCCS811();
  
  Serial.println("Really Dusty Box...");
  Serial.println("");
  printHeader();
}

void setupBME280() {
  // If fitted.
  Serial.println("Setup BME280...");
  Serial.println("BME280 error. ");
}

void setupBME680() {
  // If fitted
  Serial.println("Setup BME680...");
  Serial.println("BME680 error. ");
}


void setupCCS811() {
  Serial.println("Setup CCS811...");
  // If fitted
  bool status = myCCS811.begin();
  if (status > 0) {
    Serial.print("CCS811 error. Code: ");
    Serial.println(status);
    hasCCS811 = false;
    return;
  }
  hasCCS811 = true;
  
  // meaure every 10 seconds
  //myCCS811.setDriveMode(2);

  // meaure every 1 seconds
  myCCS811.setDriveMode(1);

  // Set defaults for now.
  // should be updated once the BME readings are present
  myCCS811.setEnvironmentalData(humidity, temperature);

  ccs811DataUsableAfter = millis() + (20 * 60 * 1000);
  Serial.print("CCS811 Data Usable After: ");
  Serial.print(ccs811DataUsableAfter/1000);
  Serial.println("s");

  // TODO: Set baseline from eeprom
}

void setupFans() {
  // Setup the fan PWMs and tach.
  for (int i = 0; i<5; i++) {
    pinMode(fan_pwm_pins[i], OUTPUT);
    pinMode(fan_tach_pins[i], INPUT_PULLUP);
  }

  // MKR1000 Rev.1: Pins [0], [1], 4, 5, 6, [7], [8], [9], A1, A2 (We're using, 0, 1, 8, 9 and 7 for fan tach)
  // and A2 + A3 for switches (A3 can probably be ignored.
  // Zero all digital pins, except 4
  attachInterrupt(fan_tach_pins[0], fan_one_pulse, FALLING); 
  attachInterrupt(fan_tach_pins[1], fan_two_pulse, FALLING);
  attachInterrupt(fan_tach_pins[2], fan_three_pulse, FALLING);
  attachInterrupt(fan_tach_pins[3], fan_four_pulse, FALLING);
  attachInterrupt(fan_tach_pins[4], fan_five_pulse, FALLING);

}

void setupSwitches() {
  // Setup the switch and switch LED pins
  pinMode(switch_enable, OUTPUT);
  digitalWrite(switch_enable, LOW); 
  
  for (int i=0; i<2; i++) {
      pinMode(switch_leds[i], OUTPUT);
      digitalWrite(switch_leds[i], LOW);
      pinMode(switch_pins[i], INPUT_PULLUP);
  }

  attachInterrupt(switch_pins[0], switch1_pressed, FALLING); 
  attachInterrupt(switch_pins[1], switch2_pressed, FALLING); 
}

void SetupDustSensor() {
  // TODO
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

void setSwitchLEDs(bool state) {
  for (int i=0; i<2; i++) {
      digitalWrite(switch_leds[i], state);
  }
}

// ==============================================================
// Loop functions
// ==============================================================

int loopCounter = 0;
long lastAirMonitor = 0;

// the loop function runs over and over again forever
void loop() {
  loopCounter++;
  //digitalWrite(LED_BUILTIN, HIGH); // D6 used for input for dust sensor when fitted.
  setSwitchLEDs(HIGH);
  delay(100);

  long now = millis();

  if (now  - lastAirMonitor > 2000) {
    checkAirQuality();
  }

  if (loopCounter > 20) {
    loopCounter = 0;
    computeFanSpeed();
    displayStatus();
  }

  handleSwitches();

  readInput();

  handleNeopixels();
  
  //digitalWrite(LED_BUILTIN, LOW);    
  setSwitchLEDs(LOW);
  delay(100);
}

void checkAirQuality() {
  // BME680
  // TODO: Set CCS811 temperature to allow for correction..
  //myCCS811.setEnvironmentalData(humidity, temperature);
  
  // BME280
  // TODO: Set CCS811 temperature to allow for correction..
  //myCCS811.setEnvironmentalData(humidity, temperature);

  // CCS811
  if (myCCS811.dataAvailable())
  {       
    myCCS811.readAlgorithmResults();
    eCO2 = myCCS811.getCO2();
    tVOC = myCCS811.getTVOC();
  } else if (myCCS811.checkForStatusError())  {
    Serial.print("Status error from CCS811: "); 
    uint8_t statusError = myCCS811.getErrorRegister();
    Serial.print(statusError); 
    Serial.println(); 
  } else {
    Serial.println("CCS811 no data"); 
  }

  lastAirMonitor = millis();
}

// Compute the fan speed in RPM
// todo: use /1 /2 or /4 as appropriate
// averaging?
void computeFanSpeed()  {
  for (int i=0; i<5; i++) {
    fan_computed_rpm[i] = fan_pulse_count[i];
    fan_pulse_count[i] = 0;
  }
}

void displayStatus() {
    Serial.print("Fans:\t");
    for (int i=0; i<5; i++) {
      Serial.print(fan_computed_rpm[i], DEC);
      Serial.print("\t");
    }

    Serial.print("CCS811:\t");
    if (hasCCS811) {
      // If were past the time the data is usable show the sensor as OK.
      // otherwise show the time remaing until stable.
      if (millis() > ccs811DataUsableAfter) {
        Serial.print("OK\t");
      } else {
        long remainingTime = (ccs811DataUsableAfter - millis())/1000;
        Serial.print("t-");
        Serial.print(remainingTime);
        Serial.print("s\t");
      }
            
      Serial.print(tVOC);
      Serial.print("\t");
      Serial.print(eCO2);
      Serial.print("\t");
    } else {
      // No CCS811 so show the error.
      Serial.print("Fault\t");// OK or t-
      Serial.print("---\t");  // VOC
      Serial.print("---\t");  // eCO2
    }
    Serial.print(master_power ? "On" : "Off");
    Serial.print("\t");

    Serial.print(fanMode);
    Serial.print("\t");
    
    Serial.print(pwmSpeed);
    Serial.print("\t");
    
    Serial.println();
}

void printHeader() {
  Serial.print("Fans\t");
  Serial.print("1\t");
  Serial.print("2\t");
  Serial.print("3\t");
  Serial.print("4\t");
  Serial.print("5\t");
  Serial.print("CCS\t");
  Serial.print("status\t");
  Serial.print("tVOCs\t");
  Serial.print("eCO2\t");
  Serial.print("Power\t");
  Serial.print("F.Mode\t");
  Serial.print("PWM\t");
  Serial.println();
}

void handleSwitches() {
  if (handle_switch1_pressed) {
    // Toggle the power but set fans 3 and 5 on regardless
    // (they will be off if the power is off).
    setPower(!master_power);
    setFan(1, pwmSpeed);
    setFan(2, pwmSpeed);
    setFan(3, pwmSpeed);
    setFan(4, pwmSpeed);
    // Fan 5 should be under special control.
    setFan(5, pwmSpeed);
    
    //redLedIndex = 4;
    if (master_power) {
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

    // wait for Switch1 to go high again.
    while(!digitalRead(switch_pins[0])) {
      delay(10);
    }
    
    handle_switch1_pressed = false;
  }

  if (handle_switch2_pressed) {

    while(!digitalRead(switch_pins[1])) {
      delay(10);
    }
    handle_switch2_pressed = false;
  }
}

void readInput() {
  if (Serial.available()) {
    char instruction = Serial.read();

    switch (instruction) {
      case '0':
        Serial.println("switching all fans off");
        fanMode = 0;
        setFan(1, 0);
        setFan(2, 0);
        setFan(3, 0);
        setFan(4, 0);
        setFan(5, 0);
        break;
      case '1':
        setFan(1, pwmSpeed);
        break;
      case '2':
        setFan(2, pwmSpeed);
        break;
      case '3':
        setFan(3, pwmSpeed);
        break;
      case '4':
        setFan(4, pwmSpeed);
        break;
      case '5':
        setFan(5, pwmSpeed);
        break;
      case 'l':
        // TODO: Use individual pwm setting for each fan for the power mode
        Serial.println("Low speed selected");
        pwmSpeed = 0;
        fanMode = 1;
        break;
      case 'm':
        Serial.println("Medium speed selected");
        pwmSpeed = 128;
        fanMode = 2;
        break;
      case 'h':
        Serial.println("High speed selected");
        pwmSpeed = 255;
        fanMode = 3;
        break;
      case 'p':
        setPower(HIGH);
        break;
      case 'o':
        setPower(LOW);
        break;
      case 'r': // read baseline
        Serial.println("Read CCS811 Baseline");
        ccsBaseline = myCCS811.getBaseline();
        Serial.print("CCS811 base line: ");
        Serial.print(ccsBaseline, HEX);
        Serial.println("");
        break;
      case 'b':
        // See: https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library/blob/master/examples/BaselineOperator/BaselineOperator.ino
        Serial.println("Setting CCS811 Baseline");
        unsigned int baselineToApply;
        baselineToApply = 0x1485;
        
        CCS811Core::status errorStatus;
        myCCS811.setBaseline( baselineToApply );
        
        if ( errorStatus == CCS811Core::SENSOR_SUCCESS ) {
          Serial.println("Baseline written to CCS811.");
        } else {
          Serial.println("Baseline write error.");
        }
        break;
      default:
        Serial.println("Unknown instruction. Select: 0, 1, 2, 3, 4, 5, l, m, h, p, o, r, b");
        Serial.println("0 - All fans off");
        Serial.println("1..5 - Fan 1..5 on");
        Serial.println("l,m,h - [l]ow, [m]edium, [h]igh speed set for next fan");
        Serial.println("p - 12v Fan [p]ower on (o - off)");
        Serial.println("o - 12v Fan power [o]ff)");
        Serial.println("r - [r]ead ccs baseline");
        Serial.println("b - set ccs [b]aseline (to 0000 at present)");
        break;
    }
  }
}

void setFan(int fanNumber, int speed) {
  Serial.print("switching fan ");
  Serial.print(fanNumber);
  Serial.print(" to speed ");
  Serial.print(speed);
  Serial.println();

  analogWrite(fan_pwm_pins[fanNumber-1], speed);
  
  // Store the speed (so we know if it's off or running and can update)
  // and also report back
  fan_speed_set[fanNumber-1] = speed;
}

void setPower(bool state) {
  digitalWrite(master_power_pin, state);
  master_power = state;

  if (master_power) {
    Serial.println("Power ON");
  } else {
    Serial.println("Power OFF");
  }
}

// ==========================================================
// Neopixel handling
// ==========================================================

// Loop handler to update the Neopixels (i.e. LED leds + possible others)
void handleNeopixels() {

  updateFan1Leds();
  updateFan2Leds();
  updateFan3Leds();
  updateFan4Leds();
  updateStrip1Leds();
  updateStrip2Leds();
  endLedUpdate();

  FastLED.show(); 
}


void updateFan1Leds() {
  // Temperature
  int fanId = 1;
  // Cycle mode....
  redLedIndex++;

  
  showOuterValue(1,  redLedIndex, 1, 12);
  showNoseValue(1);
}

void updateFan2Leds(){
  // Humidty
  showOuterValue(2,  redLedIndex, 1, 12);
  showNoseValue(2);
  }
  
void updateFan3Leds(){
  // Pressure
  showOuterValue(3,  redLedIndex, 1, 12);
  showNoseValue(3);
}

void updateFan4Leds(){
  // VOCs
  showOuterValue(4,  redLedIndex, 1, 12);
  showNoseValue(4);
}

void updateStrip1Leds(){}
void updateStrip2Leds(){}
void endLedUpdate() {
   lastRedLedIndex = redLedIndex;

    // If past the end end of the cycle rese.
    if (redLedIndex >= NUM_LEDS) {
      redLedIndex = 4;
    }
}


// Show value on fan's outer LEDs.
// fanId: 1..4
void showOuterValue(int fanId,  int value, int minValue, int maxValue) {

  // LED 0..3 are for the nose.
  int minLed = (16 * fanId-1) + 4; 
  int maxLed = 16 * fanId;
  
  //int startLed = 
  //int endLed = 
  leds[lastRedLedIndex] = CRGB::Blue; 
  leds[value] = CRGB::Red; 
  
}

// Use the fans "nose" to show a value. Uses all 4 LEDs.
void showNoseValue(int fanId) {
  // Each fan has 16 LEDs.
  int startLed = 0 + ((fanId-1) * 16);
  int endLed = 3 + ((fanId-1) * 16);
  
}

// ==========================================================
// Interrupt handlers
// ==========================================================
void fan_one_pulse() {
  fan_pulse_count[0]++;
}

void fan_two_pulse() {
  fan_pulse_count[1]++;
}

void fan_three_pulse() {
  fan_pulse_count[2]++;
}

void fan_four_pulse() {
  fan_pulse_count[3]++;
}

void fan_five_pulse() {
  fan_pulse_count[4]++;
}

void switch1_pressed() {
  handle_switch1_pressed = true;
}

void switch2_pressed() {
  handle_switch2_pressed = true;
}



