#include "display.h"

#include "acquisition.h"
#include "configuration.h"
#include "iron.h"
#include "util.h"
#include "zcd.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define ST_FCT_LOOP(fct, name, variable, step, parent)                                                                 \
  static DisplayState fct(Event event) {                                                                               \
    switch (event) {                                                                                                   \
    case EVENT_PRESS:                                                                                                  \
    case EVENT_LONG_PRESS:                                                                                             \
      return st_display_change(parent);                                                                                \
    case EVENT_LEFT:                                                                                                   \
      if (variable < name##_MIN + step)                                                                                \
        variable = name##_MAX - (name##_MIN + step - variable - 1);                                                    \
      else                                                                                                             \
        variable -= step;                                                                                              \
      break;                                                                                                           \
    case EVENT_RIGHT:                                                                                                  \
      if (variable > name##_MAX - step)                                                                                \
        variable = name##_MIN + (variable - (name##_MAX - step) - 1);                                                  \
      else                                                                                                             \
        variable += step;                                                                                              \
      break;                                                                                                           \
    default:                                                                                                           \
      break;                                                                                                           \
    }                                                                                                                  \
    return display_state;                                                                                              \
  }

#define ST_FCT_RANGE(fct, name, variable, step, parent)                                                                \
  static DisplayState fct(Event event) {                                                                               \
    switch (event) {                                                                                                   \
    case EVENT_PRESS:                                                                                                  \
    case EVENT_LONG_PRESS:                                                                                             \
      return st_display_change(parent);                                                                                \
    case EVENT_LEFT:                                                                                                   \
      if (variable < name##_MIN + step)                                                                                \
        variable = name##_MIN;                                                                                         \
      else                                                                                                             \
        variable -= step;                                                                                              \
      break;                                                                                                           \
    case EVENT_RIGHT:                                                                                                  \
      if (variable > name##_MAX - step)                                                                                \
        variable = name##_MAX;                                                                                         \
      else                                                                                                             \
        variable += step;                                                                                              \
      break;                                                                                                           \
    default:                                                                                                           \
      break;                                                                                                           \
    }                                                                                                                  \
    return display_state;                                                                                              \
  }

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  STRUCTS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

typedef DisplayState (*EventHandler)(Event);

static const DisplayState main_menu_button_table[] = {
    [MAIN_MENU_BACK] = DISPLAY_STATE_MAIN,
    [MAIN_MENU_SETBACK] = DISPLAY_STATE_ADJ_SETBACK,
    [MAIN_MENU_SETBACK_DELAY] = DISPLAY_STATE_ADJ_SETBACK_DELAY,
    [MAIN_MENU_STANDBY_DELAY] = DISPLAY_STATE_ADJ_STANDBY_DELAY,
    [MAIN_MENU_OFFSET] = DISPLAY_STATE_ADJ_OFFSET,
    [MAIN_MENU_UNIT] = DISPLAY_STATE_ADJ_UNIT,
    [MAIN_MENU_STEP_SIZE] = DISPLAY_STATE_ADJ_STEP_SIZE,
    [MAIN_MENU_DIAG] = DISPLAY_STATE_DIAG_MENU,
};

static const DisplayState diag_menu_button_table[] = {
    [DIAG_MENU_BACK] = DISPLAY_STATE_MAIN_MENU,
    [DIAG_MENU_COLD_COMPENSATION] = DISPLAY_STATE_SHOW_COLD_COMPENSATION,
    [DIAG_MENU_REFERENCE] = DISPLAY_STATE_ADJ_REFERENCE,
    [DIAG_MENU_TIP_TYPE] = DISPLAY_STATE_SHOW_TIP_TYPE,
    [DIAG_MENU_REED_STATE] = DISPLAY_STATE_SHOW_REED_STATE,
    [DIAG_MENU_SHOW_FREQUENCY] = DISPLAY_STATE_SHOW_FREQUENCY,
    [DIAG_MENU_TC_1_READING] = DISPLAY_STATE_SHOW_TC_1_READING,
    [DIAG_MENU_TC_2_READING] = DISPLAY_STATE_SHOW_TC_2_READING,
    [DIAG_MENU_PWM_1_READING] = DISPLAY_STATE_SHOW_PWM_1_READING,
    [DIAG_MENU_PWM_2_READING] = DISPLAY_STATE_SHOW_PWM_2_READING,
    [DIAG_MENU_IDLE_DUTY] = DISPLAY_STATE_ADJ_IDLE_DUTY,
    [DIAG_MENU_MAX_DUTY] = DISPLAY_STATE_ADJ_MAX_DUTY,
    [DIAG_MENU_POOR] = DISPLAY_STATE_ADJ_POOR,
    [DIAG_MENU_FW_VERSION] = DISPLAY_STATE_SHOW_FW_VERSION,
};

static DisplayState st_main_event(Event event);

static DisplayState st_main_menu_event(Event event);
static DisplayState st_adj_setback_event(Event event);
static DisplayState st_adj_setback_delay_event(Event event);
static DisplayState st_adj_standby_delay_event(Event event);
static DisplayState st_adj_offset_event(Event event);
static DisplayState st_adj_unit_event(Event event);
static DisplayState st_adj_step_size_event(Event event);

static DisplayState st_diag_menu_event(Event event);

static DisplayState st_diag_menu_show_event(Event event);
static DisplayState st_adj_reference_event(Event event);
static DisplayState st_adj_idle_duty_event(Event event);
static DisplayState st_adj_max_duty_event(Event event);
static DisplayState st_adj_poor_event(Event event);

static const EventHandler state_table[] = {
    [DISPLAY_STATE_MAIN] = st_main_event,

    [DISPLAY_STATE_MAIN_MENU] = st_main_menu_event,
    [DISPLAY_STATE_ADJ_SETBACK] = st_adj_setback_event,
    [DISPLAY_STATE_ADJ_SETBACK_DELAY] = st_adj_setback_delay_event,
    [DISPLAY_STATE_ADJ_STANDBY_DELAY] = st_adj_standby_delay_event,
    [DISPLAY_STATE_ADJ_OFFSET] = st_adj_offset_event,
    [DISPLAY_STATE_ADJ_UNIT] = st_adj_unit_event,
    [DISPLAY_STATE_ADJ_STEP_SIZE] = st_adj_step_size_event,

    [DISPLAY_STATE_DIAG_MENU] = st_diag_menu_event,
    [DISPLAY_STATE_SHOW_COLD_COMPENSATION] = st_diag_menu_show_event,
    [DISPLAY_STATE_ADJ_REFERENCE] = st_adj_reference_event,
    [DISPLAY_STATE_SHOW_TIP_TYPE] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_REED_STATE] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_FREQUENCY] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_TC_1_READING] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_TC_2_READING] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_PWM_1_READING] = st_diag_menu_show_event,
    [DISPLAY_STATE_SHOW_PWM_2_READING] = st_diag_menu_show_event,
    [DISPLAY_STATE_ADJ_IDLE_DUTY] = st_adj_idle_duty_event,
    [DISPLAY_STATE_ADJ_MAX_DUTY] = st_adj_max_duty_event,
    [DISPLAY_STATE_ADJ_POOR] = st_adj_poor_event,
    [DISPLAY_STATE_SHOW_FW_VERSION] = st_diag_menu_show_event,
};

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int DISPLAY_MAIN_PERIOD_IIR_WINDOW = 8;
static const int DISPLAY_LEFT_TEMPERATURE_IIR_WINDOW = 32;
static const int DISPLAY_RIGHT_TEMPERATURE_IIR_WINDOW = 32;

