#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

static inline int ffs(int x)
{
#if __LINUX_ARM_ARCH__ < 5
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	
	return r;
#else	/* __ARMV4__ */
	register int r;
	
	__asm__ __volatile__ (
#ifdef __THUMB__
		".THUMB\n"
		"adr r3, 2f\n"		/* Get address of label 2 */
		"bx r3\n"
		".ALIGN 2\n"			/* This is required before all ARM code. */
		".ARM\n"
		"2:\n"
#endif	// #ifdef __THUMB__		
		"clz %0, %1\n"
#ifdef __THUMB__
		"adr r3, 5f + 1\n"		/* Get address of label 5 + 1 (Thumb Mode) */
		"bx r3\n"
		".THUMB\n"
		"5:\n"
		"nop\n"
#endif	// #ifdef __THUMB__
		: "=r"(r)                /* output */
		: "r"(x)                 /* input */
		: "r3"                   /* clobber list */
		);
		
	return 32 - r;
#endif	/* __ARMV4__ */
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

#endif
