/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_spansion.c
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * -------------------------------------------------------------------------
 */
#include <common.h>
#include <malloc.h>

#include "ftspi020.h"

#if defined FTSPI020_SPANSION_H

#include "ftspi020_cntr.h"

static int spansion_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms);

struct spansion_spi_flash_params {
	uint16_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint16_t sector_size;
	const char *name;
};

static const struct spansion_spi_flash_params spansion_spi_flash_table[] = {

	{0x1502, 256, 16384, 4096, "S25FL032P"},
	{0x1820, 256, 65536, 4096, "S25FL0128P"},
};

char *span_er_string[S25_MAX_ERASE_TYPE] = {
	"Sector Erase",
	"8KB Block Erase",
	"64KB Block Erase"
};

char *span_wr_string[S25_MAX_WRITE_TYPE] = {
	"Page Program",
	"Quad Program"
};

char *span_rd_string[S25_MAX_READ_TYPE] = {
	"Read",
	"Fast Read",
	"Fast Read Dual Output",
	"Fast Read Quad Output",
	"Fast Read Dual IO",
	"Fast Read Quad IO",
};

uint32_t span_wait_ms;

static char *s25_action_get_string(uint32_t act, uint32_t type)
{
	if (act == WRITE)
		return span_wr_string[type];
	else if (act == READ)
		return span_rd_string[type];
	else
		return span_er_string[type];
}

static int spi_xfer_s25(struct spi_flash * flash, unsigned int len, const void *dout, void *din, unsigned long flags)
{

	struct ftspi020_cmd spi_cmd;
	uint8_t *data_out = (uint8_t *) dout;

	memset(&spi_cmd, 0, sizeof(struct ftspi020_cmd));

	if (flags & SPI_XFER_CMD_STATE) {

		spi_cmd.start_ce = flash->ce;
		spi_cmd.ins_code = *data_out;

		switch (*((uint8_t *) dout)) {
		case CMD_READ_ID:	// Read JEDEC ID
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = 3;
			break;
		case CMD_RESET:	// Reset the Flash
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_WRITE_ENABLE:
		case SPANSION_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_ERASE_SECTOR:
			spi_cmd.spi_addr =
			    (*(data_out + 1) << 16) | (*(data_out + 2) << 8) | (*(data_out + 3));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_READ_STATUS:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_enable;
			spi_cmd.read_status = read_status_by_hw;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_WRITE_PAGE:
			break;
		case SPANSION_READ_DATA:
			break;
		case SPANSION_READ_UNIQUE_ID:
			break;
		default:
			prints("%s: Wrong Command Code 0x%x.\n", flash->name, spi_cmd.ins_code);
			return 1;
		}
		FTSPI020_issue_cmd(&spi_cmd);

	} else if (flags & SPI_XFER_DATA_STATE) {
		FTSPI020_data_access((uint8_t *) dout, (uint8_t *) din, len);

	}

	if (flags & SPI_XFER_CHECK_CMD_COMPLETE) {
		if (FTSPI020_wait_cmd_complete(span_wait_ms))
			return 1;
	}
	return 0;
}

static int dataflash_read_fast_s25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int ret;
	uint8_t u8_rd_cmd[4];

	switch (type) {
	case S25_READ_DATA:
		u8_rd_cmd[0] = SPANSION_READ_DATA; 
		break;
	case S25_FAST_READ:
		u8_rd_cmd[0] = SPANSION_FAST_READ; 
		break;
	case S25_DUAL_READ:
		u8_rd_cmd[0] = SPANSION_DUAL_READ; 
		break;
	case S25_QUAD_READ:
		u8_rd_cmd[0] = SPANSION_QUAD_READ; 
		break;
	case S25_DUAL_IO_READ:
		u8_rd_cmd[0] = SPANSION_DUAL_IO_READ; 
		break;
	case S25_QUAD_IO_READ:
		u8_rd_cmd[0] = SPANSION_QUAD_IO_READ; 
		break;
	default:
		prints("%s: Unknown read type %zu bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	u8_rd_cmd[1] = offset & 0xFF;
	u8_rd_cmd[2] = ((offset & 0xFF00) >> 8);
	u8_rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	ret = spi_flash_cmd_read(flash, u8_rd_cmd, buf, len);
//      FTSPI020_show_content(buf, len);
	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    span_rd_string[type], len, offset);
	return ret;
}

