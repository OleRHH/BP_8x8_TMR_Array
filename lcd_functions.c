#include <lcd_functions.h>


/**********************  # intern Prototypes #   **********************/
void writeChar(uint16_t, COLOR, COLOR);

void writeCommand(unsigned char);
void writeCmdData(unsigned char);
void writeData(COLOR);
void writePosition(uint16_t, uint16_t, uint16_t, uint16_t);
void generateColors(void);
void writeLine(short, short, short, short, COLOR, uint16_t);
void writeLine0Degree  (short, short, short, short, COLOR);
void writeLine90Degree (short, short, short, short, COLOR);
void writeLine180Degree(short, short, short, short, COLOR);
void writeLine270Degree(short, short, short, short, COLOR);
void writeLineQuadrant1_I (short, short, short, short, double, COLOR);  //   0° < degree < 90°
void writeLineQuadrant1_II(short, short, short, short, double, COLOR);  //   0° < degree < 90°
void writeLineQuadrant2_I (short, short, short, short, double, COLOR);  //  90° < degree < 180°
void writeLineQuadrant2_II(short, short, short, short, double, COLOR);  //  90° < degree < 180°
void writeLineQuadrant3_I (short, short, short, short, double, COLOR);  // 180° < degree < 270°
void writeLineQuadrant3_II(short, short, short, short, double, COLOR);  // 180° < degree < 270°
void writeLineQuadrant4_I (short, short, short, short, double, COLOR);  // 270° < degree < 360°
void writeLineQuadrant4_II(short, short, short, short, double, COLOR);  // 270° < degree < 360°




uint16_t offset = 0;

int16_t oldDiffSinResults[8][8];
int16_t oldDiffCosResults[8][8];
static COLOR color[768];




/***************************  writeInfos()   *******************************/
// writes some info as text on the display.                                 //
// Infos are: absolute or relative arrow mode, maximum measured analog,     //
// arrow max length.                                                        //
/****************************************************************************/
void writeInfos(bool relative, bool adcAVG, uint16_t maxArrowLength, uint32_t maximumAnalogValue, COLOR backColor)
{
    char charValue[100];
    static bool old = true;

    if(old != relative)
    {
        old = relative;
        writeScreenColor5INCH(backColor);
    }
    if(relative == true)
    {
        printString("relative:  true", 10, 300, (COLOR)BLACK, backColor);

        sprintf(charValue, "length: %.3d", maxArrowLength);
        printString(charValue, 40, 300, (COLOR)BLACK, backColor);

        sprintf(charValue, "max analog: %.3d", maximumAnalogValue);
        printString(charValue, 70, 300, (COLOR)BLACK, backColor);
    }
    else
    {
        printString("relative: false", 10, 300, (COLOR)BLACK, backColor);

        sprintf(charValue, "length: %.3d", maxArrowLength);
        printString(charValue, 40, 300, (COLOR)BLACK, backColor);

        sprintf(charValue, "max analog: %.3d", maximumAnalogValue);
        printString(charValue, 70, 300, (COLOR)BLACK, backColor);

        if(maximumAnalogValue > maxArrowLength)
        {
            printString("Clipping!", 100, 300, (COLOR)BLACK, backColor);
        }
        else
        {
            printString("Clipping!", 100, 300, backColor, backColor);
        }
    }
    if(adcAVG == false)
    {
        printString("Oversampling off", 250, 280, (COLOR)BLACK, backColor);
    }
    else
    {
        printString("Oversampling on ", 250, 280, (COLOR)BLACK, backColor);
    }
}

/**************************  readTouchscreen()   ****************************/
//         //
//       //
//                                                  //
/****************************************************************************/
enum CommandFromTouch readTouchscreen(char * command)
{
    command = NULL;
    return noNewCommand;
}


