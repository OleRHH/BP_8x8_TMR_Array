/*****************************  # Includes #   **********************************/
#include <lcd_functions.h>
#include <tm4c1294ncpdt.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <inc/hw_memmap.h>      // GPIO_PORTX_BASE
#include <fonts.h>


/*****************************  # defines #   ***********************************/
// constants for LCD initializiation
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

// constants for writing text to display
#define FONT_WIDTH_BIG 12
#define FONT_HIGHT_BIG 16

// constants for arrow output on display
#define NO_ARROW        0
#define WITH_ARROW      1
#define ARROW_ANGLE     0.7
#define ARROW_LENGTH    5
#define MIN_LENGTH_FOR_ARROW  1
#define GRID_OFFSET_X_7_INCH ( 50 )
#define GRID_OFFSET_Y_7_INCH ( 35 )

// constants for the touch button layout
#define DIMENSIONS  ( 96 )
#define PIXEL_COUNT ( DIMENSIONS * DIMENSIONS )
#define OFFSET_IMG  ( DIMENSIONS - 1 )
#define EDGE ( 9 )


/*****************************  # intern prototypes #   *************************/
void generateColors(void);  // create the color codes for colored arrows
void setLCDBackgroundColor7(color);
void writeCommand(unsigned char);
void writeCmdData(unsigned char);
void writeData(color);

void writePosition(uint16_t, uint16_t, uint16_t, uint16_t);
void writeButton(unsigned char *, short, short, color);

void drawLine(short, short, short, short, color, uint16_t);
void writeLine0Degree  (short, short, short, short, color);
void writeLine90Degree (short, short, short, short, color);
void writeLine180Degree(short, short, short, short, color);
void writeLine270Degree(short, short, short, short, color);
void writeLineQuadrant1_I (short, short, short, short, double, color);  //   0° < degree < 90°
void writeLineQuadrant1_II(short, short, short, short, double, color);  //   0° < degree < 90°
void writeLineQuadrant2_I (short, short, short, short, double, color);  //  90° < degree < 180°
void writeLineQuadrant2_II(short, short, short, short, double, color);  //  90° < degree < 180°
void writeLineQuadrant3_I (short, short, short, short, double, color);  // 180° < degree < 270°
void writeLineQuadrant3_II(short, short, short, short, double, color);  // 180° < degree < 270°
void writeLineQuadrant4_I (short, short, short, short, double, color);  // 270° < degree < 360°
void writeLineQuadrant4_II(short, short, short, short, double, color);  // 270° < degree < 360°

void drawRectangle(short, short, short, short, color);
void drawString(char *, uint16_t, uint16_t, color, color);


/*****************************  # global variables #    *************************/
int16_t oldDiffSinResults[8][8];
int16_t oldDiffCosResults[8][8];
bool oldArrowActive[8][8];
coordinates startCord[8][8];

color arrowColor[768];
Setup * set;

/***********************  # extern global variables #    ************************/
extern unsigned char imgArrayStopButton[PIXEL_COUNT];
extern unsigned char imgArrayStartButton[PIXEL_COUNT];
extern unsigned char imgArrayArrowLButton[PIXEL_COUNT];
extern unsigned char imgArrayArrowRButton[PIXEL_COUNT];


/**************************  configureLCD()  ************************************/
//       //
/********************************************************************************/
Setup * configureLCD(uint32_t SysClock, Settings * setting)
{
    uint16_t m, n;

    // configure MCU hardware pins connected to LCD and setup LCD parameter
    configureLCDHardware(SysClock);

    // generate an array with colors used for option 'colored arrows'
    generateColors();

    // set is a pointer to the active settings used in this file and in main.
    // there is an array of settings 'setup[4]'. The following command
    // changes the address where 'set' points to
    set = &setting->setup[setting->settingNo];


    // calculate the grid position in advanced. They are later used to
    // draw the arrows.
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            // I: set new start coordinates
            startCord[m][n].x = n * 58 + GRID_OFFSET_X_7_INCH;
            startCord[m][n].y = m * 60 + GRID_OFFSET_Y_7_INCH;
        }
    }

    // now draw the hole display
    drawDisplayLayout();

    return set;
}


