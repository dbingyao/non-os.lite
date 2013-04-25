/* 
 * Modified from the 'rand.cxx' of eCos-3.0
 *
 */
#include <stdlib.h>

/* This algorithm is mentioned in the ISO C standard, here extended
   for 32 bits.  */
int rand_r (unsigned int *seed)
{
	unsigned int s = *seed;
	unsigned int uret;

	s = (s * 1103515245) + 12345; /* permutate seed */
	/* Only use top 11 bits */
	uret = s & 0xffe00000;

	s = (s * 1103515245) + 12345; /* permutate seed */
	/* Only use top 14 bits */
	uret += (s & 0xfffc0000) >> 11;

	s = (s * 1103515245) + 12345; /* permutate seed */
	/* Only use top 7 bits */
	uret += (s & 0xfe000000) >> (11+14);

	*seed = s;

	return (int)(uret & RAND_MAX);
}
