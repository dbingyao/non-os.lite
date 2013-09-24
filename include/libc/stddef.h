/*
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#ifndef _STDDEF_H
#define _STDDEF_H

typedef int             ptrdiff_t;
typedef unsigned short  wchar_t;
typedef unsigned long   size_t;
typedef long            ssize_t;

#define PTRDIFF_MAX     0x7fffffff
#define PTRDIFF_MIN     (-PTRDIFF_MAX - 1)

#define WCHAR_MAX       0xffff
#define WCHAR_MIN       0

#define SIZE_MAX        0xffffffff
#define SIZE_MIN        0

#define SSIZE_MAX       0x7fffffff
#define SSIZE_MIN       (-SSIZE_MAX - 1)

#undef  NULL
#define NULL ((void *)0)

#undef  offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#endif
