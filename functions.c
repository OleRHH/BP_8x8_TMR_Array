#include <functions.h>
#include <lcd_functions.h>


/*****************************  # global variables #   ****************************/
//All ADC-Values as int16
int16_t SinResults[8][16];
int16_t CosResults[8][16];

int16_t DiffSinResults[8][8];
int16_t DiffCosResults[8][8];

int16_t DiffCosResultsDisp[8][8];
int16_t DiffSinResultsDisp[8][8];

int16_t SinOffset[8][8];
int16_t CosOffset[8][8];

int16_t DiffResults[2][8][8];   // 256 bytes array for transmit via RS232

int16_t v_length[8][8];
/*********************************************************************************************/
//Read whole Array
void ReadArray(uint16_t step)
{
    uint32_t ADCValues_SS0[8];
    uint32_t ADCValues_SS1[4];
    uint32_t ADCValues_SS2[4];

    // Read the values from the ADC and store them in the arrays ADCValues_SSX
    ADCSequenceDataGet(ADC0_BASE, 0, ADCValues_SS0);
    ADCSequenceDataGet(ADC1_BASE, 1, ADCValues_SS1);
    ADCSequenceDataGet(ADC1_BASE, 2, ADCValues_SS2);

    if (step < 8)                                 // read Cos-Values(= first 8 cycles)
    {
        //left half
        CosResults[0][7 - step] = (int16_t) ADCValues_SS0[0];
        CosResults[1][7 - step] = (int16_t) ADCValues_SS0[1];
        //array ordered backwards!
        CosResults[2][7 - step] = (int16_t) ADCValues_SS0[2];
        CosResults[3][7 - step] = (int16_t) ADCValues_SS0[3];
        CosResults[4][7 - step] = (int16_t) ADCValues_SS0[4];
        CosResults[5][7 - step] = (int16_t) ADCValues_SS0[5];
        CosResults[6][7 - step] = (int16_t) ADCValues_SS0[6];
        CosResults[7][7 - step] = (int16_t) ADCValues_SS0[7];
        //right half
        CosResults[0][15 - step] = (int16_t) ADCValues_SS1[0];
        CosResults[1][15 - step] = (int16_t) ADCValues_SS1[1];
        CosResults[2][15 - step] = (int16_t) ADCValues_SS1[2];
        CosResults[3][15 - step] = (int16_t) ADCValues_SS1[3];

        CosResults[4][15 - step] = (int16_t) ADCValues_SS2[0];
        CosResults[5][15 - step] = (int16_t) ADCValues_SS2[1];
        CosResults[6][15 - step] = (int16_t) ADCValues_SS2[2];
        CosResults[7][15 - step] = (int16_t) ADCValues_SS2[3];
    }

    else                                //read Sin-Values(=second 8 cycles)
    {
         // left half
        SinResults[0][step - 8] = (int16_t) ADCValues_SS0[0]; // assign Values to result
        SinResults[1][step - 8] = (int16_t) ADCValues_SS0[1]; // array ordered forwards!
        SinResults[2][step - 8] = (int16_t) ADCValues_SS0[2];
        SinResults[3][step - 8] = (int16_t) ADCValues_SS0[3];
        SinResults[4][step - 8] = (int16_t) ADCValues_SS0[4];
        SinResults[5][step - 8] = (int16_t) ADCValues_SS0[5];
        SinResults[6][step - 8] = (int16_t) ADCValues_SS0[6];
        SinResults[7][step - 8] = (int16_t) ADCValues_SS0[7];
        //right half
        SinResults[0][step] = (int16_t) ADCValues_SS1[0];
        SinResults[1][step] = (int16_t) ADCValues_SS1[1];
        SinResults[2][step] = (int16_t) ADCValues_SS1[2];
        SinResults[3][step] = (int16_t) ADCValues_SS1[3];

        SinResults[4][step] = (int16_t) ADCValues_SS2[0];
        SinResults[5][step] = (int16_t) ADCValues_SS2[1];
        SinResults[6][step] = (int16_t) ADCValues_SS2[2];
        SinResults[7][step] = (int16_t) ADCValues_SS2[3];
    }
}


