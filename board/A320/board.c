/**
 * Copyright 2013 Faraday Technology
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

#include <types.h>
#include <div64.h>
#include <common.h>
#include <platform.h>
#include <flash.h>

#define TIMER_LOAD_VAL	0xffffffff

extern int interrupt_init(void);
extern void init_global_timer(int load_val, int use_extclk);

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
        if (banknum == 0) {     /* non-CFI boot flash */
                info->portwidth = FLASH_CFI_8BIT;
                info->chipwidth = FLASH_CFI_BY8;
                info->interface = FLASH_CFI_X8;
                return 1;
        } else
                return 0;
}

/*
 * Enable 37K Hz oscilator for timer source clock
 */
void ftpmu010_32768osc_enable(void)
{
        unsigned int oscc;

        /* enable the 32768Hz oscillator */
        oscc = inl(PMU_OSCC);
        oscc &= ~0x9;
        outl(oscc, PMU_OSCC);

        /* wait until ready */
        while (!(inl(PMU_OSCC) & 0x2))
                ;

        /* select 32768Hz oscillator */
        oscc = inl(PMU_OSCC);
        oscc |= 0x4;
        outl(oscc, PMU_OSCC);
}

void board_init(void)
{
	/* Interrupt */
	interrupt_init();

	/* Init default timer */
	ftpmu010_32768osc_enable();
	init_global_timer(TIMER_LOAD_VAL, 1);
}


