/**
 * @file:   led.c
 * @brief:  Light Emitting Diode control functions.
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
 *
 */

#include <stdio.h>
#include <led.h>
#include <led_hal.h>

#ifndef DEBUG
  #define DEBUG
#endif

#ifdef DEBUG
  #define print(str, args...) printf("LED--> "str"%s",##args,"\r")
  #define println(str, args...) printf("LED--> "str"%s",##args,"\r\n")
#else
  #define print(str, args...) (void)0
  #define println(str, args...) (void)0
#endif

/**
 * @addtogroup LED
 * @{
 */

static LED_State_TypeDef ledState[MAX_LEDS]; ///< States of the LEDs (MAX_LEDS is hardware dependent)

/**
 * @brief Add an LED.
 * @param led LED init structure.
 */
void LED_Init(LED_Number_TypeDef led) {

  // Check if LED number is correct.
  if (led >= MAX_LEDS) {
    println("Error: Incorrect LED number %d!", (int)led);
    return;
  }

  LED_HAL_Init(led);
  ledState[led] = LED_OFF; // LED initially off
}

/**
 * @brief Change the state of an LED.
 * @param led LED number.
 * @param state New state.
 */
void LED_ChangeState(LED_Number_TypeDef led, LED_State_TypeDef state) {

  if (led >= MAX_LEDS) {
    println("Error: Incorrect LED number %d!", (int)led);
    return;
  }

  if (ledState[led] == LED_UNUSED) {
    println("Error: Uninitialized LED %d!", (int)led);
    return;
  } else {
    if (state == LED_OFF) {
      LED_HAL_ChangeState(led, 0); // turn off LED
    } else if (state == LED_ON) {
      LED_HAL_ChangeState(led, 1); // light up LED
    }
  }

  ledState[led] = state; // update LED state
}

/**
 * @brief Toggle an LED.
 * @param led LED number.
 */
void LED_Toggle(LED_Number_TypeDef led) {

  if (led >= MAX_LEDS) {
    println("Error: Incorrect LED number %d!", (int)led);
    return;
  }

  if (ledState[led] == LED_UNUSED) {
    println("Error: Uninitialized LED %d!", (int)led);
    return;
  } else {
    if (ledState[led] == LED_OFF) {
      ledState[led] = LED_ON;
    } else if (ledState[led] == LED_ON) {
      ledState[led]= LED_OFF;
    }
    LED_HAL_Toggle(led);
  }
}

/**
 * @}
 */
