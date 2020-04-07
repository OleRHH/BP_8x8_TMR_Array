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
#include "driverlib/rom.h"
#include "inc/hw_uart.h"
#include "driverlib/rom_map.h"
#include <driverlib/uart.h>
#include "inc/hw_memmap.h"      // UART0_BASE
#include <driverlib/timer.h>
#include "driverlib/udma.h"

#include <functions.h>
#include <lcd_functions.h>


/*****************************  # defines #   *****************************/


/**************************  # Prototypes #   ****************************/
void UART0IntHandler(void);
void UART2IntHandler(void);



#endif /* INTERRUPT_HANDLER_H_ */
