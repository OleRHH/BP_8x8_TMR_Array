#include <configuration.h>


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
void ConfigureUART(uint32_t SysClock)
{
    // Enable the GPIO Peripheral used by the UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_0|GPIO_PIN_1);


    UARTConfigSetExpClk(UART0_BASE, SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    // Enable the UART interrupt
    UARTFIFOEnable(UART0_BASE);
    IntPrioritySet(INT_UART0, LOW_PRIORITY);                // set priority
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    UARTTxIntModeSet(UART0_BASE, UART_TXINT_MODE_EOT);
    UARTIntRegister(UART0_BASE, UARTIntHandler);

    IntEnable(INT_UART0);
}


/*********************************************************************************************/
void ConfigureGPIO(void)
{
    // Set Port M Pins 0-7 Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port L  0-4 Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Onboard LEDs output: Port N (Pins 0-1)  debug outputs:Pin 2-3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // LCD Control output: Port Q (Pins 0-4)
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
// LCD Panel initialize:
void ConfigureLCD(uint32_t SysClock) {
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
    write_cmd_data(value >> 8);              // Stop row High
    write_cmd_data(value);                   // Stop row Low

    write_command(0x12);                    // enter partial mode

    write_command(0x33);                    // Set scroll area
    value = 0;
    write_cmd_data(value >> 8);              // TFA high byte (TFA = 0)
    write_cmd_data(value);                   // TFA low byte
    value = 544;
    write_cmd_data(value >> 8);              // VSA high byte (VSA = 272)
    write_cmd_data(value);                   // VSA low byte
    value = 0;
    write_cmd_data(value >> 8);              // BFA high byte (BFA = 543)
    write_cmd_data(value);                   // BFA low byte

    write_command(0x29);                    // Set display on
}








