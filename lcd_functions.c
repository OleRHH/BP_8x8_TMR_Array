

#include <lcd_functions.h>

// intern prototypes
void write_char(uint16_t, COLOR, COLOR);
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
void write_line_quadrant_1_I (short, short, short, short, double, COLOR);  //   0° < degree < 90°
void write_line_quadrant_1_II(short, short, short, short, double, COLOR);  //   0° < degree < 90°
void write_line_quadrant_2_I (short, short, short, short, double, COLOR);  //  90° < degree < 180°
void write_line_quadrant_2_II(short, short, short, short, double, COLOR);  //  90° < degree < 180°
void write_line_quadrant_3_I (short, short, short, short, double, COLOR);  // 180° < degree < 270°
void write_line_quadrant_3_II(short, short, short, short, double, COLOR);  // 180° < degree < 270°
void write_line_quadrant_4_I (short, short, short, short, double, COLOR);  // 270° < degree < 360°
void write_line_quadrant_4_II(short, short, short, short, double, COLOR);  // 270° < degree < 360°




uint16_t offset = 0;

int16_t oldDiffSinResults[8][8];
int16_t oldDiffCosResults[8][8];

extern int16_t DiffCosResults[8][8];
extern int16_t DiffSinResults[8][8];

extern const char font_12_16[256][32];


/***************************  write_Infos()   *******************************/
// writes some info as text on the display.                                 //
// Infos are: absolute or relative arrow mode, maximum measured analog,     //
// arrow max length.                                                        //
/****************************************************************************/
void write_Infos(bool relative, uint16_t maxArrowSize, uint32_t maximumAnalogValue)
{
    char charValue[100];
    static bool old = true;

    if(old != relative)
    {
        old = relative;
        write_screen_color5INCH((COLOR)WHITE);
    }
    if(relative == true)
    {
        print_string("relative:  true", 10, 300, (COLOR)BLACK, (COLOR)WHITE);

        sprintf(charValue, "length: %.3d", maxArrowSize);
        print_string(charValue, 40, 300, (COLOR)BLACK, (COLOR)WHITE);

        sprintf(charValue, "max analog: %.3d", maximumAnalogValue);
        print_string(charValue, 70, 300, (COLOR)BLACK, (COLOR)WHITE);
    }
    else
    {
        print_string("relative: false", 10, 300, (COLOR)BLACK, (COLOR)WHITE);

        sprintf(charValue, "length: %.3d", maxArrowSize);
        print_string(charValue, 40, 300, (COLOR)BLACK, (COLOR)WHITE);

        sprintf(charValue, "max analog: %.3d", maximumAnalogValue);
        print_string(charValue, 70, 300, (COLOR)BLACK, (COLOR)WHITE);

        if(maximumAnalogValue > maxArrowSize)
        {
            print_string("Clipping!", 100, 300, (COLOR)BLACK, (COLOR)WHITE);
        }
        else
        {
            print_string("Clipping!", 100, 300, (COLOR)WHITE, (COLOR)WHITE);
        }
    }
}


/***************************  screen_show_nr()   ****************************/
// the display dimensions are 480*272 pixels => 130560 pixels               //
// the lcd memory is much bigger, so three display screens can be saved     //
// simultaneously there. screen_show_nr() switches between those screens.   //
/****************************************************************************/
void screen_show_nr(uint16_t nr)
{
    uint16_t value = nr * 272;

    write_command(0x37);                     // Set scroll area
    write_cmd_data(value >> 8);              // TFA high byte
    write_cmd_data(value);                   // TFA low byte
}


/***************************  screen_show_nr()   *****************************/
// the display dimensions are 480*272 pixels => 130560 pixels               //
// the lcd memory is much bigger, so three display screens can be saved     //
// simultaneously there. screen_write_nr() to which is written.   //
/****************************************************************************/
void screen_write_nr(uint16_t nr)
{
    offset = nr * 272;
}


