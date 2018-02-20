#include "customTypes.h"
#include <FastLED.h>
#include <WiFi101.h>
#include <Math.h>


// Converts from a clock 'hour' position (0..12) (0==12 to make range lookup easier) to a led id (0..15, well 4..15)
// This assumes the fan is placed so the wire exits top right....
int fanOuterLedLookup[] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14};

// Convert the scale 0..11 to an "hour" position on the display.
int normalisedHours[] = {7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6};

// Convert the scale 0..22 to an "hour" position on the display.
// this allows the full face to be used (1... 12[0] for low then 12[0].. 11 for high.
int normalisedHoursFullScale[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};


// ==========================================================
// Neopixel handling
// ==========================================================

void setupNeopixels() {
  FastLED.clear();

  CRGB ledsSetColor = CRGB::Cyan;
  //CRGB ledsSetColor = CHSV(255, 1.0, 20);
  //ledsSetColor.r=50;
  //ledsSetColor.g=50;
  //ledsSetColor.b=50;
  
  // Default all the LEDs to black on start
  // then setup the fans and strips as needed.
  for (int i=0; i< NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Make the noses off for all the fans
  // as it doesn't show up well on camera.
  for (int fanId=0; fanId<4; fanId++) {
    setNoseColor(fanId, CRGB::Black);
    setFanBackground(fanId, CRGB::Blue);
  }
  
  // 15 puts it on A0.
  // 6 - D6 - as used by protoboard at prsent.
  FastLED.addLeds<NEOPIXEL, 15>(leds, NUM_LEDS); 
  FastLED.setBrightness(ledBrightnessPercent * 2.55);
  Serial.println("Neopixels setup...");
  FastLED.show(); 
}

void showSetupStageComplete(int stage) {
    // Set the noses to show startup...
    setNoseColor(stage-1, CRGB::Green);
    setFanBackground(stage-1, CRGB::Blue);
    FastLED.show(); 
    delay(500);
}

// ==================================================


// Loop handler to update the Neopixels (i.e. LED leds + possible others)
// code for this is in the displayLeds file.
void ledsLoop() {
  updateFansLeds();
  updateStrip1Leds();
  updateStrip2Leds();
  endLedUpdate();

  FastLED.show(); 
}

// ==================================================

// Make the fan LEDs red to indicate they are starting.
void makeFanLedsRed() {
  int maxFan = 4;
  for (int fanId = 0; fanId < maxFan; fanId++) {
    setNoseColor(fanId, CRGB::Red);
    setFanBackground(fanId, CRGB::Red);
  }
  FastLED.setBrightness(ledBrightnessPercent * 2.55);
  FastLED.show();
}

// =================================================

void updateFansLeds() {
  int maxFan = 4;
  for (int fanId = 0; fanId < maxFan; fanId++) {
    showOuterValue(fanId);
    showNoseValue(fanId);
  }
}

// 64 LEDs in the 4 fans.
// Strip 1 is up facing at the back (if fitted).
int strip1StartLedNumber = 65;
int strip1EndLedNumber = strip1StartLedNumber + 61;
int strip1LedCount = 61;

// Strip 1 is front facing (if fitted).
int strip2StartLedNumber = strip1EndLedNumber + 1; // 127
int strip2EndLedNumber = strip2StartLedNumber + 162; // 288
int strip2LedCount = 162;

int strip1Direction = 1;
int strip1RunningDotPosition = strip1StartLedNumber;
int lastStrip1DotPosition = strip1StartLedNumber;

int strip2RunningDotPosition = 0;
int lastStrip2DotPosition = 0;

int showAlexaInteraction = 0; // 0: off, 1: 

void showAlexaConnectionActive() {

  // If the LEDs are off (i.e. sleep more)
  // switch them on to show the activity.
  if (ledBrightnessPercent < 2) {
    ledBrightnessPercent = 20;
  }

  showAlexaInteraction = 1;
  for (int i = 0; i<4; i++) {
    ledsLoop();
    delay(300);
  }
  
  // Ensure the strip is cleared
  setStrip2Color(CRGB::Black);
  showAlexaInteraction = 0;
  ledsLoop();
}

// Update the 1st LED strip.
// This may not be fitted.
void updateStrip1Leds(){
  // Check see if 
  if (NUM_LEDS < strip1StartLedNumber) {
    return;
  }

  // otherwise do our normal stuff.
  //showStrip1RunningDisplay();
  showStrip1FixedColor();
}

void updateStrip2Leds(){
  // Check see if 
  if (NUM_LEDS < strip2StartLedNumber) {
    return;
  }

  CRGB color1 = CRGB::Blue;
  CRGB color2 = CRGB::Cyan;

  if (showAlexaInteraction == 1) {
    showAlexaInteraction = 2;
    showStrip2AlexaInteraction(color1, color2);
    return;
  }

  if (showAlexaInteraction == 2) {
    showAlexaInteraction = 1;
    showStrip2AlexaInteraction(color2, color1);
    return;
  }

  // otherwise do our normal stuff.
  showStrip2FixedColor();
  
  // Only show the running display if the fans are on.
  if (isMasterPowerEnabled()) {
    showStrip2RunningDisplay();
  }
}

void showStrip1FixedColor() {
  setStrip1Color(ledsSetColor);
}

void showStrip2FixedColor() {
  setStrip2Color(ledsSetColor);
}

void showStrip1RunningDisplay() {
  strip1RunningDotPosition = strip1RunningDotPosition + strip1Direction;
  
  if (strip1RunningDotPosition > strip1LedCount) {
    strip1RunningDotPosition = strip1LedCount;
    strip1Direction = -1;
  }
  
  if (strip1RunningDotPosition < 0) {
    strip1RunningDotPosition = 0;
    strip1Direction = +1;
  }

  setStrip1Led(lastStrip1DotPosition, CRGB::Black);
  setStrip1Led(strip1RunningDotPosition, CRGB::Blue);
  setStrip1Led(strip1RunningDotPosition+ strip1Direction, CRGB::Green);
  setStrip1Led(strip1RunningDotPosition+ (strip1Direction*2), CRGB::Red);
  
  lastStrip1DotPosition = strip1RunningDotPosition;
}

void showStrip2RunningDisplay() {
  
  strip2RunningDotPosition++;
  if (strip2RunningDotPosition > strip2LedCount) {
    strip2RunningDotPosition = 0;
  }

  setStrip2Led(lastStrip2DotPosition, CRGB::Black);
  lastStrip2DotPosition = strip2RunningDotPosition;

  for (int offset = 0; offset < strip2LedCount; offset+=20) {
    setStrip2Led(strip2RunningDotPosition + offset, CRGB::Cyan);
    setStrip2Led(strip2RunningDotPosition + offset + 1, CRGB::Blue);
    setStrip2Led(strip2RunningDotPosition + offset + 2, CRGB::Green);
    setStrip2Led(strip2RunningDotPosition + offset + 3, CRGB::Green);
    setStrip2Led(strip2RunningDotPosition + offset + 4, CRGB::Blue);
    setStrip2Led(strip2RunningDotPosition + offset + 5, CRGB::Cyan);
  }
}

void setStrip1Color(CRGB color) {
  for (int index=0; index < strip1LedCount; index++) {
     setStrip1Led(index, color);
  }
}

void setStrip2Color(CRGB color) {
  for (int index=0; index < strip2LedCount; index++) {
     setStrip2Led(index, color);
  }
}

void showStrip2AlexaInteraction(CRGB color1, CRGB color2) {
  for (int index = 0; index < strip2LedCount; index+=6) {
    setStrip2Led(index, color1);
    setStrip2Led(index+1, color1);
    setStrip2Led(index+2, color1);
    setStrip2Led(index+3, color1);

    setStrip2Led(index+4, color2);
    setStrip2Led(index+5, color2);
    setStrip2Led(index+6, color2);
  }
}

void setStrip1Led(int index, CRGB color) {
int ledIndex;

  ledIndex = strip1StartLedNumber + index;
  if (ledIndex > strip1EndLedNumber) {
    // Ignore off the end
    return;
  }

  leds[ledIndex] = color;
}

void setStrip2Led(int index, CRGB color) {
int ledIndex;

  ledIndex = strip2StartLedNumber + index;
  if (ledIndex > strip2EndLedNumber) {
    ledIndex = ledIndex - strip2LedCount;
  }

  leds[ledIndex] = color;
}

// This is the final say in LED updates, it
// can override all other changes. e.g. to turn off the LEDs
// or to set the noses as red when the fans are starting up, or
// something else...
void endLedUpdate() {
   lastRedHourIndex = redHourIndex;
   redHourIndex++;

    // run around the outer ring
    // is the "hour" index rather than led id.
    if (redHourIndex >= 12) {
      redHourIndex = 0;
    }

    // Check each of the fan speeds and show a red
    // nose if the speed is low.
    // Enable some point in the future when fans
    // are runnable.
    for (int fanId = 0; fanId <4; fanId++) {
      showFanSpeedLowError(fanId);
    }

    // Override any settings made to the LEDs to switch them off.
    if (ledBrightnessPercent < 2) {
      FastLED.setBrightness( 0 );
    } else {
      FastLED.setBrightness( ledBrightnessPercent * 2.55 );
    }
}

// --------------------------------------------------------

// Show value on fan's outer LEDs.
// fanId: 0..3
void showOuterValue(int fanId) {
  DisplayMode fanMode = fanDisplayModes[fanId];

  //Serial.print("Fan: ");
  //Serial.print(fanId);

  switch (fanMode) {
    case DisplayMode::Ignore:
      // No action. Manual control.
      break;
    case DisplayMode::Temperature:
      showTemperature(fanId);
      break;
    case DisplayMode::Humidity:
      showHumidity(fanId);
      break;
    case DisplayMode::Pressure:
      showPressure(fanId);
      break;
    case DisplayMode::AirQuality:
      showAirQuality(fanId);
      break;
    case DisplayMode::Clock:
      showTime(fanId);
      break;
    case DisplayMode::CircleSingleFan:
      showRunningClock(fanId);
      break;
    case DisplayMode::CircleAllFans:
      // TODO: Use all fans
      showRunningClock(fanId);
      break;
    case DisplayMode::Dust:
      showDust(fanId);
      break;
    case DisplayMode::Timer:
      showTimer(fanId);
      break;
    case DisplayMode::Pomodoro:
      // TODO...
      break;
    case DisplayMode::PomodoroWorkOnly:
    // TODO...
      break;
    case DisplayMode::PomodoroPlayOnly:
    // TODO...
      break;
    case DisplayMode::FixedColor:
      showFixedColor(fanId);
      break;
    case DisplayMode::SelectedFanSpeed:
      showFanSelectedSpeed(fanId);
      break;
    case DisplayMode::FanSpeed:
      showFanRpmSpeed(fanId);
      break;
    case DisplayMode::Fancy:
      showFancy(fanId);
      break;
    case DisplayMode::WiFiStrength:
      showWiFiStrength(fanId);
      break;
    case DisplayMode::OnOff:
      showOnOff(fanId);
      break;
    case DisplayMode::MqttFeed:
      showMqttFeed(fanId);
      break;
    case DisplayMode::Automatic:
      showAutomatic(fanId);
      break;
    default: 
      // Blue nose: Not implemented
      Serial.println("Unknown display mode");
      break;
  }
}

// Use the fans "nose" to show a value. Uses all 4 LEDs.
// Doesn't cycle.
void showNoseValue(int fanId) {
  DisplayMode fanMode = fanDisplayModes[fanId];

  switch (fanMode) {
    case DisplayMode::Ignore:
      // No action. Manual control.
      break;
    case DisplayMode::Temperature:
      showNoseTemperature(fanId);
      break;
    case DisplayMode::Humidity:
      showNoseHumidity(fanId);
      break;
     case DisplayMode::Pressure:
      showNosePressure(fanId);
      break;
    case DisplayMode::AirQuality:
      showNoseAirQuality(fanId);
      break;
    case DisplayMode::Clock:
      setNoseColor(fanId, CRGB::Black);
      break;
    case DisplayMode::CircleSingleFan:
      setNoseColor(fanId, CRGB::Green);
      break;
    case DisplayMode::CircleAllFans:
      setNoseColor(fanId, CRGB::Green);
      break;
    case DisplayMode::Dust:
      showNoseDust(fanId);
      break;
    case DisplayMode::Timer:
      showNoseTimer(fanId);
      break;
    case DisplayMode::Pomodoro:
      // TODO...
      break;
    case DisplayMode::PomodoroWorkOnly:
      // TODO...
      break;
    case DisplayMode::PomodoroPlayOnly:
      // TODO...
      break;
    case DisplayMode::FixedColor:
      showNoseFixedColor(fanId);
      break;
    case DisplayMode::SelectedFanSpeed:
      // red/green for in range.
      showNoseFanSpeed(fanId); 
      break;
    case DisplayMode::FanSpeed:
      showNoseFanSpeed(fanId);
      break;
    case DisplayMode::Fancy:
      showNoseFancy(fanId);
      break;
    case DisplayMode::WiFiStrength:
      showNoseWiFiStrength(fanId);
      break;
    case DisplayMode::OnOff:
      showNoseOnOff(fanId);
      break;
    case DisplayMode::MqttFeed:
      showNoseMqttFeed(fanId);
      break;
    case DisplayMode::Automatic:
      showNoseAutomatic(fanId);
      break;
    default: 
      // Blue nose: Not implemented
      setNoseColor(fanId, CRGB::Blue);
      break;
  }
}

// ----------------------------------
// 1: Temperature display
// ----------------------------------

void showTemperature(int fanId) { 
  // Map the desired value onto the fan surround.
  // *10 to avoid float usage
  int temperatureFactorised = temperature * temperatureRange.factor;

  //Serial.print("Temperature: ");
  //Serial.println(temperature);
  
  mapToFan(fanId, temperatureFactorised, temperatureRange);
}

void showNoseTemperature(int fanId) {
  int temperatureFactorised = temperature * temperatureRange.factor;

  setGenericNose(fanId, temperatureFactorised, temperatureRange);
}
// ----------------------------------

// ----------------------------------
// 2: Humidity Display
// ----------------------------------

void showHumidity(int fanId) {
  mapToFan(fanId, (int)humidity, humidityRange);
}

void showNoseHumidity(int fanId) {
  setGenericNose(fanId, (int)humidity, humidityRange);
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
  if (hasBme680) {
    // TODO: different range
    mapToFan(fanId, gas_resistance, airQualityRange);
  } else if (hasCCS811) {
    mapToFan(fanId, eCO2, airQualityRange);
  } else {
    // No sensor.
    setNoseColor(fanId, CRGB::Black);
    setFanBackground(fanId, CRGB::Black);
  }
}

void showNoseAirQuality(int fanId) {
  if (hasBme680) {
    // TODO: different range
    setGenericNose(fanId, gas_resistance, airQualityRange); 
  } else if (hasCCS811) {
    setGenericNose(fanId, eCO2, airQualityRange);
  } else {
    // No sensor.
    setNoseColor(fanId, CRGB::Black);
    setFanBackground(fanId, CRGB::Black);
  }  
}

// ----------------------------------
// 5: Time / clock
// ----------------------------------

void showTime(int fanId) {

  int currentHour = rtc.getHours();
  int currentMinute = rtc.getMinutes(); // 0-59

  int hour = currentHour;
  float factor = (12 / 60);
  int minuteAsHour =  (currentMinute * 12)/60;
  
  Serial.print("Time: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print("  Minute as Led Hour: ");
  Serial.print(minuteAsHour);
  Serial.println();

  CRGB color;
  color.red = 0xEE;
  color.green = 0xE8;
  color.blue =  0x20;
  
  //setFanBackground(fanId, color);
  setFanBackground(fanId, CRGB::Black);

  // TODO: Handle hour and minuteAsHour when on the same LED.
  if (hour == minuteAsHour) {
    setLedByHour(fanId, hour, CRGB::Red);
  } else {
    setLedByHour(fanId, hour, CRGB::Green);
    setLedByHour(fanId, minuteAsHour, CRGB::Blue);
  } 
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
// 13: Fixed Color
// ----------------------------------
void showFixedColor(int fanId) {
  CRGB color = fanInfos[fanId].outerColor;
  setFanBackground(fanId, color);
}

void showNoseFixedColor(int fanId) {
  CRGB color = fanInfos[fanId].noseColor; 
  setNoseColor(fanId, color);
}

// ----------------------------------
// 14: Fan Selected Speed
// ----------------------------------
void showFanSelectedSpeed(int fanId) {
  // 0..11 - directly maps to the hour.
  int speed = fanInfos[fanId].speedSet * (11 / 100);

  setFanBackground(fanId, CRGB::Blue);

  for (int i = 0; i<=speed; i++) {
    setLedByHour(fanId, i, CRGB::Green);
  }
}

// ----------------------------------
// 15: Fan Speed
// ----------------------------------
void showFanRpmSpeed(int fanId) {
  displayRange_t displayRange = setupFanDisplayRange(fanId);
  int rpm = fanInfos[fanId].computedRpm;
  mapToFan(fanId, rpm, displayRange);
}

void showNoseFanSpeed(int fanId) {
  // Default to green...
  setNoseColor(fanId, CRGB::Green);
  
  // but show an error if the speed is low.
  showFanSpeedLowError(fanId);
}

// If the fan has a speed error light up the nose
// a bright red color, otherwise leave it as is.
void showFanSpeedLowError(int fanId) {
 
  // If the power is off then ignore any fan speed setting.
  if (!isFanOn(fanId)) {
    return;
  }

  // If fan is stopped then ignore.
  int selectedSpeed = fanInfos[fanId].speedSet;
  if (selectedSpeed == 0) {
    return;
  }

  if (!fanInfos[fanId].enabled) {
    return;
  }

  // Needs updating for speed as %
  /*
  int rpm = fanInfos[fanId].computedRpm;
  // Expect the rpm to be at-least that of the speed below the
  // currentl selected one.  
  int minRpm = fanInfos[fanId].expectedRpm[selectedSpeed-1];

  if (rpm < minRpm) {
    Serial.print("Fan ");
    Serial.print(fanId + 1);
    Serial.print(" RPM below range. Making a red nose.");
    Serial.println();
    setNoseColor(fanId, CRGB::Red);
  } 
  */
}

displayRange_t setupFanDisplayRange(int fanId) {

  fanInfo_t fanInfo = fanInfos[fanId];

  // Ideal value depends on the fan PWM.
  // Min/Max depend on the fan...
  displayRange_t range;
  range.idealValue = 1200; //fanInfo.expectedRpm[fanInfo.speedSet]; // lookup fan/ fan run mode. HACK!
  
  if (fanInfo.speedSet > 0) {
    range.idealRangeLow = 500; //fanInfo.expectedRpm[fanInfo.speedSet-1]; 
  } else {
    range.idealRangeLow = 0;
  }

  // 11 is the max fan speed setting (0..100).
  if (fanInfo.speedSet < 100) {
    range.idealRangeHigh = 1200; //fanInfo.expectedRpm[fanInfo.speedSet+1]; 
  } else {
    range.idealRangeHigh = 1200; //fanInfo.expectedRpm[fanInfo.speedSet] + 50;
  }

  // Hack for the -ve value to balance the display.
  // Assume fan doesn't go above 2000 RPM.
  range.minValue = -(2* fanInfo.expectedRpm[3]);
  range.maxValue = 2* fanInfo.expectedRpm[3];  

  range.factor = 1;
  return range;
}

int fancy_k = 0;
int fancy_j = 0;
int dim=2;
int8_t gHue = 0; // rotating "base color" used by many of the patterns
static uint8_t hue = 0;


// ----------------------------------
// 17: Show the WiFi signal Strength
// ----------------------------------
void showWiFiStrength(int fanId) {
 
  // Assume -40 and above for rssi is good...
  if (rssi > -40) {
    setFanBackground(fanId, CRGB::Green);
  } else {
  
    // hack with +120 se we use only the -ve (red) area.
    // Use 0,11, 10...1 hour positions to indicate
    // 0 to -120 RSSI values.
    // With Green for 0 to -50, 
    // Blue for -50 to -80
    // and red below -80.
    int hour = getRangeHour(rssi, -120, 120);
  
    // Set all the LEDs to the background color
    // then set just the ones appropriate.
    setFanBackground(fanId, CRGB::Yellow);
  
    CRGB color;
    
    if (rssi < -80) {
      color = CRGB::Red;
    } else if (rssi<-50) {
      color = CRGB::Blue;
    } else {
      color = CRGB::Green;
    }
    setLedHourRange(fanId, rssi, wifiDisplayRange, hour, color);
  }
}

// Use the nose to show connectivity.
// Unless connected with a bad signal strength
void showNoseWiFiStrength(int fanId) {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      if (rssi > -30) {
        setNoseColor(fanId, CRGB::Green);
      } else if (rssi > -60) {
        setNoseColor(fanId, CRGB::Yellow);
      } else {
        setNoseColor(fanId, CRGB::Red);
      }
      break;
    case WL_NO_SHIELD:
    case WL_IDLE_STATUS:
    case WL_NO_SSID_AVAIL:
      setNoseColor(fanId, CRGB::Blue);
      break;
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED:
      setNoseColor(fanId, CRGB::Red);
      break;
    default:
      // Unknown state.
      setNoseColor(fanId, CRGB::Blue);
      break;
  }
}

// ----------------------------------
// 19: On/Off indicator (either red/green, or off/green or white).
// ----------------------------------
void showOnOff(int fanId) {
  if (mqttFeedsValue[fanId] > 0) {
    setFanBackground(fanId, CRGB::Green);
  } else {
    setFanBackground(fanId, CRGB::Red);
  }
}

void showNoseOnOff(int fanId) {
  if (mqttFeedsValue[fanId] > 0) {
    setNoseColor(fanId, CRGB::Green);
  } else {
    setNoseColor(fanId, CRGB::Red);
  }
}

// ----------------------------------
void showMqttFeed(int fanId) {
  // +/- 11 already
  int value = mqttFeedsValue[fanId];
}

void showNoseMqttFeed(int fanId) {
  // +/- 11 already
  int value = mqttFeedsValue[fanId];
}


// ----------------------------------
// 100: Fancy LEDs
// ----------------------------------
void showFancy(int fanId) {

   fancy_j++;
  if (fancy_j > NUM_LEDS) {
    //Serial.println("fancy_j reset");
    fancy_j = 0;
  }
    
   // CRGB ledColor = wheel(fancy_j, 2);   
    //setFanBackground(fanId, ledColor);

    // This looks fancy...
    // works way thought the LEDs each look
    /*
    // Set the i'th led to red 
    leds[fancy_j] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    */

    // Sets fan to same color and works way through.
    setFanBackground(fanId, CHSV(hue++, 255, 255));
    setNoseColor(fanId, CHSV(hue++, 255, 128));
    delay(10);
    //leds[fancy_j] = CHSV(hue++, 255, 255);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

// https://github.com/FastLED/FastLED/blob/master/examples/DemoReel100/DemoReel100.ino
void fancyBeats() {
 
   // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void showNoseFancy(int fanId) {
  // Color code the nose to indicate which parameter.
  //CRGB ledColor = wheel(fancy_j , dim);   
  //setNoseColor(fanId, ledColor);
}

CRGB wheel(int WheelPos, int dim) {
  
  
  CRGB color;
  if (85 > WheelPos) {
   color.r=0;
   color.g=WheelPos * 3/dim;
   color.b=(255 - WheelPos * 3)/dim;;
  } 
  else if (170 > WheelPos) {
   color.r=WheelPos * 3/dim;
   color.g=(255 - WheelPos * 3)/dim;
   color.b=0;
  }
  else {
   color.r=(255 - WheelPos * 3)/dim;
   color.g=0;
   color.b=WheelPos * 3/dim;
  }

  Serial.print("Wheel: ");
  Serial.print(WheelPos);
  Serial.print(", R: ");
  Serial.print(color.r);
  Serial.print(", G: ");
  Serial.print(color.g);
  Serial.print(", B: ");
  Serial.print(color.b);
  Serial.println();
  return color;
}

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

// Map the desired value onto the fan surround.
// where desired value == 12 o'clock
// Min = 7 o'clock
// Max = 6 (or 5) o'clock
// int desiredValue, int idealRangeMin, int idealRangeMax, int minValue, int maxValue)
void mapToFan(int fanId, int value, displayRange_t displayRange) {
  // The hour on the clock the value represents
  int hour = getRangeHour(value, displayRange.minValue, displayRange.maxValue);
  /*
  Serial.print("Hour:");
  Serial.print(hour);
  Serial.print(", Max:");
  Serial.print(displayRange.maxValue);
  Serial.print(", Min:");
  Serial.print(displayRange.minValue);

  Serial.println();
  */

  // Set all the LEDs to the background color
  // then set just the ones appropriate.
  CRGB backgroundColor = getBackgroundColor(fanId, value, displayRange);
  setFanBackground(fanId, backgroundColor);

  CRGB color = getRangeColor(value, displayRange);
  setLedHourRange(fanId, value, displayRange, hour, color);
}

// Get the value as an hour on the clock face.
int getRangeHour(int value, int minValue, int maxValue) {

  // Assumes full scale on the clock (0, 1, 2..11 and 0, 11, 10..1
  if (value > maxValue) {
    //Serial.println("Value above max hour display range.");
    return 11;  
  } else if (value < minValue) {
    //Serial.println("Value below max hour display range.");
    return 1;
  } else {
    // Convert it to a scale of 0..11 for hour lookup
    //int hourIndex = map(value, minValue, maxValue, 0, 11);

    // Convert from a 0..11 value from the map
    // into am hour value (12 O'Clock +/- 6 hours)
    //return normalisedHours[hourIndex];

    // map doesn't do floats nicely. e.g. 15.95 -> 15.
    int hourIndex = map(value* 100, minValue * 100, maxValue * 100, 0*100, 22*100);
    hourIndex = round(hourIndex  / 100.0);
    return normalisedHoursFullScale[hourIndex];
  }
}

// Set the LEDs based on the hour including those from the 12 O'Clock position around to the value
void setLedHourRange(int fanId, int value, displayRange_t displayRange, int hour, CRGB color) {
  
  if (value > displayRange.idealValue) {
    // from 0 to hour - right hand of the clock
    for (int i = 0; i<=hour; i++) {
      setLedByHour(fanId, i, color);
    }
  } else if (value < displayRange.idealValue) {
    // from hour to 12 - left hand side of the clock
    for (int i = hour; i<=12; i++) {
      setLedByHour(fanId, i, color);
    }
  } else {
    // top digit
    setLedByHour(fanId, 0, color);
    // As it's bang on show the whole outline as the desired color.
    setFanBackground(fanId, color);
  }

  // A range of values may use the top digit, 
  // light the whole circle for any value that represents
  // the single top digit.
  if (hour == 0 || hour == 12) {
    // top hour is lit, so make the whole thing good :-)
    setLedByHour(fanId, 0, color);
    // As it's bang on show the whole outline as the desired color.
    setFanBackground(fanId, color);
  }
}

// --------------------------------

// General routine for setting the nose color.
// blue = below ideal range, red = above ideal range, green = ok.
void setGenericNose(int fanId, int value, displayRange_t displayRange) {
  // Low
  if (value < displayRange.idealRangeLow) {
    setNoseColor(fanId, CRGB::Blue);
    return;  
  } 

  // High
  if (value > displayRange.idealRangeHigh ) {
    setNoseColor(fanId, CRGB::Red);
    return;
  } 
    
  setNoseColor(fanId, CRGB::Green);
}

// Set the color of the Nose (motor) of the fan
void setNoseColor(int fanId, CRGB noseColor) {
  // Each fan has 16 LEDs. 0-3 are for the nose.
  int startLed = 0;
  int endLed = 3;

  for (int i = startLed; i<=endLed; i++) {
    setLed(fanId, i, noseColor);
  }  
}

// Set the fan background (outer ring) to a certain color.
void setFanBackground(int fanId, CRGB color) { 
  // Each fan has 16 LEDs. 0-3 are on the nose.
  int startLed = 4;
  int endLed = 15;

  for (int i = startLed; i<=endLed; i++) {
    setLed(fanId, i, color);
  }
}

// Use the "hour" clock face value to se the LED.
void setLedByHour(int fanId, int hour, CRGB color) {
  setLed(fanId, fanOuterLedLookup[hour], color);
}

// Set a specific LED on the fan.
void setLed(int fanId, int ledId, CRGB color) {
  if (ledId > 15) {
    Serial.println("LedId out of range.");
    return;
  }
  
  int ledIndex = (16 * (fanId)) + ledId;
  leds[ledIndex] = color;
  return;
}

// Helpers

// Get the color to use for the outer fan cicle.
// if it's within the ideal range then it's green
// otherwise red or blue for above or below
// Get the color to use for the value.
// For color palettes see http://fastled.io/docs/3.1/colorpalettes_8h_source.html
// CloudColors_p
// LavaColors_p
// OceanColors_p
// ForestColors_p
// RainbowColors_p
// RainbowStripeColors_p
// PartyColors_p
// HeatColors_p
CRGB getRangeColor(int value, displayRange_t displayRange) {// int idealRangeMin, int idealRangeMax, int minValue, int maxValue) {  
  // See
  // CRGB color = ColorFromPalette(HeatColors_p, heatIndex);
  if (value > displayRange.idealRangeHigh) { 
    // Todo: Do some maths to get a range from Yellow[backgroundColor]->Red[hotColor] and factor by the 
    // range from idealTemperature to maxTemperature.
    return CRGB::Red;

    // 254... returns value outside of the color palette.
    //return ColorFromPalette(rgPalette , heatIndex);
  } else if (value < displayRange.idealRangeLow) {
    // Todo: math as above! but to Blue[coldColor]
    return CRGB::Blue;
  }else {
    return CRGB::Green;
  }
}


CRGB getBackgroundColor(int fanId, int value, displayRange_t displayRange) {// int desiredValue, int minValue, int maxValue ){
  // TODO: Range this from Green through to Yellow (or blue or something...).
  if (value < displayRange.minValue) {
    return CRGB::Blue;
  } 

  if (value > displayRange.maxValue) {
    return CRGB::Red;
  }
  
  CRGB color;
  color.red = 0xEE;
  color.green = 0xE8;
  color.blue =  0x20;
  return color;
  //return CRGB::Yellow;
}


