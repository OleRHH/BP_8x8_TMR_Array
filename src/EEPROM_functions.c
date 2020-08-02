#include <EEPROM_functions.h>
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"


/**************************  # global variables #    ************************/
Settings setting;


/***********************  initSettings()   **********************************/
// Saves the settings structure in EEPROM.                                  //
/****************************************************************************/
Settings * initSettings(void)
{
    Settings * settings = &setting;
    settings->relative = true;
    settings->coloredArrows = true;
    settings->maxArrowLength = 40;
    settings->adcAVG = true;
    settings->backgroundColor = 0x00FFFFFF;
    settings->motorIncrement = 100;

    return settings;

}


/********************  loadSettingsFromEEPROM()   ***************************/
// Loads the settings structure from EEPROM.                                //
/****************************************************************************/
Settings * loadSettingsFromEEPROM(void)
{
    Settings * settings = &setting;
    uint32_t settings_array_s[6];

    EEPROMRead(settings_array_s, 0x00, sizeof(settings_array_s));

    settings->relative = (bool) settings_array_s[0];
    settings->coloredArrows = (bool) settings_array_s[1];
    settings->adcAVG = (bool) settings_array_s[2];
    settings->maxArrowLength = (uint16_t) settings_array_s[3];
    settings->backgroundColor = (int32_t) settings_array_s[4];
    settings->motorIncrement = (int32_t) settings_array_s[5];

    return settings;
}


/********************  saveSettingsToEEPROM()   *****************************/
// Saves the settings structure in EEPROM.                                  //
/****************************************************************************/
void saveSettingsToEEPROM(Settings * settings)
{
    uint32_t settings_array_s[6] = { settings->relative,
                                     settings->coloredArrows,
                                     settings->adcAVG,
                                     settings->maxArrowLength,
                                     settings->backgroundColor,
                                     settings->motorIncrement};

    EEPROMProgram(settings_array_s, 0x00, sizeof(settings_array_s));
//    EEPROMProgram((uint32_t *)settings, 0x00, sizeof(settings));
}


/************************  configureEEPROM()   ******************************/
// Configures the EEPROM peripherie.                                        //
/****************************************************************************/
void configureEEPROM(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    // EEPROMInit - if not called may lead to dataloss
    EEPROMInit();
    // EEPROMMassErase - Erases the EEPROM leaving it to divece factory default conditions
    //EEPROMMassErase();
}
