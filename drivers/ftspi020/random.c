
/* Mother *************************************************************
|  George Marsaglia's The mother of all random number generators
|   producing uniformly distributed pseudo random 32 bit values
|   with period about 2^250.
|  The text of Marsaglia's posting is provided in the file mother.txt
|
|  The arrays mother1 and mother2 store carry values in their
|   first element, and random 16 bit numbers in elements 1 to 8.
|   These random numbers are moved to elements 2 to 9 and a new
|   carry and number are generated and placed in elements 0 and 1.
|  The arrays mother1 and mother2 are filled with random 16 bit
|   values on first call of Mother by another generator.  mStart
|   is the switch.
|
|  Returns:
|   A 32 bit random number is obtained by combining the output of the
|   two generators and returned in *pSeed.  It is also scaled by
|   2^32-1 and returned as a double between 0 and 1
|
|  SEED:
|   The inital value of *pSeed may be any long value
|
|   Bob Wheeler 8/8/94
*/
/****************************************/
/* Parameters for the mother of random  */
/* number generation programs: Mother() */
/****************************************/
#include <string.h>

#include <common.h>

#define m16Long   65536L	/* 2^16 */
#define m16Mask   0xFFFF	/* mask for lower 16 bits */
#define m15Mask   0x7FFF	/* mask for lower 15 bits */
#define m31Mask   0x7FFFFFFF	/* mask for 31 bits */
#define m32Double 4294967295.0	/* 2^32-1 */

static short mother1[10];
static short mother2[10];
static short mStart = 1;

double Mother(unsigned long *pSeed)
{
	unsigned long number, number1, number2;
	short n, *p;
	unsigned short sNumber;

	/* Initialize motheri with 9 random values the first time */
	if (mStart) {
		sNumber = *pSeed & m16Mask;	/* The low 16 bits */
		number = *pSeed & m31Mask;	/* Only want 31 bits */

		p = mother1;
		for (n = 18; n--;) {
			number = 30903 * sNumber + (number >> 16);	/* One line multiply-with-cary */
			*p++ = sNumber = number & m16Mask;
			if (n == 9)
				p = mother2;
		}

		/* make cary 15 bits */
		mother1[0] &= m15Mask;
		mother2[0] &= m15Mask;
		mStart = 0;
	}

	/* Move elements 1 to 8 to 2 to 9 */
	memmove(mother1 + 2, mother1 + 1, 8 * sizeof(short));
	memmove(mother2 + 2, mother2 + 1, 8 * sizeof(short));

	/* Put the carry values in numberi */
	number1 = mother1[0];
	number2 = mother2[0];

	/* Form the linear combinations */
	number1 += 1941 * mother1[2] + 1860 * mother1[3] + 1812 * mother1[4] +
	    1776 * mother1[5] + 1492 * mother1[6] + 1215 * mother1[7] + 1066 * mother1[8] + 12013 * mother1[9];

	number2 += 1111 * mother2[2] + 2222 * mother2[3] + 3333 * mother2[4] +
	    4444 * mother2[5] + 5555 * mother2[6] + 6666 * mother2[7] + 7777 * mother2[8] + 9272 * mother2[9];

	/* Save the high bits of numberi as the new carry */
	mother1[0] = number1 / m16Long;
	mother2[0] = number2 / m16Long;

	/* Put the low bits of numberi into motheri[1] */
	mother1[1] = m16Mask & number1;
	mother2[1] = m16Mask & number2;

	/* Combine the two 16 bit random numbers into one 32 bit */
	*pSeed = (((long)mother1[1]) << 16) + (long)mother2[1];

	/* Return a double value between 0 and 1 */
	return ((double)*pSeed) / m32Double;
}

/* Reinitialize Mother */
void ReinitMother(void)
{
	int i = 0;
	for (i = 0; i < 10; i++) {
		mother1[i] = 0;
		mother2[i] = 0;
	}
	mStart = 1;
}

#if 1
unsigned long randGen(void)
{
	unsigned long kr = 31415926;

	Mother(&kr);

	return kr;
}

#else
int randGen(void)
{
	int i;
	unsigned long kr = 31415926;
	for (i = 0; i < 10; i++) {
		Mother(&kr);
		prints("Random value=%08X\n", (unsigned char)kr);
	}
	ReinitMother();
	kr = 31415926;
	for (i = 0; i < 10; i++) {
		Mother(&kr);
		prints("Random value2=%08X\n", (unsigned char)kr);
	}

	return 0;
}
#endif
