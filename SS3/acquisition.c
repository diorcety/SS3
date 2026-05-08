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

#define ACQ_MEDIAN
#ifdef ACQ_MEDIAN
#define TC1A_MEDIAN_FILTER UINT16_8
#define TC2A_MEDIAN_FILTER UINT16_8
#define TC1_MEDIAN_FILTER UINT16_16
#define TC2_MEDIAN_FILTER UINT16_16
#else
static const int TC1A_IIR_WINDOW = 8;
static const int TC1_IIR_WINDOW = 8;
static const int TC2A_IIR_WINDOW = 8;
static const int TC2_IIR_WINDOW = 8;
#endif

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  STRUCTS                                                          *
 *                                                                                                                   *
 *********************************************************************************************************************/

MEDIAN_FILTER_DEC_NDEF_TYPE(UINT16_8, uint16_t, 8)
MEDIAN_FILTER_DEC_NDEF_TYPE(UINT16_16, uint16_t, 16)

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

float tc_right_temperature;
float tc_right_voltage;
int right_temperature;
float tc_left_temperature;
float tc_left_voltage;
int left_temperature;

float kty_value;
int kty_temperature;

int reed_value;

bool new_acquisition;

static struct pt acq_process_pt;

static uint16_t raw_values[ADC_CHANNEL_MAX];

#ifdef ACQ_MEDIAN
static MEDIAN_FILTER_DEF(TC1A_MEDIAN_FILTER, tc1a_median_filter);
static MEDIAN_FILTER_DEF(TC2A_MEDIAN_FILTER, tc2a_median_filter);
static MEDIAN_FILTER_DEF(TC1_MEDIAN_FILTER, tc1_median_filter);
static MEDIAN_FILTER_DEF(TC2_MEDIAN_FILTER, tc2_median_filter);
#else
static int tc1a_acc, tc1_acc;
static int tc2a_acc, tc2_acc;
#endif

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void acquisition_init(void) {
  pt_reset(&acq_process_pt);
  kty_value = (float)KTY_FALLBACK;
  kty_temperature = left_temperature = right_temperature = KTY_FALLBACK;
  tc_left_temperature = tc_right_temperature = 0.0f;
  tc_left_voltage = tc_right_voltage = 0.0f;
  reed_value = ADC_MAX;
  new_acquisition = false;

#ifdef ACQ_MEDIAN
  MEDIAN_FILTER_INIT(TC1A_MEDIAN_FILTER, tc1a_median_filter);
  MEDIAN_FILTER_INIT(TC2A_MEDIAN_FILTER, tc2a_median_filter);
  MEDIAN_FILTER_INIT(TC1_MEDIAN_FILTER, tc1_median_filter);
  MEDIAN_FILTER_INIT(TC2_MEDIAN_FILTER, tc2_median_filter);
#else
  tc1a_acc = tc1_acc = 0;
  tc2a_acc = tc2_acc = 0;
#endif

  // Disable pull-ups
  DL_GPIO_disableOutput(GPIOA, Other_REED_PULLUP_PIN);
  DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_DISABLED);
}

