#include <SparkFunCCS811.h>

// When the sensors should next be read
unsigned long nextSensorRead = 0;
int sensorReadIntervalSeconds = 30;

int dust_sensor_pin = 6;

// I2C BME680 (Optional)
// Watch address, it might clash with a BME280 if that is also fitted.
Adafruit_BME680 bme680;

// CCS811 sensor (Optional).
#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address
CCS811 ccs811(CCS811_ADDR);

// TCS34725 Light sensor on the Environment cap.
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

// Voltage (12V across R3 (10k) and R2 (43k) )
// 12V --43k--|--10k-- GND
// R3 / (R3+R2) => 0.188 or *5.3 correction
// 3.3 reference, 12 bit ADC -> 0.80566mV/bit.
int voltage_measure_pin = 6; // A6

void setupSensors() {
  // Set to use 12 bit (0-4095) ADC
  analogReadResolution(12);

  setupBME680();
  setupCCS811();
  setupLightSensor();
}


void setupBME680() {
  // If fitted
  Serial.println("Setup BME680...");
  
  // Pimoroni's 680 is at 0x76
  // Adafruit's to 0x77
  if (bme680.begin(0x76)) {
    hasBme680 = true;
    // Set up oversampling and filter initialization
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme680.setGasHeater(320, 150); // 320*C for 150 ms
    Serial.println("BME680 Initialised");
  } else {
    hasBme680 = false;
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
  }
}

void setupCCS811() {
  Serial.println("Setup CCS811...");
  
  bool status = ccs811.begin();
  if (status > 0) {
    Serial.print("CCS811 error. Code: ");
    Serial.println(status);
    hasCCS811 = false;
    return;
  }
  hasCCS811 = true;
  
  // meaure every 10 seconds
  //ccs811.setDriveMode(2);

  // meaure every 1 seconds
  ccs811.setDriveMode(1);

  // Set defaults for now.
  // should be updated once the BME readings are present
  ccs811.setEnvironmentalData(humidity, temperature);

  ccs811DataUsableAfter = millis() + (20 * 60 * 1000);
  Serial.print("CCS811 Data Usable After: ");
  Serial.print(ccs811DataUsableAfter/1000);
  Serial.println("s");

  // TODO: Set baseline from eeprom
}

void setupLightSensor() {
  Serial.println("Setup TCS34725 light sensor...");
  
  if (tcs.begin()) {
    Serial.println("TCS34725 light sensor found and initalized.");
    hasLightSensor = true;
  } else {
    Serial.println("No TCS34725 found ... check your connections.");
    hasLightSensor = false;
    return;
  }
}

// ===================================================
// Loop to read the sensors.
// ===================================================
void sensorsLoop() {
  if (nextSensorRead < millis()) {
        
    measureFanSupplyVoltage();
    readBME280Data();
    readBME680Data();
    readCCS811Data();
    readLightLevel();
    
    measureRssi();
    
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
  
  if (! bme680.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  temperature = bme680.temperature;
  pressure = bme680.pressure / 100;
  humidity = bme680.humidity;
  gas_resistance  = bme680.gas_resistance / 1000; 
  sensorSource = 2;
}

unsigned long lastAirMonitor = 0;

void readCCS811Data() {

  if (!hasCCS811) {
    return;
  }
  
  // CCS811
  if (ccs811.dataAvailable()) {       
    ccs811.readAlgorithmResults();
    eCO2 = ccs811.getCO2();
    tVOC = ccs811.getTVOC();
  } else if (ccs811.checkForStatusError())  {
    Serial.print("Status error from CCS811: "); 
    ccsLastStatusError = ccs811.getErrorRegister();
    Serial.print(ccsLastStatusError); 
    Serial.println(); 
  } else {
    Serial.println("CCS811 no data"); 
  }

  lastAirMonitor = millis();
}

void readLightLevel() {
  if (!hasLightSensor) {
    return;
  }
 
  tcs.getRawData(&redLightLevel, &greenLightLevel, &blueLightLevel, &clearLightLevel);
  colorTemperature = tcs.calculateColorTemperature(redLightLevel, greenLightLevel, blueLightLevel);
  lightLevelLux = tcs.calculateLux(redLightLevel, greenLightLevel, blueLightLevel); 
}

void readDustSensor() {
  // TODO:...
}

