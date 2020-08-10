/*
 * touch.h
 *  Created on: 20.05.2020
 */
#ifndef TOUCH_H_
#define TOUCH_H_

#define LEFT_BUTTON  ( 1 )
#define RIGHT_BUTTON ( 2 )
#define STOP_BUTTON  ( 3 )
#define START_BUTTON ( 4 )
#define ARROW_LENGTH_BUTTON ( 5 )
#define SCALING_BUTTON ( 6 )
#define HARDW_AVG_BUTTON ( 7 )

/*****************************  # Includes #   ****************************/
#include <stdbool.h>            // for variable type bool
#include <tm4c1294ncpdt.h>      // GPIO_PORTF_AHB_DATA_R


#include <inc/hw_memmap.h>      // needed for UART0_BASE
#include <driverlib/gpio.h>
/*********************  # public Prototypes #   ***************************/
void configureTouch(void);
uint16_t touchGetItem(uint16_t *, uint16_t *);

extern void TouchInterruptHandler(void);


#endif /* TOUCH_H_ */


#define A_LENGTGH_X ( 380 )       // button 'max arrow length' 10 - 200?
#define A_LENGTGH_Y ( 1175 )

#define SCALING_X ( 380 )       // button 'scaling' relative/absolute
#define SCALING_Y ( 780 )

#define H_AVG_X ( 380 )         // button 'hardware averaging' on/off
#define H_AVG_Y ( 410 )
