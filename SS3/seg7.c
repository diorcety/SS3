#include "seg7.h"

#if defined(USE_SEG7)

#include "acquisition.h"
#include "board.h"
#include "configuration.h"
#include "display.h"
#include "heat.h"
#include "iron.h"
#include "pt.h"
#include "tick.h"
#include "tip.h"
#include "util.h"
#include "zcd.h"

#include <stdlib.h>
#include <string.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

/* Refresh rate */
static const int REFRESH_HZ = 50;
static const int SPI_PRESCALER = DL_SPI_CLOCK_DIVIDE_RATIO_6;

/*
 * 8 segments, 50 times by seconds = 400 transactions by seconds
 * 1 transaction = 16 bits => outputBitRate = 6400
 * Set the bit rate clock divider to generate the serial output clock
 *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
 *     outputBitRate*2 = (spiInputClock) / (1 + SCR)
 *     (1 + SCR) = (spiInputClock) / (outputBitRate*2)
 *     SCR = spiInputClock / (outputBitRate*2) - 1
 */
static const int SPI_OUTPUT_BIT_RATE = REFRESH_HZ * 16 * 8;
static const int SPI_INPUT_CLOCK = CPUCLK_FREQ / (SPI_PRESCALER + 1);
static const int SPI_SPEED = SPI_INPUT_CLOCK / (2 * SPI_OUTPUT_BIT_RATE) - 1;

static const int MAIN_PERIOD_IIR_WINDOW = 8;
static const int LEFT_TEMPERATURE_IIR_WINDOW = 8;
static const int RIGHT_TEMPERATURE_IIR_WINDOW = 8;

static const int SEG7_SETPOINT_DELAY = 1000;
static const int SEG7_BLINK_DELAY = 1028;
static const int SEG7_TEMPERATURE_SLOW_UPDATE_MS = 1000;
static const int SEG7_TEMPERATURE_SLOW_THRESHOLD_DEG = 2;

typedef enum {
  DISPLAY_MODE_C,
  DISPLAY_MODE_F,
  DISPLAY_MODE_NUM,
  DISPLAY_MODE_REF,
  DISPLAY_MODE_VERSION,
} DisplayMode;

// #define SEG7_COMMON_CATHODE

/* Character constants */
#define _SPACE 0x00
#define _MINUS 0x40
#define _CELSIUS 0x39
#define _DEGREE 0x04
#define _COLON 0x01
#define _POINT 0x80

#define _0P 0xbf
#define _0 0x3f
#define _1 0x06
#define _2 0x5b
#define _3 0x4f
#define _4 0x66
#define _5 0x6d
#define _6 0x7d
#define _7 0x07
#define _8 0x7f
#define _9 0x6f

#define _A 0x77
#define _B 0x7c
#define _C 0x58
#define _D 0x5e
#define _E 0x79
#define _F 0x71
#define _G 0x3d
#define _H 0x74
#define _I 0x04
#define _J 0x1e
#define _K 0x76
#define _L 0x38
#define _M 0x15
#define _N 0x54
#define _O 0x5c
#define _P 0x73
#define _Q 0x67
#define _R 0x50
#define _S 0x6d
#define _T 0x78
#define _U 0x3e
#define _V 0x1c
#define _W 0x2a
#define _X 0x76
#define _Y 0x6e
#define _Z 0x5b

