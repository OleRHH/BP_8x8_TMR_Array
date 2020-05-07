#include <configuration.h>

//*****************************************************************************
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];


//*****************************************************************************
// The transmit buffer used for the UART transfer.
extern int16_t DiffResults[2][8][8];
extern char UART0receive[8];

/******************************************************************************************************/
/* The sensor array has 64 TMR-Sensors with four analog singals each (sin+, sin-, cos+, cos-).
 * This makes in total 256 analog signals to be measured. They are multiplexed to 16 analog inputs.
 * So there are 16 analog inputs to be measured simultaneously */
void ConfigureADC(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Wait for the ADC0 module to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1));

    // set hardware oversampling for better resolution
    ADCHardwareOversampleConfigure(ADC0_BASE, 64);
    ADCHardwareOversampleConfigure(ADC1_BASE, 64);

    // ADC, sample sequencer, trigger processor, priority
    ADCSequenceConfigure(ADC0_BASE,0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC1_BASE,1, ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceConfigure(ADC1_BASE,2, ADC_TRIGGER_PROCESSOR, 2);

    // Samplesequencer0, left side of array
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ROW_1_L); // sequence0,step0
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ROW_2_L); // sequence0,step1
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ROW_3_L); // sequence0,step2
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ROW_4_L); // sequence0,step3
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ROW_5_L); // sequence0,step4
    ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ROW_6_L); // sequence0,step5
    ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ROW_7_L); // sequence0,step6
    ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ROW_8_L|  // sequence0,step7
                             ADC_CTL_IE|ADC_CTL_END);

    // Sample sequencer 1, upper right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ROW_1_R); // sequence1,step0
    ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ROW_2_R); // sequence1,step1
    ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ROW_3_R); // sequence1,step2
    ADCSequenceStepConfigure(ADC1_BASE, 1, 3, ROW_4_R); //|  // sequence1,step3
//                             ADC_CTL_IE|ADC_CTL_END);

    // Sample sequencer 2, lower right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ROW_5_R); // sequence2,step0
    ADCSequenceStepConfigure(ADC1_BASE, 2, 1, ROW_6_R); // sequence2,step1
    ADCSequenceStepConfigure(ADC1_BASE, 2, 2, ROW_7_R); // sequence2,step2
    ADCSequenceStepConfigure(ADC1_BASE, 2, 3, ROW_8_R); //|  // sequence2,step3
//                             ADC_CTL_IE|ADC_CTL_END);							// incl.interrupt



    // Enable ADC
    ADCSequenceEnable(ADC0_BASE, 0); // ADC0 for sample sequencer0
    ADCSequenceEnable(ADC1_BASE, 1); // ADC1 for sample sequencer1
    ADCSequenceEnable(ADC1_BASE, 2);  // ADC1 for sample sequencer2



    IntPrioritySet(INT_ADC0SS0, HIGH_PRIORITY);             // set priority
    ADCIntRegister(ADC0_BASE, 0, ADC0IntHandler);
    ADCIntEnable(ADC0_BASE, 0);
    IntEnable(INT_ADC0SS0);

}


/*********************************************************************************************/
void configureUDMA(void)
{
    // Enable the uDMA controller at the system level. Wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)));

    // Enable the uDMA controller error interrupt. This interrupt will occur
    // if there is a bus error during a transfer.
    //    IntEnable(INT_UDMAERR);

    // Enable the uDMA controller.
    uDMAEnable();

    // Point at the control table to use for channel control structures.
    uDMAControlBaseSet(pui8ControlTable);

    // uDMA Rx:
    uDMAChannelAssign(UDMA_CH8_UART0RX);
    // Put the attributes in a known state for the uDMA UART0RX channel.  These
    // should already be disabled by default.
    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0RX, UDMA_ATTR_ALL);

    // Configure the control parameters for the primary control structure for
    // the UART RX channel. The transfer data size is 8 bits, the
    // source address does not increment since it will be reading from a
    // register. The destination address increment is byte 8-bit bytes.  The
    // arbitration size is set to 4 to match the RX FIFO trigger threshold.
    // The uDMA controller will use a 4 byte burst transfer if possible.  This
    // will be somewhat more efficient that single byte transfers.
    uDMAChannelControlSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
                            UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 |
                            UDMA_ARB_4);

    // Set up the transfer parameters for the UART RX primary control
    // structure.  The mode is set to basic, the transfer source is the
    // UART data register, and the destination is the receive buffer. The
    // transfer size is set to match the size of the receive buffer.
    uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT, UDMA_MODE_BASIC,
                             (void *)(UART0_BASE + UART_O_DR), (void *)UART0receive, sizeof(UART0receive));

    // uDMA Tx:
    uDMAChannelAssign(UDMA_CH9_UART0TX);

    // Put the attributes in a known state for the uDMA UART0TX channel.  These
    // should already be disabled by default.
    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_ALL);


    // Set the USEBURST attribute for the uDMA UART TX channel.  This will
    // force the controller to always use a burst when transferring data from
    // the TX buffer to the UART.  This is somewhat more efficient bus usage
    // than the default which allows single or burst transfers.
    uDMAChannelAttributeEnable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_USEBURST);

    // Configure the control parameters for the UART TX.  The uDMA UART TX
    // channel is used to transfer a block of data from a buffer to the UART.
    // The data size is 8 bits.  The source address increment is 8-bit bytes
    // since the data is coming from a buffer.  The destination increment is
    // none since the data is to be written to the UART data register.  The
    // arbitration size is set to 4, which matches the UART TX FIFO trigger
    // threshold.
    uDMAChannelControlSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_8 |
                              UDMA_DST_INC_NONE |
                              UDMA_ARB_8);

    // Now both the uDMA UART TX and RX channels are primed to start a
    // transfer.  As soon as the channels are enabled, the peripheral will
    // issue a transfer request and the data transfers will begin. Also it
    // will receive bytes
    uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
}


