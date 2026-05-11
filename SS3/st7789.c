#include "st7789.h"

#if defined(USE_ST7789)
#include "st7789_cpp.h"

#include "acquisition.h"
#include "board.h"
#include "coru.h"
#include "display.h"
#include "main.h"
#include "tick.h"
#include "tip.h"
#include "util.h"
#include "zcd.h"

#include <stdio.h>

#define U8G2_FONT_SECTION(name)
#include <logisoso160.h>
#include <logisoso32.h>
#include <logisoso64.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define RGB16TO24(c) ((((uint32_t)c & 0xF800) << 8) | ((c & 0x07E0) << 5) | ((c & 0x1F) << 3))

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define ST7789_RST_DELAY 120

/* https://www.w3.org/wiki/CSS/Properties/color/keywords */
#define RGB565_ALICEBLUE RGB565(240, 248, 248)
#define RGB565_ANTIQUEWHITE RGB565(248, 236, 216)
#define RGB565_AQUA RGB565(0, 252, 248)
#define RGB565_AQUAMARINE RGB565(128, 252, 216)
#define RGB565_AZURE RGB565(240, 252, 248)
#define RGB565_BEIGE RGB565(248, 244, 224)
#define RGB565_BISQUE RGB565(248, 228, 200)
#define RGB565_BLACK RGB565(0, 0, 0)
#define RGB565_BLANCHEDALMOND RGB565(248, 236, 208)
#define RGB565_BLUE RGB565(0, 0, 248)
#define RGB565_BLUEVIOLET RGB565(136, 44, 224)
#define RGB565_BROWN RGB565(168, 44, 40)
#define RGB565_BURLYWOOD RGB565(224, 184, 136)
#define RGB565_CADETBLUE RGB565(96, 160, 160)
#define RGB565_CHARTREUSE RGB565(128, 252, 0)
#define RGB565_CHOCOLATE RGB565(208, 104, 32)
#define RGB565_CORAL RGB565(248, 128, 80)
#define RGB565_CORNFLOWERBLUE RGB565(104, 148, 240)
#define RGB565_CORNSILK RGB565(248, 248, 224)
#define RGB565_CRIMSON RGB565(224, 20, 64)
#define RGB565_CYAN RGB565(0, 252, 248)
#define RGB565_DARKBLUE RGB565(0, 0, 136)
#define RGB565_DARKCYAN RGB565(0, 140, 136)
#define RGB565_DARKGOLDENROD RGB565(184, 136, 8)
#define RGB565_DARKGRAY RGB565(168, 168, 168)
#define RGB565_DARKGREEN RGB565(0, 100, 0)
#define RGB565_DARKGREY RGB565(168, 168, 168)
#define RGB565_DARKKHAKI RGB565(192, 184, 104)
#define RGB565_DARKMAGENTA RGB565(136, 0, 136)
#define RGB565_DARKOLIVEGREEN RGB565(88, 108, 48)
#define RGB565_DARKORANGE RGB565(248, 140, 0)
#define RGB565_DARKORCHID RGB565(152, 52, 208)
#define RGB565_DARKRED RGB565(136, 0, 0)
#define RGB565_DARKSALMON RGB565(232, 152, 120)
#define RGB565_DARKSEAGREEN RGB565(144, 188, 144)
#define RGB565_DARKSLATEBLUE RGB565(72, 60, 136)
#define RGB565_DARKSLATEGRAY RGB565(48, 80, 80)
#define RGB565_DARKSLATEGREY RGB565(48, 80, 80)
#define RGB565_DARKTURQUOISE RGB565(0, 208, 208)
#define RGB565_DARKVIOLET RGB565(152, 0, 208)
#define RGB565_DEEPPINK RGB565(248, 20, 144)
#define RGB565_DEEPSKYBLUE RGB565(0, 192, 248)
#define RGB565_DIMGRAY RGB565(104, 104, 104)
#define RGB565_DIMGREY RGB565(104, 104, 104)
#define RGB565_DODGERBLUE RGB565(32, 144, 248)
#define RGB565_FIREBRICK RGB565(176, 36, 32)
#define RGB565_FLORALWHITE RGB565(248, 252, 240)
#define RGB565_FORESTGREEN RGB565(32, 140, 32)
#define RGB565_FUCHSIA RGB565(248, 0, 248)
#define RGB565_GAINSBORO RGB565(224, 220, 224)
#define RGB565_GHOSTWHITE RGB565(248, 248, 248)
#define RGB565_GOLD RGB565(248, 216, 0)
#define RGB565_GOLDENROD RGB565(216, 164, 32)
#define RGB565_GRAY RGB565(128, 128, 128)
#define RGB565_GREEN RGB565(0, 128, 0)
#define RGB565_GREENYELLOW RGB565(176, 252, 48)
#define RGB565_GREY RGB565(128, 128, 128)
#define RGB565_HONEYDEW RGB565(240, 252, 240)
#define RGB565_HOTPINK RGB565(248, 104, 184)
#define RGB565_INDIANRED RGB565(208, 92, 96)
#define RGB565_INDIGO RGB565(72, 0, 128)
#define RGB565_IVORY RGB565(248, 252, 240)
#define RGB565_KHAKI RGB565(240, 232, 144)
#define RGB565_LAVENDER RGB565(232, 232, 248)
#define RGB565_LAVENDERBLUSH RGB565(248, 240, 248)
#define RGB565_LAWNGREEN RGB565(128, 252, 0)
#define RGB565_LEMONCHIFFON RGB565(248, 252, 208)
#define RGB565_LIGHTBLUE RGB565(176, 216, 232)
#define RGB565_LIGHTCORAL RGB565(240, 128, 128)
#define RGB565_LIGHTCYAN RGB565(224, 252, 248)
#define RGB565_LIGHTGOLDENRODYELLOW RGB565(248, 252, 208)
#define RGB565_LIGHTGRAY RGB565(208, 212, 208)
#define RGB565_LIGHTGREEN RGB565(144, 240, 144)
#define RGB565_LIGHTGREY RGB565(208, 212, 208)
#define RGB565_LIGHTPINK RGB565(248, 184, 192)
#define RGB565_LIGHTSALMON RGB565(248, 160, 120)
#define RGB565_LIGHTSEAGREEN RGB565(32, 180, 168)
#define RGB565_LIGHTSKYBLUE RGB565(136, 208, 248)
#define RGB565_LIGHTSLATEGRAY RGB565(120, 136, 152)
#define RGB565_LIGHTSLATEGREY RGB565(120, 136, 152)
#define RGB565_LIGHTSTEELBLUE RGB565(176, 196, 224)
#define RGB565_LIGHTYELLOW RGB565(248, 252, 224)
#define RGB565_LIME RGB565(0, 252, 0)
#define RGB565_LIMEGREEN RGB565(48, 204, 48)
#define RGB565_LINEN RGB565(248, 240, 232)
#define RGB565_MAGENTA RGB565(248, 0, 248)
#define RGB565_MAROON RGB565(128, 0, 0)
#define RGB565_MEDIUMAQUAMARINE RGB565(104, 204, 168)
#define RGB565_MEDIUMBLUE RGB565(0, 0, 208)
#define RGB565_MEDIUMORCHID RGB565(184, 84, 208)
#define RGB565_MEDIUMPURPLE RGB565(144, 112, 216)
#define RGB565_MEDIUMSEAGREEN RGB565(64, 180, 112)
#define RGB565_MEDIUMSLATEBLUE RGB565(120, 104, 240)
#define RGB565_MEDIUMSPRINGGREEN RGB565(0, 252, 152)
#define RGB565_MEDIUMTURQUOISE RGB565(72, 208, 208)
#define RGB565_MEDIUMVIOLETRED RGB565(200, 20, 136)
#define RGB565_MIDNIGHTBLUE RGB565(24, 24, 112)
#define RGB565_MINTCREAM RGB565(248, 252, 248)
#define RGB565_MISTYROSE RGB565(248, 228, 224)
#define RGB565_MOCCASIN RGB565(248, 228, 184)
#define RGB565_NAVAJOWHITE RGB565(248, 224, 176)
#define RGB565_NAVY RGB565(0, 0, 128)
#define RGB565_OLDLACE RGB565(248, 244, 232)
#define RGB565_OLIVE RGB565(128, 128, 0)
#define RGB565_OLIVEDRAB RGB565(104, 144, 32)
#define RGB565_ORANGE RGB565(248, 164, 0)
#define RGB565_ORANGERED RGB565(248, 68, 0)
#define RGB565_ORCHID RGB565(216, 112, 216)
#define RGB565_PALEGOLDENROD RGB565(240, 232, 168)
#define RGB565_PALEGREEN RGB565(152, 252, 152)
#define RGB565_PALETURQUOISE RGB565(176, 240, 240)
#define RGB565_PALEVIOLETRED RGB565(216, 112, 144)
#define RGB565_PAPAYAWHIP RGB565(248, 240, 216)
#define RGB565_PEACHPUFF RGB565(248, 220, 184)
#define RGB565_PERU RGB565(208, 132, 64)
#define RGB565_PINK RGB565(248, 192, 200)
#define RGB565_PLUM RGB565(224, 160, 224)
#define RGB565_POWDERBLUE RGB565(176, 224, 232)
#define RGB565_PURPLE RGB565(128, 0, 128)
#define RGB565_RED RGB565(248, 0, 0)
#define RGB565_ROSYBROWN RGB565(192, 144, 144)
#define RGB565_ROYALBLUE RGB565(64, 104, 224)
#define RGB565_SADDLEBROWN RGB565(136, 68, 16)
#define RGB565_SALMON RGB565(248, 128, 112)
#define RGB565_SANDYBROWN RGB565(248, 164, 96)
#define RGB565_SEAGREEN RGB565(48, 140, 88)
#define RGB565_SEASHELL RGB565(248, 244, 240)
#define RGB565_SIENNA RGB565(160, 84, 48)
#define RGB565_SILVER RGB565(192, 192, 192)
#define RGB565_SKYBLUE RGB565(136, 208, 232)
#define RGB565_SLATEBLUE RGB565(104, 92, 208)
#define RGB565_SLATEGRAY RGB565(112, 128, 144)
#define RGB565_SLATEGREY RGB565(112, 128, 144)
#define RGB565_SNOW RGB565(248, 252, 248)
#define RGB565_SPRINGGREEN RGB565(0, 252, 128)
#define RGB565_STEELBLUE RGB565(72, 132, 184)
#define RGB565_TAN RGB565(208, 180, 144)
#define RGB565_TEAL RGB565(0, 128, 128)
#define RGB565_THISTLE RGB565(216, 192, 216)
#define RGB565_TOMATO RGB565(248, 100, 72)
#define RGB565_TURQUOISE RGB565(64, 224, 208)
#define RGB565_VIOLET RGB565(240, 132, 240)
#define RGB565_WHEAT RGB565(248, 224, 176)
#define RGB565_WHITE RGB565(248, 252, 248)
#define RGB565_WHITESMOKE RGB565(248, 244, 248)
#define RGB565_YELLOW RGB565(248, 252, 0)
#define RGB565_YELLOWGREEN RGB565(152, 204, 48)

