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
  display.setCursor(0, 10);

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", tmp_alarmHour, tmp_alarmMinute);
  display.println(buf);

  display.setCursor(0, 40);
  switch(tmp_tempCondition){
    case 0:
      display.print("OFF");
      break;
    case 1:
      display.print("<");
      break;
    case 2:
      display.print(">");
      break;
  }
  if(tmp_tempCondition != 0){
    display.setCursor(60, 40);
    display.print(tmp_tempValue);
    display.print("C");
  }

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

