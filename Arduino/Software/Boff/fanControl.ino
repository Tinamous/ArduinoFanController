
// Main 12V fan power rail switch.
int master_power_pin = 13; // RX

// State of the master power selection.
bool master_power = false;

volatile int fan_pulse_count[] = {0,0,0,0,0};

// The time when the fan RPM's were last computed.
// and with that, when the fan pulses were reset.
unsigned long lastFanRpmComputedAtMillis;

void setupFans() {
  // Switch off the 12V to the fans (TODO: Pull down resistor).
  pinMode (master_power_pin, OUTPUT);
  digitalWrite(master_power_pin, LOW);

  fanInfos[0] = setUpFan1(2, 0);
  fanInfos[1] = setUpFan2(3, 1);
  fanInfos[2] = setUpFan3(4, 8);
  fanInfos[3] = setUpFan4(5, 9); // Not listed as interrupt source on tech specs.
  fanInfos[4] = setUpFan5(10, 7);
  
  // Setup the fan PWMs and tach.
  for (int i = 0; i<5; i++) {
    pinMode(fanInfos[i].pwmPin, OUTPUT);
    analogWrite(fanInfos[i].pwmPin,0);
    pinMode(fanInfos[i].tachPin, INPUT_PULLUP);
  }

  // MKR1000 Rev.1: Pins [0], [1], 4, 5, 6, [7], [8], ([9]), A1/16, A2/17 (We're using, 0, 1, 8, 9 and 7 for fan tach)
  // Pin 9 not listed on main MKR1000 page as supporting interrupt
  // but is listed on attachInterrupt for Rev1.
  // and A2 + A3 for switches (A3 can probably be ignored.
  // Zero all digital pins, except 4
  attachInterrupt(fanInfos[0].tachPin, fan_one_pulse, FALLING); 
  attachInterrupt(fanInfos[1].tachPin, fan_two_pulse, FALLING);
  attachInterrupt(fanInfos[2].tachPin, fan_three_pulse, FALLING);
  // Not clear if pin 9 is int source + disconnected as it causes the 
  // arduino to lockup when pulsed by the tach.
  //attachInterrupt(fanInfos[3].tachPin, fan_four_pulse, FALLING);
  attachInterrupt(fanInfos[4].tachPin, fan_five_pulse, FALLING);
}

void fansLoop() {
  updateFanSpeeds();

  // Fan RPM's are computed every n seconds.
  if (millis() - lastFanRpmComputedAtMillis > 10000) {
    updateFanPulseCounts();
  }
}

void updateFanSpeeds() {
  for (int fanId = 0; fanId<4; fanId++) {
    fanInfo_t fanInfo = fanInfos[fanId];
    if (fanInfo.speedSet != fanInfo.currentSpeed) {
      setFan(fanId);
    }
  }
}

void updateFanPulseCounts() {
  // Current RPM computed from pulse counts
  int computedRpm;

  // How long (ms) since we've last computed and hence
  // how long is the pulse count over.
  int timeSinceLastCompute = (millis() - lastFanRpmComputedAtMillis);

  // Start by storing the pulses read from the fan tach interrupts.
  // into the fanInfo struct and resetting the pulse info.
  for (int fanId = 0; fanId < 4; fanId++) {
    
    fanInfos[fanId].pulseCount = fan_pulse_count[fanId];
    fan_pulse_count[fanId] = 0;

    if (fanInfos[fanId].pulseCount > 0) {
      // Ensure the array version is updated, not the local copy.
      fanInfos[fanId].computedRpm = computeFanSpeedRpm(fanId, timeSinceLastCompute);
    } else {
      fanInfos[fanId].computedRpm = 0;
    }
  }

  lastFanRpmComputedAtMillis = millis();
}

int computeFanSpeedRpm(int fanId, int timeSinceLastCompute) {
  fanInfo_t fanInfo = fanInfos[fanId];
  int time_factor = 60000 / timeSinceLastCompute;    
  int total_pulses_per_minute = fanInfo.pulseCount * time_factor;
  return total_pulses_per_minute / fanInfo.pulseToRpmFactor;
}


// Set the fans speed as requested in the speedSet
// property of fanInfo
void setFan(int fanId) {
  fanInfo_t fanInfo = fanInfos[fanId];
  int set_speed = fanInfo.speedSet;
  int current_speed = fanInfo.currentSpeed;
  int pwm_frequency = (int)(set_speed * 2.55);
     
  analogWrite(fanInfo.pwmPin, pwm_frequency);

  // Don't use fanInfo. as it's a copy and 
  // the fanInfos array isn't updated.
  fanInfos[fanId].currentPwm = pwm_frequency;
  fanInfos[fanId].currentSpeed = set_speed; //speed;
}

