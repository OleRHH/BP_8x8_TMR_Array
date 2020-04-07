#include <main.h>

int main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // Initialize the UART, GPIO, ADC and Timer peripherie
    IntMasterDisable();                     // disable all interrupts during setup

    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD5Inch(SysClock);
//    ConfigureLCD7Inch(SysClock);
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

   write_screen_color5INCH((COLOR)WHITE);       // set the display background color

    // write the frame for the Array Display
//    write_line(80, 10, 720, 10, (COLOR)YELLOW, 0);
//    write_line(80, 470, 720, 470, (COLOR)YELLOW, 0);
//
//    write_line(80, 10, 80, 470, (COLOR)YELLOW, 0);
//    write_line(720, 10, 720, 470, (COLOR)YELLOW, 0);
    IntMasterEnable();

    while(1);                               // busy waiting. Tasks running in interrupt handler.
}

