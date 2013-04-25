/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_sst.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * -------------------------------------------------------------------------
 */

#ifndef FTSPI020_SST_H
#define FTSPI020_SST_H

#define SST_WRITE_ENABLE		0x06
#define SST_WRITE_DISABLE		0x04
#define SST_ERASE_SECTOR		0x20
#define SST_ERASE_32K_BLOCK		0x52
#define SST_ERASE_64K_BLOCK		0xD8
#define SST_ERASE_CHIP			0xC7	// or 0x60
#define SST_READ_STATUS			0x05
#define SST_WRITE_STATUS_ENABLE		0x50
#define SST_WRITE_STATUS		0x01
#define SST_BYTE_PROGRAM		0x02	// One byte program
#define SST_AUTO_ADDR_INC_PROGRAM	0xAD	// for programming flash with the amount of data from 2 to infinity bytes.
#define SST_EBSY			0x70
#define SST_DBSY			0x80
#define SST_READ_DATA			0x03	// for the clock freq. below 25MHz
#define SST_READ_DATA_HIGH_SPEED	0x0B	// for the clock freq. below 50MHz

// SST Status Mapping
#define SST_STS_BUSY_BIT		(BIT0)
#define SST_STS_WEL_BIT			(BIT1)
#define SST_STS_PROTECT_BITS		(BIT5| BIT4| BIT3| BIT2)
#define SST_STS_AAI_MODE_BIT		(BIT6)
#define SST_STS_PROTECT_MODE_BIT	(BIT7)

typedef enum {
	SST25_SECTOR_ERASE = 0,
	SST25_BLOCK_32K_ERASE,
	SST25_BLOCK_64K_ERASE,
	SST25_MAX_ERASE_TYPE
} SST25_ERASE_TYPE;

typedef enum {
	SST25_BYTE_PROGRAM = 0,
	SST25_AUTO_ADDR_INC_PROGRAM,
	SST25_MAX_WRITE_TYPE
} SST25_WRITE_TYPE;

typedef enum {
	SST25_READ_DATA = 0,
	SST25_READ_DATA_HIGH_SPEED,
	SST25_MAX_READ_TYPE
} SST25_READ_TYPE;

extern struct spi_flash *spi_flash_probe_sst(uint8_t * uint8_t_idcode);

#endif
