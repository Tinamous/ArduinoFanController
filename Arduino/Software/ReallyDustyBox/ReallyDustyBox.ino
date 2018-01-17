#include <FastLED.h>
#include <WiFi101.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
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
int fan_speed_set[] = {0,0,0,0,0};

// User selected speed to set the fans to.
int pwmSpeed = 255;

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

  setupBME280();
  
  setupBME680();
  
  setupCCS811();

  //Initialize serial:
  Serial.begin(9600);

  delay(5000);
  Serial.println("Really Dusty Box...");
}

void setupBME280() {
  // If fitted.
}

void setupBME680() {
  // If fitted
}

void setupCCS811() {
  // If fitted
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
  // TODO
  //strip.begin();
  //strip.show(); // Initialize all pixels to 'off'
// A0

  for (int i=0; i< NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS); 
  Serial.println("Neopixels setup...");
  FastLED.show(); 
  
  
}

void setSwitchLEDs(bool state) {
  for (int i=0; i<2; i++) {
      digitalWrite(switch_leds[i], state);
  }
}

int loopCounter = 0;
// the loop function runs over and over again forever
void loop() {
  loopCounter++;
  //digitalWrite(LED_BUILTIN, HIGH); // D6 used for input for dust sensor when fitted.
  setSwitchLEDs(HIGH);
  delay(100);

  if (loopCounter > 20) {
    loopCounter = 0;
    
    Serial.print("Counts\t");
    for (int i=0; i<5; i++) {
      Serial.print(fan_pulse_count[i], DEC);
      Serial.print("\t");
      fan_pulse_count[i] = 0;
    }
    Serial.println();
  }

  handleSwitches();

  readInput();

  handleNeopixels();
  
  //digitalWrite(LED_BUILTIN, LOW);    
  setSwitchLEDs(LOW);
  delay(100);
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
        Serial.println("Low speed selected");
        pwmSpeed = 0;
        break;
      case 'm':
        Serial.println("Medium speed selected");
        pwmSpeed = 128;
        break;
      case 'h':
        Serial.println("High speed selected");
        pwmSpeed = 255;
        break;
      case 'p':
        setPower(HIGH);
        break;
      case 'o':
        setPower(LOW);
        break;
     default:
        Serial.println("Unknown instruction. Select: 0, 1, 2, 3, 4, 5, l, m, h");
        Serial.println("0 - All fans off");
        Serial.println("1..5 - Fan 1..5 on");
        Serial.println("l,m,h - Low, Medium, High speed set for next fan");
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

  leds[lastRedLedIndex] = CRGB::Blue; 
  leds[redLedIndex] = CRGB::Red; 
  lastRedLedIndex = redLedIndex;
  redLedIndex++;
  if (redLedIndex >= NUM_LEDS) {
    redLedIndex = 4;
  }

  FastLED.show(); 
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



