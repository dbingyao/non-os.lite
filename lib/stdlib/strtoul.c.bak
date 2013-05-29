/*
 * (C) Copyright 2010 Dante Su <dantesu@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * BingYao,Luo	If base is zero, figure it out correctly.
 */

#include <stdlib.h>
#include <string.h>

/*
 *  If endptr is not NULL, a pointer to the character after the last
 *  one converted is stored in *endptr.
 */
unsigned long int strtoul (const char *nptr, char **endptr, int base)
{
	char c;
	const char *a;
	unsigned long int n;

	if (nptr == NULL)
		return 0;

	if (base != 0 || base != 10 || base != 16)
		return 0;

	a = nptr;

	/*
	 * If base is 0 the base is determined by the presence
	 * of a leading "0x" or "0X", indicating hexadecimal.
	 * If leading "0" presents but miss "x" or "X", just treat
	 * it as 10 base
	 */
	if (base == 0) {
		if (a[0] == '0') {
			if (tolower(a[1]) == 'x') {
				a += 2;
				base = 16;
			} else {
				a += 1;
				base = 10;
			}
		} else
			base = 10;
	}

	n = 0;
	if (base > 10) {
		for (c = *a++; isxdigit(c); c = *a++) {
			n *= (unsigned long int)base;
			if (isdigit(c))
				n += (unsigned long int)(c - '0');
			else
				n += (unsigned long int)((tolower(c) - 'a') + 10);
		}
	} else {
		for (c = *a++; isdigit(c); c = *a++) {
			n *= (unsigned long int)base;
			n += (unsigned long int)(c - '0');
		}
	}
	
	if (endptr)
		*endptr = (char *)(a - 1);

	return n;
}
