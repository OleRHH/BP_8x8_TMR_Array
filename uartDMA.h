/* uartDMA.h */

#ifndef UART_DMA_H_
#define UART_DMA_H_

/*****************************  # Includes #   ****************************/
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


/***********************  # public prototypes #   ***************************/
void ConfigureUART0(uint32_t);
void ConfigureUART2(uint32_t);
void configureUDMA(void);
void prepareReceiveDMA(void);
void sendUARTDMA(void);
void sendCommandToMotor(char *, uint16_t);
uint32_t receiveDataFromMotor(void);
char * getDataFromPc(void);

// interrupt handler for UART 0 and UART 2. Needs to be implemented in project.
extern void UART0IntHandler(void);
extern void UART2IntHandler(void);


/*********************  # public global variables #   **********************/



#endif /* UART_DMA_H_ */
