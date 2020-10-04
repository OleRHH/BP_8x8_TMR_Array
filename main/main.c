/********************************************************************************/
/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/********************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>                           // for SysCtlClockFreqSet
// each module has its own include file
#include <adc_functions.h>
#include <EEPROM_functions.h>
#include <lcd_functions.h>
#include <timer_functions.h>
#include <uartDMA_functions.h>
#include <touch_functions.h>


/*****************************  # defines #   ***********************************/
/* The following defines activate functions not needed for normal operation.    */
/********************************************************************************/
// Uncomment to initialize EEPROM settings with default values.
// Default values see list in function initROM() at end of document.
//#define EEPROM_INIT


/*****************************  # global variables #   **************************/
// structure to hold all relevant sensor data information
static TMRSensorData * sensorData;
// structure to hold all relevant information for displaying the data on LCD
static Settings settings;
// pointer to the currently active layout
static Setup * setup;


/*****************************  # intern prototypes #   *************************/
void initROM(void);


/***********************  main() function  **************************************/
/* the main() function initializes the hardware components and sets the         */
/* LC-Display layout.                                                           */
/********************************************************************************/
void main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                            | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    // disable all interrupts during setup
    IntMasterDisable();

    configureEEPROM();

    // if defined: initialize EEPROM settings with default values and save them.
    // default values are stored in function initROM() (see end of file).
#ifdef EEPROM_INIT
    initROM();
    saveSettingsToEEPROM(&settings, sizeof(Settings));
#else
    // Normally the stored settings are simply loaded on startup. The stored
    // settings are loaded from EEPROM into the global structure 'settings'.
    loadSettingsFromEEPROM(&settings, sizeof(Settings));
#endif

    // configure LCD with the loaded settings (includes all layouts). Returns
    // active layout with the variables: relative, adcAVG and maxArrowLength
    setup = configureLCD(SysClock, &settings);

    // Configure the ADC, UART, Timer and Touch peripheries.
    sensorData = configureADC(setup->adcAVG);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);
    configureTimer0(SysClock);
    configureTimer1(SysClock);
    configureTouch();

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. timer0InterruptHandler(): gets periodically called every 100 ms.
    // 2. timer1InterruptHandler(): gets periodically called every 100 ms
    //                              but 85 ms delayed to Timer0.
    // 3. UART0InterruptHandler():  gets called on UART0 data receive.
    // 4. adcInterruptHandler():   gets called when ADC complete.
    // 5. touchInterruptHandler():  gets called when display is touched.

    while(1)
    {
    }
}


/***********************  TIMER 0 interrupt handler  ****************************/
/* Timer 0 is set to call this handler every 100 ms. This handler calls the     */
/* drawDisplay() function to refresh the LCD content. It also starts Timer 1    */
/* which is programmed to call the timer1InterruptHandler() 85 ms later.        */
/********************************************************************************/
void timer0InterruptHandler(void)
{
    // clear the pending interrupt
    timer0IntClear();

    // start timer1 to delayed start a new ad-conversion process
    startTimer1();

    // send sensor data to PC if requested.
    if(setup->reqSensorData == true)
    {
        setup->reqSensorData = false;
        sendUARTDMA(sensorData->resultsForUARTSend);
    }
    // compute the new arrow length from last measurement before drawing LCD
    computeArrows(setup->relative, setup->maxArrowLength);

    // Draw the arrows and button states to the LC-Display. This function also
    // calculates the new arrow lines. This is the most time consuming part of
    // the project.
    drawDisplay((void *)&sensorData->arrows);

    // write the highest measured analog value to the display
    writeMaxAnalogValue(sensorData->maxAnalogValue);
}


/***********************  TIMER 1 interrupt handler  ****************************/
/* Timer 1 gets started in timer0InterruptHandler(). It is set to 90 ms.        */
/* Because Timer 0 is set to 100 ms, this handler always gets called 10 ms      */
/* before the next timer0InterruptHandler call.                                 */
/* This Timer 1 handler starts a new ADC process which takes 9 ms to convert    */
/* all sensor-array signals with hardware averaging setting enabled. This       */
/* means the adc process is always finished shortly before the next             */
/* timer0InterruptHandler draws the new arrows.                                 */
/********************************************************************************/
void timer1InterruptHandler(void)
{
    // clear the pending interrupt
    timer1IntClear();

    // Start sensorData-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in adcInterruptHandler().
    startADConversion();
}


