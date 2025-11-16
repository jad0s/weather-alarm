#include "wifi.h"

// Your Wi-Fi credentials
const char* ssid = "HomeNet 402";
const char* password = "citroen2";



void initWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
}


