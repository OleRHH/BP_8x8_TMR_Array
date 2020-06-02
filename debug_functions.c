/*
 * debug_functions.c
 *
 *  Created on: 28.05.2020
 *      Author: ole
 */

#include <debug_functions.h>

#include <tm4c1294ncpdt.h>
#include <driverlib/sysctl.h>           // SysCtlClockFreqSet
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <inc/hw_memmap.h>      // GPIO_PORTX_BASE

//#define OSZ_YEL     0b0000100           // Debug Output to oscilloscope
//#define OSZ_BLUE    0b0001000           // Debug Output to oscilloscope
//#define LED_D1 0b0000001                            // Debug Output to oscilloscope
//#define LED_D2 0b0000010                            // Debug Output to oscilloscope
//GPIO_PORTN_DATA_R ^= LED_D1;   // for debugging: toggle debug output each time handler is called
//GPIO_PORTN_DATA_R ^= LED_D2;  // for debugging: set high when handler is called


/*********************************************************************************************/
void toggleLED(uint16_t num)
{
    GPIO_PORTN_DATA_R ^= num;
}

/*********************************************************************************************/
void toggleOszi(uint16_t num)
{
    GPIO_PORTN_DATA_R ^= num << 2;
}


/*********************************************************************************************/
void onLED(uint16_t num)
{
    GPIO_PORTN_DATA_R |= num;
}


/*********************************************************************************************/
void offLED(uint16_t num)
{
    GPIO_PORTN_DATA_R &= ~num;
}


/*********************************************************************************************/
void onOszi(uint16_t num)
{
    GPIO_PORTN_DATA_R |= num << 2;
}


/*********************************************************************************************/
void offOszi(uint16_t num)
{
    GPIO_PORTN_DATA_R &= ~(num << 2);
}



/*********************************************************************************************/
void ConfigureDebugGPIO(void)
{

    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}
