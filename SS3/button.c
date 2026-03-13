#include "button.h"

#include "board.h"
#include "tick.h"

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static const int BUTTONS_LONG_PRESS_DURATION = 1000;
static const int BUTTONS_DEBOUNCE_TIMEOUT = 20;
static const int BUTTONS_ROTATION_EDGES_COUNT = 4;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

static volatile int rotation;

static volatile timer_t press_timer;
static volatile bool press_active;

static Event event;
static uint32_t pins_previous_state;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/
void button_init(void) {
  rotation = 0;
  timer_init(&press_timer);
  press_active = false;
  event = EVENT_NONE;

  pins_previous_state = DL_GPIO_readPins(GPIOA, Buttons_A_PIN | Buttons_B_PIN);

  DL_GPIO_clearInterruptStatus(GPIOA, Buttons_A_PIN | Buttons_B_PIN | Buttons_P_PIN);
  NVIC_ClearPendingIRQ(Buttons_INT_IRQN);
}

void button_loop(void) {
  event = EVENT_NONE;

  NVIC_DisableIRQ(Buttons_INT_IRQN);

  if (timer_is_running(&press_timer, false)) {
    rotation = 0; // Cancel rotation if a press is in progress
    if (timer_elapsed(&press_timer)) {
      event = EVENT_LONG_PRESS;
    } else if (!press_active) {
      if (timer_stop(&press_timer) >= BUTTONS_DEBOUNCE_TIMEOUT) {
        event = EVENT_PRESS;
      }
    }
  } else {
    if (rotation <= -BUTTONS_ROTATION_EDGES_COUNT) {
      rotation = 0;
      event = EVENT_LEFT;
    } else if (rotation >= BUTTONS_ROTATION_EDGES_COUNT) {
      rotation = 0;
      event = EVENT_RIGHT;
    }
  }

  NVIC_EnableIRQ(Buttons_INT_IRQN);
}

Event button_get_event(void) { return event; }

void Buttons_INST_IRQHandler(void) {
  uint32_t pins_current_state = DL_GPIO_readPins(GPIOA, Buttons_A_PIN | Buttons_B_PIN | Buttons_P_PIN);
  DL_GPIO_clearInterruptStatus(GPIOA, Buttons_A_PIN | Buttons_B_PIN | Buttons_P_PIN);

  bool new_a = (pins_current_state & Buttons_A_PIN);
  bool new_b = (pins_current_state & Buttons_B_PIN);
  bool prev_a = (pins_previous_state & Buttons_A_PIN);
  bool prev_b = (pins_previous_state & Buttons_B_PIN);

  uint32_t changed = pins_current_state ^ pins_previous_state;

  // --- Encoder A changed ---
  if (changed & Buttons_A_PIN) {
    bool comb_state = prev_b ^ new_a;
    rotation += comb_state ? +1 : -1;
  }

  // --- Encoder B changed ---
  if (changed & Buttons_B_PIN) {
    bool comb_state = prev_a ^ new_b;
    rotation += comb_state ? -1 : +1;
  }

  // --- Button P changed ---
  if (changed & Buttons_P_PIN) {
    if (pins_current_state & Buttons_P_PIN) {
      timer_start(&press_timer, BUTTONS_LONG_PRESS_DURATION, false);
      press_active = true;
    } else {
      press_active = false;
    }
  }

  pins_previous_state = pins_current_state;
}
