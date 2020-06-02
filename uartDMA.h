/* uartDMA.h */

#ifndef UART_DMA_H_
#define UART_DMA_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>


/*****************************  # defines #   *****************************/
#define DMA_RX_INTERRUPT 0x10000


/***********************  # public prototypes #   ***************************/
void ConfigureUART0(uint32_t);
void ConfigureUART2(uint32_t);
void configureUartUDMA(void);
void prepareNextReceiveDMA(void);
void sendUARTDMA(TMRSensorData *);
void sendCommandToMotor(char *, uint16_t);
uint32_t receiveDataFromMotor(void);
char * getUART0RxData(void);
bool getRelativeAbsoluteSetting(void);
uint16_t getMaxArrowLength(void);
unsigned int UARTGetIntStatus(void);
void UART0ClearInterrupt(unsigned int interruptStatus);


// interrupt handler for UART 0 and UART 2. Needs to be implemented in project.
extern void UART0IntHandler(void);
extern void UART2IntHandler(void);


/*********************  # public global variables #   **********************/



#endif /* UART_DMA_H_ */
