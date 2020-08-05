#include <touch.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>      // needed for UART0_BASE

#define TDIN 0x01
#define TDO  0x02
#define TCS  0x04
#define TCLK 0x08
#define TIRQ 0x10



void configureTouch(void)
{
    // Set Port Q Pins 0-4: LCD Control output:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);  // Clock Port Q
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ))
        ;
    GPIOPinTypeGPIOOutput(
            GPIO_PORTQ_BASE,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);

    // Set Port F Pins 0-4 : Touch
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // Clock Port Q
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);

}


/****************************************************************************/
void touchWrite(unsigned char value)
{
    unsigned char i = 0x08; // 8 bit command
    unsigned char x, DI;

    GPIO_PORTF_AHB_DATA_R &= ~TCS; // CS=0

    while(i > 0)
    {
        DI = (value >> 7);
        if (DI == 0) {
            GPIO_PORTF_AHB_DATA_R &= ~TDIN;
        } // out bit=0
        else {
            GPIO_PORTF_AHB_DATA_R |= TDIN;
        } //out bit=1
        value <<= 1; // next value
        GPIO_PORTF_AHB_DATA_R |= TCLK; // Clk=1
        for(x=0;x<20;x++);
        GPIO_PORTF_AHB_DATA_R &= ~TCLK; // Clk=0
        for(x=0;x<20;x++);
        i--;
    }
}


/****************************************************************************/
unsigned int touchRead(void)
{
    unsigned char i = 12; // 12 Bit ADC
    unsigned int x, value = 0x00;

    while(i > 0)
    {
        value <<= 1;
        GPIO_PORTF_AHB_DATA_R |= TCLK; // Clk=1
        for(x=0;x<20;x++);
        GPIO_PORTF_AHB_DATA_R &= ~TCLK; // Clk=0
        for(x=0;x<20;x++);
        value |= ((GPIO_PORTF_AHB_DATA_R>>1)&0x01); // read value
        i--;
    }
    GPIO_PORTF_AHB_DATA_R |= TCS; // CS=1
    return value;
}
