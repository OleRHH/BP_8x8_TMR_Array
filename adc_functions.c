#include <adc_functions.h>


/***************************  # global variables #   ****************************/
// All ADC-Values. Temporarily stored here while ad conversion in progress.
static int16_t SinResults[8][16];
static int16_t CosResults[8][16];



/********************************************************************************/
void startAdcConversion(void)
{
    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);
}


/********************************************************************************/
void adcIntClear(void)
{
    ADCIntClear(ADC1_BASE, 2);
}


/********************************************************************************/
bool getADCHardwareAveraging(char command)
{
    bool status = false;

    if(command == '1')
    {
        // set hardware adcAVG for better resolution
        ADCHardwareOversampleConfigure(ADC0_BASE, 64);
        ADCHardwareOversampleConfigure(ADC1_BASE, 64);
        status = true;
    }
    return status;
}


/********************************************************************************/
//Read TMR sensor array
void storeArraySensorData(uint16_t step)
{
    uint32_t ADCValues_SS0[8];
    uint32_t ADCValues_SS1[4];
    uint32_t ADCValues_SS2[4];

    // Read the values from the ADC and store them in the arrays ADCValues_SSX
    ADCSequenceDataGet(ADC0_BASE, 0, ADCValues_SS0);
    ADCSequenceDataGet(ADC1_BASE, 1, ADCValues_SS1);
    ADCSequenceDataGet(ADC1_BASE, 2, ADCValues_SS2);

    if (step < 8)                         // read Cos-Values(= first 8 cycles)
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

    else                                //read Sin-Values( = 2nd 8 cycles)
    {
         // left half. Assign Values to result array ordered forwards!
        SinResults[0][step - 8] = (int16_t) ADCValues_SS0[0];
        SinResults[1][step - 8] = (int16_t) ADCValues_SS0[1];
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


/***********************  computeArrows() ***************************************/
/* Compute differential signal                                                  */
/********************************************************************************/
uint32_t computeArrows(bool relative, uint16_t maxArrowLength)
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
            negCosResults[m][n]  = CosResults[m][(n << 1)];     // 0, 2, 4, .., 14
            posCosResults[m][n]  = CosResults[m][(n << 1) + 1]; // 1, 3, 5, .., 15
            DiffResults[1][m][n] = negCosResults[m][n] - posCosResults[m][n];
            CosOffset[m][n]      = (negCosResults[m][n] + posCosResults[m][n]) >> 1;
            negSinResults[m][n]  = SinResults[m][(n << 1)];
            posSinResults[m][n]  = SinResults[m][(n << 1) + 1];
            DiffResults[0][m][n] = negSinResults[m][n] - posSinResults[m][n];
            SinOffset[m][n]      = (negSinResults[m][n] + posSinResults[m][n]) >> 1;

            // calculate arrow length
            arrowLength[m][n] = (uint16_t) sqrt(
                                    DiffResults[1][m][n]*DiffResults[1][m][n] +
                                    DiffResults[0][m][n]*DiffResults[0][m][n] );

            // store length of longest arrow
            if(arrowLength[m][n] > maxAnalogValue)
            {
                maxAnalogValue = arrowLength[m][n];
            }
        }
    }

    if(relative == true)
    {
        // DiffCosResults and DiffSinResults are needed to display the arrows.
        // They are being normalized in this function to the maximum arrow length.
        // todo:
        // DiffSinResults has (-)sign because the LC-Display is turned upside down.
        // (this is for historic reasons and should be improved in the future.)
        for(m = 0; m <= 7; m++)
        {
            for(n = 0; n <= 7; n++)
            {
                DiffCosResults[m][n]  = DiffResults[1][7-m][n] * maxArrowLength;
                DiffCosResults[m][n] /= maxAnalogValue;
                DiffSinResults[m][n]  =-DiffResults[0][7-m][n] * maxArrowLength;
                DiffSinResults[m][n] /= maxAnalogValue;
            }
        }
    }
    else
    {
        for(m = 0; m <= 7; m++)
        {
            for(n = 0; n <= 7; n++)
            {
                DiffCosResults[m][n] =  DiffResults[1][7-m][n];
                DiffSinResults[m][n] = -DiffResults[0][7-m][n];

                // limit the maximum arrow length to the max allowed value.
                if(arrowLength[7-m][n] > maxArrowLength)
                {
                    DiffCosResults[m][n] *=  maxArrowLength;
                    DiffCosResults[m][n] /=  arrowLength[7-m][n];
                    DiffSinResults[m][n] *= maxArrowLength;
                    DiffSinResults[m][n] /= arrowLength[7-m][n];
                }
            }
        }
    }

    return maxAnalogValue;
}


/***************************  ConfigureADC()  ***********************************/
/* The sensor array has 64 TMR-Sensors with four analog signals each (sin+,     */
/* sin-, cos+, cos-). This makes in total 256 analog signals to be measured.    */
/* They are multiplexed to 16 analog inputs.                                    */
/* So there are 16 analog inputs to be measured simultaneously                  */
/********************************************************************************/
void ConfigureADC(void)
{
    // enable clock for peripheries
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Wait for the ADC0 module to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1));

    // set hardware adcAVG for better resolution
    ADCHardwareOversampleConfigure(ADC0_BASE, 64);
    ADCHardwareOversampleConfigure(ADC1_BASE, 64);

    // ADC, sample sequencer, trigger processor, priority
    ADCSequenceConfigure(ADC0_BASE,0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC1_BASE,1, ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceConfigure(ADC1_BASE,2, ADC_TRIGGER_PROCESSOR, 2);

    // Samplesequencer0, left side of array
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ROW_1_L); // sequence0,step0
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ROW_2_L); // sequence0,step1
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ROW_3_L); // sequence0,step2
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ROW_4_L); // sequence0,step3
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ROW_5_L); // sequence0,step4
    ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ROW_6_L); // sequence0,step5
    ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ROW_7_L); // sequence0,step6
    ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ROW_8_L); // sequence0,step7

    // Sample sequencer 1, upper right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 1, 0, ROW_1_R); // sequence1,step0
    ADCSequenceStepConfigure(ADC1_BASE, 1, 1, ROW_2_R); // sequence1,step1
    ADCSequenceStepConfigure(ADC1_BASE, 1, 2, ROW_3_R); // sequence1,step2
    ADCSequenceStepConfigure(ADC1_BASE, 1, 3, ROW_4_R); // sequence1,step3

    // Sample sequencer 2, lower right side of the array
    ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ROW_5_R); // sequence2,step0
    ADCSequenceStepConfigure(ADC1_BASE, 2, 1, ROW_6_R); // sequence2,step1
    ADCSequenceStepConfigure(ADC1_BASE, 2, 2, ROW_7_R); // sequence2,step2
    ADCSequenceStepConfigure(ADC1_BASE, 2, 3, ROW_8_R | // sequence2,step3
                             ADC_CTL_IE|ADC_CTL_END);   // incl.interrupt


    // Enable ADC
    ADCSequenceEnable(ADC0_BASE, 0);        // ADC0 for sample sequencer0
    ADCSequenceEnable(ADC1_BASE, 1);        // ADC1 for sample sequencer1
    ADCSequenceEnable(ADC1_BASE, 2);        // ADC1 for sample sequencer2


    IntPrioritySet(INT_ADC0SS2, HIGH_PRIORITY);           // set priority
    ADCIntRegister(ADC1_BASE, 2, ADC1IntHandler);
    ADCIntEnable(ADC1_BASE, 2);
    IntEnable(INT_ADC1SS2);

}
