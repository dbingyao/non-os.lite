
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

#if defined ROM_NOR || defined ROM_SPI
.macro  wait_sdram
	ldr	r0, =DDRC_REG_BASE
1:
	ldr     r1, [r0, #0x8]
	cmp     r1, #0
	bne     1b                                                                                                                                           
.endm

	.globl init_board
init_board:
	/*
	 * flush v4 I/D caches
	 */
	mov     r0, #0
	mcr     p15, 0, r0, c7, c7, 0   /* flush v3/v4 cache */
	mcr     p15, 0, r0, c8, c7, 0   /* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc     p15, 0, r0, c1, c0, 0
	bic     r0, r0, #0x00002300     @ clear bits 13, 9:8 (--V- --RS)
	bic     r0, r0, #0x00000087     @ clear bits 7, 2:0 (B--- -CAM)
	orr     r0, r0, #0x00000002     @ set bit 2 (A) Align
	orr     r0, r0, #0x00001000     @ set bit 12 (I) I-Cache
	mcr     p15, 0, r0, c1, c0, 0

	mov	pc, lr

	.globl	init_ddr
init_ddr:
	led     0x10

	ldr	r0, =DDRC_REG_BASE 

	/* set SDRAM register */
	ldr	r1, =0x00211312
	str	r1, [r0]
	led     0x11

	ldr	r1, =0x00480180
	str	r1, [r0, #0x4]
	led     0x12

        /* set to precharge */
	ldr	r1, =0x10
	str	r1, [r0, #0x8]
	led     0x13

        wait_sdram
        led     0x14

        /* set mode register */
	ldr	r1, =0x4
	str	r1, [r0, #0x8]
        led     0x15

        wait_sdram
        led     0x16

        /* set to refresh */
	ldr	r1, =0x8
	str	r1, [r0, #0x8]
	led     0x17

	wait_sdram
	led     0x18

	ldr	r1, =0x11002326
	str	r1, [r0, #0xC]

	led     0x19

	ldr	r1, =0x18
	str	r1, [r0, #0x34]

	led     0x1a
	mov     pc, lr

/*
 * This code will remap the memory ROM and SDRAM
 * ROM will be placed on 0x80000000 and SDRAM will jump to 0x0
 */
	.globl remap
remap:
	/* first adjust sdram */
	ldr	r0, =DDRC_REG_BASE + 0xC
	ldr	r1, =0x10002326
	str	r1, [r0]

	/* then remap */
	ldr	r0, =BUSC_REG_BASE + REMAP_REG_OFFSET
	ldr	r1, [r0]
	orr	r1, r1, #0x1	//Set REMAP bit
	str	r1, [r0]

	mov     pc, lr
#endif
