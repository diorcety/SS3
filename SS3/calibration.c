#include "calibration.h"

#include "adc.h"
#include "board.h"
#include "util.h"

#include <stdint.h>


/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/
// OMP with 300kohm + 1kohm
#define TC_ADC_VALUE(x) ROUND(uint16_t, (x) / VOLTAGE_DIVIDER(300000.0f, 1000.0f) * TC_RATIO)

// Voltage divider with KTY as lower resistor and 1kohm as upper resistor
#define KTY_ADC_VALUE(x) ROUND(uint16_t, VOLTAGE_DIVIDER(1000.0f, x) * KTY_RATIO)

#define INTERP_TABLE(name, x_arr, y_arr)                                                                               \
  _Static_assert(ARRAY_SIZE(x_arr) == ARRAY_SIZE(y_arr), "X and Y tables must have same length");                      \
                                                                                                                       \
  const interpolate2d_table_u16_f32_t name = {.x = (x_arr), .y = (y_arr), .length = ARRAY_SIZE(x_arr)}

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

//
// TC
//

static const float TC_RATIO = (float)ADC_MAX / 1000.0f / (float)(ADCRight_ADCMEM_TC1A_REF_VOLTAGE_V);

// From http://kair.us/projects/weller/Weller_WMRP_and_WMRT_thermocouple_voltage_vs_temperature.xlsx
static const uint16_t tc_adc_table[] = {
    TC_ADC_VALUE(0.000f), TC_ADC_VALUE(0.130f), TC_ADC_VALUE(0.263f), TC_ADC_VALUE(0.398f), TC_ADC_VALUE(0.537f),
    TC_ADC_VALUE(0.677f), TC_ADC_VALUE(0.820f), TC_ADC_VALUE(0.966f), TC_ADC_VALUE(1.114f), TC_ADC_VALUE(1.263f),
    TC_ADC_VALUE(1.415f), TC_ADC_VALUE(1.569f), TC_ADC_VALUE(1.725f), TC_ADC_VALUE(1.882f), TC_ADC_VALUE(2.042f),
    TC_ADC_VALUE(2.203f), TC_ADC_VALUE(2.365f), TC_ADC_VALUE(2.529f), TC_ADC_VALUE(2.694f), TC_ADC_VALUE(2.860f),
    TC_ADC_VALUE(3.028f), TC_ADC_VALUE(3.196f), TC_ADC_VALUE(3.366f), TC_ADC_VALUE(3.536f), TC_ADC_VALUE(3.707f),
    TC_ADC_VALUE(3.879f), TC_ADC_VALUE(4.052f), TC_ADC_VALUE(4.225f), TC_ADC_VALUE(4.398f), TC_ADC_VALUE(4.572f),
    TC_ADC_VALUE(4.746f), TC_ADC_VALUE(4.920f), TC_ADC_VALUE(5.094f), TC_ADC_VALUE(5.268f), TC_ADC_VALUE(5.442f),
    TC_ADC_VALUE(5.616f), TC_ADC_VALUE(5.790f), TC_ADC_VALUE(5.963f), TC_ADC_VALUE(6.135f), TC_ADC_VALUE(6.307f),
    TC_ADC_VALUE(6.479f), TC_ADC_VALUE(6.649f), TC_ADC_VALUE(6.819f), TC_ADC_VALUE(6.988f), TC_ADC_VALUE(7.155f),
    TC_ADC_VALUE(7.322f), TC_ADC_VALUE(7.487f), TC_ADC_VALUE(7.652f), TC_ADC_VALUE(7.814f), TC_ADC_VALUE(7.975f),
    TC_ADC_VALUE(8.135f),
};

static const float tc_value_table[] = {
    0.0f,   10.0f,  20.0f,  30.0f,  40.0f,  50.0f,  60.0f,  70.0f,  80.0f,  90.0f,  100.0f, 110.0f, 120.0f,
    130.0f, 140.0f, 150.0f, 160.0f, 170.0f, 180.0f, 190.0f, 200.0f, 210.0f, 220.0f, 230.0f, 240.0f, 250.0f,
    260.0f, 270.0f, 280.0f, 290.0f, 300.0f, 310.0f, 320.0f, 330.0f, 340.0f, 350.0f, 360.0f, 370.0f, 380.0f,
    390.0f, 400.0f, 410.0f, 420.0f, 430.0f, 440.0f, 450.0f, 460.0f, 470.0f, 480.0f, 490.0f, 500.0f,
};

INTERP_TABLE(tc_inter_table, tc_adc_table, tc_value_table);

//
// KTY
//

static const float KTY_RATIO = (float)ADC_MAX;

// From https://www.nxp.com/docs/en/data-sheet/KTY82_SER.pdf
static const uint16_t kty_adc_table[] = {
    KTY_ADC_VALUE(515.0f),  KTY_ADC_VALUE(567.0f),  KTY_ADC_VALUE(624.0f),  KTY_ADC_VALUE(684.0f),
    KTY_ADC_VALUE(747.0f),  KTY_ADC_VALUE(815.0f),  KTY_ADC_VALUE(886.0f),  KTY_ADC_VALUE(961.0f),
    KTY_ADC_VALUE(1040.0f), KTY_ADC_VALUE(1122.0f), KTY_ADC_VALUE(1209.0f), KTY_ADC_VALUE(1299.0f),
    KTY_ADC_VALUE(1392.0f), KTY_ADC_VALUE(1490.0f), KTY_ADC_VALUE(1591.0f), KTY_ADC_VALUE(1696.0f),
    KTY_ADC_VALUE(1805.0f), KTY_ADC_VALUE(1915.0f), KTY_ADC_VALUE(2023.0f), KTY_ADC_VALUE(2124.0f),
    KTY_ADC_VALUE(2211.0f),
};

static const float kty_value_table[] = {
    -50.0f, -40.0f, -30.0f, -20.0f, -10.0f, 0.0f,   10.0f,  20.0f,  30.0f,  40.0f,  50.0f,
    60.0f,  70.0f,  80.0f,  90.0f,  100.0f, 110.0f, 120.0f, 130.0f, 140.0f, 150.0f,
};

INTERP_TABLE(kty_inter_table, kty_adc_table, kty_value_table);

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

DEFINE_INTERPOLATE2D_FUNC(interpolate2d_u16_f32, interpolate2d_table_u16_f32_t, uint16_t, float)
