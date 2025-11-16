#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"

extern Adafruit_SSD1306 display;
extern bool screenOn;

void initDisplay();
void displayLogo();
void updateDisplayTime(int hours, int minutes);
void displayTouched();
void displayUpdate();
