/*
 * lcd_functions.h
 *
 *  Created on: 17.02.2020
 */

#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_


/*****************************  # Includes #   ****************************/
#include <tm4c1294ncpdt.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <adc_functions.h>
#include <fonts.h>

// gpio configure
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <inc/hw_memmap.h>      // GPIO_PORTX_BASE

/*****************************  # defines #   *****************************/
// constants for LCD
#define RST 0x10
#define INITIAL_STATE (0x1F)
#define SOFTWARE_RESET (0x01)
#define SET_PLL_MN (0xE2)
#define START_PLL (0xE0)
#define SET_LSHIFT (0xE6)
#define SET_LCD_MODE (0xB0)
#define SET_HORI_PERIOD (0xB4)
#define SET_VERT_PERIOD (0xB6)
#define SET_ADRESS_MODE (0x36)
#define SET_PIXEL_DATA_FORMAT (0xF0)
#define SET_DISPLAY_ON (0x29)

#define FONT_WIDTH_BIG 12
#define FONT_HIGHT_BIG 16
#define NO_ARROW        0
#define WITH_ARROW      1
#define ARROW_ANGLE     0.7
#define ARROW_LENGTH    5
#define MIN_LENGTH_FOR_ARROW  1
#define GRID_OFFSET_X_5_INCH ( 30 )
#define GRID_OFFSET_Y_5_INCH ( 20 )
#define GRID_OFFSET_X_7_INCH ( 200 )
#define GRID_OFFSET_Y_7_INCH ( 50 )


/*****************************  # typedefs #   *****************************/
typedef union color
{
    int32_t all;
    struct
    {
        char red;
        char green;
        char blue;
    };
} COLOR;

// enum colors
enum colors
{
    BLACK   = 0x000000,
    RED     = 0x0000FF,
    GREEN   = 0x00FF00,
    YELLOW  = 0x00FFFF,
    BLUE    = 0xFF0000,
    WHITE   = 0xFFFFFF,
    GREY    = 0x505050
};

// x-y-coordinate structure
// todo: this concept wasn't fully implemented
typedef struct
{
    uint16_t x;
    uint16_t y;
} point;


/**************************  # public Prototypes #   **********************/
void ConfigureLCD5Inch(uint32_t);
void ConfigureLCD7Inch(uint32_t);
void ConfigureGPIO(void);

void printString(char *, uint16_t, uint16_t, COLOR, COLOR);
void writeScreenColor5INCH(COLOR);
void writeScreenColor7INCH(COLOR);
void drawDisplay5Inch(COLOR);
void drawDisplay7Inch(void);
void writeRecangle(void);
void writeInfos(bool, bool, uint16_t, uint32_t, COLOR);


#endif /* LCD_FUNCTIONS_H_ */
