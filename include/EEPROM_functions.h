/*
 * settings.h
 *  Created on: 28.05.2020
 */

#ifndef EEPROM_FUNCTIONS_H_
#define EEPROM_FUNCTIONS_H_


#include <stdbool.h>
#include <stdint.h>


void configureEEPROM(void);
void loadSettingsFromEEPROM(void *, uint32_t);
void saveSettingsToEEPROM(void *, uint32_t);


#endif /* EEPROM_FUNCTIONS_H_ */
