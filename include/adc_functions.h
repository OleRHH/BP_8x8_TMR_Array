/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: adc_functions.h                                                       */
/********************************************************************************/
#ifndef ADC_FUNCTIONS_H_
#define ADC_FUNCTIONS_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>                     // for 'int16_t'
#include <driverlib/interrupt.h>        // IntMasterDisable


/*********************  # public typedefs #   ***********************************/
// Structure that holds all sensor data
typedef struct {
    union {
        // 256 bytes array for transmit via RS-232 to matlab
        // this is used to send data via UART0
        char resultsForUARTSend[256];
        // and this is used
        struct {
            int16_t dSin[8][8];
            int16_t dCos[8][8];
        };
    };

    // Structur that holds data for the LC-Display.
    struct {
        int16_t dSin[8][8];
        int16_t dCos[8][8];
        // used to color the arrows depending on their length.
        uint16_t arrowLength[8][8];
    }arrows;

    uint16_t maxAnalogValue;

} TMRSensorData;


/**************************  # public Prototypes #   **************************/
TMRSensorData * configureADC(bool);
void storeArraySensorData(uint16_t);
void computeArrows(bool, uint16_t);
void startADConversion(void);
void adcIntClear(void);
bool setADCHardwareAveraging(bool);
void setMultiplexer(uint16_t step);

// interrupt handler for ADC 0. Needs to be implemented in project.
extern void adcInterruptHandler(void);


#endif /* ADC_FUNCTIONS_H_ */
