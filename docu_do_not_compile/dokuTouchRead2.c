/***********************  touchGetArrowLength()  ********************************/
/* When the 'max arrow length' option is activated via touch input, a window is */
/* generated with 4x4 (row x column) fields to choose from. Possible values are */
/* 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 and 160.    */
/* They define the maximum arrow length in pixels and also represent the actual */
/* measured analog value from the MCU internal 12 bit AD-converter.             */
/* There are two display modes available: relative and absolute.                */
/* In relative mode the highest value from the last analog sensor array measure-*/
/* ment (64 sensors) gets scaled to the here defined pixel length and all       */
/* other arrows get scaled from 0 to this value.                                */
/* In absolute mode the arrows are not scaled but can only be as long as        */
/* defined here. If there are actual larger measured valued the word "clipping" */
/* apears on the display as an information.                                     */
/* This function returns the value that was picked from the window.             */
/* To stabilize the x,y coordinates the analog values get averaged while        */
/* pressing the touch display. Only if more than 200 measurement where taken    */
/* (this only takes some ms) the touch input is valid.                          */
/********************************************************************************/
uint16_t touchGetArrowLength(uint16_t oldValue)
{
    uint32_t xposSum = 0, yposSum = 0;
    uint16_t pos = 0, m = 0, n = 0;     // m = row , n = column

    SysCtlDelay(1000);

    // menu can only be left with a valid touch. A touch is only valid
    // if pos gets summed up to >= 200
    while( pos <= 200)
    {
        pos = 0;
        // sum up measurements. break after 1000 loops or if touch is released.
        while( !(GPIO_PORTF_AHB_DATA_R & TIRQ) && pos < 1000)
        {
            xposSum += touchRead(0xD0);
            yposSum += touchRead(0x90);
            SysCtlDelay(100);               // wait
            pos++;
        }
    }

    // calculate the mean values.
    xposSum /= pos;
    yposSum /= pos;
    xpos = xposSum; // read xpos
    ypos = yposSum; // read ypos

    // first check if window was hit at all. Else return old value.
    // upper left corner: x = 4095, y = 4095
    if(xposSum > 3725 || xposSum < 1760 || yposSum > 3600 || yposSum < 490)
        return oldValue;
    // now check what column (n) was hit
    else if(xposSum > 3240)
        n = 0;
    else if(xposSum > 2740)
        n = 1;
    else if(xposSum > 2255)
        n = 2;
    else if(xposSum > 1760)
        n = 3;

    // now check what row (m) was hit
    if(yposSum > 2830)
        m = 0;
    else if(yposSum > 2040)
        m = 1;
    else if(yposSum > 1230)
        m = 2;
    else if(yposSum > 490)
        m = 3;

    // each row and column has its own values. The actual value is returned.
    return 10 + m*40 + n*10;    // 10 - 160
}
