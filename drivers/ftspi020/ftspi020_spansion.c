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

char *span_er_string[S25_MAX_ERASE_TYPE + 1] = {
	"Sector Erase",
	"8KB Block Erase",
	"64KB Block Erase",
	"Invalid erase type"
};

char *span_wr_string[S25_MAX_WRITE_TYPE + 1] = {
	"Page Program",
	"Quad Program",
	"Invalid write type"
};

char *span_rd_string[S25_MAX_READ_TYPE + 1] = {
	"Read",
	"Fast Read",
	"Fast Read Dual Output",
	"Fast Read Quad Output",
	"Fast Read Dual IO",
	"Fast Read Quad IO",
	"Invalid read type"
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

/**
 * Write Register with one byte: only the status is written.
 * Must write two bytes if you want to write the register
 */
static int spansion_set_quad_enable(struct spi_flash * flash, int en)
{
	uint8_t rd_sts_cmd[1];
	uint8_t wr_sts_cmd[1];
	uint8_t status[2], tmp;

	span_wait_ms = 50;

	tmp = g_check_status;
	g_check_status = check_status_by_sw;

wr_en:
	wr_sts_cmd[0] = SPANSION_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}

	rd_sts_cmd[0] = SPANSION_READ_STATUS;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status[0]);
	if (debug > 2)
		prints(" Read status register 0x%x \n", &status[0]);

	if (!(status[0] & SPANSION_STS_WEL_BIT)) {
		prints(" Write Enable Latch bit not set \n");
		goto wr_en;
	}

	rd_sts_cmd[0] = SPANSION_READ_CONFIG;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status[1]);
	if (debug > 2)
		prints(" Read configuration register 0x%x \n", &status[1]);

	if (en)
		status[1] |= SPANSION_CFG_QUAD_MODE;
	else
		status[1] &= ~SPANSION_CFG_QUAD_MODE;

	wr_sts_cmd[0] = SPANSION_WRITE_STATUS_CONF;
	if (debug > 2)
		prints(" Write status 0x%x 0x%x\n", status[0], status[1]);
	if (spi_flash_cmd_write(flash, wr_sts_cmd, (const void *) &status[0], 2)) {
		g_check_status = tmp;
		return 1;
	}

	/* Wait for Write Status Complete */
	g_check_status = check_status_by_hw;
	if (spansion_check_status_till_ready(flash, 100))
		return 1;

	g_check_status = check_status_by_sw;
	status[1] = 0;
	rd_sts_cmd[0] = SPANSION_READ_CONFIG;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status[1]);
	if (debug > 2)
		prints(" Read back status 0x%x \n", &status[1]);

	if (en && !(status[1] & SPANSION_CFG_QUAD_MODE)) {
		prints("%s: Enable Quad Mode failed.\n", flash->name);
		g_check_status = tmp;
		return 1;
	}

	if (!en && (status[1] & SPANSION_CFG_QUAD_MODE)) {
		prints("%s: Disable Quad Mode failed.\n", flash->name);	
		g_check_status = tmp;
		return 1;
	}
	g_check_status = tmp;

	return 0;
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
		case SPANSION_ERASE_8K_BLOCK:
		case SPANSION_ERASE_64K_BLOCK:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
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
		case SPANSION_READ_CONFIG:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_enable;
			spi_cmd.read_status = read_status_by_hw;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_WRITE_STATUS_CONF:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;
			break;
		case SPANSION_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_QUAD_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case SPANSION_READ_DATA:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_FAST_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPANSION_DUAL_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_dual_mode;
			break;
		case SPANSION_QUAD_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case SPANSION_DUAL_IO_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0xA0;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case SPANSION_QUAD_IO_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 4;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0xA0;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
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
	uint8_t rd_cmd[4];

	if (type == S25_QUAD_READ || type == S25_QUAD_IO_READ)
		ret = spansion_set_quad_enable(flash, 1);
	else
		ret = spansion_set_quad_enable(flash, 0);

	if (ret)
		return 1;

	switch (type) {
	case S25_READ_DATA:
		rd_cmd[0] = SPANSION_READ_DATA;
		break;
	case S25_FAST_READ:
		rd_cmd[0] = SPANSION_FAST_READ;
		break;
	case S25_DUAL_READ:
		rd_cmd[0] = SPANSION_DUAL_READ;
		break;
	case S25_QUAD_READ:
		rd_cmd[0] = SPANSION_QUAD_READ;
		break;
	case S25_DUAL_IO_READ:
		rd_cmd[0] = SPANSION_DUAL_IO_READ;
		break;
	case S25_QUAD_IO_READ:
		rd_cmd[0] = SPANSION_QUAD_IO_READ;
		break;
	default:
		prints("%s: Unknown read type %zu bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);
	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    span_rd_string[type], len, offset);

	if (type == S25_QUAD_IO_READ || type == S25_DUAL_IO_READ) {
		rd_cmd[0] = SPANSION_WRITE_DISABLE;
		ret = spi_flash_cmd(flash, rd_cmd, NULL, 0);
	}

	return ret;
}