static const int DISPLAY_TEMPERATURE_SLOW_UPDATE_MS = 1000;
static const int DISPLAY_TEMPERATURE_SLOW_THRESHOLD_DEG = 2;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

DisplayState display_state;
MainMenu main_menu;
DiagMenu diag_menu;

// Direct update: no debounce
CACHED_VALUE_DEF(DisplayState, display_state, 0);
CACHED_VALUE_DEF(TipType, tip_type, 0);
CACHED_VALUE_DEF(ReedState, reed_state, 0);
CACHED_VALUE_DEF(HeatState, heat_state, 0);
CACHED_VALUE_DEF(int, heat_setpoint, 0);
CACHED_VALUE_DEF(int, setback, 0);
CACHED_VALUE_DEF(int, setback_delay, 0);
CACHED_VALUE_DEF(int, standby_delay, 0);
CACHED_VALUE_DEF(int, temperature_offset, 0);
CACHED_VALUE_DEF(int, temperature_unit, 0);
CACHED_VALUE_DEF(int, step_size, 0);
CACHED_VALUE_DEF(int, kty_value, 0);
CACHED_VALUE_DEF(int, reference, 0);
CACHED_VALUE_DEF(int, idle_duty, 0);
CACHED_VALUE_DEF(int, max_duty, 0);
CACHED_VALUE_DEF(int, poor_mode, 0);

