#ifndef TICK_H_
#define TICK_H_

#include <stdbool.h>
#include <stdint.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  STRUCTS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

typedef struct timer {
  uint32_t start;
  uint32_t timeout;
  bool armed;
} timer_t;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

extern volatile uint32_t systick_counter;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static inline void systick_inc(void) { systick_counter++; }

static inline uint32_t systick_get(void) { return systick_counter; }

static inline bool systick_elapsed(uint32_t start, uint32_t timeout) { return (systick_counter - start) >= timeout; }

void timer_init(volatile timer_t *t);
void timer_start(volatile timer_t *t, uint32_t timeout, bool restart);
uint32_t timer_stop(volatile timer_t *t);
bool timer_elapsed(volatile timer_t *t);
bool timer_is_running(volatile timer_t *t, bool update);

#endif // UTILS_H_
