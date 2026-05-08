#ifndef ARDUINO_H__
#define ARDUINO_H__

#include <stddef.h>
#include <stdint.h>
#include <string>

#include "coru.h"
#ifndef ERROR_HANDLER
#define ERROR_HANDLER() error_handler()

extern "C" void error_handler(void);
#endif

#define printf(...)
#if 0
static inline void yield() { coru_yield(); }
#else
static inline void yield() { }
#endif
static inline void delay(int delay) {

  extern volatile uint32_t systick_counter;

  uint32_t start = systick_counter;
  while ((systick_counter - start) < delay) {
    yield();
  }
}
#define __FlashStringHelper uint8_t
#define String std::string
static inline void pinMode(int a, int b) { ERROR_HANDLER(); }
static inline void digitalWrite(int a, int b) { ERROR_HANDLER(); }
#define OUTPUT 1
#define HIGH 1
#define LOW 0
#define INPUT 0

#endif // ARDUINO_H__