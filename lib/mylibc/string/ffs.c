/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dante Su: add 'clz' implementation support
 */

#include <limits.h>
#include <string.h>

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
int ffs(int i)
{
#if __LINUX_ARM_ARCH__ >= 5
	register int r;

	__asm volatile(
#if defined(__thumb__) && !defined(__thumb2__)
		".THUMB\n"
		"adr r3, 2f\n"
		"bx  r3\n"
		".ALIGN 2\n"
		".ARM\n"
		"2:\n"
#endif
		"clz %0, %1\n"
#if defined(__thumb__) && !defined(__thumb2__)
		"adr r3, 5f + 1\n"
		"bx  r3\n"
		".THUMB\n"
		"5:\n"
#endif
	: "=r"(r) /* output */
			: "r"(i)  /* input */
			: "r3"    /* clobber list */
		);

	return 32 - r;
#else  /* __LINUX_ARM_ARCH__ >= 5 */
	/* inlined binary search method */
	char n = 1;
#if UINT_MAX == 0xffffU
	/* nothing to do here -- just trying to avoiding possible problems */
#elif UINT_MAX == 0xffffffffU
	if (!(i & 0xffff)) {
		n += 16;
		i >>= 16;
	}
#else
#error ffs needs rewriting!
#endif

	if (!(i & 0xff)) {
		n += 8;
		i >>= 8;
	}
	if (!(i & 0x0f)) {
		n += 4;
		i >>= 4;
	}
	if (!(i & 0x03)) {
		n += 2;
		i >>= 2;
	}
	return (i) ? (n + ((i + 1) & 0x01)) : 0;
#endif /* __LINUX_ARM_ARCH__ >= 5 */
}
