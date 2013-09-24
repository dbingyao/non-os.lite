/*
 * dietlibc-0.33
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <math.h>

extern const float  __half;

double  sinh(double x)
{
	long double  y = exp(x);
	return (y - 1. / y) * __half;
}
