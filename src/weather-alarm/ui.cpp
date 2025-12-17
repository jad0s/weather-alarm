#include "ui.h"
#include "encoder.h"
#include "alarm.h"
#include "weather.h"
#include "display.h"
#include "systemtime.h"
#include "icons.h"

UIState uiState = UI_HOME;
bool screenDirty = false;

int cursorIndex = 0;
int selectedAlarmIndex = 0;
int editIndex = -1; // -1 = no field being edited
int subIndex = 0; //index of the selected sub-setting
int weatherPickListType = 0; // 1 = positive, 2 = negative
int weatherPickChosenIndex = 0; // chosen index in carousel


/*void drawAlarmUI(){
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
}*/

void drawAlarmList() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    // "+ Add" button
    if (cursorIndex == 0) display.print("> ");
    else                   display.print("  ");
    display.println("+ Add alarm");

    for (int i = 0; i < alarms.size(); i++) {
        if (cursorIndex == i+1) display.print("> ");
        else                    display.print("  ");

        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", alarms[i].hour, alarms[i].minute);
        display.print(i+1);
        display.print(") ");
        display.print(buf);
        display.print(alarms[i].enabled ? " *" : "");
        display.println();
    }

    display.display();
}



void drawAlarmEdit() {
    // guard: if no alarms, go back to list
    if (alarms.empty()) {
        uiState = UI_ALARM_LIST;
        cursorIndex = 0;
        screenDirty = true;
        return;
    }

    // clamp selectedAlarmIndex
    if (selectedAlarmIndex < 0) selectedAlarmIndex = 0;
    if (selectedAlarmIndex >= (int)alarms.size()) selectedAlarmIndex = alarms.size() - 1;

    Alarm &a = tempAlarm;

    display.clearDisplay();
    display.setTextSize(1);

    // keep cursor within [0..4]
    cursorIndex = constrain(cursorIndex, 0, 4);

    int y = 0;

    auto line = [&](const char *label, int index) {
        display.setCursor(0, y);
        if (cursorIndex == index) display.print("> ");
        else display.print("  ");
        display.println(label);
        y += 12;
    };

    char buf[24];

    // Time line
    snprintf(buf, sizeof(buf), "Time: %02d:%02d", a.hour, a.minute);
    line(buf, 0);

    // Temp cond + value line
    const char *condSym = " ";
    if (a.tempCond == COND_LT) condSym = "<";
    else if (a.tempCond == COND_GT) condSym = ">";
    // show like: "Temp: <  5C"
    snprintf(buf, sizeof(buf), "Temp: %s %dC", condSym, a.tempValue);
    line(buf, 1);

    // Weather count line
    int totalWeather = a.positive.size() + a.negative.size();
    snprintf(buf, sizeof(buf), "Weather: %d", totalWeather);
    line(buf, 2);

    // Buddy line
    if (a.buddyEnabled) {
        snprintf(buf, sizeof(buf), "Buddy: %02d:%02d", a.buddyHour, a.buddyMinute);
    } else {
        snprintf(buf, sizeof(buf), "Buddy: Off");
    }
    line(buf, 3);

    line("Back", 4);

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
    // draw wind arrow and speed on the right side
    if (weatherValid && !isnan(cachedWindSpeed)) {
        // choose a small area on the right
        int cx = 100;
        int cy = 18;
        int len = 10;
        // wind direction: degrees from north (0 = north/up)
        float rad = cachedWindDirection * PI / 180.0f;
        float dx = sin(rad);
        float dy = -cos(rad);
        int ex = cx + (int)(dx * len);
        int ey = cy + (int)(dy * len);
        // main shaft
        display.drawLine(cx, cy, ex, ey, 1);
        // arrowhead
        float ahang = 20.0f * PI / 180.0f;
        float sx = cos(ahang) * (-dx) - sin(ahang) * (-dy);
        float sy = sin(ahang) * (-dx) + cos(ahang) * (-dy);
        int ax1 = ex + (int)(sx * 6);
        int ay1 = ey + (int)(sy * 6);
        float sx2 = cos(-ahang) * (-dx) - sin(-ahang) * (-dy);
        float sy2 = sin(-ahang) * (-dx) + cos(-ahang) * (-dy);
        int ax2 = ex + (int)(sx2 * 6);
        int ay2 = ey + (int)(sy2 * 6);
        display.drawLine(ex, ey, ax1, ay1, 1);
        display.drawLine(ex, ey, ax2, ay2, 1);

        // speed text under the arrow
        display.setTextSize(1);
        char windBuf[16];
        snprintf(windBuf, sizeof(windBuf), "%d km/h", (int)round(cachedWindSpeed));
        display.setCursor(84, 36);
        display.println(windBuf);
    }
    display.display();
}