#define CHAR_NC "\x83"
#define CHAR_WXUP "\x81"
#define CHAR_WMRP "\x80"
#define CHAR_WMRT "\x82"
#define CHAR_SETBACK "\x8C"
#define CHAR_STANDBY "\x8D"
#define CHAR_REED "\x8E"
#define CHAR_TARGET "\x8F"
#define CHAR_LIGTHING "\x90"
#define CHAR_HEAT "\x91"
#define CHAR_SPACE " "

#define CHAR_SIGNAL1 "\x96"
#define CHAR_SIGNAL2 "\x97"
#define CHAR_SIGNAL3 "\x98"
#define CHAR_SIGNAL4 "\x99"
#define CHAR_SIGNAL5 "\x9A"

#define RGB565_SIGNAL1 RGB565(0, 255, 0)
#define RGB565_SIGNAL2 RGB565(128, 255, 0)
#define RGB565_SIGNAL3 RGB565(255, 255, 0)
#define RGB565_SIGNAL4 RGB565(255, 165, 0)
#define RGB565_SIGNAL5 RGB565(255, 0, 0)

static const uint16_t TEMPEATURE_COLOR_LUT[] = {
    // 0-189 : Blue -> Cyan
    RGB565(0, 0, 255),
    RGB565(0, 5, 255),
    RGB565(0, 10, 255),
    RGB565(0, 15, 255),
    RGB565(0, 20, 255),
    RGB565(0, 25, 255),
    RGB565(0, 30, 255),
    RGB565(0, 35, 255),
    RGB565(0, 40, 255),
    RGB565(0, 45, 255),
    RGB565(0, 50, 255),
    RGB565(0, 55, 255),
    RGB565(0, 60, 255),
    RGB565(0, 65, 255),
    RGB565(0, 70, 255),
    RGB565(0, 75, 255),
    RGB565(0, 80, 255),
    RGB565(0, 85, 255),
    RGB565(0, 90, 255),
    RGB565(0, 95, 255),
    RGB565(0, 100, 255),
    RGB565(0, 105, 255),
    RGB565(0, 110, 255),
    RGB565(0, 115, 255),
    RGB565(0, 120, 255),
    RGB565(0, 125, 255),
    RGB565(0, 130, 255),
    RGB565(0, 135, 255),
    RGB565(0, 140, 255),
    RGB565(0, 145, 255),
    RGB565(0, 150, 255),
    RGB565(0, 155, 255),
    RGB565(0, 160, 255),
    RGB565(0, 165, 255),
    RGB565(0, 170, 255),
    RGB565(0, 175, 255),
    RGB565(0, 180, 255),
    RGB565(0, 185, 255),

    // 190-239 : Cyan -> Green
    RGB565(0, 190, 245),
    RGB565(0, 195, 235),
    RGB565(0, 200, 225),
    RGB565(0, 205, 215),
    RGB565(0, 210, 205),
    RGB565(0, 215, 195),
    RGB565(0, 220, 185),
    RGB565(0, 225, 175),
    RGB565(0, 230, 165),
    RGB565(0, 235, 155),
    RGB565(0, 240, 145),
    RGB565(0, 245, 135),
    RGB565(0, 250, 125),
    RGB565(0, 255, 115),
    RGB565(0, 255, 105),
    RGB565(0, 255, 95),
    RGB565(0, 255, 85),
    RGB565(0, 255, 75),
    RGB565(0, 255, 65),
    RGB565(0, 255, 55),

    // 240-319 : Green -> Orange
    RGB565(0, 255, 0),
    RGB565(15, 255, 0),
    RGB565(30, 255, 0),
    RGB565(45, 255, 0),
    RGB565(60, 255, 0),
    RGB565(75, 255, 0),
    RGB565(90, 255, 0),
    RGB565(105, 255, 0),
    RGB565(120, 255, 0),
    RGB565(135, 255, 0),
    RGB565(150, 255, 0),
    RGB565(165, 255, 0),
    RGB565(180, 255, 0),
    RGB565(195, 245, 0),
    RGB565(210, 235, 0),
    RGB565(225, 225, 0),
    RGB565(240, 215, 0),
    RGB565(255, 205, 0),
    RGB565(255, 195, 0),
    RGB565(255, 185, 0),

    // 320-399 : Orange -> Red
    RGB565(255, 175, 0),
    RGB565(255, 165, 0),
    RGB565(255, 155, 0),
    RGB565(255, 145, 0),
    RGB565(255, 135, 0),
    RGB565(255, 125, 0),
    RGB565(255, 115, 0),
    RGB565(255, 105, 0),
    RGB565(255, 95, 0),
    RGB565(255, 85, 0),
    RGB565(255, 75, 0),
    RGB565(255, 65, 0),
    RGB565(255, 55, 0),
    RGB565(255, 45, 0),
    RGB565(255, 35, 0),
    RGB565(255, 25, 0),
    RGB565(255, 15, 0),
    RGB565(255, 5, 0),
    RGB565(255, 0, 0),
    RGB565(255, 0, 10),

    // 400+ : Red -> Fuchsia
    RGB565(255, 0, 20),
    RGB565(255, 0, 35),
    RGB565(255, 0, 50),
    RGB565(255, 0, 65),
    RGB565(255, 0, 80),
    RGB565(255, 0, 95),
    RGB565(255, 0, 110),
    RGB565(255, 0, 125),
    RGB565(255, 0, 140),
    RGB565(255, 0, 155),
    RGB565(255, 0, 170),
    RGB565(255, 0, 185),
    RGB565(255, 0, 200),
    RGB565(255, 0, 215),
    RGB565(255, 0, 230),
    RGB565(255, 0, 245),
    RGB565(255, 0, 255),
    RGB565(250, 0, 255),
    RGB565(245, 0, 255),
    RGB565(240, 0, 255),
};
static const uint16_t TEMPERATURE_COLOR_MAX = 500;

