#include "st7789.h"

#if defined(USE_ST7789)

extern "C" {
#include "board.h"
#include "coru.h"
#include "main.h"
#include "tick.h"
}

#include "Arduino_MSPM0SPIDMA.h"
#include <display/Arduino_ST7789.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

// static coru_t co;
// static uint8_t co_stack[4096];

static timer_t delay_timer;

alignas(Arduino_ST7789) uint8_t display_buffer[sizeof(Arduino_ST7789)];
Arduino_ST7789 *display;
alignas(Arduino_MSPM0SPIDMA) uint8_t bus_buffer[sizeof(Arduino_MSPM0SPIDMA)];
Arduino_MSPM0SPIDMA *bus;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void st7789_coru(void *) {

  DL_GPIO_setPins(Screen_PORT, Screen_Reset_PIN | Screen_BL_PIN);

  timer_start(&delay_timer, 1, true); /* 1 ms */
  while (!timer_elapsed(&delay_timer)) {
    yield();
  }
  DL_GPIO_clearPins(Screen_PORT, Screen_Reset_PIN);

  timer_start(&delay_timer, ST7789_RST_DELAY, true); /* ST7789_RST_DELAY ms */
  while (!timer_elapsed(&delay_timer)) {
    yield();
  }

  // Init Display
  if (!display->begin()) {
    ERROR_HANDLER();
  }
  display->fillScreen(RGB565_BLACK);

  // Light the screen
  DL_GPIO_clearPins(Screen_PORT, Screen_BL_PIN);

  //display->setCursor(10, 10);
  //display->setTextColor(RGB565_RED);
  //display->println("Hello World!");

  while (1) {
    yield();
  }
}

extern "C" void st7789_init(void) {
  // Reconfigure CS1 as GPIO for manual control
  DL_GPIO_initPeripheralOutputFunction(GPIO_SPI_0_IOMUX_CS1, IOMUX_PINCM8_PF_GPIOA_DIO03);
  DL_GPIO_setPins(GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);
  DL_GPIO_enableOutput(GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);

  bus = new (bus_buffer) Arduino_MSPM0SPIDMA(
      SPI0, SPI_DMA_TX_CHAN_ID, Screen_PORT, Screen_CMD_PIN, GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);
  display = new (display_buffer) Arduino_ST7789(
      bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */);

  timer_init(&delay_timer);
  // coru_create_inplace(&co, st7789_coru, NULL, co_stack, sizeof(co_stack));

  NVIC_EnableIRQ(SPI_0_INST_INT_IRQN);
  st7789_coru(NULL);
}

extern "C" void st7789_loop(void) {
  // coru_resume(&co); // returns 0, prints hi!
}

extern "C" void st7789_INST_IRQHandler(void) {
  switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
  case DL_SPI_IIDX_IDLE:
    bus->transaction_complete();
    break;
  default:
    break;
  }
}

#endif