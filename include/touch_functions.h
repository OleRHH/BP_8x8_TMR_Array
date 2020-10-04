/*  Bachelor-Project Visualization of a 8x8 TMR sensor-array on a 7'' LCD       */
/*  HAW-Hamburg, September 2020, Ole Rönna, van Hung Le.                        */
/*  File: touch_functions.h                                                     */
/********************************************************************************/
#ifndef TOUCH_H_
#define TOUCH_H_

/************************  # public Includes #   ********************************/
#include <stdbool.h>
#include <stdint.h>


/************************  # public defines #   *********************************/
#define LEFT_BUTTON  ( 1 )
#define RIGHT_BUTTON ( 2 )
#define STOP_BUTTON  ( 3 )
#define CHANGE_SETTING_BUTTON ( 4 )
#define ARROW_LENGTH_BUTTON ( 5 )
#define SCALING_BUTTON ( 6 )
#define HARDW_AVG_BUTTON ( 7 )


/**************************  # public Prototypes #   ****************************/
void configureTouch(void);
void touchInterruptClear(void);
uint16_t touchGetMenuItem(void);
uint16_t touchGetArrowLength(uint16_t);
uint16_t touchGetSettingNum(uint16_t);

extern void touchInterruptHandler(void);


#endif /* TOUCH_H_ */
