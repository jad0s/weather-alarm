#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void alarm();
void setAlarm();
extern bool alarmActive;
extern int alarmHour;
extern int alarmMinute;
extern int tmp_alarmHour;
extern int tmp_alarmMinute;
extern int8_t tempCondition;
extern int8_t tempValue;
extern int8_t tmp_tempCondition;
extern int8_t tmp_tempValue;

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
  int hour, minute;
  TempCondition tempCond; //0 = OFF, 1 = < (less than), 2 = > (greater than)
  int tempValue;
  std::vector<AlarmCondition> weather;
  bool enabled;
};

#endif
