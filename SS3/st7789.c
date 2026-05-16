#include "st7789.h"

#if defined(USE_ST7789)
#include "st7789_cpp.h"

#include "acquisition.h"
#include "board.h"
#include "cached_value.h"
#include "configuration.h"
#include "display.h"
#include "heat.h"
#include "iron.h"
#include "main.h"
#include "tick.h"
#include "tip.h"
#include "util.h"
#include "zcd.h"

#include <coru.h>
#include <stdio.h>
#include <string.h>

#define U8G2_FONT_SECTION(name)
#include <inconsolata24.h>
#include <inconsolata48.h>
#include <inconsolata60.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define RGB16TO24(c) ((((uint32_t)c & 0xF800) << 8) | ((c & 0x07E0) << 5) | ((c & 0x1F) << 3))

#define MAX_SCREEN_BUFFER_SIZE(SCREEN, CHARACTER) (((SCREEN) / (CHARACTER)) + 1)

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
#define CHAR_ERR "\x92"
#define CHAR_REED "\x8E"
#define CHAR_TARGET "\x8F"
#define CHAR_LIGTHING "\x90"
#define CHAR_HEAT "\x91"
#define CHAR_SPACE "\x20"

#define CHAR_UP "\x9c"
#define CHAR_DOWN "\x9d"
#define CHAR_LEFT "\x9e"
#define CHAR_RIGHT "\x9f"

#define CHAR_SIGNAL1 "\x96"
#define CHAR_SIGNAL2 "\x97"
#define CHAR_SIGNAL3 "\x98"
#define CHAR_SIGNAL4 "\x99"
#define CHAR_SIGNAL5 "\x9A"
#define CHAR_DEGREE "\xB0"

#define SIGNAL1_COLOR RGB565(0, 255, 0)
#define SIGNAL2_COLOR RGB565(128, 255, 0)
#define SIGNAL3_COLOR RGB565(255, 255, 0)
#define SIGNAL4_COLOR RGB565(255, 165, 0)
#define SIGNAL5_COLOR RGB565(255, 0, 0)

static char const *const SIGNAL_CHARACTERS[5] = {CHAR_SIGNAL1, CHAR_SIGNAL2, CHAR_SIGNAL3, CHAR_SIGNAL4, CHAR_SIGNAL5};

static const uint16_t SIGNAL_COLORS[5] = {SIGNAL1_COLOR, SIGNAL2_COLOR, SIGNAL3_COLOR, SIGNAL4_COLOR, SIGNAL5_COLOR};

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

static char const *const ON_TXT = "On";
static char const *const OFF_TXT = "Off";

static char const *const FREQUENCY_SUFFIX = " Hz";
static char const *const CELSIUS_SUFFIX = CHAR_DEGREE "C";
static char const *const FAHRENHEIT_SUFFIX = CHAR_DEGREE "F";

static char const *const main_menu_txt[] = {
    [MAIN_MENU_BACK] = "Back",
    [MAIN_MENU_SETBACK] = "Setback",
    [MAIN_MENU_SETBACK_DELAY] = "Setback delay",
    [MAIN_MENU_STANDBY_DELAY] = "Standby delay",
    [MAIN_MENU_OFFSET] = "Temperature offset",
    [MAIN_MENU_UNIT] = "Unit",
    [MAIN_MENU_STEP_SIZE] = "Step size",
    [MAIN_MENU_DIAG] = "Diagnostic",
};

static char const *const diag_menu_txt[] = {
    [DIAG_MENU_BACK] = "Back",
    [DIAG_MENU_COLD_COMPENSATION] = "Cold compensation",
    [DIAG_MENU_REFERENCE] = "Reference voltage",
    [DIAG_MENU_TIP_TYPE] = "Tip type",
    [DIAG_MENU_REED_STATE] = "Reed state",
    [DIAG_MENU_SHOW_FREQUENCY] = "Frequency",
    [DIAG_MENU_TC_1_READING] = "Thermocouple 1",
    [DIAG_MENU_TC_2_READING] = "Thermocouple 2",
    [DIAG_MENU_PWM_1_READING] = "PWM 1",
    [DIAG_MENU_PWM_2_READING] = "PWM 2",
    [DIAG_MENU_IDLE_DUTY] = "Idle duty",
    [DIAG_MENU_MAX_DUTY] = "Max duty",
    [DIAG_MENU_POOR] = "Poor mode",
    [DIAG_MENU_FW_VERSION] = "Version",
};

static char const *const header_txt[] = {
    [DISPLAY_STATE_MAIN] = "",

    [DISPLAY_STATE_MAIN_MENU] = "Main",
    [DISPLAY_STATE_ADJ_SETBACK] = "Setback",
    [DISPLAY_STATE_ADJ_SETBACK_DELAY] = "Setback delay",
    [DISPLAY_STATE_ADJ_STANDBY_DELAY] = "Standby delay",
    [DISPLAY_STATE_ADJ_OFFSET] = "Temperature offset",
    [DISPLAY_STATE_ADJ_UNIT] = "Unit",
    [DISPLAY_STATE_ADJ_STEP_SIZE] = "Step size",

    [DISPLAY_STATE_DIAG_MENU] = "Diagnostic",
    [DISPLAY_STATE_SHOW_COLD_COMPENSATION] = "Cold compensation",
    [DISPLAY_STATE_ADJ_REFERENCE] = "Reference voltage",
    [DISPLAY_STATE_SHOW_TIP_TYPE] = "Tip type",
    [DISPLAY_STATE_SHOW_REED_STATE] = "Reed state",
    [DISPLAY_STATE_SHOW_FREQUENCY] = "Frequency",
    [DISPLAY_STATE_SHOW_TC_1_READING] = "Thermocouple 1",
    [DISPLAY_STATE_SHOW_TC_2_READING] = "Thermocouple 2",
    [DISPLAY_STATE_SHOW_PWM_1_READING] = "PWM 1",
    [DISPLAY_STATE_SHOW_PWM_2_READING] = "PWM 2",
    [DISPLAY_STATE_ADJ_IDLE_DUTY] = "Idle duty",
    [DISPLAY_STATE_ADJ_MAX_DUTY] = "Max duty",
    [DISPLAY_STATE_ADJ_POOR] = "Poor mode",
    [DISPLAY_STATE_SHOW_FW_VERSION] = "Version",
};

