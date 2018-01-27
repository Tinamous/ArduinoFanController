#include "customTypes.h"
#include <FastLED.h>
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

void SetLedsOnOff(bool newState) {
  ledsEnabled = newState;
  
  if (ledsEnabled) {
    Serial.println("All LEDs enabled.");
  } else {
    Serial.println("All LEDs off.");
  }
}

void updateFansLeds() {
  int maxFan = 4;
  for (int fanId = 0; fanId < maxFan; fanId++) {
    showOuterValue(fanId);
    showNoseValue(fanId);
  }
}

void updateStrip1Leds(){}
void updateStrip2Leds(){}

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
    if (!ledsEnabled) {
      FastLED.setBrightness( 0 );
    } else {
      FastLED.setBrightness( ledBrightness );
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

  Serial.print("Temperature: ");
  Serial.println(temperature);
  
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
  Serial.println("Humidity");
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
   mapToFan(fanId, eCO2, airQualityRange);
}

void showNoseAirQuality(int fanId) {
  setGenericNose(fanId, eCO2, airQualityRange);
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
  int speed = fanInfos[fanId].speedSet;

  setFanBackground(fanId-1, CRGB::Blue);

  for (int i = 0; i<=speed; i++) {
    setLedByHour(fanId, i, CRGB::Green);
  }
}

// ----------------------------------
// 15: Fan Speed
// ----------------------------------
void showFanRpmSpeed(int fanId) {
  displayRange_t displayRange = setupFanDisplayRange(fanId-1);
  int rpm = fanInfos[fanId-1].computedRpm;
  mapToFan(fanId-1, rpm, displayRange);
}

void showNoseFanSpeed(int fanId) {
  // Default to green...
  setNoseColor(fanId-1, CRGB::Green);
  
  // but show an error if the speed is low.
  showFanSpeedLowError(fanId);
}

// If the fan has a speed error light up the nose
// a bright red color, otherwise leave it as is.
void showFanSpeedLowError(int fanId) {
 
  // If the power is off then ignore any fan speed setting.
  if (!master_power) {
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
}

displayRange_t setupFanDisplayRange(int fanId) {

  fanInfo_t fanInfo = fanInfos[fanId];
  
  // Ideal value depends on the fan PWM.
  // Min/Max depend on the fan...
  displayRange_t range;
  range.idealValue = fanInfo.expectedRpm[fanInfo.speedSet]; // lookup fan/ fan run mode.
  
  if (fanInfo.speedSet > 0) {
    range.idealRangeLow = fanInfo.expectedRpm[fanInfo.speedSet-1]; 
  } else {
    range.idealRangeLow = 0;
  }

  // 11 is the max fan speed setting (0..11).
  if (fanInfo.speedSet < 11) {
    range.idealRangeHigh = fanInfo.expectedRpm[fanInfo.speedSet+1]; 
  } else {
     range.idealRangeHigh = fanInfo.expectedRpm[fanInfo.speedSet] + 50;
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
// ...: Fancy
// ----------------------------------
// Automatic - show which ever measurement needs attemption
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
    Serial.println("Above max");
    return 11;  
  } else if (value < minValue) {
    Serial.println("below min");
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
    Serial.println("Nose below range");
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

// TODO: Pass colour in...
void setLed(int fanId, int ledId, CRGB color) {
  if (ledId > 15) {
    Serial.println("LedId out of range.");
    return;
  }
  
  int ledIndex = (16 * (fanId-1)) + ledId;
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


