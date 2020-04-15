/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>


/*****************************  # global variables #   ****************************/
// Array holding the sin and cos values
extern int16_t DiffResults[2][8][8];
char receive[8];

bool relative = true;
bool busy = false;
uint16_t maxArrowSize = 32;
uint16_t step = 0;

uint32_t maximumAnalogValue;
/***********************  TIMER 0 interrupt handler   ************************/
/* Periodically measure the sensor Array values and draw them to the display */
void Timer0IntHandler(void)
{

    GPIO_PORTN_DATA_R ^= YELLOW;                  // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= BLUE;                    // for debugging: set high when handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

//    IntTrigger(INT_ADC0SS0);

    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);

    drawDisplay5Inch();
    write_Infos(relative, maxArrowSize, maximumAnalogValue);

//    drawDisplay7Inch();

    // send command to stepper-motor to send back position data (absolute)
//    for(i = 0; i < 9; i++)
//    {
//        UARTCharPutNonBlocking(UART2_BASE, readCommand[i]);
//    }

    GPIO_PORTN_DATA_R ^= BLUE;                   // for debugging: set low when handler is finished
}




void ADC0IntHandler(void)
{
    while(!ADCIntStatus(ADC0_BASE, 0, false));
    while(!ADCIntStatus(ADC1_BASE, 1, false));
    while(!ADCIntStatus(ADC1_BASE, 2, false));
    ADCIntClear(ADC0_BASE, 0);
    ADCIntClear(ADC1_BASE, 1);
    ADCIntClear(ADC1_BASE, 2);

    if(step == 8)
    {
        GPIO_PORTL_DATA_R &= ~GPIO_PIN_4;
    }

    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);

    GetADCValues();
    SysCtlDelay(8);
    ReadArray(step);

    (relative == true) ? compute_relative(maxArrowSize): compute_absolute(maxArrowSize);

    if(step < 15)
    {
        step++;
        ADCProcessorTrigger(ADC0_BASE, 0);
        ADCProcessorTrigger(ADC1_BASE, 1);
        ADCProcessorTrigger(ADC1_BASE, 2);
    }
    else
    {
        step = 0;
        GPIO_PORTL_DATA_R |= GPIO_PIN_4;
    }

////    GPIO_PORTN_DATA_R ^= YELLOW;
//    while(!ADCIntStatus(ADC0_BASE, 0, false));
//    // Quit interrupt
//
//    while(!ADCIntStatus(ADC1_BASE, 1, false));
//    // Quit interrupt
//
//    while(!ADCIntStatus(ADC1_BASE, 2, false));
////     Quit interrupt
}


/*********************************************************************************************/
void UART0IntHandler(void)
{
    static char command[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t ui32Status;
    int i = 0;
    int checksum = 1;

    // Read the interrupt status of the UART.
    ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. We are expecting a uDMA Receive Interrupt
    UARTIntClear(UART0_BASE, ui32Status);

//    GPIO_PORTN_DATA_R ^= LED_D2;

    if( ui32Status & UART_INT_DMARX) //  || UIstatus & UART_INT_RT)
    {
        // prepare uDMA for the next receive (8 bytes)
        uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC,
                                   (void *)(UART0_BASE + UART_O_DR),
                                   receive, sizeof(receive));
        uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
        printf("%s\n", receive);

        // now its time to see what we received:
        // '0': send Array Data (256 bytes) via serial interface to Matlab

        if(receive[0] == '0')
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
        else if(receive[0] == '1')
        {
            if(receive[1] == '0')
            {
                relative = false;
            }
            else if(receive[1] == '1')
            {
                relative = true;
            }
            // restore the 32 bit integer what was send in four peaces
            maxArrowSize = receive[4] << 24 | receive[5] << 16 | receive[6] << 8 | receive[7];
        }
        // '2': commands for the stepper motor
        else if(receive[0] == '2')
        {
            for(i = 1; i < 8; i++)
            {
                command[i] = receive[i];
                checksum += receive[i];
            }
            command[8] = checksum;
            for(i = 0; i < 9; i++)
            {
                UARTCharPutNonBlocking(UART2_BASE, command[i]);
            }
        }
        // oversampling enabled/disabled
        else if(receive[0] == '4')
        {
            if(receive[1] == '0')
            {
                GPIO_PORTN_DATA_R ^= LED_D1;
                ADCHardwareOversampleConfigure(ADC0_BASE, 1);
                ADCHardwareOversampleConfigure(ADC1_BASE, 1);
                printf("Oversampling off\n");
            }
            else if(receive[1] == '1')
            {
                // set hardware oversampling for better resolution
                ADCHardwareOversampleConfigure(ADC0_BASE, 64);
                ADCHardwareOversampleConfigure(ADC1_BASE, 64);
                printf("Oversampling on\n");

            }
        }
    }
}


/*********************************************************************************************/
void UART2IntHandler(void)
{
    int i = 0;
    char receive[100];
    char text[100];
    int value;

    uint32_t UIstatus = UARTIntStatus(UART2_BASE, true);    // Get the interrupt status.
    UARTIntClear(UART2_BASE, UIstatus);

    if( UIstatus & UART_INT_RX  || UIstatus & UART_INT_RT)
    {
        while(UARTCharsAvail(UART2_BASE))
        {
            receive[i++] = UARTCharGetNonBlocking(UART2_BASE);
        }

        // receive position data from stepper-motor (absolute)
        if(receive[2] == 100 && receive[3] == 6)
        {
            value = receive[4];
            value <<= 8;
            value |= receive[5];
            value <<= 8;
            value |= receive[6];
            value <<= 8;
            value |= receive[7];

            // print the stepper-motor position on the LC-Display
            sprintf(text, "%.6u", value);
            print_string(text, 190, 360, (COLOR)BLACK, (COLOR)WHITE);
        }
    }
}