static char const *const tip_txt[] = {
    [TIP_TYPE_NC] = "N.C",
    [TIP_TYPE_WXUP] = "WXRP",
    [TIP_TYPE_WMRP] = "WMRP",
    [TIP_TYPE_WMRT] = "WMRT",
};

static char const *const reed_txt[] = {
    [REED_STATE_CLOSED] = "Closed",
    [REED_STATE_OPENED] = "Opened",
};

static char const *const TIP_ICONS[] = {
    [TIP_TYPE_NC] = CHAR_NC,
    [TIP_TYPE_WXUP] = CHAR_WXUP,
    [TIP_TYPE_WMRP] = CHAR_WMRP,
    [TIP_TYPE_WMRT] = CHAR_WMRT,
};

static char const *const TIP_VOLTAGES[] = {
    [TIP_TYPE_NC] = CHAR_SPACE CHAR_SPACE CHAR_SPACE CHAR_SPACE,
    [TIP_TYPE_WXUP] = CHAR_LIGTHING "24V",
    [TIP_TYPE_WMRP] = CHAR_LIGTHING "12V",
    [TIP_TYPE_WMRT] = CHAR_LIGTHING "12V",
};

static char const *const REED_ICONS[] = {
    [REED_STATE_CLOSED] = CHAR_REED,
    [REED_STATE_OPENED] = CHAR_SPACE,
};

static char const *const SETBACK_ICONS[] = {
    [HEAT_STATE_NORMAL] = CHAR_SPACE,
    [HEAT_STATE_SETBACK] = CHAR_SETBACK,
    [HEAT_STATE_STANDBY] = CHAR_SETBACK,
    [HEAT_STATE_ERROR] = CHAR_SPACE,
};

static const uint16_t ST7789_BG_COLOR = RGB565_BLACK;
static const uint16_t ST7789_HEADER_COLOR = RGB565_DARKBLUE;
static const uint16_t ST7789_SELECTED_COLOR = RGB565_LIGHTYELLOW;
static const uint16_t ST7789_VOLTAGE_COLOR = RGB565_CYAN;

static const unsigned int ST7789_SCREEN_HEIGHT = 170;
static const unsigned int ST7789_SCREEN_WIDTH = 320;

// 1024 by  613
static const unsigned int ST7789_FONT_60_H = 60;
static const unsigned int ST7789_FONT_60_W = 36;

// static const unsigned int ST7789_FONT_48_H = 48;
// static const unsigned int ST7789_FONT_48_W = 29;

static const unsigned int ST7789_FONT_24_H = 24;
static const unsigned int ST7789_FONT_24_W = 14;

static const unsigned int ST7789_MAIN_BASELINE = (ST7789_SCREEN_HEIGHT + (ST7789_FONT_60_H * 8 / 10) * 2) / 2;

static const unsigned int ST7789_RIGHT_COLUMN_MARGIN = 10;
static const unsigned int ST7789_RIGHT_COLUMN = (ST7789_FONT_60_W * 2) * 3 + ST7789_RIGHT_COLUMN_MARGIN;
static const unsigned int ST7789_RIGHT_COLUMN_X = (ST7789_SCREEN_WIDTH - ST7789_RIGHT_COLUMN) / 3;
static const unsigned int ST7789_RIGHT_COLUMN_BOTTOM = ST7789_SCREEN_HEIGHT - 8;

static const unsigned int ST7789_SETPOINT_BOTTOM = ST7789_FONT_24_H * 1;
static const unsigned int ST7789_SETPOINT_LEFT =
    (ST7789_RIGHT_COLUMN + ST7789_SCREEN_WIDTH) / 2 - (ST7789_FONT_24_W * 6) / 2;

static const unsigned int ST7789_LINE_SPACE = 6;

static const unsigned int ST7789_VOLTAGE_BOTTOM = ST7789_FONT_24_H * 2 + ST7789_LINE_SPACE * 1;

static const unsigned int ST7789_DUTY_BOTTOM = ST7789_FONT_24_H * 3 + ST7789_LINE_SPACE * 2;

static const unsigned int ST7789_HEADER_CHARACTER_WIDTH = ST7789_FONT_24_W;
static const unsigned int ST7789_HEADER_BASELINE = 30;
static const unsigned int ST7789_HEADER_HEIGHT = 40;

static const unsigned int ST7789_VIEW_CHARACTER_WIDTH = ST7789_FONT_24_W;
static const unsigned int ST7789_VIEW_BASELINE = (ST7789_SCREEN_HEIGHT + 30) / 2;

static const unsigned int ST7789_MENU_CHARACTER_WIDTH = ST7789_FONT_24_W;
static const unsigned int ST7789_MENU_TOP = ST7789_HEADER_HEIGHT + ST7789_FONT_24_H;
static const unsigned int ST7789_MENU_ROW_HEIGHT = ST7789_FONT_24_H;
static const unsigned int ST7789_MENU_VISIBLE = 5;