static int dataflash_write_fast_s25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int start_page, offset_in_start_page, len_each_times;
	uint32_t original_offset = offset, original_len = len;
	uint8_t wr_en_cmd[1], u8_wr_cmd[4];
	uint8_t *u8_buf = buf, *alignment_buf;

	alignment_buf = (uint8_t *) malloc(flash->page_size);
	start_page = offset / flash->page_size;
	offset_in_start_page = offset % flash->page_size;

	/*
	   This judgement, "if(len + offset_in_start_page <= flash->page_size)"
	   is for the case of (offset + len) doesn't exceed the nearest next page boundary. 
	   0                                255
	   -------------------------------------
	   | | | | |.......................| | |
	   -------------------------------------
	   256                               511                                
	   -------------------------------------
	   | |*|*|*|.......................|*| |
	   -------------------------------------
	   512                               767
	   -------------------------------------
	   | | | | |.......................| | |
	   -------------------------------------
	 */
	if (len + offset_in_start_page <= flash->page_size) {
		len_each_times = len;
	} else {
		len_each_times = ((((start_page + 1) * flash->page_size) - 1) - offset) + 1;
	}

	spansion_check_status_till_ready(flash, 100);

	wr_en_cmd[0] = SPANSION_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	switch (type) {
	case S25_PAGE_PROGRAM:
		u8_wr_cmd[0] = SPANSION_WRITE_PAGE;
		break;
	case S25_QUAD_PROGRAM:
		u8_wr_cmd[0] = SPANSION_QUAD_WRITE_PAGE;
		break;
	default:
		prints("%s: Unknown write type %zu bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	u8_wr_cmd[1] = offset & 0xFF;
	u8_wr_cmd[2] = ((offset & 0xFF00) >> 8);
	u8_wr_cmd[3] = ((offset & 0xFF0000) >> 16);

	if (spi_flash_cmd_write(flash, u8_wr_cmd, u8_buf, len_each_times))
		return 1;

	u8_buf = u8_buf + len_each_times;

	do {
		// To avoid the "u8_buf" isn't alignment. 
		memcpy(alignment_buf, u8_buf, flash->page_size);

		len = len - len_each_times;
		offset = ((offset / flash->page_size) + 1) * flash->page_size;
		if (len < (flash->page_size)) {
			if (len == 0) {
				break;
			} else {
				len_each_times = len;
			}
		} else {
			len_each_times = flash->page_size;
		}

		spansion_check_status_till_ready(flash, 100);

		wr_en_cmd[0] = SPANSION_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		switch (type) {
		case S25_PAGE_PROGRAM:
			u8_wr_cmd[0] = SPANSION_WRITE_PAGE;
			break;
		case S25_QUAD_PROGRAM:
			u8_wr_cmd[0] = SPANSION_QUAD_WRITE_PAGE;
			break;
		}

		u8_wr_cmd[1] = offset & 0xFF;
		u8_wr_cmd[2] = ((offset & 0xFF00) >> 8);
		u8_wr_cmd[3] = ((offset & 0xFF0000) >> 16);

		//      prints("offset:0x%x len:0x%x\n", offset, len_each_times);
		if (spi_flash_cmd_write(flash, u8_wr_cmd, alignment_buf, len_each_times))
			return 1;
		u8_buf = u8_buf + len_each_times;

	} while (1);

	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA",  
		    span_wr_string[type], original_len, original_offset);
	free(alignment_buf);

	return 0;
}

