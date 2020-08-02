/*****************************  # Includes #   **********************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>                           // SysCtlClockFreqSet

// each module has its own include file
#include <adc_functions.h>
#include <EEPROM_functions.h>
#include <lcd_functions.h>
#include <timer_functions.h>
#include <uartDMA.h>
#include <touch.h>
#include <debug_functions.h>


/*****************************  # defines #   ***********************************/
#define CLOCK_FREQ ( 120000000 )                        // 120 MHz clock freq.
#define LCD5INCH
//#define EEPROM_INIT          // uncomment to init settings with default values
//#define DEBUG

/*****************************  # global variables #   **************************/
static char * command;
static enum CommandFromTouch commandFromTouch;
static TMRSensorData * sensorData;
static Settings * settings;

extern void menu(void);
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

    // Configure the GPIO, ADC, UART, EEPROM and Timer peripheries.
    configureDebugGPIO();
    sensorData = configureADC();
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);
    configureTimer0(SysClock);
    configureEEPROM();

    // if defined: initialize EEPROM settings with default values.
#ifdef EEPROM_INIT
    settings = initSettings();
    saveSettingsToEEPROM(settings);
#endif

    // Load stored settings from EEPROM.
    settings = loadSettingsFromEEPROM();

    // This program works with a 5'' or 7'' display. If defined: 5''
#ifdef LCD5INCH
    configureLCD5Inch(SysClock, (COLOR)settings->backgroundColor);
#else
    configureLCD7Inch(SysClock, (COLOR)settings->backgroundColor);
#endif

    menu();

    IntMasterEnable();


    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. Timer0InterruptHandler(): gets periodically called every 100 ms.
    // 2. UART0InterruptHandler():  gets called on UART0 data receive.
    // 3. ADC1InterruptHandler():   gets called when ADC complete.
    while(1)
    {
    }
}


/***********************  TIMER 0 interrupt handler  ****************************/
/* Periodically measures the sensor Array values and calls function to draw the */
/* display. Also it sends commands to the stepper-motor and calls setup-menu.   */
/********************************************************************************/
void Timer0InterruptHandler(void)
{
    // clear the pending interrupt
    timer0IntClear();

    #ifdef DEBUG
        toggleOszi(1);                                // for debuging with osci
        onOszi(2);                                    // for debuging with osci
    #endif

    // Draw the arrows and button states to the LC-Display. This function also
    // calculates the new arrow lines. This is the most time consuming part of
    // the program.

    #ifdef LCD5INCH
        drawDisplay5Inch(&sensorData->arrows);
    #else
        drawDisplay7Inch(&sensorData->arrows);
    #endif

    writeInfos(settings->relative, settings->adcAVG, settings->maxArrowLength, sensorData->maxAnalogValue);

    // Reads touch screen status. Returns command information and
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
    // read bursts. The other 15 bursts will be triggered in ADC1InterruptHandler().
    startADConversion();

    #ifdef DEBUG
        offOszi(2);                                    // for debuging with osci
    #endif
}


/***********************  ADC Interrupt handler  ********************************/
/* captures the analog sensorData array signals without busy waiting.               */
/* The digitized signals are being processed at the end.                        */
/********************************************************************************/
void ADC1InterruptHandler(void)
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
/* Communication via RS232 interface with the PC. The PC sends commands. The    */
/* MCU sends Sensor-Array data. More options like sending Stepper-Motor         */
/* temetery could also be implemented in the future.                            */
/* 'char UART0receive[8]' contains commands from PC. They are always 8 bytes.   */
/* The type of command depends on the value of the first byte.                  */
/********************************************************************************/
void UART0InterruptHandler(void)
{

    // Read the interrupt status of the UART0 peripherie.
    uint32_t interruptStatus = UARTGetIntStatus();

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UART0ClearInterrupt(interruptStatus);

    // Message (8 bytes) from Pc received (via UDMA receive)
    if( interruptStatus & DMA_RX_INTERRUPT)
    {
        // pointer to a char string that is defined as UART0receive[8] in uartDMA.c
        char * UART0receive = getUART0RxData();

        // setup DMA for next receive
        prepareNextReceiveDMA();


        // byte 0 contains the type of command from Pc:
        switch (UART0receive[0])
        {
        // send array data via RS-232 to a PC (matlab)
        case '0':   sendUARTDMA(sensorData->resultsForUARTSend); break;

        // set arrow relative/absolute and set arrow size.
        case '1':   settings->relative = getRelativeAbsoluteSetting();
                    settings->maxArrowLength = getMaxArrowLength();
                    saveSettingsToEEPROM(settings);
                    break;

        // commands for the stepper-motor are being past forwarded to UART2.
        case '2':   sendCommandToMotor(UART0receive, 8); break;

        // hardware averaging enabled/disabled
        case '3':   settings->adcAVG = getADCHardwareAveraging(UART0receive[1]);
                    saveSettingsToEEPROM(settings);
                    break;

        // todo: additional commands from UART here, if desired.
        case '4': break;
        }
    }
}
