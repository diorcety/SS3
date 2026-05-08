#include "main.h"
#include "acquisition.h"
#include "adc.h"
#include "board.h"
#include "button.h"
#include "configuration.h"
#include "display.h"
#include "heat.h"
#include "iron.h"
#include "seg7.h"
#include "tick.h"
#include "tip.h"
#include "zcd.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static void init(void) {
  //
  // Configuration first
  //
  configuration_init();
  configuration_load();

  zcd_init();
  adc_init();
  button_init();

  acquisition_init();
  tip_init();
  heat_init();
  display_init();

  iron_init();
#if defined(USE_SEG7)
  seg7_init();
#endif
}

static void loop(void) {
#ifndef NO_WDG
  DL_WWDT_restart(Watchdog_INST);
#endif

  // Inputs part
  zcd_loop();
  adc_loop();
  button_loop();

  // Process part
  acquisition_loop();
  tip_loop();
  heat_loop();
  display_loop();

  // Outputs part
  iron_loop();
#if defined(USE_SEG7)
  seg7_loop();
#endif

  // Don't freeze during a tricky moment
  if (iron_is_standby()) {
      configuration_loop();
  }
}

int main(void) {
  SYSCFG_DL_init();

  // Can't be set by default in syscfg
  DL_GPIO_disableOutput(GPIOA, Other_REED_PULLUP_PIN);
  DL_OPA_setOutputPinState(SecondVRef_INST, OA_CFG_OUTPIN_DISABLED);

  init();

  for (;;) {
    loop();
  }
}

//
// Interrupt handlers
//

void SysTick_Handler(void) { systick_inc(); }

// Dispatch  GROUP0 interrupt to fake interrupt handlers
void PowerProtection_IRQHandler(void);
void GROUP0_IRQHandler(void) { PowerProtection_IRQHandler(); }

// Dispatch  GROUP1 interrupt to fake interrupt handlers
void ZCD_INST_IRQHandler(void);
void Buttons_INST_IRQHandler(void);
void GROUP1_IRQHandler(void) {
  ZCD_INST_IRQHandler();
  Buttons_INST_IRQHandler();
}

//
// Error handlers
//
void HardFault_Handler(void) { ERROR_HANDLER(); }

void NMI_Handler(void) { ERROR_HANDLER(); }

void iron_set_output(uint32_t value);
void error_handler(void) {
  iron_set_output(0);
  __disable_irq();
  for (;;) {
  }
}

#ifdef NO_WDG

//
// Bypass watchdog init procedures
//

void SYSCFG_DL_Watchdog_init(void) {}

void SYSCFG_DL_PowerProtection_init(void) {}
#endif
