/* main.h */

#ifndef MAIN_H_
#define MAIN_H_


/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"           // SysCtlClockFreqSet

#include <adc_functions.h>
#include <timer_functions.h>
#include <lcd_functions.h>
#include <uartDMA.h>


/*****************************  # defines #   *****************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.
#define OSZ_YEL     0b0000100           // Debug Output to oscilloscope
#define OSZ_BLUE    0b0001000           // Debug Output to oscilloscope
#define GPIO_PIN_3_DOWNTO_0 0x0000000F  // ADC0IntHandler()


#endif /* MAIN_H_ */
