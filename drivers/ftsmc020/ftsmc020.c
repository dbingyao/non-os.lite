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

#include <stdio.h>
#include <common.h>
#include <platform.h>

#include "ftsmc020.h"


#define FTSMC020_BANK0_CONFIG   (FTSMC020_BANK_ENABLE             |     \
				 FTSMC020_BANK_BASE(PHYS_FLASH_1) |     \
				 FTSMC020_BANK_SIZE_1M            |     \
				 FTSMC020_BANK_MBW_8)

#define FTSMC020_BANK0_TIMING   (FTSMC020_TPR_RBE      |        \
				 FTSMC020_TPR_AST(3)   |        \
				 FTSMC020_TPR_CTW(3)   |        \
				 FTSMC020_TPR_ATI(0xf) |        \
				 FTSMC020_TPR_AT2(3)   |        \
				 FTSMC020_TPR_WTC(3)   |        \
				 FTSMC020_TPR_AHT(3)   |        \
				 FTSMC020_TPR_TRNA(0xf))

#if 0
#define FTSMC020_BANK1_CONFIG   (FTSMC020_BANK_ENABLE             |     \
				 FTSMC020_BANK_BASE(PHYS_FLASH_2) |     \
				 FTSMC020_BANK_SIZE_32M           |     \
				 FTSMC020_BANK_MBW_32)

#define FTSMC020_BANK1_TIMING   (FTSMC020_TPR_AST(3)   |        \
				 FTSMC020_TPR_CTW(3)   |        \
				 FTSMC020_TPR_ATI(0xf) |        \
				 FTSMC020_TPR_AT2(3)   |        \
				 FTSMC020_TPR_WTC(3)   |        \
				 FTSMC020_TPR_AHT(3)   |        \
				 FTSMC020_TPR_TRNA(0xf))
#endif

#define CONFIG_SYS_FTSMC020_CONFIGS     {                       \
	{ FTSMC020_BANK0_CONFIG, FTSMC020_BANK0_TIMING, },      \
}
#if 0
	{ FTSMC020_BANK1_CONFIG, FTSMC020_BANK1_TIMING, },      \
}
#endif

struct ftsmc020_config {
	unsigned int    config;
	unsigned int    timing;
};

static void ftsmc020_setup_bank(unsigned int bank, struct ftsmc020_config *cfg)
{
	struct ftsmc020 *smc = (struct ftsmc020 *)SMC_REG_BASE;

	if (bank > 3) {
		prints("bank # %u invalid\n", bank);
		return;
	}

	prints("bank %d: config 0x%x = 0x%x\n", bank, &smc->bank[bank].cr, cfg->config);
	prints("bank %d: timing 0x%x = 0x%x\n", bank, &smc->bank[bank].tpr, cfg->timing);

	outl(cfg->config, &smc->bank[bank].cr);
	outl(cfg->timing, &smc->bank[bank].tpr);
}

void ftsmc020_init(void)
{
	struct ftsmc020_config config[] = CONFIG_SYS_FTSMC020_CONFIGS;
	int i;

	for (i = 0; i < ARRAY_SIZE(config); i++)
		ftsmc020_setup_bank(i, &config[i]);
}

