/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_winbond.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * 2010/08/16   BingJiun	 - Scan all CEs for available flash.         
 *				 - Implement mode commands.        
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_WINBOND_H
#define FTSPI020_WINBOND_H

// Command
#define WINBOND_WRITE_ENABLE		0x06
#define WINBOND_WRITE_DISABLE		0x04
#define WINBOND_ERASE_SECTOR		0x20
#define WINBOND_ERASE_32K_BLOCK		0x52
#define WINBOND_ERASE_64K_BLOCK		0xD8
#define WINBOND_ERASE_CHIP		0xC7
#define WINBOND_READ_STATUS1		0x05
#define WINBOND_READ_STATUS2		0x35
#define WINBOND_WRITE_STATUS		0x01
#define WINBOND_WRITE_PAGE		0x02
#define WINBOND_QUAD_WRITE_PAGE		0x32
#define WINBOND_READ_DATA		0x03
#define WINBOND_FAST_READ		0x0B
#define WINBOND_FAST_READ_DUAL		0x3B
#define WINBOND_FAST_READ_DUAL_IO	0xBB
#define WINBOND_FAST_READ_QUAD		0x6B
#define WINBOND_FAST_READ_QUAD_IO	0xEB
#define WINBOND_WORD_READ_QUAD_IO	0xE7
#define WINBOND_READ_UNIQUE_ID		0x4B

/* Status register 1 bits */
#define WINBOND_STS_BUSY		(1 << 0)
#define WINBOND_STS_WE_LATCH		(1 << 1)
#define WINBOND_STS_REG_PROTECT0	(1 << 7)

/* Status register 2 bits */
#define WINBOND_STS_REG_PROTECT1	(1 << 0)
#define WINBOND_STS_QUAD_ENABLE		(1 << 1)


typedef enum {
	W25_SECTOR_ERASE = 0,
	W25_BLOCK_32K_ERASE,
	W25_BLOCK_64K_ERASE,
	W25_MAX_ERASE_TYPE
} W25_ERASE_TYPE;

// Cmd Type for choosing
typedef enum {
	W25_PAGE_PROGRAM = 0,
	W25_QUAD_PROGRAM,
	W25_MAX_WRITE_TYPE
} W25_WRITE_TYPE;

typedef enum {
	W25_READ_DATA = 0,
	W25_FAST_READ,
	W25_FAST_READ_DUAL,
	W25_FAST_READ_DUAL_IO,
	W25_FAST_READ_QUAD,
	W25_FAST_READ_QUAD_IO,
	W25_WORD_READ_QUAD_IO,
	W25_MAX_READ_TYPE
} W25_READ_TYPE;

extern struct spi_flash *spi_flash_probe_winbond(uint8_t * uint8_t_idcode);

#endif