/****************************  setLCDLayout()  **********************************/
//       //
/********************************************************************************/
Setup * setLCDLayout(Settings * setting)
{
    set = &setting->setup[setting->settingNo];
    return set;
}


/*************************  drawDisplayLayout()  ********************************/
//       //
/********************************************************************************/
void drawDisplayLayout(void)
{
    drawRectangle(0, 0, 506, 479, set->backColorArrowWindow); // arrow background

    drawRectangle(510, 0, 799, 274, set->backColorMotor);  // background motor control
    drawRectangle(510, 278, 799, 479, set->backColorTable);    // background infos

    drawRectangle(507,   0, 510, 479, set->spacerColor);   // spacerColor
    drawRectangle(680, 278, 682, 479, set->spacerColor); // little spacerColor
    drawRectangle(510, 275, 799, 278, set->spacerColor);
    drawRectangle(510, 330, 799, 333, set->spacerColor);
    drawRectangle(510, 385, 799, 388, set->spacerColor);
    drawRectangle(510, 440, 799, 443, set->spacerColor);

    // 1.
    writeButton(imgArrayArrowLButton, 540, 20, set->backColorImgLeft);
    // 2.
    writeButton(imgArrayArrowRButton, 680, 20, set->backColorImgRight);
    // 3.
    writeButton(imgArrayStopButton, 540, 150, set->backColorImgStop);
    // 4.
    writeButton(imgArrayStartButton, 680, 150, set->backColorImgStart);

    drawString("max measured", 285, 520, set->fontColor, set->backColorTable);
    drawString("analog value", 305, 520, set->fontColor, set->backColorTable);

    drawString("max arrow", 340, 520, set->fontColor, set->backColorTable);
    drawString("length", 360, 520, set->fontColor, set->backColorTable);

    drawString("scaling", 410, 520, set->fontColor, set->backColorTable);

    drawString("hardware avg", 450, 520, set->fontColor, set->backColorTable);

    writeInfo(MAX_ARROW_LENGTH | SCALING | HARDW_AVG);
}


/***************************  drawArrowLengthMenu()  ****************************/
// draws the 'Change max. arrow size' menu: background, lines and words.        //
/********************************************************************************/
void drawSettingsMenu(void)
{
    uint16_t n, value = 1;
    char charValue[10];

    // set menu background
    drawRectangle(50, 50, 456, 150, set->backColorArrowLengthMenu);

    // draw the vertical spacer lines
    drawRectangle( 48, 48,  49, 150, set->spacerColor);   // spacer
    drawRectangle(150, 50, 151, 150, set->spacerColor);   // spacer
    drawRectangle(252, 50, 253, 150, set->spacerColor);   // spacer
    drawRectangle(354, 50, 355, 150, set->spacerColor);   // spacer
    drawRectangle(456, 48, 457, 150, set->spacerColor);   // spacer

    // draw the horizontal spacer lines
    drawRectangle(50,  48, 456,  49, set->spacerColor);   // spacer
    drawRectangle(50, 150, 456, 151, set->spacerColor);   // spacer

    // print available number choices
    for(n = 0; n < 4; n++)
    {
        sprintf(charValue, "%.1d", value);
        drawString(charValue, 95, 90 + 100 * n,
                   set->fontColor, set->backColorArrowLengthMenu);
        value++;
    }
}


