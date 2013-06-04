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
 *     0x00000000 | SRAM/ROM/FLASH (Slave port 4)
 *                |
 *     0x10000000 |==========================================
 *                | DDRII (Slave port 6)
 *                |==========================================
 *     0x90000000 |Controller's reg
 *                |
 *                |0x90000000 FA626 DMA port (Slave 1)
 *                |0x90100000 AHB controller
 *                |0x90200000 SMC registers (Slave 2)
 *                |0x90400000 FTDMAC020 registers (Slave 5)
 *                |0x90500000 AHB2APB bridge (Slave 7)
 *                |0x90600000 Reserved
 *                |0x98000000-9AFFFFFF APB Device
 *                |
 *     0x98000000 |==========================================
 *                | APB Device's Reg
 *                |
 *                |0x98000000 Reserved
 *                |0x98100000 FTDDR3030 (Slave 0)
 *                |0x98200000 INTC (Slave 1)
 *                |0x98300000 UART 0 (Slave 2)
 *                |0x98400000 RTC (Slave 3)
 *                |0x98500000 UART 1 (Slave 4)
 *                |0x98600000 GPIO (Slave 5)
 *                |0x98700000 TIMER (Slave 6)
 *                |0x98800000 Reserved
 *                |
 *                +==========================================
 */

/**
 * AHB Controller
 */
#define	FTAHBC_REG_BASE 	0x90100000
#define	SLAVE0_REG_OFFSET	0x0
#define	SLAVE1_REG_OFFSET	0x4
#define	SLAVE2_REG_OFFSET	0x8
#define	SLAVE3_REG_OFFSET	0xC
#define	SLAVE4_REG_OFFSET	0x10
#define	SLAVE5_REG_OFFSET	0x14
#define	SLAVE6_REG_OFFSET	0x18
#define	SLAVE7_REG_OFFSET	0x1C
#define	SLAVE10_REG_OFFSET	0x28

#define	PRIORITY_REG_OFFSET	0x80
#define	DEFMASTER_REG_OFFSET	0x84
#define	REMAP_REG_OFFSET	0x88

/**
 * SMC ROM/Flash Controller
 */
#define SMC_REG_BASE	0x90200000
#ifdef ROM_SPI
#define SMC_LED_ADDR	0xA400000C
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
 * DDR3030 Controller
 */
#define DDRC_REG_BASE	0x98100000


/**
 * Interrupt Controller
 */
#define	INTC_REG_BASE	0x98200000
#define NR_IRQS	32

/**
 * UART Controller
 */
#define	UART0_REG_BASE	0x98300000
#define	UART1_REG_BASE	0x98500000

#define BOARD_UART_CLOCK	18432000

#define UART_BAUD_115200	(BOARD_UART_CLOCK / 1843200)
#define UART_BAUD_57600 	(BOARD_UART_CLOCK / 921600)
#define UART_BAUD_38400 	(BOARD_UART_CLOCK / 614400)
#define UART_BAUD_19200 	(BOARD_UART_CLOCK / 307200)
#define UART_BAUD_14400 	(BOARD_UART_CLOCK / 230400)


/**
 * FTSPI020 SPI Flash Controller
 */
#define FTSPI020_REG_BASE	0x90700000
#define	FTSPI020_IRQ    	12

/**
 * FTTMR010 timer controller
 */
#define TIMERC_REG_BASE 	0x98700000
#define TIMERC_IRQ1   		1
#define TIMERC_IRQ2   		2
#define TIMERC_IRQ3   		3
#define TIMER_CLOCK     	37748736
#define CONFIG_SYS_HZ   	1000    /* timer ticks per second */
#define TIMER_USE_EXTCLK        0

/**
 * FTDMAC020 DMA controller
 */
#define FTDMAC020_REG_BASE	0x90400000

/**
 * Common for all platforms
 */
#define BUSC_REG_BASE FTAHBC_REG_BASE
#define DDR_SDRAM_SLAVE SLAVE6_REG_OFFSET

#define CONFIG_SHOW_LED_ADDR 0x14000000
#define LED_ADDR_REMAPPED 0x1400000C

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
