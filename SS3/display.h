#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "button.h"
#include "tip.h"
#include "heat.h"
#include "cached_value.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

/* States */
typedef enum {
  DISPLAY_STATE_MAIN,

  /* Menu */
  DISPLAY_STATE_MAIN_MENU,
  DISPLAY_STATE_ADJ_SETBACK,
  DISPLAY_STATE_ADJ_SETBACK_DELAY,
  DISPLAY_STATE_ADJ_STANDBY_DELAY,
  DISPLAY_STATE_ADJ_OFFSET,
  DISPLAY_STATE_ADJ_UNIT,
  DISPLAY_STATE_ADJ_STEP_SIZE,

  /* Menu Diag */
  DISPLAY_STATE_DIAG_MENU,
  DISPLAY_STATE_SHOW_COLD_COMPENSATION,
  DISPLAY_STATE_ADJ_REFERENCE,
  DISPLAY_STATE_SHOW_TIP_TYPE,
  DISPLAY_STATE_SHOW_REED_STATE,
  DISPLAY_STATE_SHOW_FREQUENCY,
  DISPLAY_STATE_SHOW_TC_1_READING,
  DISPLAY_STATE_SHOW_TC_2_READING,
  DISPLAY_STATE_SHOW_PWM_1_READING,
  DISPLAY_STATE_SHOW_PWM_2_READING,
  DISPLAY_STATE_ADJ_IDLE_DUTY,
  DISPLAY_STATE_ADJ_MAX_DUTY,
  DISPLAY_STATE_ADJ_POOR,
  DISPLAY_STATE_SHOW_FW_VERSION,
} DisplayState;

typedef enum {
  MAIN_MENU_BACK,
  MAIN_MENU_SETBACK,
  MAIN_MENU_SETBACK_DELAY,
  MAIN_MENU_STANDBY_DELAY,
  MAIN_MENU_OFFSET,
  MAIN_MENU_UNIT,
  MAIN_MENU_STEP_SIZE,
  MAIN_MENU_DIAG,
} MainMenu;

static const MainMenu MAIN_MENU_MIN = MAIN_MENU_BACK;
static const MainMenu MAIN_MENU_MAX = MAIN_MENU_DIAG;

typedef enum {
  DIAG_MENU_BACK,
  DIAG_MENU_COLD_COMPENSATION,
  DIAG_MENU_REFERENCE,
  DIAG_MENU_TIP_TYPE,
  DIAG_MENU_REED_STATE,
  DIAG_MENU_SHOW_FREQUENCY,
  DIAG_MENU_TC_1_READING,
  DIAG_MENU_TC_2_READING,
  DIAG_MENU_PWM_1_READING,
  DIAG_MENU_PWM_2_READING,
  DIAG_MENU_IDLE_DUTY,
  DIAG_MENU_MAX_DUTY,
  DIAG_MENU_POOR,
  DIAG_MENU_FW_VERSION,
} DiagMenu;

static const DiagMenu DIAG_MENU_MIN = DIAG_MENU_BACK;
static const DiagMenu DIAG_MENU_MAX = DIAG_MENU_FW_VERSION;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

extern DisplayState display_state;
extern MainMenu main_menu;
extern DiagMenu diag_menu;

// Direct update: no debounce
CACHED_VALUE_DECL(extern, DisplayState, display_state);
CACHED_VALUE_DECL(extern, TipType, tip_type);
CACHED_VALUE_DECL(extern, ReedState, reed_state);
CACHED_VALUE_DECL(extern, HeatState, heat_state);
CACHED_VALUE_DECL(extern, int, heat_setpoint);
CACHED_VALUE_DECL(extern, int, setback);
CACHED_VALUE_DECL(extern, int, setback_delay);
CACHED_VALUE_DECL(extern, int, standby_delay);
CACHED_VALUE_DECL(extern, int, temperature_offset);
CACHED_VALUE_DECL(extern, int, temperature_unit);
CACHED_VALUE_DECL(extern, int, step_size);
CACHED_VALUE_DECL(extern, int, kty_value);
CACHED_VALUE_DECL(extern, int, reference);
CACHED_VALUE_DECL(extern, int, idle_duty);
CACHED_VALUE_DECL(extern, int, max_duty);
CACHED_VALUE_DECL(extern, int, poor_mode);

// Frequent updates: debounce the input
CACHED_VALUE_DECL(extern, int, right_duty);
CACHED_VALUE_DECL(extern, int, left_duty);
CACHED_VALUE_DECL(extern, int, right_temperature);
CACHED_VALUE_DECL(extern, int, left_temperature);
CACHED_VALUE_DECL(extern, int, tc_right_temperature);
CACHED_VALUE_DECL(extern, int, tc_left_temperature);
CACHED_VALUE_DECL(extern, int, main_period);

// Menu cache
CACHED_VALUE_DECL(extern, MainMenu, main_menu);
CACHED_VALUE_DECL(extern, DiagMenu, diag_menu);

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void display_init(void);
void display_loop(void);
void display_invalidate_cached_values(void);

#endif // DISPLAY_H_
