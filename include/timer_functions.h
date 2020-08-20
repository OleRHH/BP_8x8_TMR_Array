/* timer_functions.h */

#ifndef TIMER_FUNCTIONS_H_
#define TIMER_FUNCTIONS_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/interrupt.h>        // IntMasterDisable


// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80


/*********************  # public Prototypes #   ***************************/
void configureTimer0(uint32_t);
void timer0IntClear(void);
void timerInterruptDisable(void);
void timerInterruptEnable(void);
// interrupt handler for Timer 0. Needs to be implemented in project.
extern void Timer0InterruptHandler(void);


#endif /* TIMER_FUNCTIONS_H_ */
