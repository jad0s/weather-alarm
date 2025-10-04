#include "encoder.h"
#include <driver/gpio.h> // gpio_get_level

// transition table: index = (old<<2) | new
// values are -1, 0 or +1 for valid quarter-steps
const int8_t EncoderClass::_ttable[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

// static volatile definitions
volatile int8_t  EncoderClass::_accumulator = 0;
volatile int32_t EncoderClass::_queuedSteps = 0;
volatile int32_t EncoderClass::_position = 0;
volatile uint8_t EncoderClass::_prevState = 0;
volatile bool    EncoderClass::_btnPressedFlag = false;
const uint16_t   EncoderClass::BUTTON_DEBOUNCE_MS = 50;

EncoderClass encoder;

EncoderClass::EncoderClass() {}

void EncoderClass::begin() {
  // pins.h must define ENCODER_CLK, ENCODER_DT, ENCODER_SW
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT,  INPUT_PULLUP);
  pinMode(ENCODER_SW,  INPUT_PULLUP);

  // read initial AB state (use gpio_get_level for ISR safety)
  uint8_t s = (gpio_get_level((gpio_num_t)ENCODER_CLK) << 1)
            |  gpio_get_level((gpio_num_t)ENCODER_DT);
  _prevState = s;

  // attach same ISR to both pins so we catch every transition
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), _isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT),  _isr, CHANGE);
}

// Small, fast ISR: read AB and update accumulator/queued steps.
// Placed in IRAM to be safe on ESP32.
void IRAM_ATTR EncoderClass::_isr() {
  // read both pins atomically (gpio_get_level is safe in ISR)
  uint8_t s = (gpio_get_level((gpio_num_t)ENCODER_CLK) << 1)
            |  gpio_get_level((gpio_num_t)ENCODER_DT);

  uint8_t idx = (_prevState << 2) | s;
  int8_t v = _ttable[idx];
  if (v) {
    _accumulator += v;
    // Many mechanical encoders produce 4 valid transitions per detent.
    // Only create a full step when accumulator reaches +/-4. This filters bounce.
    while (_accumulator >= 2) {
      _queuedSteps++;
      _position++;
      _accumulator -= 2;
    }
    while (_accumulator <= -2) {
      _queuedSteps--;
      _position--;
      _accumulator += 2;
    }
  }
  _prevState = s;
}

// Non-blocking button debounce: call from loop() often.
void EncoderClass::tick() {
  uint32_t now = millis();
  uint8_t reading = digitalRead(ENCODER_SW);

  if (reading != _btnLastReading) {
    _btnLastDebounce = now;
    _btnLastReading = reading;
  }

  if ((now - _btnLastDebounce) > BUTTON_DEBOUNCE_MS) {
    if (reading != _btnStableState) {
      _btnStableState = reading;
      // assuming wiring: INPUT_PULLUP, pressed = LOW
      if (reading == LOW) {
        _btnPressedFlag = true;
      }
    }
  }
}

// return and clear queued steps atomically
int32_t EncoderClass::getSteps() {
  noInterrupts();
  int32_t s = _queuedSteps;
  _queuedSteps = 0;
  interrupts();
  return s;
}

bool EncoderClass::wasClicked() {
  if (_btnPressedFlag) {
    _btnPressedFlag = false;
    return true;
  }
  return false;
}

int32_t EncoderClass::getPosition() {
  noInterrupts();
  int32_t p = _position;
  interrupts();
  return p;
}
