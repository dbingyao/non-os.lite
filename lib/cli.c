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

#include <stdlib.h>
#include <stdio.h>

#include <common.h>
#include <net_core.h>
#include <types.h>
#include <platform.h>


#define FMT_UINT	"%08X"

#if defined(CONFIG_FTSPI020)
extern int FTSPI020_main(int argc, char * const argv[]);
#endif
#if defined(CONFIG_FTSATA100)
extern int FTSATA100_main(int argc, char * const argv[]);
#endif
#if defined(CONFIG_FTGMAC030)
extern int FTGMAC030_main(int argc, char * const argv[]);
#endif
extern int do_spi_bootmode_test(int argc, char * const argv[]);
extern int enable_d_cache(int argc, char * const argv[]);
extern int disable_d_cache(int argc, char * const argv[]);

int do_rd32(int argc, char * const  argv[]);
int do_wr32(int argc, char * const  argv[]);
int do_tftpboot(int argc, char * const  argv[]);
int do_ping(int argc, char * const  argv[]);
int do_print_irqinfo(int argc, char * const  argv[]);
int do_help(int argc, char * const argv[], cmd_t * tbl);

cmd_t main_cmd_tbl[] = {
			{"md", "<addr> [num]", do_rd32},
			{"mw", "<addr> <data> [num [inc]]", do_wr32},
			{"irq","Interrupt information", do_print_irqinfo},
#if defined(CONFIG_FTSPI020)
			{"spi","FTSPI020 commands mode", FTSPI020_main},
#endif
#if defined(CONFIG_FTSATA100)
			{"satah","FTSATA100 commands mode", FTSATA100_main},
#endif
#if defined(CONFIG_FTGMAC030)
			{"gmac","FTGMAC030 commands mode", FTGMAC030_main},
#endif
			{"spt", "Code on SPI flash test", do_spi_bootmode_test},
			{"en_cache", "Enable D-cache and I-cache", enable_d_cache},
			{"dis_cache", "Disable D-cache and I-cache", disable_d_cache},
			{"tftpboot", "<filename> [loadaddr] [serverip]", do_tftpboot},
			{"ping", "<ipaddr>", do_ping},
			{"?", "print all cmds", 0},
			{0}
			};

#define BACKSP_KEY 0x08
#define RETURN_KEY 0x0A
#define DELETE_KEY 0x7F
#define BELL       0x07

int scan_string(char *buf)
{
	char    *cp;
	char    ch;
	int  count;

	count = 0;
	cp = buf;

	do {
		ch = ftuart_getc();

		switch(ch) {
		case RETURN_KEY:
			if(count < 256) {
				*cp = '\0';
				ftuart_putc('\n');
			}
			break;
		case BACKSP_KEY:
		case DELETE_KEY:
			if(count) {
				count--;
				*(--cp) = '\0';
				puts("\b \b");
			}
			break;
		default:
			if( ch > 0x1F && ch < 0x7F && count < 256) {
				*cp = (char) ch;
				cp++;
				count++;
				ftuart_putc(ch);
			}
			break;
        	}

	} while(ch != RETURN_KEY);

	return count;
}

/* Usage: md <addr> [num] */
int do_rd32(int argc, char * const  argv[])
{
	int i, j;
	int addr;
	int num;
	int *pd;
	int data;

	if ((argc < 2) || (argc > 3))
		return 1;

	addr = strtol(argv[1], 0, 0);

	if (argc == 3)
		num = strtol(argv[2], 0, 0);
	else
		num = 4;

	pd = (int *) addr;
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (int) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return (0);
}

