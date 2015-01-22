/*
 * (C) Copyright 2001-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <net_core.h>

/*
 * CPU and board-specific Ethernet initializations.  Aliased function
 * signals caller to move on
 */
static int __def_eth_init(void)
{
	return -1;
}
int board_eth_init(void) __attribute__((weak, alias("__def_eth_init")));

static struct eth_device *eth_device;
struct eth_device *eth_current;

int eth_register(struct eth_device *dev)
{
	struct eth_device *d;
	static int index;

	if (!eth_current) {
		eth_current = dev;
	} 
	dev->state = ETH_STATE_INIT;

	return 0;
}

int eth_unregister(struct eth_device *dev)
{
	struct eth_device *cur;

	/* No device */
	if (!eth_current)
		return -1;

	return 0;
}

static void eth_env_init(char *filename)
{
	if (!filename)
		copy_filename(BootFile, "zImage", sizeof(BootFile));
	else
		copy_filename(BootFile, filename, sizeof(BootFile));
}

int eth_initialize(char *filename)
{
	int num_devices = 0;
	eth_current = NULL;

	eth_env_init(filename);

	/*
	 * If board-specific initialization exists, call it.
	 * If not, call a CPU-specific one
	 */
	if (board_eth_init != __def_eth_init) {
		if (board_eth_init() < 0)
			prints("Board Net Initialization Failed\n");
		else
			num_devices++;
	} else
		prints("Net Initialization Skipped\n");

	if (!eth_current) {
		puts("No ethernet found.\n");
	}

	return num_devices;
}

int eth_send(void *packet, int length)
{
	if (!eth_current)
		return -1;

	return eth_current->send(eth_current, packet, length);
}

int eth_rx(void)
{
	if (!eth_current)
		return -1;

	return eth_current->recv(eth_current);
}

char *eth_get_name(void)
{
	return eth_current ? eth_current->name : "unknown";
}
