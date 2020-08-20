/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, August 2020, Ole Rönna, van Hung Le.                           */
/********************************************************************************/

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
#define VERION_NUMBER ( 0.8 )
//#define EEPROM_INIT          // uncomment to initialize settings with default
                               // values (defaults see list at end of document).
#define DEBUG


/*****************************  # global variables #   **************************/
uint16_t xpos, ypos;
static TMRSensorData * sensorData;
static Settings settings;
static Setup * setup;


/*****************************  # intern prototypes #   *************************/
void initROM(void);


/***********************  main() function  **************************************/
/* the main() function initializes the hardware components and sets the         */
/* LC-Display layout.                                        */
/********************************************************************************/
void main(void)
{
    // set the clock frequency to CLOCK_FREQ (120 MHz)
    uint32_t SysClock = SysCtlClockFreqSet( (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN
                            | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), CLOCK_FREQ);

    // disable all interrupts during setup
    IntMasterDisable();

    #ifdef DEBUG
        configureDebugGPIO();
    #endif

    configureEEPROM();

    // if defined: initialize EEPROM settings with default values and save them.
    // default values are stored in function initROM() (see end of file).
    #ifdef EEPROM_INIT
        initROM();
        saveSettingsToEEPROM(&settings, sizeof(settings));
    #endif

    // Normally the stored settings are simply loaded on startup. The stored
    // settings are loaded from EEPROM into the global structure 'settings'.
    loadSettingsFromEEPROM(&settings, sizeof(settings));

    // configure LCD with the loaded settings. Returns a pointer to
    // active layout with the variables: relative, adcAVG and maxArrowLength
    setup = configureLCD(SysClock, &settings);

    // Configure the ADC, UART, Timer and Touch peripheries.
    sensorData = configureADC(setup->adcAVG);
    configureUartUDMA();
    ConfigureUART0(SysClock);
    ConfigureUART2(SysClock);
    configureTimer0(SysClock);
    configureTouch();

    IntMasterEnable();

    // busy waiting. Tasks now running in interrupt handler. The tasks are
    // 1. Timer0InterruptHandler(): gets periodically called every 100 ms.
    // 2. UART0InterruptHandler():  gets called on UART0 data receive.
    // 3. ADC1InterruptHandler():   gets called when ADC complete.
    // 4. TouchInterruptHandler():  gets called when display is touched.
    while(1)
    {
    }
}


/***********************  Touch Interrupt handler  ******************************/
/* An GPIO edge interrupt occures when the touch screen is pressed.             */
/********************************************************************************/
void TouchInterruptHandler(void)
{
    uint16_t item;
    GPIOIntClear(GPIO_PORTF_AHB_BASE, GPIO_PIN_4);

    timerInterruptDisable();

    // Read touch screen status.
    item = touchGetMenuItem(&xpos, &ypos);

    // check what (or if) a button was pressed
    switch(item)
    {
    case  LEFT_BUTTON:
        sendCommandToMotor(LEFT_BUTTON);
        setup = setLCDLayout(0, &settings);
        break;

    case RIGHT_BUTTON:
        sendCommandToMotor(RIGHT_BUTTON);
        setup = setLCDLayout(1, &settings);
        break;

    case  STOP_BUTTON:
        sendCommandToMotor(STOP_BUTTON);
        break;

    case START_BUTTON:
        sendCommandToMotor(START_BUTTON);
        break;

    // draws a window where the user can choose from 16 arrow length sizes
    // ranging from 10 to 160. Selection via touch.
    case ARROW_LENGTH_BUTTON:
        drawArrowLengthMenu();
        setup->maxArrowLength = touchGetArrowLength(setup->maxArrowLength, &xpos, &ypos);
        drawDisplayLayout();

        // show touch coordinates on LCD for debug verification and calibration
        #ifdef DEBUG_TOUCH
            uint32_t touchPosition = (uint32_t)xpos << 16 | ypos;
            writeInfo(POS_DEBUG, (void *)&touchPosition, settings->style);
    //        writeInfo(POS_DEBUG);
        #endif

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

    timerInterruptEnable();
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


        // byte 0 contains the type of command from Pc:
        switch (UART0receive[0])
        {
        // send sensor-array data via RS-232 to a PC (matlab)
        case '0':
            sendUARTDMA(sensorData->resultsForUARTSend);
            break;

        // set maximum arrow length
        case '1':
            setup->maxArrowLength = getMaxArrowLengthUART();
            writeInfo(MAX_ARROW_LENGTH | SCALING);

//            writeInfo(MAX_ARROW_LENGTH, (void *)&settings->maxArrowLength, settings->style);
//            writeInfo(SCALING, (void *)&settings->relative);
//            writeInfo(MAX_ARROW_LENGTH, (void *)&settings->maxArrowLength);
            saveSettingsToEEPROM(&settings, sizeof(settings));
            break;

        // toggle arrow setting scaling: relative <-> absolute
        case '2':
            setup->relative = setup->relative ? false:true;
            writeInfo(SCALING);
            saveSettingsToEEPROM(&settings, sizeof(settings));
            break;

        // toggle hardware averaging setting: on <-> off
        case '3':
            setup->adcAVG = setADCHardwareAveraging((UART0receive[1] == '1'));
            writeInfo(HARDW_AVG);
            saveSettingsToEEPROM(&settings, sizeof(settings));
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
#include <math.h>
#define _2PI ( (double)6.28318530718 )
uint16_t A = 200, a = 0;
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
    // the project.
    int16_t m, n;

    int16_t cosinus = 50 * cos(_2PI * a/A);
    int16_t sinus = 50 * sin(_2PI * a/A);
    a = (++a)%A;

    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            sensorData->arrows.dCos[m][n] = cosinus;
            sensorData->arrows.dSin[m][n] = sinus;
        }
    }

    drawDisplay7Inch((void *)&sensorData->arrows);

    #ifdef DEBUG
    offOszi(2);                                    // for debuging with osci
    #endif

    #ifndef DEBUG_TOUCH
    writeMaxAnalogValue(sensorData->maxAnalogValue);
    #endif

    // Start sensorData-array ad-conversion. This starts the first of 16 ADC
    // read bursts. The other 15 bursts will be triggered in ADC1InterruptHandler().
//    startADConversion();

}


