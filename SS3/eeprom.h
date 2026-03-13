#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdbool.h>
#include <stdint.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void eeprom_init(void);

bool eeprom_read(uint16_t variable_id, uint32_t *value);
bool eeprom_write(uint16_t variable_id, uint32_t value);

void eeprom_loop(void);

#endif // EEPROM_H_