/*********************************************************************************************/
/* UART0 is used to transmit Array Data (256 byes) via RS232 and to receive control commands*/
void ConfigureUART0(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART and wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)));

    // Enable the UART peripheral and wait until it is ready.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)));

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART communication parameters.
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE);

    // TX trigger thresholds to 7 (14 bytes) and RX trigger thresholds to 4 (8 bytes).This will
    // be used by the uDMA controller to signal when more data should be transferred.
    UARTFIFOEnable(UART0_BASE);
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);

    // Enable the uDMA interface for both TX and RX channels.
    UARTDMAEnable(UART0_BASE, UART_DMA_RX | UART_DMA_TX);

    // Enable the UART DMA TX/RX interrupts.
    UARTIntRegister(UART0_BASE, UART0IntHandler);
//  UARTIntEnable(UART0_BASE, UART_INT_RX);
    UARTIntEnable(UART0_BASE, UART_INT_DMARX);

    // Enable the UART peripheral interrupts.
    IntEnable(INT_UART0);
}





//void ConfigureUART0(uint32_t SysClock)
//{
//    // Enable the uDMA controller at the system level.  Enable it to continue
//    // to run while the processor is in sleep.
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
////    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
//
//    // Enable the uDMA controller error interrupt.  This interrupt will occur
//    // if there is a bus error during a transfer.
////    IntEnable(INT_UDMAERR);
//
//    // Enable the uDMA controller.
//    uDMAEnable();
//
//    // Point at the control table to use for channel control structures.
//    uDMAControlBaseSet(pui8ControlTable);
//
//    // Enable the GPIO Peripheral used by the UART.
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//
//    // Enable the UART peripheral
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
//
//    // Configure GPIO Pins for UART mode.
//    GPIOPinConfigure(GPIO_PA0_U0RX);
//    GPIOPinConfigure(GPIO_PA1_U0TX);
//    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
//
//    // Configure the UART communication parameters.
//    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,
//                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
//                            UART_CONFIG_PAR_NONE);
//
//
//    // Set both the TX and RX trigger thresholds to 4.  This will be used by
//    // the uDMA controller to signal when more data should be transferred.  The
//    // uDMA TX and RX channels will be configured so that it can transfer 4
//    // bytes in a burst when the UART is ready to transfer more data.
//    UARTFIFOEnable(UART0_BASE);
//    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);
//
//    // Enable the UART for operation, and enable the uDMA interface for both TX
//    // and RX channels.
//    UARTEnable(UART0_BASE);
//    UARTDMAEnable(UART0_BASE, UART_DMA_RX | UART_DMA_TX);
//
//
//    // uDMA Rx:
//    // Put the attributes in a known state for the uDMA UART0RX channel.  These
//      // should already be disabled by default.
//      uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0RX,
//                                      UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
//                                      UDMA_ATTR_HIGH_PRIORITY |
//                                      UDMA_ATTR_REQMASK);
//
//      // Configure the control parameters for the primary control structure for
//      // the UART RX channel.  The primary contol structure is used for the "A"
//      // part of the ping-pong receive.  The transfer data size is 8 bits, the
//      // source address does not increment since it will be reading from a
//      // register.  The destination address increment is byte 8-bit bytes.  The
//      // arbitration size is set to 4 to match the RX FIFO trigger threshold.
//      // The uDMA controller will use a 4 byte burst transfer if possible.  This
//      // will be somewhat more effecient that single byte transfers.
//      uDMAChannelControlSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
//                                UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 |
//                                UDMA_ARB_4);
//
//
//      // Set up the transfer parameters for the UART RX primary control
//      // structure.  The mode is set to ping-pong, the transfer source is the
//      // UART data register, and the destination is the receive "A" buffer.  The
//      // transfer size is set to match the size of the
//      uDMAChannelTransferSet(UDMA_CHANNEL_UART0RX | UDMA_PRI_SELECT,
//                             UDMA_MODE_BASIC,
//                                 (void *)(UART0_BASE + UART_O_DR),
//                                 UART0receive, 8);
//
//
//    // uDMA Tx:
//    // Put the attributes in a known state for the uDMA UART0TX channel.  These
//    // should already be disabled by default.
//    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0TX,
//                                    UDMA_ATTR_ALTSELECT |
//                                    UDMA_ATTR_HIGH_PRIORITY |
//                                    UDMA_ATTR_REQMASK);
//
//
//    // Set the USEBURST attribute for the uDMA UART TX channel.  This will
//    // force the controller to always use a burst when transferring data from
//    // the TX buffer to the UART.  This is somewhat more effecient bus usage
//    // than the default which allows single or burst transfers.
//    uDMAChannelAttributeEnable(UDMA_CHANNEL_UART0TX, UDMA_ATTR_USEBURST);
//
//    // Configure the control parameters for the UART TX.  The uDMA UART TX
//    // channel is used to transfer a block of data from a buffer to the UART.
//    // The data size is 8 bits.  The source address increment is 8-bit bytes
//    // since the data is coming from a buffer.  The destination increment is
//    // none since the data is to be written to the UART data register.  The
//    // arbitration size is set to 4, which matches the UART TX FIFO trigger
//    // threshold.
//    uDMAChannelControlSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
//                              UDMA_SIZE_8 | UDMA_SRC_INC_8 |
//                              UDMA_DST_INC_NONE |
//                              UDMA_ARB_8);
//
//    // Now both the uDMA UART TX and RX channels are primed to start a
//    // transfer.  As soon as the channels are enabled, the peripheral will
//    // issue a transfer request and the data transfers will begin. Also it
//    // will receive bytes
////    uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
//    uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
//
//
//    // Enable the UART DMA TX/RX interrupts.
//    UARTIntRegister(UART0_BASE, UART0IntHandler);
////  UARTIntEnable(UART0_BASE, UART_INT_RX);
//    UARTIntEnable(UART0_BASE, UART_INT_DMARX);
//
//    // Enable the UART peripheral interrupts.
//    IntEnable(INT_UART0);
//}