/***********************  ADC Interrupt handler  ********************************/
/* captures the analog sensorData array signals without busy waiting.           */
/* The hardware can convert up to 16 analog signals to digital when triggered.  */
/* When done with the conversion this function is called and another conversion */
/* is started until all sensor data was read.                                   */
/**/
/* So it has to be triggered 16 times to convert all 16*16 = 256 analog values. */
/* The digitized signals are being processed at the end.                        */
//*******************************************************************************/
void ADC1InterruptHandler(void)
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
        // process arrow length and store results to be later drawn on LCD.
        computeArrows(setup->relative, setup->maxArrowLength);
        // reset step counter for next use
        step = 0;
        // reset digital multiplexer inputs
        setMultiplexer(step);
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
    settings.setupNo = 0;

    // Setting No 0:
    settings.setup[0].relative = true;
    settings.setup[0].maxArrowLength = 100;
    settings.setup[0].adcAVG = true;

    settings.setup[0].coloredArrows = false;
    settings.setup[0].backColorArrowWindow = (color)WHITE;
    settings.setup[0].arrowColor = (color)BLACK;
    settings.setup[0].gridColor = (color)BLACK;

    settings.setup[0].backColorTable = (color)GREEN;
    settings.setup[0].backColorMotor = (color)BLACK;
    settings.setup[0].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[0].backColorImgStart = (color)BLUE;
    settings.setup[0].backColorImgStop  = (color)RED;
    settings.setup[0].backColorImgLeft  = (color)GREEN;
    settings.setup[0].backColorImgRight = (color)GREEN;
    settings.setup[0].spacerColor = (color)BLUE;
    settings.setup[0].fontColor = (color)BLACK;

// Setting No 1:
    settings.setup[1].relative = true;
    settings.setup[1].maxArrowLength = 100;
    settings.setup[1].adcAVG = true;

    settings.setup[1].coloredArrows = true;
    settings.setup[1].backColorArrowWindow = (color)BLACK;
    settings.setup[1].arrowColor = (color)BLACK;
    settings.setup[1].gridColor = (color)WHITE;

    settings.setup[1].backColorArrowWindow = (color)BLACK;
    settings.setup[1].backColorTable = (color)GREEN;
    settings.setup[1].backColorMotor = (color)BLACK;
    settings.setup[1].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[1].backColorImgStart = (color)BLUE;
    settings.setup[1].backColorImgStop  = (color)RED;
    settings.setup[1].backColorImgLeft  = (color)GREEN;
    settings.setup[1].backColorImgRight = (color)GREEN;
    settings.setup[1].spacerColor = (color)BLUE;
    settings.setup[1].fontColor = (color)BLACK;

// Setting No 2 (todo):
    settings.setup[2].relative = true;
    settings.setup[2].maxArrowLength = 100;
    settings.setup[2].adcAVG = true;

    settings.setup[2].coloredArrows = true;
    settings.setup[2].backColorArrowWindow = (color)BLACK;
    settings.setup[2].arrowColor = (color)BLACK;
    settings.setup[2].gridColor = (color)WHITE;

    settings.setup[2].backColorArrowWindow = (color)BLACK;
    settings.setup[2].backColorTable = (color)GREEN;
    settings.setup[2].backColorMotor = (color)BLACK;
    settings.setup[2].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[2].backColorImgStart = (color)BLUE;
    settings.setup[2].backColorImgStop  = (color)RED;
    settings.setup[2].backColorImgLeft  = (color)GREEN;
    settings.setup[2].backColorImgRight = (color)GREEN;
    settings.setup[2].spacerColor = (color)BLUE;
    settings.setup[2].fontColor = (color)BLACK;

// Setting No 3 (todo):
    settings.setup[3].relative = true;
    settings.setup[3].maxArrowLength = 100;
    settings.setup[3].adcAVG = true;

    settings.setup[3].coloredArrows = true;
    settings.setup[3].backColorArrowWindow = (color)BLACK;
    settings.setup[3].arrowColor = (color)BLACK;
    settings.setup[3].gridColor = (color)WHITE;

    settings.setup[3].backColorArrowWindow = (color)BLACK;
    settings.setup[3].backColorTable = (color)GREEN;
    settings.setup[3].backColorMotor = (color)BLACK;
    settings.setup[3].backColorArrowLengthMenu = (color)YELLOW;
    settings.setup[3].backColorImgStart = (color)BLUE;
    settings.setup[3].backColorImgStop  = (color)RED;
    settings.setup[3].backColorImgLeft  = (color)GREEN;
    settings.setup[3].backColorImgRight = (color)GREEN;
    settings.setup[3].spacerColor = (color)BLUE;
    settings.setup[3].fontColor = (color)BLACK;

// more setups could be added any time.
}


