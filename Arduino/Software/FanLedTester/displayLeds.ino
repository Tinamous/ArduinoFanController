#include "customTypes.h"
#include <FastLED.h>


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

int getRangeHour(int value, int minValue, int maxValue) {
  
  if (value > maxValue) {
    Serial.println("Above Max");
      return 6;  
  } else if (value < minValue) {
    Serial.println("Below Min");
    return 6;
  } else {
    // Convert it to a scale of 0..11 for hour lookup
    int hourIndex = map(value, minValue, maxValue, 0, 11);
    
    int hour = normalisedHours[hourIndex];
    Serial.print("Temperature : ");
    Serial.print(value);
    Serial.print(", hourIndex: ");
    Serial.print(hourIndex, DEC);
    Serial.print(", Hour: ");
    Serial.print(hour, DEC);
    Serial.println();
    return hour;
  }
}

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
