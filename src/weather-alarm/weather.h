#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

enum WeatherCode {
    WC_CLEAR = 0,
    WC_MAINLY_CLEAR = 1,
    WC_PARTLY_CLOUDY = 2,
    WC_OVERCAST = 3,
    WC_FOG = 45,
    WC_RIME_FOG = 48,
    WC_DRIZZLE_LIGHT = 51,
    WC_DRIZZLE_MOD = 53,
    WC_DRIZZLE_DENSE = 55,
    WC_RAIN_SLIGHT = 61,
    WC_RAIN_MOD = 63,
    WC_RAIN_HEAVY = 65,
    WC_SHOWER_SLIGHT = 80,
    WC_SHOWER_MOD = 81,
    WC_SHOWER_VIOLENT = 82,
    WC_STORM = 95,
    WC_STORM_HAIL_SLIGHT = 96,
    WC_STORM_HAIL_HEAVY = 99
};

extern const float LAT;
extern const float LON;
extern float cachedTemp;
extern float cachedRain;
extern float cachedPrecip;
extern int cachedWeatherCode;
extern bool weatherValid;
extern unsigned long lastFetch;

struct Alarm;

void fetchWeather();
bool checkWeather(const Alarm& alarm, WeatherCode current);



#endif
