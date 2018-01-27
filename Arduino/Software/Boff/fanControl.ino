void setupFans() {
  // Setup the fan PWMs and tach.
  for (int i = 0; i<5; i++) {
    pinMode(fan_pwm_pins[i], OUTPUT);
    analogWrite(fan_pwm_pins[i],0);
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

void loopFans() {
  // 
  updateFanSpeeds();
}

void updateFanSpeeds() {
  Serial.println("Update fan speeds");
  for (int selectedFanId = 1; selectedFanId<=4; selectedFanId++) {
      int setSpeed = fanInfos[selectedFanId].speedSet;
      setFan(selectedFanId, setSpeed);
    }
}



void setFan(int fanNumber, int speed) {
  int pwm_frequency = fanInfos[fanNumber].speedPwm[speed];
  
  Serial.print("switching fan ");
  Serial.print(fanNumber);
  Serial.print(" to speed ");
  Serial.print(speed);
  Serial.print(" (pwm frequency ");
  Serial.print(pwm_frequency);
  Serial.print(" on pin ");
  Serial.print(fan_pwm_pins[fanNumber-1]);
  Serial.println(")");
    
  analogWrite(fan_pwm_pins[fanNumber-1], pwm_frequency);
  
  // Store the speed (so we know if it's off or running and can update)
  // and also report back
  fan_speed_set[fanNumber] = speed;
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


