#include <touch.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>      // needed for UART0_BASE
#include <driverlib/interrupt.h>
#include <math.h>

// define touch pin names
#define TDIN GPIO_PIN_0         // 0b0 0001
#define TDO  GPIO_PIN_1         // 0b0 0010
#define TCS  GPIO_PIN_2         // 0b0 0100
#define TCLK GPIO_PIN_3         // 0b0 1000
#define TIRQ GPIO_PIN_4         // 0b1 0000

// these defines represent the return value from touch in the middle the buttons
#define LEFT_X ( 1125 )
#define LEFT_Y ( 3455 )

#define RIGHT_X  ( 435 )
#define RIGHT_Y  ( 3455 )

#define STOP_X  ( 1125 )
#define STOP_Y  ( 2460 )

#define START_X ( 435 )
#define START_Y ( 2460 )

#define A_LENGTGH_X ( 380 )       // button 'max arrow length' 10 - 200?
#define A_LENGTGH_Y ( 1175 )

#define SCALING_X ( 380 )       // button 'scaling' relative/absolute
#define SCALING_Y ( 780 )

#define H_AVG_X ( 380 )         // button 'hardware averaging' on/off
#define H_AVG_Y ( 410 )

uint16_t touchRead(unsigned char);


/****************************************************************************/
void configureTouch(void)
{
    // Set Port F Pins 0-4 : Touch
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // Clock Port Q
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOInput (GPIO_PORTF_BASE, TDO  | TIRQ);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, TDIN | TCS | TCLK);

    GPIOIntTypeSet(GPIO_PORTF_AHB_BASE, GPIO_PIN_4,
                   GPIO_FALLING_EDGE);

    // the touch screen controller pulls down its IRQ pin when it detects a
    // touch.
    GPIOIntRegister(GPIO_PORTF_AHB_BASE, TouchInterruptHandler);
    GPIOIntEnable(GPIO_PORTF_AHB_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOF);
}


/****************************************************************************/
uint16_t touchGetItem(uint16_t *xpos, uint16_t *ypos)
{
    uint32_t xposSum = 0, yposSum = 0;
    uint16_t pos = 0;

    SysCtlDelay(1000);

    while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) && pos < 1000)
    {
        xposSum += touchRead(0xD0);
        yposSum += touchRead(0x90);
        SysCtlDelay(100);               // wait
        pos++;
    }
    if(pos > 200)
    {
        xposSum /= pos;
        yposSum /= pos;
        *xpos = xposSum; // read xpos
        *ypos = yposSum; // read ypos

        if(abs(LEFT_X - xposSum) < 250 && abs(LEFT_Y - yposSum) < 440)
            pos = LEFT_BUTTON;

        else if(abs(RIGHT_X - xposSum) < 250 && abs(RIGHT_Y - yposSum) < 440)
            pos = RIGHT_BUTTON;

        else if(abs(STOP_X - xposSum) < 250 && abs(STOP_Y - yposSum) < 440)
            pos = STOP_BUTTON;

        else if(abs(START_X - xposSum) < 250 && abs(START_Y - yposSum) < 440)
            pos = START_BUTTON;

    // settings: max arrow length, scaling relative/absolute, hardware avg on/off
        else if(yposSum < 1410 && xposSum < 670)
        {
            if(yposSum > 1000 )
                pos = ARROW_LENGTH_BUTTON;

            else if(yposSum > 595 )
                pos = SCALING_BUTTON;

            else if(xposSum > 0)
                pos = HARDW_AVG_BUTTON;
        }
    }
    while(GPIOPinRead(GPIO_PORTF_AHB_BASE, GPIO_PIN_4) != GPIO_PIN_4);
    SysCtlDelay(1200000);
    return pos;
}


/****************************************************************************/
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
        } // out bit=0
        else
        {
            GPIO_PORTF_AHB_DATA_R |= TDIN;
        } //out bit=1
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

