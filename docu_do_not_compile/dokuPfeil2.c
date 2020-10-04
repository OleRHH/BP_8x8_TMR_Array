[...]
 int16_t delta_x = stop_x - start_x;
 int16_t delta_y = stop_y - start_y;
[...]
else
{
    double gain, angle;
    // quadrant I or IV
    if(start_x < stop_x)
    {
        // quadrant I
        if(start_y < stop_y)
        {
            // quadrant I  1. (gain < 1)
            if(delta_x > delta_y)
            {
                gain = (double)delta_y / delta_x;
                writeLineQuadrant1_I(start_x, start_y, stop_x, stop_y, gain, color);

                if(arrowOption == WITH_ARROW & delta_x > MIN_LENGTH_FOR_ARROW)
                {
                    angle = atan2(delta_y, delta_x);
                    // upper arrow line
                    drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle - 2.5),
                    stop_y + ARROW_LENGTH*sin(angle - 2.5), color, NO_ARROWHEAD);

                    // lower arrow line
                    drawLine(stop_x, stop_y, stop_x + ARROW_LENGTH*cos(angle + 2.5),
                    stop_y + ARROW_LENGTH*sin(angle + 2.5), color, NO_ARROWHEAD);
                }
            }
[...]

/********************************************************************************/
void writeLineQuadrant1_I(short start_x, short start_y,
                          short stop_x, short stop_y, double gain,color color)
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
