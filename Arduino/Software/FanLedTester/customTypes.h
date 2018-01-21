#ifndef __INC_CUSTOM_TYPES_H
#define __INC_CUSTOM_TYPES_H

struct DisplayRangeType {
  // The "Perfect" value. represents 12 O'Clock on the clock
  float idealValue;
  // The ideal range that the value is desired/comfortable within
  float idealRangeLow;
  float idealRangeHigh;

  // Min/Max value that is on the display
  float minValue;
  float maxValue;

  int factor = 10;
};

typedef DisplayRangeType displayRange_t;

typedef enum {
    Ignore = 0,
    Temperature = 1, 
    Humidity = 2,
    Pressure = 3,
    AirQuality = 4,
    Clock = 5,
    CircleSingleFan = 6,
    CircleAllFans = 7,
    Dust = 8,
    Timer = 9,
    Pomodoro = 10,
    PomodoroWorkOnly = 11,
    PomodoroPlayOnly = 12,
    Automatic = 255,
} DisplayMode;

#endif