// Switch the power on/off for the fans.
// PWM fans tend to run even at 0 value
// so master power here shuts down the 12v rail
void setPower(bool state) { 
  digitalWrite(master_power_pin, state);
  master_power = state;

  if (master_power) {
    Serial.println("Fan Power ON");
  } else {
    Serial.println("Fan Power OFF");
  }
}

bool isMasterPowerEnabled() {
  return master_power;
}

bool isFanOn(int fanId) {
  if (!master_power) {
    return false;
  }

  if (!fanInfos[fanId].enabled) {
    return true;
  }

  return false;
}

// Set the speed of the fans
void setFansSpeed(int speed) {
  String message;
  message = "Setting fans to speed ";
  message = message + speed;
  message = message + "%";
  
  publishTinamousStatus(message);
  
  for (int i=0; i<4; i++) {
    setFansSpeed(i, speed);
  }
}

void setFansSpeed(int fanId, int speed) {
  fanInfos[fanId].speedSet = speed;
  
  if (fanInfos[fanId].speedSet > 100) {
    fanInfos[fanId].speedSet = 100;
  }
}

// Delay on the main loop (and hence LED cycle) 
// based on the fan speed. So running dots
// go faster for a faster fan speed.
void fanSpeedDelay() {
int delayFactor;

  if (isMasterPowerEnabled()) {
    delayFactor = 100 - fanInfos[0].currentSpeed;
  } else {
    // fans not on, go very slow...
    delayFactor = 100;
  }

  // 0 to 1s delay in loop for fan speed (100% to 0%)
  delay(delayFactor);
}

// ============================================
// Setup fan parameters
// ============================================

fanInfo_t setUpLL120Fan(int fanId, int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo;
  fanInfo.fanId = fanId;
  fanInfo.pwmPin = pwm_pin;
  fanInfo.tachPin = tach_pin;
  fanInfo.enabled = true;
  fanInfo.pulseCount = 0;
  // Current RPM computed from pulse counts
  fanInfo.computedRpm = 0;
  // Array of RPM's expected indexed by fanModel
  // e.g. [0] = 0, [1] = 400, [2] = 600, [3] = 800, ... [11]
  // Leave as defaults
  //fanInfo.expectedRpm[0] = 0;
  fanInfo.speedSet = 0;
  // 600 - 1500 +/- 10% RPM
  fanInfo.pulseToRpmFactor = 2; // 1, 2, or 4 typically.
  return fanInfo;
}

fanInfo_t setUpFan1(int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo = setUpLL120Fan(1, pwm_pin, tach_pin);
  fanInfo.outerColor = CRGB::Green;
  fanInfo.noseColor = CRGB::Orange;
  return fanInfo;
}

fanInfo_t  setUpFan2(int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo = setUpLL120Fan(2, pwm_pin, tach_pin);
  fanInfo.outerColor = CRGB::Green;
  fanInfo.noseColor = CRGB::Blue;
  return fanInfo;
}

fanInfo_t  setUpFan3(int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo = setUpLL120Fan(3, pwm_pin, tach_pin);
  fanInfo.outerColor = CRGB::Green;
  fanInfo.noseColor = CRGB::Orange;
  return fanInfo;
}

fanInfo_t  setUpFan4(int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo = setUpLL120Fan(4, pwm_pin, tach_pin);
  fanInfo.outerColor = CRGB::Green;
  fanInfo.noseColor = CRGB::Orange;
  return fanInfo;
}

// Expected to be 40mm fan 
fanInfo_t setUpFan5(int pwm_pin, int tach_pin) {
  fanInfo_t fanInfo;
  fanInfo.fanId = 5;
  fanInfo.pwmPin = pwm_pin;
  fanInfo.tachPin = tach_pin;
  fanInfo.enabled = false; // not fitted
  fanInfo.pulseCount = 0;
  // Current RPM computed from pulse counts
  fanInfo.computedRpm = 0;
  // Array of RPM's expected indexed by fanModel
  // e.g. [0] = 0, [1] = 400, [2] = 600, [3] = 800, ... [11]
  // Leave as defaults
  //fanInfo.expectedRpm[0] = 0;
  fanInfo.speedSet = 0;
  fanInfo.pulseToRpmFactor = 1; // 1, 2, or 4 typically.
  fanInfo.outerColor = CRGB::Green;
  fanInfo.noseColor = CRGB::Orange;
  return fanInfo;
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


