#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "weather.h"
#include "alarm.h"

float cachedTemp = NAN;
float cachedRain = NAN;
float cachedPrecip = NAN;
int cachedWeatherCode = NAN;
bool weatherValid = false;
unsigned long lastFetch = 0;

const float LAT = 49.686956;
const float LON = 18.351402;


void fetchWeather(){
  HTTPClient http;

  String url = "https://api.open-meteo.com/v1/forecast?latitude=" +
               String(LAT) + "&longitude=" + String(LON) +
               "&current=temperature_2m,precipitation,rain,weather_code";

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

    float temp = doc["current"]["temperature_2m"];
    float rain = doc["current"]["rain"];
    float precipitation = doc["current"]["precipitation"];
    int weatherCode = doc["current"]["weather_code"];

    cachedWeatherCode = weatherCode;
    cachedTemp = temp;
    cachedRain = rain;
    cachedPrecip = precipitation;
    lastFetch = millis();
    weatherValid = true;

    Serial.println("Parsed values:");
    Serial.print("Temperature: ");
    Serial.println(temp);

    Serial.print("Rain (mm/h): ");
    Serial.println(rain);

    Serial.print("Precipitation (mm): ");
    Serial.println(precipitation);
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