/****************************  print_string()   *****************************/
// writes a string to the given position on the LD-Display                  //
/****************************************************************************/
void print_string(char *text, uint16_t row, uint16_t column, COLOR color, COLOR backcolor)
{
    uint16_t letter, numLetter, lv;
    uint16_t length = strlen(text);
    uint16_t columnStart = column;
    uint16_t columnStop = columnStart + FONT_WIDTH_BIG - 1;
    uint16_t rowStart = row;
    uint16_t rowStop = rowStart + FONT_HIGHT_BIG - 1;

    for (numLetter = 0; numLetter < length; numLetter++)
    {
        write_position(columnStart, rowStart, columnStop, rowStop);
        write_command(0x2C);
        for (lv = 0; lv < 32; lv += 2)
        {
            letter = (font_12_16[text[numLetter]][lv + 1] << 4)
                    | (font_12_16[text[numLetter]][lv] >> 4);
            write_char(letter, color, backcolor);
        }
        columnStart += FONT_WIDTH_BIG;
        columnStop  += FONT_WIDTH_BIG;
    }
}


/****************************  write_char()   *******************************/
// helper function for print_string():                                      //
// writes a single letter with height 12 pixel                              //
/****************************************************************************/
void write_char(uint16_t letter, COLOR color, COLOR backcolor)
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


/***********************  write_screen_color5INCH()   ***********************/
// Writes the hole screen in one color                                      //
/****************************************************************************/
void write_screen_color5INCH(COLOR color)
{
    uint32_t count = 0;

    write_position(0, 0, 479, 271);
    write_command(0x2C);

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


/***********************  write_screen_color7INCH()   ***********************/
// Writes the hole screen in one color                                      //
/****************************************************************************/
void write_screen_color7INCH(COLOR color)
{
    uint32_t count = 0;

    write_position(0, 0, 799, 479);
    write_command(0x2C);

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
void drawDisplay5Inch(void)
{
    int16_t m = 0, n = 0, xGrid, yGrid;

    // delete the old arrows
    for(xGrid = GRID_OFFSET_X_5_INCH; xGrid < ( 256 + GRID_OFFSET_X_5_INCH); xGrid += 32)
    {
        for(yGrid = 272 - GRID_OFFSET_Y_5_INCH; yGrid > GRID_OFFSET_Y_5_INCH; yGrid -= 32)
        {
            write_line(xGrid, yGrid, xGrid + oldDiffCosResults[m][n], yGrid - oldDiffSinResults[m][n], (COLOR)0xffffff, WITH_ARROW);
            m++;
        }
        m = 0;
        n++;
    }

    // write the new arrows
    n = 0;
    for(xGrid = GRID_OFFSET_X_5_INCH; xGrid < ( 256 + GRID_OFFSET_X_5_INCH); xGrid += 32)
    {
        for(yGrid = 272 - GRID_OFFSET_Y_5_INCH; yGrid > GRID_OFFSET_Y_5_INCH; yGrid -= 32)
        {
            write_line(xGrid - 2, yGrid, xGrid + 2, yGrid, (COLOR)0x0000ff, NO_ARROW);        // draw a small cross..
            write_line(xGrid, yGrid - 2, xGrid, yGrid + 2, (COLOR)0x0000ff, NO_ARROW);        // ..as as grid indicator
            write_line(xGrid, yGrid, xGrid + DiffCosResults[m][n], yGrid - DiffSinResults[m][n], (COLOR)0xff0000, WITH_ARROW);
            oldDiffCosResults[m][n] = DiffCosResults[m][n];
            oldDiffSinResults[m][n] = DiffSinResults[m][n];
            m++;
        }
        m = 0;
        n++;
    }
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
            write_line(xGrid, yGrid, xGrid + oldDiffCosResults[m][n], yGrid - oldDiffSinResults[m][n], (COLOR)0x000000, WITH_ARROW);
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
            write_line(xGrid, yGrid, xGrid + DiffCosResults[m][n], yGrid - DiffSinResults[m][n], (COLOR)0xffffff, WITH_ARROW);
            write_line(xGrid - 2, yGrid, xGrid + 2, yGrid, (COLOR)0x0000FF, NO_ARROW);        // draw a small cross..
            write_line(xGrid, yGrid - 2, xGrid, yGrid + 2, (COLOR)0x0000FF, NO_ARROW);        // ..as as grid indicator
            oldDiffCosResults[m][n] = DiffCosResults[m][n];
            oldDiffSinResults[m][n] = DiffSinResults[m][n];
            m++;
        }
        m = 0;
        n++;
    }
}


