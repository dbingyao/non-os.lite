/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

char *strsep(char **s1, const char *s2)
{
	register char *s = *s1;
	register char *p = NULL;

	if (s && *s && (p = strpbrk(s, s2)))
		* p++ = 0;

	*s1 = p;
	return s;
}
