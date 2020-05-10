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

typedef struct
{
    uint16_t x;
    uint16_t y;
} point;


/**************************  # Prototypes #   ****************************/

void ConfigureLCD5Inch(uint32_t);
void ConfigureLCD7Inch(uint32_t SysClock);

void print_string(char *, uint16_t, uint16_t, COLOR, COLOR);
void writeScreenColor5INCH(COLOR);
void write_screen_color7INCH(COLOR);
void drawDisplay5Inch(COLOR);
void drawDisplay7Inch(void);
void write_Infos(bool, bool, uint16_t, uint32_t);

void writeLine(short, short, short, short, COLOR, uint16_t);



#endif /* LCD_FUNCTIONS_H_ */
