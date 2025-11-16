#include "display.h"
#include "encoder.h"
#include "touch.h"
#include "wifi.h"
#include "alarm.h"
#include "buzzer.h"
#include <Arduino.h>
#include "pins.h"
#include "weather.h"
#include "systemtime.h"
#include "ui.h"



void setup() {
  initDisplay();
  displayLogo();
  Serial.begin(115200);
  initWiFi();
  initTime();
  updateTime();  
  initBuzzer();
  encoder.begin();
  fetchWeather();
  updateDisplayTime();
  screenDirty = true;
  display.clearDisplay();
  display.display();
}

void loop() {
  encoder.tick();
  int32_t steps = encoder.getSteps();
  bool clicked = encoder.wasClicked();
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  hours = (getHours() + 2) % 24;
  minutes = getMinutes();
  

  // read steps (signed) and print direction
  
  /*if (steps != 0) {
    // if steps magnitude >1, user turned quickly; print each detent
    while (steps > 0) { Serial.println("CW"); steps--; }
    while (steps < 0) { Serial.println("CCW"); steps++; }
  }*/

  static int alarmField = 0;

  // read button
  if (clicked && screenOn == true) {
    Serial.println("Click");
    if(uiState == UI_HOME){
      uiState = UI_SET_ALARM;
      screenDirty = true;
    }else if(uiState == UI_SET_ALARM){
      if(alarmField == 1){
        uiState = UI_HOME;
        alarmField = 0;
        alarmHour = tmp_alarmHour;
        alarmMinute = tmp_alarmMinute;
        Serial.printf("Alarm set for %d:%d\n", alarmHour, alarmMinute);
      }else{
        alarmField = (alarmField + 1) % 2;
      }
      screenDirty = true;
    }
  }

  if(touched || clicked || steps != 0){
    displayTouched();
  }

  if(steps != 0){
    if (uiState == UI_SET_ALARM) {
      if (alarmField == 0) {
        tmp_alarmHour = (tmp_alarmHour + (int)steps) % 24;
        if (tmp_alarmHour < 0) tmp_alarmHour += 24;
      } else {
        tmp_alarmMinute = (tmp_alarmMinute + (int)steps) % 60;
        if (tmp_alarmMinute < 0) tmp_alarmMinute += 60;
      }
      screenDirty = true;
    } else {
      // optional: global encoder actions in HOME (scroll menu, etc.)
    }
  }

  if(screenDirty){
    switch(uiState){
      case UI_HOME:
        updateDisplayTime();
        break;
      case UI_SET_ALARM:
        drawAlarmUI();
        break;
    }
    screenDirty = false;
  }


  if(millis()%60000 == 0){
    updateTime();
  }
          // update NTP time
  

  if (millis() - lastFetch > 1 * 60 * 1000){
    weatherValid = false;
    fetchWeather();
  }
    

  displayUpdate();

  if(hours == alarmHour && minutes == alarmMinute){
    alarmActive = true;
  }


  if(alarmActive){
    alarm();
    Serial.printf("alarm time is %d:%d, current time is %d:%d, alarming\n", alarmHour, alarmMinute, hours, minutes);
    if(digitalRead(TOUCH_PIN) == HIGH){
      alarmActive = false;
      noTone(BUZZER_PIN);
    }
  }

    

    
}