static const int ST7789_MAIN_PERIOD_IIR_WINDOW = 8;
static const int ST7789_LEFT_TEMPERATURE_IIR_WINDOW = 32;
static const int ST7789_RIGHT_TEMPERATURE_IIR_WINDOW = 32;

static const int ST7789_TEMPERATURE_SLOW_UPDATE_MS = 1000;
static const int ST7789_TEMPERATURE_SLOW_THRESHOLD_DEG = 2;

static const int ST7789_BLINK_DELAY = 1028;

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
#ifndef CORU_DISABLED
static coru_t co;
static _Alignas(32) uint8_t co_stack[4096];
static inline void yield(void) { coru_yield(); }
#else
static inline void yield(void) {}
#endif
// #define TEST
#ifdef TEST
#define reed_state reed_state_test
#define tip_type tip_type_test
#define heat_state heat_state_test
#define right_duty right_duty_test
#define left_duty left_duty_test
#define right_temperature right_temperature_test
#define left_temperature left_temperature_test
#define heat_setpoint heat_setpoint_test
#define heat_state heat_state_test
#define display_state display_state_test

static DisplayState display_state = DISPLAY_STATE_ADJ_UNIT;
static TipType tip_type = TIP_TYPE_WMRT;
static ReedState reed_state = REED_STATE_CLOSED;
static HeatState heat_state = HEAT_STATE_NORMAL;
static int right_duty = MAX_DUTY_MAX / 2;
static int left_duty = MAX_DUTY_MAX / 2;
static int heat_setpoint = 240;
static int right_temperature = 180;
static int left_temperature = 180;
#endif

static tick_timer_t delay_timer;

static bool st7789_force_redraw;

// Direct update: no debounce
static CACHED_VALUE_DECL(DisplayState, display_state, 0);
static CACHED_VALUE_DECL(int, blink, 0);
static CACHED_VALUE_DECL(TipType, tip_type, 0);
static CACHED_VALUE_DECL(ReedState, reed_state, 0);
static CACHED_VALUE_DECL(HeatState, heat_state, 0);
static CACHED_VALUE_DECL(int, heat_setpoint, 0);
static CACHED_VALUE_DECL(int, setback, 0);
static CACHED_VALUE_DECL(int, setback_delay, 0);
static CACHED_VALUE_DECL(int, standby_delay, 0);
static CACHED_VALUE_DECL(int, temperature_offset, 0);
static CACHED_VALUE_DECL(int, temperature_unit, 0);
static CACHED_VALUE_DECL(int, step_size, 0);
static CACHED_VALUE_DECL(int, kty_value, 0);
static CACHED_VALUE_DECL(int, reference, 0);
static CACHED_VALUE_DECL(int, idle_duty, 0);
static CACHED_VALUE_DECL(int, max_duty, 0);
static CACHED_VALUE_DECL(int, poor_mode, 0);

// Frequent updates: debounce the input
static CACHED_VALUE_DECL(int, right_duty, 200);
static CACHED_VALUE_DECL(int, left_duty, 200);
static CACHED_VALUE_DECL(int, right_temperature, 200);
static CACHED_VALUE_DECL(int, left_temperature, 200);
static CACHED_VALUE_DECL(int, tc_right_temperature, 200);
static CACHED_VALUE_DECL(int, tc_left_temperature, 200);
static CACHED_VALUE_DECL(int, main_period, 200);

// Menu cache
static CACHED_VALUE_DECL(MainMenu, main_menu, 0);
static CACHED_VALUE_DECL(DiagMenu, diag_menu, 0);

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

TESTABLE int snprint_int(char *buf, size_t size, int value, char pad, int width) {
  int neg = value < 0;
  unsigned int uval = ABS(value);

  /* Count digits */
  int ndigits = 0;
  unsigned int tmp = uval;
  do {
    ndigits++;
    tmp /= 10;
  } while (tmp > 0);

  /* Count pads */
  int padding_width = neg ? width - 1 : width;
  int npad = MAX(padding_width - ndigits, 0);

  int would_write = (neg ? 1 : 0) + npad + ndigits;
  if (size == 0)
    return would_write;

  int len = MIN(would_write, (int)size - 1);

  /* Fill right-to-left: digits, then padding, then sign */
  int pos = len - 1;
  unsigned int u = uval;
  for (int i = 0; i < ndigits && pos >= 0; i++, pos--)
    buf[pos] = '0' + (u % 10), u /= 10;
  for (int i = 0; i < npad && pos >= 0; i++, pos--)
    buf[pos] = pad;
  if (neg && pos >= 0)
    buf[pos] = '-';

  buf[len] = '\0';

  return would_write;
}

TESTABLE int print_temperature(char *s, size_t n, int temperature, bool unit, bool offset) {
  if ((TemperatureUnit)temperature_unit == TEMPERATURE_UNIT_F) {
    temperature = temperature * 9 / 5;
    if (offset) {
      temperature += 32;
    }
  }

  int written = snprint_int(s, n, temperature, '0', 3);

  if (unit) {
    const char *suffix = (TemperatureUnit)temperature_unit == TEMPERATURE_UNIT_C ? CELSIUS_SUFFIX : FAHRENHEIT_SUFFIX;
    while (*suffix != '\0') {
      if (written < (int)n - 1) {
        s[written] = *suffix;
      }
      written++;
      suffix++;
    }
    if (written < (int)n) {
      s[written] = '\0';
    }
  }

  return written;
}

static uint16_t st7789_temperature_color(int temperature) {
  int index = temperature * ARRAY_SIZE(TEMPEATURE_COLOR_LUT) / TEMPERATURE_COLOR_MAX;
  index = MIN(MAX(index, 0), ARRAY_SIZE(TEMPEATURE_COLOR_LUT) - 1);
  return TEMPEATURE_COLOR_LUT[index];
}