static void acquisition_compute(void) {

  uint16_t filtred_value, compensated_value;

  //
  // KTY/TC2
  //
#ifdef ACQ_MEDIAN
  MEDIAN_FILTER_INSERT(TC2_MEDIAN_FILTER, tc2_median_filter, raw_values[ADC_CHANNEL_TC2]);
  if (!MEDIAN_FILTER_MEDIAN(TC2_MEDIAN_FILTER, tc2_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }
#else
  IIR_FILTER_ADD(TC2_IIR_WINDOW, tc2_acc, raw_values[ADC_CHANNEL_TC2]);
  filtred_value = IIR_FILTER_GET(TC2_IIR_WINDOW, tc2_acc);
#endif

  // TC2 is isometric: not compensed
  // -> Supply voltage is the same that ADC VREF+
  kty_value = interpolate2d_u16_f32(&kty_inter_table, filtred_value);
  kty_temperature = (!poor_mode) ? ROUND(int, kty_value) : KTY_FALLBACK;

  //
  // REED/TC1
  //
#ifdef ACQ_MEDIAN
  MEDIAN_FILTER_INSERT(TC1_MEDIAN_FILTER, tc1_median_filter, raw_values[ADC_CHANNEL_TC1]);
  if (!MEDIAN_FILTER_MEDIAN(TC1_MEDIAN_FILTER, tc1_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }
#else
  IIR_FILTER_ADD(TC1_IIR_WINDOW, tc1_acc, raw_values[ADC_CHANNEL_TC1]);
  filtred_value = IIR_FILTER_GET(TC1_IIR_WINDOW, tc1_acc);
#endif

  // TC1 use VDD: not compensed
  reed_value = filtred_value;

  //
  // LEFT TEMP/TC2A
  //
#ifdef ACQ_MEDIAN
  MEDIAN_FILTER_INSERT(TC2A_MEDIAN_FILTER, tc2a_median_filter, raw_values[ADC_CHANNEL_TC2A]);
  if (!MEDIAN_FILTER_MEDIAN(TC2A_MEDIAN_FILTER, tc2a_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }
#else
  IIR_FILTER_ADD(TC2A_IIR_WINDOW, tc2a_acc, raw_values[ADC_CHANNEL_TC2A]);
  filtred_value = IIR_FILTER_GET(TC2A_IIR_WINDOW, tc2a_acc);
#endif

  // We need to compensate the amplified values regarding the real voltage reference.
  // -> If real voltage reference is bigger, the adc value will be lower: TC is not related to voltage reference.
  compensated_value = (uint16_t)MIN(((int)filtred_value * (int)reference) / (int)2500, (int)ADC_MAX);

  // Final temperature is interpolated TC value plus KTY value.
  tc_left_temperature = interpolate2d_u16_f32(&tc_inter_table, compensated_value);
  left_temperature = ROUND(int, tc_left_temperature + ((!poor_mode) ? kty_value : KTY_FALLBACK));

  //
  // RIGHT TEMP/TC1A
  //
#ifdef ACQ_MEDIAN
  MEDIAN_FILTER_INSERT(TC1A_MEDIAN_FILTER, tc1a_median_filter, raw_values[ADC_CHANNEL_TC1A]);
  if (!MEDIAN_FILTER_MEDIAN(TC1A_MEDIAN_FILTER, tc1a_median_filter, filtred_value, true)) {
    ERROR_HANDLER();
  }
#else
  IIR_FILTER_ADD(TC1A_IIR_WINDOW, tc1a_acc, raw_values[ADC_CHANNEL_TC1A]);
  filtred_value = IIR_FILTER_GET(TC1A_IIR_WINDOW, tc1a_acc);
#endif

  // We need to compensate the amplified values regarding the real voltage reference.
  // -> If real voltage reference is bigger, the adc value will be lower: TC is not related to voltage reference.
  compensated_value = (uint16_t)MIN(((int)filtred_value * (int)reference) / (int)2500, (int)ADC_MAX);

  // Final temperature is interpolated TC value plus KTY value.
  tc_right_temperature = interpolate2d_u16_f32(&tc_inter_table, compensated_value);
  right_temperature = ROUND(int, tc_right_temperature + ((!poor_mode) ? kty_value : KTY_FALLBACK));

  //
  // LEFT VOLTAGE/TC2V
  //
  tc_left_voltage = (float)raw_values[ADC_CHANNEL_TC2V] / (float)ADC_MAX * (float)(ADCLeft_ADCMEM_TC2V_REF_VOLTAGE_V);

  //
  // RIGHT VOLTAGE/TC1V
  //
  tc_right_voltage = (float)raw_values[ADC_CHANNEL_TC1V] / (float)ADC_MAX * (float)(ADCRight_ADCMEM_TC1V_REF_VOLTAGE_V);
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

  DL_TimerA_startCounter(MeasurementTimer_INST);

  // Disable pull-ups
  DL_GPIO_disableOutput(GPIOA, Other_REED_PULLUP_PIN);
  if (display_state != DISPLAY_STATE_ADJ_REFERENCE) {
    DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_DISABLED);
  }

  pt_yield(&acq_process_pt);
  //
  // Pause
  //

  DL_TimerA_startCounter(MeasurementTimer_INST);

  // Start ADCs
  adc_trigger(ADC_CHANNEL_TC1V);
  adc_trigger(ADC_CHANNEL_TC2V);

  pt_yield(&acq_process_pt);
  //
  // Pause
  //

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
