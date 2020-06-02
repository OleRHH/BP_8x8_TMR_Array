/*****************************  # Includes #   **********************************/
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"           // SysCtlClockFreqSet

#include <adc_functions.h>
#include <EEPROM_functions.h>
#include <lcd_functions.h>
#include <timer_functions.h>
#include <uartDMA.h>
#include <touch.h>
#include <main.h>
#include <debug_functions.h>


/*****************************  # defines #   ***********************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.


/*****************************  # global variables #   **************************/
static char * command;
enum CommandFromTouch commandFromTouch;

TMRSensorData * sensorData;
Settings * settings;

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
    settings = loadSettings();
    ConfigureDebugGPIO();
    sensorData = ConfigureADC();
    ConfigureTimer0(SysClock);
    ConfigureLCD5Inch(SysClock);
//    ConfigureLCD7Inch(SysClock);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);

    // set the display background color
    setLCDBackgroundColor((COLOR)settings->backgroundColor);

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. Timer0IntHandler(): gets periodically called every 100 ms.
    // 2. UART0IntHandler():  gets called on UART0 data receive.
    // 3. ADC1IntHandler():   gets called when ADC complete.
    while(1)
    {
    }
}


/***********************  TIMER 0 interrupt handler  ****************************/
/* Periodically measures the sensor Array values and call function to draw the  */
/* display. Also it sends commands to the stepper-motor and calls setup-menu.   */
/********************************************************************************/
void Timer0IntHandler(void)
{
    // clear the pending interrupt
    timer0IntClear();
    toggleOszi(1);
//    onOszi(2);

    // Draw the arrows and button states to the LC-Display. This function also
    // calculates the new arrow lines. This is the most time consuming part of
    // the program.
    drawDisplay5Inch((void *)&sensorData->diff);
//    drawDisplay7Inch(backColor);

    writeInfos(settings->relative, settings->adcAVG, settings->maxArrowLength, sensorData->maxAnalogValue);

    // Reads touch screen status. Returns command information and (if so)
    // the command itself as a pointer.
    commandFromTouch = readTouchscreen(command);

    // Commands for the motor could be: start, stop, left, right and so on.
    // It can originate from the touch screen or UART0.
    switch(commandFromTouch)
    {
        noNewCommand:           break;
//        enterSettings:          settings(command); break;
        newCommandForMotor:     sendCommandToMotor(command, 9); break;
    }

    // Start sensorData-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in ADC1IntHandler().
    startADConversion();
//    offOszi(2);
}


/***********************  ADC Interrupt handler  ********************************/
/* captures the analog sensorData array signals without busy waiting.               */
/* The digitized signals are being processed at the end.                        */
/********************************************************************************/
void ADC1IntHandler(void)
{
    static uint16_t step = 0;

    // Interrupt is set when a AD-conversion is finished. It needs to be cleared.
    adcIntClear();


    // advance step count each time an AD-conversion is finished. The first one
    // was already started in Timer0Inthandler. There are 16 steps in total.
    step++;

    //
    setMultiplexer(step);

    // store the just captured analog values into buffers to be later processed.
    storeArraySensorData(step - 1);

    // trigger the next AD-conversion (16 in total)
    if(step <= 15)
    {
        startADConversion();
    }

    // after 16 conversions: process analog data and reset values for the next
    // AD-conversion (which gets started during the next Timer0 Interrupt).
    else
    {
        // process arrow length and store results to be later drawn on LCD.
        computeArrows(settings->relative, settings->maxArrowLength, sensorData);
        // reset step counter for next use
        step = 0;
        // reset digital multiplexer inputs
        setMultiplexer(step);
    }
}


/***********************  UART0 Interrupt handler  ******************************/
/* communication via RS232 interface with the PC. The PC sends commands. The    */
/* MCU sends Sensor-Array data. More options like sending Stepper-Motor         */
/* temetery could also be implemented in the future.                            */
/********************************************************************************/
void UART0IntHandler(void)
{
    // pointer to a char string that is defined as UART0receive[8] in uartDMA.c
    char * UART0receive;

    // Read the interrupt status of the UART.
    uint32_t interruptStatus = UARTGetIntStatus();

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UART0ClearInterrupt(interruptStatus);

    if( interruptStatus & DMA_RX_INTERRUPT)
    {
        UART0receive = getUART0RxData();

        // setup DMA for next receive
        prepareNextReceiveDMA();

        // 'char UART0receive[8]' contains commands from Pc. They are always
        // 8 bytes. The commands depend on the value of the first byte:

        // byte 0 contains the type of command:
        switch (UART0receive[0])
        {
        // send array data via RS-232 to a PC (matlab)
        case '0':   sendUARTDMA(sensorData->resultsForUARTSend); break;

        // set arrow relative/absolute and arrow size.
        case '1':   settings->relative = getRelativeAbsoluteSetting();
                    settings->maxArrowLength = getMaxArrowLength();
                    saveSettings(settings);
                    break;

        // commands for the stepper-motor are being past forwarded to UART2.
        case '2':   sendCommandToMotor(UART0receive, 8); break;

        // hardware averaging enabled/disabled
        case '3':   settings->adcAVG = getADCHardwareAveraging(UART0receive[1]);
                    saveSettings(settings);
                    break;

        // todo: additional commands from UART here, if desired.
        case '4': break;
        }
    }
}
