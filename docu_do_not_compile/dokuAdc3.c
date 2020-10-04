if(relative == true)    // scaling: relative
{
    // diff.dSin and diff.dSin are needed to display the arrows.
    // They are being normalized in this function to the maximum arrow length.
    // todo:
    // diff.dSin has (-)sign because the LC-Display is turned upside down.
    // (this is for historic reasons and should be improved in the future.)
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            SensorData.arrows.dCos[m][n]  = (int16_t) ( SensorData.dCos[7-m][n]
                                            * maxArrowLength / SensorData.maxAnalogValue);
            SensorData.arrows.dSin[m][n]  = (int16_t) (-SensorData.dSin[7-m][n]
                                            * maxArrowLength / SensorData.maxAnalogValue);
        }
    }
}
else    // scaling: absolute
{
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            SensorData.arrows.dCos[m][n] =  SensorData.dCos[7-m][n];
            SensorData.arrows.dSin[m][n] = -SensorData.dSin[7-m][n];

            // limit the maximum arrow length to the max allowed value.
            if(SensorData.arrows.arrowLength[7-m][n] > maxArrowLength)
            {
                SensorData.arrows.dCos[m][n] *= (double)maxArrowLength
                                                / SensorData.arrows.arrowLength[7-m][n];
                SensorData.arrows.dSin[m][n] *= (double)maxArrowLength
                                                / SensorData.arrows.arrowLength[7-m][n];
            }
        }
    }
}
