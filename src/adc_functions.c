/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: adc_functions.c                                                       */
/********************************************************************************/
#include <adc_functions.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>      // ADCIntClear(), ADCIntStatus(),
                                // ADCProcessorTrigger(), ADCSequenceDataGet()
#include <inc/hw_memmap.h>      // ADC0_BASE, ADC1_BASE, GPIO_PORTX_BASE
#include <tm4c1294ncpdt.h>
#include <math.h>               // sqrt()


/*****************************  # defines #   ***********************************/
// defines for ADC init             // Port and Pin
#define ROW_1_R ADC_CTL_CH13        // PD2
#define ROW_2_R ADC_CTL_CH15        // PD0
#define ROW_3_R ADC_CTL_CH14        // PD1
#define ROW_4_R ADC_CTL_CH12        // PD3
#define ROW_5_R ADC_CTL_CH5         // PD6
#define ROW_6_R ADC_CTL_CH4         // PD7
#define ROW_7_R ADC_CTL_CH7         // PD4
#define ROW_8_R ADC_CTL_CH6         // PD5
#define ROW_8_L ADC_CTL_CH19        // PK3
#define ROW_7_L ADC_CTL_CH18        // PK2
#define ROW_6_L ADC_CTL_CH17        // PK1
#define ROW_5_L ADC_CTL_CH16        // PK0
#define ROW_4_L ADC_CTL_CH8         // PE5
#define ROW_3_L ADC_CTL_CH9         // PE4
#define ROW_2_L ADC_CTL_CH0         // PE3
#define ROW_1_L ADC_CTL_CH1         // PE2
#define GPIO_PIN_3_DOWNTO_0 0x0F    // adcInterruptHandler()

// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define MID_PRIORITY_3 0x60


/***************************  # global variables #   ****************************/
// All ADC-Values. Temporarily stored here while ADC in progress.
static int16_t SinResults[8][16];   // 128 bytes
static int16_t CosResults[8][16];   // 128 bytes

// These offset values are being computed but are not used at the moment.
// They are here for historic reasons and to be used again in the future.
// TODO: use :)
int16_t SinOffset[8][8];
int16_t CosOffset[8][8];

// Struct to hold all sensor data used in other functions
TMRSensorData SensorData;


/***************************  startADConversion()  ******************************/
/* This function has been outsourced for better readability. It simply starts   */
/* a ad-conversion sequence.                                                    */
/********************************************************************************/
void startADConversion(void)
{
    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC1_BASE, 1);
    ADCProcessorTrigger(ADC1_BASE, 2);
}


/***************************  adcIntClear()  ************************************/
/* Clear all pending adc interrupts. This is outsourced because the needed      */
/* include file is declared here.                                               */
/********************************************************************************/
void adcIntClear(void)
{
    ADCIntClear(ADC1_BASE, 2);
}


/***************************  setMultiplexer()  *********************************/
/* Sets the addresses for the analog multiplexers.                              */
/* Port L is used to address the analog multiplexers on the TMR sensor array.   */
/* GPIO_PIN_4 is inverted after half the measures (Step 0 - 7, measure 0 - 127) */
/* are done. This is because of the sensor-array hardware layout.               */
/********************************************************************************/
void setMultiplexer(uint16_t step)
{
    if(step == 0)
    {
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_PIN_4);
    }
    if(step == 8)
    {
        GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
    }
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3_DOWNTO_0, step);
}


/***********************  setADCHardwareAveraging()  ****************************/
/* Toggles the hardware averaging feature.                                      */
/********************************************************************************/
bool setADCHardwareAveraging(bool adcAVG)
{
    if(adcAVG == true)
    {
    // disable hardware adcAVG for better resolution.
        ADCHardwareOversampleConfigure(ADC0_BASE, 1);
        ADCHardwareOversampleConfigure(ADC1_BASE, 1);
        adcAVG = false;
    }
    else
    {
    // enable hardware adcAVG for better speed.
        ADCHardwareOversampleConfigure(ADC0_BASE, 64);
        ADCHardwareOversampleConfigure(ADC1_BASE, 64);
        adcAVG = true;
    }

    return adcAVG;
}


/***********************  storeArraySensorData()  ****************************/
/* After converting all 256 analog signals from sensor-array this function                 */
/********************************************************************************/
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
/* Compute differential signals.                                                */
/********************************************************************************/
void computeArrows(bool relative, uint16_t maxArrowLength)
{
    static int16_t negSinResults[8][8];
    static int16_t posSinResults[8][8];

    static int16_t negCosResults[8][8];
    static int16_t posCosResults[8][8];

    uint16_t m, n;

    SensorData.maxAnalogValue = 1;

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
}


/***************************  configureADC()  ***********************************/
/* The sensor array has 64 TMR-Sensors with four analog signals each (sin+,     */
/* sin-, cos+, cos-). This makes in total 256 analog signals to be measured.    */
/* They are multiplexed to 16 analog inputs.                                    */
/* So there are 16 analog inputs to be measured simultaneously                  */
/********************************************************************************/
TMRSensorData * configureADC(bool adcAVG)
{
    // enable clock for peripheries
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Wait for the ADC0 module to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1));

    if(adcAVG == true)
    {
        // enable hardware adcAVG for better resolution.
        ADCHardwareOversampleConfigure(ADC0_BASE, 64);
        ADCHardwareOversampleConfigure(ADC1_BASE, 64);
    }
    else
    {
        // disable hardware adcAVG for better speed.
        ADCHardwareOversampleConfigure(ADC0_BASE, 1);
        ADCHardwareOversampleConfigure(ADC1_BASE, 1);
    }

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


    IntPrioritySet(INT_ADC0SS2, MID_PRIORITY_3);           // set priority
    ADCIntRegister(ADC1_BASE, 2, adcInterruptHandler);
    ADCIntEnable(ADC1_BASE, 2);
    IntEnable(INT_ADC1SS2);

    return &SensorData;
}
