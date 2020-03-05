/*
 * Interrupt_handler.h
 *
 *  Created on: 13.02.2020
 */

#ifndef INTERRUPT_HANDLER_H_
#define INTERRUPT_HANDLER_H_


/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include "inc/hw_memmap.h"      // UART0_BASE
#include <driverlib/timer.h>

#include <functions.h>
#include <lcd_functions.h>


/*****************************  # defines #   *****************************/


/**************************  # Prototypes #   ****************************/
void UARTIntHandler(void);


extern int16_t DiffSinResults[8][8];
extern int16_t DiffCosResults[8][8];
extern uint32_t SysClock;

#endif /* INTERRUPT_HANDLER_H_ */
