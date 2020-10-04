    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            //shiftleft1: multiplication by 2
            //differential: 0-1, 2-3, ... , 14-15
            negCosResults[m][n] = CosResults[m][(n << 1)];     // 0, 2, 4, .., 14
            negSinResults[m][n] = SinResults[m][(n << 1)];
            posCosResults[m][n] = CosResults[m][(n << 1) + 1]; // 1, 3, 5, .., 15
            posSinResults[m][n] = SinResults[m][(n << 1) + 1];

            CosOffset[m][n] = (negCosResults[m][n] + posCosResults[m][n]) >> 1;
            SinOffset[m][n] = (negSinResults[m][n] + posSinResults[m][n]) >> 1;

            SensorData.dCos[m][n] = negCosResults[m][n] - posCosResults[m][n];
            SensorData.dSin[m][n] = negSinResults[m][n] - posSinResults[m][n];

            // calculate arrow length
            SensorData.arrows.arrowLength[m][n] = (uint16_t) sqrt(
                    SensorData.dCos[m][n] * SensorData.dCos[m][n] +
                    SensorData.dSin[m][n] * SensorData.dSin[m][n] );

            // store length of longest arrow
            if(SensorData.arrows.arrowLength[m][n] > SensorData.maxAnalogValue)
            {
                SensorData.maxAnalogValue = SensorData.arrows.arrowLength[m][n];
            }
        }
    }
