const float LAT = 49.686956;
const float LON = 18.351402;
extern float cachedTemp;
extern float cachedRain;
extern float cachedPrecip;
extern bool weatherValid;
extern unsigned long lastFetch;

void fetchWeather();
