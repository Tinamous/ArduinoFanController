#include "customTypes.h"
#include <FastLED.h>


// Converts from a clock 'hour' position (0..12) (0==12 to make range lookup easier) to a led id (0..15, well 4..15)
// This assumes the fan is placed so the wire exits top right....
int fanOuterLedLookup[] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14};

// Convert the scale 0..11 to an "hour" position on the display.
int normalisedHours[] = {7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6};


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



// ----------------------------------
// 1: Temperature display
// ----------------------------------

void showTemperature(int fanId) { 
  // Map the desired value onto the fan surround.
  // *10 to avoid float usage
  int temperatureFactorised = temperature * temperatureRange.factor;
  
  mapToFan(fanId, 
    temperatureFactorised, 
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
  int temperatureFactorised = temperature * temperatureRange.factor;
  
  // "Cold"
  if (temperatureFactorised < temperatureRange.idealRangeLow) {
    setNoseColor(fanId, CRGB::Blue);
    return;  
  } 

  // "Hot"
  if (temperatureFactorised > temperatureRange.idealRangeHigh ) {
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

// Map the desired value onto the fan surround.
// where desired value == 12 o'clock
// Min = 7 o'clock
// Max = 6 (or 5) o'clock
// int desiredValue, int idealRangeMin, int idealRangeMax, int minValue, int maxValue)
void mapToFan(int fanId, int value, displayRange_t displayRange) {
  // The hour on the clock the value represents
  int hour = getRangeHour(value, displayRange.minValue, displayRange.maxValue);

  // Set all the LEDs to the background color
  // then set just the ones appropriate.
  CRGB backgroundColor = getBackgroundColor(fanId, value, displayRange);
  setFanBackground(fanId, backgroundColor);

  CRGB color = getRangeColor(value, displayRange);
  setLedHourRange(fanId, value, displayRange, hour, color);
}

// Get the value as an hour on the clock face.
int getRangeHour(int value, int minValue, int maxValue) {
  
  if (value > maxValue) {
    return 6;  
  } else if (value < minValue) {
    return 6;
  } else {
    // Convert it to a scale of 0..11 for hour lookup
    int hourIndex = map(value, minValue, maxValue, 0, 11);

    // Convert from a 0..11 value from the map
    // into am hour value (12 O'Clock +/- 6 hours)
    return normalisedHours[hourIndex];
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
    Serial.print("Perfect match, lighting whole range");
    // top digit
    setLedByHour(fanId, 0, color);
    // As it's bang on show the whole outline as the desired color.
    setFanBackground(fanId, color);
  }
}

// --------------------------------

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
  int ledIndex = (16 * (fanId-1)) + ledId;
  leds[ledIndex] = color;
  return;
  
  Serial.print("Setting LED: ");
  Serial.print(ledIndex);
  Serial.print(" (Fan: ");
  Serial.print(fanId);
  Serial.print(" ,Led: ");
  Serial.print(ledId);
  Serial.print(") to: ");
  Serial.print(color, HEX);
  Serial.println();
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
    // Map the value to the heat index (0..255) range.
  //uint8_t heatIndex = map(value, minValue, maxValue, 0, 254);
  //Serial.print("heatIndex: ");
  //Serial.print(heatIndex);
  //Serial.println();
    
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
