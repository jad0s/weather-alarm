#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define TOUCH_PIN 13   // OUT pin of TTP223
#define BUZZER_PIN 12  // passive buzzer
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool buzzing = false;       // persist across loops
bool lastState = LOW;       // to detect edges

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);  // or INPUT_PULLDOWN if needed
  Wire.begin(8, 9);  // SDA, SCL

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(115200);
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hello!");
  display.display();
}

void loop() {
  bool state = digitalRead(TOUCH_PIN);
  display.clearDisplay();
  // Detect rising edge (LOW -> HIGH)
  if (state == HIGH && lastState == LOW) {
    buzzing = !buzzing;  // toggle
    if (buzzing) {
      tone(BUZZER_PIN, 262);   // play tone continuously
      Serial.println("Buzzer ON");
      display.setCursor(0, 0);
      display.println("Touch!");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.display();
    } else {
      noTone(BUZZER_PIN);      // stop
      Serial.println("Buzzer OFF");
    }
  }

  lastState = state;
}