static const char *TIP_ICONS[] = {
    [TIP_TYPE_NC] = CHAR_NC,
    [TIP_TYPE_WXUP] = CHAR_WXUP,
    [TIP_TYPE_WMRP] = CHAR_WMRP,
    [TIP_TYPE_WMRT] = CHAR_WMRT,
};

static const char *TIP_VOLTAGES[] = {
    [TIP_TYPE_NC] = CHAR_SPACE "   ",
    [TIP_TYPE_WXUP] = CHAR_LIGTHING "24V",
    [TIP_TYPE_WMRP] = CHAR_LIGTHING "12V",
    [TIP_TYPE_WMRT] = CHAR_LIGTHING "12V",
};

static const char *REED_ICONS[] = {
    [REED_STATE_CLOSED] = CHAR_REED,
    [REED_STATE_OPENED] = CHAR_SPACE,
};

static const uint16_t ST7789_BG_COLOR = RGB565_BLACK;
static const uint16_t ST7789_VOLTAGE_COLOR = RGB565_CYAN;
static const unsigned int ST7789_SCREEN_HEIGHT = 170;
static const unsigned int ST7789_SCREEN_WIDTH = 320;
static const unsigned int ST7789_MAIN_BASELINE = 140;
static const unsigned int ST7789_RIGHT_COLUMN = 72 * 3 + 10;
static const unsigned int ST7789_RIGHT_COLUMN_X = (ST7789_SCREEN_WIDTH - ST7789_RIGHT_COLUMN) / 3;
static const unsigned int ST7789_RIGHT_COLUMN_BOTTOM = ST7789_SCREEN_HEIGHT - 10;

