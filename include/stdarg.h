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
#ifndef _STD_ARG_H_
#define _STD_ARG_H_

#if 0
#include <stdarg.h> /* va_list, va_start(), va_arg(), va_end() */
#else
/* home-brew STDARG.H, also public-domain: */

/* Assume: width of stack == width of int. Don't use sizeof(char *) or
other pointer because sizeof(char *)==4 for LARGE-model 16-bit code.
Assume: width is a power of 2 */
#define	STACK_WIDTH	sizeof(int)

/* Round up object width so it's an even multiple of STACK_WIDTH.
Using & for division here, so STACK_WIDTH must be a power of 2. */
#define	TYPE_WIDTH(TYPE)				\
	((sizeof(TYPE) + STACK_WIDTH - 1) & ~(STACK_WIDTH - 1))

/* point the va_list pointer to LASTARG,
then advance beyond it to the first variable arg */
#define	va_start(PTR, LASTARG)				\
	PTR = (va_list)((char *)&(LASTARG) + TYPE_WIDTH(LASTARG))

#define va_end(PTR)	/* nothing */

#define va_arg_helper(PTR, SIZE)					\
do { 									\
	if (SIZE == 8) PTR = (char *) PTR + 4;     			\
} while (0)

/* Increment the va_list pointer, then "return"
(evaluate to, actually) the previous value of the pointer. */
#define va_arg(PTR, TYPE)	(			\
	PTR = (char *)(PTR) + TYPE_WIDTH(TYPE)		\
				,			\
	*((TYPE *)((char *)(PTR) - TYPE_WIDTH(TYPE)))	\
				)

typedef void *va_list;
#endif

#endif
