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

void timer_init(volatile timer_t *t) { t->armed = false; }

void timer_start(volatile timer_t *t, uint32_t timeout, bool restart) {
  if (!restart && t->armed)
    return;
  t->timeout = timeout;
  t->start = systick_get();
  t->armed = true;
}

uint32_t timer_stop(volatile timer_t *t) {
  t->armed = false;
  return systick_counter - t->start;
}

bool timer_elapsed(volatile timer_t *t) {
  if (!t->armed)
    return false;

  if (systick_elapsed(t->start, t->timeout)) {
    t->armed = false; // disarm → prevents retrigger
    return true;
  }
  return false;
}

bool timer_is_running(volatile timer_t *t, bool update) {
  if (update)
    timer_elapsed(t);
  return t->armed;
}
