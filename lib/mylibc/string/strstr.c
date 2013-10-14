/*
 * newlib-2.0.0
 */

#include <string.h>

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
# define RETURN_TYPE char *
# define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
# include "str-two-way.h"
#endif

char *strstr(const char *searchee, const char *lookfor)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

	/* Less code size, but quadratic performance in the worst case.  */
	if (*searchee == 0) {
		if (*lookfor)
			return (char *) NULL;
		return (char *) searchee;
	}

	while (*searchee) {
		size_t i;
		i = 0;

		while (1) {
			if (lookfor[i] == 0) {
				return (char *) searchee;
			}

			if (lookfor[i] != searchee[i]) {
				break;
			}
			i++;
		}
		searchee++;
	}

	return (char *) NULL;

#else /* compilation for speed */

	/* Larger code size, but guaranteed linear performance.  */
	const char *haystack = searchee;
	const char *needle = lookfor;
	size_t needle_len; /* Length of NEEDLE.  */
	size_t haystack_len; /* Known minimum length of HAYSTACK.  */
	int ok = 1; /* True if NEEDLE is prefix of HAYSTACK.  */

	/* Determine length of NEEDLE, and in the process, make sure
	   HAYSTACK is at least as long (no point processing all of a long
	   NEEDLE if HAYSTACK is too short).  */
	while (*haystack && *needle)
		ok &= *haystack++ == *needle++;
	if (*needle)
		return NULL;
	if (ok)
		return (char *) searchee;

	/* Reduce the size of haystack using strchr, since it has a smaller
	   linear coefficient than the Two-Way algorithm.  */
	needle_len = needle - lookfor;
	haystack = strchr(searchee + 1, *lookfor);
	if (!haystack || needle_len == 1)
		return (char *) haystack;
	haystack_len = (haystack > searchee + needle_len ? 1
					: needle_len + searchee - haystack);

	/* Perform the search.  */
	if (needle_len < LONG_NEEDLE_THRESHOLD)
		return two_way_short_needle((const unsigned char *) haystack,
									haystack_len,
									(const unsigned char *) lookfor, needle_len);
	return two_way_long_needle((const unsigned char *) haystack, haystack_len,
							   (const unsigned char *) lookfor, needle_len);
#endif /* compilation for speed */
}
