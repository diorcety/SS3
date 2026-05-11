#include "st7789_cpp.h"

#include "Arduino_MSPM0SPIDMA.h"
#include <display/Arduino_ST7789.h>

extern "C" {
#include "board.h"
}

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

alignas(Arduino_ST7789) static uint8_t st7889_buffer[sizeof(Arduino_ST7789)];
alignas(Arduino_MSPM0SPIDMA) static uint8_t bus_buffer[sizeof(Arduino_MSPM0SPIDMA)];

static Arduino_MSPM0SPIDMA *bus;
static Arduino_ST7789 *st7889;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void st7789_init_screen(uint8_t r, bool ips, int16_t w, int16_t h, uint8_t col_offset1, uint8_t row_offset1,
                 uint8_t col_offset2, uint8_t row_offset2) {
  DL_GPIO_initPeripheralOutputFunction(GPIO_SPI_0_IOMUX_CS1, IOMUX_PINCM8_PF_GPIOA_DIO03);
  DL_GPIO_setPins(GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);
  DL_GPIO_enableOutput(GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);

  bus = new (bus_buffer) Arduino_MSPM0SPIDMA(
      SPI_0_INST, SPI_0_DMA_TX_CHAN_ID, Screen_PORT, Screen_CMD_PIN, GPIO_SPI_0_CS1_PORT, GPIO_SPI_0_CS1_PIN);

  st7889 = new (st7889_buffer) Arduino_ST7789(bus,
                                              GFX_NOT_DEFINED /* RST */,
                                              r /* rotation */,
                                              ips /* IPS */,
                                              w /* width */,
                                              h /* height */,
                                              col_offset1,
                                              row_offset1,
                                              col_offset2,
                                              row_offset2);

  if (!st7889->begin()) {
    ERROR_HANDLER();
  }
}

void st7889_fill_screen(uint16_t color) { st7889->fillScreen(color); }
void st7889_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  st7889->drawLine(x0, y0, x1, y1, color);
}
void st7889_set_font(const uint8_t *font) { st7889->setFont(font); }
void st7889_set_cursor(int16_t x, int16_t y) { st7889->setCursor(x, y); }
void st7889_set_text_color(uint16_t color) { st7889->setTextColor(color); }
void st7889_print(const char *str) { st7889->print(str); }
