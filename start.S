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

#include <cpu.h>
#include <platform.h>

	.globl _stext
	.align 5
	.text
_stext:
	b       reset
	b	undefined_instruction
	b	software_interrupt
	b	prefetch_abort
	b	data_abort
	b	not_used
	b	irq
	b	fiq

reset:
	msr	cpsr_c, #PSR_F_BIT | PSR_I_BIT | SVC_MODE

	/* Disable MMU if it was enabled */
	mrc	p15, 0, r0, c1, c0, 0
	tst	r0, #P15_C1_MMU
	beq	mmu_done
	bic	r0, #P15_C1_MMU
	mcr	p15, 0, r0, c1, c0, 0

mmu_done:
#if defined ROM_NOR || defined ROM_SPI
	bl	init_board

	bl	init_ddr

	/*bl	CalibDdr //fLib_CheckMem */

	led	0x1

	/* Put memory base address to r1  */
	ldr	r2, =BUSC_REG_BASE + DDR_SDRAM_SLAVE
	ldr	r1, [r2]
	ldr	r2, =0xfff
	and	r1, r2, lsl #20

#ifdef COPY_DATA_ONLY
	/* Relocate .data section */
	ldr	r0, _lma_addr		/* append after text section */
	ldr	r3, _data_start
	ldr	r2, _data_end
	sub	r2, r2, r3
	add	r2, r0, r2		/* data end address   */
copy_data:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	blo	copy_data

	led 	0x2

	ldr	r0, _lma_addr		/* append after text section */
	bl	verify_copy
#else
	/* Copy code to ram */
	adr     r0, _stext
	ldr	r2, _data_end	/* r2 <- source end address, cover data section */

copy_text:
	ldmia   r0!, {r3-r10} 	/* copy from source address [r0] */
	stmia   r1!, {r3-r10} 	/* copy to   target address [r1] */
	cmp     r0, r2    	/* until source end address [r2] */
	blo     copy_text

	led 	0x2

	/* Verify the code */
	adr     r0, _stext
	bl	verify_copy

	bl	remap
#endif
#endif /* ROM_NOR or ROM_SPI */

	/* Show led number after copy text or data done */
#if defined ROM_SPI || defined COPY_DATA_ONLY
	led	0x5
#else
	led	0x5, LED_ADDR_REMAPPED
#endif

	/* setup stack pointer for SVC first */
	ldr	r0, _stack
	mov     sp, r0
	/* and for the rest */
	bl	setup_stacks

	/* clear .bss */
	ldr	r0, _bss_start
	ldr	r1, _bss_end

	mov	r2, #0x00000000		/* clear */

clbss_l:
	cmp     r0, r1                  /* clear loop... */
        bhs     clbss_e                 /* if reached end of bss, exit */
        str     r2, [r0]
        add     r0, r0, #4
        b       clbss_l
clbss_e:

	.align 5
	bl      hardware_init

#if defined ROM_SPI || defined COPY_DATA_ONLY
	led	0x6
#else
	led	0x6, LED_ADDR_REMAPPED
#endif
	b	main

verify_copy:
	mov	r9, lr

	/* Put memory base address to r1  */
	ldr	r3, =BUSC_REG_BASE + DDR_SDRAM_SLAVE
	ldr	r1, [r3]
	ldr	r3, =0xfff
	and	r1, r3, lsl #20

1:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	cmp	r3, r4
	bne	copy_error
	cmp	r0, r2
	blo     1b

	mov 	pc, r9

copy_error:
	led	0x40000000
2:
	b	2b

#ifdef COPY_DATA_ONLY
_lma_addr:
	.word	_sidata
#endif

_data_start:
	.word	__data_start

_data_end:
	.word	__data_end

_bss_start:
	.word	__bss_start

_bss_end:
	.word	__bss_end

	.globl _heap
_heap:
	.word	__heap_start

	.globl _stack_end
_stack_end:
	.word	__stack_end

_stack:
	.word	__stack_start

	.globl _pg_tb
_pg_tb:
	.word	__pg_tb_start

#define S_FRAME_SIZE	72

#define S_OLD_R0        68
#define S_PSR           64
#define S_PC            60
#define S_LR            56
#define S_SP            52

#define S_IP            48
#define S_FP            44
#define S_R10           40
#define S_R9            36
#define S_R8            32
#define S_R7            28
#define S_R6            24
#define S_R5            20
#define S_R4            16
#define S_R3            12
#define S_R2            8
#define S_R1            4

	.macro	save_user_regs
	sub	sp, sp, #S_FRAME_SIZE		@ carve out a frame on current
						@ user stack
	stmib	sp, {r1 - r12}			@ Save user registers (now in
						@ svc mode) r1-r12
	ldmia	r0, {r1 - r3}
        /**
	 * r1 = r0 orig
	 * r2 = lr_<exp> (original pc)
	 * r3 = spsr_<exp>
	*/
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack
	add	r5, sp, #S_SP

	str	r1, [sp]

	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr

	mov	r0, sp				@ save current stack into r0
						@ (param register)
	.endm

	.macro	restore_user_regs

	ldr	r0, [sp, #S_PSR]		@ r0 = spsr_irq
	msr	spsr_cxsf, r0
	ldmia	sp, {r0 - pc}^			@ Calling r0 - lr

	.endm

	.macro get_stack
	stmia	sp, {r0, lr}			@ save r0 and lr in position 0 and 1
						@ of saved stack
	mrs	lr, spsr			@ get the spsr
	str	lr, [sp, #8]			@ save spsr in position 2 of
						@ saved stack
	/**
	 * sp_irq -> +----------+ small address
	 *           | r0 orig  |
	 *           | lr_irq   | (origianl PC)
	 *           | spsr_irq |
	 *           +----------+ large address
	 */
	/* prepare for SVC32 mode */
	mov	r0, #PSR_F_BIT | PSR_I_BIT | SVC_MODE	@ prepare SVC-Mode
	msr	spsr, r0			@ switch modes, make sure
						@ moves will execute
	mov	r0, sp				@ r0 = sp_irq
	mov	lr, pc				@ capture return pc
	movs	pc, lr				@ jump to next instruction &
						@ switch modes.
	.endm

/*
 * exception handlers
 */
	.align	5
undefined_instruction:
	get_stack
	save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_stack
	save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	/* Correct the lr_<exp> */
	sub	lr, lr, #4
	get_stack
	save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	/* Correct the lr_<exp> */
	sub	lr, lr, #8
	get_stack
	save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_stack
	save_user_regs
	bl	do_not_used

	.align	5
irq:
	/* Correct the lr_<exp> */
	sub	lr, lr, #4
	get_stack
	save_user_regs
	bl	do_irq
	restore_user_regs

	.align	5
fiq:
	/* Correct the lr_<exp> */
	sub	lr, lr, #4
	get_stack
	/* someone ought to write a more effective fiq_save_user_regs */
	save_user_regs
	bl	do_fiq
	restore_user_regs
