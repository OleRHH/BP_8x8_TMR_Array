uint16_t touchRead(unsigned char command)
{
    unsigned char i = 0x08; // 8 bit command
    uint16_t value = 0;

    GPIO_PORTF_AHB_DATA_R &= ~TCS; // CS=0

    while(i > 0)
    {
        if ((command >> 7) == 0)
        {
            GPIO_PORTF_AHB_DATA_R &= ~TDIN;
        } // out bit = 0
        else
        {
            GPIO_PORTF_AHB_DATA_R |= TDIN;
        } //out bit = 1
        command <<= 1; // next value
        GPIO_PORTF_AHB_DATA_R |= TCLK; // Clk=1
        SysCtlDelay(100);               // wait
        GPIO_PORTF_AHB_DATA_R &= ~TCLK; // Clk=0
        SysCtlDelay(100);               // wait
        i--;
    }

    i = 12; // 12 Bit ADC
    value = 0;

    while(i > 0)
    {
        value <<= 1;
        GPIO_PORTF_AHB_DATA_R |= TCLK; // Clk=1
        SysCtlDelay(100);               // wait
        GPIO_PORTF_AHB_DATA_R &= ~TCLK; // Clk=0
        SysCtlDelay(100);               // wait
        value |= ((GPIO_PORTF_AHB_DATA_R>>1)&0x01); // read value
        i--;
    }

    GPIO_PORTF_AHB_DATA_R |= TCS; // CS=1
    return value;
}
