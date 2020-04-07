/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>

#define YELLOW 0b0000100                            // Debug Output to oscilloscope
#define BLUE   0b0001000                            // Debug Output to oscilloscope

/*****************************  # global variables #   ****************************/
char command[9] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Array holding the sin and cos values
extern int16_t DiffResults[2][8][8];

/***********************  TIMER 0 interrupt handler   ***********************/
/* Periodically measure the Array values and draw them to the display
   Also start the UART transmit sequences: This handler triggers the first 16 bytes */
void Timer0IntHandler(void)
{
    GPIO_PORTN_DATA_R ^= YELLOW;                        // for debugging: toggle debug output each time handler is called
    GPIO_PORTN_DATA_R |= BLUE;                          // for debugging: set high when handler is called

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ReadArray();
    Computations();
    drawDisplay5Inch();
//    drawDisplay7Inch();

    GPIO_PORTN_DATA_R ^= BLUE;                          // for debugging: set low when handler is finished
}


/*********************************************************************************************/
void UART0IntHandler(void)
{
    uint32_t ui32Status;
    int i = 0;
    int checksum = 1;

   char receive[100];

    // Read the interrupt status of the UART.
    ui32Status = UARTIntStatus(UART0_BASE, 1);

    // Clear any pending status, even though there should be none since no UART
    // interrupts were enabled.  If UART error interrupts were enabled, then
    // those interrupts could occur here and should be handled.  Since uDMA is
    // used for both the RX and TX, then neither of those interrupts should be
    // enabled.
    UARTIntClear(UART0_BASE, ui32Status);

    if( ui32Status & UART_INT_RX) //  || UIstatus & UART_INT_RT)
    {
        GPIO_PORTN_DATA_R ^= 1;     // debug toggle for osci

        while(UARTCharsAvail(UART0_BASE))
        {
            receive[i++] = UARTCharGetNonBlocking(UART0_BASE);
        }

        // command to send Array Data via serial interface to Matlab
        if(receive[0] == 0)
            uDMAChannelEnable(UDMA_CHANNEL_UART0TX);    // The uDMA TX channel must be re-enabled to send a data burst.

        // commands for the stepper motor
        else if(receive[0] == 1)
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