/**************************  generateColors()   ****************************/
// if option activated, the arrows have different colors in relation        //
// to their length. The color goes from dark blue for small arrows to       //
// red for long arrows.                                                     //
/****************************************************************************/
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
        color[i].red = red;
        color[i].green = green;
        color[i].blue = blue;
        val++;
    }
}


/****************************  printString()   *****************************/
// writes a string to the given position on the LD-Display                  //
/****************************************************************************/
void printString(char *text, uint16_t row, uint16_t column, COLOR color, COLOR backcolor)
{
    uint16_t letter, numLetter, lv;
    uint16_t length = strlen(text);
    uint16_t columnStart = column;
    uint16_t columnStop = columnStart + FONT_WIDTH_BIG - 1;
    uint16_t rowStart = row;
    uint16_t rowStop = rowStart + FONT_HIGHT_BIG - 1;

    for (numLetter = 0; numLetter < length; numLetter++)
    {
        writePosition(columnStart, rowStart, columnStop, rowStop);
        writeCommand(0x2C);
        for (lv = 0; lv < 32; lv += 2)
        {
            letter = (font_12_16[text[numLetter]][lv + 1] << 4)
                    | (font_12_16[text[numLetter]][lv] >> 4);
            writeChar(letter, color, backcolor);
        }
        columnStart += FONT_WIDTH_BIG;
        columnStop  += FONT_WIDTH_BIG;
    }
}


