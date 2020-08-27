/***********************  main() function  **************************************/
/* the main() function initializes the hardware components and sets the         */
/* LC-Display layout.                                                           */
/********************************************************************************/
void main(void)
{
    // set the CPU clock frequency to 120 MHz
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                            | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    // disable all interrupts during setup
    IntMasterDisable();

    configureEEPROM();

    // if defined: initialize EEPROM settings with default values and save them.
    // default values are stored in function initROM() (see end of file).
#ifdef EEPROM_INIT
    initROM();
    saveSettingsToEEPROM(&settings, sizeof(Settings));
#else
    // Normally the stored settings are simply loaded on startup. The stored
    // settings are loaded from EEPROM into the global structure 'settings'.
    loadSettingsFromEEPROM(&settings, sizeof(Settings));
#endif

    // configure LCD with the loaded settings (includes all layouts). Returns
    // active layout with the variables: relative, adcAVG and maxArrowLength
    setup = configureLCD(SysClock, &settings);

    // Configure the ADC, UART, Timer and Touch peripheries.
    sensorData = configureADC(setup->adcAVG);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);
    configureTimer0(SysClock);
    configureTimer1(SysClock);
    configureTouch();

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. timer0InterruptHandler(): gets periodically called every 100 ms.
    // 2. timer1InterruptHandler(): gets periodically called every 100 ms
    //                              but 85 ms delayed to Timer0.
    // 3. UART0InterruptHandler():  gets called on UART0 data receive.
    // 4. adcInterruptHandler():   gets called when ADC complete.
    // 5. touchInterruptHandler():  gets called when display is touched.
    while(1)
    {
    }
}
