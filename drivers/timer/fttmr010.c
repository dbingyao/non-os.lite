/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
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
#include <platform.h>

#include "fttmr010.h"

struct _time_handler {
	void (*m_func)(void);
};

static void fttmr010_t2_isr(void *data);
static void fttmr010_t3_isr(void *data);

static struct _time_handler timer_handler[3];

static unsigned long long tbu, tbl;
static int timer_load_val[3];
volatile struct fttmr010 * tmr = (struct fttmr010 *) TIMERC_REG_BASE;

#if 0
int timer_init(void)
{
	unsigned int cr;

	tmr = (struct fttmr010 *) TIMERC_REG_BASE;

	prints(" %s: reg base 0x%08x\n", __func__, (int) tmr);

	/* disable timers */
	tmr->cr = 0;

	/* setup timer */
	tmr->cfg[0].timer_load = TIMER_LOAD_VAL;
	tmr->cfg[0].timer_counter = TIMER_LOAD_VAL;
	tmr->cfg[0].timer_match1 = 0;
	tmr->cfg[0].timer_match2 = 0;

	/* we don't want timer to issue interrupts */
	tmr->interrupt_mask = FTTMR010_TM1_MATCH1 |
	       FTTMR010_TM1_MATCH2 |
	       FTTMR010_TM1_OVERFLOW;

	cr = tmr->cr;
	cr |= (FTTMR010_TM1_ENABLE | FTTMR010_TM1_CLOCK);
	tmr->cr = cr;

	tbu = tbl = 0;

	irq_set_type(TIMERC_IRQ2, IRQ_TYPE_EDGE_RISING);
	irq_install_handler(TIMERC_IRQ2, fttmr010_t2_isr, 0);
	irq_set_enable(TIMERC_IRQ2);

	irq_set_type(TIMERC_IRQ3, IRQ_TYPE_EDGE_RISING);
	irq_install_handler(TIMERC_IRQ3, fttmr010_t3_isr, 0);
	irq_set_enable(TIMERC_IRQ3);

	return 0;
}
#endif

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	ulong now = timer_load_val[0] - tmr->cfg[0].timer_counter;

	/* increment tbu if tbl has rolled over */
	if (now < tbl)
		tbu++;
	tbl = now;
	return (((unsigned long long)tbu) << 32) | tbl;
}

static void fttmr010_t2_isr(void *data)
{
	int sts;

	tmr->cr &= ~FTTMR010_TM2_ENABLE;

	/* clear interrupt status */
	irq_edge_clear(TIMERC_IRQ2);

	if (timer_handler[1].m_func)
		timer_handler[1].m_func();

	tmr->cr |= FTTMR010_TM2_ENABLE;
}

static void fttmr010_t3_isr(void *data)
{
	int sts;

	tmr->cr &= ~FTTMR010_TM3_ENABLE;

	/* clear interrupt status */
	irq_edge_clear(TIMERC_IRQ3);

	if (timer_handler[2].m_func)
		timer_handler[2].m_func();

	tmr->cr |= FTTMR010_TM3_ENABLE;
}

void disable_timer(int timer_index)
{
	int idx;

	if (timer_index < 1 || timer_index > 3) {
		prints("%s: index out of range\n", __func__);
		return 1;
	}

	idx = timer_index - 1;

	tmr->cfg[idx].timer_load =
	tmr->cfg[idx].timer_counter =
	tmr->cfg[idx].timer_match2 =
	tmr->cfg[idx].timer_match1 = 0;

	tmr->cr &= ~(FTTMR010_TM1_ENABLE << (idx * 3));
}

/**
 * Init timer for app
 */
int init_timer(int timer_index, int load_val, int use_extclk, void  timer_callback(void))
{
	int idx, cr;

	if (timer_index < 1 || timer_index > 3) {
		prints("%s: index out of range\n", __func__);
		return 1;
	}

	prints(" %s: index %d load val %d\n", __func__, timer_index, load_val);

	idx = timer_index - 1;

	timer_load_val[idx] = load_val;

	/* setup timer */
	tmr->cfg[idx].timer_load =
	tmr->cfg[idx].timer_counter = load_val;
	tmr->cfg[idx].timer_match2 = 
	tmr->cfg[idx].timer_match1 = 0;

	if (timer_callback) {

		timer_handler[idx].m_func = timer_callback;

		tmr->interrupt_mask &= ~((FTTMR010_TM1_MATCH1 |
		       FTTMR010_TM1_MATCH2 |
		       FTTMR010_TM1_OVERFLOW) << (3 * idx));

		/* Unmask CPU CPSR I bit */
		enable_interrupts();
	} else {
		tmr->interrupt_mask |= ((FTTMR010_TM1_MATCH1 |
		       FTTMR010_TM1_MATCH2 |
		       FTTMR010_TM1_OVERFLOW) << (3 * idx));
	}

	cr = FTTMR010_TM1_ENABLE;

	if (use_extclk)
		cr |= FTTMR010_TM1_CLOCK;

	/* Enable timer and count down */
	tmr->cr |= (cr << (3 * idx)) ;

	return 0;
}


void init_global_timer(int load_val, int use_extclk)
{

	init_timer(0, load_val, use_extclk, 0);

	irq_set_type(TIMERC_IRQ2, IRQ_TYPE_EDGE_RISING);
	irq_install_handler(TIMERC_IRQ2, fttmr010_t2_isr, 0);
	irq_set_enable(TIMERC_IRQ2);

	irq_set_type(TIMERC_IRQ3, IRQ_TYPE_EDGE_RISING);
	irq_install_handler(TIMERC_IRQ3, fttmr010_t3_isr, 0);
	irq_set_enable(TIMERC_IRQ3);
}
