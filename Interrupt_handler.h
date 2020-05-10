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
#include <driverlib/gpio.h>     // GPIO_PIN_X
#include <tm4c1294ncpdt.h>      // GPIO_PORTX_BASE
#include <driverlib/timer.h>    // TimerIntClear()
#include <lcd_functions.h>
#include <adc_functions.h>
#include <uartDMA.h>


/*****************************  # defines #   *****************************/
#define LED_D1 0b0000001                            // Debug Output to oscilloscope
#define LED_D2 0b0000010                            // Debug Output to oscilloscope
#define YELLOW 0b0000100                            // Debug Output to oscilloscope
#define BLUE   0b0001000                            // Debug Output to oscilloscope
#define GPIO_PIN_3_DOWNTO_0 0x0000000F


/**************************  # public Prototypes #   ****************************/
void Timer0IntHandler(void);
void UART0IntHandler(void);
void UART2IntHandler(void);
void ADC0IntHandler(void);


/*********************  # public global variables #   **********************/

// The transmit buffer used for the UART transfer.
char UART0receive[8];


#endif /* INTERRUPT_HANDLER_H_ */
