#include "display.h"
#include <Wire.h>
#include "weather.h"
#include "ui.h"
#include "systemtime.h"
#include "icons.h"


#define SCREEN_TIMEOUT 5000
static unsigned long lastTouch=0;

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

void displayLogo() {
  unsigned long millis_last;
  while(loading == true){
    static int loading_phase = 0;
    loading_phase = (++loading_phase % 3);
    if(millis() - millis_last >= 200){
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(2);           
      display.setCursor(0,0);
      switch(loading_phase){
        case 0:
          display.drawBitmap(0, 0, loading_0, 128, 64, 1);
        case 1:
          display.drawBitmap(0, 0, loading_1, 128, 64, 1);
        case 2:
          display.drawBitmap(0, 0, loading_2, 128, 64, 1);
        case 3:
          display.drawBitmap(0, 0, loading_3, 128, 64, 1);
      }
      
      display.display(); 
    }
    millis_last = millis();
    
    
  }
    
}

/*void updateDisplayTime() {
  if(!screenOn){
    return;
  }
    display.clearDisplay();           // clear previous time
    display.setTextSize(3);
    display.setCursor(0, 0);

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
    display.println(buf);
    display.setTextSize(2);
    display.setCursor(0, 30);         // adjust vertical position

    if(weatherValid){
        char weatherBuf[32];
        snprintf(weatherBuf, sizeof(weatherBuf),
                 "%.1fC \n%.1fmm/h",
                 cachedTemp, cachedRain);
        display.println(weatherBuf);
    } else {
        display.println("Weather N/A");
    }
    display.display();
}*/

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


