/************************  configureEEPROM()   ******************************/
// Configures the EEPROM peripherie.                                        //
/****************************************************************************/
void configureEEPROM(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    // EEPROMInit - if not called may lead to dataloss
    EEPROMInit();
}
