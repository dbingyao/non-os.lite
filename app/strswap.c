/**
 * (C) Copyright 2013 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>

extern void swapstr(char * str1, char * str2);

char str1[24] = "str1.str1.str1.str1";
char str2[24] = "str2.str2.str2.str2";

void strswap(void)
{
	int  times = 0x10,i;
		
	for(i = 0; i < times;i++) {
		prints(" .....before.....\n");
		prints(" str1=%s\n str2=%s\n", str1, str2);

		swapstr(str1, str2);
		
		prints(" .....after.....\n");
		prints(" str1=%s\n str2=%s\n", str1, str2);
	}
}
