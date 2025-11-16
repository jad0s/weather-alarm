#include "pins.h"
#include <Arduino.h>
#include "encoder.h"

bool alarmActive = false;
int alarmHour;
int alarmMinute;
int tmp_alarmHour = 7;
int tmp_alarmMinute = 0;


void alarm(){
  static bool buzzing = false;
  if(!buzzing){
    tone(BUZZER_PIN, 262);
    buzzing = true;
  }
  
  
}



void setAlarm(){
  static int alarmField = 0;

  int32_t steps = encoder.getSteps();
  if(steps != 0){
      if(alarmField == 0){
          alarmHour = (alarmHour + steps + 24) % 24;
      } else {
          alarmMinute = (alarmMinute + steps + 60) % 60;
      }
  }
  if(encoder.wasClicked()){
    alarmField = (alarmField + 1) % 2; // toggle field
  }


}