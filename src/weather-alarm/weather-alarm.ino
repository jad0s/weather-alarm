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
#include "icons.h"


void animationTask(void * parameter){
  int phase = 0;
  while(loading){
    display.clearDisplay();
    switch(phase){
      case 0: display.drawBitmap(0,0,loading_0,128,64,1); break;
      case 1: display.drawBitmap(0,0,loading_1,128,64,1); break;
      case 2: display.drawBitmap(0,0,loading_2,128,64,1); break;
      case 3: display.drawBitmap(0,0,loading_3,128,64,1); break;
    }
    display.display();
    phase = (phase + 1) % 4;
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  display.clearDisplay();
  display.display();

  vTaskDelete(NULL);
}


void setup() {
  initDisplay();
  xTaskCreate(animationTask, "animTask", 2048, NULL, 1, NULL);
  Serial.begin(115200);
  Alarm defaultAlarm;
  defaultAlarm.hour = 7;
  defaultAlarm.minute = 0;
  defaultAlarm.tempCond = COND_OFF;
  defaultAlarm.tempValue = 0;
  defaultAlarm.enabled = true;
  alarms.push_back(defaultAlarm);
  initWiFi();
  initTime();
  updateTime();  
  initBuzzer();
  encoder.begin();
  fetchWeather();
  updateDisplayTime();
  screenDirty = true;
  loading = false;
  display.clearDisplay();
  display.display();
  displayTouched();
}

void loop() {
  encoder.tick();
  int32_t steps = encoder.getSteps();
  bool clicked = encoder.wasClicked();
  bool touched = (digitalRead(TOUCH_PIN) == HIGH);

  hours = (getHours() + 1) % 24;
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
      uiState = UI_ALARM_LIST;
      screenDirty = true;
    }/*else if(uiState == UI_SET_ALARM){
      if(alarmField == 3){
        uiState = UI_HOME;
        alarmField = 0;
        alarmHour = tmp_alarmHour;
        alarmMinute = tmp_alarmMinute;
        tempValue = tmp_tempValue;
        tempCondition = tmp_tempCondition;
        Serial.printf("Alarm set for %d:%d\n", alarmHour, alarmMinute);
      }else{
        alarmField = (alarmField + 1) % 4;
      }
      screenDirty = true;
    }*/
  }

  if(touched || clicked || steps != 0){
    displayTouched();
  }

  if(steps != 0){
    if (uiState == UI_ALARM_EDIT) {
      if (encoder.wasClicked()) {
          if (editIndex == -1) {
              // Enter editing mode for the selected field
              editIndex = cursorIndex;
          } else {
              // Exit editing mode
              editIndex = -1;
          }
          screenDirty = true;
      }

      if (steps != 0) {
          if (editIndex == -1) {
              // moving the cursor
              cursorIndex = constrain(cursorIndex + steps, 0, 4);
          } else {
              // editing the selected field
              editAlarmField(alarms[selectedAlarmIndex], editIndex, steps);
          }
          screenDirty = true;
      }
    }

  }

  if(screenDirty){
    switch(uiState){
      case UI_HOME:
        updateDisplayTime();
        break;
      case UI_ALARM_LIST:
        drawAlarmList();
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

  static bool rang = false;

  if(hours == alarmHour && minutes == alarmMinute && rang == false){
    switch(tempCondition){
      case 0:
        alarmActive = true;
        break;
      case 1: 
        if(cachedTemp < tempValue){
          alarmActive = true;
        }
        break;
      case 2:
        if(cachedTemp > tempValue){
          alarmActive = true;
        }
    }
  }


  if(alarmActive){
    alarm();
    rang = true;
    Serial.printf("alarm time is %d:%d, current time is %d:%d, alarming\n", alarmHour, alarmMinute, hours, minutes);
    if(digitalRead(TOUCH_PIN) == HIGH){
      alarmActive = false;
      noTone(BUZZER_PIN);
    }
  }

  if(hours == 0 && minutes == 0){
    rang = false;
  }

    

    
}
