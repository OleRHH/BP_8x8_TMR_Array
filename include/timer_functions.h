/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: timer_functions.h                                                     */
/********************************************************************************/
#ifndef TIMER_FUNCTIONS_H_
#define TIMER_FUNCTIONS_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/interrupt.h>        // IntMasterDisable


// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY_0 0x00
#define HIGH_PRIORITY_1 0x20
#define MID_PRIORITY_2 0x40
#define MID_PRIORITY_3 0x60
#define MID_PRIORITY_4 0x80
#define LOW_PRIORITY_5 0xA0
#define LOW_PRIORITY_6 0xC0
#define LOW_PRIORITY_7 0xE0


/*********************  # public Prototypes #   ***************************/
void configureTimer0(uint32_t);
void configureTimer1(uint32_t);
void timer0IntClear(void);
void timer1IntClear(void);
void startTimer1(void);
void timer0InterruptDisable(void);
void timer0InterruptEnable(void);

// interrupt handler for Timer 0. Needs to be implemented in project.
extern void timer0InterruptHandler(void);
extern void timer1InterruptHandler(void);


#endif /* TIMER_FUNCTIONS_H_ */
