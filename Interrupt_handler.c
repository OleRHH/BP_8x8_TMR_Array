/*
 * Interrupt_handler.c
 *
 *  Created on: 13.02.2020
 */
#include <Interrupt_handler.h>


extern int16_t DiffSinResults[8][8];
extern int16_t DiffCosResults[8][8];

/*****************************  # global variables #   ****************************/
//Base address pointer to Value Arrays
uint8_t *ucPtr;
int m = 0, n = 0;


/***********************  TIMER 1 interrupt handler   ***********************/
/* Periodicaly send Array Data via UART. This handler triggers the first byte */
void Timer0IntHandler(void)
{
    #define YELLOW 0b0000100     // Debug Output to oscilloscope
    GPIO_PORTN_DATA_R ^= YELLOW;

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ReadArray();
    Computations();
    draw_arrows();

    m = 0;
    n = 0;
    ucPtr = (uint8_t*) DiffSinResults;
    while(n < 16)
    {
        UARTCharPutNonBlocking(UART0_BASE, *(ucPtr++));
        m++;
        n++;
    }
    n = 0;
}


/*********************************************************************************************/
void UARTIntHandler(void)
{
    // Get the interrrupt status.
    uint32_t UIstatus = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, UIstatus);


    /* reset transfer if '0' signal received */
    if( UIstatus & UART_INT_RX) //  || UIstatus & UART_INT_RT)
    {
        // GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_PIN_4);
        char receive = UARTCharGet(UART0_BASE);
        while(UARTCharsAvail(UART0_BASE))
        {
            UARTCharGet(UART0_BASE);
        }

        if(receive == '0')
        {
            m = 256;
            n = 16;
            TimerDisable(TIMER0_BASE, TIMER_A);
            UARTFIFODisable(UART0_BASE);
            SysCtlDelay(1000);
            UARTFIFOEnable(UART0_BASE);
//            TimerLoadSet(TIMER0_BASE, TIMER_A, SysClock / 10);
            TimerEnable(TIMER0_BASE, TIMER_A);
        }
    }

    /* send data while m < 256 (= send sin 0-127 and cos 0-127 = 256 values) */
    if(UIstatus & UART_INT_TX && ( m < 256 ) )
    {
//        GPIO_PORTM_DATA_R ^= YELLOW;
        if(m == 128)
        {
            ucPtr = (uint8_t*) DiffCosResults;
        }
        while(n < 16)
        {
            UARTCharPutNonBlocking(UART0_BASE, *(ucPtr++));
            m++;
            n++;
        }
        n = 0;
    }
}


// some debug junk to be reused.. sometime ;)

//#define LED1   0b0000001
//#define LED2   0b0000010
//#define YELLOW 0b0000100     // Debug Output to oscilloscope
//#define BLUE   0b0001000     // Debug Output to oscilloscope
//
//// some debug outputs:
//GPIO_PORTN_DATA_R ^= YELLOW;
//GPIO_PORTN_DATA_R |= BLUE;
//GPIO_PORTN_DATA_R ^= 0b1;
//
//GPIO_PORTN_DATA_R ^= BLUE;
//GPIO_PORTN_DATA_R |= 0b10;


