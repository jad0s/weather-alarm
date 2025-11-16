#include <wifi.h>

// NTP setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC, 60s update

int hours = 0;
int minutes = 0;

void initTime(){
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