/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: EEPROM_functions.h                                                    */
/********************************************************************************/
#ifndef EEPROM_FUNCTIONS_H_
#define EEPROM_FUNCTIONS_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>


/**************************  # public Prototypes #   ****************************/
void configureEEPROM(void);
void loadSettingsFromEEPROM(void *, uint32_t);
void saveSettingsToEEPROM(void *, uint32_t);


#endif /* EEPROM_FUNCTIONS_H_ */
