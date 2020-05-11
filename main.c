#include <main.h>

int main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                            | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // disable all interrupts during setup
    IntMasterDisable();

    // Initialize the UART, GPIO, ADC and Timer peripheries
    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD5Inch(SysClock);
//    ConfigureLCD7Inch(SysClock);
    configureUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

    // set the display background color
    writeScreenColor5INCH((COLOR)WHITE);
//    writeRecangle();

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler.
    while(1)
    {
    }
}

