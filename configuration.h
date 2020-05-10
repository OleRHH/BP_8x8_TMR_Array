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
#include <tm4c1294ncpdt.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>      // GPIOPinConfigure
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <Interrupt_handler.h>  // interrupt handler


// Interrupt priority. Lower numbers = higher priority.
// Valid numbers: 0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0
#define HIGH_PRIORITY 0x00
#define LOW_PRIORITY  0x80

void ConfigureGPIO(void);
void ConfigureTimer0(uint32_t);



#endif /* CONFIGURATION_H_ */
