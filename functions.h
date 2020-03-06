/*
 * functions.h
 *
 *  Created on: 13.02.2020
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>      // ADCIntClear(), ADCIntStatus(), ADCProcessorTrigger(), ADCSequenceDataGet()
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>      // UART0_BASE, ADC0_BASE, ADC1_BASE
#include <driverlib/uart.h>



/*****************************  # defines #   *****************************/
#define GPIO_PIN_3_DOWNTO_0 0x0000000F

/**************************  # Prototypes #   ****************************/
void ReadArray(void);
void GetADCValues(void);
void Computations(void);



#endif /* FUNCTIONS_H_ */

