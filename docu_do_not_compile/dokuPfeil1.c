[...]
// 90 or 270 degree line
if (start_x == stop_x)
{
    // 270 degree line
    if (start_y > stop_y)
    {
        writeLine270Degree(start_x, start_y, stop_x, stop_y, color);

        if(arrowOption == WITH_ARROW && -delta_y > 1)
        {
            drawLine(start_x, stop_y, start_x - ARROW_LENGTH*sin(ARROW_ANGLE),
                       stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROWHEAD);
            drawLine(start_x, stop_y, start_x + ARROW_LENGTH*sin(ARROW_ANGLE),
                       stop_y + ARROW_LENGTH*cos(ARROW_ANGLE), color, NO_ARROWHEAD);
        }
    }
    // 90 degree line
    else
[...]

 /********************************************************************************/
void writeLine270Degree(short start_x, short start_y,
                        short stop_x, short stop_y, color color)
{
    int16_t y;

    writePosition(start_x, stop_y, start_x, start_y);
    writeCommand(0x2C);
    for (y = stop_y; y <= start_y; y++)
    {
        writeData(color);
    }
}
