#include "eeprom.h"
#include "main.h"
#include "util.h"

#ifndef SIMULATION
#include <ti/eeprom/emulation_type_b/eeprom_emulation_type_b.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

void eeprom_init(void) {
  /* Initialize */
  uint32_t EEPROMEmulationState = EEPROM_TypeB_init();
  if (EEPROMEmulationState == EEPROM_EMULATION_INIT_ERROR || EEPROMEmulationState == EEPROM_EMULATION_TRANSFER_ERROR) {
    ERROR_HANDLER();
  }
}

bool eeprom_read(uint16_t variable_id, uint32_t *value) {
  uint32_t var_data_read = EEPROM_TypeB_readDataItem(variable_id);
  if (gEEPROMTypeBSearchFlag == 0) {
    return false;
  }
  *value = var_data_read;
  return true;
}

bool eeprom_write(uint16_t variable_id, uint32_t value) {
  uint32_t old_value;
  if (eeprom_read(variable_id, &old_value) && old_value == value) {
    return false;
  }
  return EEPROM_TypeB_write(variable_id, value) == EEPROM_EMULATION_WRITE_OK ? true : false;
}

void eeprom_loop(void) {
  /* Erase operation */
  if (gEEPROMTypeBEraseFlag == 1) {
    EEPROM_TypeB_eraseGroup();
    gEEPROMTypeBEraseFlag = 0;
  }
}

#else
void eeprom_init(void) {}
void eeprom_loop(void) {}
bool eeprom_read(uint16_t variable_id, uint32_t *value) {
  UNUSED(variable_id);
  UNUSED(value);
  return false;
}
bool eeprom_write(uint16_t variable_id, uint32_t value) {
  UNUSED(variable_id);
  UNUSED(value);
  return false;
}
#endif
