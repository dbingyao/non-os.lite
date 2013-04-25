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

#include <common.h>
#include <flash.h>
#include "cfi_flash.h"

extern flash_info_t flash_info[];       /* info for FLASH chips */

int do_flinfo ( int argc, char * const argv[])
{
        uint32_t bank;

        if (argc == 1) {        /* print info for all FLASH banks */
                for (bank=0; bank <CONFIG_SYS_MAX_FLASH_BANKS; ++bank) {
                        prints ("\nBank # %d: ", bank+1);

                        flash_print_info (&flash_info[bank]);
                }
                return 0;
        }

        bank = str_to_hex(argv[1], 0, sizeof(bank) * 2);
        if ((bank < 1) || (bank > CONFIG_SYS_MAX_FLASH_BANKS)) {
                prints ("Only FLASH Banks # 1 ... # %d supported\n",
                        CONFIG_SYS_MAX_FLASH_BANKS);
                return 1;
        }
        prints ("\nBank # %d: ", bank);
        flash_print_info (&flash_info[bank-1]);

        return 0;
}

