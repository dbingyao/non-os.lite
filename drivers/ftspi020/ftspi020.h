/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * 2010/08/13   BingJiun	 Scan all CEs for available flash.         
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_H
#define FTSPI020_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
#include <platform.h>

/* Transfer Type*/
typedef enum {
	PIO,
	DMA
} Transfer_type;

#define FTSPI020_FW_VERSION "ver:13.04.24"

/*
 * How many CEs available on system?
 */
#define FTSPI020_MAX_CE			4

/*
 * Flash Vendor List. Only define the vendor available 
 * on your system to reduce code size. 
 */ 
#define Winbond_W25Q32BV
#define Sst_SST25VF080B
#define Mxic_MX25L12845EM1
#define Winbond_W25Q128BV
#define Spansion_S25FL032P

#if defined(Winbond_W25Q32BV) || defined(Winbond_W25Q128BV)
#include "ftspi020_winbond.h"
#endif

#if defined(Mxic_MX25L12845EM1)
#include "ftspi020_mxic.h"
#endif

#if defined(Sst_SST25VF080B)
#include "ftspi020_sst.h"
#endif

#if defined(Spansion_S25FL032P)
#include "ftspi020_spansion.h"
#endif

#undef FTSPI020_USE_INTERRUPT
#define FTSPI020_USE_DMA

#ifdef FTSPI020_USE_DMA

#define FTSPI020_DMA_WR_CHNL	0
#define FTSPI020_DMA_RD_CHNL	0

// Select from 0 to 3 in FTSPI020_DMA_PRIORITY
#define FTSPI020_DMA_PRIORITY	3

extern uint32_t g_uint32_t_burst_sz;

extern int ftspi020_Start_DMA(uint32_t Channel,	// use which channel for AHB DMA, 0..7
			    uint32_t SrcAddr,	// source begin address
			    uint32_t DstAddr,	// dest begin address
			    uint32_t Size,	// total bytes
			    uint32_t SrcWidth,	// source width 8/16/32 bits -> 0/1/2
			    uint32_t DstWidth,	// dest width 8/16/32 bits -> 0/1/2
			    uint32_t SrcSize,	// source burst size, How many "SrcWidth" will be transmmited at one times ?
			    uint32_t SrcCtrl,	// source address change : Inc/dec/fixed --> 0/1/2
			    uint32_t DstCtrl,	// dest address change : Inc/dec/fixed --> 0/1/2
			    uint32_t Priority);	// priority for this chaanel 0(low)/1/2/3(high)
	/*
	   Access Type of PIO 
	   Note: Mark the following both two statements will be byte access.
	 */
#endif

#define WORD_ACCESS
	//#define HALFWORD_ACCESS

#define FTSPI020_32BIT(offset)		*((volatile uint32_t *)(FTSPI020_REG_BASE + offset))
#define FTSPI020_16BIT(offset)		*((volatile uint16_t *)(FTSPI020_REG_BASE + offset))
#define FTSPI020_8BIT(offset)		*((volatile uint8_t  *)(FTSPI020_REG_BASE + offset))

#define check_status_by_hw		0
#define check_status_by_sw		1

#define SPI_FLASH_ADDR			0x0000

#define SPI_CMD_FEATURE1		0x0004
	// bit[2:0]
#define no_addr_state			0
#define addr_1byte			1
#define addr_2byte			2
#define addr_3byte			3
#define addr_4byte			4
	// bit[23:16]
#define no_dummy_cycle_2nd		0
	// bit[25:24]
#define no_instr_state			0
#define instr_1byte			1
#define instr_2byte			2
	// bit28
#define no_op_code_state		0
#define op_code_1byte			1

#define SPI_DATA_CNT			0x0008

#define SPI_CMD_FEATURE2		0x000C
	// bit1
#define spi_read			0
#define spi_write			1
	// bit2
#define read_status_disable		0
#define read_status_enable		1
	// bit3 
#define read_status_by_hw		0
#define read_status_by_sw		1
	// bit4
