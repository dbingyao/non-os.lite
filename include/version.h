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
#ifndef _VERSION_H_
#define _VERSION_H_

#if defined PLATFORM_AHB
#define PLATFORM	"AHB"
#elif defined PLATFORM_AXI
#define PLATFORM	"AXI"
#elif defined PLATFORM_A320
#define PLATFORM	"A320"
#endif

#if defined RAM
#define IMGTYPE 	"RAM"
#elif defined ROM_NOR
#define IMGTYPE 	"ROM-NOR"
#elif defined ROM_SPI
#define IMGTYPE 	"ROM-SPI"
#endif

#define PRINT_IMG_VERS \
	do { \
		prints("\n   Boot mode Platform(%s-2013-06-04(%s)) \n", IMGTYPE, PLATFORM); } \
	while (0)

#endif