/* Usage: mw <addr> <data> [num [inc]] */
int do_wr32(int argc, char * const  argv[])
{
	int i, j;
	int addr;
	int num;
	int data;
	int datainc;
	int *pd;

	if ((argc < 3) || (argc > 5))
		return 1;

	addr = strtol(argv[1], 0, 0);
	data = strtol(argv[2], 0, 0);

	num = 1;
	datainc = 0;
	if (argc >= 4) {
		num = strtol(argv[3], 0, 0);

		if (argc == 5)
			datainc = strtol(argv[4], 0, 0);
	}

	pd = (int *) addr;
	for (i = 0; i < num; i++) {
		*pd++ = data;
		data += datainc;
	}

	pd = (int *) addr;
	for (i = 0, j = 0; i < num; i++) {
		if (j == 0)
			prints(FMT_UINT ": ", (int) pd);

		data = *pd++;
		prints("%08X ", data);

		if (++j == 4) {
			if ((i + 1) < num)
				prints("\n");
			j = 0;
		}
	}

	return (0);
}

#define CONFIG_MACH_TYPE            758 /* Faraday */
ulong load_addr = 0x2000000;

/* tftpboot <filename> [loadaddr] [serverip] */
int do_tftpboot(int argc, char * const  argv[])
{
	void (*kernel_entry)(int zero, int arch, unsigned int params);

	if (argc < 2)
		return 1;

	if (argc == 3)
		load_addr = strtoul(argv[2], NULL, 16);

	if (argc == 4)
		NetServerIP = string_to_ip((const char *) argv[3]);

	/* Check if we had MAC controller */
	if (!eth_initialize(argv[1]))
		return 0;

	if (NetLoop(TFTPGET) < 0) {
		prints("Download error\n");
		return 0;
	}

	disable_d_cache(0, NULL);

	kernel_entry = (void (*)(int, int, unsigned int)) load_addr;

	/* Never return */
	kernel_entry(0, CONFIG_MACH_TYPE, 0);

	return 0;
}

/* ping <ipaddr> */
int do_ping(int argc, char * const argv[])
{
        if (argc < 2)
                return -1;

	/* Check if we had MAC controller */
	if (!eth_initialize(NULL))
		return 0;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0)
		return 1;

	if (NetLoop(PING) < 0) {
		prints("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	prints("host %s is alive\n", argv[1]);

	return 0;
}

extern int do_irqinfo (void);
/* Usage: irq */
int do_print_irqinfo(int argc, char * const  argv[])
{

	do_irqinfo();

	return 0;
}

int do_help(int argc, char * const argv[], cmd_t * tbl)
{
	cmd_t *cmdp = &tbl[0];

	if (argc == 1) {	/* display all commands */
		while (cmdp->name != 0) {
			prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else if (argc == 2) {	/*Display argv[0] command */
		while (cmdp->name != 0) {
			if (strcmp(argv[0], cmdp->name) == 0)
				prints("  %-15s %-26s\n\r", cmdp->name, cmdp->usage);
			cmdp++;
		}
	} else {
		return 1;
	}

	return 0;
}
static int   argc;
static char *argv[16];
int cmd_exec(char *line, cmd_t * tbl)
{
	int i, rc;
	char *tok;
	cmd_t *cmd;

	argc = 0;

	tok  = strtok (line, " \r\n\t");
	while(tok) {
		argv[argc++] = tok;
		tok = strtok (NULL, " \r\n\t");
	}

	if (argc == 0)
		return 0;

	if (!strcmp("quit",argv[0]))
		return 1;

	for (i = 0; ; ++i) {
		cmd = &tbl[i];
		if ((cmd->name == NULL) || !strcmp("help", argv[0]) ) {
			do_help(1, 0, tbl);	/* list all support commands */
			return 0;
		} else if (!strcmp(cmd->name, argv[0]) && cmd->func) {
			if (cmd->func(argc, argv))
				do_help (2, argv, tbl);

			break;
		}
	}

	return 0;
}

void shell(void)
{
	char cmdstr[CMDLEN];

	while (1) {
#ifdef COPY_DATA_ONLY
		puts("\nroot-d:> ");
#else
		puts("\nroot-t:> ");
#endif
		scan_string(cmdstr);

		cmd_exec(cmdstr, main_cmd_tbl);
	}
}