/***************************  drawArrowLengthMenu()  ****************************/
// draws the 'Change settings' menu.                                            //
/********************************************************************************/
void drawArrowLengthMenu(void)
{
    uint16_t m, n, value = 10;
    char charValue[10];

    // set menu background
    drawRectangle(50, 50, 456, 456, set->backColorArrowLengthMenu);

    // draw the vertical spacer lines
    drawRectangle( 48, 48,  49, 458, set->spacerColor);   // spacer
    drawRectangle(150, 50, 151, 458, set->spacerColor);   // spacer
    drawRectangle(252, 50, 253, 458, set->spacerColor);   // spacer
    drawRectangle(354, 50, 355, 458, set->spacerColor);   // spacer
    drawRectangle(456, 48, 457, 458, set->spacerColor);   // spacer

    // draw the horizontal spacer lines
    drawRectangle(50,  48, 456,  49, set->spacerColor);   // spacer
    drawRectangle(50, 150, 456, 151, set->spacerColor);   // spacer
    drawRectangle(50, 252, 456, 253, set->spacerColor);   // spacer
    drawRectangle(50, 354, 456, 355, set->spacerColor);   // spacer
    drawRectangle(50, 457, 456, 458, set->spacerColor);   // spacer

    // print available number choices
    for(m = 0; m < 4; m++)
    {
        for(n = 0; n < 4; n++)
        {
            sprintf(charValue, "%.2d", value);
            drawString(charValue, 95 + 100 * m, 90 + 100 * n,
                       set->fontColor, set->backColorArrowLengthMenu);
            value += 10;
        }
    }
}


/******************************  writeButton()  *********************************/
// Helper function for setDisplayLayout().                                      //
/********************************************************************************/
void writeButton(unsigned char *imgArray, short offsetX, short offsetY, color color)
{
    int pixel;
    short stopX, stopY;

    stopX = offsetX + OFFSET_IMG;
    stopY = offsetY + OFFSET_IMG;

    drawRectangle(offsetX - EDGE, offsetY - EDGE, EDGE + stopX, EDGE + stopY, color);

// draws a cross for touch calibration
#ifdef DEBUG
    drawLine( offsetX + OFFSET_IMG/2, offsetY - EDGE, offsetX + OFFSET_IMG/2, stopY + EDGE, (color)BLACK, NO_ARROW);
    drawLine( offsetX - EDGE, offsetY + OFFSET_IMG/2, stopX + EDGE, offsetY + OFFSET_IMG/2, (color)BLACK, NO_ARROW);
#endif

    writePosition(offsetX, offsetY, stopX, stopY);
    writeCommand(0x2C);                        // start writing

    for (pixel = 0; pixel < PIXEL_COUNT; pixel++)
    {
        if(imgArray[pixel] > 100)
        {
            GPIO_PORTM_DATA_R = color.red;
            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state

            GPIO_PORTM_DATA_R = color.green;
            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state

            GPIO_PORTM_DATA_R = color.blue;
            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        }
        else
        {
            GPIO_PORTM_DATA_R = 0x00;

            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state

            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state

            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        }
    }
}


/***************************  writeInfo()   *************************************/
// writes some info as text on the display.                                     //
// Infos are: absolute or relative arrow mode, maximum measured analog,         //
// arrow max length.                                                            //
/********************************************************************************/
void writeInfo(uint16_t info)
{
    char charValue[20];

    if( (info & MAX_ANALOG_VALUE) == MAX_ANALOG_VALUE)
    {
        // TODO: implement here. Currently in separate function.
    }
    if( (info & MAX_ARROW_LENGTH) == MAX_ARROW_LENGTH)
    {
        sprintf(charValue, "%.3d", set->maxArrowLength);
        drawString(charValue, 350, 720, set->fontColor, set->backColorTable);
    }
    if( (info & SCALING) == SCALING)
    {
        (set->relative) ?  sprintf(charValue, "relative") : sprintf(charValue, "absolute");
        drawString(charValue, 410, 690, set->fontColor, set->backColorTable);
    }
    if( (info & HARDW_AVG) == HARDW_AVG)
    {
        (set->adcAVG) ?  sprintf(charValue, " on") : sprintf(charValue, "off");
        drawString(charValue, 450, 720, set->fontColor, set->backColorTable);
    }
    if( (info & POS_DEBUG) == POS_DEBUG)
    {
//        sprintf(charValue, "x: %.4d", (*(uint32_t *)value) >> 16);
//        drawString(charValue, 290, 700, (color)BLACK, backColorTable);
//        sprintf(charValue, "y: %.4d", (*(uint16_t *)value));
//        drawString(charValue, 310, 700, (color)BLACK, backColorTable);
    }
}


