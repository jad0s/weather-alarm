#ifndef WIFI_H
#define WIFI_H

#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Initialize Wi-Fi and NTP
void initWiFi();

// Call every loop to update time
void updateTime();

// Get current hours and minutes
int getHours();
int getMinutes();

#endif
