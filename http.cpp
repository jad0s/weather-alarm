#include <HTTPClient.h>
#include <ArduinoJson.h>

const float LAT = 49.686956;
const float LON = 18.351402;

void fetchWeather(){
  HTTPClient http;

  String url = "https://api.open-meteo.com/v1/forecast?latitude=" +
               String(LAT) + "&longitude=" + String(LON) +
               "&current=temperature_2m,precipitation,rain";

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
  }

  http.end();
}