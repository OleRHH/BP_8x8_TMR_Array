/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: lcd_functions.h                                                       */
/********************************************************************************/
#ifndef LCD_FUNCTIONS_H_
#define LCD_FUNCTIONS_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>

/************************  # public defines #   ***********************************/
#define MAX_ANALOG_VALUE ( 1 )
#define MAX_ARROW_LENGTH ( 2 )
#define SCALING ( 3 )
#define HARDW_AVG ( 4 )


/*******************  # typedefs, enums and structs #   *************************/
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

typedef struct {
    int16_t dSin[8][8];
    int16_t dCos[8][8];
    uint16_t arrowLength[8][8];
} lcdArrows;

// x-y-coordinate structure
// todo: this concept wasn't fully implemented
typedef struct
{
    int32_t x;
    int32_t y;
    bool arrowHeadActive;
} coordinates;

typedef struct
{
    bool relative;
    bool adcAVG;
    uint16_t maxArrowLength;
    bool reqSensorData;

    bool coloredArrows;
    color backColorArrow;
    color arrowColor;
    color gridColor;

    color backColorTable;
    color backColorMenu;
    color backColorArrowLengthMenu;
    color backColorImgStop;
    color backColorImgLeft;
    color backColorImgRight;
    color backColorMoreSettings;
    color spacerColor;
    color fontColor;
} Setup;


typedef struct
{
  uint16_t settingNo;
  Setup setup[4];
} Settings;


/**************************  # public Prototypes #   ****************************/
Setup * configureLCD(uint32_t, Settings *);
Setup * setLCDLayout(Settings *);
void drawSettingsMenu(void);
void drawDisplayLayout(void);

void drawDisplay(lcdArrows *);
void drawArrowLengthMenu(void);
void writeInfo(uint16_t);
void writeMaxAnalogValue(uint16_t);


#endif /* LCD_FUNCTIONS_H_ */