static const unsigned int ST7789_SETPOINT_BOTTOM = 32;
static const unsigned int ST7789_SETPOINT_LEFT = (ST7789_RIGHT_COLUMN + ST7789_SCREEN_WIDTH) / 2 - (14 * 6) / 2;

static const unsigned int ST7789_VOLTAGE_BOTTOM = 64;

static const unsigned int ST7789_DUTY_BOTTOM = 96;

static const int ST7789_MAIN_PERIOD_IIR_WINDOW = 8;
static const int ST7789_LEFT_TEMPERATURE_IIR_WINDOW = 32;
static const int ST7789_RIGHT_TEMPERATURE_IIR_WINDOW = 32;

static const int ST7789_TEMPERATURE_SLOW_UPDATE_MS = 1000;
static const int ST7789_TEMPERATURE_SLOW_THRESHOLD_DEG = 2;

static int main_period_acc;
static int right_temperature_acc;
static int right_temperature_filtred;
static int left_temperature_acc;
static int left_temperature_filtred;

static uint32_t previous_temperature_update;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/
#ifndef TEST
static coru_t co;
static uint8_t co_stack[256];
#else
#define reed_state reed_state_test
#define tip_type tip_type_test
static TipType tip_type = TIP_TYPE_WMRT;
static ReedState reed_state = REED_STATE_CLOSED;
#endif

