#include "wifi.h"

// Your Wi-Fi credentials
const char* ssid = "HomeNet 402";
const char* password = "citroen2";

// NTP setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC, 60s update

void initWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
    timeClient.begin();
    timeClient.update();
}

void updateTime() {
    timeClient.update();
}

int getHours() {
    return timeClient.getHours();
}

int getMinutes() {
    return timeClient.getMinutes();
}
