/**
 * @file:   assert.c
 * @brief:  Functions for assertion checking
 * @date:   9 kwi 2014
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

#include <stdio.h>
#include <inttypes.h>

/**
 * @brief This function is called when an assert is failed.
 *
 * @details Information about the error is sent to the serial port
 * and the program is stopped.
 *
 * @param file File name where error occurred
 * @param line Line number where error occurred
 */
void assert_failed(uint8_t* file, uint32_t line) {
      printf("Assert fail at File %s Line %d\n", file, (int)line);
      while(1); // hold program
}
