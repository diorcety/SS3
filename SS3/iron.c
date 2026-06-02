#include "iron.h"

#include "acquisition.h"
#include "board.h"
#include "configuration.h"
#include "heat.h"
#include "main.h"
#include "tick.h"
#include "tip.h"
#include "util.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int TIP_CHANGE_WAIT_DELAY = 1000;
static const int IRON_HEAT_BASE = 100;
static const int IRON_DIFF_FACTOR = 5;      // 1° diff == 5%
static const int IRON_DIFF_FACTOR_BASE = 1; // 1° diff == 5%
static const float TC_MAX_VOLTAGE = 1.0f;   // We NORMALLY should not get any voltage on TC
static const int HEAT_SETPOINT_WINDOW = 64; // 0 to 240 reached in 392 samples: about 4 seconds on 50HZ

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

bool right_heat;
bool left_heat;
int right_duty;
int left_duty;

static int left_acc;
static int right_acc;
static tick_timer_t tip_type_change_timer;
static TipType previous_tip_type;
static int heat_setpoint_counter;

static volatile uint32_t current_sw_state;
static volatile uint32_t next_sw_state;
static volatile bool next_set;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static int iron_temperature_to_heat(int temperature) {
  int setpoint = IIR_FILTER_GET(HEAT_SETPOINT_WINDOW, heat_setpoint_counter);
  int temp_diff = MAX(0, setpoint - temperature);
  int max_heat = IRON_HEAT_BASE * max_duty / DUTY_BASE;
  return MIN(temp_diff * IRON_DIFF_FACTOR / IRON_DIFF_FACTOR_BASE, max_heat);
}

static bool iron_can_heat(void) {
  // Do nothing if there is no tip
  if (tip_type == TIP_TYPE_NC) {
    return false;
  }

  // Do nothing in standby or error state
  if (heat_state == HEAT_STATE_ERROR || heat_state == HEAT_STATE_STANDBY) {
    return false;
  }

  // Wait few time (and stabilisation) before heating
  if (tick_timer_is_running(&tip_type_change_timer, true)) {
    return false;
  }

  return true;
}

static bool iron_should_heat(int *pAcc, int duty) {
  *pAcc += duty;

  if (*pAcc < IRON_HEAT_BASE) {
    return false;
  } else {
    *pAcc -= IRON_HEAT_BASE;
    return true;
  }
}

void iron_init(void) {
  left_acc = right_acc = 0;
  previous_tip_type = tip_type;
  heat_setpoint_counter = 0;

  right_heat = left_heat = 0;
  right_duty = left_duty = 0;

  next_sw_state = current_sw_state = 0;
  next_set = false;

  tick_timer_init(&tip_type_change_timer);
}

bool iron_is_standby(void) { return current_sw_state == 0 && next_set && next_sw_state == 0; }

void iron_set_output(uint32_t value) {
  current_sw_state = value;
  DL_GPIO_writePinsVal(Switches_PORT, Switches_R24_PIN | Switches_R12_PIN | Switches_L24_PIN | Switches_L12_PIN, value);
}

void iron_loop(void) {
  NVIC_EnableIRQ(PowerProtection_INT_IRQN);

  if (!new_acquisition) {
    return;
  }

  // Fail-safe: With a 3.5 mm barrel jack, an incorrectly inserted plug can short the tip and ring,
  // causing power to appear on the TC pin. In this case, we immediately shut down the power.
  if (current_sw_state != 0) {
    if (tc_right_voltage >= TC_MAX_VOLTAGE || tc_left_voltage >= TC_MAX_VOLTAGE) {
      iron_set_output(0);
      heat_error();
    }
  }

  // Filter setpoint in order to avoid heat peak
  if (tip_type != TIP_TYPE_NC) {
    IIR_FILTER_ADD(HEAT_SETPOINT_WINDOW, heat_setpoint_counter, heat_setpoint);
  } else {
    IIR_FILTER_ADD(HEAT_SETPOINT_WINDOW, heat_setpoint_counter, kty_temperature);
  }

  // Detect tip_type changes
  if (previous_tip_type != tip_type) {
    previous_tip_type = tip_type;
    tick_timer_start(&tip_type_change_timer, TIP_CHANGE_WAIT_DELAY, true);
  }

  // Check if we can heat
  uint32_t switches = 0;
  if (iron_can_heat()) {
    right_duty = iron_temperature_to_heat(right_temperature);
    left_duty = iron_temperature_to_heat(left_temperature);
    right_heat = iron_should_heat(&right_acc, right_duty);
    left_heat = iron_should_heat(&left_acc, left_duty);

    if (tip_type == TIP_TYPE_WMRP || tip_type == TIP_TYPE_WMRT) {
      if (right_heat) {
        switches |= Switches_R12_PIN;
      }
    } else if (tip_type == TIP_TYPE_WXUP) {
      if (right_heat) {
        switches |= Switches_R24_PIN;
      }
    } else {
      right_duty = 0;
      right_acc = 0;
      right_heat = false;
    }

    if (tip_type == TIP_TYPE_WMRT) {
      if (left_heat) {
        switches |= Switches_L12_PIN;
      }
    } else {
      left_duty = 0;
      left_acc = 0;
      left_heat = false;
    }
  } else {
    right_acc = 0;
    right_duty = 0;
    right_heat = false;

    left_acc = 0;
    left_duty = 0;
    left_heat = false;
  }

  // Will set the switches at next trigger
  next_sw_state = switches;
  next_set = true;
}

void iron_trigger(void) {
  if (!next_set) {
    return;
  }
  next_set = false;

  iron_set_output(next_sw_state);

#ifndef NO_WDG
  DL_WWDT_restart(PowerProtection_INST);
#endif
}

void PowerProtection_IRQHandler(void) {
  if (DL_WWDT_getPendingInterrupt(PowerProtection_INST)) {
    ERROR_HANDLER();
  }
}
