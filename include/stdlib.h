/*
 * Copyright (c) 2010, Dante Su (dantesu@faraday-tech.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

/* Convert a string to an integer.  */
int atoi (char *__nptr);
/* Convert an integer to a string.  */
char *itoa(int __value, char *__nptr, int __base);
/* Convert a string to a long integer.  */
long int atol (char *__nptr);
/* Convert a long integer to a string.  */
char *ltoa(long int __value, char *__nptr, int __base);
/* Convert a string to a long integer.  */
long int strtol (const char *__nptr, char **__endptr, int __base);
/* Convert a string to an unsigned long integer.  */
unsigned long int strtoul (const char *__nptr, char **__endptr, int __base);

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
void qsort(void *base, size_t nmemb, size_t size,
           int(*compar)(const void *, const void *));

/* Abort execution and generate a core-dump.  */
#define abort()			do { } while(0)

/* Return the absolute value of X.  */
#define abs(x)			((x) > 0 ? (x) : -(x))

/* Returned by `div'.  */
typedef struct {
    int quot;			/* Quotient.  */
    int rem;			/* Remainder.  */
} div_t;

/* Returned by `ldiv'.  */
#ifndef __ldiv_t_defined
typedef struct {
    long int quot;		/* Quotient.  */
    long int rem;		/* Remainder.  */
} ldiv_t;
# define __ldiv_t_defined	1
#endif

/* Return the `div_t', `ldiv_t' or `lldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
div_t div (int __numer, int __denom);
ldiv_t ldiv (long int __numer, long int __denom);

/* Return a random integer between 0 and RAND_MAX inclusive.  */
int rand (void);

/* Reentrant interface according to POSIX.1.  */
int rand_r (unsigned int *seed);

/* Seed the random number generator with the given number.  */
void srand (unsigned int seed);

#endif	/* #ifndef _STDLIB_H */
