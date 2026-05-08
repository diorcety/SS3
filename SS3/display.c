#include "display.h"

#include "configuration.h"
#include "heat.h"
#include "tip.h"
#include "util.h"

#include <stdlib.h>

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
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

DisplayState display_state;
MainMenu main_menu;
DiagMenu diag_menu;

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

void display_init(void) {
  display_state = DISPLAY_STATE_MAIN;
  main_menu = MAIN_MENU_BACK;
  diag_menu = DIAG_MENU_BACK;
}

static void display_state_event(Event event) {
  const EventHandler event_handler = state_table[display_state];

  if (event_handler != NULL) {
    display_state = event_handler(event);
  }
}

void display_loop(void) { display_state_event(button_get_event()); }