static int dataflash_erase_fast_s25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[4];
	uint8_t cmd_code;
	uint32_t wait_t;

	if (type ==  S25_SECTOR_ERASE) {
		cmd_code =  SPANSION_ERASE_SECTOR;
		erase_size = flash->erase_sector_size;
		wait_t = 200;
	} else if (type ==  S25_BLOCK_8K_ERASE) {
		cmd_code =  SPANSION_ERASE_8K_BLOCK;
		erase_size = 8 << 10;
		wait_t = 400;
	} else {
		cmd_code = SPANSION_ERASE_64K_BLOCK;
		erase_size = 64 << 10;
		wait_t = 1000;
	}

	offset = offset + erase_size + ~(erase_size - 1);

	if (spansion_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = SPANSION_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		er_cmd[0] = cmd_code;
		er_cmd[1] = addr & 0xFF;
		er_cmd[2] = ((addr & 0xFF00) >> 8);
		er_cmd[3] = ((addr & 0xFF0000) >> 16);

		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		spansion_check_status_till_ready(flash, wait_t);

		prints("%s: %s @ 0x%x\n", flash->name, span_er_string[type], addr);

	}

	return ret;
}

static int dataflash_report_status_s25(struct spi_flash *flash)
{
	int ret;
	uint8_t rd_sts_cmd[1];

	span_wait_ms = 100;
	rd_sts_cmd[0] = SPANSION_READ_STATUS;
	ret = spi_flash_cmd(flash, rd_sts_cmd, NULL, 0);
	FTSPI020_show_status();

	prints("%s: Successfully read status\n", flash->name);
	return ret;
}

static int dataflash_erase_all_s25(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	span_wait_ms = 1000;

	er_all_cmd[0] = SPANSION_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = SPANSION_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	if (spansion_check_status_till_ready(flash, 75000))
		return 1;

	return 0;
}

static int spansion_check_status_till_ready(struct spi_flash * flash, uint32_t wait_ms)
{
	uint8_t rd_sts_cmd[1];
	uint8_t status;

	span_wait_ms = wait_ms;

	rd_sts_cmd[0] = SPANSION_READ_STATUS;
	do {
		if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
			prints("%s: Failed to check status by SW\n", flash->name);
			return 1;
		}

		FTSPI020_read_status(&status);
	} while ((g_check_status == check_status_by_sw) && (status & SPANSION_STS_BUSY_BIT));

	return 0;
}

struct spi_flash *spi_flash_probe_spansion(uint8_t * code)
{
	uint8_t i;
	uint16_t idcode;
	const struct spansion_spi_flash_params *params;
	struct spi_flash *spsf;

	memcpy(&idcode, (code + 1), 2);
	for (i = 0; i < 2; i++) {
		params = &spansion_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == 2) {
		prints("Spansion: Unsupported DataFlash ID 0x%02x\n", idcode);
		return NULL;
	}

	spsf = malloc(sizeof(struct spi_flash));
	if (!spsf) {
		prints("Spansion: Failed to allocate memory\n");
		return NULL;
	}

	spsf->name = params->name;
	spsf->code = params->idcode1_2;
	spsf->page_size = params->page_size;
	spsf->nr_pages = params->nr_pages ;
	spsf->size = params->page_size * params->nr_pages;
	spsf->erase_sector_size = params->sector_size;

	spsf->max_rd_type = W25_MAX_READ_TYPE;
	spsf->max_wr_type = W25_MAX_WRITE_TYPE;
	spsf->max_er_type = W25_MAX_ERASE_TYPE;

	switch (idcode) {
	case 0x1820:
	case 0x1502:
		prints("Find Flash Name: %s.\n", spsf->name);
		FTSPI020_busy_location(0);
		spsf->spi_xfer = spi_xfer_s25;		
		spsf->read = dataflash_read_fast_s25;
		spsf->write = dataflash_write_fast_s25;
		spsf->erase = dataflash_erase_fast_s25;
		spsf->erase_all = dataflash_erase_all_s25;
		spsf->report_status = dataflash_report_status_s25;
		spsf->get_string = s25_action_get_string;
		return spsf;
	default:
		prints("Spansion: Unsupported DataFlash family 0x%x\n", idcode);
		goto err;
		break;
	}

      err:
	free(spsf);
	return NULL;

}

#endif
