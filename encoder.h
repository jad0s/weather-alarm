#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include "pins.h"

// Simple, single-instance class for rotary encoder + push switch.
class EncoderClass {
public:
  EncoderClass();
  // call once in setup()
  void begin();

  // call frequently from loop() to handle button debouncing
  void tick();

  // Returns signed number of detent steps since last call and clears the internal counter.
  // One detent = one “click” of the encoder (CW -> +1, CCW -> -1).
  int32_t getSteps();

  // Returns true once if the button was pressed since last call.
  bool wasClicked();

  // Total position (not cleared)
  int32_t getPosition();

private:
  // ISR (must be static)
  static void IRAM_ATTR _isr();

  // Quadrature transition table (oldState<<2 | newState) -> -1/0/+1
  static const int8_t _ttable[16];

  // ISR-shared state (volatile)
  static volatile int8_t  _accumulator;   // accumulates quarter transitions
  static volatile int32_t _queuedSteps;   // full-detent steps ready to be read
  static volatile int32_t _position;      // running total position
  static volatile uint8_t _prevState;     // previous AB state (2 bits)

  // Button debounce state (instance members)
  uint32_t _btnLastDebounce = 0;
  uint8_t  _btnLastReading = HIGH;
  uint8_t  _btnStableState = HIGH;

  // Flag set when button pressed; accessed from tick() and main
  static volatile bool _btnPressedFlag;

  // Debounce time (ms)
  static const uint16_t BUTTON_DEBOUNCE_MS;
};

extern EncoderClass encoder;

#endif // ENCODER_H
