#include <timer_functions.h>


/*********************************************************************************************/
void timer0IntClear(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

/*********************************************************************************************/
void ConfigureTimer0(uint32_t SysClock)
/* this timer is used to run the program periodically. Upon call the analog inputs are read,
 * converted and send to the LC-Display */
{
    // Configure Timer0 Interrupt
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);           // Clock Gate enable TIMER0upt
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysClock / 10);      // fires every 100 ms
    TimerEnable(TIMER0_BASE, TIMER_A);
    IntPrioritySet(INT_TIMER0A, LOW_PRIORITY);             // set priority
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0IntHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

