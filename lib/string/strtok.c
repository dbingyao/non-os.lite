/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * stupid library routines.. The optimized versions should generally be found
 * as inline code in <asm-xx/string.h>
 *
 * These are buggy as well..
 *
 * * Fri Jun 25 1999, Ingo Oeser <ioe@informatik.tu-chemnitz.de>
 * -  Added strsep() which will replace strtok() soon (because strsep() is
 *    reentrant and should be faster). Use only strsep() in new code, please.
 */

#include <stdlib.h>
#include <string.h>

/**
 * strtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * WARNING: strtok is deprecated, use strsep instead.
 */
char * strtok(char * s, const char * ct)
{
	static char * ___strtok = NULL;
	char *sbegin, *send;

	sbegin  = s ? s : ___strtok;
	if (!sbegin) {
		return NULL;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___strtok = NULL;
		return( NULL );
	}
	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___strtok = send;
	return (sbegin);
}
