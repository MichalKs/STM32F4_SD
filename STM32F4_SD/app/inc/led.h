/**
 * @file:   led.h
 * @brief:  Light Emitting Diodes control functions.
 * @date:   9 kwi 2014
 * @author: Michal Ksiezopolski
 * @details A simple library to add an abstraction
 * layer to blinking LEDs.
 * To use the library you need to call LED_Init using
 * one of the LEDs defined in LED_Number_TypeDef and then
 * use LED_Toggle or LED_ChangeState with the initialized
 * LED number.
 * The various LED ports and pins are defined in
 * led_hal.c and led_hal.h.
 * 
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the 
 * accompanying materials are made available 
 * under the terms of the GNU Public License 
 * v3.0 which accompanies this distribution, 
 * and is available at 
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#ifndef LED_H_
#define LED_H_

/**
 * @defgroup  LED LED
 * @brief     Light Emitting Diode control functions.
 */

/**
 * @addtogroup LED
 * @{
 */

/**
 * @brief LED enum - for identifying an LED.
 */
typedef enum {
  LED0,//!< LED0
  LED1,//!< LED1
  LED2,//!< LED2
  LED3,//!< LED3
  LED4,//!< LED4
  LED5,//!< LED5
  LED6,//!< LED6
  LED7,//!< LED7
  LED8,//!< LED8
  LED9,//!< LED9

} LED_Number_TypeDef;

/**
 * @brief State of an LED.
 */
typedef enum {
  LED_UNUSED, //!< LED_UNUSED LED not initialized
  LED_OFF,    //!< LED_OFF    Turn off LED
  LED_ON,     //!< LED_ON     Turn on LED
} LED_State_TypeDef;

void LED_Init         (LED_Number_TypeDef led);
void LED_Toggle       (LED_Number_TypeDef led);
void LED_ChangeState  (LED_Number_TypeDef led, LED_State_TypeDef state);

/**
 * @}
 */

#endif /* LED_H_ */
