/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: touch_functions.c                                                     */
/********************************************************************************/
#include <touch_functions.h>
#include <tm4c1294ncpdt.h>      // GPIO_PORTF_AHB_DATA_R
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>      // needed for UART0_BASE
#include <driverlib/interrupt.h>
#include <math.h>


/*****************************  # defines #   ***********************************/
// define touch pin names
#define TDIN GPIO_PIN_0         // 0b0 0001
#define TDO  GPIO_PIN_1         // 0b0 0010
#define TCS  GPIO_PIN_2         // 0b0 0100
#define TCLK GPIO_PIN_3         // 0b0 1000
#define TIRQ GPIO_PIN_4         // 0b1 0000

// these defines represent the return value from touch in the middle of the buttons
#define LEFT_X ( 1125 )
#define LEFT_Y ( 3455 )

#define RIGHT_X  ( 435 )
#define RIGHT_Y  ( 3455 )

#define STOP_X  ( 1125 )
#define STOP_Y  ( 2460 )

#define CHANGE_SETTING_X ( 435 )
#define CHANGE_SETTING_Y ( 2460 )

#define A_LENGTGH_X ( 380 )       // button 'max arrow length' 10 - 200?
#define A_LENGTGH_Y ( 1175 )

#define SCALING_X ( 380 )       // button 'scaling' relative/absolute
#define SCALING_Y ( 780 )

#define H_AVG_X ( 380 )         // button 'hardware averaging' on/off
#define H_AVG_Y ( 410 )


/*****************************  # intern prototypes #   *************************/
uint16_t touchRead(unsigned char);


/***********************  touchInterruptClear()  ********************************/
/*                                                                              */
/********************************************************************************/
void touchInterruptClear(void)
{
    GPIOIntClear(GPIO_PORTF_AHB_BASE, GPIO_PIN_4);
}


/***********************  touchGetArrowLength()  ********************************/
/* When the 'max arrow length' option is activated via touch input, a window is */
/* generated with 4x4 (row x column) fields to choose from. Possible values are */
/* 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 and 160.    */
/* They define the maximum arrow length in pixels and also represent the actual */
/* measured analog value from the MCU internal 12 bit AD-converter.             */
/* There are two display modes available: relative and absolute.                */
/* In relative mode the highest value from the last analog sensor array measure-*/
/* ment (64 sensors) gets scaled to the here defined pixel length and all       */
/* other arrows get scaled from 0 to this value.                                */
/* In absolute mode the arrows are not scaled but can only be as long as        */
/* defined here. If there are actual larger measured valued the word "clipping" */
/* apears on the display as an information.                                     */
/* This function returns the value that was picked from the window.             */
/* To stabilize the x,y coordinates the analog values get averaged while        */
/* pressing the touch display. Only if more than 200 measurement where taken    */
/* (this only takes some ms) the touch input is valid.                          */
/********************************************************************************/
uint16_t touchGetArrowLength(uint16_t oldValue)
{
    uint32_t xposSum = 0, yposSum = 0;
    uint16_t pos = 0, m = 0, n = 0;     // m = row , n = column

    SysCtlDelay(1000);

    // menu can only be left with a valid touch. A touch is only valid
    // if pos gets summed up to >= 200
    while( pos <= 200)
    {
        pos = 0;
        // sum up measurements. break after 1000 loops or if touch is released.
        while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) && pos < 1000)
        {
            xposSum += touchRead(0xD0);
            yposSum += touchRead(0x90);
            SysCtlDelay(100);               // wait
            pos++;
        }
    }

    // calculate the mean values.
    xposSum /= pos;
    yposSum /= pos;

    // first check if window was hit at all. Else return old value.
    // upper left corner: x = 4095, y = 4095
    if(xposSum > 3725 || xposSum < 1760 || yposSum > 3600 || yposSum < 490)
        return oldValue;
    // now check what column (n) was hit
    else if(xposSum > 3240)
        n = 0;
    else if(xposSum > 2740)
        n = 1;
    else if(xposSum > 2255)
        n = 2;
    else if(xposSum > 1760)
        n = 3;

    // now check what row (m) was hit
    if(yposSum > 2830)
        m = 0;
    else if(yposSum > 2040)
        m = 1;
    else if(yposSum > 1230)
        m = 2;
    else if(yposSum > 490)
        m = 3;

    // each row and column has its own values. The actual value is returned.
    return 10 + m*40 + n*10;    // 10 - 160
}