static uint8_t const cold[] = {_C, _O, _L, _D, _SPACE};       //  CoLd
static uint8_t const stby[] = {_S, _T, _B, _Y, _SPACE};       //  Stby
static uint8_t const setb[] = {_S, _E, _T, _B, _SPACE};       //  SEtb
static uint8_t const bacc[] = {_B, _A, _C, _C, _SPACE};       //  bAcc
static uint8_t const dly1[] = {_D, _L, _Y, _1, _SPACE};       //  dLy1
static uint8_t const dly2[] = {_D, _L, _Y, _2, _SPACE};       //  dLy2
static uint8_t const off[] = {_SPACE, _O, _F, _F, _SPACE};    //  OFF
static uint8_t const ofse[] = {_O, _F, _S, _E, _SPACE};       //  oFSE
static uint8_t const unit[] = {_U, _N, _I, _T, _SPACE};       //  Unit
static uint8_t const step[] = {_S, _T, _E, _P, _SPACE};       //  StEP
static uint8_t const diag[] = {_D, _I, _A, _G, _SPACE};       //  diAG
static uint8_t const ref[] = {_SPACE, _R, _E, _F, _SPACE};    //  rEF
static uint8_t const type[] = {_T, _Y, _P, _E, _SPACE};       //  tyPE
static uint8_t const wmrp[] = {_W, _M, _R, _P, _SPACE};       //  WMrP
static uint8_t const wmup[] = {_W, _M, _U, _P, _SPACE};       //  WMuP
static uint8_t const wmrt[] = {_W, _M, _R, _T, _SPACE};       //  WMrt
static uint8_t const nc[] = {_SPACE, _N, _C, _SPACE, _SPACE}; //  nC
static uint8_t const reed[] = {_R, _E, _E, _D, _SPACE};       //  rEEd
static uint8_t const open[] = {_O, _P, _E, _N, _SPACE};       //  oPEn
static uint8_t const clos[] = {_C, _L, _O, _S, _SPACE};       //  CLoS
static uint8_t const tc_1[] = {_T, _C, _SPACE, _1, _SPACE};   //  tC 1
static uint8_t const tc_2[] = {_T, _C, _SPACE, _2, _SPACE};   //  tC 2
static uint8_t const pwm1[] = {_D, _C, _SPACE, _1, _SPACE};   //  dC 1
static uint8_t const pwm2[] = {_D, _C, _SPACE, _2, _SPACE};   //  dC 2
static uint8_t const idle[] = {_I, _D, _L, _E, _SPACE};       //  idLE
static uint8_t const max[] = {_SPACE, _M, _A, _X, _SPACE};    //  mAX
static uint8_t const poor[] = {_P, _O, _O, _R, _SPACE};       //  Poor
static uint8_t const on[] = {_SPACE, _SPACE, _O, _N, _SPACE}; // on
static uint8_t const freq[] = {_F, _R, _E, _Q, _SPACE};       //	FrEQ
static uint8_t const vers[] = {_V, _E, _R, _S, _SPACE};       //  vErS

static uint8_t const hex_table[] = {_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F};

static uint8_t const *main_menu_txt[] = {
    [MAIN_MENU_BACK] = bacc,
    [MAIN_MENU_SETBACK] = setb,
    [MAIN_MENU_SETBACK_DELAY] = dly1,
    [MAIN_MENU_STANDBY_DELAY] = dly2,
    [MAIN_MENU_OFFSET] = ofse,
    [MAIN_MENU_UNIT] = unit,
    [MAIN_MENU_STEP_SIZE] = step,
    [MAIN_MENU_DIAG] = diag,
};

static uint8_t const *diag_menu_txt[] = {
    [DIAG_MENU_BACK] = bacc,
    [DIAG_MENU_COLD_COMPENSATION] = cold,
    [DIAG_MENU_REFERENCE] = ref,
    [DIAG_MENU_TIP_TYPE] = type,
    [DIAG_MENU_REED_STATE] = reed,
    [DIAG_MENU_SHOW_FREQUENCY] = freq,
    [DIAG_MENU_TC_1_READING] = tc_1,
    [DIAG_MENU_TC_2_READING] = tc_2,
    [DIAG_MENU_PWM_1_READING] = pwm1,
    [DIAG_MENU_PWM_2_READING] = pwm2,
    [DIAG_MENU_IDLE_DUTY] = idle,
    [DIAG_MENU_MAX_DUTY] = max,
    [DIAG_MENU_POOR] = poor,
    [DIAG_MENU_FW_VERSION] = vers,
};

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

static struct pt seg7_process_pt;

static volatile bool trigger;

static timer_t delay_timer;
static timer_t current_setpoint_change_timer;
static int previous_setpoint;

