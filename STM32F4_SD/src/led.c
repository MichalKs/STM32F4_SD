/**
 * @file: 	led.c
 * @brief:  Light Emitting Diode control functions.
 * @date: 	9 kwi 2014
 * @author: Michal Ksiezopolski
 * 
 * This is a very simple library for controlling
 * LEDs.
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
 *
 */

#include <stdio.h>

#include "led.h"

static LED_TypeDef leds[LED_MAX]; ///< LED table.

/**
 * Add an LED.
 * @param led LED init structure.
 */
void LED_Add(LED_TypeDef* led) {

	// Check if led number is correct.
	if (led->nr >= LED_MAX) {
		printf("Error: Incorrect LED number!\r\n");
		return;
	}

	RCC_AHB1PeriphClockCmd(led->clk, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	// Configure pin in output push/pull mode
	GPIO_InitStructure.GPIO_Pin = (1 << led->pin);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // less interference
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(led->gpio, &GPIO_InitStructure);

	leds[led->nr] = *led;

}

/**
 * Toggle an LED.
 * @param led LED number.
 */
void LED_Toggle(LED_Number_TypeDef led) {

	if (!leds[led].gpio) {
		printf("Error: LED doesn't exist!\r\n");
		return;
	}

	GPIO_WriteBit(leds[led].gpio, 1 << leds[led].pin,
			1-GPIO_ReadOutputDataBit(leds[led].gpio, 1 << leds[led].pin));

}

/**
 * Change the state of an LED.
 * @param led LED number.
 * @param state New state.
 */
void LED_ChangeState(LED_Number_TypeDef led, LED_State_TypeDef state) {

	if (!leds[led].gpio) {
		printf("Error: LED doesn't exist!\r\n");
		return;
	}

	GPIO_WriteBit(leds[led].gpio, 1 << leds[led].pin, state);

}
