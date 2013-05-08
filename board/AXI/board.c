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
#define TIMER_LOAD_VAL	0xffffffff

extern int interrupt_init(void);
extern void init_global_timer(int load_val, int use_extclk);

void board_init(void)
{
	/* Interrupt */
	interrupt_init();

	/* Init default timer */
	init_global_timer(TIMER_LOAD_VAL, 0);
}
