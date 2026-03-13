#ifndef ACQUISITION_H_
#define ACQUISITION_H_

#include <stdbool.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                   DATA                                                            *
 *                                                                                                                   *
 *********************************************************************************************************************/

extern int right_temperature;
extern int left_temperature;
extern int reed_value;
extern int kty_value;

extern bool new_acquisition;

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/
void acquisition_init(void);
bool acquisition_trigger(void);
void acquisition_loop(void);

#endif // ACQUISITION_H_