static int main_period_acc;
static int right_temperature_acc;
static int right_temperature_filtred;
static int left_temperature_acc;
static int left_temperature_filtred;
static uint32_t previous_temperature_update;

static uint8_t display[5];
static uint8_t current_index;
static uint16_t spi_data;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static inline int SEG_DIG(uint8_t *display, int DIG, int SEG) { return (((display[DIG] >> SEG) & 0x1) << DIG); }

#ifndef SEG7_COMMON_CATHODE
static inline int SPI_DIG(uint8_t *display, int SEG) {
  return (SEG_DIG(display, 0, SEG) | SEG_DIG(display, 1, SEG) | SEG_DIG(display, 2, SEG) | SEG_DIG(display, 3, SEG) |
          SEG_DIG(display, 4, SEG));
}

static inline int SPI_SEG(uint8_t *display, int SEG) {
  UNUSED(display);

  return (1 << SEG) ^ 0xFF;
}
#else
static inline int SPI_DIG(uint8_t *display, int SEG) {
  return (SEG_DIG(display, 0, SEG) | SEG_DIG(display, 1, SEG) | SEG_DIG(display, 2, SEG) | SEG_DIG(display, 3, SEG) |
          SEG_DIG(display, 4, SEG)) ^
         0xFF;
}

static inline int SPI_SEG(uint8_t *display, int SEG) { return (1 << SEG); }
#endif

static inline void display_text(uint8_t const *txt) {
  if (txt != NULL) {
    memcpy(display, txt, sizeof(display));
  }
}

static inline DisplayMode temp_unit(void) {
  return ((TemperatureUnit)temperature_unit == TEMPERATURE_UNIT_C) ? DISPLAY_MODE_C : DISPLAY_MODE_F;
}

void display_number(DisplayMode display_mode, int num) {
  int j; // Digit index

  switch (display_mode) {
  case DISPLAY_MODE_C:
    display[3] = _CELSIUS;
    display[4] = _DEGREE;
    j = 2;
    break;
  case DISPLAY_MODE_F:
    display[3] = _F;
    display[4] = _DEGREE;
    num = num * 9 / 5;
    if (display_state != DISPLAY_STATE_ADJ_OFFSET && display_state != DISPLAY_STATE_ADJ_STEP_SIZE)
      num = num + 32;
    if (num > 999)
      num = 999;
    j = 2;
    break;
  case DISPLAY_MODE_NUM:
  case DISPLAY_MODE_REF:
  case DISPLAY_MODE_VERSION:
    display[4] = _SPACE;
    j = 3;
    break;
  }

  int temp = abs(num);
  bool has_sign = false;

  while (j >= 0) {
    if (temp == 0 && (display_mode != DISPLAY_MODE_REF && display_mode != DISPLAY_MODE_VERSION)) {
      if (has_sign) {
        display[j] = _SPACE;
      } else {
        display[j] = (num < 0) ? _MINUS : _SPACE;
        has_sign = true;
      }
    } else {
      display[j] = hex_table[temp % 10];
    }

    temp /= 10;
    j--;
  }

  if (display_mode == DISPLAY_MODE_REF) {
    display[0] |= _POINT;
  }
  if (display_mode == DISPLAY_MODE_VERSION) {
    display[1] |= _POINT;
  }
}

void update_main_menu_display(void) {
  if (main_menu < ARRAY_SIZE(main_menu_txt)) {
    display_text(main_menu_txt[main_menu]);
  }
}

void update_menu_diag_display(void) {
  if (diag_menu < ARRAY_SIZE(diag_menu_txt)) {
    display_text(diag_menu_txt[diag_menu]);
  }
}

