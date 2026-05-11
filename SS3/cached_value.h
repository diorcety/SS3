#ifndef CACHED_VALUE_H_
#define CACHED_VALUE_H_

#include <stdbool.h>
#include <stdint.h>

#include "tick.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

/**
 * CACHED_VALUE_DECL(TYPE, NAME, DEBOUNCE_MS)
 *
 * Declares and zero-initialises a cached value variable named `cv_NAME`.
 * Place at file scope (static) or inside a struct.
 */
#define CACHED_VALUE_DECL(TYPE, NAME, DEBOUNCE_MS)                                                                     \
  struct {                                                                                                             \
    cached_value_base_t base;                                                                                          \
    TYPE value;                                                                                                        \
  } cv_##NAME = {                                                                                                      \
      .base = {.debounce_ms = (DEBOUNCE_MS), .dirty = true},                                                           \
      .value = (TYPE){0},                                                                                              \
  }

/**
 * CACHED_VALUE_SET(NAME, NEW_VAL)
 *
 * Feed a new value.  Marks dirty only when the value actually changes.
 * Uses memcmp so it works for any plain-data type (int, enum, struct …).
 */
#define CACHED_VALUE_SET(NAME, NEW_VAL)                                                                                \
  do {                                                                                                                 \
    __typeof__(cv_##NAME.value) _nv = (NEW_VAL);                                                                       \
    if (cv_##NAME.value != _nv) {                                                                                      \
      cv_##NAME.value = _nv;                                                                                           \
      _cached_value_set(&cv_##NAME.base);                                                                              \
    }                                                                                                                  \
  } while (0)

/**
 * CACHED_VALUE_NEEDS_REDRAW(NAME, FORCE)
 *
 * Returns true when:
 *   - FORCE is true, OR
 *   - the value is dirty AND the debounce period has elapsed.
 */
#define CACHED_VALUE_NEEDS_REDRAW(NAME, FORCE) _cached_value_needs_redraw(&cv_##NAME.base, (FORCE))

/**
 * CACHED_VALUE_ACK(NAME)
 *
 * Call after drawing to clear the dirty flag.
 */
#define CACHED_VALUE_ACK(NAME) _cached_value_ack(&cv_##NAME.base)

/**
 * CACHED_VALUE_GET(NAME)
 *
 * Read the current stored value.
 */
#define CACHED_VALUE_GET(NAME) (cv_##NAME.value)

/**
 * CACHED_VALUE_FORCE_DIRTY(NAME)
 *
 * Mark as dirty without changing the value (e.g. after a full screen clear).
 */
#define CACHED_VALUE_FORCE_DIRTY(NAME)                                                                                 \
  do {                                                                                                                 \
    cv_##NAME.base.dirty = true;                                                                                       \
    cv_##NAME.base.last_set_tick = systick_get();                                                                      \
  } while (0)

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  STRUCTS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

typedef struct {
  uint32_t last_set_tick; /* tick when the value last changed            */
  uint32_t debounce_ms;   /* minimum ms between accepted redraws         */
  bool dirty;             /* value changed and not yet acknowledged      */
} cached_value_base_t;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static inline bool _cached_value_needs_redraw(const cached_value_base_t *b, bool force) {
  if (force)
    return true;
  if (!b->dirty)
    return false;
  if (b->debounce_ms == 0)
    return true;
  return systick_elapsed(b->last_set_tick, b->debounce_ms);
}

static inline void _cached_value_ack(cached_value_base_t *b) { b->dirty = false; }

static inline void _cached_value_set(cached_value_base_t *b) {
  b->dirty = true;
  b->last_set_tick = systick_get();
}

#endif
