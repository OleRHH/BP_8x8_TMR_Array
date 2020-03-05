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
#define GRID_OFFSET_X ( 100 )
#define GRID_OFFSET_Y ( 20 )
#define FONT_WIDTH_BIG 12
#define FONT_HIGHT_BIG 16


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


/**************************  # Prototypes #   ****************************/
void screen_show_nr(uint16_t);
void screen_write_nr(uint16_t);
void write_char(uint16_t, COLOR, COLOR);
void print_string(char *, uint16_t, uint16_t, COLOR, COLOR);
void write_screen_color(COLOR);
void draw_grid(void);
void draw_arrows(void);

void write_command(unsigned char);
void write_cmd_data(unsigned char);
void write_data(COLOR);
void write_position(uint16_t, uint16_t, uint16_t, uint16_t);
void write_line(short start_x, short start_y, short stop_x, short stop_y, COLOR color);

//void erase_line(short, short, short, short);


#endif /* LCD_FUNCTIONS_H_ */
