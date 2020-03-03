/*
 * lcd_functions.h
 *
 *  Created on: 17.02.2020
 *      Author: ole
 */

#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_


/*****************************  # Includes #   ****************************/
#include <tm4c1294ncpdt.h>
#include <string.h>


/*****************************  # defines #   *****************************/


/**************************  # Prototypes #   ****************************/

// Typedefs:
// Typedefs:
typedef union color
{
    int all;
    struct
    {
        char red;
        char green;
        char blue;
    };
} COLOR;


void screen_show_nr(short);
void screen_write_nr(short);
void write_char(int, COLOR, COLOR);
void print_string(char *, int, int, COLOR, COLOR);
void write_screen_color(COLOR color);
void write_command(unsigned char data);
void write_cmd_data(unsigned char);
void write_data(COLOR);
void write_position(short, short, short, short);
//void write_line(short, short, short, short, COLOR);
//void erase_line(short, short, short, short);




#endif /* LCD_FUNCTIONS_H_ */
