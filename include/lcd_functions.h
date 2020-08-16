/*
 * lcd_functions.h
 *
 *  Created on: 17.02.2020
 */

#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_

#define MAX_ANALOG_VALUE ( 1 )
#define MAX_ARROW_LENGTH ( 2 )
#define SCALING ( 3 )
#define HARDW_AVG ( 4 )
#define POS_DEBUG ( 5 )

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>


/*******************  # typedefs, enums and structs #   *******************/
typedef union Color
{
    int32_t all;
    struct
    {
        char blue;
        char green;
        char red;
    };
} color;

// some predefined basic colors
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
    int16_t x;
    int16_t y;
} coordinates;

typedef struct
{
    bool relative;
    bool adcAVG;
    uint16_t maxArrowLength;

    bool coloredArrows;
    color backColorArrowWindow;
    color arrowColor;
    color gridColor;

    color backColorTable;
    color backColorMotor;
    color backColorArrowLengthMenu;
    color backColorImgStart;
    color backColorImgStop;
    color backColorImgLeft;
    color backColorImgRight;
    color spacerColor;
    color fontColor;
} Setup;


typedef struct
{
  uint16_t setupNo;
  Setup setup[4];
} Settings;

/**************************  # public Prototypes #   **********************/
void configureLCD5Inch(uint32_t);
Setup * configureLCD(uint32_t, Settings *);
void configureLCDHardware(uint32_t);
Setup * setLCDLayout(uint16_t, Settings *);
void drawDisplayLayout(void);

void drawDisplay7Inch(struct arrows *);
void drawArrowLengthMenu(void);
void writeInfo(uint16_t);
void writeMaxAnalogValue(uint16_t);

bool getAdcAVG(void);
bool getScaling(void);
uint16_t getmaxArrow(void);

#endif /* LCD_FUNCTIONS_H_ */
