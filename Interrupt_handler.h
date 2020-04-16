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
#define LED_D1 0b0000001                            // Debug Output to oscilloscope
#define LED_D2 0b0000010                            // Debug Output to oscilloscope
#define YELLOW 0b0000100                            // Debug Output to oscilloscope
#define BLUE   0b0001000                            // Debug Output to oscilloscope


/**************************  # Prototypes #   ****************************/
void Timer0IntHandler(void);
void UART0IntHandler(void);
void UART2IntHandler(void);
void ADC0IntHandler(void);


#endif /* INTERRUPT_HANDLER_H_ */