void editAlarmField(int index, int subIndex, int steps) {
    Alarm &a = tempAlarm;

    switch (index) {

        case 0: // Time row
            if (subIndex == 0) {
                // hour
                a.hour = (a.hour + steps + 24) % 24;
            } else if (subIndex == 1) {
                // minute
                a.minute = (a.minute + steps + 60) % 60;
            }
            break;

        case 1: // Temperature row
            if (subIndex == 0) {
                // temp condition (OFF, <, >)
                a.tempCond = (TempCondition) constrain((int)a.tempCond + steps, 0, 2);
            } else if (subIndex == 1) {
                // temp value
                if (a.tempCond != COND_OFF)
                    a.tempValue = constrain(a.tempValue + steps, -40, 60);
            }
            break;

        case 3: // Buddy row: hour + minute
            if (!a.buddyEnabled) a.buddyEnabled = true; // entering edit enables buddy
            if (subIndex == 0) {
                a.buddyHour = (a.buddyHour + steps + 24) % 24;
            } else if (subIndex == 1) {
                a.buddyMinute = (a.buddyMinute + steps + 60) % 60;
            }
            break;
        // 3 = Delete  (handled outside)
        // 4 = Back    (handled outside)

    }
}

void drawWeatherMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    Alarm &a = tempAlarm;

    // layout constants
    const int rowHeight = 12;
    const int visibleRows = 5; // 64px / 12px ~= 5 rows

    // build total list length (indices: 0=+POS,1=+NEG, 2.. = items, last = Back)
    int totalItems = 2 + a.positive.size() + a.negative.size() + 1; // +1 for Back

    // decide window start so cursor is visible and roughly centered
    int start = cursorIndex - (visibleRows/2);
    if (start < 0) start = 0;
    if (start > totalItems - visibleRows) start = max(0, totalItems - visibleRows);

    int y = 0;
    for (int idx = start; idx < start + visibleRows && idx < totalItems; idx++) {
        display.setCursor(0, y);
        if (cursorIndex == idx) display.print("> "); else display.print("  ");

        if (idx == 0) {
            display.println("+ Add POSITIVE");
        } else if (idx == 1) {
            display.println("+ Add NEGATIVE");
        } else {
            int dataIndex = idx - 2; // 0.. for positive then negative
                if (dataIndex < (int)a.positive.size()) {
                const char *name = weatherIconName(a.positive[dataIndex]);
                char lineBuf[32];
                snprintf(lineBuf, sizeof(lineBuf), "POS: %s", name);
                display.println(lineBuf);
            } else if (dataIndex < (int)a.positive.size() + (int)a.negative.size()) {
                int negIndex = dataIndex - a.positive.size();
                const char *name = weatherIconName(a.negative[negIndex]);
                char lineBuf[32];
                snprintf(lineBuf, sizeof(lineBuf), "NEG: %s", name);
                display.println(lineBuf);
            } else {
                // Back (this happens when idx == totalItems-1)
                display.println("Back");
            }
        }

        y += rowHeight;
    }

    display.display();
}

const WeatherIcon weatherList[] = {
    WI_CLEAR,
    WI_PARTLY_CLOUDY,
    WI_CLOUDY,
    WI_FOG,
    WI_DRIZZLE,
    WI_RAIN,
    WI_SHOWERS,
    WI_SNOW,
    WI_STORM
};

const int weatherListCount = sizeof(weatherList)/sizeof(weatherList[0]);