static int dataflash_write_fast_s25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *outbuf)
{
	int start_page, offset_in_start_page, len_each_times, ret;
	uint32_t original_offset = offset, original_len = len;
	uint8_t wr_en_cmd[1], wr_cmd[4];
	uint8_t *buf = outbuf, *alignment_buf;

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

	if (type == S25_PAGE_PROGRAM)
		ret = spansion_set_quad_enable(flash, 0);
	else
		ret = spansion_set_quad_enable(flash, 1);

	if (ret)
		return ret;
	if (spansion_check_status_till_ready(flash, 100)) {
		prints ("%s: write type %s, set quad failed\n", span_wr_string[type]);
		return 1;
	}

	wr_en_cmd[0] = SPANSION_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	switch (type) {
	case S25_PAGE_PROGRAM:
		wr_cmd[0] = SPANSION_WRITE_PAGE;
		break;
	case S25_QUAD_PROGRAM:
		wr_cmd[0] = SPANSION_QUAD_WRITE_PAGE;
		break;
	default:
		prints("%s: Unknown write type %zu bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	wr_cmd[1] = offset & 0xFF;
	wr_cmd[2] = ((offset & 0xFF00) >> 8);
	wr_cmd[3] = ((offset & 0xFF0000) >> 16);

	if (spi_flash_cmd_write(flash, wr_cmd, buf, len_each_times))
		return 1;

	buf = buf + len_each_times;

	do {
		// To avoid the "buf" isn't alignment.
		memcpy(alignment_buf, buf, flash->page_size);

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
			wr_cmd[0] = SPANSION_WRITE_PAGE;
			break;
		case S25_QUAD_PROGRAM:
			wr_cmd[0] = SPANSION_QUAD_WRITE_PAGE;
			break;
		}

		wr_cmd[1] = offset & 0xFF;
		wr_cmd[2] = ((offset & 0xFF00) >> 8);
		wr_cmd[3] = ((offset & 0xFF0000) >> 16);

		//      prints("offset:0x%x len:0x%x\n", offset, len_each_times);
		if (spi_flash_cmd_write(flash, wr_cmd, alignment_buf, len_each_times))
			return 1;
		buf = buf + len_each_times;

	} while (1);

	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA",  
		    span_wr_string[type], original_len, original_offset);
	free(alignment_buf);

	return 0;
}

/*
 * If TBPARM=0, valid address for 4K and 8K erase is 0 to 0x1FFFF.
 */
static int dataflash_erase_fast_s25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[4];
	uint8_t cmd_code, status, tmp;
	uint32_t wait_t;

	if (type ==  S25_SECTOR_ERASE) {
		cmd_code =  SPANSION_ERASE_SECTOR;
		erase_size = flash->erase_sector_size;
		wait_t = 200;
	} else if (type ==  S25_BLOCK_8K_ERASE) {
		cmd_code =  SPANSION_ERASE_8K_BLOCK;
		erase_size = 8 << 10;
		wait_t = 400;
	} else if (type == S25_BLOCK_64K_ERASE) {
		cmd_code = SPANSION_ERASE_64K_BLOCK;
		erase_size = 64 << 10;
		wait_t = 1000;
	} else {
		prints("%s: %s @ 0x%x\n", flash->name, span_er_string[type], addr);
		return 1;
	}

	offset &= ~(erase_size - 1);

	if (spansion_check_status_till_ready(flash, 100))
		return 1;

	tmp = g_check_status;
	g_check_status = check_status_by_sw;
	er_cmd[0] = SPANSION_READ_STATUS;
	ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
	g_check_status = tmp;
	FTSPI020_read_status(&status);
	if (status & SPANSION_STS_BPROTECT_BITS)
		prints(" Block protected bits set 0x%x \n", status);

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = SPANSION_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		/* Assume TBPARM=0, valid address for 4K and 8K erase is 0 to 0x1FFFF. */
		if ((addr > 0x1FFFF) && (cmd_code != SPANSION_ERASE_64K_BLOCK)) {
			cmd_code = SPANSION_ERASE_64K_BLOCK;
			type = S25_BLOCK_64K_ERASE;
			erase_size = 64 << 10;
			wait_t = 1000;
			addr &= ~(erase_size - 1);
		}

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

	spsf->max_rd_type = S25_MAX_READ_TYPE;
	spsf->max_wr_type = S25_MAX_WRITE_TYPE;
	spsf->max_er_type = S25_MAX_ERASE_TYPE;

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
