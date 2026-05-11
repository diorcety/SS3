#include "tick.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

volatile uint32_t systick_counter = 0;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void tick_timer_init(volatile tick_timer_t *t) { t->armed = false; }

void tick_timer_start(volatile tick_timer_t *t, uint32_t timeout, bool restart) {
  if (!restart && t->armed)
    return;
  t->timeout = timeout;
  t->start = systick_get();
  t->armed = true;
}

uint32_t tick_timer_stop(volatile tick_timer_t *t) {
  t->armed = false;
  return systick_counter - t->start;
}

bool tick_timer_elapsed(volatile tick_timer_t *t) {
  if (!t->armed)
    return false;

  if (systick_elapsed(t->start, t->timeout)) {
    t->armed = false; // disarm → prevents retrigger
    return true;
  }
  return false;
}

bool tick_timer_is_running(volatile tick_timer_t *t, bool update) {
  if (update)
    tick_timer_elapsed(t);
  return t->armed;
}
