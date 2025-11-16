#include "ui.h"
#include "encoder.h"
#include "alarm.h"
#include "weather.h"
#include "display.h"
#include "systemtime.h"

UIState uiState = UI_HOME;
bool screenDirty = false;

void drawAlarmUI(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 30);

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", tmp_alarmHour, tmp_alarmMinute);
  display.println(buf);

  display.display();
}

void updateDisplayTime(){
  if(!screenOn){
    return;
  }
    display.clearDisplay();           // clear previous time
    display.setTextSize(3);
    display.setCursor(0, 0);

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
    display.println(buf);
    display.setTextSize(2);
    display.setCursor(0, 30);         // adjust vertical position

    if(weatherValid){
        char weatherBuf[32];
        snprintf(weatherBuf, sizeof(weatherBuf),
                 "%.1fC \n%.1fmm/h",
                 cachedTemp, cachedRain);
        display.println(weatherBuf);
    } else {
        display.println("Weather N/A");
    }
    display.display();
}

