/****************************  drawDisplay()  ***********************************/
// draws all arrows on the 7 inch LC-Display to a window on the LCD             //
// Window coordinates: start: (0,0) End: (506,479) [(x,y)]                      //
// 1. delete old arrow                                                          //
// 2. write cross grid                                                          //
// 3. Check, if new arrow is in window boundary. If not: scale down.            //
// 4. write new arrow                                                           //
/********************************************************************************/
void drawDisplay(lcdArrows * arrow)
{
    int16_t m, n;   // m = row , n = column
    static coordinates stop, oldStop[8][8];
    static bool oldArrowActive[8][8];
    bool arrowHeadActive;

    // write the arrows
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            // I: delete old arrow (paint it over with the background color)
            drawLine(startCord[m][n].x, startCord[m][n].y, oldStop[m][n].x,
                     oldStop[m][n].y, set->backColorArrow, oldArrowActive[m][n]);

            // II: write grid cross
            drawLine(startCord[m][n].x-2, startCord[m][n].y, startCord[m][n].x+2,
                     startCord[m][n].y, set->gridColor, NO_ARROWHEAD);
            drawLine(startCord[m][n].x, startCord[m][n].y-2, startCord[m][n].x,
                     startCord[m][n].y+2, set->gridColor, NO_ARROWHEAD);

            // III: check if new arrow is inside the display window. scale down
            // if not. Otherwise the arrows would paint over the menu.
            // Due to the LCD's internal memory layout it is only
            // necessary to check the x-axes.
            stop.x  = startCord[m][n].x + arrow->dCos[m][n];

            // all good: arrow is in window
            if( (stop.x >= 0) && (stop.x < 507) )
            {
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                arrowHeadActive = true;
            }
            // bad: arrow moved out off screen on left side
            // => scale stop.x and stop.y  down to left boundary
            else if(stop.x < 0)
            {
                stop.x  = 0;
                // minus because we know dCos < 0:
                arrow->dSin[m][n] *= (double)startCord[m][n].x / -arrow->dCos[m][n];
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                // disable arrowhead to indicate that the arrow was cropped
                arrowHeadActive = false;
            }
            // bad:  moved out off screen on right side => scale stop.y and stop.x
            // => scale down to right boundary
            else
            {
                // 506 is the x-coordinate from right boundary
                stop.x  = 506;
                arrow->dSin[m][n] *= (double)(stop.x-startCord[m][n].x) / arrow->dCos[m][n];
                stop.y  = startCord[m][n].y + arrow->dSin[m][n];
                // disable arrowhead to indicate that the arrow was cropped
                arrowHeadActive = false;
            }
            if(stop.y < 0 || stop.y > 479)
            {
                arrowHeadActive = false;
            }
            // if the option 'colored arrows' is enabled, the colors get linked to an
            // array with 768 different colors from dark blue to red.
            // if option is disabled colors get linked to an array with only 'black'.
            if( set->coloredArrows == true)
            {
                drawLine(startCord[m][n].x, startCord[m][n].y, stop.x, stop.y,
                         arrowColor[arrow->arrowLength[m][n]], arrowHeadActive);
            }
            else
            {
                drawLine(startCord[m][n].x, startCord[m][n].y, stop.x, stop.y,
                         set->arrowColor, arrowHeadActive);
            }
            oldStop[m][n].x = stop.x;
            oldStop[m][n].y = stop.y;
            oldArrowActive[m][n] = arrowHeadActive;
        }
    }
}
