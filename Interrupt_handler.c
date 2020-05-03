/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>
#include <math.h>

/*****************************  # global variables #   ****************************/
// Array holding the sin and cos values
extern int16_t DiffResults[2][8][8];
extern int16_t DiffCosResults[8][8];
extern int16_t DiffSinResults[8][8];

char UART0receive[8];

bool relative = true, oversampling = false;
uint16_t maxArrowSize = 32;
uint32_t maximumAnalogValue;
COLOR backColor = (COLOR)WHITE;

#define _2PI ( 6.28318530718 )
uint16_t a = 0, A = 400;

/***********************  TIMER 0 interrupt handler   ************************/
/* Periodically measure the sensor Array values and draw them to the display */
void Timer0IntHandler(void)
{
    uint16_t m, n;

    GPIO_PORTN_DATA_R ^= YELLOW;       // for debugging: toggle debug output each time handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

//    for(m = 0; m <= 7; m++)
//    {
//        for(n = 0; n <= 7; n++)
//        {
//            DiffCosResults[m][n] = 32 * cos(_2PI * a/A);
//            DiffSinResults[m][n] = 32 * sin(_2PI * a/A);
//        }
//    }
//    if(++a == 400) a = 0;

//    GPIO_PORTN_DATA_R |= BLUE;         // for debugging: set high when handler is called

    drawDisplay5Inch(backColor);
    // start the first of 16 ADC read. The others will be triggered in the ADC handler
    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);

//    GPIO_PORTN_DATA_R |= BLUE;         // for debugging: set high when handler is called

//    drawDisplay7Inch();
    write_Infos(relative, oversampling, maxArrowSize, maximumAnalogValue);


    // send command to stepper-motor to send back position data (absolute)
//    for(i = 0; i < 9; i++)
//    {
//        UARTCharPutNonBlocking(UART2_BASE, readCommand[i]);
//    }
//    GPIO_PORTN_DATA_R ^= BLUE;                   // for debugging: set low when handler is finished
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
        maximumAnalogValue = (relative == true) ? compute_relative(maxArrowSize): compute_absolute(maxArrowSize);
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
        // prepare uDMA for the next UART0receive (8 bytes)
        uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                                   (void *)(UART0_BASE + UART_O_DR),
                                   UART0receive, sizeof(UART0receive));
        uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
//        printf("%s %x%x%x%x \n", UART0receive, UART0receive[4], UART0receive[5], UART0receive[6], UART0receive[7]);

        // now its time to see what we UART0received:
        // '0': send Array Data (256 bytes) via serial interface to Matlab

        if(UART0receive[0] == '0')
        {
            // Set up the transfer parameters for the uDMA UART TX channel.  This will
            // configure the transfer source and destination and the transfer size.
            // Basic mode is used because the peripheral is making the uDMA transfer
            // request.  The source is the TX buffer and the destination is the UART
            // data register.
            uDMAChannelTransferSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                                       UDMA_MODE_BASIC, (char *)DiffResults,
                                       (void *)(UART0_BASE + UART_O_DR),
                                       sizeof(DiffResults));
            // The uDMA TX channel must be enabled to send a data burst.
            // It starts immediately because the Tx FIFO is empty (or should be)
            uDMAChannelEnable(UDMA_CHANNEL_UART0TX);
        }

        // '1': set arrow relative/absolute and arrow size
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
            // restore the 32 bit integer what was send in four peaces
            maxArrowSize = UART0receive[4] << 24 | UART0receive[5] << 16 | UART0receive[6] << 8 | UART0receive[7];
        }
        // '2': commands for the stepper motor
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
                write_screen_color5INCH(backColor);
            }
            else if(UART0receive[1] == '1')
            {
                backColor = (COLOR)BLACK;
                write_screen_color5INCH(backColor);
            }
            else if(UART0receive[1] == '2')
            {
                backColor = (COLOR)GREY;
                write_screen_color5INCH(backColor);
            }
        }
    }
}


/*********************************************************************************************/
/* send commands to the stepper motor and receiving telemetry data from motor via RS485 */
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
