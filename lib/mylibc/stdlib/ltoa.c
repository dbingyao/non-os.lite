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
 */

#include <stdlib.h>

static char buf[12] = { 0 };
static const char *hex = "0123456789ABCDEF";

char *ltoa(long int value, char *nptr, int base)
{
	unsigned int ret = 0;
	int i, len = 0, off = 0;
	char tmp[12];
	
	if (nptr == NULL)
		nptr = buf;

	if (value == 0) {
		tmp[len++] = '0';
	} else {
		if (value < 0) {
			off = 1;
			nptr[0] = '-';
			value = -1 * value;
		}
		for(; value > 0; value /= base) {
			ret = value % base;
			tmp[len++] = hex[ret];
		}
	}
	tmp[len] = 0;
	
	for (i = 0; i < len; ++i) 
		nptr[i + off] = tmp[len - 1 - i];
	nptr[len + off] = 0;	

	return nptr;
}
