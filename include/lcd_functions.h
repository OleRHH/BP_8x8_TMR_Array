/*
 * lcd_functions.h
 *
 *  Created on: 17.02.2020
 */

#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_


/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>


/*****************************  # typedefs #   *****************************/
typedef union color
{
    int32_t all;
    struct
    {
        char blue;
        char green;
        char red;
    };
} COLOR;

// enum colors
enum colors
{
    BLACK   = 0x00000000,
    RED     = 0x00FF0000,
    GREEN   = 0x0000FF00,
    YELLOW  = 0x00FFFF00,
    BLUE    = 0x000000FF,
    WHITE   = 0x00FFFFFF,
    GREY    = 0x00AAAAAA,
};

enum CommandFromTouch
{
    noNewCommand = 0,
    enterSettings = 1,
    newCommandForMotor = 2,
};

struct arrows {
    int16_t dSin[8][8];
    int16_t dCos[8][8];
    uint16_t arrowLength[8][8];
};

// x-y-coordinate structure
// todo: this concept wasn't fully implemented
typedef struct
{
    uint16_t x;
    uint16_t y;
} point;

COLOR color[768];

/**************************  # public Prototypes #   **********************/
void ConfigureGPIO(void);
void configureLCD5Inch(uint32_t, COLOR);
void configureLCD7Inch(uint32_t, COLOR);

void setLCDBackgroundColor(COLOR);
void setLCDBackgroundColor7(COLOR);
void drawDisplay5Inch(struct arrows *);
void drawDisplay7Inch(struct arrows *);

void writeInfos(bool, bool, uint16_t, uint16_t, uint16_t, uint16_t);
enum CommandFromTouch readTouchscreen(char *);

void writeCommand(unsigned char);
void writePosition(uint16_t, uint16_t, uint16_t, uint16_t);

#endif /* LCD_FUNCTIONS_H_ */
