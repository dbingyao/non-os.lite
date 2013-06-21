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

	.globl led_addr
led_addr:
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

	.globl	fLib_CheckMem
fLib_CheckMem:
        MOV        r9, lr

        //copy 1KB data from ROM
        LDR  r1, =0x10000000
        LDR  r3, =0x0
        ADD r2, r1, #0x400
1:
        LDR  r0, [r3], #4
        STR  r0, [r1], #4
        CMP r1, r2
        BLO 1b

        LDR  r5, =0x11111111
        MOV        r6, #0
        MOV        r7, #2

        // msdly: It's fixed to 0x11111111
        LDR  r4, =0xA0300074
        STR  r5, [r4]

set_gds:
        // GDS: from 0 to 3
        LDR  r4, =0xA0300000
        LDR  r3, [r4]
        BIC   r3, r3, #3
        ORR r3, r6, r3
        STR  r3, [r4]

set_rdlat:
        // phy_rdlat: from 2 to 11
        LDR  r4, =0xA030003C
        LDR  r3, [r4]
        BIC  r3, r3, #0xf00000
		MOV  r8, r7, lsl #20
        ORR  r3, r8, r3
        STR  r3, [r4]
check:
        LDR  r1, =0x10000000
        LDR  r4, =0x0
2:
        LDR  r3, [r1], #4
        LDR  r0, [r4], #4
        CMP r0, r3
        BNE next_rdlat
        CMP r1, r2
        BLO 2b

        // success & return
        MOV        pc, r9

next_rdlat:
        ADD r7, r7, #1
        CMP r7, #12
        BLO set_rdlat
        MOV        r7, #2
next_gds:
        ADD r6, r6, #1
        CMP r6, #4
        BLO set_gds
        MOV        r6, #0
err:
        B      err


	.globl remap
remap:
	led 	0x3
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