/************************  writeMaxAnalogValue()   ******************************/
// arrow max length.                                                            //
/********************************************************************************/
void writeMaxAnalogValue(uint16_t maxAnalogValue)
{
    char charValue[20];

    sprintf(charValue, "%.3d", maxAnalogValue);
    drawString(charValue, 295, 720, set->fontColor, set->backColorTable);
}


/****************************  drawString()   **********************************/
// writes a string to the given position on the LC-Display                      //
/********************************************************************************/
void drawString(char *text, uint16_t rowStart, uint16_t columnStart, color fontColor, color backcolor)
{
    uint16_t letter, numLetter, byteNum, lv;
    uint16_t columnStop = columnStart + FONT_WIDTH_BIG - 1;
    uint16_t rowStop = rowStart + FONT_HIGHT_BIG - 1;

    // write all characters from given string
    for (numLetter = 0; numLetter < strlen(text); numLetter++)
    {
        writePosition(columnStart, rowStart, columnStop, rowStop);
        writeCommand(0x2C);

        // Write the 16 rows with 2 bytes each (lv: 0,2,4,..,30,32)
        for (byteNum = 0; byteNum < 32; byteNum += 2)
        {
            // letter = 16 bit = highByte<<4 OR lowByte>>4
            letter = (font_12_16[text[numLetter]][byteNum + 1] << 4)
                    | (font_12_16[text[numLetter]][byteNum] >> 4);

            for (lv = 0; lv < 12; lv++) {
                if (letter & 1) {             // Write data byte
                    GPIO_PORTM_DATA_R = fontColor.red;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                    GPIO_PORTM_DATA_R = fontColor.green;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                    GPIO_PORTM_DATA_R = fontColor.blue;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                } else {
                    GPIO_PORTM_DATA_R = backcolor.red;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                    GPIO_PORTM_DATA_R = backcolor.green;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                    GPIO_PORTM_DATA_R = backcolor.blue;
                    GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
                    GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
                }
                letter >>= 1;
            }
        }
        columnStart += FONT_WIDTH_BIG;
        columnStop  += FONT_WIDTH_BIG;
    }
}


