/*
 * lcd_functions.h
 *
 *  Created on: 17.02.2020
 */

#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_

#define ANALOG_VALUE ( 1 )
#define MAX_ARROW_LENGTH ( 2 )
#define SCALING ( 3 )
#define HARDW_AVG ( 4 )
#define POS_DEBUG ( 5 )

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

typedef struct
{
    int16_t x;
    int16_t y;
} coordinates;
COLOR color[768];

/**************************  # public Prototypes #   **********************/
void ConfigureGPIO(void);
void configureLCD5Inch(uint32_t);
void configureLCD7Inch(uint32_t);
void setDisplayLayout(COLOR);

void drawDisplay5Inch(struct arrows *);
void drawDisplay7Inch(struct arrows *);
void drawArrowLengthMenu(void);
void setArrowWindowBackground(COLOR);
void writeInfo(uint16_t, void *);

void writeCommand(unsigned char);
void writePosition(uint16_t, uint16_t, uint16_t, uint16_t);

#endif /* LCD_FUNCTIONS_H_ */
