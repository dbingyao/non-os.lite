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
	/* Disable X2P Slave error */
	ldr     r0, =AXI2APB_REG_BASE
	ldr     r1, [r0,#X2P_INTR_OFFSET]
	orr     r1, r1, #0x2
	str     r1, [r0,#X2P_INTR_OFFSET]

	/* Disable DMAC APB error response */
	ldr     r0, =FTDMAC030_REG_BASE
	ldr     r1, =0
	str     r1, [r0,#DMAC030_PSIVERR_OFFSET]

	/* Disable AXIC APB error response */
	ldr     r0, =FTAXIC030_REG_BASE
	ldr     r1, [r0,#AXIC_CTR_REG]
	orr     r1, r1, #0x8
	str     r1, [r0,#AXIC_CTR_REG]

#ifndef CONFIG_FTSATA100
	/* prepare SMC to show debug LED */
	ldr	r1, =SMC_REG_BASE + 0x18
	ldr	r2, =CONFIG_SHOW_LED_ADDR
	str	r2, [r1]
#endif

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
#ifdef CONFIG_FTSATA100
	.long	DDRC_REG_BASE, 0x00039e01
	.long	DDRC_REG_BASE + 0x08, 0x00040432
	.long	DDRC_REG_BASE + 0x10, 0x10015656
	.long	DDRC_REG_BASE + 0x14, 0x14050604
	.long	DDRC_REG_BASE + 0x18, 0x22411323
	.long	DDRC_REG_BASE + 0x1C, 0x020026a0
	.long	DDRC_REG_BASE + 0x20, 0x00000f41
	.long	DDRC_REG_BASE + 0x34, 0x03030303
	.long	DDRC_REG_BASE + 0x38, 0x03030303
	.long	DDRC_REG_BASE + 0x74, 0x11111111
#else
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
#endif
	.long	DDRC_REG_BASE + 0x04, 0x00000001
	.long	0


	.globl	CalibDdr
CalibDdr:
	mov	r11, lr
	/* copy 1KB data from ROM */
	ldr	r0, =0xb0000000
	ldr	r1, =0x00000000
	add	r2, r1, #0x400
1:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r1, r2			/* until target end address [r2]    */
	blo	1b

	/**
	 * 1) FPGA: Use fixed msdly, like:0x11111111
	 *    ASIC: Find out msdly value.
	 * 2) Use previous step msdly value, go through
	 *    all gds, and select the middle.
	 * 3) The value of phy_rdlat 2 to 11
	 * ------------------------------------------------
	 * r5: gds,    r6: phy_rdlat,     r7: mdsly
	 * r9: gds_list
	 * r10: 0xff means finding msdly value, after that
	 *      is used to put phy_rdlat value.
	 */
	mov	r10, #0xff
	mov	r7, #0 /* msdly = 0 */
find_gds:
	mov	r5, #0  /* gds = 0 */
	mov	r6, #2  /* phy_rdlat = 2 */

set_gds:
	ldr	r4, =0xA0300000
	ldr	r3, [r4]
	bic	r3, r3, #7
	orr	r3, r3, r5
	str	r3, [r4]

set_rdlat:
	ldr	r4, =0xA030003c
	ldr	r3, [r4]
	bic	r3, r3, #0xf00000
	orr	r3, r3, r6, lsl #20
	str	r3, [r4]

set_msdly:
	ldr	r4, =0xA0300074
	mov	r3, r7
	orr	r3, r3, r7, lsl #8
	orr	r3, r3, r7, lsl #16
	orr	r3, r3, r7, lsl #24
	str	r3, [r4]

check:
	ldr	r0, =0xb0000000
	ldr	r1, =0x00000000
	/* Dummy write after set msdly */
	ldr	r3, [r0]
	str	r3, [r1]
2:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	cmp	r3, r4
	bne	next_msdly
	cmp	r1, r2
	blo	2b

	/* this msdly value OK */
	cmp	r10, #0xff
	addeq	r7, r7, #0x11

	/* we are finding gds now.
	   Use r10 to save rdlat value for each gds.
	   r10[3:0] for gds 0, r10[7:4] for gds 1.
	 */
	moveq	r9, #0
	moveq	r10, #0
	beq	find_gds

	/* this gds value OK */
	mov	r3, #1
	mov	r3, r3, lsl r5
	orr	r9, r9, r3

	/* keep phy_rdlat value for this gds */
	mov	r4, r5, lsl #2
	mov	r3, r6, lsl r4
	orr	r10, r10, r3

	/* Go to next gds if less than or equal to 7 */
	cmp	r5, #7
	movle	r6, #2
	ble	next_gds

	/* msdly: from 0 to 0xf
	   FPGA is one byte, ASIC from byte 0 to 7
	 */
next_msdly:
	/* If r10 is not 0xff, it means we are
	   finding gds not msdly, so go to next rdlat
	 */
	cmp	r10, #0xff
	bne	next_rdlat

	add	r7, r7, #0x11
	cmp	r7, #0xff
	ble	set_msdly
	mov	r7, #0

next_rdlat:
	add	r6, r6, #1
	cmp	r6, #11
	ble	set_rdlat
	mov	r6, #2

	/* GDS: from 0 to 7 */
next_gds:
	add	r5, r5, #1
	cmp	r5, #0x7
	ble	set_gds

	/* Calibration done, select gds value from gds_list (r9) */
	mov	r0, #0xf
	ands	r1, r9, #0xf
	beq	3f

	mov	r0, #0
	movs	r2, r1, lsr #2
	movne	r1, r2
	addne	r0, #2

	movs	r2, r1, lsr #1
	movne	r1, r2
	addne	r0, #1

3:
	cmp	r0, #0xf
	bne	4f
	/* no gds found at r9[3:0]
	   search r9[7:4] */
	ands	r1, r9, #0xf0
	beq	err

	mov	r0, #4
	mov	r1, r1, lsl #24
	movs	r2, r1, lsl #2
	movne	r1, r2
	addeq	r0, #2

	movs	r2, r1, lsl #1
	addeqs	r0, #1

4:
	/* set gds to hardware */
	ldr	r4, =0xA0300000
	ldr	r3, [r4]
	bic	r3, r3, #7
	orr	r3, r3, r0
	str	r3, [r4]

	/* take phy_rdlat value for this gds */
	mov	r4, r0, lsl #2
	mov	r6, r10, lsr r4
	and	r6, r6, #0xf

	ldr	r4, =0xA030003c
	ldr	r3, [r4]
	bic	r3, r3, #0xf00000
	orr	r3, r3, r6, lsl #20
	str	r3, [r4]

	mov	pc, r11

	/* Can't find correct gds, rdlat and msdly value.
	 * Adjust dll_sel, this is not the process to choose
	 * dll_sel middle value, just find one dll_sel to
	 * setup gds, rdlat and msdly value.
	 *
	 * Note:
	 * This will be infinite loop if no suitable value is
	 * find.
	 */
err:
	ldr	r4, =0xA0300024
	ldr	r3, [r4]
	and	r3, r3, #0x7
	cmp	r3, #0x7
	addne	r3, r3, #0x1
	bne	set_dll

	/* dll_sell value reach max(0x7), flip the
	   control bit, so that if DQ is fixed, change
	   to fix DQS and vice versa.
	   1 = Read DQ is fixed, delay DQS.
	   0 = Read DQS is fixed, delay DQ.
	*/
	mov	r3, #0x0
	ldr	r2, =0xA030006c
	ldr	r1, [r2]
	eor	r1, r1, #0x11
	eor	r1, r1, #0x1100
	str	r1, [r2]

set_dll:
	orr	r3, r3, r3, lsl #4
	orr	r3, r3, r3, lsl #8
	str	r3, [r4]

	/* Very bad, have to go through above process again*/
	mov	r10, #0xff
	mov	r7, #0 /* msdly = 0 */
	b	find_gds

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

