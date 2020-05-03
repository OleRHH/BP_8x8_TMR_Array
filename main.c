#include <main.h>
#include <math.h>

extern int16_t DiffCosResults[8][8];
extern int16_t DiffSinResults[8][8];


int main(void)
{

    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // disable all interrupts during setup
    IntMasterDisable();
    double _2pi = 6.28318530718;
    uint16_t a = 350, A = 400;
    uint16_t m, n;


    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            DiffCosResults[m][n] = 32 * cos(_2pi * a/A);
            DiffSinResults[m][n] = 32 * sin(_2pi * a/A);
        }
    }

    // Initialize the UART, GPIO, ADC and Timer peripherie
    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD5Inch(SysClock);
//    ConfigureLCD7Inch(SysClock);
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

    // set the display background color


    write_screen_color5INCH((COLOR)WHITE);
//    init_grid();

    // write the frame for the Array Display
//    write_line(80, 10, 720, 10, (COLOR)YELLOW, 0);
//    write_line(80, 470, 720, 470, (COLOR)YELLOW, 0);
//    write_line(80, 10, 80, 470, (COLOR)YELLOW, 0);
//    write_line(720, 10, 720, 470, (COLOR)YELLOW, 0);

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler.
    while(1)
    {
    }
}

