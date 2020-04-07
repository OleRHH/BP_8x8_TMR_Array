#include <configuration.h>

//*****************************************************************************
// The size of the UART transmit and receive buffers.  They do not need to be
// the same size.
#define UART_TXBUF_SIZE         256

//*****************************************************************************
// The transmit and receive buffers used for the UART transfers.  There is one
// transmit buffer and a pair of recieve ping-pong buffers.
extern int16_t DiffResults[2][8][8];
char g_ui8TxBuf[256];



// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

/******************************************************************************************************/
void ConfigureADC(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Wait for the ADC0 module to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1));

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
    ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ROW_8_L); // sequence0,step7

    // Sample sequencer 1, upper right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ROW_1_R); // sequence1,step0
    ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ROW_2_R); // sequence1,step1
    ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ROW_3_R); // sequence1,step2
    ADCSequenceStepConfigure(ADC1_BASE, 1, 3, ROW_4_R); // sequence1,step3

    // Sample sequencer 1, lower right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ROW_5_R); // sequence2,step0
    ADCSequenceStepConfigure(ADC1_BASE, 2, 1, ROW_6_R); // sequence2,step1
    ADCSequenceStepConfigure(ADC1_BASE, 2, 2, ROW_7_R); // sequence2,step2
    ADCSequenceStepConfigure(ADC1_BASE, 2, 3, ROW_8_R|  // sequence2,step3
    ADC_CTL_IE|ADC_CTL_END);							// incl.interrupt

    // Enable ADC
    ADCSequenceEnable(ADC0_BASE, 0); // ADC0 for sample sequencer0
    ADCSequenceEnable(ADC1_BASE, 1); // ADC1 for sample sequencer1
    ADCSequenceEnable(ADC1_BASE, 2);  // ADC1 for sample sequencer2
}


/*********************************************************************************************/
/* UART0 is used to transfer Array Data (256 byes) via RS232 */
void ConfigureUART0(uint32_t SysClock)
{
    // Enable the uDMA controller at the system level.  Enable it to continue
    // to run while the processor is in sleep.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
//    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);

    // Enable the uDMA controller error interrupt.  This interrupt will occur
    // if there is a bus error during a transfer.
//    IntEnable(INT_UDMAERR);

    // Enable the uDMA controller.
    uDMAEnable();

    // Point at the control table to use for channel control structures.
    uDMAControlBaseSet(pui8ControlTable);

    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable the UART peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART communication parameters.
    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE);


    // Set both the TX and RX trigger thresholds to 4.  This will be used by
    // the uDMA controller to signal when more data should be transferred.  The
    // uDMA TX and RX channels will be configured so that it can transfer 4
    // bytes in a burst when the UART is ready to transfer more data.
    UARTFIFOEnable(UART0_BASE);
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);

    // Enable the UART for operation, and enable the uDMA interface for both TX
    // and RX channels.
    UARTEnable(UART0_BASE);
    UARTDMAEnable(UART0_BASE, UART_DMA_TX);


    // Put the attributes in a known state for the uDMA UART0TX channel.  These
    // should already be disabled by default.
    uDMAChannelAttributeDisable(UDMA_CHANNEL_UART0TX,
                                    UDMA_ATTR_ALTSELECT |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);


    // Set the USEBURST attribute for the uDMA UART TX channel.  This will
    // force the controller to always use a burst when transferring data from
    // the TX buffer to the UART.  This is somewhat more effecient bus usage
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
                              UDMA_ARB_4);

    // Set up the transfer parameters for the uDMA UART TX channel.  This will
    // configure the transfer source and destination and the transfer size.
    // Basic mode is used because the peripheral is making the uDMA transfer
    // request.  The source is the TX buffer and the destination is the UART
    // data register.
    uDMAChannelTransferSet(UDMA_CHANNEL_UART0TX | UDMA_PRI_SELECT,
                               UDMA_MODE_BASIC, (char *)DiffResults,
                               (void *)(UART0_BASE + UART_O_DR),
                               sizeof(DiffResults));

    // Now both the uDMA UART TX and RX channels are primed to start a
    // transfer.  As soon as the channels are enabled, the peripheral will
    // issue a transfer request and the data transfers will begin.
                                        //    ROM_uDMAChannelEnable(UDMA_CHANNEL_UART0RX);
