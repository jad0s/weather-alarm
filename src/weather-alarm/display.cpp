#include "display.h"
#include <Wire.h>
#include "weather.h"
#include "ui.h"
#include "systemtime.h"
#include "icons.h"


#define SCREEN_TIMEOUT 10000
static unsigned long lastTouch = 0;

bool screenOn = true;
bool loading = true;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initDisplay() {
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);           
    display.setCursor(0,0);
    display.println("Weather Alarm");
    display.display(); 
}

void displayTouched(){
  lastTouch = millis();
  if (!screenOn){
    display.ssd1306_command(SSD1306_DISPLAYON);
    screenOn = true;
    screenDirty = true;
  }
}

void displayUpdate(){
  if (screenOn && (millis() - lastTouch > SCREEN_TIMEOUT)){
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    screenOn = false;
  }
}


