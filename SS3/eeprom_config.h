#ifndef EEPROM_CONFIG_H_
#define EEPROM_CONFIG_H_

/*!
 * @brief The sector address to use
 */
#ifndef LINKER_SCRIPT
#ifndef EEPROM_EMULATION_ADDRESS
extern unsigned int EEPROM_START;
#define EEPROM_EMULATION_ADDRESS                                    ((uint32_t)&EEPROM_START)
#endif
#endif
/*!
 * @brief The number of groups to use
 */
#define EEPROM_EMULATION_GROUP_ACCOUNT                                       (2)
/*!
 * @brief The number of sectors in groups to use
 */
#define EEPROM_EMULATION_SECTOR_INGROUP_ACCOUNT                              (2)

#define EEPROM_EMULATION_SIZE (EEPROM_EMULATION_GROUP_ACCOUNT * EEPROM_EMULATION_SECTOR_INGROUP_ACCOUNT * 1024)

#endif // EEPROM_CONFIG_H_