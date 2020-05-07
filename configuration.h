/*
 * configuration.h
 *
 *  Created on: 13.02.2020
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>
#include <tm4c1294ncpdt.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>      // needed for UART0_BASE
#include "inc/hw_uart.h"        // needed for UART_O_DR
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/udma.h>


/*****************************  # defines #   *****************************/
// defines for ADC init             // Port and Pin
#define ROW_1_L ADC_CTL_CH13        // PD2
#define ROW_2_L ADC_CTL_CH15        // PD0
#define ROW_3_L ADC_CTL_CH14        // PD1
#define ROW_4_L ADC_CTL_CH12        // PD3
#define ROW_5_L ADC_CTL_CH5         // PD6
#define ROW_6_L ADC_CTL_CH4         // PD7
#define ROW_7_L ADC_CTL_CH7         // PD4
#define ROW_8_L ADC_CTL_CH6         // PD5
#define ROW_8_R ADC_CTL_CH19        // PK3
#define ROW_7_R ADC_CTL_CH18        // PK2
#define ROW_6_R ADC_CTL_CH17        // PK1
#define ROW_5_R ADC_CTL_CH16        // PK0
#define ROW_4_R ADC_CTL_CH8         // PE5
#define ROW_3_R ADC_CTL_CH9         // PE4
#define ROW_2_R ADC_CTL_CH0         // PE3
#define ROW_1_R ADC_CTL_CH1         // PE2

// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80

/**************************  # Prototypes #   ****************************/
void ConfigureUART0(uint32_t);
void ConfigureUART2(uint32_t);
void configureUDMA(void);
void ConfigureGPIO(void);
void ConfigureADC(void);
void ConfigureTimer0(uint32_t);

extern void UART0IntHandler(void);
extern void UART2IntHandler(void);
extern void Timer0IntHandler(void);
extern void ADC0IntHandler(void);


#endif /* CONFIGURATION_H_ */
