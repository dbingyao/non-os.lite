/**
 * (C) Copyright 2013 Faraday Technology
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

static inline void 
enable_icache(void)
{
	asm volatile (
		/* Invalidate I-Cache all */
		"mcr	p15, 0, %0, c7, c5, 0\n"

		"mrc     p15, 0, r0, c1, c0, 0\n"
		"orr     r0, r0, #0x00000002\n"     /* set bit 2 (A) Align */
		"orr     r0, r0, #0x00001000\n"     /* set bit 12 (I) I-Cache */
		"mcr     p15, 0, r0, c1, c0, 0\n"
		:
		: "r" (0)
		);
}
