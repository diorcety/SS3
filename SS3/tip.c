
#include "tip.h"

#include "acquisition.h"
#include "adc.h"
#include "board.h"
#include "configuration.h"
#include "util.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/
#define VOLTAGE_DIVIDER_MIN_VALUE(upper, lower, err) VOLTAGE_DIVIDER(upper * (1.0f + err), lower * (1.0f - err))
#define VOLTAGE_DIVIDER_MAX_VALUE(upper, lower, err) VOLTAGE_DIVIDER(upper * (1.0f - err), lower * (1.0f + err))

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const float REED_SUPPLY_VOLTAGE_V = 3.3f;
static const float RATIO = (float)ADC_MAX / (float)ADCRight_ADCMEM_TC1_REF_VOLTAGE_V;

static const float REED_TOLERANCE = 0.1f; // 10%

static const int REED_WMUP_VALUE_MIN =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MIN_VALUE(1000.0f, 510.0f, REED_TOLERANCE) * RATIO);
static const int REED_WMUP_VALUE_MAX =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MAX_VALUE(1000.0f, 510.0f, REED_TOLERANCE) * RATIO);
static const int REED_WMRP_VALUE_MIN =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MIN_VALUE(1000.0f, 1000.0f, REED_TOLERANCE) * RATIO);
static const int REED_WMRP_VALUE_MAX =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MAX_VALUE(1000.0f, 1000.0f, REED_TOLERANCE) * RATIO);
static const int REED_WMRT_VALUE_MIN =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MIN_VALUE(1000.0f, 2000.0f, REED_TOLERANCE) * RATIO);
static const int REED_WMRT_VALUE_MAX =
    ROUND(int, REED_SUPPLY_VOLTAGE_V *VOLTAGE_DIVIDER_MAX_VALUE(1000.0f, 2000.0f, REED_TOLERANCE) * RATIO);

static const int REED_CLOSED_VALUE = ROUND(int, REED_SUPPLY_VOLTAGE_V * 0.2f * RATIO);
static const int REED_NC_VALUE = ROUND(int, REED_SUPPLY_VOLTAGE_V * 0.8f * RATIO);

_Static_assert((REED_WMUP_VALUE_MIN - REED_CLOSED_VALUE) > 0, "Not enough room between CLOSED and WMUP");
_Static_assert((REED_WMRP_VALUE_MIN - REED_WMUP_VALUE_MAX) > 0, "Not enough room between WMUP and WMRP");
_Static_assert((REED_WMRT_VALUE_MIN - REED_WMRP_VALUE_MAX) > 0, "Not enough room between WMRP and WMRT");
_Static_assert((REED_NC_VALUE - REED_WMRT_VALUE_MAX) > 0, "Not enough room between WMRT and NC");

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

TipType tip_type;
ReedState reed_state;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static inline int reed_in_range(int value, int min, int max) { return value > min && value < max; }

void tip_init(void) {
  tip_type = TIP_TYPE_NC;
  reed_state = REED_STATE_OPENED;
}

void tip_loop(void) {
  if (!new_acquisition) {
    return;
  }

  /* Check */
  if (reed_value < REED_CLOSED_VALUE) {
    if (poor_mode) {
      tip_type = TIP_TYPE_WMRP;
      reed_state = REED_STATE_OPENED;
    } else if (tip_type == TIP_TYPE_NC) {
      reed_state = REED_STATE_CLOSED;
    } else {
      reed_state = REED_STATE_CLOSED;
    }
  } else if (reed_in_range(reed_value, REED_WMUP_VALUE_MIN, REED_WMUP_VALUE_MAX)) {
    tip_type = TIP_TYPE_WMUP;
    reed_state = REED_STATE_OPENED;
  } else if (reed_in_range(reed_value, REED_WMRP_VALUE_MIN, REED_WMRP_VALUE_MAX)) {
    tip_type = TIP_TYPE_WMRP;
    reed_state = REED_STATE_OPENED;
  } else if (reed_in_range(reed_value, REED_WMRT_VALUE_MIN, REED_WMRT_VALUE_MAX)) {
    tip_type = TIP_TYPE_WMRT;
    reed_state = REED_STATE_OPENED;
  } else if (reed_value > REED_NC_VALUE) {
    tip_type = TIP_TYPE_NC;
    reed_state = REED_STATE_OPENED;
  }
}
