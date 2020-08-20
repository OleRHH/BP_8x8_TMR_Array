#include <EEPROM_functions.h>
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"


/********************  loadSettingsFromEEPROM()   ***************************/
// Loads the settings structure from EEPROM.                                //
/****************************************************************************/
void loadSettingsFromEEPROM(void * data, uint32_t byteCount)
{
    EEPROMRead( (uint32_t *) data, 0x00, byteCount);
}


/********************  saveSettingsToEEPROM()   *****************************/
// Saves the settings structure in EEPROM.                                  //
/****************************************************************************/
void saveSettingsToEEPROM(void * data, uint32_t byteCount)
{
    EEPROMProgram( (uint32_t *) data, 0x00, byteCount);
}



/************************  configureEEPROM()   ******************************/
// Configures the EEPROM peripherie.                                        //
/****************************************************************************/
void configureEEPROM(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    // EEPROMInit - if not called may lead to dataloss
    EEPROMInit();
}
