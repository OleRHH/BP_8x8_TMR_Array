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




/*********************************************************************************************/
void ConfigureGPIO(void)
{
    // Set Port L  0-4: Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Set Port M Pins 0-7: Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // Set Port Q Pins 0-4: LCD Control output:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);  // Clock Port Q
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                                         | GPIO_PIN_4);
}
