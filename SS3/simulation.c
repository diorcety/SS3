#include "simulation.h"

#include <pthread.h>
#include <time.h>

extern void SysTick_Handler(void);

static pthread_t _systick_thread;

static void *_systick_loop(void *arg) {
  (void)arg;
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000000L}; /* 1 ms */
  for (;;) {
    nanosleep(&ts, NULL);
    SysTick_Handler();
  }
  return NULL;
}

void SYSCFG_DL_init(void) { pthread_create(&_systick_thread, NULL, _systick_loop, NULL); }