static tick_timer_t delay_timer;

static bool st7789_force_redraw;
static TipType previous_tip_type;
static ReedState previous_reed_state;

static DisplayState previous_display_state;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

#if 1
static inline void yield(void) { coru_yield(); }
#else
static inline void yield(void) {}
#define TEST
#endif

static void st7789_clear(void) { st7889_fill_screen(ST7789_BG_COLOR); }

static uint16_t st7789_temperature_color(int temperature) {
  int index = temperature * ARRAY_SIZE(TEMPEATURE_COLOR_LUT) / TEMPERATURE_COLOR_MAX;
  index = MIN(MAX(index, 0), ARRAY_SIZE(TEMPEATURE_COLOR_LUT) - 1);
  return TEMPEATURE_COLOR_LUT[index];
}

static void st7789_duty_signal(uint16_t x, uint16_t y, int duty) {
  static const char *bars[5] = {CHAR_SIGNAL1, CHAR_SIGNAL2, CHAR_SIGNAL3, CHAR_SIGNAL4, CHAR_SIGNAL5};

  static const uint16_t colors[5] = {RGB565_SIGNAL1, RGB565_SIGNAL2, RGB565_SIGNAL3, RGB565_SIGNAL4, RGB565_SIGNAL5};

  uint8_t level;

  if (duty >= 80)
    level = 5;
  else if (duty >= 60)
    level = 4;
  else if (duty >= 40)
    level = 3;
  else if (duty >= 20)
    level = 2;
  else if (duty > 0)
    level = 1;
  else
    level = 0;

  st7889_set_font(logisoso32);
  st7889_set_cursor(x, y);
  st7889_set_text_color(RGB565_WHITE);
  st7889_print(CHAR_HEAT);

  for (uint8_t i = 0; i < 5; i++) {
    if (i < level) {
      st7889_set_text_color(colors[i]);
      st7889_print(bars[i]);
    } else {
      st7889_print(CHAR_SPACE);
    }
  }
}

