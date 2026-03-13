#include "adc.h"
#include "board.h"
#include "main.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int adc_channel_map[] = {
    [ADC_CHANNEL_TC1A] = ADCRight_ADCMEM_TC1A,
    [ADC_CHANNEL_TC1] = ADCRight_ADCMEM_TC1,
    [ADC_CHANNEL_TC2A] = ADCLeft_ADCMEM_TC2A,
    [ADC_CHANNEL_TC2] = ADCLeft_ADCMEM_TC2,
};

static ADC12_Regs *const adc_instance_map[] = {
    [ADC_CHANNEL_TC1A] = ADCRight_INST,
    [ADC_CHANNEL_TC1] = ADCRight_INST,
    [ADC_CHANNEL_TC2A] = ADCLeft_INST,
    [ADC_CHANNEL_TC2] = ADCLeft_INST,
};

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void adc_init(void) {
#if 0
  NVIC_EnableIRQ(ADCRight_INST_INT_IRQN);
  NVIC_EnableIRQ(ADCLeft_INST_INT_IRQN);
#endif
}

void adc_loop(void) {}

void adc_trigger(AdcChannel channel) {
  ADC12_Regs *instance = adc_instance_map[channel];
  uint32_t adc_channel = adc_channel_map[channel];

  // Cancel
  DL_ADC12_disableConversions(instance);

  // Configure
  DL_ADC12_clearInterruptStatus(instance, ADC12_CPU_INT_IMASK_MEMRESIFG0_SET << adc_channel);
  DL_ADC12_setStartAddress(instance, adc_channel << ADC12_CTL2_STARTADD_OFS);
  DL_ADC12_enableConversions(instance);

  // Trigger
  DL_ADC12_startConversion(instance);
}

bool adc_getvalue(AdcChannel channel, uint16_t *pValue) {
  ADC12_Regs *instance = adc_instance_map[channel];
  uint32_t adc_channel = adc_channel_map[channel];
  bool ret = false;
  if (DL_ADC12_getEnabledInterruptStatus(instance, ADC12_CPU_INT_IMASK_MEMRESIFG0_SET << adc_channel)) {
    *pValue = DL_ADC12_getMemResult(instance, adc_channel);
    ret = true;
  }
  return ret;
}

#if 0
void ADCRight_INST_IRQHandler(void) { ERROR_HANDLER(); }

void ADCLeft_INST_IRQHandler(void) { ERROR_HANDLER(); }
#endif