/******************************************************************************************************/
void write_command(unsigned char command)
{
    GPIO_PORTM_DATA_R = command;        // Write command byte
    GPIO_PORTQ_DATA_R = 0x11;           // Chip select = 0, Command mode select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/******************************************************************************************************/
void write_cmd_data(unsigned char data)
{
    GPIO_PORTM_DATA_R = data;           // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}


/******************************************************************************************************/
void write_data(COLOR color)
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
void write_position(uint16_t point1_x, uint16_t point1_y, uint16_t point2_x, uint16_t point2_y)
{
    GPIO_PORTN_DATA_R ^= 4;
	// offset: 1x272 or 2x272 or 3x272 => points to screen 1 or 2 or 3
    point1_y += offset;
    point2_y += offset;

    write_command(0x2A);                // Set page address (x-axis)
    write_cmd_data(point1_x >> 8);      // Set start page address                HB
    write_cmd_data(point1_x);           //                                       LB
    write_cmd_data(point2_x >> 8);      // Set display_stop page address         HB
    write_cmd_data(point2_x);           //                                       LB

    write_command(0x2B);                // Set column address (y-axis)
    write_cmd_data(point1_y >> 8);      // Set start column address              HB
    write_cmd_data(point1_y);           //                                       LB
    write_cmd_data(point2_y >> 8);      // Set display_stop column address       HB
    write_cmd_data(point2_y);           //                                       LB
}


/******************************  LCD WRITE LINE  *************************************************/
//draws a line from startpoint x to stoppoint y directly to the display
void write_line(short start_x, short start_y, short stop_x, short stop_y, COLOR color, uint16_t arrowOption)
{
    int16_t delta_x = stop_x - start_x;
    int16_t delta_y = stop_y - start_y;

    // 90° or 270° line
    if (start_x == stop_x)
    {
        if (start_y > stop_y)
        {
            write_line_270_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_y < 1)
            {
                write_line(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                write_line(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else
        {
            write_line_90_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_y > 1)
            {
                write_line(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                write_line(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
    }
    // 0° or 180° line
    else if (start_y == stop_y)
    {
        if (start_x > stop_x)
        {
            write_line_180_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_x < 1)
            {
                write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
        else
        {
            write_line_0_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_x > 1)
            {
                write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y - ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
                write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                           stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROW);
            }
        }
    }
/////////////////////////////////////////////////////////////////////////////////////////
    else
    {
        double gain, angle;
        if(start_x < stop_x)                    // Quadrant I oder IV (rechts von y-Achse)
        {
            if(start_y < stop_y)                // Quadrant I   (oberhalb von x-Achse)
            {
                if(delta_x > delta_y)           // Quadrant I  1. (Steigung < 1)
                {
                    gain = (double)delta_y / delta_x;
                    write_line_quadrant_1_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW && delta_x > 1)
                    {
                        // TODO: arrows
                        angle = atan(gain);
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
                else                            // Quadrant I  2. (Steigung >= 1)
                {
                    gain = (double)delta_x / delta_y;
                    write_line_quadrant_1_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW && delta_x > 1)
                    {
                        angle = atan(gain);
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y - 10*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
            }

            else                                // Quadrant IV   (unterhalb von x-Achse)
            {
                if(delta_x > -delta_y)          // Quadrant IV  1. (Steigung < 1)
                {
                    gain = (double)-delta_y / delta_x;       // start_y -> stop_y  ;  y--
                    write_line_quadrant_4_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = atan(gain);
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
                else                            // Quadrant IV  2. (Steigung >= 1)
                {
                    gain = (double)delta_x / -delta_y;
                    write_line_quadrant_4_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x - ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
            }
        }
/////////////////////////////////////////////////////////////////////////////////////////
        else     // x < 0                       // Quadrant II oder III (links von y-Achse)
        {
            if(start_y < stop_y)                // Quadrant II (oberhalb von x-Achse)
            {
                if(-delta_x > delta_y)          // Quadrant II 1. (Steigung < 1)
                {
                    gain = (double)delta_y / -delta_x;
                    write_line_quadrant_2_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = atan(gain);
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
                else                            // Quadrant II  2. (Steigung >= 1)
                {
                    gain = (double)-delta_x / delta_y;
                    write_line_quadrant_2_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
            }
            else                                // Quadrant III  (unterhalb von x-Achse)
            {
                if(delta_x < delta_y)       // gain < -1 (delta_x < 0 and delta_y < 0 !)
                {
                    gain = (double)delta_y / delta_x;
                    write_line_quadrant_3_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = atan(gain);
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
                else
                {
                    gain = (double)delta_x / delta_y;
                    write_line_quadrant_3_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                        angle = 1.571 - atan(gain);
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE-angle),
                                   stop_y - ARROW_LENGTH*sin(ARROW_ANGLE-angle), color, NO_ARROW);// upper
                        write_line(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(ARROW_ANGLE+angle),
                                   stop_y + ARROW_LENGTH*sin(ARROW_ANGLE+angle), color, NO_ARROW);// lower
                    }
                }
            }
        }
    }
}


/****************************************************************************/
void write_line_0_degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t x;

    write_position(start_x, start_y, stop_x, start_y);
    write_command(0x2C);
    for (x = start_x; x <= stop_x; x++)
    {
        write_data(color);
    }
}


/****************************************************************************/
void write_line_90_degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t y;

    write_position(start_x, start_y, start_x, stop_y);
    write_command(0x2C);
    for (y = start_y; y <= stop_y; y++)
    {
        write_data(color);
    }
}


/****************************************************************************/
void write_line_180_degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t x;

    write_position(stop_x, start_y, start_x, start_y);
    write_command(0x2C);
    for (x = stop_x; x <= start_x; x++)
    {
        write_data(color);
    }
}


/****************************************************************************/
void write_line_270_degree(short start_x, short start_y, short stop_x, short stop_y, COLOR color)
{
    int16_t y;

    write_position(start_x, stop_y, start_x, start_y);
    write_command(0x2C);
    for (y = stop_y; y <= start_y; y++)
    {
        write_data(color);
    }
}


/****************************************************************************/
void write_line_quadrant_1_I(short start_x, short start_y, short stop_x, short stop_y, double gain,COLOR color)
{
    double gain2 = 0;

    while(start_y <= stop_y)
    {
        write_position(start_x, start_y, stop_x, start_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( start_x < stop_x) start_x++;
        }
        gain2 -= 1;
        start_y++;
    }
}


/****************************************************************************/
void write_line_quadrant_1_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(start_x <= stop_x)
    {
        write_position(start_x, start_y, start_x, stop_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( start_y < stop_y) start_y++;
        }
        gain2 -= 1;
        start_x++;
    }
}


/****************************************************************************/
void write_line_quadrant_2_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(stop_y >= start_y)
    {
        write_position(stop_x, stop_y, start_x, stop_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( stop_x < start_x) stop_x++;
        }
        gain2 -= 1;
        stop_y--;
    }
}


/****************************************************************************/
void write_line_quadrant_2_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(start_x >= stop_x)
    {
        write_position(start_x, start_y, start_x, stop_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( start_y < stop_y) start_y++;
        }
        gain2 -= 1;
        start_x--;
    }
}


/****************************************************************************/
void write_line_quadrant_3_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(stop_y <= start_y)
    {
        write_position(stop_x, stop_y, start_x, stop_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( stop_x < start_x) stop_x++;
        }
        gain2 -= 1;
        stop_y++;
    }
}


/****************************************************************************/
void write_line_quadrant_3_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(stop_x <= start_x)
    {
        write_position(stop_x, stop_y, stop_x, start_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( stop_y < start_y) stop_y++;
        }
        gain2 -= 1;
        stop_x++;
    }
}


/****************************************************************************/
void write_line_quadrant_4_I(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(start_y >= stop_y)
    {
        write_position(start_x, start_y, stop_x, start_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( start_x < stop_x) start_x++;
        }
        gain2 -= 1;
        start_y--;
    }
}


/****************************************************************************/
void write_line_quadrant_4_II(short start_x, short start_y, short stop_x, short stop_y, double gain, COLOR color)
{
    double gain2 = 0;

    while(stop_x >= start_x)
    {
        write_position(stop_x, stop_y, stop_x, start_y);
        write_command(0x2C);
        while(gain2 < 1)
        {
            write_data(color);
            gain2 += gain;
            if( stop_y < start_y) stop_y++;
        }
        gain2 -= 1;
        stop_x--;
    }
}

/******************************************************************************************************/
// LCD Panel initialize:
void ConfigureLCD5Inch(uint32_t SysClock) {
    uint32_t value;

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    write_command(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    write_cmd_data(0x24);                   //
    write_cmd_data(0x02);                   //
    write_cmd_data(0x04);                   //

    write_command(START_PLL);               // Start PLL
    write_cmd_data(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(START_PLL);               // Lock PLL
    write_cmd_data(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    value = 0x01EFFF;
    write_command(SET_LSHIFT);              // Set LCD Pixel Clock 12.7 Mhz (0x01EFFF)
    write_cmd_data(value>>16);              //
    write_cmd_data(value>>8);               //
    write_cmd_data(value);                  //
    write_command(SET_LCD_MODE);            // Set LCD Panel mode to:
    write_cmd_data(0x20);                   // ..TFT panel 24bit
    write_cmd_data(0x00);                   // ..TFT mode
    write_cmd_data(0x01);                   // Horizontal size 480-1 (aka 479 ;)    HB
    write_cmd_data(0xDF);                   // Horizontal size 480-1                LB
    write_cmd_data(0x01);                   // Vertical size 272-1   (aka 271 ;)    HB
    write_cmd_data(0x0F);                   // Vertical size 272-1                  LB
    write_cmd_data(0x00);                   // even/odd line RGB

    write_command(SET_HORI_PERIOD);         // Set Horizontal period
    write_cmd_data(0x02);                   // Set HT total pixel=531               HB
    write_cmd_data(0x13);                   // Set HT total pixel=531               LB
    write_cmd_data(0x00);                   // Set Horiz.sync pulse start pos = 43  HB
    write_cmd_data(0x2B);                   // Set Horiz.sync pulse start pos = 43  LB
    write_cmd_data(0x0A);                   // Set horiz.sync pulse with = 10
    write_cmd_data(0x00);                   // Set horiz.Sync pulse start pos= 8    HB
    write_cmd_data(0x08);                   // Set horiz.Sync pulse start pos= 8    LB
    write_cmd_data(0x00);                   //

    write_command(SET_VERT_PERIOD);         // Set Vertical Period
    write_cmd_data(0x01);                   // Set VT lines = 288                   HB
    write_cmd_data(0x20);                   // Set VT lines = 288                   LB
    write_cmd_data(0x00);                   // Set VPS = 12                         HB
    write_cmd_data(0x0C);                   // Set VPS = 12                         LB
    write_cmd_data(0x0A);                   // Set vert.sync pulse with = 10
    write_cmd_data(0x00);                   // Set vert.Sync pulse start pos= 8    HB
    write_cmd_data(0x00);                   // Set vert.Sync pulse start pos= 8    LB
    write_cmd_data(0x04);                   //

    write_command(0xF0);                    // Set LCD color data format
    write_cmd_data(0x00);                   // Set pixel data format = 8 bit

    write_command(0x30);                    // Set partial area
    write_cmd_data(0);                      // Start row High
    write_cmd_data(0);                      // Start row Low
    value = 543;
    write_cmd_data(value >> 8);             // Stop row High
    write_cmd_data(value);                  // Stop row Low

    write_command(0x12);                    // enter partial mode

    write_command(0x33);                    // Set scroll area
    value = 0;
    write_cmd_data(value >> 8);             // TFA high byte (TFA = 0)
    write_cmd_data(value);                  // TFA low byte
    value = 544;
    write_cmd_data(value >> 8);             // VSA high byte (VSA = 272)
    write_cmd_data(value);                  // VSA low byte
    value = 0;
    write_cmd_data(value >> 8);             // BFA high byte (BFA = 543)
    write_cmd_data(value);                  // BFA low byte

    write_command(0x29);                    // Set display on
}


/******************************************************************************************************/
void ConfigureLCD7Inch(uint32_t SysClock) {

    GPIO_PORTQ_DATA_R = 0x00;
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R = INITIAL_STATE;      // Initial state
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    GPIO_PORTQ_DATA_R &= ~RST;              // Hardware reset
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms
    GPIO_PORTQ_DATA_R |= RST;               //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

    write_command(SET_PLL_MN);              // Set PLL Freq to 120 MHz
    write_cmd_data(0x24);                   //
    write_cmd_data(0x02);                   //
    write_cmd_data(0x04);                   //

    write_command(START_PLL);               // Start PLL
    write_cmd_data(0x01);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(START_PLL);               // Lock PLL
    write_cmd_data(0x03);                   //
    SysCtlDelay((SysClock/3) / 1000);       // wait 1 ms

    write_command(SOFTWARE_RESET);          // Software reset
    SysCtlDelay((SysClock/3) / 100);        // wait 10 ms

/*************************************************************************/
    write_command(0xe6);       // pixel clock frequency
    write_cmd_data(0x04); // LCD_FPR = 290985 = 33.300 Mhz Result for 7" Display
    write_cmd_data(0x70);   //
    write_cmd_data(0xA9);   //

    write_command(0xB0);          //SET LCD MODE   SIZE !!
    write_cmd_data(0x19); //19 TFT panel data width - Enable FRC or dithering for color depth enhancement 8 18  1f- 38
    write_cmd_data(0x20); //SET TFT MODE & hsync+Vsync+DEN MODE   20  or 00
    write_cmd_data(0x03); //SET horizontal size=800+1 HightByte   !!!!!!!!!!!!
    write_cmd_data(0x1F);      //SET horizontal size=800+1 LowByte
    write_cmd_data(0x01);      //SET vertical size=480+1 HightByte
    write_cmd_data(0xDF);      //SET vertical size=480+1 LowByte
    write_cmd_data(0x00); //Even line RGB sequence / Odd line RGB sequence RGB

    write_command(0xB4);            // Set Horizontal Period
    write_cmd_data(0x03); //03 High byte of horizontal total period (display + non-display)
    write_cmd_data(0x5E); //51 Low byte of the horizontal total period (display + non-display)
    write_cmd_data(0x00); //High byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data.
    write_cmd_data(0x46); //**   // 46 Low byte of the non-display period between the start of the horizontal sync (LLINE) signal and the first display data
    write_cmd_data(0x09);       // Set the vertical sync
//    write_cmd_data(0x00);       //SET Hsync pulse start
//    write_cmd_data(0x00);                                       //00
//    write_cmd_data(0x00); //SET Hsync pulse subpixel start position  //00
    write_cmd_data(0x00);             // Set horiz.Sync pulse start pos= 8    HB
    write_cmd_data(0x08);             // Set horiz.Sync pulse start pos= 8    LB
    write_cmd_data(0x00);                   //
    //   ** too small will give you half a PICTURE !!

    write_command(0xB6);          //Set Vertical Period
    write_cmd_data(0x01); //01 High byte of the vertical total (display + non-display)
    write_cmd_data(0xFE); //F4 Low byte F5 INCREASES SYNC TIME AND BACK PORCH
    write_cmd_data(0x00);      // 00
    write_cmd_data(0x0C); //0C =12 The non-display period in lines between the start of the frame and the first display data in line.
//    write_cmd_data(0x00); //Set the vertical sync pulse width (LFRAME) in lines.
//    write_cmd_data(0x00);      //SET Vsync pulse start position
//    write_cmd_data(0x00);
    write_cmd_data(0x00);              // Set vert.Sync pulse start pos= 8    HB
    write_cmd_data(0x00);              // Set vert.Sync pulse start pos= 8    LB
    write_cmd_data(0x04);                   //

    write_command(0x36);
    write_cmd_data(0x01);

    // PWM signal frequency = PLL clock
    write_command(0xBE);
    write_cmd_data(0x08);
    write_cmd_data(0x80);
    write_cmd_data(0x01);

    write_command(0x0A);
    write_cmd_data(0x1C);         //Power Mode

    write_command(0xF0); //set pixel data format 8bit
    write_cmd_data(0x00);

    write_command(0x29);                    // Set display on

}
