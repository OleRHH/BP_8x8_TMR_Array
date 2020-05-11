/* timer_functions.h */

#ifndef TIMER_FUNCTIONS_H_
#define TIMER_FUNCTIONS_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/interrupt.h>  // intEnable(), IntPrioritySet()
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>      // TIMER0_BASE_BASE


// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80


/*********************  # public Prototypes #   ***************************/
void ConfigureTimer0(uint32_t);
void timer0IntClear(void);

// interrupt handler for Timer 0. Needs to be implemented in project.
extern void Timer0IntHandler(void);


#endif /* TIMER_FUNCTIONS_H_ */