//setup->coloredArrows = setup->coloredArrows ? false:true;
/**************************  drawDisplay7Inch()   *******************************/
// draws all arrows to the 7 inch LC-Display.                                   //
/********************************************************************************/
void drawDisplay7Inch(lcdArrows * arrow)
{
    static int16_t m, n;                            // m = row , n = column
    static coordinates stop;
    bool arrowActive;
    // write the arrows
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            // II: delete old arrows
            stop.x  = startCord[m][n].x + oldDiffCosResults[m][n];
            stop.y  = startCord[m][n].y + oldDiffSinResults[m][n];

            drawLine(startCord[m][n].x, startCord[m][n].y, stop.x, stop.y, set->backColorArrowWindow, oldArrowActive[m][n]);

            // III: write grid cross
            drawLine(startCord[m][n].x - 2, startCord[m][n].y, startCord[m][n].x + 2, startCord[m][n].y, set->gridColor, NO_ARROW);    // draw a small cross..
            drawLine(startCord[m][n].x, startCord[m][n].y - 2, startCord[m][n].x, startCord[m][n].y + 2, set->gridColor, NO_ARROW);    // ..as as grid indicator

            // IV: write new arrows
            stop.x  = startCord[m][n].x + arrow->dCos[m][n];

            // IV a: check if arrow is in designated display window
            // all good: arrow is in window
            if( (stop.x >= 0) && (stop.x < 507) )
            {
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                arrowActive = true;
            }
            // arrow moved out off screen on left side => scale stop.y and stop.x  down to left boundary
            else if(stop.x < 0)
            {
                arrow->dSin[m][n] *= (double)startCord[m][n].x / -arrow->dCos[m][n];    // minus because we know: dCos < 0
                arrow->dCos[m][n] = -startCord[m][n].x;
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                stop.x  = 0;
                arrowActive = false;
            }
            // if arrow moves out off screen on right side => scale down to right boundary
            else
            {
                arrow->dSin[m][n] *= (double)(506 - startCord[m][n].x) / arrow->dCos[m][n];
                arrow->dCos[m][n] = 506 - startCord[m][n].x;
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                stop.x  = 506;
                arrowActive = false;
            }
            if(stop.y < 0 || stop.y > 479)
            {
                arrowActive = false;
            }

            // if the option 'colored arrows' is enabled, the colors get linked to an
            // array with 768 different colors from dark blue to red.
            // if option is disabled colors get linked to an array with only 'black'.
            if( set->coloredArrows == true)
            {
                drawLine(startCord[m][n].x, startCord[m][n].y, stop.x, stop.y, arrowColor[arrow->arrowLength[m][n]], arrowActive);
            }
            else
            {
                drawLine(startCord[m][n].x, startCord[m][n].y, stop.x, stop.y, set->arrowColor, arrowActive);
            }

            oldDiffCosResults[m][n] = arrow->dCos[m][n];
            oldDiffSinResults[m][n] = arrow->dSin[m][n];
            oldArrowActive[m][n] = arrowActive;
        }
    }
}


/***************************  drawRectangle()   *********************************/
// Draws rectangles with specified color.                                       //
// This function is used to draw the menu backgrounds.                          //
/********************************************************************************/
void drawRectangle(short start_x, short start_y, short stop_x, short stop_y, color color)
{
    uint32_t start, stop;

    stop = (1+stop_x - start_x) * (1+stop_y - start_y);

    writePosition(start_x, start_y, stop_x, stop_y);
    writeCommand(0x2C);                        // start writing

    for (start = 0; start < stop; start++)
    {
        GPIO_PORTM_DATA_R = color.red;
        GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        GPIO_PORTM_DATA_R = color.green;
        GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        GPIO_PORTM_DATA_R = color.blue;
        GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
    }
}


