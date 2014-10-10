/**
 * @file: 	systick.h
 * @brief:	   
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

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <inttypes.h>


/**
 * @defgroup  SYSTICK SYSTICK
 * @brief     SYSTICK control functions.
 */

/**
 * @addtogroup SYSTICK
 * @{
 */
void      SYSTICK_Init    (uint32_t freq);
uint32_t  SYSTICK_GetTime (void);

/**
 * @}
 */

#endif /* SYSTICK_H_ */