#define dtr_disable			0
#define dtr_enable			1
	// bit[7:5]
#define spi_operate_serial_mode		0
#define spi_operate_dual_mode		1
#define spi_operate_quad_mode		2
#define spi_operate_dualio_mode		3
#define spi_operate_quadio_mode		4
	// bit[9:8]
#define	start_ce0			0
#define	start_ce1			1
#define	start_ce2			2
#define	start_ce3			3
	// bit[23:16] continuous read mode code

	// bit[31:24] instruction code

#define CONTROL_REG			0x0010
	// bit[1:0] divider
#define divider_2			0
#define divider_4			1
#define divider_6			2
#define divider_8			3
	// bit 4 mode
#define mode0				0
#define mode3				1
	// bit 8 abort
	// bit 9 wp_en
	// bit[18:16] busy_loc
#define BUSY_BIT0			0
#define BUSY_BIT1			1
#define BUSY_BIT2			2
#define BUSY_BIT3			3
#define BUSY_BIT4			4
#define BUSY_BIT5			5
#define BUSY_BIT6			6
#define BUSY_BIT7			7
	// bit [20]
#define BOOT_MODE			BIT20

#define AC_TIMING			0x0014
// bit[3:0] CS timing
// bit[7:4] Trace delay

#define STATUS_REG			0x0018
// bit0 TX FIFO ready
// bit1 RX FIFO ready

#define INTR_CONTROL_REG		0x0020
#define dma_handshake_enable		BIT0
#define cmd_complete_intr_enable	BIT1

#define INTR_STATUS_REG			0x0024
#define cmd_complete			(1 << 0)
#define rx_fifo_underrun		(1 << 1)
#define tx_fifo_overrun			(1 << 2)

#define SPI_READ_STATUS			0x0028
#define SPI_ADDRESS_MASK		0x0028

#define TX_DATA_CNT			0x0030
#define RX_DATA_CNT			0x0034

#define REVISION			0x0050
#define FEATURE				0x0054
#define		support_dtr_mode		(1 << 24)
#define SPI020_DATA_PORT		0x0100

// =====================================   Common commands   =====================================
#define CMD_READ_ID  	                0x9f
#define CMD_RESET			0xFF
// ===================================== Variable and Struct =====================================
enum {
	WRITE,
	READ,
	ERASE
};

struct spi_flash {
	uint32_t ce;
	uint16_t code;
	const char *name;
	uint32_t erase_sector_size;
	uint16_t page_size;
	uint32_t nr_pages;
	uint32_t size;
	int(*spi_xfer)(struct spi_flash * flash, unsigned int len, const void *dout, void *din, unsigned long flags);
	int(*read) (struct spi_flash * flash, uint8_t type, uint32_t offset, uint32_t len, void *buf);
	int(*write) (struct spi_flash * flash, uint8_t type, uint32_t offset, uint32_t len, void *buf);
	int(*erase) (struct spi_flash * flash, uint8_t type, uint32_t offset, uint32_t len);
	int(*erase_all) (struct spi_flash * flash);
	int(*report_status) (struct spi_flash * flash);
	char * (*get_string) (uint32_t act, uint32_t type);

	uint16_t max_rd_type;
	uint16_t max_wr_type;
	uint16_t max_er_type;
};

struct ftspi020_cmd {
	// offset 0x00
	uint32_t spi_addr;
	// offset 0x04
	uint8_t addr_len;
	uint8_t dum_2nd_cyc;
	uint8_t ins_len;
	uint8_t conti_read_mode_en;
	// offset 0x08
	uint32_t data_cnt;
	// offset 0x0C
	uint8_t write_en;
	uint8_t read_status_en;
	uint8_t read_status;
	uint8_t dtr_mode;
	uint8_t spi_mode;
	uint8_t start_ce;
	uint8_t conti_read_mode_code;
	uint8_t ins_code;

};

extern volatile uint32_t ftspi020_cmd_complete;
extern char g_check_status;
extern Transfer_type g_trans_mode;
extern uint8_t g_divider;
extern int debug;

#endif
