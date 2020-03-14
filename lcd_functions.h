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


/*****************************  # defines #   *****************************/
#define GRID_OFFSET_X ( 120 )
#define GRID_OFFSET_Y ( 20 )
#define FONT_WIDTH_BIG 12
#define FONT_HIGHT_BIG 16
#define NO_ARROW        0
#define WITH_ARROW      1
#define ARROW_ANGLE     0.5236          // RAD = 30°
#define ARROW_LENGTH    5


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
void draw_display(void);
void draw_arrow(short, short, short, short, COLOR);

void write_command(unsigned char);
void write_cmd_data(unsigned char);
void write_data(COLOR);
void write_position(uint16_t, uint16_t, uint16_t, uint16_t);

void write_line(short, short, short, short, COLOR, uint16_t);

void write_line_0_degree  (short, short, short, short, COLOR);
void write_line_90_degree (short, short, short, short, COLOR);
void write_line_180_degree(short, short, short, short, COLOR);
void write_line_270_degree(short, short, short, short, COLOR);

void write_line_quadrant_1_I (short, short, short, short, double, COLOR);      //   0° < degree < 90°
void write_line_quadrant_1_II(short, short, short, short, double, COLOR);      //   0° < degree < 90°
void write_line_quadrant_2_I (short, short, short, short, double, COLOR);      //  90° < degree < 180°
void write_line_quadrant_2_II(short, short, short, short, double, COLOR);      //  90° < degree < 180°
void write_line_quadrant_3_I (short, short, short, short, double, COLOR);      // 180° < degree < 270°
void write_line_quadrant_3_II(short, short, short, short, double, COLOR);      // 180° < degree < 270°
void write_line_quadrant_4_I (short, short, short, short, double, COLOR);      // 270° < degree < 360°
void write_line_quadrant_4_II(short, short, short, short, double, COLOR);      // 270° < degree < 360°


#endif /* LCD_FUNCTIONS_H_ */
