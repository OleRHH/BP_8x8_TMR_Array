/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>


/*****************************  # global variables #   ****************************/
// Array holding the sin and cos values
extern int16_t DiffResults[2][8][8];

bool relative = true;
uint16_t maxArrowSize = 16;


/***********************  TIMER 0 interrupt handler   ************************/
/* Periodically measure the sensor Array values and draw them to the display */
void Timer0IntHandler(void)
{
    uint32_t maximumAnalogValue;
    GPIO_PORTN_DATA_R ^= YELLOW;                  // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= BLUE;                    // for debugging: set high when handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ReadArray();

    if(relative == true)
    {
        maximumAnalogValue = compute_relative(maxArrowSize);
    }
    else
    {
        maximumAnalogValue = compute_absolute(maxArrowSize);
    }

    write_Infos(relative, maxArrowSize, maximumAnalogValue);

    drawDisplay5Inch();
//    drawDisplay7Inch();

    // send command to stepper-motor to send back position data (absolute)
//    for(i = 0; i < 9; i++)
//    {
//        UARTCharPutNonBlocking(UART2_BASE, readCommand[i]);
//    }


    GPIO_PORTN_DATA_R ^= BLUE;                   // for debugging: set low when handler is finished
}


/*********************************************************************************************/
void UART0IntHandler(void)
{
    static char command[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t ui32Status;
    int i = 0;
    int checksum = 1;
    char receive[100];

    // Read the interrupt status of the UART.
    ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. If UART error interrupts were enabled, then
    // those interrupts could occur here and should be handled.  Since uDMA is
    // used for TX, it interrupt should not be enabled.
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_RX) //  || UIstatus & UART_INT_RT)
    {

        while(UARTCharsAvail(UART0_BASE))
        {
            receive[i++] = UARTCharGetNonBlocking(UART0_BASE);
        }

        // command to send Array Data via serial interface to Matlab
        if(receive[0] == '0')
            uDMAChannelEnable(UDMA_CHANNEL_UART0TX);    // The uDMA TX channel must be re-enabled to send a data burst.

        // set arrow relative/absolute and arrow size
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
            maxArrowSize = receive[4] << 24 | receive[5] << 16 | receive[6] << 8 | receive[7];
        }
        // commands for the stepper motor
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
    }
    // If the UART0 DMA TX channel is disabled, that means the TX DMA transfer is done.
    if(!uDMAChannelIsEnabled(UDMA_CHANNEL_UART0TX))
    {
        // initialize another DMA transfer to UART0 TX.
        uDMAChannelTransferSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC, (char *)DiffResults,
                                   (void *)(UART0_BASE + UART_O_DR),
                                   sizeof(DiffResults));
    }
}


/*********************************************************************************************/
void UART2IntHandler(void)
{
    int i = 0;
    char receive[100];
    char text[100];
    int value;
    GPIO_PORTN_DATA_R ^= 2;     // debug toggle for osci

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
            GPIO_PORTN_DATA_R ^= 1;     // debug toggle for osci
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