/************************  touchGetSettingNum()  ********************************/
/* See touchGetArrowLength()                                                    */
/********************************************************************************/
uint16_t touchGetSettingNum(uint16_t oldValue)
{
    uint32_t xposSum = 0, yposSum = 0;
    uint16_t pos = 0;

    SysCtlDelay(1000);

    // menu can only be left with a valid touch
    while( pos <= 200)
    {
        pos = 0;
        // sum up measurements. break after 1000 loops or if touch is released.
        while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) && pos < 1000)
        {
            xposSum += touchRead(0xD0);
            yposSum += touchRead(0x90);
            SysCtlDelay(100);               // wait
            pos++;
        }
    }

    xposSum /= pos;
    yposSum /= pos;

    // first check if window was hit at all. Else return old value.
    // upper left corner: x = 4095, y = 4095
    if(xposSum > 3725 || xposSum < 1760 || yposSum > 3600 || yposSum < 2040)
        return oldValue;

    // now check what field was hit
    else if(xposSum > 3240)
        return 0;
    else if(xposSum > 2740)
        return 1;
    else if(xposSum > 2255)
        return 2;
    else
        return 3;
}


/**************************  touchGetMenuItem()  ********************************/
/* See touchGetArrowLength()                                                    */
/********************************************************************************/
uint16_t touchGetMenuItem(void)
{
    uint32_t xposSum = 0, yposSum = 0;
    uint16_t pos = 0;

    SysCtlDelay(1000);
    // sum up to 1000 values or until touch screen released
    while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) && pos < 1000)
    {
        xposSum += touchRead(0xD0);
        yposSum += touchRead(0x90);
        SysCtlDelay(100);               // wait
        pos++;
    }
    // if more than 200 values have been measured the result is valid
    // and gets processed. If not the touch is ignored.
    if(pos > 200)
    {
        xposSum /= pos;
        yposSum /= pos;

        if(abs(LEFT_X - xposSum) < 250 && abs(LEFT_Y - yposSum) < 440)
            pos = LEFT_BUTTON;

        else if(abs(RIGHT_X - xposSum) < 250 && abs(RIGHT_Y - yposSum) < 440)
            pos = RIGHT_BUTTON;

        else if(abs(STOP_X - xposSum) < 250 && abs(STOP_Y - yposSum) < 440)
            pos = STOP_BUTTON;

        else if(abs(CHANGE_SETTING_X - xposSum) < 250 && abs(CHANGE_SETTING_Y - yposSum) < 440)
            pos = CHANGE_SETTING_BUTTON;

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
    while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) );
    SysCtlDelay(1200000);
    return pos;
}


/****************************  touchRead()  *************************************/
/* The Touch-Controller has a synchronous serial interface.                     */
/* This code was taken from Mr. A. Prosch and modified.                         */
/********************************************************************************/
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


/**************************  configureTouch()  **********************************/
/*                                                                              */
/********************************************************************************/
void configureTouch(void)
{
    // Set Port F Pins 0-4 : Touch
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // Clock Port Q
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOInput (GPIO_PORTF_BASE, TDO  | TIRQ);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, TDIN | TCS | TCLK);

    // the touch screen controller sets its output 'IRQ' pin to 0 when it detects
    // a sensor touch. This triggers an interrupt.
    GPIOIntTypeSet(GPIO_PORTF_AHB_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);

    GPIOIntRegister(GPIO_PORTF_AHB_BASE, touchInterruptHandler);
    GPIOIntEnable(GPIO_PORTF_AHB_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOF);
}
