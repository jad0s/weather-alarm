#include "pins.h"
#include <Arduino.h>

bool alarmActive = false;

void alarm(){
  static bool buzzing = false;
  if(!buzzing){
    tone(BUZZER_PIN, 262);
    buzzing = true;
  }
  
  
}