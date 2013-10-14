/*
 * newlib-2.0.0
 */

#include <string.h>
#include <limits.h>

/*SUPPRESS 560*/
/*SUPPRESS 530*/

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

#define TOO_SMALL(LEN) ((LEN) < sizeof (long))

char *strncpy(char *dst0, const char *src0, size_t count)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
	char *dscan;
	const char *sscan;

	dscan = dst0;
	sscan = src0;
	while (count > 0) {
		--count;
		if ((*dscan++ = *sscan++) == '\0')
			break;
	}
	while (count-- > 0)
		*dscan++ = '\0';

	return dst0;
#else
	char *dst = dst0;
	const char *src = src0;
	long *aligned_dst;
	const long *aligned_src;

	/* If SRC and DEST is aligned and count large enough, then copy words.  */
	if (!UNALIGNED(src, dst) && !TOO_SMALL(count)) {
		aligned_dst = (long *)dst;
		aligned_src = (long *)src;

		/* SRC and DEST are both "long int" aligned, try to do "long int"
		sized copies.  */
		while (count >= sizeof(long int) && !DETECTNULL(*aligned_src)) {
			count -= sizeof(long int);
			*aligned_dst++ = *aligned_src++;
		}

		dst = (char *)aligned_dst;
		src = (char *)aligned_src;
	}

	while (count > 0) {
		--count;
		if ((*dst++ = *src++) == '\0')
			break;
	}

	while (count-- > 0)
		*dst++ = '\0';

	return dst0;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
