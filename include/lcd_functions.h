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
    GREY    = 0x505050,
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

void writeInfos(bool, bool, uint16_t, uint16_t);
enum CommandFromTouch readTouchscreen(char *);

void writeCommand(unsigned char);
void writePosition(uint16_t, uint16_t, uint16_t, uint16_t);

#endif /* LCD_FUNCTIONS_H_ */
