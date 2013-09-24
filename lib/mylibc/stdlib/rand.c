/*
 * (C) Copyright 2010 Dante Su <dantesu@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdlib.h>

#ifdef CONFIG_LIB_FREERTOS

/* POSIX.1c requires that there is mutual exclusion for the `rand' and
   `srand' functions to prevent concurrent calls from modifying common
   data.  */
#include <FreeRTOS.h>
#include <task.h>

#define __lock()		vTaskSuspendAll()
#define __unlock()		xTaskResumeAll()

#else	/* CONFIG_LIB_FREERTOS */

#define __lock()		do { } while(0)
#define __unlock()		do { } while(0)

#endif	/* CONFIG_LIB_FREERTOS */

static unsigned int _seed = 1;

/* Seed the random number generator with the given number.  */
void srand (unsigned int seed)
{
	__lock();
	_seed = seed;
	__unlock();
}

/* Return a random integer between 0 and RAND_MAX inclusive.  */
int rand(void)
{
	int n;
	__lock();
	n = rand_r(&_seed);
	__unlock();
	return n;
}
