/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>


/*****************************  # global variables #   ****************************/

static bool relative = true, oversampling = true;
static uint16_t maxArrowLength = 32;
static uint32_t maximumAnalogValue;
static COLOR backColor = (COLOR)WHITE;


/***********************  TIMER 0 interrupt handler   ************************/
/* Periodically measure the sensor Array values and draw them to the display */
void Timer0IntHandler(void)
{
    GPIO_PORTN_DATA_R ^= YELLOW;       // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= BLUE;         // for debugging: set high when handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    drawDisplay5Inch(backColor);

//    drawDisplay7Inch();
    write_Infos(relative, oversampling, maxArrowLength, maximumAnalogValue);

    // start the first of 16 ADC read. The others will be triggered in the ADC handler
    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);

    // send command to stepper-motor to send back position data (absolute)
//    for(i = 0; i < 9; i++)
//    {
//        UARTCharPutNonBlocking(UART2_BASE, readCommand[i]);
//    }

    GPIO_PORTN_DATA_R ^= BLUE;                   // for debugging: set low when handler is finished
}


/*********************************************************************************************/
/* capture the analog sensor array signals without busy waiting until ad-conversion is complete */
void ADC0IntHandler(void)
{
    static uint16_t step = 0;

    ADCIntClear(ADC0_BASE, 0);

    // advance step count each time an AD-conversion is finished
    step++;

    // Port L is used to address the analog multiplexers on the TMR sensor array
    if(step == 8)
    {
        GPIO_PORTL_DATA_R &= ~GPIO_PIN_4;
    }
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);

    // store the previously captured analog values into local buffers
    ReadArray(step-1);

    // trigger the next AD-conversion (16 in total)
    if(step <= 15)
    {
        ADCProcessorTrigger(ADC0_BASE, 0);
        ADCProcessorTrigger(ADC1_BASE, 1);
        ADCProcessorTrigger(ADC1_BASE, 2);
    }

    // after 16 conversions process analog data and prepare for next ADC cycle
    else
    {
        maximumAnalogValue = (relative == true) ? computeRelative(maxArrowLength): computeAbsolute(maxArrowLength);
        step = 0;
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);
        GPIO_PORTL_DATA_R |= GPIO_PIN_4;
    }
}


/*********************************************************************************************/
/* communication via RS232 interface with the PC (sending sensor data to matlab, receiving commands etc) */
void UART0IntHandler(void)
{
    static char stepperMotorCommand[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t ui32Status;
    int i = 0;
    int checksum = 1;

    // Read the interrupt status of the UART.
    ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_DMARX)
    {
        prepareReceiveDMA();

        // receive[] contains commands from another device. The commands depend on
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
            for(i = 0; i < 9; i++)
            {
                UARTCharPutNonBlocking(UART2_BASE, stepperMotorCommand[i]);
            }
        }
        // oversampling enabled/disabled
        else if(UART0receive[0] == '4')
        {
            if(UART0receive[1] == '0')
            {
                GPIO_PORTN_DATA_R ^= LED_D1;
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
        // change background color
        else if(UART0receive[0] == '5')
        {
            if(UART0receive[1] == '0')
            {
                backColor = (COLOR)WHITE;
                writeScreenColor5INCH(backColor);
            }
            else if(UART0receive[1] == '1')
            {
                backColor = (COLOR)BLACK;
                writeScreenColor5INCH(backColor);
            }
            else if(UART0receive[1] == '2')
            {
                backColor = (COLOR)GREY;
                writeScreenColor5INCH(backColor);
            }
        }
        // more commands.
        else if(UART0receive[0] == '6')
        {
           // todo: additional commands from UART here
        }
    }
}


/******************************************************************************************/
/* send commands to stepper-motor and receive telemetry data from stepper-motor via RS485 */
void UART2IntHandler(void)
{
    int i = 0;
    char UART0receive[100];
    char text[100];
    int value;

    uint32_t UIstatus = UARTIntStatus(UART2_BASE, true);    // Get the interrupt status.
    UARTIntClear(UART2_BASE, UIstatus);

    if( UIstatus & UART_INT_RX  || UIstatus & UART_INT_RT)
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
            print_string(text, 190, 360, (COLOR)BLACK, (COLOR)WHITE);
        }
    }
}