/******************************  LCD WRITE LINE  ********************************/
//
/********************************************************************************/
//draws a line from start point x to stop point y directly to the display
void drawLine(int16_t start_x, int16_t start_y, int16_t stop_x, int16_t stop_y, color color, uint16_t arrowOption)
{
    int16_t delta_x = stop_x - start_x;
    int16_t delta_y = stop_y - start_y;

    // 90° or 270° line
    if (start_x == stop_x)
    {
        if (start_y > stop_y)       // 270° line
        {
            writeLine270Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && -delta_y > 1)
            {
                drawLine(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                drawLine(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else                        // 90° line
        {
            writeLine90Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && delta_y > 1)
            {
                drawLine(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                drawLine(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
    }
    // 0° or 180° line
    else if (start_y == stop_y)
    {
        if (start_x > stop_x)   // 180° line
        {
            writeLine180Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && -delta_x > 1)
            {
                drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else                    // 0° line
        {
            writeLine0Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && delta_x > 1)
            {
                drawLine(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                drawLine(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
    }
//////////////////////////////////////////////////////////////////////////////////
    else
    {
        double gain, angle;
        if(start_x < stop_x)                    // quadrant I or IV
        {
            if(start_y < stop_y)                // quadrant I
            {
                if(delta_x > delta_y)           // quadrant I  1. (gain < 1)
                {
                    gain = (double)delta_y / delta_x;
                    writeLineQuadrant1_I(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW & delta_x > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = atan2(delta_y, delta_x);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);    // upper arrow line

                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle + 2.5), color, NO_ARROW);    // lower arrow line
                    }
                }
                else                            // quadrant I  2. (gain >= 1)
                {
                    gain = (double)delta_x / delta_y;
                    writeLineQuadrant1_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = atan2(delta_y, delta_x);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle + 2.5), color, NO_ARROW);// lower arrow line
                    }
                }
            }

            else                                // quadrant IV
            {
                if(delta_x > -delta_y)          // quadrant IV  1. (gain < 1)
                {
                    gain = (double)-delta_y / delta_x;       // start_y -> stop_y  ;  y--
                    writeLineQuadrant4_I(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && delta_x > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = atan2(delta_y, delta_x);
                        //                        printf("%lf\n", angle);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle + 2.5), color, NO_ARROW);// lower arrow line
                    }
                }
                else                            // quadrant IV  2. (gain >= 1)
                {
                    gain = (double)delta_x / -delta_y;
                    writeLineQuadrant4_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = 1.571 - atan(gain);
                        drawLine(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
            }
        }
//////////////////////////////////////////////////////////////////////////////////
        else     // x < 0                       // quadrant II or III
        {
            if(start_y < stop_y)                // quadrant II
            {
                if(-delta_x > delta_y)          // quadrant II 1. (gain < 1)
                {
                    gain = (double)delta_y / -delta_x;
                    writeLineQuadrant2_I(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_x > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = atan(gain);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE - angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE - angle), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE + angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE + angle), color, NO_ARROW);// lower arrow line
                    }
                }
                else                            // quadrant II  2. (gain >= 1)
                {
                    gain = (double)-delta_x / delta_y;
                    writeLineQuadrant2_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = 1.571 - atan(gain);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
            }
            else                                // quadrant III 2.
            {
                if(delta_x < delta_y)       // gain < -1 (delta_x < 0 and delta_y < 0 !)
                {
                    gain = (double)delta_y / delta_x;
                    writeLineQuadrant3_I(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = atan(gain);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
                else                                // quadrant III 1.
                {
                    gain = (double)delta_x / delta_y;
                    writeLineQuadrant3_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_x > MIN_LENGTH_FOR_ARROW)
                    {
                        angle = 1.571 - atan(gain);
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
            }
        }
    }
}


/********************************************************************************/
void writeLine0Degree(short start_x, short start_y, short stop_x, short stop_y, color color)
{
    int16_t x;

    writePosition(start_x, start_y, stop_x, start_y);
    writeCommand(0x2C);
    for (x = start_x; x <= stop_x; x++)
    {
        writeData(color);
    }
}


/********************************************************************************/
void writeLine90Degree(short start_x, short start_y, short stop_x, short stop_y, color color)
{
    int16_t y;

    writePosition(start_x, start_y, start_x, stop_y);
    writeCommand(0x2C);
    for (y = start_y; y <= stop_y; y++)
    {
        writeData(color);
    }
}


/********************************************************************************/
void writeLine180Degree(short start_x, short start_y, short stop_x, short stop_y, color color)
{
    int16_t x;

    writePosition(stop_x, start_y, start_x, start_y);
    writeCommand(0x2C);
    for (x = stop_x; x <= start_x; x++)
    {
        writeData(color);
    }
}


/********************************************************************************/
void writeLine270Degree(short start_x, short start_y, short stop_x, short stop_y, color color)
{
    int16_t y;

    writePosition(start_x, stop_y, start_x, start_y);
    writeCommand(0x2C);
    for (y = stop_y; y <= start_y; y++)
    {
        writeData(color);
    }
}


/********************************************************************************/
void writeLineQuadrant1_I(short start_x, short start_y, short stop_x, short stop_y, double gain,color color)
{
    double gain2 = 0.5;

    while(start_y <= stop_y)
    {
        writePosition(start_x, start_y, stop_x, start_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if(start_x < stop_x)
                start_x++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        start_y++;
    }
}


/********************************************************************************/
void writeLineQuadrant1_II(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(start_x <= stop_x)
    {
        writePosition(start_x, start_y, start_x, stop_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( start_y < stop_y)
                start_y++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        start_x++;
    }
}


/********************************************************************************/
void writeLineQuadrant2_I(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(stop_y >= start_y)
    {
        writePosition(stop_x, stop_y, start_x, stop_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( stop_x < start_x)
                stop_x++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        stop_y--;
    }
}


/********************************************************************************/
void writeLineQuadrant2_II(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(start_x >= stop_x)
    {
        writePosition(start_x, start_y, start_x, stop_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( start_y < stop_y)
                start_y++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        start_x--;
    }
}


/********************************************************************************/
void writeLineQuadrant3_I(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(stop_y <= start_y)
    {
        writePosition(stop_x, stop_y, start_x, stop_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( stop_x < start_x)
                stop_x++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        stop_y++;
    }
}


/********************************************************************************/
void writeLineQuadrant3_II(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(stop_x <= start_x)
    {
        writePosition(stop_x, stop_y, stop_x, start_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( stop_y < start_y)
                stop_y++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        stop_x++;
    }
}


/********************************************************************************/
void writeLineQuadrant4_I(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(start_y >= stop_y)
    {
        writePosition(start_x, start_y, stop_x, start_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( start_x < stop_x)
                start_x++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        start_y--;
    }
}


/********************************************************************************/
void writeLineQuadrant4_II(short start_x, short start_y, short stop_x, short stop_y, double gain, color color)
{
    double gain2 = 0.5;

    while(stop_x >= start_x)
    {
        writePosition(stop_x, stop_y, stop_x, start_y);
        writeCommand(0x2C);
        while(gain2 < 1)
        {
            writeData(color);
            gain2 += gain;
            if( stop_y < start_y)
                stop_y++;
            else
                gain2 = 1;
        }
        gain2 -= 1;
        stop_x--;
    }
}


/****************************  generateColors()   *******************************/
// if option activated, the arrows have different colors in relation            //
// to their length. The color goes from dark blue for small arrows to           //
// red for long arrows.                                                         //
/********************************************************************************/
void generateColors(void)
{
    int i, val = 100, red, green, blue;

    for(i = 0; i < 768; i++)
    {
        if(val < 256){
            red = 0;
            blue = 0;
            blue = val;
        }
        else if(val < 512)
        {
            if(val > 250 && val < 400) val++;
            red = 0;
            green = val - 256;
            blue = 511 - val;
        }
        else if(val < 768)
        {
            if(val < 680) val++;
            red = val - 512;
            green = 255;
            blue = 0;
        }
        else if(val < 1024)
        {
            red = 255;
            green = 1023 - val;
            blue = 0;
        }
        arrowColor[i].red = red;
        arrowColor[i].green = green;
        arrowColor[i].blue = blue;

        val++;
    }
}


/********************************************************************************/
void writeCommand(unsigned char command)
{
    GPIO_PORTM_DATA_R = command;        // Write command byte
    GPIO_PORTQ_DATA_R = 0x11;           // Chip select = 0, Command mode select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/********************************************************************************/
void writeCmdData(unsigned char data)
{
    GPIO_PORTM_DATA_R = data;           // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/********************************************************************************/
void writeData(color color_)
{
    GPIO_PORTM_DATA_R = color_.red;      // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

    GPIO_PORTM_DATA_R = color_.green;    // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

    GPIO_PORTM_DATA_R = color_.blue;     // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/********************************************************************************/
void writePosition(uint16_t point1_x, uint16_t point1_y, uint16_t point2_x, uint16_t point2_y)
{
    writeCommand(0x2A);                 // Set page address (x-axis)
    writeCmdData(point1_x >> 8);        // Set start page address            HB
    writeCmdData(point1_x);             //                                   LB
    writeCmdData(point2_x >> 8);        // Set display_stop page addres      HB
    writeCmdData(point2_x);             //                                   LB

    writeCommand(0x2B);                 // Set column address (y-axis)
    writeCmdData(point1_y >> 8);        // Set start column address          HB
    writeCmdData(point1_y);             //                                   LB
    writeCmdData(point2_y >> 8);        // Set display_stop column address   HB
    writeCmdData(point2_y);             //                                   LB
}


/********************************************************************************/
void configureLCDHardware(uint32_t SysClock)
{
    // Set Port L  0-4: Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Set Port M Pins 0-7: Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port Q Pins 0-4: LCD Control output:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);  // Clock Port Q
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                                         | GPIO_PIN_4);

//////////////////////////////////////////////////////////////////////////////////
    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);           // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);           // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    writeCommand(SET_PLL_MN);               // Set PLL Freq to 120 MHz
    writeCmdData(0x24);                     //
    writeCmdData(0x02);                     //
    writeCmdData(0x04);                     //

    writeCommand(START_PLL);                // Start PLL
    writeCmdData(0x01);                     //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(START_PLL);                // Lock PLL
    writeCmdData(0x03);                     //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);           // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

//////////////////////////////////////////////////////////////////////////////////
    writeCommand(0xe6);                     // pixel clock frequency
    writeCmdData(0x04);                     // LCD_FPR = 290985 = 33.300 Mhz Result for 7" Display
    writeCmdData(0x70);
    writeCmdData(0xA9);

    writeCommand(SET_LCD_MODE);             // SET LCD MODE SIZE
    writeCmdData(0x20);                     // ..TFT panel 24bit
    writeCmdData(0x00);                     // ..TFT mode
    writeCmdData(0x03);                     // SET horizontal size = 800-1 HightByte
    writeCmdData(0x1F);                     // SET horizontal size = 800-1 LowByte
    writeCmdData(0x01);                     // SET vertical size = 480-1 HightByte
    writeCmdData(0xDF);                     // SET vertical size = 480-1 LowByte
    writeCmdData(0x00);                     // Even line RGB sequence / Odd line RGB sequence RGB

    writeCommand(SET_HORI_PERIOD);          // Set Horizontal Period
    writeCmdData(0x03);                     // 03 High byte of horizontal total period (display + non-display)
    writeCmdData(0x5E);                     // 51 Low byte of the horizontal total period (display + non-display)
    writeCmdData(0x00);                     // High byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data.
    writeCmdData(0x46);                     // 46 Low byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data
    writeCmdData(0x09);                     // Set the vertical sync
    writeCmdData(0x00);                     // Set horiz.Sync pulse start pos= 8    HB
    writeCmdData(0x08);                     // Set horiz.Sync pulse start pos= 8    LB
    writeCmdData(0x00);                     //

    writeCommand(SET_VERT_PERIOD);          // Set Vertical Period
    writeCmdData(0x01);                     // 01 High byte of the vertical total (display + non-display)
    writeCmdData(0xFE);                     // F4 Low byte F5 INCREASES SYNC TIME AND BACK PORCH
    writeCmdData(0x00);                     // 00
    writeCmdData(0x0C);                     // 0C =12 The non-display period in lines between the start of the frame and the first display data in line.

    writeCmdData(0x00);                     // Set vert.Sync pulse start pos= 8    HB
    writeCmdData(0x00);                     // Set vert.Sync pulse start pos= 8    LB
    writeCmdData(0x04);

    writeCommand(SET_ADRESS_MODE);
    writeCmdData(0x01);

    // PWM signal frequency = PLL clock
    writeCommand(0xBE);
    writeCmdData(0x08);
    writeCmdData(0x80);
    writeCmdData(0x01);

    writeCommand(0x0A);
    writeCmdData(0x1C);                     // Power Mode

    writeCommand(SET_PIXEL_DATA_FORMAT);    // set pixel data format 8bit
    writeCmdData(0x00);

    writeCommand(SET_DISPLAY_ON);           // Set display on

}
