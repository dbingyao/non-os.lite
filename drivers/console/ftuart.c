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
#include "ftuart.h"

static volatile ftuart_t * uart;

static void
ftuart_set_baud(int baud)
{
	uart->LCR |= FTUART_LCR_DLAB;
	uart->DLM = (baud >> 8) & 0xff;
	uart->DLL = baud & 0xff;
	uart->LCR &= ~FTUART_LCR_DLAB;
}

void
uart_init(int reg_base, int baud)
{
	uart = (volatile ftuart_t *) reg_base;

	ftuart_set_baud(baud);

	uart->LCR = FTUART_LCR_WL0 | FTUART_LCR_WL1;
	uart->FCR = FTUART_FCR_FIFO_ENABLE | FTUART_FCR_RXFIFO_RESET | FTUART_FCR_TXFIFO_RESET;
}

void
ftuart_fini(void)
{
	/* wait until all output done */
	while (!(uart->LSR & FTUART_LSR_THR_EMPTY)) ;

	/* disable uart */
	uart->FCR = FTUART_FCR_RXFIFO_RESET | FTUART_FCR_TXFIFO_RESET;
}


void
ftuart_putc(const char c)
{
	/* CR+LF as newline */
	if (c == '\n')
		ftuart_putc('\r');

	/* wait for room in the tx FIFO */
	while (!(uart->LSR & FTUART_LSR_THR_EMPTY)) ;

	uart->THR = c;
}

char
ftuart_getc(void)
{
	char	ret;

	while (!(uart->LSR & FTUART_LSR_DATA_READY)) ;

	if ((ret = uart->RBR) == '\r')
		ret = '\n';

	return ret;
}

int 
ftuart_puts(const char *s)
{
	while (s && *s) {
		if (*s == '\n')
			ftuart_putc ('\r');
		ftuart_putc (*s++);
	}
	return 0;
}

char *ftuart_gets(char *s)
{
	int c, i = 0;

	do {
		c = ftuart_getc();
		s[i++] = (char)c;
	} while(c && c != -1 && c != '\r' && c != '\n');

	if (c)
		s[i - 1] = 0;

	return s;
}

char ftuart_kbhit(void)
{
    	if (uart->LSR & FTUART_LSR_DATA_READY)
		return uart->RBR;
	else
		return 0;
}

