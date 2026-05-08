#ifndef ADC_H_
#define ADC_H_

#include <stdbool.h>
#include <stdint.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int ADC_BITS = 16;
static const int ADC_MAX = ((1 << ADC_BITS) - 1);

typedef enum {
  ADC_CHANNEL_TC1A,
  ADC_CHANNEL_TC1V,
  ADC_CHANNEL_TC1,
  ADC_CHANNEL_TC2A,
  ADC_CHANNEL_TC2V,
  ADC_CHANNEL_TC2,
  ADC_CHANNEL_MAX
} AdcChannel;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void adc_init(void);
void adc_loop(void);

void adc_trigger(AdcChannel channel);
bool adc_getvalue(AdcChannel channel, uint16_t *pValue);

#endif // ADC_H_
