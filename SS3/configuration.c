#include "configuration.h"

#include "board.h"
#include "eeprom.h"
#include "tick.h"

#include <stdint.h>

#define PARAM_LOAD(name, var) param_load_check(EEPROM_DATA_##name, &var, name##_MIN, name##_MAX, name##_DEFAULT)
#define PARAM_SAVE(name, var) eeprom_write(EEPROM_DATA_##name, (uint32_t)var)

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

enum {
  EEPROM_DATA_SETPOINT,
  EEPROM_DATA_SETBACK,
  EEPROM_DATA_SETBACK_DELAY,
  EEPROM_DATA_STANDBY_DELAY,
  EEPROM_DATA_TEMPERATURE_OFFSET,
  EEPROM_DATA_REFERENCE,
  EEPROM_DATA_STEP_SIZE,
  EEPROM_DATA_TEMPERATURE_UNIT,
  EEPROM_DATA_POOR_MODE,
  EEPROM_DATA_MAX_DUTY,
  EEPROM_DATA_IDLE_DUTY,
} EepromData;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

int setpoint;
int setback;
int setback_delay;
int standby_delay;
int temperature_offset;
int reference;
int step_size;
int temperature_unit;
int poor_mode;
int max_duty;
int idle_duty;

static tick_timer_t configuration_save_timer;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static bool param_load_check(uint16_t variable_id, int *pvariable, int min_value, int max_value, int default_value) {
  if (eeprom_read(variable_id, (uint32_t *)pvariable) == false) {
    *pvariable = default_value;
    return false;
  }
  if (*pvariable < min_value || *pvariable > max_value) {
    *pvariable = default_value;
    return false;
  }
  return true;
}

void configuration_init(void) {
  eeprom_init();
  tick_timer_init(&configuration_save_timer);
}

bool configuration_load(void) {
  bool loaded = true;
  loaded &= PARAM_LOAD(SETPOINT, setpoint);
  loaded &= PARAM_LOAD(SETBACK, setback);
  loaded &= PARAM_LOAD(SETBACK_DELAY, setback_delay);
  loaded &= PARAM_LOAD(STANDBY_DELAY, standby_delay);
  loaded &= PARAM_LOAD(TEMPERATURE_OFFSET, temperature_offset);
  loaded &= PARAM_LOAD(REFERENCE, reference);
  loaded &= PARAM_LOAD(STEP_SIZE, step_size);
  loaded &= PARAM_LOAD(TEMPERATURE_UNIT, temperature_unit);
  loaded &= PARAM_LOAD(POOR_MODE, poor_mode);
  loaded &= PARAM_LOAD(MAX_DUTY, max_duty);
  loaded &= PARAM_LOAD(IDLE_DUTY, idle_duty);
  return loaded;
}

void configuration_save(void) { tick_timer_start(&configuration_save_timer, CONFIGURATION_SAVE_DELAY, true); }

void configuration_loop(void) {
  if (tick_timer_elapsed(&configuration_save_timer)) {
    PARAM_SAVE(SETPOINT, setpoint);
    PARAM_SAVE(SETBACK, setback);
    PARAM_SAVE(SETBACK_DELAY, setback_delay);
    PARAM_SAVE(STANDBY_DELAY, standby_delay);
    PARAM_SAVE(TEMPERATURE_OFFSET, temperature_offset);
    PARAM_SAVE(REFERENCE, reference);
    PARAM_SAVE(STEP_SIZE, step_size);
    PARAM_SAVE(TEMPERATURE_UNIT, temperature_unit);
    PARAM_SAVE(POOR_MODE, poor_mode);
    PARAM_SAVE(MAX_DUTY, max_duty);
    PARAM_SAVE(IDLE_DUTY, idle_duty);
  } else {
    eeprom_loop();
  }
}
