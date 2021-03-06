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
#include <linux/types.h>
#include <platform.h>
#include <cp15.h>
#include <common.h>
#include <flash.h>
#include <version.h>
#include <malloc.h>

extern int uart_init(int reg_base, int baud_rate);
extern int board_init(void);

extern flash_info_t flash_info[];       /* info for FLASH chips */

void hardware_init()
{
	enable_icache();

	uart_init(UART0_REG_BASE, UART_BAUD_38400);

	board_init();

	prints("\n%s\n", PRINT_IMG_VERS);
	prints("-------------------------------------------------\n");

	mem_malloc_init();

}

void main()
{
	shell();
}
