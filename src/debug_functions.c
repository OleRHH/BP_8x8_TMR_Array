/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: debug_functions.c                                                     */
/********************************************************************************/
#include <debug_functions.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/sysctl.h>   // SysCtlClockFreqSet
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <inc/hw_memmap.h>      // GPIO_PORTX_BASE


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
void configureDebugGPIO(void)
{
    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}


/*********************************************************************************************/
/*********************************************************************************************/
// place into main.c
/*
#include <math.h>
#include <debug_functions.h>
#define A 200
#define _2PI ( (double) 6.28318530718 )
static int16_t bmCos[A], bmSin[A], a, m, n;

// main():

    configureDebugGPIO();
    for(a = 0; a < A; a++)
    {
        bmCos[a] = 60 * cos(_2PI * a/A);
        bmSin[a] = 60 * sin(_2PI * a/A);
    }
    toggleLED(1);
    onLED(2);
    a++;
    for(m = 0; m < 8; m++)
    {
        for(n = 0; n < 8; n++)
        {
            sensorData->arrows.dCos[m][n] = bmCos[a%A];
            sensorData->arrows.dSin[m][n] = bmSin[a%A];
        }
    }
    offLED(2);
*/

