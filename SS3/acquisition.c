#include "acquisition.h"

#include "adc.h"
#include "board.h"
#include "calibration.h"
#include "configuration.h"
#include "display.h"
#include "main.h"
#include "medianfilter.h"
#include "pt.h"
#include "util.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define TC1A_MEDIAN_FILTER UINT16_2
#define TC2A_MEDIAN_FILTER UINT16_2
#define TC1_MEDIAN_FILTER UINT16_16
#define TC2_MEDIAN_FILTER UINT16_16

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  STRUCTS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

MEDIAN_FILTER_DEC_NDEF_TYPE(UINT16_2, uint16_t, 2)
MEDIAN_FILTER_DEC_NDEF_TYPE(UINT16_16, uint16_t, 16)

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

int right_temperature;
int left_temperature;

int reed_value;
int kty_value;
bool new_acquisition;

static struct pt acq_process_pt;

static float kty_value_float;

static uint16_t raw_values[ADC_CHANNEL_MAX];

static MEDIAN_FILTER_DEF(TC1A_MEDIAN_FILTER, tc1a_median_filter);
static MEDIAN_FILTER_DEF(TC2A_MEDIAN_FILTER, tc2a_median_filter);
static MEDIAN_FILTER_DEF(TC1_MEDIAN_FILTER, tc1_median_filter);
static MEDIAN_FILTER_DEF(TC2_MEDIAN_FILTER, tc2_median_filter);

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void acquisition_init(void) {
  pt_reset(&acq_process_pt);
  kty_value = left_temperature = right_temperature = 25;
  reed_value = ADC_MAX;
  new_acquisition = false;

  MEDIAN_FILTER_INIT(TC1A_MEDIAN_FILTER, tc1a_median_filter);
  MEDIAN_FILTER_INIT(TC2A_MEDIAN_FILTER, tc2a_median_filter);
  MEDIAN_FILTER_INIT(TC1_MEDIAN_FILTER, tc1_median_filter);
  MEDIAN_FILTER_INIT(TC2_MEDIAN_FILTER, tc2_median_filter);

  // Disable pull-ups
  DL_GPIO_disableOutput(GPIOA, Other_REED_PULLUP_PIN);
  DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_DISABLED);
}

static void acquisition_compute(void) {

  uint16_t filtred_value, compensated_value;

  //
  // KTY/TC2
  //
  MEDIAN_FILTER_INSERT(TC2_MEDIAN_FILTER, tc2_median_filter, raw_values[ADC_CHANNEL_TC2]);
  if (!MEDIAN_FILTER_MEDIAN(TC2_MEDIAN_FILTER, tc2_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }

  // TC2 is isometric: not compensed
  // -> Supply voltage is the same that ADC VREF+
  kty_value_float = interpolate2d_u16_f32(&kty_inter_table, filtred_value);
  kty_value = ROUND(int, kty_value_float);

  //
  // REED/TC1
  //
  MEDIAN_FILTER_INSERT(TC1_MEDIAN_FILTER, tc1_median_filter, raw_values[ADC_CHANNEL_TC1]);
  if (!MEDIAN_FILTER_MEDIAN(TC1_MEDIAN_FILTER, tc1_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }

  // TC1 use VDD: not compensed
  reed_value = filtred_value;

  //
  // LEFT TEMP/TC2A
  //
  MEDIAN_FILTER_INSERT(TC2A_MEDIAN_FILTER, tc2a_median_filter, raw_values[ADC_CHANNEL_TC2A]);
  if (!MEDIAN_FILTER_MEDIAN(TC2A_MEDIAN_FILTER, tc2a_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }

  // We need to compensate the amplified values regarding the real voltage reference.
  // -> If real voltage reference is bigger, the adc value will be lower: TC is not related to voltage reference.
  compensated_value = (uint16_t)MIN(((int)filtred_value * (int)reference) / (int)2500, (int)ADC_MAX);

  // Final temperature is interpolated TC value plus KTY value.
  left_temperature = ROUND(int, interpolate2d_u16_f32(&tc_inter_table, compensated_value) + kty_value_float);

  //
  // RIGHT TEMP/TC1A
  //
  MEDIAN_FILTER_INSERT(TC1A_MEDIAN_FILTER, tc1a_median_filter, raw_values[ADC_CHANNEL_TC1A]);
  if (!MEDIAN_FILTER_MEDIAN(TC1A_MEDIAN_FILTER, tc1a_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }

  // We need to compensate the amplified values regarding the real voltage reference.
  // -> If real voltage reference is bigger, the adc value will be lower: TC is not related to voltage reference.
  compensated_value = (uint16_t)MIN(((int)filtred_value * (int)reference) / (int)2500, (int)ADC_MAX);

  // Final temperature is interpolated TC value plus KTY value.
  right_temperature = ROUND(int, interpolate2d_u16_f32(&tc_inter_table, compensated_value) + kty_value_float);
}

void acquisition_loop(void) {
  if (pt_status(&acq_process_pt) != PT_STATUS_FINISHED) {
    new_acquisition = false;
    return;
  }

  // Get ADCs values
  for (int i = 0; i < ADC_CHANNEL_MAX; ++i) {
    if (!adc_getvalue(i, &raw_values[i])) {
      // Should not happen
      ERROR_HANDLER();
    }
  }

  acquisition_compute();
  new_acquisition = true;
  pt_reset(&acq_process_pt);
}

void acquisition_step(void) {
  //
  // Start of asynchronous part
  //

  pt_begin(&acq_process_pt);

  DL_TimerA_startCounter(MeasurementTimer_INST);

  // Start ADCs
  adc_trigger(ADC_CHANNEL_TC1A);
  adc_trigger(ADC_CHANNEL_TC2A);

  pt_yield(&acq_process_pt);
  //
  // Pause
  //

  DL_TimerA_startCounter(MeasurementTimer_INST);

  // Enable pull-ups
  DL_GPIO_enableOutput(GPIOA, Other_REED_PULLUP_PIN);
  DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_ENABLED);

  pt_yield(&acq_process_pt);
  //
  // Pause
  //

  DL_TimerA_startCounter(MeasurementTimer_INST);

  // Start ADCs
  adc_trigger(ADC_CHANNEL_TC1);
  adc_trigger(ADC_CHANNEL_TC2);

  pt_yield(&acq_process_pt);
  //
  // Pause
  //

  // Disable pull-ups
  DL_GPIO_disableOutput(GPIOA, Other_REED_PULLUP_PIN);
  if (display_state != DISPLAY_STATE_ADJ_REFERENCE) {
    DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_DISABLED);
  }

  pt_end(&acq_process_pt);

  //
  // End of asynchronous part
  //
}

bool acquisition_trigger(void) {
  bool ret = false;
  NVIC_DisableIRQ(MeasurementTimer_INST_INT_IRQN);
  if (pt_status(&acq_process_pt) == PT_STATUS_IDLE) {
    acquisition_step();
    ret = true;
  }
  NVIC_EnableIRQ(MeasurementTimer_INST_INT_IRQN);
  return ret;
}

void MeasurementTimer_INST_IRQHandler(void) {
  if (DL_TimerA_getPendingInterrupt(MeasurementTimer_INST) == DL_TIMERA_INTERRUPT_ZERO_EVENT) {
    acquisition_step();
  }
}