void drawWeatherPick() {
    // Carousel style picker: show current (big) and next (small)
    display.clearDisplay();

    // main large item
        int idx = constrain(cursorIndex, 0, weatherListCount - 1);
        WeatherIcon mainIcon = weatherList[idx];
        WeatherIcon nextIcon = weatherList[(idx + 1) % weatherListCount];

        // draw large icon (32x32) at left/center
        const unsigned char* mainPtr = nullptr;
        switch(mainIcon){
            case WI_CLEAR: mainPtr = icon_clear_32; break;
            case WI_PARTLY_CLOUDY: mainPtr = icon_partly_cloudy_32; break;
            case WI_CLOUDY: mainPtr = icon_cloudy_32; break;
            case WI_DRIZZLE: mainPtr = icon_drizzle_32; break;
            case WI_RAIN: mainPtr = icon_rain_32; break;
            case WI_SHOWERS: mainPtr = icon_showers_32; break;
            case WI_SNOW: mainPtr = icon_snow_32; break;
            case WI_STORM: mainPtr = icon_storm_32; break;
            case WI_FOG: mainPtr = icon_fog_32; break;
            default: mainPtr = icon_unknown_32; break;
        }
        if(mainPtr) display.drawBitmap(10, 6, mainPtr, 32, 32, 1);

        // draw small next icon on right (16x16)
        const unsigned char* nextPtr = nullptr;
        switch(nextIcon){
            case WI_CLEAR: nextPtr = icon_clear_16; break;
            case WI_PARTLY_CLOUDY: nextPtr = icon_partly_cloudy_16; break;
            case WI_CLOUDY: nextPtr = icon_cloudy_16; break;
            case WI_DRIZZLE: nextPtr = icon_drizzle_16; break;
            case WI_RAIN: nextPtr = icon_rain_16; break;
            case WI_SHOWERS: nextPtr = icon_showers_16; break;
            case WI_SNOW: nextPtr = icon_snow_16; break;
            case WI_STORM: nextPtr = icon_storm_16; break;
            case WI_FOG: nextPtr = icon_fog_16; break;
            default: nextPtr = icon_unknown_16; break;
        }
        if(nextPtr) display.drawBitmap(94, 20, nextPtr, 16, 16, 1);

    // footer: show the name of the currently-selected condition (single-line)
    display.setTextSize(1);
    display.setCursor(0, 52);
    char nameBuf[21];
    const char* namePtr = weatherIconName(mainIcon);
    int i = 0;
    for (; i < 20 && namePtr[i]; ++i) nameBuf[i] = namePtr[i];
    nameBuf[i] = '\0';
    display.println(nameBuf);

    display.display();
}

void drawWeatherPickConfirm() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 6);
    char buf[16];
        // display the currently-selected weather icon and name
        int chosenIdx = constrain(weatherPickChosenIndex, 0, weatherListCount - 1);
        WeatherIcon chosen = weatherList[chosenIdx];
        const unsigned char* chosenPtr = nullptr;
        switch(chosen){
            case WI_CLEAR: chosenPtr = icon_clear_16; break;
            case WI_PARTLY_CLOUDY: chosenPtr = icon_partly_cloudy_16; break;
            case WI_CLOUDY: chosenPtr = icon_cloudy_16; break;
            case WI_DRIZZLE: chosenPtr = icon_drizzle_16; break;
            case WI_RAIN: chosenPtr = icon_rain_16; break;
            case WI_SHOWERS: chosenPtr = icon_showers_16; break;
            case WI_SNOW: chosenPtr = icon_snow_16; break;
            case WI_STORM: chosenPtr = icon_storm_16; break;
            case WI_FOG: chosenPtr = icon_fog_16; break;
            default: chosenPtr = icon_unknown_16; break;
        }
        if(chosenPtr) display.drawBitmap(0, 6, chosenPtr, 16, 16, 1);
        display.setCursor(20, 8);
        display.println(weatherIconName(chosen));

    // Buttons
    display.setTextSize(1);
    display.setCursor(0, 36);
    if (cursorIndex == 0) display.print("> "); else display.print("  ");
    display.println("Save");

    display.setCursor(0, 48);
    if (cursorIndex == 1) display.print("> "); else display.print("  ");
    display.println("Cancel");

    display.display();
}

void drawWeatherDeleteConfirm() {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0,0);
    display.println("Delete?");
    
    display.setCursor(0,12);
    if (cursorIndex == 0) display.print("> ");
    else display.print("  ");
    display.println("Yes");

    display.setCursor(0,24);
    if (cursorIndex == 1) display.print("> ");
    else display.print("  ");
    display.println("No");

    display.display();
}

void drawAlarmDeleteConfirm() {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0,0);
    display.println("Delete alarm?");
    
    display.setCursor(0,12);
    if (cursorIndex == 0) display.print("> ");
    else display.print("  ");
    display.println("Yes");

    display.setCursor(0,24);
    if (cursorIndex == 1) display.print("> ");
    else display.print("  ");
    display.println("No");

    display.display();
}






