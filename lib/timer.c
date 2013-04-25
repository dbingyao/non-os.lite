/**
 * Copyright 2013 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <types.h>
#include <div64.h>
#include <common.h>
#include <platform.h>

unsigned long long get_ticks(void);

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, TIMER_CLOCK);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= TIMER_CLOCK;
	do_div(usec, 1000000);

	return usec;
}

void udelay(unsigned long usec)
{
	unsigned long long start;
	ulong tmo;

	start = get_ticks();		/* get current timestamp */
	tmo = usec_to_tick(usec);	/* convert usecs to ticks */
	while ((get_ticks() - start) < tmo)
		;			/* loop till time has passed */
}

/*
 * get_timer(base) can be used to check for timeouts or
 * to measure elasped time relative to an event:
 *
 * ulong start_time = get_timer(0) sets start_time to the current
 * time value.
 * get_timer(start_time) returns the time elapsed since then.
 *
 * The time is used in CONFIG_SYS_HZ units!
 */
unsigned long get_timer(unsigned long base)
{
	unsigned long long time, ticks;

	ticks = get_ticks();
	time = tick_to_time(ticks);

	return time - base;
}

