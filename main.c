#include <main.h>

int main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // Initialize the UART, GPIO, ADC and Timer peripherie
    IntMasterDisable();                     // disable all interrupts during setup
    ConfigureUART(SysClock);
    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD(SysClock);

    write_screen_color((COLOR)WHITE);       // set the display background color

    IntMasterEnable();

    while(1);                               // busy waiting. Tasks running in interrupt handler.
}