/***********************  ADC Interrupt handler  ********************************/
/* The hardware can convert up to 16 analog signals to digital when triggered.  */
/* When done with the conversion this function is called and another conversion */
/* is started until all sensor data was read. This function has to be triggered */
/* 16 times to convert all 16*16 = 256 analog values.                           */
/********************************************************************************/
void adcInterruptHandler(void)
{
    static uint16_t step = 0;

    // Interrupt is set when a AD-conversion is finished. It needs to be cleared.
    adcIntClear();

    // advance step count each time an AD-conversion is finished. The first one
    // was already started in Timer0Inthandler. There are 16 steps in total.
    step++;

    // set the analog multiplexer addresses or the next 16 sensor values.
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
        // reset step counter for next use
        step = 0;
        // reset digital multiplexer inputs
        setMultiplexer(step);
    }
}


/***********************  UART0 Interrupt handler  ******************************/
/* Communication via RS232 interface with the PC. The PC sends commands. The    */
/* MCU sends Sensor-Array data. More options like sending Stepper-Motor         */
/* telemetery could also be implemented.                                        */
/* 'char UART0receive[8]' contains commands from PC. They are always 8 bytes.   */
/* The type of command depends on the value of the first byte. This is checked  */
/* in the switch case.                                                          */
/********************************************************************************/
void UART0InterruptHandler(void)
{
    // Read the interrupt status of the UART0 peripherie.
    uint32_t interruptStatus = UARTGetIntStatus();

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UART0ClearInterrupt(interruptStatus);

    // Message (8 bytes) from PC received (via UDMA receive)
    if( interruptStatus & DMA_RX_INTERRUPT)
    {
        // pointer to a char string that is defined as UART0receive[8] in uartDMA.c
        char * UART0receive = getUART0RxData();

        // setup DMA for next receive
        prepareNextReceiveDMA();


        // byte 0 contains the type of command from PC:
        switch (UART0receive[0])
        {
        // send sensor-array data via RS-232 to a PC (matlab)
        case '0':
            setup->reqSensorData = true;
            break;

        // set maximum arrow length
        case '1':
            setup->maxArrowLength = getMaxArrowLengthUART();
            writeInfo(MAX_ARROW_LENGTH);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // toggle arrow setting scaling: relative <-> absolute
        case '2':
            setup->relative = setup->relative ? false:true;
            writeInfo(SCALING);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // toggle hardware averaging setting: on <-> off
        case '3':
            setup->adcAVG = setADCHardwareAveraging((UART0receive[1] == '1'));
            writeInfo(HARDW_AVG);
            saveSettingsToEEPROM(&settings, sizeof(Settings));
            break;

        // commands for the stepper-motor are being past forwarded to UART2.
        case '4':
            sendRawCommandToMotor(UART0receive);
            break;

        // todo: additional commands from UART here, if desired.
        case '5':
            break;
        }
    }
}


/***********************  Touch Interrupt handler  ******************************/
/* A GPIO falling edge interrupt occurs when the touch screen is pressed.       */
/********************************************************************************/
void touchInterruptHandler(void)
{
    touchInterruptClear();

    // Read touch screen status.
    uint16_t item = touchGetMenuItem();

    // check what (or if) a button was pressed and act accordingly
    switch(item)
    {
    case LEFT_BUTTON:
        sendCommandToMotor(LEFT_BUTTON);
        break;

    case RIGHT_BUTTON:
        sendCommandToMotor(RIGHT_BUTTON);
        break;

    case STOP_BUTTON:
        sendCommandToMotor(STOP_BUTTON);
        break;

    case CHANGE_SETTING_BUTTON:
        drawSettingsMenu();
        settings.settingNo = touchGetSettingNum(settings.settingNo);
        setup = setLCDLayout(&settings);
        drawDisplayLayout();
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // draws a window where the user can choose from 16 arrow length sizes
    // ranging from 10 to 160. Selection via touch.
    case ARROW_LENGTH_BUTTON:
        drawArrowLengthMenu();
        setup->maxArrowLength = touchGetArrowLength(setup->maxArrowLength);
        drawDisplayLayout();
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // toggle arrow setting scaling: relative <-> absolute
    case SCALING_BUTTON:
        setup->relative = setup->relative ? false:true;
        writeInfo(SCALING);
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;

    // toggle setting hardware averaging: on <-> off
    case HARDW_AVG_BUTTON:
        setup->adcAVG = setADCHardwareAveraging(setup->adcAVG);
        writeInfo(HARDW_AVG);
        saveSettingsToEEPROM(&settings, sizeof(settings));
        break;
    }
}