static void st7789_clear(void) { st7889_fill_screen(ST7789_BG_COLOR); }

static void st7789_duty_signal(uint16_t x, uint16_t y, int duty) {
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

  st7889_set_font(inconsolata24);
  st7889_set_cursor(x, y);
  st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
  st7889_print(CHAR_HEAT);

  for (uint8_t i = 0; i < 5; i++) {
    if (i < level) {
      st7889_set_text_color(SIGNAL_COLORS[i], ST7789_BG_COLOR);
      st7889_print(SIGNAL_CHARACTERS[i]);
    } else {
      st7889_print(CHAR_SPACE);
    }
  }
}

static void update_main(void) {
  bool ack_heat_state = false;
  bool ack_tip_type = false;
  bool ack_blink = false;

  /* --- separator line: only on full redraw ----------------------------- */
  if (st7789_force_redraw) {
    st7889_draw_line(ST7789_RIGHT_COLUMN - ST7789_RIGHT_COLUMN_MARGIN / 2,
                     0,
                     ST7789_RIGHT_COLUMN - ST7789_RIGHT_COLUMN_MARGIN / 2,
                     ST7789_SCREEN_HEIGHT,
                     RGB565_WHITE);
  }

  if (CACHED_VALUE_GET(heat_state) == HEAT_STATE_STANDBY) {
    if (CACHED_VALUE_NEEDS_REDRAW(heat_state, st7789_force_redraw)) {
      st7889_set_font(inconsolata60);
      st7889_set_text_size(2);
      st7889_set_cursor(0, ST7789_MAIN_BASELINE);
      st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
      st7889_print(CHAR_SPACE CHAR_STANDBY CHAR_SPACE);
      st7889_set_text_size(1);

      ack_heat_state = true;
    }
  } else if (CACHED_VALUE_GET(heat_state) == HEAT_STATE_ERROR) {
    bool redraw = false;
    if (CACHED_VALUE_NEEDS_REDRAW(heat_state, st7789_force_redraw)) {
      redraw = true;
      ack_heat_state = true;
    }
    if (CACHED_VALUE_NEEDS_REDRAW(blink, st7789_force_redraw)) {
      redraw = true;
      ack_blink = true;
    }
    if (redraw) {
      st7889_set_font(inconsolata60);
      st7889_set_text_size(2);
      st7889_set_cursor(0, ST7789_MAIN_BASELINE);
      st7889_set_text_color(RGB565_RED, ST7789_BG_COLOR);
      st7889_print((CACHED_VALUE_GET(blink) % 2 == 0) ? (CHAR_SPACE CHAR_ERR CHAR_SPACE)
                                                      : (CHAR_ERR CHAR_SPACE CHAR_ERR));
      st7889_set_text_size(1);
    }
  } else {
    if (CACHED_VALUE_NEEDS_REDRAW(heat_state, st7789_force_redraw)) {
      if (CACHED_VALUE_NEEDS_REDRAW(tip_type, st7789_force_redraw)) {
        if (CACHED_VALUE_GET(tip_type) == TIP_TYPE_NC) {
          st7889_set_font(inconsolata60);
          st7889_set_text_size(2);
          st7889_set_cursor(0, ST7789_MAIN_BASELINE);
          st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
          st7889_print("---");
          st7889_set_text_size(1);
        }

        ack_tip_type = true;
      }

      ack_heat_state = true;
    }

    if (CACHED_VALUE_GET(tip_type) != TIP_TYPE_NC) {
      int temperature;
      bool draw = false;

      if (tip_type != TIP_TYPE_WMRT) {
        if (CACHED_VALUE_NEEDS_REDRAW(right_temperature, st7789_force_redraw)) {
          temperature = CACHED_VALUE_GET(right_temperature);

          CACHED_VALUE_ACK(right_temperature);
          draw = true;
        }
      } else {
        if (CACHED_VALUE_NEEDS_REDRAW(right_temperature, st7789_force_redraw) ||
            CACHED_VALUE_NEEDS_REDRAW(left_temperature, st7789_force_redraw)) {
          temperature = (CACHED_VALUE_GET(right_temperature) + CACHED_VALUE_GET(left_temperature)) / 2;

          CACHED_VALUE_ACK(right_temperature);
          CACHED_VALUE_ACK(left_temperature);
          draw = true;
        }
      }

      if (draw) {
        st7889_set_font(inconsolata60);
        st7889_set_text_size(2);
        st7889_set_cursor(0, ST7789_MAIN_BASELINE);
        st7889_set_text_color(st7789_temperature_color(temperature), ST7789_BG_COLOR);
        char buffer[4];
        print_temperature(buffer, sizeof(buffer), temperature, false, true);
        st7889_print(buffer);
        st7889_set_text_size(1);
      }
    }
  }

  /* --- setpoint -------------------------------------------------------- */
  if (CACHED_VALUE_NEEDS_REDRAW(heat_setpoint, st7789_force_redraw)) {
    st7889_set_font(inconsolata24);
    st7889_set_cursor(ST7789_SETPOINT_LEFT, ST7789_SETPOINT_BOTTOM);
    st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
    char sp_buf[8];
    sp_buf[0] = CHAR_TARGET[0];
    print_temperature(&sp_buf[1], sizeof(sp_buf) - 1, CACHED_VALUE_GET(heat_setpoint), true, true);
    st7889_print(sp_buf);

    CACHED_VALUE_ACK(heat_setpoint);
  }

  /* --- reed state icon ------------------------------------------------- */
  if (CACHED_VALUE_NEEDS_REDRAW(reed_state, st7789_force_redraw)) {
    st7889_set_font(inconsolata48);
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 0, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
    st7889_print(REED_ICONS[CACHED_VALUE_GET(reed_state)]);

    CACHED_VALUE_ACK(reed_state);
  }

  /* --- setback icon ---------------------------------------------------- */
  if (CACHED_VALUE_NEEDS_REDRAW(heat_state, st7789_force_redraw)) {
    st7889_set_font(inconsolata48);
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 1, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
    st7889_print(SETBACK_ICONS[CACHED_VALUE_GET(heat_state)]);

    CACHED_VALUE_ACK(heat_state);
  }

  /* --- duty signal bar ------------------------------------------------- */
  if (CACHED_VALUE_NEEDS_REDRAW(right_duty, st7789_force_redraw) ||
      CACHED_VALUE_NEEDS_REDRAW(left_duty, st7789_force_redraw)) {
    int duty = MAX(CACHED_VALUE_GET(right_duty), CACHED_VALUE_GET(left_duty)) * 100 / MAX_DUTY_MAX;
    st7789_duty_signal(ST7789_RIGHT_COLUMN, ST7789_DUTY_BOTTOM, duty);

    CACHED_VALUE_ACK(right_duty);
    CACHED_VALUE_ACK(left_duty);
  }

  /* --- tip type icon + voltage ----------------------------------------- */
  if (CACHED_VALUE_NEEDS_REDRAW(tip_type, st7789_force_redraw)) {
    st7889_set_font(inconsolata48);
    st7889_set_cursor(ST7789_RIGHT_COLUMN + ST7789_RIGHT_COLUMN_X * 2, ST7789_RIGHT_COLUMN_BOTTOM);
    st7889_set_text_color(RGB565_WHITE, ST7789_BG_COLOR);
    st7889_print(TIP_ICONS[CACHED_VALUE_GET(tip_type)]);

    st7889_set_font(inconsolata24);
    st7889_set_cursor(ST7789_SETPOINT_LEFT, ST7789_VOLTAGE_BOTTOM);
    st7889_set_text_color(ST7789_VOLTAGE_COLOR, ST7789_BG_COLOR);
    st7889_print(TIP_VOLTAGES[CACHED_VALUE_GET(tip_type)]);

    ack_tip_type = true;
  }

  if (ack_heat_state) {
    CACHED_VALUE_ACK(heat_state);
  }
  if (ack_tip_type) {
    CACHED_VALUE_ACK(tip_type);
  }
  if (ack_blink) {
    CACHED_VALUE_ACK(blink);
  }
}

