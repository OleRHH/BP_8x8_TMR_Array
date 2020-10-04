/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: timer_functions.c                                                     */
/********************************************************************************/
#include <timer_functions.h>
#include <driverlib/sysctl.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/interrupt.h>  // intEnable(), IntPrioritySet()
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>       // TIMER0_BASE_BASE


/********************************************************************************/
void timer0InterruptDisable(void)
{
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


/********************************************************************************/
void timer0InterruptEnable(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


/********************************************************************************/
void timer0IntClear(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


/********************************************************************************/
void timer1IntClear(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}


/********************************************************************************/
void startTimer1(void)
{
    TimerEnable(TIMER1_BASE, TIMER_A);
}


/*************************  configureTimer0()   *********************************/
/* this timer is used to run the program periodically. Upon call the analog     */
/* inputs are converted and send to the LC-Display                              */
/********************************************************************************/
void configureTimer0(uint32_t SysClock)
{
    // Configure Timer0 Interrupt
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);           // Clock Gate enable TIMER0upt
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysClock / 10);      // fires every 100 ms
    TimerEnable(TIMER0_BASE, TIMER_A);
    IntPrioritySet(INT_TIMER0A, LOW_PRIORITY_5);             // set priority
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer0InterruptHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


/*************************  configureTimer1()   *********************************/
/* this timer is used to start ad-conversion 85 ms after timer 0 has started    */
/* to refresh the LCD.                                                          */
/********************************************************************************/
void configureTimer1(uint32_t SysClock)
{
    // Configure Timer1 Interrupt
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);           // Clock Gate enable TIMER0upt
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);        // one shot. Has to be started manually
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysClock / 12);      // fires after 90 ms
    IntPrioritySet(INT_TIMER1A, HIGH_PRIORITY_1);             // set priority
    TimerIntRegister(TIMER1_BASE, TIMER_A, timer1InterruptHandler);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

