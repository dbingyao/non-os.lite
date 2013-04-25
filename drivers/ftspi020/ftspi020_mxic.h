/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_mxic.h
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

#ifndef FTSPI020_MXIC_H
#define FTSPI020_MXIC_H

// Command
#define MXIC_WRITE_ENABLE		0x06
#define MXIC_WRITE_DISABLE		0x04
#define MXIC_ERASE_SECTOR		0x20
#define MXIC_ERASE_32K_BLOCK		0x52
#define MXIC_ERASE_64K_BLOCK		0xD8
#define MXIC_ERASE_CHIP			0xC7	// or 0x60
#define MXIC_READ_STATUS		0x05
#define MXIC_WRITE_STATUS		0x01
#define MXIC_WRITE_PAGE			0x02
#define MXIC_4xIO_WRITE_PAGE		0x38
#define MXIC_READ_DATA			0x03
#define	MXIC_FAST_READ			0x0B
#define	MXIC_FAST_READ_DT		0x0D
#define MXIC_2xIO_READ			0xBB
#define MXIC_2xIO_READ_DT		0xBD
#define MXIC_4xIO_READ			0xEB
#define MXIC_4xIO_READ_DT		0xED

// Cmd Type for choosing
typedef enum {
	MX25_SECTOR_ERASE = 0,
	MX25_BLOCK_32K_ERASE,
	MX25_BLOCK_64K_ERASE,
	MX25_MAX_ERASE_TYPE
} MX25_ERASE_TYPE;

typedef enum {
	MX25_PAGE_PROGRAM = 0,
	MX25_x4_IO_PAGE_PROGRAM,
	MX25_MAX_WRITE_TYPE
} MX25_WRITE_TYPE;

typedef enum {
	MX25_READ_DATA = 0,
	MX25_FAST_READ,
	MX25_FAST_READ_DT,
	MX25_x2_IO_READ,
	MX25_x2_IO_READ_DT,
	MX25_x4_IO_READ,
	MX25_x4_IO_READ_DT,
	MX25_MAX_READ_TYPE
} MX25_READ_TYPE;

// MXIC Status Mapping
#define MXIC_STS_BUSY_BIT		(BIT0)
#define MXIC_STS_WEL_BIT		(BIT1)
#define MXIC_STS_PROTECT_BITS		(BIT5| BIT4| BIT3| BIT2)
#define MXIC_STS_QUAD_MODE_BIT		(BIT6)
#define MXIC_STS_WR_STS_DISABLE_BIT	(BIT7)

extern struct spi_flash *spi_flash_probe_mxic(uint8_t * uint8_t_idcode);

#endif
