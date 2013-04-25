/*
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

#include <stdio.h>

#include <cpu.h>
#include <common.h>
#include <platform.h>

int intr_reg_base;

void irq_edge_clear(int irq)
{
	*((volatile int *)(intr_reg_base + 0x8)) = (1 << irq);
}

void irq_set_enable(int irq)
{
	int enable;

	enable = *((volatile int *)(intr_reg_base + 0x4));
	enable |= (1 << irq);

	*((volatile int *)(intr_reg_base + 0x4)) = enable;
}

void irq_set_disable(int irq)
{
	int enable;

	enable = *((volatile int *)(intr_reg_base + 0x4));
	enable &= ~(1 << irq);

	*((volatile int *)(intr_reg_base + 0x4)) = enable;
}

int irq_set_type(int irq, unsigned int type)
{
        int mode, level;

	if (irq >= NR_IRQS)
		return -1;

	/* Trigger Mode: 0 - Level, 1 - Edge */
	mode = *((volatile int *)(intr_reg_base + 0xC));

	/* Trigger level: 0 - Active High, 1 - Active Low or Falling Edge */
	level = *((volatile int *)(intr_reg_base + 0x10));

	switch (type) {
	case IRQ_TYPE_LEVEL_LOW:
		level |= (1 << irq);
		/* fall through */

	case IRQ_TYPE_LEVEL_HIGH:
		break;

	case IRQ_TYPE_EDGE_FALLING:
		level |= (1 << irq);
		/* fall through */

	case IRQ_TYPE_EDGE_RISING:
		mode |= (1 << irq);
		break;

	default:
		return -1;
	}

	*((volatile int *)(intr_reg_base + 0xC)) = mode;

	*((volatile int *)(intr_reg_base + 0x10)) = level;

	return 0;
}


int interrupt_hw_init(void)
{
	intr_reg_base = INTC_REG_BASE;
	
	prints(" %s: reg base 0x%08x\n", __func__, intr_reg_base);

	/* configure interrupts for IRQ mode */
	*((volatile int *)(intr_reg_base + 0x4)) = 0;
	/* Interrupt Clear */
	*((volatile int *)(intr_reg_base + 0x8)) = ~0;
	/* Trigger Mode: 0 - Level, 1 - Edge */
	*((volatile int *)(intr_reg_base + 0xC)) = 0;
	/* Trigger level: 0 - Active High, 1 - Active Low */
	*((volatile int *)(intr_reg_base + 0x10)) = 0;

	return 0;
}