void update_display(void) {
  switch (display_state) {
  case DISPLAY_STATE_MAIN:
    if (heat_state == HEAT_STATE_STANDBY) {
      display_text(stby);
    } else if (tip_type == TIP_TYPE_NC) {
      display_text(nc);
    } else if (timer_is_running(&current_setpoint_change_timer, true)) {
      display_number(temp_unit(), heat_setpoint);
    } else if (tip_type != TIP_TYPE_WMRT) {
      display_number(temp_unit(), right_temperature_filtred);
    } else {
      display_number(temp_unit(), (left_temperature_filtred + right_temperature_filtred) / 2);
    }
    if (right_heat) {
      display[3] |= _POINT;
    }
    if (left_heat) {
      display[0] |= _POINT;
    }
    if (reed_state == REED_STATE_CLOSED) {
      display[1] |= _POINT;
    }

    // Blink when not in normal state
    if (heat_state != HEAT_STATE_NORMAL && (systick_counter % SEG7_BLINK_DELAY) < (SEG7_BLINK_DELAY / 2)) {
      display[4] |= _COLON;
    }
    break;

  case DISPLAY_STATE_MAIN_MENU:
    update_main_menu_display();
    break;

  case DISPLAY_STATE_ADJ_SETBACK:
    display_number(temp_unit(), setback);
    break;

  case DISPLAY_STATE_ADJ_SETBACK_DELAY:
    if (setback_delay == SETBACK_DELAY_MIN) {
      display_text(off);
    } else {
      display_number(DISPLAY_MODE_NUM, setback_delay);
    }
    break;

  case DISPLAY_STATE_ADJ_STANDBY_DELAY:
    if (standby_delay == STANDBY_DELAY_MIN) {
      display_text(off);
    } else {
      display_number(DISPLAY_MODE_NUM, standby_delay);
    }
    break;

  case DISPLAY_STATE_ADJ_OFFSET:
    display_number(temp_unit(), temperature_offset);
    break;

  case DISPLAY_STATE_ADJ_UNIT:
    display_number(temp_unit(), 0);
    break;

  case DISPLAY_STATE_ADJ_STEP_SIZE:
    display_number(temp_unit(), step_size);
    break;

  case DISPLAY_STATE_DIAG_MENU:
    update_menu_diag_display();
    break;

  case DISPLAY_STATE_SHOW_COLD_COMPENSATION:
    display_number(temp_unit(), kty_value);
    break;

  case DISPLAY_STATE_ADJ_REFERENCE:
    display_number(DISPLAY_MODE_REF, reference);
    break;

  case DISPLAY_STATE_SHOW_TIP_TYPE:
    switch (tip_type) {
    case TIP_TYPE_WMUP:
      display_text(wmup);
      break;
    case TIP_TYPE_WMRP:
      display_text(wmrp);
      break;
    case TIP_TYPE_WMRT:
      display_text(wmrt);
      break;
    case TIP_TYPE_NC:
      display_text(nc);
      break;
    }
    break;

  case DISPLAY_STATE_SHOW_REED_STATE:
    switch (reed_state) {
    case REED_STATE_OPENED:
      display_text(open);
      break;
    case REED_STATE_CLOSED:
      display_text(clos);
      break;
    }
    break;

  case DISPLAY_STATE_SHOW_FREQUENCY: {
    int main_frequency = (int)((1000.0f / 2.0f) / (float)IIR_FILTER_GET(MAIN_PERIOD_IIR_WINDOW, main_period_acc));
    display_number(DISPLAY_MODE_NUM, main_frequency);
  } break;

  case DISPLAY_STATE_SHOW_TC_1_READING:
    display_number(temp_unit(), right_temperature);
    break;

  case DISPLAY_STATE_SHOW_TC_2_READING:
    display_number(temp_unit(), left_temperature);
    break;

  case DISPLAY_STATE_SHOW_PWM_1_READING:
    display_number(DISPLAY_MODE_NUM, right_duty);
    break;

  case DISPLAY_STATE_SHOW_PWM_2_READING:
    display_number(DISPLAY_MODE_NUM, left_duty);
    break;

  case DISPLAY_STATE_ADJ_IDLE_DUTY:
    if (idle_duty == 0) {
      display_text(off);
    } else {
      display_number(DISPLAY_MODE_NUM, idle_duty);
    }
    break;

  case DISPLAY_STATE_ADJ_MAX_DUTY:
    display_number(DISPLAY_MODE_NUM, max_duty);
    break;

  case DISPLAY_STATE_ADJ_POOR:
    if (poor_mode) {
      display_text(on);
    } else {
      display_text(off);
    }
    break;

  case DISPLAY_STATE_SHOW_FW_VERSION:
    display_number(DISPLAY_MODE_VERSION, FW_VERSION);
    break;
  }
}

