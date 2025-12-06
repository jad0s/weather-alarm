#include "alarm.h"

enum UIState{
  UI_HOME,
  UI_ALARM_LIST,
  UI_ALARM_EDIT,
  UI_EDIT_FIELD,
  UI_WEATHER_MENU,
  UI_WEATHER_PICK,
  UI_WEATHER_DELETE_CONFIRM
};

extern UIState uiState;
extern bool screenDirty;
extern int cursorIndex;
extern int selectedAlarmIndex;
extern int editIndex;
extern int subIndex;
extern const int weatherListCount;
extern const WeatherCode weatherList[];

void updateUi();
void updateDisplayTime();
void drawAlarmUI();
void drawAlarmList();
void drawAlarmEdit();
void editAlarmField(int index, int subIndex, int steps);
void drawWeatherMenu();
void drawWeatherPick();
void drawWeatherDeleteConfirm();