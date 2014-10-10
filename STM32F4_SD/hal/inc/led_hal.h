/**
 * @file: 	led_hal.h
 * @brief:	HAL for using LEDs
 * @date: 	25 sie 2014
 * @author: Michal Ksiezopolski
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

#ifndef LED_HAL_H_
#define LED_HAL_H_

#include <inttypes.h>


/**
 * @defgroup  LED_HAL LED_HAL
 * @brief     HAL - Light Emitting Diode control functions.
 */

/**
 * @addtogroup LED_HAL
 * @{
 */

#define MAX_LEDS    4 ///< Maximum number of LEDs available in design

void LED_HAL_Init         (uint8_t led);
void LED_HAL_Toggle       (uint8_t led);
void LED_HAL_ChangeState  (uint8_t led, uint8_t state);

/**
 * @}
 */

#endif /* LED_HAL_H_ */