//    uDMAChannelEnable(UDMA_CHANNEL_UART0TX);      // starts the first uDMA transfer (256 bytes)

    // Enable the UART DMA TX/RX interrupts.
    UARTIntRegister(UART0_BASE, UART0IntHandler);
    UARTIntEnable(UART0_BASE, UART_INT_RX);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_DMATX | UART_INT_DMATX);

    // Enable the UART peripheral interrupts.
    IntEnable(INT_UART0);
}


/*********************************************************************************************/
/* UART3 is used for communication with the stepper motor via RS485 */
void ConfigureUART2(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART
    // Rx: PA4  Tx: PA5
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    // Configure GPIO Pins for UART mode
    GPIOPinConfigure(GPIO_PA6_U2RX);
    GPIOPinConfigure(GPIO_PA7_U2TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_6|GPIO_PIN_7);


    UARTConfigSetExpClk(UART2_BASE, SysClock, 9600, //115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    // Enable the UART interrupt
    UARTFIFOEnable(UART2_BASE);
//    IntPrioritySet(INT_UART2, LOW_PRIORITY);                // set priority
//    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    UARTFIFOLevelSet(UART2_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
//    UARTTxIntModeSet(UART2_BASE, UART_TXINT_MODE_EOT);
//    UARTIntRegister(UART2_BASE, UART2IntHandler);
//
//    IntEnable(INT_UART2);
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
{
    // Configure Timer0 Interrupt
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);           // Clock Gate enable TIMER0upt
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysClock / 10);      // fires every 100 ms
    TimerEnable(TIMER0_BASE, TIMER_A);
    IntPrioritySet(INT_TIMER0A, HIGH_PRIORITY);             // set priority
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

/******************************************************************************************************/
// LCD Panel initialize:
void ConfigureLCD5Inch(uint32_t SysClock) {
    uint32_t value;

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    write_command(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    write_cmd_data(0x24);                   //
    write_cmd_data(0x02);                   //
    write_cmd_data(0x04);                   //

    write_command(START_PLL);               // Start PLL
    write_cmd_data(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(START_PLL);               // Lock PLL
    write_cmd_data(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    value = 0x01EFFF;
    write_command(SET_LSHIFT);              // Set LCD Pixel Clock 12.7 Mhz (0x01EFFF)
    write_cmd_data(value>>16);              //
    write_cmd_data(value>>8);               //
    write_cmd_data(value);                  //
    write_command(SET_LCD_MODE);            // Set LCD Panel mode to:
    write_cmd_data(0x20);                   // ..TFT panel 24bit
    write_cmd_data(0x00);                   // ..TFT mode
    write_cmd_data(0x01);                   // Horizontal size 480-1 (aka 479 ;)    HB
    write_cmd_data(0xDF);                   // Horizontal size 480-1                LB
    write_cmd_data(0x01);                   // Vertical size 272-1   (aka 271 ;)    HB
    write_cmd_data(0x0F);                   // Vertical size 272-1                  LB
    write_cmd_data(0x00);                   // even/odd line RGB

    write_command(SET_HORI_PERIOD);         // Set Horizontal period
    write_cmd_data(0x02);                   // Set HT total pixel=531               HB
    write_cmd_data(0x13);                   // Set HT total pixel=531               LB
    write_cmd_data(0x00);                   // Set Horiz.sync pulse start pos = 43  HB
    write_cmd_data(0x2B);                   // Set Horiz.sync pulse start pos = 43  LB
    write_cmd_data(0x0A);                   // Set horiz.sync pulse with = 10
    write_cmd_data(0x00);                   // Set horiz.Sync pulse start pos= 8    HB
    write_cmd_data(0x08);                   // Set horiz.Sync pulse start pos= 8    LB
    write_cmd_data(0x00);                   //

    write_command(SET_VERT_PERIOD);         // Set Vertical Period
    write_cmd_data(0x01);                   // Set VT lines = 288                   HB
    write_cmd_data(0x20);                   // Set VT lines = 288                   LB
    write_cmd_data(0x00);                   // Set VPS = 12                         HB
    write_cmd_data(0x0C);                   // Set VPS = 12                         LB
    write_cmd_data(0x0A);                   // Set vert.sync pulse with = 10
    write_cmd_data(0x00);                   // Set vert.Sync pulse start pos= 8    HB
    write_cmd_data(0x00);                   // Set vert.Sync pulse start pos= 8    LB
    write_cmd_data(0x04);                   //

    write_command(0xF0);                    // Set LCD color data format
    write_cmd_data(0x00);                   // Set pixel data format = 8 bit

    write_command(0x30);                    // Set partial area
    write_cmd_data(0);                      // Start row High
    write_cmd_data(0);                      // Start row Low
    value = 543;
    write_cmd_data(value >> 8);             // Stop row High
    write_cmd_data(value);                  // Stop row Low

    write_command(0x12);                    // enter partial mode

    write_command(0x33);                    // Set scroll area
    value = 0;
    write_cmd_data(value >> 8);             // TFA high byte (TFA = 0)
    write_cmd_data(value);                  // TFA low byte
    value = 544;
    write_cmd_data(value >> 8);             // VSA high byte (VSA = 272)
    write_cmd_data(value);                  // VSA low byte
    value = 0;
    write_cmd_data(value >> 8);             // BFA high byte (BFA = 543)
    write_cmd_data(value);                  // BFA low byte

    write_command(0x29);                    // Set display on
}


/******************************************************************************************************/
void ConfigureLCD7Inch(uint32_t SysClock) {

    GPIO_PORTQ_DATA_R = 0x00;
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    write_command(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    write_cmd_data(0x24);                   //
    write_cmd_data(0x02);                   //
    write_cmd_data(0x04);                   //

    write_command(START_PLL);               // Start PLL
    write_cmd_data(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(START_PLL);               // Lock PLL
    write_cmd_data(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    write_command(0xe6);       // pixel clock frequency
    write_cmd_data(0x04); // LCD_FPR = 290985 = 33.300 Mhz Result for 7" Display
    write_cmd_data(0x70);   //
    write_cmd_data(0xA9);   //

    write_command(0xB0);          //SET LCD MODE   SIZE !!
    write_cmd_data(0x19); //19 TFT panel data width - Enable FRC or dithering for color depth enhancement 8 18  1f- 38
    write_cmd_data(0x20); //SET TFT MODE & hsync+Vsync+DEN MODE   20  or 00
    write_cmd_data(0x03); //SET horizontal size=800+1 HightByte   !!!!!!!!!!!!
    write_cmd_data(0x1F);      //SET horizontal size=800+1 LowByte
    write_cmd_data(0x01);      //SET vertical size=480+1 HightByte
    write_cmd_data(0xDF);      //SET vertical size=480+1 LowByte
    write_cmd_data(0x00); //Even line RGB sequence / Odd line RGB sequence RGB

    write_command(0xB4);            // Set Horizontal Period
    write_cmd_data(0x03); //03 High byte of horizontal total period (display + non-display)
    write_cmd_data(0x5E); //51 Low byte of the horizontal total period (display + non-display)
    write_cmd_data(0x00); //High byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data.
    write_cmd_data(0x46); //**   // 46 Low byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data
    write_cmd_data(0x09);       // Set the vertical sync
//    write_cmd_data(0x00);       //SET Hsync pulse start
//    write_cmd_data(0x00);                                       //00
//    write_cmd_data(0x00); //SET Hsync pulse subpixel start position  //00
    write_cmd_data(0x00);             // Set horiz.Sync pulse start pos= 8    HB
    write_cmd_data(0x08);             // Set horiz.Sync pulse start pos= 8    LB
    write_cmd_data(0x00);                   //
    //   ** too small will give you half a PICTURE !!

    write_command(0xB6);          //Set Vertical Period
    write_cmd_data(0x01); //01 High byte of the vertical total (display + non-display)
    write_cmd_data(0xFE); //F4 Low byte F5 INCREASES SYNC TIME AND BACK PORCH
    write_cmd_data(0x00);      // 00
    write_cmd_data(0x0C); //0C =12 The non-display period in lines between the start of the frame and the first display data in line.
//    write_cmd_data(0x00); //Set the vertical sync pulse width (LFRAME) in lines.
//    write_cmd_data(0x00);      //SET Vsync pulse start position
//    write_cmd_data(0x00);
    write_cmd_data(0x00);              // Set vert.Sync pulse start pos= 8    HB
    write_cmd_data(0x00);              // Set vert.Sync pulse start pos= 8    LB
    write_cmd_data(0x04);                   //

    write_command(0x36);
    write_cmd_data(0x01);

    // PWM signal frequency = PLL clock
    write_command(0xBE);
    write_cmd_data(0x08);
    write_cmd_data(0x80);
    write_cmd_data(0x01);

    write_command(0x0A);
    write_cmd_data(0x1C);         //Power Mode

    write_command(0xF0); //set pixel data format 8bit
    write_cmd_data(0x00);

    write_command(0x29);                    // Set display on

}





