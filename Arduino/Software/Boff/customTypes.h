#ifndef __INC_CUSTOM_TYPES_H
#define __INC_CUSTOM_TYPES_H

#include <FastLED.h>

typedef enum {
    Ignore = 0, // done
    Temperature = 1, // done
    Humidity = 2, // done
    Pressure = 3, // kind of done
    AirQuality = 4,// done
    Clock = 5, // done
    CircleSingleFan = 6, // done
    CircleAllFans = 7, // todo
    Dust = 8, // todo - need ranges
    Timer = 9, // todo
    Pomodoro = 10, // todo
    PomodoroWorkOnly = 11, // todo
    PomodoroPlayOnly = 12, // todo
    FixedColor = 13, // todo
    lightLevel = 14, // todo (need range)
    // The user selected fan speed (0..11)
    SelectedFanSpeed = 15, // done
    // Fan Speed in RPM
    FanSpeed = 16, // broken
    WiFiStrength = 17, // TODO
    OnOff = 18, // TODO
    MqttFeed = 19, // TODO
    Fancy = 100,
    Automatic = 255, // todo
} DisplayMode;

struct DisplayRangeType {
  // The "Perfect" value. represents 12 O'Clock on the clock
  float idealValue;
  // The ideal range that the value is desired/comfortable within
  float idealRangeLow;
  float idealRangeHigh;

  // Min/Max value that is on the display
  float minValue;
  float maxValue;

  int factor;

  // 0 = relative +/- the 12 O'Clock position
  int mode = 0;

  // Low, Medium and High set points
  // when above the ideal point
  int fanSpeedAboveIdeal[3];

  // Low, Medium and High set points
  // when below the ideal point
  int fanSpeedBelowIdeal[3];
};

typedef DisplayRangeType displayRange_t;


struct FanInfoType {
  // The fan Id (1..4) rather than the index in the array
  // a more user friendly (and aligned with the PCB labels!)
  int fanId;
  
  // Control pin for the fan
  int pwmPin;
  
  // Pulse pin from the fan.
  int tachPin;
  
  // If the fan is enabled for use.
  bool enabled;
  
  // The count of the pulses.
  int pulseCount;
  
  // Current RPM computed from pulse counts
  int computedRpm;
  
  // 0..11. Use speedPwm to get the PWM value for the set speet.
  // Uses 0..11 as it displays nicely on the 12 pixel fan LED display
  // This is the requested speed for the fan. See currentSpeed for the 
  // current fan speed
  int speedSet;   

  // May be different to the speedSet if this has not
  // yet been actionsed.
  int currentSpeed = 12;

  // The currently set PWM value for the fan
  // debug/diagnostic use only
  int currentPwm = 0;

  // Indexed by speedSet, pwm value to use at each setting
  // i.e. how fast to run the fan at that speed.
  int speedPwm[12] = {0, 23, 46, 69, 92, 115, 138, 180, 200, 220, 240, 255};
  
  // Array of RPM's expected RPMs indexed by speedSet (0..11)
  // i.e. how fast we expect the fan to be going at a selected speed.
  // e.g. [0] = 0, [1] = 400, [2] = 600, [3] = 800, 
  int expectedRpm[12] = {0, 100, 800, 800, 800, 800, 1200, 1200, 1200, 1500, 1500, 1500};
  
  // How many pulses the fan makes for each RPM
  int pulseToRpmFactor = 4; // 1, 2, or 4 typically.

  // Prefered fan (outer) color for fixed colours.
  CRGB outerColor;
  
  // Color for the fans nose
  CRGB noseColor;
};

typedef FanInfoType fanInfo_t;



#endif

