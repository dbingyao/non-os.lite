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

/**
 * FPGA board address map
 *                +==========================================
 *     0x00000000 | SRAM/ROM/FLASH (Slave port 3)
 *                |
 *     0x10000000 |==========================================
 *                | DDRIII (Slave port 4)
 *                |==========================================
 *     0x90000000 |Controller's reg
 *                |
 *                |0xA0000000 AXI to APB Bridge (Slave 6)
 *                |0xE0000000 FTSPI020 (Slave 8)
 *                |
 *     0xA0000000 |==========================================
 *                | APB Device's Reg
 *                |
 *                |0xA0100000 BUSC AXI controller
 *                |0xA0200000 FTDMAC030
 *                |0xA0300000 FTDDR3030
 *                |0xA0400000 FTSMC030
 *                |0xA8000000 UART 0
 *                |0xA8100000 UART 1
 *                |0xA8200000 TIMER
 *                |0xA8300000 INTC
 *                |0xA8400000 RTC
 *                |0xA8500000 GPIO
 *                |
 *                +==========================================
 */

/**
 * AXI Controller
 */
#define	FTAXIC030_REG_BASE 	0xA0100000
/* Port 1 has 8 memory regions */
#define	PORT1_MEMXREG_OFFSET(x)	(0x0 + x * 0x4)
#define	PORT2_REG_OFFSET	0x20
#define	PORT3_REG_OFFSET	0x24
#define	PORT4_REG_OFFSET	0x28
#define	PORT5_REG_OFFSET	0x2C
#define	PORT6_REG_OFFSET	0x30
#define	PORT7_REG_OFFSET	0x34
#define	PORT8_REG_OFFSET	0x38
#define	PORT9_REG_OFFSET	0x3C
#define	PORT10_REG_OFFSET	0x40

#define	REMAP_REG_OFFSET	0x130

/**
 * FTDMAC030 DMA controller
 */
#define FTDMAC030_REG_BASE	0xA0200000

/**
 * DDR3030 Controller
 */
#define DDRC_REG_BASE		0xA0300000

/**
 * SMC030 ROM/Flash Controller
 */
#define SMC_REG_BASE	0xA0400000

#ifdef ROM_SPI
#define SMC_LED_ADDR	0xC400000C
#else
#define SMC_LED_ADDR	0x0400000C
#endif

#define	SRAMC_CONFIG_REG_OFFSET 	0x0
#define	SRAMC_TIME0_REG_OFFSET 		0x4
/* SRAM bank config register */
#define	SRAMC_CONFIG_ENABLE 	(1<<28)

/* bank size */
#define	SRAMC_BANKSIZE_32K	0xb0
#define	SRAMC_BANKSIZE_64K	0xc0
#define	SRAMC_BANKSIZE_128K	0xd0
#define	SRAMC_BANKSIZE_256K	0xe0
#define	SRAMC_BANKSIZE_512K	0xf0
#define	SRAMC_BANKSIZE_1M	0x00
#define	SRAMC_BANKSIZE_2M	0x10
#define	SRAMC_BANKSIZE_4M	0x20
#define	SRAMC_BANKSIZE_8M	0x30
#define	SRAMC_BANKSIZE_16M	0x40
#define	SRAMC_BANKSIZE_32M	0x50

/* bus width */
#define	SRAMC_BUSWIDTH_8	0x0
#define	SRAMC_BUSWIDTH_16	0x1
#define	SRAMC_BUSWIDTH_32	0x2

/**
 * UART Controller
 */
#define	UART0_REG_BASE  	0xA8000000
#define	UART1_REG_BASE  	0xA8100000

#define BOARD_UART_CLOCK	18432000

#define UART_BAUD_115200	(BOARD_UART_CLOCK / 1843200)
#define UART_BAUD_57600 	(BOARD_UART_CLOCK / 921600)
#define UART_BAUD_38400 	(BOARD_UART_CLOCK / 614400)
#define UART_BAUD_19200 	(BOARD_UART_CLOCK / 307200)
#define UART_BAUD_14400 	(BOARD_UART_CLOCK / 230400)

/**
 * FTTMR010 timer controller
 */
#define TIMERC_REG_BASE 	0xA8200000
#define TIMERC_IRQ1   		1
#define TIMERC_IRQ2   		2
#define TIMERC_IRQ3   		3
#define TIMER_CLOCK     	37748736
#define CONFIG_SYS_HZ   	1000    /* timer ticks per second */

/**
 * Interrupt Controller
 */
#define	INTC_REG_BASE	0xA8300000
#define NR_IRQS	32

/**
 * FTSPI020 SPI Flash Controller
 */
#define FTSPI020_REG_BASE	0xE0000000
#define	FTSPI020_IRQ    	7

/**
 * Common for all platforms
 */
#define BUSC_REG_BASE FTAXIC030_REG_BASE
#define DDR_SDRAM_SLAVE PORT4_REG_OFFSET

#define CONFIG_SHOW_LED_ADDR 0x04000501
#define LED_ADDR_REMAPPED 0x8400000C

#ifdef __ASSEMBLER__
.macro  led, num, addr=0
	mov	r7, #\num
	lsl	r7, r7, #28
	.if \addr
	ldr	r8, =\addr
	.else
	ldr	r8, =SMC_LED_ADDR
	.endif

	str	r7, [r8]
.endm
#endif