/*********************************************************************************************/
/* UART2 is used for communication with the stepper motor via RS485 */
void ConfigureUART2(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART
    // Rx: PA6  Tx: PA7
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    //Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    // Configure GPIO Pins for UART mode
    GPIOPinConfigure(GPIO_PA6_U2RX);
    GPIOPinConfigure(GPIO_PA7_U2TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_6 | GPIO_PIN_7);


    UARTConfigSetExpClk(UART2_BASE, SysClock, 9600, //115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    UARTIntRegister(UART2_BASE, UART2IntHandler);
    UARTFIFOEnable(UART2_BASE);
    UARTFIFOLevelSet(UART2_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);
    IntEnable(INT_UART2);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT); // | UART_INT_TX);
}


/*********************************************************************************************/
void ConfigureGPIO(void)
{
    // Set Port L  0-4: Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Set Port M Pins 0-7: Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // Set Port Q Pins 0-4: LCD Control output:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);  // Clock Port Q
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                                         | GPIO_PIN_4);
}


/*********************************************************************************************/
void ConfigureTimer0(uint32_t SysClock)
/* this timer is used to run the program periodically. Upon call the analog inputs are read,
 * converted and send to the LC-Display */
{
    // Configure Timer0 Interrupt
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);           // Clock Gate enable TIMER0upt
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysClock / 10);      // fires every 100 ms
    TimerEnable(TIMER0_BASE, TIMER_A);
    IntPrioritySet(INT_TIMER0A, LOW_PRIORITY);             // set priority
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0IntHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

/******************************************************************************************************/
void ConfigureUDMA(void)
{
//    // enable the UDMA peripheral
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
//
//    // wait for the UDMA module to be ready
//    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))
//    {
//    }
//
//    // enable the uDMA controller
//    uDMAEnable();
//
//    // set the base for the channel control table
//    uDMAControlBaseSet(&puiDMAcontroltable);
//
//    // set attribute
//    uDMAChannelAttributeDisable(UDMA_CHANNEL_SW, UDMA_ATTR_ALL);
//
//    // set channel control
//    uDMAChannelControlSet(
//    UDMA_CH8_UART0RX | UDMA_CH9_UART0TX | UDMA_CH9_UART0TX | UDMA_PRI_SELECT,
//                          UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE |
//                          UDMA_ARB_1);
//
//    uDMAChannelEnable(UDMA_CHANNEL_SW);
//    uDMAChannelRequest(UDMA_CHANNEL_SW);

}



