#ifndef ARDUINO_H__
#define ARDUINO_H__

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

// Implementations
#ifndef ERROR_HANDLER
#if !defined(SIMULATION)
extern "C" void error_handler(void);
#define ERROR_HANDLER() error_handler()
#else
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#define ERROR_HANDLER() abort()
#ifdef __cplusplus
}
#endif
#endif
#endif

#ifndef CORU_DISABLED
extern "C" void coru_yield(void);
static inline void yield() { coru_yield(); }
#else
static inline void yield() {}
#endif
extern "C" {
static inline void delay(int delay) {
  extern volatile uint32_t systick_counter;
  uint32_t start = systick_counter;
  while ((systick_counter - start) < delay) {
    yield();
  }
}
}
// Stubs
#define printf(...)
#define __FlashStringHelper uint8_t
#define String std::string
static inline void pinMode(int a, int b) { ERROR_HANDLER(); }
static inline void digitalWrite(int a, int b) { ERROR_HANDLER(); }
#define OUTPUT 1
#define HIGH 1
#define LOW 0
#define INPUT 0

#endif // ARDUINO_H__
