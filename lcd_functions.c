#include <lcd_functions.h>

uint16_t offset = 0;

int16_t oldDiffSinResults[8][8];
int16_t oldDiffCosResults[8][8];

extern int16_t DiffSinResults[8][8];
extern int16_t DiffCosResults[8][8];
extern const char font_12_16[256][32];


/***************************  screen_show_nr()   ****************************/
// the display dimensions are 480*272 pixels => 130560 pixels               //
// the lcd memory is much bigger, so three display screens can be saved     //
// simultaneously there. screen_show_nr() switches between those screens.   //
/****************************************************************************/
void screen_show_nr(uint16_t nr)
{
    uint16_t value = nr * 272;

    write_command(0x37);                    // Set scroll area
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


/****************************  write_char()   *******************************/
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


/****************************************************************************/
// Writes the hole screen in one color                                      //
/****************************************************************************/
void write_screen_color(COLOR color)
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


/****************************************************************************/
void draw_display(void)
{
    int16_t m = 0, n = 0, xGrid, yGrid;

//    write_line(20, 50, 100, 80, (COLOR)0xff0000, WITH_ARROW);         // debug line
//    write_line(40, 200, 40, 50, (COLOR)0xff0000, WITH_ARROW);         // debug line


//     delete the old arrows
    for(xGrid = GRID_OFFSET_X; xGrid < ( 256 + GRID_OFFSET_X); xGrid += 32)
    {
        for(yGrid = 272 - GRID_OFFSET_Y; yGrid > GRID_OFFSET_Y; yGrid -= 32)
        {
            write_line(xGrid, yGrid, xGrid + oldDiffCosResults[m][n], yGrid - oldDiffSinResults[m][n], (COLOR)0xffffff, WITH_ARROW);
            m++;
        }
        m = 0;
        n++;
    }

    // write the new arrows
    n = 0;
    for(xGrid = GRID_OFFSET_X; xGrid < ( 256 + GRID_OFFSET_X); xGrid += 32)
    {
        for(yGrid = 272 - GRID_OFFSET_Y; yGrid > GRID_OFFSET_Y; yGrid -= 32)
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
    double gain;
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
                write_line(start_x, stop_y, start_x - 3, stop_y + 5, color, NO_ARROW);
                write_line(start_x, stop_y, start_x + 3, stop_y + 5, color, NO_ARROW);
            }
        }
        else
        {
            write_line_90_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_y > 1)
            {
                write_line(start_x, stop_y, start_x - 3, stop_y - 5, color, NO_ARROW);
                write_line(start_x, stop_y, start_x + 3, stop_y - 5, color, NO_ARROW);
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
                write_line(stop_x, stop_y, stop_x + 5, stop_y - 3, color, NO_ARROW);
                write_line(stop_x, stop_y, stop_x + 5, stop_y + 3, color, NO_ARROW);
            }
        }
        else
        {
            write_line_0_degree(start_x, start_y, stop_x, stop_y, color);
            if(arrowOption == WITH_ARROW && delta_x > 1)
            {
                write_line(stop_x, stop_y, stop_x - 5, stop_y - 3, color, NO_ARROW);
                write_line(stop_x, stop_y, stop_x - 5, stop_y + 3, color, NO_ARROW);
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
    else
    {
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
//                        write_line(stop_x, stop_y, stop_x - 5/gain, stop_y + gain/3, color, NO_ARROW); //unten
//                        write_line(stop_x, stop_y, stop_x - 5/gain, stop_y - 3/gain, color, NO_ARROW);
                    }
                }
                else                            // Quadrant I  2. (Steigung >= 1)
                {
                    gain = (double)delta_x / delta_y;
                    write_line_quadrant_1_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW && delta_x > 1)
                    {
                        // TODO: arrows
//                        write_line(stop_x, stop_y, stop_x - 5, stop_y - 3, color, NO_ARROW);
//                        write_line(stop_x, stop_y, stop_x - 5, stop_y + 3, color, NO_ARROW);
                    }
                }
            }

            else                                // Quadrant IV   (unterhalb von x-Achse)
            {
                if(delta_x > -delta_y)
                {
                    gain = (double)-delta_y / delta_x;       // start_y -> stop_y  ;  y--
                    write_line_quadrant_4_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                    }
                }
                else
                {
                    gain = (double)delta_x / -delta_y;
                    write_line_quadrant_4_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                    }
                }
            }
        }
/////////////////////////////////////////////////////////////////////////////////////////
        else     // x < 0                       // Quadrant 2 oder 3 (links von y-Achse)
        {
            if(start_y < stop_y)                // Quadrant 2 (oberhalb von x-Achse)
            {
                if(-delta_x > delta_y)  // gain > 1
                {
                    gain = (double)delta_y / -delta_x;
                    write_line_quadrant_2_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                    }
                }
                else
                {
                    gain = (double)-delta_x / delta_y;
                    write_line_quadrant_2_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                    }
                }
            }
            else // delta_y < 0
            {
                if(delta_x < delta_y)       // gain < -1
                {
                    gain = (double)delta_y / delta_x;
                    write_line_quadrant_3_I(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
                    }
                }
                else
                {
                    gain = (double)delta_x / delta_y;
                    write_line_quadrant_3_II(start_x, start_y, stop_x, stop_y, gain, color);
                    if(arrowOption == WITH_ARROW)
                    {
                        // TODO: arrows
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
