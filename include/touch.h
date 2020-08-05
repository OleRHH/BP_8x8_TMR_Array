/*
 * touch.h
 *  Created on: 20.05.2020
 */
#ifndef TOUCH_H_
#define TOUCH_H_


/*****************************  # Includes #   ****************************/
#include <stdbool.h>            // for variable type bool
#include <tm4c1294ncpdt.h>      // GPIO_PORTF_AHB_DATA_R


/*********************  # public Prototypes #   ***************************/
void configureTouch(void);
unsigned int touchRead(void);
void touchWrite(unsigned char);


#endif /* TOUCH_H_ */
