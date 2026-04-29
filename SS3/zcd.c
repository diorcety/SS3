#include "zcd.h"

#include "acquisition.h"
#include "board.h"
#include "iron.h"
#include "tick.h"
#include "util.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/
int main_period;

static uint32_t zcd_previous_tick;
static volatile uint32_t zcd_current_tick;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void zcd_init(void) {
  main_period = 0;
  zcd_previous_tick = zcd_current_tick = systick_get();
  DL_COMP_clearInterruptStatus(ZCD_INST, DL_COMP_INTERRUPT_OUTPUT_EDGE | DL_COMP_INTERRUPT_OUTPUT_EDGE);
  NVIC_ClearPendingIRQ(ZCD_INST_INT_IRQN);
}

void zcd_loop(void) {
  NVIC_EnableIRQ(ZCD_INST_INT_IRQN);
  uint32_t zcd_tick = zcd_current_tick;
  if (zcd_tick > zcd_previous_tick) {
    main_period = (zcd_tick - zcd_previous_tick);
    zcd_previous_tick = zcd_tick;
  }
}

void ZCD_INST_IRQHandler(void) {
  if (DL_COMP_getPendingInterrupt(ZCD_INST) & (DL_COMP_INTERRUPT_OUTPUT_EDGE | DL_COMP_INTERRUPT_OUTPUT_EDGE_INV)) {
    zcd_current_tick = systick_get();
    iron_trigger();
    acquisition_trigger();
  }
}
