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

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <types.h>

#define CONFIG_SYS_FLASH_CFI

/* support JEDEC */
#define CONFIG_FLASH_CFI_LEGACY

#define PHYS_FLASH_1                    0x10000000
#define CONFIG_SYS_FLASH_BASE           PHYS_FLASH_1

#define CONFIG_SYS_MONITOR_BASE         PHYS_FLASH_1

/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS      1

#undef CONFIG_SYS_FLASH_EMPTY_INFO
/* max number of sectors on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT       512

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * Read/Write registers macro
 */
#define outb(val, adrr) 	(*(volatile unsigned char *)(addr) = (val))
#define outw(val, adrr) 	(*(volatile unsigned short *)(addr) = (val))
#define outl(val, addr) 	(*(volatile unsigned int *) (addr) = val)

#define inb(addr) 		(*(volatile unsigned char *) (addr))
#define inw(addr) 		(*(volatile unsigned short *) (addr))
#define inl(addr) 		(*(volatile unsigned int *) (addr))

/*
 * Timer
 */
extern void udelay(unsigned long usec);
extern unsigned long get_timer(unsigned long base);

#define min_t(x,y) ( x < y ? x: y )
#define max_t(x,y) ( x > y ? x: y )

#define BIT0							(1 << 0)
#define BIT1							(1 << 1)
#define BIT2							(1 << 2)
#define BIT3							(1 << 3)
#define BIT4							(1 << 4)
#define BIT5							(1 << 5)
#define BIT6							(1 << 6)
#define BIT7							(1 << 7)
#define BIT8							(1 << 8)
#define BIT9							(1 << 9)
#define BIT10							(1 << 10)
#define BIT11							(1 << 11)
#define BIT12							(1 << 12)
#define BIT13							(1 << 13)
#define BIT14							(1 << 14)
#define BIT15							(1 << 15)
#define BIT16							(1 << 16)
#define BIT17							(1 << 17)
#define BIT18							(1 << 18)
#define BIT19							(1 << 19)
#define BIT20							(1 << 20)
#define BIT21							(1 << 21)
#define BIT22							(1 << 22)
#define BIT23							(1 << 23)
#define BIT24							(1 << 24)
#define BIT25							(1 << 25)
#define BIT26							(1 << 26)
#define BIT27							(1 << 27)
#define BIT28							(1 << 28)
#define BIT29							(1 << 29)
#define BIT30							(1 << 30)
#define BIT31							(1 << 31)

//#define container_of(ptr, type, member) ({const typeof( ((type *)0)->member ) *__mptr = (ptr); (type *)( (char *)__mptr - offsetof(type,member));})
//#define container_of(ptr, type, member) (const struct spi_flash(((struct winbond_spi_flash *)0)->member) *__mptr = (ptr);)
//#define container_of(ptr, type, member) (offsetof(type,member))

/**
 * Allocate memory
 */
extern void * simple_malloc(int size);

/**
 * Console
 */
struct cmd {
	char *name;
	char *usage;
	int(*func) (int argc, char * const argv[]);
};

typedef struct cmd cmd_t;

extern int cmd_exec(char *line, cmd_t * tbl);

#define	CMDLEN  	50

extern int str_to_hex(char * str, void *num, int digits);
extern int ctrlc(void);

extern void shell(void);

/**
 * Interrupt
 */
enum {
        IRQ_TYPE_NONE           = 0x00000000,
        IRQ_TYPE_EDGE_RISING    = 0x00000001,
        IRQ_TYPE_EDGE_FALLING   = 0x00000002,
        IRQ_TYPE_EDGE_BOTH      = (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
        IRQ_TYPE_LEVEL_HIGH     = 0x00000004,
        IRQ_TYPE_LEVEL_LOW      = 0x00000008,
        IRQ_TYPE_LEVEL_MASK     = (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
        IRQ_TYPE_SENSE_MASK     = 0x0000000f,
};

typedef void (interrupt_handler_t)(void *);

/* CPU I bit */
extern void enable_interrupts(void);
extern int disable_interrupts (void);

/* Interrupt controller */
extern void irq_edge_clear(int irq);
extern int irq_set_type(int irq, unsigned int type);
extern void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
extern void irq_set_enable(int irq);
extern void irq_set_disable(int irq);

#endif /* _COMMON_H */
