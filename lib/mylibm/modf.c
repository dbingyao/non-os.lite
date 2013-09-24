/*
 * dietlibc-0.33
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <math.h>

double modf(double x, double *iptr)
{
	double fmod_result = fmod(x, 1.0);
	*iptr = x - fmod_result;
	return fmod_result;
}
