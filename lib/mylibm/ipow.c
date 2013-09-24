/*
 * dietlibc-0.33
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#define _GNU_SOURCE
#include <math.h>
/*
 * This is not standard, but often you only need such this function
 * which is much shorter than the generic pow() function.
 *
 *   double  ipow ( double mant, int expo );
 */

double  ipow(double mant, int expo)
{
	double        ret = 1.;
	unsigned int  e   = expo;	/* Some attention is necessary for expo = 2^31 */

	if ((int)e < 0) {
		e    = -e;
		mant = 1. / mant;
	}

	while (1) {
		if (e & 1)
			ret *= mant;
		if ((e >>= 1) == 0)
			break;
		mant *= mant;
	}

	return ret;
}
