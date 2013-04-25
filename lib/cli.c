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
#include <types.h>
#include <platform.h>


#define FMT_UINT	"%08X"

extern int FTSPI020_main(int argc, char * const argv[]);
extern int do_spi_bootmode_test(int argc, char * const argv[]);

int do_rd32(int argc, char * const  argv[]);
int do_wr32(int argc, char * const  argv[]);
int do_print_irqinfo(int argc, char * const  argv[]);
int do_help(int argc, char * const argv[], cmd_t * tbl);

cmd_t main_cmd_tbl[] = { 
			{"md", "<addr> [num]", do_rd32},
			{"mw", "<addr> <data> [num [inc]]", do_wr32},
			{"irq","Interrupt information", do_print_irqinfo},
			{"spi","FTSPI020 commands mode", FTSPI020_main},
			{"spt", "Code on SPI flash test", do_spi_bootmode_test},
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

#define	ISHEX(c)	((((c>='0')&&(c<='9')) || ((c>='a')&&(c<='f')) || \
			  ((c>='A')&&(c<'F'))) ? 1 : 0)

int str_to_hex(char * str, void * num, int digits)
{
	char *value = (char *) num;
	char ch, byte;
	int i = 0, j;

	if ((str[0] == '0') && ((str[1] == 'X') || (str[1] == 'x')))
		str += 2;

	while (str[i] != '\0') {
		if (!ISHEX(str[i]))
			return 0;
		i++;
	}

	if ((i == 0) || (i > digits))
		return 0;

	i--;
	for (j = 0; j < ((digits + 1) / 2); j++)
		*value++ = 0;

#ifdef __BIG_ENDIAN
	value = (char *) num + (digits + 1) / 2 - 1;
#else
	value = (char *) num;
#endif
	while (i >= 0) {
		byte = str[i--] - 48;
		if (byte > 9) {
			byte -= 7;
			if (byte > 0xf)
				byte -= 32;
		}
		if (i >= 0) {
			ch = str[i--] - 48;
			if (ch > 9) {
				ch -= 7;
				if (ch > 0xf)
					ch -= 32;
			}
			byte += ch << 4;
			*value = byte;
#ifdef __BIG_ENDIAN
			value--;
#else
			value++;
#endif
		} else {
			*(char *) value = byte;
			break;
		}
	}
	return 1;
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

	if (!str_to_hex(argv[1], &addr, sizeof(addr) * 2))
		return 1;

	if (argc == 3) {
		if (!str_to_hex(argv[2], &num, sizeof(num) * 2))
			return 1;
	} else
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

	if (!str_to_hex(argv[1], &addr, sizeof(addr) * 2))
		return 1;

	if (!str_to_hex(argv[2], &data, sizeof(data) * 2))
		return 1;

	num = 1;
	datainc = 0;
	if (argc >= 4) {
		if (!str_to_hex(argv[3], &num, sizeof(num) * 2))
			return 1;
		if (argc == 5) {
			if (!str_to_hex(argv[4], &datainc, sizeof(datainc) * 2))
				return 1;
		}
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
