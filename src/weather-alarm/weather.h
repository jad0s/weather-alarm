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
extern float cachedWindSpeed; // in km/h
extern int cachedWindDirection; // degrees from north
extern int cachedWeatherCode;
extern bool weatherValid;
extern unsigned long lastFetch;

struct Alarm;

// Grouped, display-friendly weather icon categories
enum WeatherIcon {
    WI_CLEAR,
    WI_PARTLY_CLOUDY,
    WI_CLOUDY,
    WI_DRIZZLE,
    WI_RAIN,
    WI_SHOWERS,
    WI_SNOW,
    WI_STORM,
    WI_FOG,
    WI_UNKNOWN,
    WI_COUNT
};

// Fetch current weather and populate cached* globals
void fetchWeather();

// Map raw Open-Meteo weather code to a grouped `WeatherIcon`
WeatherIcon mapWeatherCodeToIcon(int weatherCode);

// Human-readable name for icons (useful in UI lists)
const char* weatherIconName(WeatherIcon icon);

// Evaluate whether an alarm's weather conditions match the current weather.
bool checkWeather(const Alarm& alarm, WeatherIcon current);



#endif
