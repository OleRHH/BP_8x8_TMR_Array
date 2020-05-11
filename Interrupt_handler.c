/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>


/*****************************  # global variables #   ****************************/

static bool relative = false, oversampling = true;
static uint16_t maxArrowLength = 32;
static uint32_t maximumAnalogValue;
static COLOR backColor = (COLOR)WHITE;


/***********************  TIMER 0 interrupt handler  ******************************/
/* Periodically measure the sensor Array values and draw them to the display      */
void Timer0IntHandler(void)
{
    GPIO_PORTN_DATA_R ^= OSZ_YEL;       // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= OSZ_BLUE;      // for debugging: set high when handler is called

    timer0IntClear();
    drawDisplay5Inch(backColor);

//    drawDisplay7Inch();
    writeInfos(relative, oversampling, maxArrowLength, maximumAnalogValue);

    // start the first of 16 ADC read. The others will be triggered in the ADC handler
    startAdcConversion();

//    sendCommandToMotor(charCommand, 9);

    GPIO_PORTN_DATA_R ^= OSZ_BLUE;                   // for debugging: set low when handler is finished
}


/***********************  ADC Interrupt handler  **********************************/
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
    static char stepperMotorCommand[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    char * UART0receive;
    int i = 0;
    int checksum = 1;

    // Read the interrupt status of the UART.
    uint32_t ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_DMARX)
    {
        UART0receive = getDataFromPc();
        prepareReceiveDMA();

        // receive[] contains commands from Pc. The commands depend on
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
            maxArrowLength = UART0receive[4] << 24 | UART0receive[5] << 16 | UART0receive[6] << 8 | UART0receive[7];
        }
        // '2': commands past forwarded to the stepper-motor by UART2.
        else if(UART0receive[0] == '2')
        {
            for(i = 1; i < 8; i++)
            {
                stepperMotorCommand[i] = UART0receive[i];
                checksum += UART0receive[i];
            }
            stepperMotorCommand[8] = checksum;
            sendCommandToMotor(stepperMotorCommand, 9);
        }
        // oversampling enabled/disabled
        else if(UART0receive[0] == '4')
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
        else if(UART0receive[0] == '5')
        {
           // todo: additional commands from UART here
        }
    }
}


/***********************  UART2 Interrupt handler  ****************************************/
/* receive telemetry data from stepper-motor via RS485 */
void UART2IntHandler(void)
{
    int i = 0;
    char UART0receive[100];
    char text[100];
    int value;

    uint32_t UIstatus = UARTIntStatus(UART2_BASE, true);    // Get the interrupt status.
    UARTIntClear(UART2_BASE, UIstatus);

    if( UIstatus & UART_INT_RX)
    {
        while(UARTCharsAvail(UART2_BASE))
        {
            UART0receive[i++] = UARTCharGetNonBlocking(UART2_BASE);
        }

        // UART0receive position data from stepper-motor (absolute)
        if(UART0receive[2] == 100 && UART0receive[3] == 6)
        {
            value = UART0receive[4];
            value <<= 8;
            value |= UART0receive[5];
            value <<= 8;
            value |= UART0receive[6];
            value <<= 8;
            value |= UART0receive[7];

            // print the stepper-motor position on the LC-Display
            sprintf(text, "%.6u", value);
            printString(text, 190, 360, (COLOR)BLACK, (COLOR)WHITE);
        }
    }
}
