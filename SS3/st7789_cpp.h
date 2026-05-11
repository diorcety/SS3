#ifndef ST7789_CPP_H_
#define ST7789_CPP_H_

#include <stdbool.h>
#include <stdint.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle */
void st7789_init_screen(uint8_t r, bool ips, int16_t w, int16_t h, uint8_t col_offset1, uint8_t row_offset1,
                        uint8_t col_offset2, uint8_t row_offset2);

/* 1:1 wrappers */
void st7889_fill_screen(uint16_t color);
void st7889_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void st7889_set_font(const uint8_t *font);
void st7889_set_cursor(int16_t x, int16_t y);
void st7889_set_text_color(uint16_t fg_color, uint16_t bg_color);
void st7889_print(const char *str);
void st7889_set_text_size(uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // ST7789_CPP_H_