/****************************  writeChar()   *******************************/
// helper function for printString():                                      //
// writes a single letter with height 12 pixel                              //
/****************************************************************************/
void writeChar(uint16_t letter, COLOR color, COLOR backcolor)
{
    uint16_t lv;

    for (lv = 0; lv < 12; lv++) {
        if (letter & 1) {             // Write data byte
            GPIO_PORTM_DATA_R = color.red;
            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = color.green;
            GPIO_PORTQ_DATA_R = 0x15;               // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = color.blue;
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


/***********************  writeScreenColor5INCH()   ***********************/
// Writes the hole screen in one color                                      //
/****************************************************************************/
void writeScreenColor5INCH(COLOR color)
{
    uint32_t count = 0;

    generateColors();

    writePosition(0, 0, 479, 271);
    writeCommand(0x2C);

    while (count++ < 130560) {
        GPIO_PORTM_DATA_R = color.red;       // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;            // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;            // Initial state

        GPIO_PORTM_DATA_R = color.green;    // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

        GPIO_PORTM_DATA_R = color.blue;     // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
    }
}


/***********************  writeRectangle()   ********************************/
// Writes a rectangle as frame for the arrow field on the display.          //
/****************************************************************************/
void writeRecangle(void)
{
    // write the frame for the Array Display
    writeLine(80, 10, 720, 10, (COLOR)YELLOW, 0);
    writeLine(80, 470, 720, 470, (COLOR)YELLOW, 0);
    writeLine(80, 10, 80, 470, (COLOR)YELLOW, 0);
    writeLine(720, 10, 720, 470, (COLOR)YELLOW, 0);
}


/***********************  writeScreenColor7INCH()   *************************/
// Writes the hole screen in one color                                      //
/****************************************************************************/
void writeScreenColor7INCH(COLOR color)
{
    uint32_t count = 0;

    writePosition(0, 0, 799, 479);
    writeCommand(0x2C);

    while (count++ < 384000) {
        GPIO_PORTM_DATA_R = color.red;       // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;            // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;            // Initial state

        GPIO_PORTM_DATA_R = color.green;    // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

        GPIO_PORTM_DATA_R = color.blue;     // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
    }
}


/**************************  drawDisplay5Inch()   ***************************/
// draws all arrows to the 5 inch LC-Display.                               //
/****************************************************************************/
void drawDisplay5Inch(COLOR backColor)
{
    GPIO_PORTN_DATA_R |= 0b1000;        // for debugging: set high when handler is called

    int16_t m = 0, n = 0;               // m = row , n = column
    point start, stop;

    // write the arrows
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
//            init_grid();

            // I. delete old arrows
            start.x = n * 32 + GRID_OFFSET_X_5_INCH;
            start.y = m * 32 + GRID_OFFSET_Y_5_INCH;
            stop.x  = start.x + oldDiffCosResults[m][n];
            stop.y  = start.y + oldDiffSinResults[m][n];

            writeLine(start.x, start.y, stop.x, stop.y, backColor, WITH_ARROW);

            // II. write grid cross
            stop.x  = start.x;
            stop.y  = start.y;

            writeLine(start.x - 2, start.y, stop.x + 2, stop.y, (COLOR)BLACK, NO_ARROW);    // draw a small cross..
            writeLine(start.x, start.y - 2, stop.x, stop.y + 2, (COLOR)BLACK, NO_ARROW);    // ..as as grid indicator

            // III. write new arrows
            stop.x  = n * 32 + GRID_OFFSET_X_5_INCH + DiffCosResults[m][n];
            stop.y  = m * 32 + GRID_OFFSET_Y_5_INCH + DiffSinResults[m][n];

            writeLine(start.x, start.y, stop.x, stop.y, color[arrowLength[m][n]], WITH_ARROW);
            oldDiffCosResults[m][n] = DiffCosResults[m][n];
            oldDiffSinResults[m][n] = DiffSinResults[m][n];
        }
    }
    GPIO_PORTN_DATA_R ^= 0b1000;
}



/**************************  drawDisplay7Inch()   ***************************/
// draws all arrows to the 7 inch LC-Display.                               //
/****************************************************************************/
void drawDisplay7Inch(void)
{
    int16_t m = 0, n = 0, xGrid, yGrid;

    // delete the old arrows
    for(xGrid = GRID_OFFSET_X_7_INCH; xGrid < ( 480 + GRID_OFFSET_X_7_INCH); xGrid += 60)
    {
        for(yGrid = 470 - GRID_OFFSET_Y_7_INCH; yGrid > GRID_OFFSET_Y_7_INCH; yGrid -= 50)
        {
            writeLine(xGrid, yGrid, xGrid + oldDiffCosResults[m][n],
                       yGrid - oldDiffSinResults[m][n], (COLOR)0x000000, WITH_ARROW);
            m++;
        }
        m = 0;
        n++;
    }

    // write the new arrows
    n = 0;
    for(xGrid = GRID_OFFSET_X_7_INCH; xGrid < ( 480 + GRID_OFFSET_X_7_INCH); xGrid += 60)
    {
        for(yGrid = 470 - GRID_OFFSET_Y_7_INCH; yGrid > GRID_OFFSET_Y_7_INCH; yGrid -= 50)
        {
            writeLine(xGrid, yGrid, xGrid + DiffCosResults[m][n],
                       yGrid - DiffSinResults[m][n], (COLOR)0xffffff, WITH_ARROW);
            writeLine(xGrid - 2, yGrid, xGrid + 2, yGrid, (COLOR)0x0000FF, NO_ARROW);    // draw a small cross..
            writeLine(xGrid, yGrid - 2, xGrid, yGrid + 2, (COLOR)0x0000FF, NO_ARROW);    // ..as as grid indicator
            oldDiffCosResults[m][n] = DiffCosResults[m][n];
            oldDiffSinResults[m][n] = DiffSinResults[m][n];
            m++;
        }
        m = 0;
        n++;
    }
}


/******************************************************************************************************/
void writeCommand(unsigned char command)
{
    GPIO_PORTM_DATA_R = command;        // Write command byte
    GPIO_PORTQ_DATA_R = 0x11;           // Chip select = 0, Command mode select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/******************************************************************************************************/
void writeCmdData(unsigned char data)
{
    GPIO_PORTM_DATA_R = data;           // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/******************************************************************************************************/
void writeData(COLOR color)
{
    GPIO_PORTM_DATA_R = color.red;      // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

    GPIO_PORTM_DATA_R = color.green;    // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

    GPIO_PORTM_DATA_R = color.blue;     // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/******************************************************************************************************/
void writePosition(uint16_t point1_x, uint16_t point1_y, uint16_t point2_x, uint16_t point2_y)
{
	// offset: 1x272 or 2x272 or 3x272 => points to screen 1 or 2 or 3
    point1_y += offset;
    point2_y += offset;

    writeCommand(0x2A);                // Set page address (x-axis)
    writeCmdData(point1_x >> 8);      // Set start page address                HB
    writeCmdData(point1_x);           //                                       LB
    writeCmdData(point2_x >> 8);      // Set display_stop page address         HB
    writeCmdData(point2_x);           //                                       LB

    writeCommand(0x2B);                // Set column address (y-axis)
    writeCmdData(point1_y >> 8);      // Set start column address              HB
    writeCmdData(point1_y);           //                                       LB
    writeCmdData(point2_y >> 8);      // Set display_stop column address       HB
    writeCmdData(point2_y);           //                                       LB
}


/******************************  LCD WRITE LINE  *************************************************/
//draws a line from start point x to stop point y directly to the display
void writeLine(short start_x, short start_y, short stop_x, short stop_y, COLOR color, uint16_t arrowOption)
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
                writeLine(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                writeLine(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else                        // 90° line
        {
            writeLine90Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && delta_y > 1)
            {
                writeLine(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                writeLine(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
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
                writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else                    // 0° line
        {
            writeLine0Degree(start_x, start_y, stop_x, stop_y, color);

            if(arrowOption == WITH_ARROW && delta_x > 1)
            {
                writeLine(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                writeLine(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////
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
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);    // upper arrow line

                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
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
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
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
                        // TODO: arrows
                        angle = atan2(delta_y, delta_x);
                        //                        printf("%lf\n", angle);
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
                                   stop_y + ARROW_LENGTH*sin(angle + 2.5), color, NO_ARROW);// lower arrow line
                    }
                }
                else                            // quadrant IV  2. (gain >= 1)
                {
                    gain = (double)delta_x / -delta_y;
                    writeLineQuadrant4_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        writeLine(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
            }
        }
/////////////////////////////////////////////////////////////////////////////////////////
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
                        // TODO: arrows
                        angle = atan(gain);
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE - angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE - angle), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE + angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE + angle), color, NO_ARROW);// lower arrow line
                    }
                }
                else                            // quadrant II  2. (gain >= 1)
                {
                    gain = (double)-delta_x / delta_y;
                    writeLineQuadrant2_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && delta_y > MIN_LENGTH_FOR_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
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
                        // TODO: arrows
                        angle = atan(gain);
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
                else                                // quadrant III 1.
                {
                    gain = (double)delta_x / delta_y;
                    writeLineQuadrant3_II(start_x, start_y, stop_x, stop_y, gain, color);

                    if(arrowOption == WITH_ARROW && -delta_x > MIN_LENGTH_FOR_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper arrow line
                        writeLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower arrow line
                    }
                }
            }
        }
    }
}


/****************************************************************************/
void writeLine0Degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t x;

    writePosition(start_x, start_y, stop_x, start_y);
    writeCommand(0x2C);
    for (x = start_x; x <= stop_x; x++)
    {
        writeData(color);
    }
}