static void update_header(void) {
  /* --- header ----------------------------- */
  if (st7789_force_redraw) {
    const char *header = header_txt[display_state];
    st7889_fill_rect(0, 0, ST7789_SCREEN_WIDTH, ST7789_HEADER_HEIGHT, ST7789_HEADER_COLOR);
    st7889_draw_line(0, ST7789_HEADER_HEIGHT, ST7789_SCREEN_WIDTH, ST7789_HEADER_HEIGHT, RGB565_WHITE);

    int x = MAX(0, (ST7789_SCREEN_WIDTH - strlen(header) * ST7789_HEADER_CHARACTER_WIDTH) / 2);

    st7889_set_font(inconsolata24);
    st7889_set_cursor(x, ST7789_HEADER_BASELINE);
    st7889_set_text_color(RGB565_WHITE, RGB565_WHITE);
    st7889_print(header);
  }
}

static void update_view(const char *txt) {
  update_header();
  int txt_len = strlen(txt);
  char line[MAX_SCREEN_BUFFER_SIZE(ST7789_SCREEN_WIDTH, ST7789_VIEW_CHARACTER_WIDTH)];
  int x = MAX(0, ((sizeof(line) - 1) - txt_len) / 2);
  memset(line, CHAR_SPACE[0], x);
  int index = x;
  const int max = sizeof(line) - 1;
  while (index < max && *txt != '\0') {
    line[index++] = *txt++;
  }
  while (index < max) {
    line[index++] = CHAR_SPACE[0];
  }
  line[max] = '\0';

  st7889_set_font(inconsolata24);
  st7889_set_cursor(0, ST7789_VIEW_BASELINE);
  st7889_set_text_color(RGB565_WHITE, RGB565_WHITE);
  st7889_print(line);
}

static void update_menu(const char *const items[], unsigned int item_count, unsigned int selected_index) {
  /* --- header ----------------------------- */
  update_header();

  /* ---------------------------------------------------------- */
  /* Compute visible window                                     */
  /* ---------------------------------------------------------- */

  int scroll_offset = 0;

  if (item_count > ST7789_MENU_VISIBLE) {
    scroll_offset = selected_index - (ST7789_MENU_VISIBLE / 2);

    if (scroll_offset < 0)
      scroll_offset = 0;

    int max_offset = item_count - ST7789_MENU_VISIBLE;

    if (scroll_offset > max_offset)
      scroll_offset = max_offset;
  }

  /* ---------------------------------------------------------- */
  /* Draw rows                                                  */
  /* ---------------------------------------------------------- */

  st7889_set_font(inconsolata24);

  char line[MAX_SCREEN_BUFFER_SIZE(ST7789_SCREEN_WIDTH, ST7789_MENU_CHARACTER_WIDTH)];

  for (unsigned int row = 0; row < ST7789_MENU_VISIBLE; row++) {

    unsigned int item_index = scroll_offset + row;

    int y = ST7789_MENU_TOP + row * ST7789_MENU_ROW_HEIGHT;

    bool selected = (item_index == selected_index);

    uint16_t bg = selected ? ST7789_SELECTED_COLOR : RGB565_BLACK;

    uint16_t fg = selected ? RGB565_BLACK : RGB565_WHITE;

    st7889_set_cursor(0, y);

    st7889_set_text_color(fg, bg);

    int index = 0;
    const int max = sizeof(line) - 1;

    if (item_index < item_count) {
      line[index++] = selected ? CHAR_RIGHT[0] : CHAR_SPACE[0];
      line[index++] = CHAR_SPACE[0];

      const char *item = items[item_index];
      while (index < max && *item != '\0') {
        line[index++] = *item++;
      }
    }

    while (index < max) {
      line[index++] = CHAR_SPACE[0];
    }
    line[max] = '\0';
    st7889_print(line);
  }

  /* ---------------------------------------------------------- */
  /* Scroll indicators                                          */
  /* ---------------------------------------------------------- */

  st7889_set_text_color(RGB565_WHITE, RGB565_BLACK);

  /* up */
  st7889_set_cursor(ST7789_SCREEN_WIDTH - ST7789_HEADER_CHARACTER_WIDTH, ST7789_MENU_TOP);

  st7889_print((scroll_offset > 0) ? CHAR_UP : CHAR_SPACE);

  /* down */
  st7889_set_cursor(ST7789_SCREEN_WIDTH - ST7789_HEADER_CHARACTER_WIDTH,
                    ST7789_MENU_TOP + (ST7789_MENU_VISIBLE - 1) * ST7789_MENU_ROW_HEIGHT);

  st7889_print((scroll_offset + ST7789_MENU_VISIBLE < item_count) ? CHAR_DOWN : CHAR_SPACE);
}

