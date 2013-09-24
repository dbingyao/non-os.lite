/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _ASSERT_H
#define _ASSERT_H

/* void assert (int expression);

   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

# define assert(expr)	do { } while (0)

#else  /* Not NDEBUG.  */

/* This prints an "Assertion failed" message and aborts.  */
void __assert(const char *, const char *, unsigned int, const char *);

# define assert(expr)	\
	do { \
		if (!expr) { \
			__assert(#expr, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
		} \
	} while (0)

#endif /* NDEBUG.  */

#endif
