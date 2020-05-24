/* functions.h */

#ifndef ADC_FUNCTIONS_H_
#define ADC_FUNCTIONS_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <tm4c1294ncpdt.h>
#include <math.h>               // sqrt()
#include <driverlib/adc.h>      // ADCIntClear(), ADCIntStatus(), ADCProcessorTrigger(), ADCSequenceDataGet()
#include <inc/hw_memmap.h>      // ADC0_BASE, ADC1_BASE
#include <driverlib/interrupt.h>


/*****************************  # defines #   *****************************/
// defines for ADC init             // Port and Pin
#define ROW_1_L ADC_CTL_CH13        // PD2
#define ROW_2_L ADC_CTL_CH15        // PD0
#define ROW_3_L ADC_CTL_CH14        // PD1
#define ROW_4_L ADC_CTL_CH12        // PD3
#define ROW_5_L ADC_CTL_CH5         // PD6
#define ROW_6_L ADC_CTL_CH4         // PD7
#define ROW_7_L ADC_CTL_CH7         // PD4
#define ROW_8_L ADC_CTL_CH6         // PD5
#define ROW_8_R ADC_CTL_CH19        // PK3
#define ROW_7_R ADC_CTL_CH18        // PK2
#define ROW_6_R ADC_CTL_CH17        // PK1
#define ROW_5_R ADC_CTL_CH16        // PK0
#define ROW_4_R ADC_CTL_CH8         // PE5
#define ROW_3_R ADC_CTL_CH9         // PE4
#define ROW_2_R ADC_CTL_CH0         // PE3
#define ROW_1_R ADC_CTL_CH1         // PE2

// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80


/*********************  # public Prototypes #   ****************************/
void storeArraySensorData(uint16_t);
uint32_t computeArrows(bool, uint16_t);
void ConfigureADC(void);
void startAdcConversion(void);
void adcIntClear(void);
bool getADCHardwareAveraging(char);

// interrupt handler for ADC 0. Needs to be implemented in project.
extern void ADC1IntHandler(void);


/*********************  # public global variables #   **********************/

// 256 bytes array for transmit via RS-232 in one burst
int16_t DiffResults[2][8][8];

// final ADC-values used to calculate the arrows on the LC-Display.
int16_t DiffSinResults[8][8];
int16_t DiffCosResults[8][8];

// these values are used to color the arrows depending on their length.
int16_t arrowLength[8][8];

// not used at the moment. It is here for historic reasons and to be used again sometime.
int16_t SinOffset[8][8];
int16_t CosOffset[8][8];


#endif /* ADC_FUNCTIONS_H_ */