/***************************  initROM()  ****************************************/
/* A new MCU has no saved settings and needs to be initialized with default     */
/* values. This function is called on every startup and checks if the first     */
/* 4 bytes contain a per Macro defined number RANDOM_NUMBER. If so, the EEPROM  */
/* has already been initialized. If not it will be initialized.                 */
/********************************************************************************/
void initROM(void)
{
    settings.settingNo = 0;

// Setting No 0:
    settings.setup[0].relative = true;
    settings.setup[0].adcAVG = true;
    settings.setup[0].coloredArrows = false;
    settings.setup[0].maxArrowLength = 60;
    settings.setup[0].reqSensorData = false;

    settings.setup[0].backColorArrow = (color)WHITE;
    settings.setup[0].arrowColor = (color)BLACK;
    settings.setup[0].gridColor = (color)BLACK;

    settings.setup[0].backColorTable = (color)GREEN;
    settings.setup[0].backColorMenu = (color)BLACK;
    settings.setup[0].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[0].backColorImgStop  = (color)RED;
    settings.setup[0].backColorImgLeft  = (color)GREEN;
    settings.setup[0].backColorImgRight = (color)GREEN;
    settings.setup[0].backColorMoreSettings = (color)BLUE;
    settings.setup[0].spacerColor = (color)BLUE;
    settings.setup[0].fontColor = (color)BLACK;

// Setting No 1:
    settings.setup[1].relative = true;
    settings.setup[1].adcAVG = true;
    settings.setup[1].coloredArrows = false;
    settings.setup[1].maxArrowLength = 60;
    settings.setup[1].reqSensorData = false;

    settings.setup[1].backColorArrow = (color)BLACK;
    settings.setup[1].arrowColor = (color)WHITE;
    settings.setup[1].gridColor = (color)YELLOW;

    settings.setup[1].backColorArrow = (color)BLACK;
    settings.setup[1].backColorTable = (color)GREEN;
    settings.setup[1].backColorMenu = (color)BLACK;
    settings.setup[1].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[1].backColorMoreSettings = (color)BLUE;
    settings.setup[1].backColorImgStop  = (color)RED;
    settings.setup[1].backColorImgLeft  = (color)GREEN;
    settings.setup[1].backColorImgRight = (color)GREEN;
    settings.setup[1].backColorMoreSettings = (color)BLUE;
    settings.setup[1].spacerColor = (color)BLUE;
    settings.setup[1].fontColor = (color)BLACK;

// Setting No 2:
    settings.setup[2].relative = true;
    settings.setup[2].adcAVG = true;
    settings.setup[2].coloredArrows = true;
    settings.setup[2].maxArrowLength = 100;
    settings.setup[2].reqSensorData = false;

    settings.setup[2].backColorArrow = (color)GREY;
    settings.setup[2].arrowColor = (color)BLACK;
    settings.setup[2].gridColor = (color)BLACK;

    settings.setup[2].backColorTable = (color)YELLOW;
    settings.setup[2].backColorMenu = (color)YELLOW;
    settings.setup[2].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[2].backColorImgStop  = (color)RED;
    settings.setup[2].backColorImgLeft  = (color)GREEN;
    settings.setup[2].backColorImgRight = (color)GREEN;
    settings.setup[2].backColorMoreSettings = (color)BLUE;
    settings.setup[2].spacerColor = (color)BLUE;
    settings.setup[2].fontColor = (color)BLACK;

// Setting No 3:
    settings.setup[3].relative = true;
    settings.setup[3].adcAVG = true;
    settings.setup[3].coloredArrows = true;
    settings.setup[3].maxArrowLength = 100;
    settings.setup[3].reqSensorData = false;

    settings.setup[3].backColorArrow = (color)BLACK;
    settings.setup[3].arrowColor = (color)WHITE;
    settings.setup[3].gridColor = (color)YELLOW;

    settings.setup[3].backColorArrow = (color)BLACK;
    settings.setup[3].backColorTable = (color)GREEN;
    settings.setup[3].backColorMenu = (color)BLACK;
    settings.setup[3].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[3].backColorMoreSettings = (color)BLUE;
    settings.setup[3].backColorImgStop  = (color)RED;
    settings.setup[3].backColorImgLeft  = (color)GREEN;
    settings.setup[3].backColorImgRight = (color)GREEN;
    settings.setup[3].backColorMoreSettings = (color)BLUE;
    settings.setup[3].spacerColor = (color)BLUE;
    settings.setup[3].fontColor = (color)BLACK;
// ...
// more setups could be added here at any time.
}


