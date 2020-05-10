/*
 * uartDMA.h
 *
 *  Created on: 10.05.2020
 */
#include <adc_functions.h>
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>          // needed for UART0_BASE
#include "inc/hw_uart.h"            // needed for UART_O_DR
#include <driverlib/pin_map.h>      // GPIOPinConfigure
#include <driverlib/interrupt.h>
#include <driverlib/udma.h>

#include <Interrupt_handler.h>      // interrupt handler prototypes


/**************************  # Prototypes #   ****************************/
void ConfigureUART0(uint32_t);
void ConfigureUART2(uint32_t);
void configureUDMA(void);
void prepareReceiveDMA(void);
void sendUARTDMA(void);

