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
    uint16_t xpos;
    uint16_t ypos;
} Settings;

Settings * initSettings(void);
void configureEEPROM(void);
void saveSettingsToEEPROM(Settings *);
Settings * loadSettingsFromEEPROM(void);


#endif /* EEPROM_FUNCTIONS_H_ */
