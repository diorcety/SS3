#ifndef TIP_H_
#define TIP_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  TIP_TYPE_NC,
  TIP_TYPE_WMUP,
  TIP_TYPE_WMRP,
  TIP_TYPE_WMRT
} TipType;

typedef enum {
  REED_STATE_CLOSED,
  REED_STATE_OPENED,
} ReedState;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

extern TipType tip_type;
extern ReedState reed_state;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void tip_init(void);
void tip_loop(void);

static inline bool tip_has_left(void) { return tip_type == TIP_TYPE_WMRT; }
static inline bool tip_has_right(void) { return tip_type != TIP_TYPE_NC; }

#endif // TIP_H_
