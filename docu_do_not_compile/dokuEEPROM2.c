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
