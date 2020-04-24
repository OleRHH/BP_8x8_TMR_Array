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


/*****************************  # defines #   *****************************/
// defines for LCD init
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
#define ARROW_ANGLE     0.5236          // RAD = 30°
#define ARROW_LENGTH    5
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
enum
{
    BLACK   = 0x000000,
    RED     = 0x0000FF,
    GREEN   = 0x00FF00,
    YELLOW  = 0x00FFFF,
    BLUE    = 0xFF0000,
    WHITE   = 0xFFFFFF,
    GREY    = 0x505050
};



/**************************  # Prototypes #   ****************************/

void ConfigureLCD5Inch(uint32_t);
void ConfigureLCD7Inch(uint32_t SysClock);

void screen_show_nr(uint16_t);
void screen_write_nr(uint16_t);
void print_string(char *, uint16_t, uint16_t, COLOR, COLOR);
void write_screen_color5INCH(COLOR);
void write_screen_color7INCH(COLOR);
void drawDisplay5Inch(COLOR);
void drawDisplay7Inch(void);
void write_Infos(bool, bool, uint16_t, uint32_t);

void write_line(short, short, short, short, COLOR, uint16_t);
void enter_sleepmode(void);
void exit_sleepmode(void);



#endif /* LCD_FUNCTIONS_H_ */