void seg7_init(void) {
  pt_reset(&seg7_process_pt);
  timer_init(&delay_timer);
  timer_init(&current_setpoint_change_timer);

  trigger = false;
  previous_setpoint = heat_setpoint;
  current_index = 0;
  main_period_acc = right_temperature_acc = left_temperature_acc = 0;
  right_temperature_filtred = 0;
  left_temperature_filtred = 0;
  previous_temperature_update = systick_get();
  display_text(nc);

  SPI_0_INST->CLKDIV = SPI_PRESCALER;
  _Static_assert(SPI_SPEED >= 0 && SPI_SPEED <= SPI_CLKCTL_SCR_MASK,
                 "SPI_SPEED out of range (must be between 0 and" TOSTRING(SPI_CLKCTL_SCR_MASK) ")");
  DL_SPI_setBitRateSerialClockDivider(SPI_0_INST, SPI_SPEED);
}

void seg7_send(void) {
  // Generate SPI frame
  spi_data = SPI_SEG(display, current_index) << 0 | SPI_DIG(display, current_index) << 8;

  DL_SPI_transmitData16(SPI_0_INST, spi_data);
}

void seg7_update(void) {
  if (current_index == 0) {
    // Refresh the data to display
    update_display();
  }

  seg7_send();

  current_index = (current_index + 1) % 8;
}

void seg7_loop(void) {
  NVIC_EnableIRQ(SPI_0_INST_INT_IRQN);

  if (new_acquisition) {
    IIR_FILTER_ADD(MAIN_PERIOD_IIR_WINDOW, main_period_acc, main_period);
    IIR_FILTER_ADD(RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc, right_temperature);
    IIR_FILTER_ADD(LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc, left_temperature);

    // Get filtred value
    int tmp_right_temperature_filtred = IIR_FILTER_GET(RIGHT_TEMPERATURE_IIR_WINDOW, right_temperature_acc);
    int tmp_left_temperature_filtred = IIR_FILTER_GET(LEFT_TEMPERATURE_IIR_WINDOW, left_temperature_acc);

    // Only update above threshold or after some delay: avoid flikering
    if (abs(tmp_right_temperature_filtred - right_temperature_filtred) >= SEG7_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        abs(tmp_left_temperature_filtred - left_temperature_filtred) >= SEG7_TEMPERATURE_SLOW_THRESHOLD_DEG ||
        systick_elapsed(previous_temperature_update, SEG7_TEMPERATURE_SLOW_UPDATE_MS)) {
      right_temperature_filtred = tmp_right_temperature_filtred;
      left_temperature_filtred = tmp_left_temperature_filtred;
      previous_temperature_update = systick_get();
    }
  }

  //
  // Start of asynchronous part
  //

  pt_begin(&seg7_process_pt);

  DL_GPIO_setPins(GPIOA, Screen_Reset_PIN | Screen_BL_PIN);
  timer_start(&delay_timer, 1, true); /* 1 ms */
  pt_wait(&seg7_process_pt, timer_elapsed(&delay_timer));
  DL_GPIO_clearPins(GPIOA, Screen_Reset_PIN);

  for (;;) {
    // Wait trigger
    pt_wait(&seg7_process_pt, trigger);
    trigger = false;

    // Detect setpoint changes
    if (heat_setpoint != previous_setpoint) {
      previous_setpoint = heat_setpoint;
      timer_start(&current_setpoint_change_timer, SEG7_SETPOINT_DELAY, true);
    }

    seg7_update();
  }

  pt_end(&seg7_process_pt);

  //
  // End of asynchronous part
  //
}

void SPI_0_INST_IRQHandler(void) {
  switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
  case DL_SPI_IIDX_IDLE:
    trigger = true;
    break;
  default:
    break;
  }
}

#endif
