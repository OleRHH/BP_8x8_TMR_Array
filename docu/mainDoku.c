/*****************************  # Includes #   **********************************/
#include <stdbool.h>                    // needed for sysctl.h
#include <stdint.h>                     // needed for sysctl.h
#include <driverlib/sysctl.h>           // SysCtlClockFreqSet

#include <adc_functions.h>
#include <timer_functions.h>
#include <lcd_functions.h>
#include <uartDMA.h>


/*****************************  # defines #   ***********************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.


/*****************************  # global variables #   **************************/
static COLOR backColor = (COLOR)WHITE;


/***********************  main() function  **************************************/
/* the main() function initializes the hardware components and sets the         */
/* LC-Display background color to white.                                        */
/********************************************************************************/
void main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                            | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // disable all interrupts during setup
    IntMasterDisable();

    // Initialize the ADC, Timer, LCD and UART peripheries
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD(SysClock);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

    // set the display background color
    writeScreenColor(backColor);

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. Timer0IntHandler(): gets periodically called every 100 ms.
    // 2. UART0IntHandler():  gets called on UART0 data receive.
    // 3. ADC0IntHandler():   gets called when ADC complete.
    while(1)
    {
    }
}

