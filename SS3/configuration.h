#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <stdbool.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int CONFIGURATION_SAVE_DELAY = 5000;

// Assign some 'factory default' values to use when run for the first time
// These are also used if any of the parameters stored in eeprom are outside
// of valid range
static const int SETPOINT_DEFAULT = 240;
static const int SETBACK_DEFAULT = 120;
static const int SETBACK_DELAY_DEFAULT = 60;  // in seconds
static const int STANDBY_DELAY_DEFAULT = 180; // in seconds
static const int TEMPERATURE_OFFSET_DEFAULT = 0;
static const int REFERENCE_DEFAULT = 2500;
static const int STEP_SIZE_DEFAULT = 5;
static const int TEMPERATURE_UNIT_DEFAULT = 0;
static const int POOR_MODE_DEFAULT = 0;
static const int MAX_DUTY_DEFAULT = 45;  // 45%
static const int IDLE_DUTY_DEFAULT = 20; // Idle detection from duty cycle off as default

// Assign sane / safe limits to the above parameters
static const int SETPOINT_MIN = 100;
static const int SETPOINT_MAX = 450;
static const int SETBACK_MIN = SETPOINT_MIN;
static const int SETBACK_MAX = SETPOINT_MAX;
static const int SETBACK_DELAY_MIN = 0;
static const int SETBACK_DELAY_MAX = 3600;
static const int STANDBY_DELAY_MIN = 0;
static const int STANDBY_DELAY_MAX = 3600;
static const int TEMPERATURE_OFFSET_MIN = -40;
static const int TEMPERATURE_OFFSET_MAX = 40;
static const int REFERENCE_MIN = 2450;
static const int REFERENCE_MAX = 2550; // max. tolerance is 2% of 2.500
static const int STEP_SIZE_MIN = 1;
static const int STEP_SIZE_MAX = 10;
static const int TEMPERATURE_UNIT_MIN = 0;
static const int TEMPERATURE_UNIT_MAX = 1;
static const int POOR_MODE_MIN = 0;
static const int POOR_MODE_MAX = 1;
static const int MAX_DUTY_MAX = 60;
static const int MAX_DUTY_MIN = 10;
static const int IDLE_DUTY_MIN = 0;
static const int IDLE_DUTY_MAX = 75;

static const int DUTY_BASE = 100;

static const int FW_VERSION = 1;

static const int KTY_FALLBACK = 30;

typedef enum {
  TEMPERATURE_UNIT_C,
  TEMPERATURE_UNIT_F
} TemperatureUnit;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

extern int setpoint;
extern int setback;
extern int setback_delay;
extern int standby_delay;
extern int temperature_offset;
extern int reference;
extern int step_size;
extern int temperature_unit;
extern int poor_mode;
extern int max_duty;
extern int idle_duty;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                FUNCTIONS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

void configuration_init(void);
bool configuration_load(void);
void configuration_save(void);
void configuration_loop(void);

#endif // CONFIGURATION_H_
