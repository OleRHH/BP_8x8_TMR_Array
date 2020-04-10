/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>

#define YELLOW 0b0000100                            // Debug Output to oscilloscope
#define BLUE   0b0001000                            // Debug Output to oscilloscope

/*****************************  # global variables #   ****************************/


// Array holding the sin and cos values
extern int16_t DiffResults[2][8][8];

bool relative = true;
uint16_t maxArrowSize = 16;

/***********************  TIMER 0 interrupt handler   ************************/
/* Periodically measure the sensor Array values and draw them to the display */
void Timer0IntHandler(void)
{
    GPIO_PORTN_DATA_R ^= YELLOW;                        // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= BLUE;                          // for debugging: set high when handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ReadArray();
    Computations(relative, maxArrowSize);
    drawDisplay5Inch();
    print_string("Grid gap:", 10, 300, (COLOR)BLACK, (COLOR)WHITE);
    print_string("relative:", 40, 300, (COLOR)BLACK, (COLOR)WHITE);
    print_string("arrows", 60, 300, (COLOR)BLACK, (COLOR)WHITE);
    print_string("  max:", 90, 300, (COLOR)BLACK, (COLOR)WHITE);
    print_string("length", 110, 300, (COLOR)BLACK, (COLOR)WHITE);
    if(relative == true)
    {
        print_string("absolute", 200, 300, (COLOR)BLACK, (COLOR)WHITE);
        print_string("maximum:", 220, 300, (COLOR)BLACK, (COLOR)WHITE);
    }
    else
    {
        print_string("absolute", 200, 300, (COLOR)WHITE, (COLOR)WHITE);
        print_string("maximum:", 220, 300, (COLOR)WHITE, (COLOR)WHITE);
    }
//    drawDisplay7Inch();

    GPIO_PORTN_DATA_R ^= BLUE;                          // for debugging: set low when handler is finished
}


/*********************************************************************************************/
void UART0IntHandler(void)
{
    static char command[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t ui32Status;
    int i = 0;
    int checksum = 1;
    static char text[100];

   char receive[100];

    // Read the interrupt status of the UART.
    ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status. If UART error interrupts were enabled, then
    // those interrupts could occur here and should be handled.  Since uDMA is
    // used for TX, it interrupt should not be enabled.
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_RX) //  || UIstatus & UART_INT_RT)
    {
        GPIO_PORTN_DATA_R ^= 1;     // debug toggle for osci

        while(UARTCharsAvail(UART0_BASE))
        {
            receive[i++] = UARTCharGetNonBlocking(UART0_BASE);
        }

        // command to send Array Data via serial interface to Matlab
        if(receive[0] == '0')
            uDMAChannelEnable(UDMA_CHANNEL_UART0TX);    // The uDMA TX channel must be re-enabled to send a data burst.

        // arrow realtive/absolute and arrow size
        else if(receive[0] == '1')
        {
            if(receive[1] == '0')
            {
                relative = false;
                print_string("relative: false", 40, 300, (COLOR)BLACK, (COLOR)WHITE);
            }
            else if(receive[1] == '1')
            {
                relative = true;
                print_string("relative:  true", 40, 300, (COLOR)BLACK, (COLOR)WHITE);
            }
            maxArrowSize = (receive[5]-'0') * 100 + (receive[6]-'0') * 10 + (receive[7]-'0');
            sprintf(text, "%.3u", maxArrowSize);
            print_string(text, 90, 420, (COLOR)BLACK, (COLOR)WHITE);
        }
        // arrow realtive/absolute and arrow size
        else if(receive[0] == '1')
        {
            if(receive[1] == '0')
            {
                relative = false;
                print_string("relative: false", 40, 300, (COLOR)BLACK, (COLOR)WHITE);
            }
            else if(receive[1] == '1')
            {
                relative = true;
                print_string("relative:  true", 40, 300, (COLOR)BLACK, (COLOR)WHITE);
            }
            maxArrowSize = (receive[5]-'0') * 100 + (receive[6]-'0') * 10 + (receive[7]-'0');
            sprintf(text, "%.3u", maxArrowSize);
            print_string(text, 90, 420, (COLOR)BLACK, (COLOR)WHITE);
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
    // Get the interrupt status.
    uint32_t UIstatus = UARTIntStatus(UART2_BASE, true);
    UARTIntClear(UART2_BASE, UIstatus);

    /* reset transfer if '0' signal received */
    if( UIstatus & UART_INT_RX) //  || UIstatus & UART_INT_RT)
    {

    }
}
