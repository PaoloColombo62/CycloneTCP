/**
 * @file system_lm3s6965.c
 * @brief System configuration for LM3S6965 devices
 *
 * @section License
 *
 * Copyright (C) 2010-2013 Oryx Embedded. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.5
 **/

#ifndef __SYSTEM_LM3S6965_H
#define __SYSTEM_LM3S6965_H

//Dependencies
#include <stdint.h>

//Exported variables
extern uint32_t SystemCoreClock;

//Exported functions
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

#endif
