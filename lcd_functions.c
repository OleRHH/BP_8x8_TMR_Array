#include <lcd_functions.h>
#include <fonts.h>

uint16_t offset = 0;


/***************************  screen_show_nr()   *****************************/
// the display dimensions are 480*272 pixels => 130560 pixels               //
// the lcd memory is much bigger, so three display screens can be saved     //
// simultaneously there. screen_show_nr() switches between those screens.   //
/****************************************************************************/
void screen_show_nr(short nr) {
    short wert = nr * 272;
    write_command(0x37);                    // Set scroll area
    write_cmd_data(wert >> 8);              // TFA high byte (TFA = 0)
    write_cmd_data(wert);
}

/***************************  screen_show_nr()   *****************************/
// the display dimensions are 480*272 pixels => 130560 pixels               //
// the lcd memory is much bigger, so three display screens can be saved     //
// simultaneously there. screen_show_nr() switches between those screens.   //
/****************************************************************************/
void screen_write_nr(short nr) {
    offset = nr * 272;
}

/****************************  write_char()   *******************************/
// writes a single letter with heigth 12 pixel                              //
/****************************************************************************/
void write_char(int w, COLOR color, COLOR backcolor) {
    int lv;
    for (lv = 0; lv < 12; lv++) {
        if (w & 1) {             // Write data byte
            GPIO_PORTM_DATA_R = color.red;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = color.green;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = color.blue;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        } else {
            GPIO_PORTM_DATA_R = backcolor.red;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = backcolor.green;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
            GPIO_PORTM_DATA_R = backcolor.blue;
            GPIO_PORTQ_DATA_R = 0x15;        // Chip select = 0, Write state = 0
            GPIO_PORTQ_DATA_R = 0x1F;               // Initial state
        }
        w = w >> 1;
    }
}

/******************************************************************************************************/
void print_string(char *text, int row, int column, COLOR color, COLOR backcolor) {
    int w;
    int lv1, numChar;
    int length = strlen(text);
    int font_width = FONT_WIDTH_BIG;
    int font_hight = FONT_HIGHT_BIG;
    int columnStart = column;
    int columnStop = columnStart + font_width - 1;
    int rowStart = row;
    int rowStop = rowStart + font_hight - 1;

    for (numChar = 0; numChar < length; numChar++) {
        write_position(columnStart, rowStart, columnStop, rowStop);
        write_command(0x2C);
        for (lv1 = 0; lv1 < 32; lv1 = lv1 + 2) {
            w = (font_12_16[text[numChar]][lv1 + 1] << 4)
                    | (font_12_16[text[numChar]][lv1] >> 4);
            write_char(w, color, backcolor);
        }
        columnStart += font_width;
        columnStop += font_width;
    }
}


/******************************************************************************************************/
//Writes the hole screen in one color
void write_screen_color(COLOR color) {
    int count = 0;
//    COLOR color = (COLOR)0xFF0000;
    write_position(0, 0, 479, 271);
    write_command(0x2C);

    while (count++ < 130560) {
        GPIO_PORTM_DATA_R = color.red;              // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;            // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;               // Initial state

        GPIO_PORTM_DATA_R = color.green;            // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state

        GPIO_PORTM_DATA_R = color.blue;              // Write data byte
        GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
        GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
    }
}

/******************************************************************************************************/
void write_command(unsigned char command) {
    GPIO_PORTM_DATA_R = command;        // Write command byte
    GPIO_PORTQ_DATA_R = 0x11; // Chip select = 0, Command mode select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}

/******************************************************************************************************/
void write_cmd_data(unsigned char data) {
    GPIO_PORTM_DATA_R = data;           // Write data byte
    GPIO_PORTQ_DATA_R = 0x15;           // Chip select = 0, Write state = 0
    GPIO_PORTQ_DATA_R = 0x1F;           // Initial state
}

/******************************************************************************************************/
void write_data(COLOR color) {
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
void write_position(short point1_x, short point1_y, short point2_x, short point2_y) {

	// offset (+ 1x272, + 2x272, + 3x272) points to screen 1, 2 or 3
    point1_y += offset;
    point2_y += offset;

    write_command(0x2A);                    // Set page address (x-axis)
    write_cmd_data(point1_x >> 8);   // Set start page address                HB
    write_cmd_data(point1_x);        //                                       LB
    write_cmd_data(point2_x >> 8);   // Set display_stop page address                 HB
    write_cmd_data(point2_x);        //                                       LB

    write_command(0x2B);                    // Set column address (y-axis)
    write_cmd_data(point1_y >> 8);   // Set start column address              HB
    write_cmd_data(point1_y);        //                                       LB
    write_cmd_data(point2_y >> 8);   // Set display_stop column address               HB
    write_cmd_data(point2_y);        //                                       LB
}

