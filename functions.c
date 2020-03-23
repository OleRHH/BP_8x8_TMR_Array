#include <functions.h>


/*****************************  # global variables #   ****************************/
//All ADC-Values as int16
int16_t SinResults[8][16];
int16_t CosResults[8][16];

int16_t DiffSinResults[8][8];
int16_t DiffCosResults[8][8];

int16_t DiffCosResultsDisp[8][8];
int16_t DiffSinResultsDisp[8][8];

int16_t negSinResults[8][8];
int16_t posSinResults[8][8];

int16_t negCosResults[8][8];
int16_t posCosResults[8][8];

int16_t SinOffset[8][8];
int16_t CosOffset[8][8];

uint32_t ADCValues_SS0[8];
uint32_t ADCValues_SS1[4];
uint32_t ADCValues_SS2[4];


/*********************************************************************************************/
//Read whole Array
void ReadArray(void)
{
    uint16_t i;

    GPIO_PORTL_DATA_R |= GPIO_PIN_4;
    for(i = 0; i <= 7; i++)                                 // read Cos-Values(= first 8 cycles)
    {
        // write address to the GPIO Pins for the analog multiplexer
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, i);


        SysCtlDelay(8);                                     // set-up time for multiplexer (~320 ns)

        GetADCValues();                                     // read current values

        //left half
        CosResults[0][7 - i] = (int16_t) ADCValues_SS0[0];
        CosResults[1][7 - i] = (int16_t) ADCValues_SS0[1];
        //array ordered backwards!
        CosResults[2][7 - i] = (int16_t) ADCValues_SS0[2];
        CosResults[3][7 - i] = (int16_t) ADCValues_SS0[3];
        CosResults[4][7 - i] = (int16_t) ADCValues_SS0[4];
        CosResults[5][7 - i] = (int16_t) ADCValues_SS0[5];
        CosResults[6][7 - i] = (int16_t) ADCValues_SS0[6];
        CosResults[7][7 - i] = (int16_t) ADCValues_SS0[7];
        //right half
        CosResults[0][15 - i] = (int16_t) ADCValues_SS1[0];
        CosResults[1][15 - i] = (int16_t) ADCValues_SS1[1];
        CosResults[2][15 - i] = (int16_t) ADCValues_SS1[2];
        CosResults[3][15 - i] = (int16_t) ADCValues_SS1[3];

        CosResults[4][15 - i] = (int16_t) ADCValues_SS2[0];
        CosResults[5][15 - i] = (int16_t) ADCValues_SS2[1];
        CosResults[6][15 - i] = (int16_t) ADCValues_SS2[2];
        CosResults[7][15 - i] = (int16_t) ADCValues_SS2[3];
    }

    GPIO_PORTL_DATA_R &= ~GPIO_PIN_4;
    for(i = 8; i <= 15; i++)                                //read Sin-Values(=second 8 cycles)
    {
        // write address to the GPIO Pins for MU
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, i);

        SysCtlDelay(8);                                     // set-up time for multiplexer (~320 ns)

        GetADCValues();                                     // read current values

        // left half
        SinResults[0][i - 8] = (int16_t) ADCValues_SS0[0]; // assign Values to result
        SinResults[1][i - 8] = (int16_t) ADCValues_SS0[1]; // array ordered forwards!
        SinResults[2][i - 8] = (int16_t) ADCValues_SS0[2];
        SinResults[3][i - 8] = (int16_t) ADCValues_SS0[3];
        SinResults[4][i - 8] = (int16_t) ADCValues_SS0[4];
        SinResults[5][i - 8] = (int16_t) ADCValues_SS0[5];
        SinResults[6][i - 8] = (int16_t) ADCValues_SS0[6];
        SinResults[7][i - 8] = (int16_t) ADCValues_SS0[7];
        //right half
        SinResults[0][i] = (int16_t) ADCValues_SS1[0];
        SinResults[1][i] = (int16_t) ADCValues_SS1[1];
        SinResults[2][i] = (int16_t) ADCValues_SS1[2];
        SinResults[3][i] = (int16_t) ADCValues_SS1[3];

        SinResults[4][i] = (int16_t) ADCValues_SS2[0];
        SinResults[5][i] = (int16_t) ADCValues_SS2[1];
        SinResults[6][i] = (int16_t) ADCValues_SS2[2];
        SinResults[7][i] = (int16_t) ADCValues_SS2[3];
    }
}


/*********************************************************************************************/
// Get ADC Values
void GetADCValues(void)
{
	// Trigger the sample sequence
	ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);

    while(!ADCIntStatus(ADC1_BASE, 2, false));

    // Quit interrupt
    ADCIntClear(ADC1_BASE, 2);

    // Read the values from the ADC and store them in the arrays ADCValues_SSX
    ADCSequenceDataGet(ADC0_BASE, 0, ADCValues_SS0);
    ADCSequenceDataGet(ADC1_BASE, 1, ADCValues_SS1);
    ADCSequenceDataGet(ADC1_BASE, 2, ADCValues_SS2);
}


/*********************************************************************************************/
//Compute differential signal

void Computations(void)
{
    int32_t max = 0, absolute;
	uint16_t m, n;

    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            //shiftleft1: multiplication by 2
            //differential: 0-1, 2-3, ... , 14-15
             negCosResults[m][n] = CosResults[m][(n << 1)];                     // 0, 2, 4, ..., 14
             posCosResults[m][n] = CosResults[m][(n << 1) + 1];                 // 1, 3, 5, ..., 15
            DiffCosResults[m][n] =  negCosResults[m][n] - posCosResults[m][n];
                 CosOffset[m][n] = (negCosResults[m][n] + posCosResults[m][n]) >> 1;
             negSinResults[m][n] = SinResults[m][(n << 1)];
             posSinResults[m][n] = SinResults[m][(n << 1) + 1];
            DiffSinResults[m][n] =  negSinResults[m][n] - posSinResults[m][n];
                 SinOffset[m][n] = (negSinResults[m][n] + posSinResults[m][n]) >> 1;

            // berechne die Betragsquadrate:
            absolute = DiffCosResults[m][n]*DiffCosResults[m][n] + DiffSinResults[m][n]*DiffSinResults[m][n];

            // finde den größten Betragsquadrat und speicher ihn
            if(absolute > max)
            {
                max = absolute;
            }
        }
    }
    // Berechne aus dem ermittelten größten Betragsquadrat den Betrag
    max = sqrt(max);
    // Normalisiere den Betrag auf +- 16 (das ist die Breite des Rasters vom Array)
    max >>= 6;

    // normalisiere alle anderen Vektoren
    for(m = 0; m <= 7; m++)
    {
        for(n = 0; n <= 7; n++)
        {
            DiffCosResults[m][n] = DiffCosResults[m][n] / (int16_t)max;
            DiffSinResults[m][n] = DiffSinResults[m][n] / (int16_t)max;
        }
    }
}