// Frequent updates: debounce the input
CACHED_VALUE_DEF(int, right_duty, 200);
CACHED_VALUE_DEF(int, left_duty, 200);
CACHED_VALUE_DEF(int, right_temperature, 200);
CACHED_VALUE_DEF(int, left_temperature, 200);
CACHED_VALUE_DEF(int, tc_right_temperature, 200);
CACHED_VALUE_DEF(int, tc_left_temperature, 200);
CACHED_VALUE_DEF(int, main_period, 200);

// Menu cache
CACHED_VALUE_DEF(MainMenu, main_menu, 0);
CACHED_VALUE_DEF(DiagMenu, diag_menu, 0);

static int main_period_acc;
static int right_temperature_acc;
static int right_temperature_filtered;
static int left_temperature_acc;
static int left_temperature_filtered;
static uint32_t previous_temperature_update;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static DisplayState st_display_change(DisplayState state) {
  // Save params when going back to main
  if (state == DISPLAY_STATE_MAIN) {
    configuration_save();
  }
  return state;
}

static DisplayState st_main_event(Event event) {
  switch (event) {
  case EVENT_PRESS:
    heat_toggle();
    break;
  case EVENT_LONG_PRESS:
    return st_display_change(DISPLAY_STATE_MAIN_MENU);
  case EVENT_LEFT:
    if (heat_state == HEAT_STATE_NORMAL) {
      heat_setpoint = setpoint = MAX(setpoint - step_size, SETPOINT_MIN);
      configuration_save();
    }
    break;
  case EVENT_RIGHT:
    if (heat_state == HEAT_STATE_NORMAL) {
      heat_setpoint = setpoint = MIN(setpoint + step_size, SETPOINT_MAX);
      configuration_save();
    }
    break;
  default:
    break;
  }
  return display_state;
}

ST_FCT_LOOP(st_main_menu_event, MAIN_MENU, main_menu, 1, main_menu_button_table[main_menu])
ST_FCT_LOOP(st_diag_menu_event, DIAG_MENU, diag_menu, 1, diag_menu_button_table[diag_menu])

static DisplayState st_diag_menu_show_event(Event event) {
  switch (event) {
  case EVENT_PRESS:
  case EVENT_LONG_PRESS:
    return st_display_change(DISPLAY_STATE_DIAG_MENU);
  default:
    break;
  }
  return display_state;
}

ST_FCT_RANGE(st_adj_setback_delay_event, SETBACK_DELAY, setback_delay, 10, DISPLAY_STATE_MAIN_MENU)
ST_FCT_RANGE(st_adj_standby_delay_event, STANDBY_DELAY, standby_delay, 10, DISPLAY_STATE_MAIN_MENU)
ST_FCT_RANGE(st_adj_offset_event, TEMPERATURE_OFFSET, temperature_offset, step_size, DISPLAY_STATE_MAIN_MENU)
ST_FCT_RANGE(st_adj_setback_event, SETBACK, setback, step_size, DISPLAY_STATE_MAIN_MENU)
ST_FCT_RANGE(st_adj_step_size_event, STEP_SIZE, step_size, 1, DISPLAY_STATE_MAIN_MENU)
ST_FCT_LOOP(st_adj_unit_event, TEMPERATURE_UNIT, temperature_unit, 1, DISPLAY_STATE_MAIN_MENU)

