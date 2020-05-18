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


/*****************************  # defines #   ***********************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.


#endif /* MAIN_H_ */
