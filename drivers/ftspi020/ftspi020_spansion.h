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

#define SPANSION_WRITE_ENABLE		0x06
#define SPANSION_WRITE_DISABLE		0x04
#define SPANSION_ERASE_SECTOR		0x20
#define SPANSION_ERASE_8K_BLOCK		0x40
#define	SPANSION_ERASE_64K_BLOCK	0xD8
#define SPANSION_ERASE_CHIP		0xC7
#define SPANSION_READ_STATUS		0x05
#define SPANSION_WRITE_STATUS_CONF	0x01
#define SPANSION_READ_CONFIG		0x35
#define SPANSION_READ_DATA		0x03
#define SPANSION_FAST_READ		0x0B
#define SPANSION_DUAL_READ		0x3B
#define SPANSION_QUAD_READ		0x6B
#define SPANSION_DUAL_IO_READ		0xBB
#define SPANSION_QUAD_IO_READ		0xEB
#define SPANSION_WRITE_PAGE		0x02
#define SPANSION_QUAD_WRITE_PAGE	0x32
#define SPANSION_READ_UNIQUE_ID		0x4B

#define SPANSION_STS_BUSY_BIT		BIT0
#define SPANSION_STS_WEL_BIT		BIT1
#define SPANSION_STS_BPROTECT_BITS	(BIT4| BIT3| BIT2)
#define SPANSION_STS_QUAD_MODE_BIT	BIT6
#define SPANSION_STS_REG_WRTIE_BIT	BIT7

#define SPANSION_CFG_QUAD_MODE  	BIT1
typedef enum {
	S25_SECTOR_ERASE = 0,
	S25_BLOCK_8K_ERASE,
	S25_BLOCK_64K_ERASE,
	S25_MAX_ERASE_TYPE
} S25_ERASE_TYPE;

typedef enum {
	S25_PAGE_PROGRAM = 0,
	S25_QUAD_PROGRAM,
	S25_MAX_WRITE_TYPE
} S25_WRITE_TYPE;

typedef enum {
	S25_READ_DATA = 0,
	S25_FAST_READ,
	S25_DUAL_READ,
	S25_QUAD_READ,
	S25_DUAL_IO_READ,
	S25_QUAD_IO_READ,
	S25_MAX_READ_TYPE
} S25_READ_TYPE;

#endif
