#include "heat.h"

#include "configuration.h"
#include "iron.h"
#include "tick.h"
#include "tip.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int HEAT_NC_ERROR_TIMEOUT = 2000;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

HeatState heat_state;
int heat_setpoint;

static uint32_t idle_timestamp;
static TipType previous_tip_type;
static ReedState previous_reed_state;
static HeatState previous_heat_state;
static timer_t nc_timer;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void heat_init(void) {
  previous_tip_type = tip_type;
  previous_reed_state = reed_state;

  timer_init(&nc_timer);

  // Start in normal heat state
  previous_heat_state = heat_state = HEAT_STATE_NORMAL;
  heat_setpoint = setpoint;
}

void heat_loop(void) {
  if (reed_state == REED_STATE_OPENED && (left_duty >= idle_duty || right_duty >= idle_duty)) {
    idle_timestamp = systick_get();
  }

  if (tip_type != TIP_TYPE_NC) {
    timer_start(&nc_timer, HEAT_NC_ERROR_TIMEOUT, true);
  }

  if (heat_state != HEAT_STATE_ERROR) {
    if (setback_delay > 0 && heat_state < HEAT_STATE_SETBACK && systick_elapsed(idle_timestamp, setback_delay * 1000)) {
      heat_state = HEAT_STATE_SETBACK;
    } else if (standby_delay > 0 && heat_state < HEAT_STATE_STANDBY &&
               systick_elapsed(idle_timestamp, standby_delay * 1000)) {
      heat_state = HEAT_STATE_STANDBY;
    }

    if (previous_reed_state != reed_state) {
      if (reed_state == REED_STATE_OPENED) {
        heat_state = HEAT_STATE_NORMAL;
      }
    }
    if (previous_tip_type != tip_type) {
      if (tip_type != TIP_TYPE_NC) {
        heat_state = HEAT_STATE_NORMAL;
      }
    }
  } else {
    // In case of error, ensure that the user completly unplug the tip before retrying anything
    if (timer_elapsed(&nc_timer)) {
      heat_state = HEAT_STATE_STANDBY;
    }
  }

  // Process heat state changes
  if (previous_heat_state != heat_state) {
    switch (heat_state) {
    case HEAT_STATE_NORMAL:
      idle_timestamp = systick_get();
      heat_setpoint = setpoint;
      break;
    case HEAT_STATE_SETBACK:
      if (setback < heat_setpoint) {
        heat_setpoint = setback;
      }
      break;
    case HEAT_STATE_ERROR:
    case HEAT_STATE_STANDBY:
      heat_setpoint = 0;
      break;
    }
  }

  previous_tip_type = tip_type;
  previous_reed_state = reed_state;
  previous_heat_state = heat_state;
}

void heat_toggle(void) {
  idle_timestamp = systick_get();
  if (heat_state == HEAT_STATE_NORMAL) {
    heat_state = HEAT_STATE_STANDBY;
  } else if (heat_state == HEAT_STATE_STANDBY) {
    heat_state = HEAT_STATE_NORMAL;
  }
}

void heat_error(void) { heat_state = HEAT_STATE_ERROR; }