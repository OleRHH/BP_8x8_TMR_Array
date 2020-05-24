/* main.h */

#ifndef MAIN_H_
#define MAIN_H_


/*****************************  # Includes #   **********************************/
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"           // SysCtlClockFreqSet

#include <adc_functions.h>
#include <timer_functions.h>
#include <lcd_functions.h>
#include <uartDMA.h>
#include <touch.h>

/*****************************  # defines #   ***********************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.

#define GPIO_PIN_3_DOWNTO_0 0x0000000F  // ADC1IntHandler()

//#define OSZ_YEL     0b0000100           // Debug Output to oscilloscope
//#define OSZ_BLUE    0b0001000           // Debug Output to oscilloscope
//#define LED_D1 0b0000001                            // Debug Output to oscilloscope
//#define LED_D2 0b0000010                            // Debug Output to oscilloscope
//GPIO_PORTN_DATA_R ^= LED_D1;   // for debugging: toggle debug output each time handler is called
//GPIO_PORTN_DATA_R ^= LED_D2;  // for debugging: set high when handler is called


#endif /* MAIN_H_ */
