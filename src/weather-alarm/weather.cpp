#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "weather.h"
#include "alarm.h"

float cachedTemp = NAN;
float cachedRain = NAN;
float cachedPrecip = NAN;
float cachedWindSpeed = NAN;
int cachedWindDirection = 0;
int cachedWeatherCode = NAN;
bool weatherValid = false;
unsigned long lastFetch = 0;

const float LAT = 49.686956;
const float LON = 18.351402;


void fetchWeather(){
  HTTPClient http;

  String url = "https://api.open-meteo.com/v1/forecast?latitude=" +
               String(LAT) + "&longitude=" + String(LON) +
               "&current_weather=true&windspeed_unit=kmh";

  http.begin(url);

  int code = http.GET();
  if(code > 0){
    String payload = http.getString();
    Serial.println("Raw JSON:");
    Serial.println(payload);

    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if(err){
      Serial.print("JSON error: ");
      Serial.println(err.c_str());
      return;
    }

    // Prefer current_weather object if present
    float temp = NAN;
    float rain = NAN;
    float precipitation = NAN;
    int weatherCode = 0;
    float windSpeed = NAN;
    int windDir = 0;

    if (doc.containsKey("current_weather")) {
      JsonObject cw = doc["current_weather"].as<JsonObject>();
      if (cw.containsKey("temperature")) temp = cw["temperature"].as<float>();
      if (cw.containsKey("weathercode")) weatherCode = cw["weathercode"].as<int>();
      if (cw.containsKey("windspeed")) windSpeed = cw["windspeed"].as<float>();
      if (cw.containsKey("winddirection")) windDir = cw["winddirection"].as<int>();
    }

    // fallbacks for older payload shapes
    if (doc.containsKey("current")){
      JsonObject cur = doc["current"].as<JsonObject>();
      if (isnan(temp) && cur.containsKey("temperature_2m")) temp = cur["temperature_2m"].as<float>();
      if (cur.containsKey("rain")) rain = cur["rain"].as<float>();
      if (cur.containsKey("precipitation")) precipitation = cur["precipitation"].as<float>();
      if (cur.containsKey("weather_code") && weatherCode == 0) weatherCode = cur["weather_code"].as<int>();
      if (cur.containsKey("windspeed_10m") && isnan(windSpeed)) windSpeed = cur["windspeed_10m"].as<float>();
      if (cur.containsKey("winddirection_10m") && windDir == 0) windDir = cur["winddirection_10m"].as<int>();
    }

    cachedWeatherCode = weatherCode;
    cachedTemp = temp;
    cachedRain = rain;
    cachedPrecip = precipitation;
    cachedWindSpeed = windSpeed;
    cachedWindDirection = windDir;
    lastFetch = millis();
    // Guard missing numeric values so UI doesn't print "nan"
    if (isnan(cachedRain)) cachedRain = 0.0;
    if (isnan(cachedPrecip)) cachedPrecip = 0.0;
    if (isnan(cachedWindSpeed)) cachedWindSpeed = 0.0;

    // consider weather valid if we got at least a temperature or a weather code
    weatherValid = !isnan(cachedTemp) || cachedWeatherCode != 0;

    Serial.println("Parsed values:");
    Serial.print("Temperature: ");
    Serial.println(temp);

    Serial.print("Rain (mm/h): ");
    Serial.println(rain);

    Serial.print("Precipitation (mm): ");
    Serial.println(precipitation);

    Serial.print("Wind speed (km/h): ");
    Serial.println(windSpeed);

    Serial.print("Wind dir (deg): ");
    Serial.println(windDir);
  }else {
    Serial.print("HTTP Error: ");
    Serial.println(code);
    weatherValid = false;
  }

  http.end();
}

bool checkWeather(const Alarm& alarm, WeatherCode current){
  //This function checks if weather conditions for an alarm pass
  //positive conditions are grouped with OR logic
  //meaning at least one has to be true for the alarm to activate
  //negative condition are grouped with AND logic
  //meaning all of them have to be false for the alarm to activate



  if(!alarm.positive.empty()){
    bool anyPositive = false;
    for(auto c : alarm.positive){
      if(c == current){
        anyPositive = true;
        break;
      }
      if(!anyPositive) return false;

    }
  }

  for(auto c : alarm.negative){
    if(c == current){
      return false;
    }
  }

  return true;

}