ST_FCT_RANGE(st_adj_reference_event, REFERENCE, reference, 1, DISPLAY_STATE_DIAG_MENU)
ST_FCT_RANGE(st_adj_idle_duty_event, IDLE_DUTY, idle_duty, 1, DISPLAY_STATE_DIAG_MENU)
ST_FCT_RANGE(st_adj_max_duty_event, MAX_DUTY, max_duty, 1, DISPLAY_STATE_DIAG_MENU)
ST_FCT_LOOP(st_adj_poor_event, POOR_MODE, poor_mode, 1, DISPLAY_STATE_DIAG_MENU)

static void init_cached_values(void) {
  /* SET once to capture the current live value and mark dirty=true */
  CACHED_VALUE_SET(display_state, display_state);
  CACHED_VALUE_SET(tip_type, tip_type);
  CACHED_VALUE_SET(reed_state, reed_state);
  CACHED_VALUE_SET(heat_state, heat_state);
  CACHED_VALUE_SET(right_duty, right_duty);
  CACHED_VALUE_SET(left_duty, left_duty);
  CACHED_VALUE_SET(right_temperature, right_temperature);
  CACHED_VALUE_SET(left_temperature, left_temperature);
  CACHED_VALUE_SET(heat_setpoint, heat_setpoint);
  CACHED_VALUE_SET(setback, setback);
  CACHED_VALUE_SET(setback_delay, setback_delay);
  CACHED_VALUE_SET(standby_delay, standby_delay);
  CACHED_VALUE_SET(temperature_offset, temperature_offset);
  CACHED_VALUE_SET(temperature_unit, temperature_unit);
  CACHED_VALUE_SET(step_size, step_size);
  CACHED_VALUE_SET(kty_value, kty_value);
  CACHED_VALUE_SET(reference, reference);
  CACHED_VALUE_SET(tc_right_temperature, tc_right_temperature);
  CACHED_VALUE_SET(tc_left_temperature, tc_left_temperature);
  CACHED_VALUE_SET(idle_duty, idle_duty);
  CACHED_VALUE_SET(max_duty, max_duty);
  CACHED_VALUE_SET(poor_mode, poor_mode);
  CACHED_VALUE_SET(main_period, main_period);
  CACHED_VALUE_SET(main_menu, main_menu);
  CACHED_VALUE_SET(diag_menu, diag_menu);
}

void display_invalidate_cached_values(void) {
  /* Force dirty so the very first paint always runs */
  CACHED_VALUE_FORCE_DIRTY(display_state);
  CACHED_VALUE_FORCE_DIRTY(tip_type);
  CACHED_VALUE_FORCE_DIRTY(reed_state);
  CACHED_VALUE_FORCE_DIRTY(heat_state);
  CACHED_VALUE_FORCE_DIRTY(right_duty);
  CACHED_VALUE_FORCE_DIRTY(left_duty);
  CACHED_VALUE_FORCE_DIRTY(right_temperature);
  CACHED_VALUE_FORCE_DIRTY(left_temperature);
  CACHED_VALUE_FORCE_DIRTY(heat_setpoint);
  CACHED_VALUE_FORCE_DIRTY(setback);
  CACHED_VALUE_FORCE_DIRTY(setback_delay);
  CACHED_VALUE_FORCE_DIRTY(standby_delay);
  CACHED_VALUE_FORCE_DIRTY(temperature_offset);
  CACHED_VALUE_FORCE_DIRTY(temperature_unit);
  CACHED_VALUE_FORCE_DIRTY(step_size);
  CACHED_VALUE_FORCE_DIRTY(kty_value);
  CACHED_VALUE_FORCE_DIRTY(reference);
  CACHED_VALUE_FORCE_DIRTY(tc_right_temperature);
  CACHED_VALUE_FORCE_DIRTY(tc_left_temperature);
  CACHED_VALUE_FORCE_DIRTY(idle_duty);
  CACHED_VALUE_FORCE_DIRTY(max_duty);
  CACHED_VALUE_FORCE_DIRTY(poor_mode);
  CACHED_VALUE_FORCE_DIRTY(main_period);
  CACHED_VALUE_FORCE_DIRTY(main_menu);
  CACHED_VALUE_FORCE_DIRTY(diag_menu);
}

