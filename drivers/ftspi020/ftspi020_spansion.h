/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_spansion.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_SPANSION_H
#define FTSPI020_SPANSION_H

/* Read Memory Array Commands */
#define SPANSION_READ			0x03
#define SPANSION_FAST_READ		0x0B
#define SPANSION_READ_DUAL_OUT		0x3B
#define SPANSION_READ_QUAD_OUT		0x6B
#define SPANSION_DUAL_IO_READ		0xBB
#define SPANSION_QUAD_IO_READ		0xEB
#define SPANSION_READ_DDR_FAST		0x0D
#define SPANSION_DDR_DUAL_IO_READ	0xBD
#define SPANSION_DDR_QUAD_IO_READ	0xED

/* Program Flash Array Commands */
#define SPANSION_WRITE_PAGE		0x02
#define SPANSION_QUAD_WRITE_PAGE	0x32

/* Erase Flash Array Commands */
#define SPANSION_ERASE_PARAM_4K 	0x20
#define SPANSION_ERASE_PARAM_8K 	0x40
#define SPANSION_ERASE_64K_BLOCK	0xD8
#define SPANSION_ERASE_CHIP		0xC7

/* Register Access Commands */
#define SPANSION_READ_STATUS1		0x05
#define SPANSION_READ_STATUS2		0x07
#define SPANSION_READ_CONFIG		0x35
#define SPANSION_BANK_REG_READ		0x16
#define SPANSION_BANK_REG_WRITE 	0x17
#define SPANSION_WRITE_REGISTER 	0x01
#define SPANSION_WRITE_ENABLE		0x06
#define SPANSION_WRITE_DISABLE		0x04

/* Status Register 1 */
#define SPANSION_STS1_BUSY_BIT			BIT0
#define SPANSION_STS1_WEL_BIT			BIT1
#define SPANSION_STS1_BPROTECT_BITS		(BIT4| BIT3| BIT2)
#define SPANSION_STS1_ERASE_ERROR_BIT		BIT5
#define SPANSION_STS1_PROGRAM_ERROR_BIT		BIT6
#define SPANSION_STS1_STSREG_WRITE_DIS_BIT	BIT7

/* Configuration Register */
#define SPANSION_CFG_QUAD_MODE  	BIT1

typedef enum {
	S25_PARAM_SECTOR_ERASE = 0,
	S25_BLOCK_64K_ERASE,
	S25_MAX_ERASE_TYPE
} S25_ERASE_TYPE;

typedef enum {
	S25_PAGE_PROGRAM = 0,
	S25_QUAD_PROGRAM,
	S25_MAX_WRITE_TYPE
} S25_WRITE_TYPE;

typedef enum {
	S25_READ = 0,
	S25_FAST_READ,
	S25_READ_DUAL_OUT,
	S25_READ_QUAD_OUT,
	S25_DUAL_IO_READ,
	S25_QUAD_IO_READ,
	S25_MAX_READ_TYPE
} S25_READ_TYPE;

#endif
