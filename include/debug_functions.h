/*
 * debug_functions.h
 *
 *  Created on: 28.05.2020
 *      Author: ole
 */

#ifndef DEBUG_FUNCTIONS_H_
#define DEBUG_FUNCTIONS_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>


void toggleLED(uint16_t);
void toggleOszi(uint16_t);
void onLED(uint16_t);
void offLED(uint16_t);
void onOszi(uint16_t);
void offOszi(uint16_t);
void configureDebugGPIO(void);


#endif /* DEBUG_FUNCTIONS_H_ */