void display_init(void) {
  display_state = DISPLAY_STATE_MAIN;
  main_menu = MAIN_MENU_BACK;
  diag_menu = DIAG_MENU_BACK;

  init_cached_values();
  display_invalidate_cached_values();

  main_period_acc = 0;
  right_temperature_acc = left_temperature_acc = 0;
  right_temperature_filtered = 0;
  left_temperature_filtered = 0;

  previous_temperature_update = systick_get();
}

static void display_state_event(Event event) {
  const EventHandler event_handler = state_table[display_state];

  if (event_handler != NULL) {
    display_state = event_handler(event);
  }
}

void display_loop(void) {
  if (new_acquisition) {
    IIR_FILTER_ADD(DISPLAY_MAIN_PERIOD_IIR_WINDOW, main_period_acc, main_period);
    int main_period_acc_filtered = IIR_FILTER_GET(DISPLAY_MAIN_PERIOD_IIR_WINDOW, main_period_acc);

    if (tip_has_right()) {
      IIR_FILTER_ADD(DISPLAY_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc, right_temperature);
    } else {
      IIR_FILTER_ADD(DISPLAY_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc, kty_temperature);
    }
    if (tip_has_left()) {
      IIR_FILTER_ADD(DISPLAY_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc, left_temperature);
    } else {
      IIR_FILTER_ADD(DISPLAY_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc, kty_temperature);
    }

    // Get filtered value
    int tmp_right_temperature_filtered = IIR_FILTER_GET(DISPLAY_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc);
    int tmp_left_temperature_filtered = IIR_FILTER_GET(DISPLAY_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc);

    // Only update above threshold or after some delay: avoid flikering
    if (ABS(tmp_right_temperature_filtered - right_temperature_filtered) >= DISPLAY_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        ABS(tmp_left_temperature_filtered - left_temperature_filtered) >= DISPLAY_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        systick_elapsed(previous_temperature_update, DISPLAY_TEMPERATURE_SLOW_UPDATE_MS)) {
      right_temperature_filtered = tmp_right_temperature_filtered;
      left_temperature_filtered = tmp_left_temperature_filtered;
      previous_temperature_update = systick_get();
    }

    CACHED_VALUE_SET(display_state, display_state);
    CACHED_VALUE_SET(main_menu, main_menu);
    CACHED_VALUE_SET(diag_menu, diag_menu);

    CACHED_VALUE_SET(tip_type, tip_type);
    CACHED_VALUE_SET(reed_state, reed_state);
    CACHED_VALUE_SET(heat_state, heat_state);
    CACHED_VALUE_SET(right_duty, right_duty);
    CACHED_VALUE_SET(left_duty, left_duty);
    CACHED_VALUE_SET(right_temperature, right_temperature_filtered);
    CACHED_VALUE_SET(left_temperature, left_temperature_filtered);
    CACHED_VALUE_SET(heat_setpoint, heat_setpoint);
    CACHED_VALUE_SET(setback, setback);
    CACHED_VALUE_SET(setback_delay, setback_delay);
    CACHED_VALUE_SET(standby_delay, standby_delay);
    CACHED_VALUE_SET(temperature_offset, temperature_offset);
    CACHED_VALUE_SET(temperature_unit, temperature_unit);
    CACHED_VALUE_SET(step_size, step_size);
    CACHED_VALUE_SET(kty_value, (int)kty_value);
    CACHED_VALUE_SET(reference, reference);
    CACHED_VALUE_SET(tc_right_temperature, (int)tc_right_temperature);
    CACHED_VALUE_SET(tc_left_temperature, (int)tc_left_temperature);
    CACHED_VALUE_SET(idle_duty, idle_duty);
    CACHED_VALUE_SET(max_duty, max_duty);
    CACHED_VALUE_SET(poor_mode, poor_mode);
    CACHED_VALUE_SET(main_period, main_period_acc_filtered);
  }

  display_state_event(button_get_event());
}