void st7789_screen_main(void) {
  if (st7789_force_redraw) {
    st7889_draw_line(ST7789_RIGHT_COLUMN - 5, 0, ST7789_RIGHT_COLUMN - 5, ST7789_SCREEN_HEIGHT, RGB565_WHITE);
  }

  if (st7789_force_redraw) {
    st7889_set_font(logisoso160);
    st7889_set_cursor(0, ST7789_MAIN_BASELINE);
    st7889_set_text_color(st7789_temperature_color(123));
    static char buffer[4];
    snprintf(buffer, sizeof(buffer), "%d", 123);
    st7889_print(buffer);
  }

  if (st7789_force_redraw) {
    st7889_set_font(logisoso32);
    st7889_set_cursor(ST7789_SETPOINT_LEFT, ST7789_SETPOINT_BOTTOM);
    st7889_set_text_color(RGB565_WHITE);
    static char buffer[7];
    snprintf(buffer, sizeof(buffer), CHAR_TARGET "%d°c", 240);
    st7889_print(buffer);
  }

  if (st7789_force_redraw || previous_reed_state != reed_state) {
    previous_reed_state = reed_state;

    st7889_set_font(logisoso64);
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 0, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE);
    st7889_print(REED_ICONS[reed_state]);
  }

  if (st7789_force_redraw) {
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 1, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE);
    st7889_print(CHAR_SETBACK);
  }

  if (st7789_force_redraw) {
    st7789_duty_signal(ST7789_RIGHT_COLUMN, ST7789_DUTY_BOTTOM, 80);
  }

  if (st7789_force_redraw || previous_tip_type != tip_type) {
    previous_tip_type = tip_type;

    st7889_set_font(logisoso64);
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 2, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE);
    st7889_print(TIP_ICONS[tip_type]);

    st7889_set_font(logisoso32);
    st7889_set_cursor(ST7789_SETPOINT_LEFT, ST7789_VOLTAGE_BOTTOM);
    st7889_set_text_color(ST7789_VOLTAGE_COLOR);
    st7889_print(TIP_VOLTAGES[tip_type]);
  }

  if (st7789_force_redraw) {
    st7789_force_redraw = false;
  }
}

