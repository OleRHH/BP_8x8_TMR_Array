#include <main.h>

/*****************************  # global variables #   **************************/
static bool relative = false, oversampling = true;
static char * command;
enum CommandFromTouch commandFromTouch;
static uint16_t maxArrowLength = 32;
static uint32_t maximumAnalogValue;
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

    // Initialize the UART, GPIO, ADC and Timer peripheries
    ConfigureGPIO();
    ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD5Inch(SysClock);
//    ConfigureLCD7Inch(SysClock);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

    // set the display background color
    writeScreenColor5INCH(backColor);

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. Timer0IntHandler(): gets periodically called every 100 ms.
    // 2. UART0IntHandler():  gets called on UART0 data receive.
    // 3. ADC0IntHandler():   gets called when ADC complete.
    while(1)
    {
    }
}



/***********************  TIMER 0 interrupt handler  ****************************/
/* Periodically measures the sensor Array values and draw them to the display.   */
/* Send commands to the stepper-motor.                                          */
/********************************************************************************/
void Timer0IntHandler(void)
{
    // clear the pending interrupt
    timer0IntClear();

    // draw the arrows and button statuses to the LC-Display.
    drawDisplay5Inch(backColor);

//    drawDisplay7Inch();
    writeInfos(relative, oversampling, maxArrowLength, maximumAnalogValue, backColor);

    // Read touch screen informations. Returns command information.
    // Commands for the motor could be: start, stop, left, right and so on.
    // It can originate from the touch screen or UART0.
    commandFromTouch = readTouchscreen(command);
    switch(commandFromTouch)
    {
        noNewCommand:           break;
        enterSettings:          disableTimer0(); settings(); break;
        newCommandForMotor:     sendCommandToMotor(command, 9); break;
    }

    // Start sensor-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in ADC0IntHandler().
    startAdcConversion();
}


/***********************  ADC Interrupt handler  ********************************/
/* capture the analog sensor array signals without busy waiting until ad-conversion
 * is complete */
void ADC0IntHandler(void)
{
    static uint16_t step = 0;

    // An interrupt is triggered when a AD-conversion is finished. It needs to be cleared.
    adcIntClear();

    // advance step count each time an AD-conversion is finished. The first one was
    // already started in Timer0Inthandler. There are 16 steps in total.
    step++;

    // Port L is used to address the analog multiplexers on the TMR sensor array
    if(step == 8)
    {
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
    }
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);

    // store the previously captured analog values into local buffers
    ReadArray(step-1);

    // trigger the next AD-conversion (16 in total)
    if(step <= 15)
    {
        startAdcConversion();
    }

    // after 16 conversions process analog data and prepare for next ADC cycle
    else
    {
        maximumAnalogValue = computeArrows(relative, maxArrowLength);
        step = 0;
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
}


/***********************  UART0 Interrupt handler  ******************************/
/* communication via RS232 interface with the PC (sending sensor data to matlab,
 * receiving commands etc) */
void UART0IntHandler(void)
{
    char * UART0receive;

    // Read the interrupt status of the UART.
    uint32_t ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_DMARX)
    {
        UART0receive = getUART0RxData();

        // setup DMA for next receive
        prepareReceiveDMA();

        // UART0receive[] contains commands from Pc. The commands depend on
        // the value of the first byte:
        // '0': send Array Data (256 bytes) via serial interface to Matlab
        if(UART0receive[0] == '0')
        {
            sendUARTDMA();
        }
        // '1': set arrow relative/absolute and arrow size.
        else if(UART0receive[0] == '1')
        {
            if(UART0receive[1] == '0')
            {
                relative = false;
            }
            else if(UART0receive[1] == '1')
            {
                relative = true;
            }
            // the value was transmitted as char coded number.
            maxArrowLength = UART0receive[4]<<24 | UART0receive[5]<<16 | UART0receive[6]<<8 | UART0receive[7];
        }
        // '2': commands past forwarded to the stepper-motor by UART2.
        else if(UART0receive[0] == '2')
        {
            sendCommandToMotor(UART0receive, 8);
        }
        // hardware averaging enabled/disabled
        else if(UART0receive[0] == '3')
        {
            if(UART0receive[1] == '0')
            {
                ADCHardwareOversampleConfigure(ADC0_BASE, 1);
                ADCHardwareOversampleConfigure(ADC1_BASE, 1);
                oversampling = false;
            }
            else if(UART0receive[1] == '1')
            {
                // set hardware oversampling for better resolution
                ADCHardwareOversampleConfigure(ADC0_BASE, 64);
                ADCHardwareOversampleConfigure(ADC1_BASE, 64);
                oversampling = true;
            }
        }
        // more commands.
        else if(UART0receive[0] == '4')
        {
           // todo: additional commands from UART here, if desired.
        }
    }
}
