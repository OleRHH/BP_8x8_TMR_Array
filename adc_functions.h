/* functions.h */
#ifndef ADC_FUNCTIONS_H_
#define ADC_FUNCTIONS_H_

/************************  # public Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>


typedef struct {
    // final ADC-values used to calculate the arrows on the LC-Display.
    union {
        // 256 bytes array for transmit via RS-232 in one burst
        char resultsForUARTSend[256];
        struct
        {
            int16_t dSin[8][8];
            int16_t dCos[8][8];
        };
    };
    struct {
        int16_t dSin[8][8];
        int16_t dCos[8][8];
    }diff;

    int16_t DiffSinResults[8][8];
    int16_t DiffCosResults[8][8];
    // these values are used to color the arrows depending on their length.
    uint16_t arrowLength[8][8];
    uint16_t maxAnalogValue;
} TMRSensorData;


/*********************  # public Prototypes #   ****************************/
TMRSensorData * ConfigureADC(void);
void storeArraySensorData(uint16_t);
void computeArrows(bool, uint16_t, TMRSensorData *);
void startADConversion(void);
void adcIntClear(void);
bool getADCHardwareAveraging(char);
void setMultiplexer(uint16_t step);

// interrupt handler for ADC 0. Needs to be implemented in project.
extern void ADC1IntHandler(void);





#endif /* ADC_FUNCTIONS_H_ */