void st7789_screen_redraw(void) {
  if (previous_display_state != display_state) {
    st7789_force_redraw = true;
    previous_display_state = display_state;
  }

  if (st7789_force_redraw) {
    st7789_clear();
  }

  switch (display_state) {
  case DISPLAY_STATE_MAIN:
    st7789_screen_main();
    break;
  default:
    break;
  }
}

void st7789_coru(void *param) {
  UNUSED(param);

  DL_GPIO_setPins(Screen_PORT, Screen_Reset_PIN | Screen_BL_PIN);

  tick_timer_start(&delay_timer, 2, true); /* minimum 1 ms */
  while (!tick_timer_elapsed(&delay_timer)) {
    yield();
  }
  DL_GPIO_clearPins(Screen_PORT, Screen_Reset_PIN);

  tick_timer_start(&delay_timer, ST7789_RST_DELAY, true); /* ST7789_RST_DELAY ms */
  while (!tick_timer_elapsed(&delay_timer)) {
    yield();
  }

  // Init Display
  st7789_init_screen(3 /* rotation */,
                     true /* IPS */,
                     ST7789_SCREEN_HEIGHT /* width */,
                     ST7789_SCREEN_WIDTH /* height */,
                     35,
                     0,
                     35,
                     0);

  // Enable backlight
  DL_GPIO_clearPins(Screen_PORT, Screen_BL_PIN);

  while (1) {
    st7789_screen_redraw();

    yield();
  }
}

void st7789_init(void) {
  st7789_force_redraw = true;
  previous_tip_type = tip_type;
  previous_reed_state = reed_state;

  main_period_acc = 0;
  right_temperature_acc = left_temperature_acc = 0;
  right_temperature_filtred = 0;
  left_temperature_filtred = 0;

  previous_temperature_update = systick_get();

  previous_display_state = display_state;

  tick_timer_init(&delay_timer);
#ifndef TEST
  coru_create_inplace(&co, st7789_coru, NULL, co_stack, sizeof(co_stack));
#else
  st7789_coru(NULL);
#endif
}

void st7789_loop(void) {

  if (new_acquisition) {
    IIR_FILTER_ADD(ST7789_MAIN_PERIOD_IIR_WINDOW, main_period_acc, main_period);

    if (tip_has_right()) {
      IIR_FILTER_ADD(ST7789_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc, right_temperature);
    } else {
      IIR_FILTER_ADD(ST7789_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc, kty_temperature);
    }
    if (tip_has_left()) {
      IIR_FILTER_ADD(ST7789_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc, left_temperature);
    } else {
      IIR_FILTER_ADD(ST7789_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc, kty_temperature);
    }

    // Get filtred value
    int tmp_right_temperature_filtred = IIR_FILTER_GET(ST7789_RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc);
    int tmp_left_temperature_filtred = IIR_FILTER_GET(ST7789_LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc);

    // Only update above threshold or after some delay: avoid flikering
    if (abs(tmp_right_temperature_filtred - right_temperature_filtred) >= ST7789_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        abs(tmp_left_temperature_filtred - left_temperature_filtred) >= ST7789_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        systick_elapsed(previous_temperature_update, ST7789_TEMPERATURE_SLOW_UPDATE_MS)) {
      right_temperature_filtred = tmp_right_temperature_filtred;
      left_temperature_filtred = tmp_left_temperature_filtred;
      previous_temperature_update = systick_get();
    }
  }

#ifndef TEST
  coru_resume(&co);
#endif
}

#endif
