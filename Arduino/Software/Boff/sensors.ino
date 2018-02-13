// When the sensors should next be read
unsigned long nextSensorRead = 0;
int sensorReadIntervalSeconds = 2;

int dust_sensor_pin = 6;

// What sensors are attached.
bool hasBme280 = false;
bool hasBme680 = false;
bool hasCCS811 = false;

// Voltage (12V across R3 (10k) and R2 (43k) )
// 12V --43k--|--10k-- GND
// R3 / (R3+R2) => 0.188 or *5.3 correction
// 3.3 reference, 12 bit ADC -> 0.80566mV/bit.
int voltage_measure_pin = 6; // A6

void setupSensors() {
    // Set to use 12 bit (0-4095) ADC
  analogReadResolution(12);
  
}

void sensorsLoop() {
  if (nextSensorRead < millis()) {
    //Serial.println("Read sensors...");
    
    // meausre other sensors...
    measureRssi();

    measureFanSupplyVoltage();
    readBME280Data();
    readBME680Data();
    readCCS811Data();
    readLightLevel();
    
    
    // refresh every n seconds
    nextSensorRead = millis() + (sensorReadIntervalSeconds * 1000);
  }
}

void measureFanSupplyVoltage() {
  // 1/5th of actual fan supply voltage.
  // should measure 0 when master power is off.
  int adcBits = analogRead(voltage_measure_pin);

  // Analog reference is 3.3V
  // Bits are 4096
  // so... 0.000806v per bit
  // 0.80566mV/bit.
  float measured = (0.80566F * (float)adcBits);
  voltage = (measured * 5.3F) / 1000.0;
}

void readBME280Data() {
  if (!hasBme280) {
    return;
  }
}

// Read Temperature/RH/Pressure/VOC's 
// from the BME680 sensor.
void readBME680Data() {
  if (!hasBme680) {
    return;
  }
  
}

void readCCS811Data() {
  if (!hasCCS811) {
    return;
  }
}

void readLightLevel() {
  // TODO:...
}

void readDustSensor() {
  // TODO:...
}

