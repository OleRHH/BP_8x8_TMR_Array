#include <main.h>

/*********************  # global variables #   ******************/
uint32_t SysClock;

int main(void)
{
    SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // Initialize the UART, GPIO, ADC and Timer
    IntMasterDisable();
    ConfigureUART(SysClock);
    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD();

    write_screen_color((COLOR)WHITE);

    IntMasterEnable();

    while(1);       // busy waiting. Tasks running in interrupt handler.
}

