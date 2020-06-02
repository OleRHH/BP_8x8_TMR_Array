/*
 * settings.c
 *
 *  Created on: 28.05.2020
 */
#include <EEPROM_functions.h>


Settings setting;

Settings * loadSettings(void)
{
    // todo: loading setting from EEPROM here:
    Settings * settings = &setting;
    settings->relative = true;
    settings->coloredArrows = true;
    settings->maxArrowLength = 32;
    settings->adcAVG = false;
    settings->backgroundColor = 0x00FFFFFF;
    settings->motorIncrement = 100;

    return settings;
}


void saveSettings(Settings * settings)
{
    // todo: saving setting in EEPROM here:

}