/*********************************************************************************************/
//Compute differential signal

uint32_t compute_relative(uint16_t maxArrowSize)
{
    int16_t negSinResults[8][8];
    int16_t posSinResults[8][8];

    int16_t negCosResults[8][8];
    int16_t posCosResults[8][8];

    int16_t maxAnalogValue = 1;
    uint16_t m, n;

    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            //shiftleft1: multiplication by 2
            //differential: 0-1, 2-3, ... , 14-15
            negCosResults[m][n]  = CosResults[m][(n << 1)];                     // 0, 2, 4, ..., 14
            posCosResults[m][n]  = CosResults[m][(n << 1) + 1];                 // 1, 3, 5, ..., 15
            DiffResults[1][m][n] = negCosResults[m][n] - posCosResults[m][n];
            CosOffset[m][n]      = (negCosResults[m][n] + posCosResults[m][n]) >> 1;
            negSinResults[m][n]  = SinResults[m][(n << 1)];
            posSinResults[m][n]  = SinResults[m][(n << 1) + 1];
            DiffResults[0][m][n] = negSinResults[m][n] - posSinResults[m][n];
            SinOffset[m][n]      = (negSinResults[m][n] + posSinResults[m][n]) >> 1;

            // berechne die Betragsquadrate:
            v_length[m][n] = (uint16_t) sqrt(DiffResults[1][m][n]*DiffResults[1][m][n] +
                                DiffResults[0][m][n]*DiffResults[0][m][n]);

            // finde den größten Betragsquadrat und speicher ihn
            if(v_length[m][n] > maxAnalogValue)
            {
                maxAnalogValue = v_length[m][n];
            }
        }
    }
    // normalisiere alle anderen Vektoren
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            DiffCosResults[m][n] = DiffResults[1][m][n] * maxArrowSize / maxAnalogValue;
            DiffSinResults[m][n] = DiffResults[0][m][n] * maxArrowSize / maxAnalogValue;
        }
    }
    return maxAnalogValue;
}
/*********************************************************************************************/
uint32_t compute_absolute(uint16_t maxArrowSize)
{
    int16_t negSinResults[8][8];
    int16_t posSinResults[8][8];

    int16_t negCosResults[8][8];
    int16_t posCosResults[8][8];

    int32_t maxAnalogValue = 1;
    uint16_t m, n;

    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            //shiftleft1: multiplication by 2
            //differential: 0-1, 2-3, ... , 14-15
            negCosResults[m][n] = CosResults[m][(n << 1)];                     // 0, 2, 4, ..., 14
            posCosResults[m][n] = CosResults[m][(n << 1) + 1];                 // 1, 3, 5, ..., 15
            DiffResults[1][m][n] = negCosResults[m][n] - posCosResults[m][n];
            CosOffset[m][n] = (negCosResults[m][n] + posCosResults[m][n]) >> 1;
            negSinResults[m][n] = SinResults[m][(n << 1)];
            posSinResults[m][n] = SinResults[m][(n << 1) + 1];
            DiffResults[0][m][n] = negSinResults[m][n] - posSinResults[m][n];
            SinOffset[m][n] = (negSinResults[m][n] + posSinResults[m][n]) >> 1;

            // berechne die Betragsquadrate:
            v_length[m][n] = (uint16_t) sqrt(DiffResults[1][m][n]*DiffResults[1][m][n] +
                                DiffResults[0][m][n]*DiffResults[0][m][n]);

            if(v_length[m][n] > maxAnalogValue)
            {
                maxAnalogValue = v_length[m][n];
            }
            // limit the maximum arrow length to the max allowed value (maxArrowSize)
            if(v_length[m][n] > maxArrowSize)
            {
                DiffResults[1][m][n] *= maxArrowSize;
                DiffResults[1][m][n] /= v_length[m][n];
                DiffResults[0][m][n] *= maxArrowSize;
                DiffResults[0][m][n] /= v_length[m][n];
            }
        }
    }

    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            DiffCosResults[m][n] = DiffResults[1][m][n];
            DiffSinResults[m][n] = DiffResults[0][m][n];
        }
    }
    return maxAnalogValue;
}
