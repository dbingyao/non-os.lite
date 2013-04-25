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

/*
 * UART and IrDA Communications Controller
 */
#ifndef __FTUART_H
#define __FTUART_H

	/* for FIFO Control Register */
#define FTUART_FCR_FIFO_ENABLE		(1 << 0)
#define FTUART_FCR_RXFIFO_RESET		(1 << 1)
#define FTUART_FCR_TXFIFO_RESET		(1 << 2)

	/* for Line Control Register */
#define FTUART_LCR_WL0			(1 << 0)
#define FTUART_LCR_WL1			(1 << 1)
#define FTUART_LCR_STOP			(1 << 2)
#define FTUART_LCR_PARITY_ENABLE	(1 << 3)
#define FTUART_LCR_EVEN_PARITY		(1 << 4)
#define FTUART_LCR_STICK_PARITY		(1 << 5)
#define FTUART_LCR_SET_BREAK		(1 << 6)
#define FTUART_LCR_DLAB			(1 << 7)

	/* for Line Status Register */
#define FTUART_LSR_DATA_READY		(1 << 0)
#define FTUART_LSR_OVERRUN		(1 << 1)
#define FTUART_LSR_PARITY_ERR		(1 << 2)
#define FTUART_LSR_FRAMING_ERR		(1 << 3)
#define FTUART_LSR_BREAK_INTERRUPT	(1 << 4)
#define FTUART_LSR_THR_EMPTY		(1 << 5)
#define FTUART_LSR_TRANSMIT_EMPTY	(1 << 6)
#define FTUART_LSR_FIFO_DATA_ERR	(1 << 7)

#ifndef __ASSEMBLER__
#include <stdint.h>

typedef struct {
	union {					/* 0x00 */
		volatile uint8_t	RBR;
		volatile uint8_t	THR;
		volatile uint8_t	DLL;
	} __attribute__ ((aligned (4)));
	union {					/* 0x04 */
		volatile uint8_t	IER;
		volatile uint8_t	DLM;
	} __attribute__ ((aligned (4)));
	union {					/* 0x08 */
		volatile uint8_t	IIR;
		volatile uint8_t	FCR;
		volatile uint8_t	PSR;
	} __attribute__ ((aligned (4)));
	volatile uint8_t		LCR		__attribute__ ((aligned (4)));	/* 0x0C */
	volatile uint8_t		MCR		__attribute__ ((aligned (4)));	/* 0x10 */
	union {					/* 0x14 */
		volatile uint8_t	LSR;
		volatile uint8_t	TST;
	} __attribute__ ((aligned (4)));
	volatile uint8_t		MSR		__attribute__ ((aligned (4)));	/* 0x18 */
	volatile uint8_t		SPR		__attribute__ ((aligned (4)));	/* 0x1C */
	volatile uint8_t		MDR		__attribute__ ((aligned (4)));	/* 0x20 */
	volatile uint8_t		ACR		__attribute__ ((aligned (4)));	/* 0x24 */
	volatile uint8_t		TXLENL		__attribute__ ((aligned (4)));	/* 0x28 */
	volatile uint8_t		TXLENH		__attribute__ ((aligned (4)));	/* 0x2C */
	volatile uint8_t		MRXLENL		__attribute__ ((aligned (4)));	/* 0x30 */
	volatile uint8_t		MRXLENH		__attribute__ ((aligned (4)));	/* 0x34 */
	volatile uint8_t		PLR		__attribute__ ((aligned (4)));	/* 0x38 */
	union {					/* 0x3C */
		volatile uint8_t	FMIIR_PIO;
		volatile uint8_t	FMIIR_DMA;
	} __attribute__ ((aligned (4)));
	union {					/* 0x40 */
		volatile uint8_t	FMIIER_PIO;
		volatile uint8_t	FMIIER_DMA;
	} __attribute__ ((aligned (4)));
	volatile uint8_t		STFF_STS	__attribute__ ((aligned (4)));	/* 0x44 */
	volatile uint8_t		STFF_RXLENL	__attribute__ ((aligned (4)));	/* 0x48 */
	volatile uint8_t		STFF_RXLENH	__attribute__ ((aligned (4)));	/* 0x4C */
	volatile uint8_t		FMLSR		__attribute__ ((aligned (4)));	/* 0x50 */
	volatile uint8_t		FMLSIER		__attribute__ ((aligned (4)));	/* 0x54 */
	volatile uint8_t		RSR		__attribute__ ((aligned (4)));	/* 0x58 */
	volatile uint8_t		RXFF_CNTR	__attribute__ ((aligned (4)));	/* 0x5C */
	volatile uint8_t		LSTFMLENL	__attribute__ ((aligned (4)));	/* 0x60 */
	volatile uint8_t		LSTFMLENH	__attribute__ ((aligned (4)));	/* 0x64 */
} ftuart_t;

char ftuart_kbhit(void);
void ftuart_putc(const char c);
char ftuart_getc(void);
int ftuart_puts(const char *s);
char *ftuart_gets(char *s);
void ftuart_fini(void);

#endif	/* __ASSEMBLER__*/

#endif	/* __FTUART_H */
