#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void ring_alarm();
void setAlarm();

extern Alarm tempAlarm;

enum WeatherType{
  RAIN,
  SNOW,
  STORM,
  SUNNY,
  OVERCAST
};

enum TempCondition{
  COND_OFF = 0, //OFF
  COND_LT = 1, //Less than
  COND_GT = 2 //Greater than
};

struct AlarmCondition{
  WeatherType type;
  bool boolToTrigger; //false = condition must be false for alarm to trigger, true = condition must be true
};

struct Alarm{
  int hour = 7;
  int minute = 0;
  TempCondition tempCond = COND_OFF; //0 = OFF, 1 = < (less than), 2 = > (greater than)
  int tempValue = 0;
  std::vector<AlarmCondition> weather;
  bool enabled = true;
  bool rang = false;
  bool active = false;
};

extern std::vector<Alarm> alarms;

#endif