/****************************************************************************/
void writeLine90Degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t y;

    writePosition(start_x, start_y, start_x, stop_y);
    writeCommand(0x2C);
    for (y = start_y; y <= stop_y; y++)
    {
        writeData(color);
    }
}


/****************************************************************************/
void writeLine180Degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t x;

    writePosition(stop_x, start_y, start_x, start_y);
    writeCommand(0x2C);
    for (x = stop_x; x <= start_x; x++)
    {
        writeData(color);
    }
}


/****************************************************************************/
void writeLine270Degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t y;

    writePosition(start_x, stop_y, start_x, start_y);
    writeCommand(0x2C);
    for (y = stop_y; y <= start_y; y++)
    {
        writeData(color);
    }
}


/****************************************************************************/
void writeLineQuadrant1_I(short start_x, short start_y, short stop_x, short stop_y, double gain,COLOR color)
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


/****************************************************************************/
void writeLineQuadrant1_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant2_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant2_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant3_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant3_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant4_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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


/****************************************************************************/
void writeLineQuadrant4_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
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

/******************************************************************************************************/
// LCD Panel initialize:
void ConfigureLCD5Inch(uint32_t SysClock) {
    uint32_t value;

    // Set Port L  0-4: Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Set Port M Pins 0-7: Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

/*************************************************************************/
    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    writeCommand(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    writeCmdData(0x24);                   //
    writeCmdData(0x02);                   //
    writeCmdData(0x04);                   //

    writeCommand(START_PLL);               // Start PLL
    writeCmdData(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(START_PLL);               // Lock PLL
    writeCmdData(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    value = 0x01EFFF;
    writeCommand(SET_LSHIFT);              // Set LCD Pixel Clock 12.7 Mhz (0x01EFFF)
    writeCmdData(value>>16);              //
    writeCmdData(value>>8);               //
    writeCmdData(value);                  //
    writeCommand(SET_LCD_MODE);            // Set LCD Panel mode to:
    writeCmdData(0x20);                   // ..TFT panel 24bit
    writeCmdData(0x00);                   // ..TFT mode
    writeCmdData(0x01);                   // Horizontal size 480-1 (aka 479 ;)    HB
    writeCmdData(0xDF);                   // Horizontal size 480-1                LB
    writeCmdData(0x01);                   // Vertical size 272-1   (aka 271 ;)    HB
    writeCmdData(0x0F);                   // Vertical size 272-1                  LB
    writeCmdData(0x00);                   // even/odd line RGB

    writeCommand(SET_HORI_PERIOD);         // Set Horizontal period
    writeCmdData(0x02);                   // Set HT total pixel=531               HB
    writeCmdData(0x13);                   // Set HT total pixel=531               LB
    writeCmdData(0x00);                   // Set Horiz.sync pulse start pos = 43  HB
    writeCmdData(0x2B);                   // Set Horiz.sync pulse start pos = 43  LB
    writeCmdData(0x0A);                   // Set horiz.sync pulse with = 10
    writeCmdData(0x00);                   // Set horiz.Sync pulse start pos= 8    HB
    writeCmdData(0x08);                   // Set horiz.Sync pulse start pos= 8    LB
    writeCmdData(0x00);                   //

    writeCommand(SET_VERT_PERIOD);         // Set Vertical Period
    writeCmdData(0x01);                   // Set VT lines = 288                   HB
    writeCmdData(0x20);                   // Set VT lines = 288                   LB
    writeCmdData(0x00);                   // Set VPS = 12                         HB
    writeCmdData(0x0C);                   // Set VPS = 12                         LB
    writeCmdData(0x0A);                   // Set vert.sync pulse with = 10
    writeCmdData(0x00);                   // Set vert.Sync pulse start pos= 8    HB
    writeCmdData(0x00);                   // Set vert.Sync pulse start pos= 8    LB
    writeCmdData(0x04);                   //

    writeCommand(0xF0);                    // Set LCD color data format
    writeCmdData(0x00);                   // Set pixel data format = 8 bit

//    writeCommand(SET_ADRESS_MODE);         // Set address mode
//    writeCmdData(0b00000001);             // flip vertical

    writeCommand(0x29);                    // Set display on
}


/******************************************************************************************************/
void ConfigureLCD7Inch(uint32_t SysClock) {

    // Set Port L  0-4: Multiplexer address output for 8x8 Array
    // Pin 3 = D; Pin 2 = C; Pin 1 = B; Pin 0 = A; Pin 4 = nD
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);

    // Set Port M Pins 0-7: Output LCD Data
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);            // enable clock-gate Port M
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));     // wait until clock ready
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0xFF);

    // Set Port N Pins 0-3: Onboard LEDs output (0-1)  debug outputs (2-3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

