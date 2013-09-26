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
#include <platform.h>

	.globl init_board
init_board:
	/* prepare SMC to show debug LED */
	ldr	r1, =SMC_REG_BASE + 0x18
	ldr	r2, =CONFIG_SHOW_LED_ADDR
	str	r2, [r1]

	mov	pc, lr

#if defined ROM_NOR || defined ROM_SPI
	.globl	init_ddr
init_ddr:
	adr	r3, ddr3_config_table

config_ddr:
	ldr	r1, [r3], #4
	cmp	r1, #0
	beq	ddr2_wait
	ldr	r2, [r3], #4
	str	r2, [r1]
	bne	config_ddr

ddr2_wait:
	ldr	r1, =DDRC_REG_BASE + 0x04
	ldr	r2, [r1]
	tst	r2, #0x100
	beq	ddr2_wait

	mov	pc, lr

ddr3_config_table:
	.long	DDRC_REG_BASE, 0x00074111
	.long	DDRC_REG_BASE + 0x08, 0x00040432
	.long	DDRC_REG_BASE + 0x10, 0x10005624
	.long	DDRC_REG_BASE + 0x14, 0x05010201
	.long	DDRC_REG_BASE + 0x18, 0x12111111
	.long	DDRC_REG_BASE + 0x1C, 0x41002690
	.long	DDRC_REG_BASE + 0x20, 0x00002f41
	.long	DDRC_REG_BASE + 0x24, 0x00000000
	.long	DDRC_REG_BASE + 0x34, 0x03030303
	.long	DDRC_REG_BASE + 0x38, 0x03030303
	.long	DDRC_REG_BASE + 0x3C, 0x00610011
	.long	DDRC_REG_BASE + 0x74, 0x00001111
	.long	DDRC_REG_BASE + 0x04, 0x00000001
	.long	0

	.globl	remap
remap:
	led 	0x30000000
	//adjust DDRII sdram base to 0x0 after remap
	//setup External Rank 0/1 Register
	ldr	r0, =DDRC_REG_BASE + 0x10
	ldr     r1, [r0]
	bic	r1, r1, #0x10000000
	str	r1, [r0]

	//This code will remap the memory ROM and DDR2 SDRAM.
	//DDR2 SDRAM will jump to 0x0
	ldr	r0, =BUSC_REG_BASE + REMAP_REG_OFFSET
	ldr	r1, [r0]
	orr	r1, r1, #0x1	//Set REMAP bit
	str	r1, [r0]

	mov	pc, lr
#endif