static void update_main_menu(void) {
  if (!CACHED_VALUE_NEEDS_REDRAW(main_menu, st7789_force_redraw))
    return;

  update_menu(main_menu_txt, ARRAY_SIZE(main_menu_txt), CACHED_VALUE_GET(main_menu));

  CACHED_VALUE_ACK(main_menu);
}

static void update_diag_menu(void) {
  if (!CACHED_VALUE_NEEDS_REDRAW(diag_menu, st7789_force_redraw))
    return;

  update_menu(diag_menu_txt, ARRAY_SIZE(diag_menu_txt), CACHED_VALUE_GET(diag_menu));

  CACHED_VALUE_ACK(diag_menu);
}

static void screen_redraw(void) {
  char buf[32];
  CACHED_VALUE_SET(display_state, display_state);
  CACHED_VALUE_SET(main_menu, main_menu);
  CACHED_VALUE_SET(diag_menu, diag_menu);

  st7789_force_redraw = false;
  if (CACHED_VALUE_NEEDS_REDRAW(display_state, false)) {
    st7789_force_redraw = true;
    CACHED_VALUE_ACK(display_state);
  }

  if (st7789_force_redraw) {
    st7789_clear();

    /* After a full clear every region must repaint regardless of debounce */
    CACHED_VALUE_FORCE_DIRTY(blink);
    CACHED_VALUE_FORCE_DIRTY(tip_type);
    CACHED_VALUE_FORCE_DIRTY(reed_state);
    CACHED_VALUE_FORCE_DIRTY(heat_state);
    CACHED_VALUE_FORCE_DIRTY(right_duty);
    CACHED_VALUE_FORCE_DIRTY(left_duty);
    CACHED_VALUE_FORCE_DIRTY(right_temperature);
    CACHED_VALUE_FORCE_DIRTY(left_temperature);
    CACHED_VALUE_FORCE_DIRTY(heat_setpoint);

    CACHED_VALUE_FORCE_DIRTY(main_menu);
    CACHED_VALUE_FORCE_DIRTY(diag_menu);
  }

  switch (display_state) {
  case DISPLAY_STATE_MAIN:
    update_main();
    break;
  case DISPLAY_STATE_MAIN_MENU:
    update_main_menu();
    break;
  case DISPLAY_STATE_DIAG_MENU:
    update_diag_menu();
    break;
  case DISPLAY_STATE_ADJ_SETBACK:
    if (CACHED_VALUE_NEEDS_REDRAW(setback, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(setback), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(setback);
    }
    break;
  case DISPLAY_STATE_ADJ_SETBACK_DELAY:
    if (CACHED_VALUE_NEEDS_REDRAW(setback_delay, st7789_force_redraw)) {
      if (CACHED_VALUE_GET(setback_delay) == SETBACK_DELAY_MIN)
        update_view(OFF_TXT);
      else {
        snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(setback_delay), CHAR_SPACE[0], 0);
        update_view(buf);
      }

      CACHED_VALUE_ACK(setback_delay);
    }
    break;
  case DISPLAY_STATE_ADJ_STANDBY_DELAY:
    if (CACHED_VALUE_NEEDS_REDRAW(standby_delay, st7789_force_redraw)) {
      if (CACHED_VALUE_GET(standby_delay) == STANDBY_DELAY_MIN)
        update_view(OFF_TXT);
      else {
        snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(standby_delay), CHAR_SPACE[0], 0);
        update_view(buf);
      }

      CACHED_VALUE_ACK(standby_delay);
    }
    break;
  case DISPLAY_STATE_ADJ_OFFSET:
    if (CACHED_VALUE_NEEDS_REDRAW(temperature_offset, st7789_force_redraw)) {
      print_temperature(buf, sizeof(buf), CACHED_VALUE_GET(temperature_offset), true, false);
      update_view(buf);

      CACHED_VALUE_ACK(temperature_offset);
    }
    break;
  case DISPLAY_STATE_ADJ_UNIT:
    if (CACHED_VALUE_NEEDS_REDRAW(temperature_unit, st7789_force_redraw)) {
      update_view((CACHED_VALUE_GET(temperature_unit) == TEMPERATURE_UNIT_C) ? CELSIUS_SUFFIX : FAHRENHEIT_SUFFIX);

      CACHED_VALUE_ACK(temperature_unit);
    }
    break;
  case DISPLAY_STATE_ADJ_STEP_SIZE:
    if (CACHED_VALUE_NEEDS_REDRAW(step_size, st7789_force_redraw)) {
      print_temperature(buf, sizeof(buf), CACHED_VALUE_GET(step_size), true, false);
      update_view(buf);

      CACHED_VALUE_ACK(step_size);
    }
    break;
  case DISPLAY_STATE_SHOW_COLD_COMPENSATION:
    if (CACHED_VALUE_NEEDS_REDRAW(kty_value, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(kty_value), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(kty_value);
    }
    break;
  case DISPLAY_STATE_ADJ_REFERENCE:
    if (CACHED_VALUE_NEEDS_REDRAW(reference, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(reference), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(reference);
    }
    break;
  case DISPLAY_STATE_SHOW_TIP_TYPE:
    if (CACHED_VALUE_NEEDS_REDRAW(tip_type, st7789_force_redraw)) {
      update_view(tip_txt[CACHED_VALUE_GET(tip_type)]);

      CACHED_VALUE_ACK(tip_type);
    }
    break;
  case DISPLAY_STATE_SHOW_REED_STATE:
    if (CACHED_VALUE_NEEDS_REDRAW(reed_state, st7789_force_redraw)) {
      update_view(reed_txt[CACHED_VALUE_GET(reed_state)]);
      CACHED_VALUE_ACK(reed_state);
    }
    break;
  case DISPLAY_STATE_SHOW_FREQUENCY:
    if (CACHED_VALUE_NEEDS_REDRAW(main_period, st7789_force_redraw)) {
      int freq = (int)((1000.0f / 2.0f) / (float)CACHED_VALUE_GET(main_period));
      int written = snprint_int(buf, sizeof(buf), freq, CHAR_SPACE[0], 0);
      strncat(&buf[written], FREQUENCY_SUFFIX, sizeof(buf) - written - 1);
      update_view(buf);

      CACHED_VALUE_ACK(main_period);
    }
    break;
  case DISPLAY_STATE_SHOW_TC_1_READING:
    if (CACHED_VALUE_NEEDS_REDRAW(tc_right_temperature, st7789_force_redraw)) {
      print_temperature(buf, sizeof(buf), CACHED_VALUE_GET(tc_right_temperature), true, true);
      update_view(buf);

      CACHED_VALUE_ACK(tc_right_temperature);
    }
    break;
  case DISPLAY_STATE_SHOW_TC_2_READING:
    if (CACHED_VALUE_NEEDS_REDRAW(tc_left_temperature, st7789_force_redraw)) {
      print_temperature(buf, sizeof(buf), CACHED_VALUE_GET(tc_left_temperature), true, true);
      update_view(buf);

      CACHED_VALUE_ACK(tc_left_temperature);
    }
    break;
  case DISPLAY_STATE_SHOW_PWM_1_READING:
    if (CACHED_VALUE_NEEDS_REDRAW(right_duty, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(right_duty), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(right_duty);
    }
    break;
  case DISPLAY_STATE_SHOW_PWM_2_READING:
    if (CACHED_VALUE_NEEDS_REDRAW(left_duty, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(left_duty), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(left_duty);
    }
    break;
  case DISPLAY_STATE_ADJ_IDLE_DUTY:
    if (CACHED_VALUE_NEEDS_REDRAW(idle_duty, st7789_force_redraw)) {
      if (CACHED_VALUE_GET(idle_duty) == 0)
        update_view(OFF_TXT);
      else {
        snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(idle_duty), CHAR_SPACE[0], 0);
        update_view(buf);
      }

      CACHED_VALUE_ACK(idle_duty);
    }
    break;
  case DISPLAY_STATE_ADJ_MAX_DUTY:
    if (CACHED_VALUE_NEEDS_REDRAW(max_duty, st7789_force_redraw)) {
      snprint_int(buf, sizeof(buf), CACHED_VALUE_GET(max_duty), CHAR_SPACE[0], 0);
      update_view(buf);

      CACHED_VALUE_ACK(max_duty);
    }
    break;
  case DISPLAY_STATE_ADJ_POOR:
    if (CACHED_VALUE_NEEDS_REDRAW(poor_mode, st7789_force_redraw)) {
      update_view(CACHED_VALUE_GET(poor_mode) ? ON_TXT : OFF_TXT);

      CACHED_VALUE_ACK(poor_mode);
    }
    break;
  case DISPLAY_STATE_SHOW_FW_VERSION:
    if (st7789_force_redraw) {
      int written = 0;
      written += snprint_int(&buf[written], sizeof(buf) - written, (FW_VERSION / 100) % 100, '0', 2);
      if (sizeof(buf) - written > 1) {
        buf[written++] = '.';
      }
      written += snprint_int(&buf[written], sizeof(buf) - written, FW_VERSION % 100, '0', 2);
      update_view(buf);
    }
    break;
  }
}

void st7789_coru(void *param) {
  UNUSED(param);

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
    screen_redraw();

    yield();
  }
}

static void st7789_init_cached_values(void) {
  /* SET once to capture the current live value and mark dirty=true */
  CACHED_VALUE_SET(display_state, display_state);
  CACHED_VALUE_SET(blink, systick_get());
  CACHED_VALUE_SET(tip_type, tip_type);
  CACHED_VALUE_SET(reed_state, reed_state);
  CACHED_VALUE_SET(heat_state, heat_state);
  CACHED_VALUE_SET(right_duty, right_duty);
  CACHED_VALUE_SET(left_duty, left_duty);
  CACHED_VALUE_SET(right_temperature, right_temperature);
  CACHED_VALUE_SET(left_temperature, left_temperature);
  CACHED_VALUE_SET(heat_setpoint, heat_setpoint);
  CACHED_VALUE_SET(setback, setback);
  CACHED_VALUE_SET(setback_delay, setback_delay);
  CACHED_VALUE_SET(standby_delay, standby_delay);
  CACHED_VALUE_SET(temperature_offset, temperature_offset);
  CACHED_VALUE_SET(temperature_unit, temperature_unit);
  CACHED_VALUE_SET(step_size, step_size);
  CACHED_VALUE_SET(kty_value, kty_value);
  CACHED_VALUE_SET(reference, reference);
  CACHED_VALUE_SET(tc_right_temperature, tc_right_temperature);
  CACHED_VALUE_SET(tc_left_temperature, tc_left_temperature);
  CACHED_VALUE_SET(idle_duty, idle_duty);
  CACHED_VALUE_SET(max_duty, max_duty);
  CACHED_VALUE_SET(poor_mode, poor_mode);
  CACHED_VALUE_SET(main_period, main_period);
  CACHED_VALUE_SET(main_menu, main_menu);
  CACHED_VALUE_SET(diag_menu, diag_menu);

  /* Force dirty so the very first paint always runs */
  CACHED_VALUE_FORCE_DIRTY(display_state);
  CACHED_VALUE_FORCE_DIRTY(blink);
  CACHED_VALUE_FORCE_DIRTY(tip_type);
  CACHED_VALUE_FORCE_DIRTY(reed_state);
  CACHED_VALUE_FORCE_DIRTY(heat_state);
  CACHED_VALUE_FORCE_DIRTY(right_duty);
  CACHED_VALUE_FORCE_DIRTY(left_duty);
  CACHED_VALUE_FORCE_DIRTY(right_temperature);
  CACHED_VALUE_FORCE_DIRTY(left_temperature);
  CACHED_VALUE_FORCE_DIRTY(heat_setpoint);
  CACHED_VALUE_FORCE_DIRTY(setback);
  CACHED_VALUE_FORCE_DIRTY(setback_delay);
  CACHED_VALUE_FORCE_DIRTY(standby_delay);
  CACHED_VALUE_FORCE_DIRTY(temperature_offset);
  CACHED_VALUE_FORCE_DIRTY(temperature_unit);
  CACHED_VALUE_FORCE_DIRTY(step_size);
  CACHED_VALUE_FORCE_DIRTY(kty_value);
  CACHED_VALUE_FORCE_DIRTY(reference);
  CACHED_VALUE_FORCE_DIRTY(tc_right_temperature);
  CACHED_VALUE_FORCE_DIRTY(tc_left_temperature);
  CACHED_VALUE_FORCE_DIRTY(idle_duty);
  CACHED_VALUE_FORCE_DIRTY(max_duty);
  CACHED_VALUE_FORCE_DIRTY(poor_mode);
  CACHED_VALUE_FORCE_DIRTY(main_period);
  CACHED_VALUE_FORCE_DIRTY(main_menu);
  CACHED_VALUE_FORCE_DIRTY(diag_menu);
}

void st7789_init(void) {
  st7789_force_redraw = true;

  main_period_acc = 0;
  right_temperature_acc = left_temperature_acc = 0;
  right_temperature_filtred = 0;
  left_temperature_filtred = 0;

  previous_temperature_update = systick_get();

  st7789_init_cached_values();

  tick_timer_init(&delay_timer);
#ifndef CORU_DISABLED
  coru_create_inplace(&co, st7789_coru, NULL, co_stack, sizeof(co_stack));
#else
  st7789_coru(NULL);
#endif
}

void st7789_loop(void) {
  CACHED_VALUE_SET(blink, systick_get() / (ST7789_BLINK_DELAY / 2));

  if (new_acquisition) {
    IIR_FILTER_ADD(ST7789_MAIN_PERIOD_IIR_WINDOW, main_period_acc, main_period);
    int main_period_acc_filtred = IIR_FILTER_GET(ST7789_MAIN_PERIOD_IIR_WINDOW, main_period_acc);

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

    CACHED_VALUE_SET(tip_type, tip_type);
    CACHED_VALUE_SET(reed_state, reed_state);
    CACHED_VALUE_SET(heat_state, heat_state);
    CACHED_VALUE_SET(right_duty, right_duty);
    CACHED_VALUE_SET(left_duty, left_duty);
    CACHED_VALUE_SET(right_temperature, right_temperature_filtred);
    CACHED_VALUE_SET(left_temperature, left_temperature_filtred);
    CACHED_VALUE_SET(heat_setpoint, heat_setpoint);
    CACHED_VALUE_SET(setback, setback);
    CACHED_VALUE_SET(setback_delay, setback_delay);
    CACHED_VALUE_SET(standby_delay, standby_delay);
    CACHED_VALUE_SET(temperature_offset, temperature_offset);
    CACHED_VALUE_SET(temperature_unit, temperature_unit);
    CACHED_VALUE_SET(step_size, step_size);
    CACHED_VALUE_SET(kty_value, (int)kty_value);
    CACHED_VALUE_SET(reference, reference);
    CACHED_VALUE_SET(tc_right_temperature, (int)tc_right_temperature);
    CACHED_VALUE_SET(tc_left_temperature, (int)tc_left_temperature);
    CACHED_VALUE_SET(idle_duty, idle_duty);
    CACHED_VALUE_SET(max_duty, max_duty);
    CACHED_VALUE_SET(poor_mode, poor_mode);
    CACHED_VALUE_SET(main_period, main_period_acc_filtred);
  }

#ifndef CORU_DISABLED
  coru_resume(&co);
#endif
}

#endif
