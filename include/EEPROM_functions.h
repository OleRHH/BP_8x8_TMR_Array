/*
 * settings.h
 *  Created on: 28.05.2020
 */

#ifndef EEPROM_FUNCTIONS_H_
#define EEPROM_FUNCTIONS_H_


#include <stdbool.h>
#include <stdint.h>


typedef struct
{
    bool relative;
    bool adcAVG;
    bool coloredArrows;
    uint16_t maxArrowLength;
    int32_t backgroundColor;
    int32_t motorIncrement;
} Settings;

Settings * initSettings(void);
void configureEEPROM(void);
void saveSettingsToEEPROM2(Settings *);
Settings * loadSettingsFromEEPROM2(void);

void loadSettingsFromEEPROM(void *, uint32_t);
void saveSettingsToEEPROM(void *, uint32_t);

#endif /* EEPROM_FUNCTIONS_H_ */
