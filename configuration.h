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
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>

#include <lcd_functions.h>

/*****************************  # defines #   *****************************/
// defines for ADC init
#define ROW_1_L ADC_CTL_CH13
#define ROW_2_L ADC_CTL_CH15
#define ROW_3_L ADC_CTL_CH14
#define ROW_4_L ADC_CTL_CH12
#define ROW_5_L ADC_CTL_CH5
#define ROW_6_L ADC_CTL_CH4
#define ROW_7_L ADC_CTL_CH7
#define ROW_8_L ADC_CTL_CH6
#define ROW_8_R ADC_CTL_CH19
#define ROW_7_R ADC_CTL_CH18
#define ROW_6_R ADC_CTL_CH17
#define ROW_5_R ADC_CTL_CH16
#define ROW_4_R ADC_CTL_CH8
#define ROW_3_R ADC_CTL_CH9
#define ROW_2_R ADC_CTL_CH0
#define ROW_1_R ADC_CTL_CH1

// defines for LCD init
#define RST 0x10
#define INITIAL_STATE (0x1F)
#define SOFTWARE_RESET (0x01)
#define SET_PLL_MN (0xE2)
#define START_PLL (0xE0)
#define SET_LSHIFT (0xE6)
#define SET_LCD_MODE (0xB0)
#define SET_HORI_PERIOD (0xB4)
#define SET_VERT_PERIOD (0xB6)
#define SET_ADRESS_MODE (0x36)
#define SET_PIXEL_DATA_FORMAT (0xF0)
#define SET_DISPLAY_ON (0x29)

// Interrupt priority
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80

/**************************  # Prototypes #   ****************************/
void ConfigureUART(int);
void ConfigureGPIO(void);
void ConfigureADC(void);
void ConfigureTimer0(int);
void ConfigureLCD(void);

extern void UARTIntHandler(void);
extern void Timer0IntHandler(void);


#endif /* CONFIGURATION_H_ */