/*************************************************************************/
    GPIO_PORTQ_DATA_R = 0x00;
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    writeCommand(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    writeCmdData(0x24);                   //
    writeCmdData(0x02);                   //
    writeCmdData(0x04);                   //

    writeCommand(START_PLL);               // Start PLL
    writeCmdData(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(START_PLL);               // Lock PLL
    writeCmdData(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    writeCommand(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    writeCommand(0xe6);       // pixel clock frequency
    writeCmdData(0x04); // LCD_FPR = 290985 = 33.300 Mhz Result for 7" Display
    writeCmdData(0x70);   //
    writeCmdData(0xA9);   //

    writeCommand(0xB0);          //SET LCD MODE   SIZE !!
    writeCmdData(0x19); //19 TFT panel data width - Enable FRC or dithering for color depth enhancement 8 18  1f- 38
    writeCmdData(0x20); //SET TFT MODE & hsync+Vsync+DEN MODE   20  or 00
    writeCmdData(0x03); //SET horizontal size=800+1 HightByte   !!!!!!!!!!!!
    writeCmdData(0x1F);      //SET horizontal size=800+1 LowByte
    writeCmdData(0x01);      //SET vertical size=480+1 HightByte
    writeCmdData(0xDF);      //SET vertical size=480+1 LowByte
    writeCmdData(0x00); //Even line RGB sequence / Odd line RGB sequence RGB

    writeCommand(0xB4);            // Set Horizontal Period
    writeCmdData(0x03); //03 High byte of horizontal total period (display + non-display)
    writeCmdData(0x5E); //51 Low byte of the horizontal total period (display + non-display)
    writeCmdData(0x00); //High byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data.
    writeCmdData(0x46); //**   // 46 Low byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data
    writeCmdData(0x09);       // Set the vertical sync
//    writeCmdData(0x00);       //SET Hsync pulse start
//    writeCmdData(0x00);                                       //00
//    writeCmdData(0x00); //SET Hsync pulse subpixel start position  //00
    writeCmdData(0x00);             // Set horiz.Sync pulse start pos= 8    HB
    writeCmdData(0x08);             // Set horiz.Sync pulse start pos= 8    LB
    writeCmdData(0x00);                   //
    //   ** too small will give you half a PICTURE !!

    writeCommand(0xB6);          //Set Vertical Period
    writeCmdData(0x01); //01 High byte of the vertical total (display + non-display)
    writeCmdData(0xFE); //F4 Low byte F5 INCREASES SYNC TIME AND BACK PORCH
    writeCmdData(0x00);      // 00
    writeCmdData(0x0C); //0C =12 The non-display period in lines between the start of the frame and the first display data in line.
//    writeCmdData(0x00); //Set the vertical sync pulse width (LFRAME) in lines.
//    writeCmdData(0x00);      //SET Vsync pulse start position
//    writeCmdData(0x00);
    writeCmdData(0x00);              // Set vert.Sync pulse start pos= 8    HB
    writeCmdData(0x00);              // Set vert.Sync pulse start pos= 8    LB
    writeCmdData(0x04);                   //

    writeCommand(0x36);
    writeCmdData(0x01);

    // PWM signal frequency = PLL clock
    writeCommand(0xBE);
    writeCmdData(0x08);
    writeCmdData(0x80);
    writeCmdData(0x01);

    writeCommand(0x0A);
    writeCmdData(0x1C);         //Power Mode

    writeCommand(0xF0); //set pixel data format 8bit
    writeCmdData(0x00);

    writeCommand(0x29);                    // Set display on

}


/*********************************************************************************************/
void ConfigureGPIO(void)
{
    // Set Port Q Pins 0-4: LCD Control output:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);  // Clock Port Q
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ));
    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                                         | GPIO_PIN_4);
}
