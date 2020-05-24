#include <touch.h>

/**********************  # intern Prototypes #   **********************/
void touch_write(unsigned char value);
unsigned int touch_read(void);


void touch_write(unsigned char value)
{
    unsigned char i = 0x08; // 8 bit command
    unsigned char x, DI;

    GPIO_PORTF_AHB_DATA_R &= 0xFB; //CS=0

    while (i > 0)
    {
        DI = (value >> 7);
        if (DI == 0)
        {
            GPIO_PORTF_AHB_DATA_R &= 0xFE;
        } //out bit=0
        else
        {
            GPIO_PORTF_AHB_DATA_R |= 0x01;
        } //out bit=1
        value <<= 1; //next value
        GPIO_PORTF_AHB_DATA_R |= 0x08; //Clk=1
        for (x = 0; x < 20; x++)
            ;
        GPIO_PORTF_AHB_DATA_R &= 0xF7; //Clk=0
        for (x = 0; x < 20; x++)
            ;
        i--;
    }
}

unsigned int touch_read()
{
    unsigned char i = 12; // 12 Bit ADC
    unsigned int x, value = 0x00;

    while (i > 0)
    {
        value <<= 1;
        GPIO_PORTF_AHB_DATA_R |= 0x08; //Clk=1
        for (x = 0; x < 20; x++)
            ;
        GPIO_PORTF_AHB_DATA_R &= 0xF7; //Clk=0
        for (x = 0; x < 20; x++)
            ;
        value |= ((GPIO_PORTF_AHB_DATA_R >> 1) & 0x01); // read value
        i--;
    }
    GPIO_PORTF_AHB_DATA_R |= 0x04; //CS=1
    return value;
}

void read_coordinate(int *xpos, int *ypos)
{
    int x;
    GPIO_PORTF_AHB_DATA_R &= 0xF7; //Clk=0
    touch_write(0xD0); //Touch Command XPos read
    for (x = 0; x < 10; x++)
        ; //Busy wait
    *xpos = touch_read(); //xpos value read ( 0......4095 )
    //printf("xpox= %5d ", *xpos);
    touch_write(0x90); //Touch Command YPos read
    for (x = 0; x < 10; x++)
        ; //Busy wait
    *ypos = touch_read(); //ypos value read ( 0.....4095 )
    //printf("ypox= %5d\n", *ypos);
}

// arrows = 1, display = 2, stepper-motor = 3
int read_button(int xpos, int ypos)
{
    bool x, y;
    x = (xpos > 3500 && xpos < 3700);
    y = (ypos > 2900 && ypos < 3500);
    if (x && y)
    {
        return 1; // arrows

    }

    x = (xpos > 3500 && xpos < 3700);
    y = (ypos > 1700 && ypos < 2300);
    if (x && y)
    {
        return 2; // display

    }

    x = (xpos > 3500 && xpos < 3700);
    y = (ypos > 700 && ypos < 1300);
    if (x && y)
    {
        return 3; // stepper-motor

    }

    return 0;

}
