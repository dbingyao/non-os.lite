/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <cpu.h>
#include <types.h>
#include <common.h>
#include <platform.h>

extern int intr_reg_base;

struct stack {
	unsigned int fiq[3];
	unsigned int irq[3];
	unsigned int abt[3];
	unsigned int und[3];
} __attribute__((__aligned__(32))); 

static struct stack stacks;

struct _irq_handler {
	void *m_data;
	void (*m_func)(void *data);
};

static struct _irq_handler IRQ_HANDLER[NR_IRQS];

static void default_isr(void *data)
{
	prints("\n default_isr():  called for IRQ %d\n",
	       (int)data);

}

int interrupt_init(void)
{
	int i;

	/* install default interrupt handlers */
	for (i = 0; i < NR_IRQS; i++)
		irq_install_handler(i, default_isr, (void *)i);

	interrupt_hw_init();

	return 0;
}

int do_irqinfo (void)
{
	int i;

	puts ("\nInterrupt-Information:\n\n"
		"Nr  Routine   Arg       Count\n"
		"-----------------------------\n");

	for (i = 0; i < NR_IRQS; i++) {
		if (IRQ_HANDLER[i].m_func != (interrupt_handler_t*) default_isr) {
			prints ("%02d  %08x  %08x \n", i,
				(int)IRQ_HANDLER[i].m_func, (int)IRQ_HANDLER[i].m_data);
		}
	}
	puts ("\n");
	return 0;
}

void setup_stacks(void)
{
	struct stack *stk = &stacks;

	/*
	* setup stacks for re-entrant exception handlers
	*/
	__asm__ (
	"msr    cpsr_c, %1\n\t"
	"add    r14, %0, %2\n\t"
	"mov    sp, r14\n\t"
	"msr    cpsr_c, %3\n\t"
	"add    r14, %0, %4\n\t"
	"mov    sp, r14\n\t"
	"msr    cpsr_c, %5\n\t"
	"add    r14, %0, %6\n\t"
	"mov    sp, r14\n\t"
	"msr    cpsr_c, %7\n\t"
	"add    r14, %0, %8\n\t"
	"mov    sp, r14\n\t"
	"msr    cpsr_c, %9"
		:
		: "r" (stk),
		"I" (PSR_F_BIT | PSR_I_BIT | FIQ_MODE),
		"I" (offsetof(struct stack, fiq[0])),
		"I" (PSR_F_BIT | PSR_I_BIT | IRQ_MODE),
		"I" (offsetof(struct stack, irq[0])),
		"I" (PSR_F_BIT | PSR_I_BIT | ABT_MODE),
		"I" (offsetof(struct stack, abt[0])),
		"I" (PSR_F_BIT | PSR_I_BIT | UND_MODE),
		"I" (offsetof(struct stack, und[0])),
		"I" (PSR_F_BIT | PSR_I_BIT | SVC_MODE)
		: "r14");
}

/* enable IRQ interrupts */
void enable_interrupts (void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "bic %0, %0, #0x80\n"
			     "msr cpsr_c, %0"
			     : "=r" (temp)
			     :
			     : "memory");
}


/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */
int disable_interrupts (void)
{
	unsigned long old,temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0xc0\n"
			     "msr cpsr_c, %1"
			     : "=r" (old), "=r" (temp)
			     :
			     : "memory");
	return (old & 0x80) == 0;
}

void bad_mode (void)
{
	prints ("Resetting CPU ...\n");

	while (1) {;}
}

void show_regs (struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] = {
	"USER_26",	"FIQ_26",	"IRQ_26",	"SVC_26",
	"UK4_26",	"UK5_26",	"UK6_26",	"UK7_26",
	"UK8_26",	"UK9_26",	"UK10_26",	"UK11_26",
	"UK12_26",	"UK13_26",	"UK14_26",	"UK15_26",
	"USER_32",	"FIQ_32",	"IRQ_32",	"SVC_32",
	"UK4_32",	"UK5_32",	"UK6_32",	"ABT_32",
	"UK8_32",	"UK9_32",	"UK10_32",	"UND_32",
	"UK12_32",	"UK13_32",	"UK14_32",	"SYS_32",
	};

	flags = condition_codes (regs);

	prints ("pc : [<%08lx>]	   lr : [<%08lx>]\n"
		"sp : %08lx  ip : %08lx	 fp : %08lx\n",
		instruction_pointer (regs),
		regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	prints ("r10: %08lx  r9 : %08lx	 r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	prints ("r7 : %08lx  r6 : %08lx	 r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	prints ("r3 : %08lx  r2 : %08lx	 r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	prints ("Flags: %c%c%c%c",
		flags & CC_N_BIT ? 'N' : 'n',
		flags & CC_Z_BIT ? 'Z' : 'z',
		flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	prints ("  IRQs %s  FIQs %s  Mode %s%s\n",
		interrupts_enabled (regs) ? "on" : "off",
		fast_interrupts_enabled (regs) ? "on" : "off",
		processor_modes[processor_mode (regs)],
		thumb_mode (regs) ? " (T)" : "");
}

void do_undefined_instruction (struct pt_regs *pt_regs)
{
	prints ("undefined instruction\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_software_interrupt (struct pt_regs *pt_regs)
{
	prints ("software interrupt\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_prefetch_abort (struct pt_regs *pt_regs)
{
	prints ("prefetch abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_data_abort (struct pt_regs *pt_regs)
{
	prints ("data abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_not_used (struct pt_regs *pt_regs)
{
	prints ("not used\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_fiq (struct pt_regs *pt_regs)
{
	prints ("fast interrupt request\n");
	show_regs (pt_regs);
	bad_mode ();
}

void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data)
{
	if (irq >= NR_IRQS || !handle_irq)
		return;

	IRQ_HANDLER[irq].m_data = data;
	IRQ_HANDLER[irq].m_func = handle_irq;
}

void do_irq (struct pt_regs *pt_regs)
{
	int irq, vintc_is;

	/* Prevent Nested interrupts */
	disable_interrupts();

	vintc_is = *((volatile int *) (intr_reg_base + 0x14));

	irq = 0;
	while (vintc_is) {
		int tmp;

		tmp = (1 << irq);
		if (vintc_is & tmp)
			IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);

		vintc_is &= ~tmp;

		irq ++;
	}

	enable_interrupts();
}
