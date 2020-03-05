/*
 * main.h
 *
 *  Created on: 13.02.2020
 */

#ifndef MAIN_H_
#define MAIN_H_


/*****************************  # Includes #   ****************************/
#include <stdbool.h>
#include <stdint.h>
#include <tm4c1294ncpdt.h>
#include "driverlib/sysctl.h"

#include <configuration.h>
#include <Interrupt_handler.h>
#include <functions.h>
#include <lcd_functions.h>


/*****************************  # defines #   *****************************/
#define CLOCK_FREQ ( 120000000 )        // 120 MHz clock freq.


/**************************  # Prototypes #   ****************************/

// enum colors
enum
{
    BLACK   = 0x000000,
    RED     = 0x0000FF,
    GREEN   = 0x00FF00,
    YELLOW  = 0x00FFFF,
    BLUE    = 0xFF0000,
    WHITE   = 0xFFFFFF
};

#endif /* MAIN_H_ */
