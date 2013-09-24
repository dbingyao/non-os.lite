/*
 * dietlibc-0.33
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <math.h>

double  asinh(double x)
{
	return log(x + sqrt(x * x + 1.));
}
