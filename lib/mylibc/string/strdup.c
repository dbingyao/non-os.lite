/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <malloc.h>
#include <string.h>

char *strdup(const char *s)
{
	char *d = NULL;
	size_t len = s ? (strlen(s) + 1) : 0;

	if (len) {
		d = malloc(len);
		if (d)
			memcpy(d, s, len);
	}

	return d;
}
