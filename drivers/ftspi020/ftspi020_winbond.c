/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_winbond.c
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * 2010/08/13   BingJiun	 - Scan all CEs for available flash.         
 *				 - Implement mode commands.        
 * -------------------------------------------------------------------------
 */
#include <common.h>
#include <malloc.h>

#include "ftspi020.h"

#if defined(FTSPI020_WINBOND_H)

#include "ftspi020_cntr.h"

static int winbond_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms);

struct winbond_spi_flash_params {
	uint16_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint16_t sector_size;
	const char *name;
};

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	/*{
	   .idcode1                = 0x15,      // Device ID
	   .l2_page_size           = 10,        
	   .pages_per_block        = 8,
	   .blocks_per_sector      = 32,
	   .nr_sectors             = 32,
	   .name                   = "W25Q32BV",
	   } */
	{0x1640, 256, 16384, 4096, "W25Q32BV"},
	{0x1840, 256, 65536, 4096, "W25Q128BV"},
};

char *w25_er_string[W25_MAX_ERASE_TYPE] = {
	"Sector Erase",
	"32KB Block Erase",
	"64KB Block Erase"
};

char *w25_wr_string[W25_MAX_WRITE_TYPE] = {
	"Page Program",
	"Quad Input Page Program"
};

char *w25_rd_string[W25_MAX_READ_TYPE] = {
	"Read",
	"Fast Read",
	"Fast Read Dual Output",
	"Fast Read Dual IO",
	"Fast Read Quad Output",
	"Fast Read Quad IO",
	"Word Read Quad IO",
};

uint32_t w25_wait_ms;

static char *w25_action_get_string(uint32_t act, uint32_t type)
{
	if (act == WRITE)
		return w25_wr_string[type];
	else if (act == READ)
		return w25_rd_string[type];
	else
		return w25_er_string[type];
}

static int winbond_set_quad_enable(struct spi_flash * flash, int en)
{
	uint8_t rd_sts_cmd[1];
	uint8_t wr_sts_cmd[1];
	uint8_t status[2], tmp;

	w25_wait_ms = 50;

	tmp = g_check_status;
	g_check_status = check_status_by_sw;

	rd_sts_cmd[0] = WINBOND_READ_STATUS1;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status[0]);
	if (debug > 2)
		prints(" Read status 1 0x%x \n", status[0]);

	rd_sts_cmd[0] = WINBOND_READ_STATUS2;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status[1]);
	if (debug > 2)
		prints(" Read status 2 0x%x \n", status[1]);

	if ((status[0] & WINBOND_STS_REG_PROTECT0) || 
	    (status[1] & WINBOND_STS_REG_PROTECT1))  {
		wr_sts_cmd[0] = WINBOND_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
			g_check_status = tmp;
			return 1;
		}
		// Clear the protect bit
		status[0] &= ~WINBOND_STS_REG_PROTECT0;
		status[1] &= ~WINBOND_STS_REG_PROTECT1;
		wr_sts_cmd[0] = WINBOND_WRITE_STATUS;
		if (debug > 2)
			prints(" Write status 0x%x 0x%x\n", status[0], status[1]);
		if (spi_flash_cmd_write(flash, wr_sts_cmd, (const void *) &status[0], 2)) {
			g_check_status = tmp;
			return 1;
		}
	}

	wr_sts_cmd[0] = WINBOND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	if (en)
		status[1] |= (WINBOND_STS_QUAD_ENABLE);
	else
		status[1] &= ~(WINBOND_STS_QUAD_ENABLE);
	wr_sts_cmd[0] = WINBOND_WRITE_STATUS;
	if (debug > 2)
		prints(" Write status 0x%x 0x%x\n", status[0], status[1]);
	if (spi_flash_cmd_write(flash, wr_sts_cmd, (const void *) &status[0], 2)) {
		g_check_status = tmp;
		return 1;
	}
	
	/* Wait for Write Status Complete */
	g_check_status = check_status_by_hw;
	if (winbond_check_status_till_ready(flash, 100))
		return 1;

	g_check_status = check_status_by_sw;
	rd_sts_cmd[0] = WINBOND_READ_STATUS2;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	status[1] = 0;
	FTSPI020_read_status(&status[1]);
	if (debug > 2)
		prints(" Read back status 2 0x%x \n", status[1]);
	
	if (en && !(status[1] & WINBOND_STS_QUAD_ENABLE)) {
		prints("%s: Enable Quad Mode failed.\n", flash->name);	
		g_check_status = tmp;
		return 1;
	}

	if (!en && (status[1] & WINBOND_STS_QUAD_ENABLE)) {
		prints("%s: Disable Quad Mode failed.\n", flash->name);	
		g_check_status = tmp;
		return 1;
	}
	g_check_status = tmp;

	return 0;
}

static int spi_xfer_w25(struct spi_flash * flash, unsigned int len, const void *dout, void *din, unsigned long flags)
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
		case WINBOND_WRITE_ENABLE:
		case WINBOND_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case WINBOND_ERASE_SECTOR:
		case WINBOND_ERASE_32K_BLOCK:
		case WINBOND_ERASE_64K_BLOCK:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case WINBOND_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case WINBOND_WRITE_STATUS:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;
			break;
		case WINBOND_READ_STATUS1:
		case WINBOND_READ_STATUS2:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_enable;
			if (g_check_status == check_status_by_sw) {
				spi_cmd.read_status = read_status_by_sw;
			} else {
				spi_cmd.read_status = read_status_by_hw;
			}
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case WINBOND_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case WINBOND_QUAD_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case WINBOND_READ_DATA:
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
		case WINBOND_FAST_READ:
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
		case WINBOND_FAST_READ_DUAL:
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
		case WINBOND_FAST_READ_DUAL_IO:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case WINBOND_FAST_READ_QUAD:
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
		case WINBOND_FAST_READ_QUAD_IO:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.dum_2nd_cyc = 4;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
			break;
		case WINBOND_WORD_READ_QUAD_IO:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.dum_2nd_cyc = 2;
			spi_cmd.conti_read_mode_en = 1;
			spi_cmd.conti_read_mode_code = 0;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
			break;
		case WINBOND_READ_UNIQUE_ID:
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
		if (FTSPI020_wait_cmd_complete(w25_wait_ms))
			return 1;
	}

	return 0;
}

static int dataflash_read_fast_w25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int ret;
	uint8_t rd_cmd[4];

	if (type == W25_FAST_READ_QUAD || type == W25_FAST_READ_QUAD_IO || type == W25_WORD_READ_QUAD_IO)
		ret = winbond_set_quad_enable(flash, 1);
	else
		ret = winbond_set_quad_enable(flash, 0);

	if (ret)
		return 1;

	if (winbond_check_status_till_ready(flash, 100))
		return 1;

	switch (type) {
	case W25_READ_DATA:
		rd_cmd[0] = WINBOND_READ_DATA;
		break;
	case W25_FAST_READ:
		rd_cmd[0] = WINBOND_FAST_READ;
		break;
	case W25_FAST_READ_DUAL:
		rd_cmd[0] = WINBOND_FAST_READ_DUAL;
		break;
	case W25_FAST_READ_DUAL_IO:
		rd_cmd[0] = WINBOND_FAST_READ_DUAL_IO;
		break;
	case W25_FAST_READ_QUAD:
		rd_cmd[0] = WINBOND_FAST_READ_QUAD;
		break;
	case W25_FAST_READ_QUAD_IO:
		rd_cmd[0] = WINBOND_FAST_READ_QUAD_IO;
		break;
	case W25_WORD_READ_QUAD_IO:
		rd_cmd[0] = WINBOND_WORD_READ_QUAD_IO;
		break;
	default:
		prints("%s: Unknown read type %d bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);
//      FTSPI020_show_content(buf, len);
	prints("%s: %s: %s %d bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    w25_rd_string[type], len, offset);
	return ret;
}

static int dataflash_write_fast_w25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int ret;
	int start_page, offset_in_start_page, len_each_times;
	uint32_t original_offset = offset, original_len = len;
	uint8_t wr_en_cmd[1], wr_cmd[4];
	uint8_t *buff = buf, *alignment_buf;

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

	if (type == W25_PAGE_PROGRAM)
		ret = winbond_set_quad_enable(flash, 0);
	else
		ret = winbond_set_quad_enable(flash, 1);
	if (ret)
		return ret;
	if (winbond_check_status_till_ready(flash, 100)) {
		prints ("%s: write type %s, set quad failed\n", w25_wr_string[type]);
		return 1;
	}

	wr_en_cmd[0] = WINBOND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	switch (type) {
	case W25_PAGE_PROGRAM:
		wr_cmd[0] = WINBOND_WRITE_PAGE;
		break;
	case W25_QUAD_PROGRAM:
		wr_cmd[0] = WINBOND_QUAD_WRITE_PAGE;
		break;
	default:
		prints("%s: Unknown write type %d bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	wr_cmd[1] = offset & 0xFF;
	wr_cmd[2] = ((offset & 0xFF00) >> 8);
	wr_cmd[3] = ((offset & 0xFF0000) >> 16);

	if (spi_flash_cmd_write(flash, wr_cmd, buff, len_each_times))
		return 1;

	buff = buff + len_each_times;

	do {
		// To avoid the "buff" isn't alignment. 
		memcpy(alignment_buf, buff, flash->page_size);

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

		if (winbond_check_status_till_ready(flash, 100))
			return 1;

		wr_en_cmd[0] = WINBOND_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		switch (type) {
		case W25_PAGE_PROGRAM:
			wr_cmd[0] = WINBOND_WRITE_PAGE;
			break;
		case W25_QUAD_PROGRAM:
			wr_cmd[0] = WINBOND_QUAD_WRITE_PAGE;
			break;
		}

		wr_cmd[1] = offset & 0xFF;
		wr_cmd[2] = ((offset & 0xFF00) >> 8);
		wr_cmd[3] = ((offset & 0xFF0000) >> 16);

		//      prints("offset:0x%x len:0x%x\n", offset, len_each_times);
		if (spi_flash_cmd_write(flash, wr_cmd, alignment_buf, len_each_times))
			return 1;
		buff = buff + len_each_times;

	} while (1);

	prints("%s: %s: %s %d bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA",  
		    w25_wr_string[type], original_len, original_offset);
	free(alignment_buf);

	return 0;
}

static int dataflash_erase_fast_w25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[4];
	uint8_t cmd_code;
	uint32_t wait_t;

	if (type ==  W25_SECTOR_ERASE) {
		cmd_code =  WINBOND_ERASE_SECTOR;
		erase_size = flash->erase_sector_size;
		wait_t = 200;
	} else if (type ==  W25_BLOCK_32K_ERASE) {
		cmd_code =  WINBOND_ERASE_32K_BLOCK;
		erase_size = 32 << 10;
		wait_t = 800;
	} else {
		cmd_code = WINBOND_ERASE_64K_BLOCK;
		erase_size = 64 << 10;
		wait_t = 1000;
	}

	offset = offset + erase_size + ~(erase_size - 1);

	if (winbond_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = WINBOND_WRITE_ENABLE;
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

		if (winbond_check_status_till_ready(flash, wait_t))
			return 1;

		prints("%s: %s @ 0x%x\n", flash->name, w25_er_string[type], addr);

	}

	return ret;
}

static int dataflash_report_status_w25(struct spi_flash *flash)
{
	int ret;
	uint8_t rd_sts_cmd[1];

	if (winbond_check_status_till_ready(flash, 100))
		return 1;

	rd_sts_cmd[0] = WINBOND_READ_STATUS1;
	ret = spi_flash_cmd(flash, rd_sts_cmd, NULL, 0);
	FTSPI020_show_status();

	rd_sts_cmd[0] = WINBOND_READ_STATUS2;
	ret = spi_flash_cmd(flash, rd_sts_cmd, NULL, 0);
	FTSPI020_show_status();

	prints("%s: Successfully read status\n", flash->name);
	return ret;
}

static int dataflash_erase_all_w25(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	w25_wait_ms = 1000;

	er_all_cmd[0] = WINBOND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = WINBOND_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	if (winbond_check_status_till_ready(flash, 50000))
		return 1;

	return 0;
}

static int winbond_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms)
{
	uint8_t rd_sts_cmd[1];
	uint8_t status;

	w25_wait_ms = wait_ms;

	rd_sts_cmd[0] = WINBOND_READ_STATUS1;
	do {
		if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
			prints("%s: Failed to check status by SW\n", flash->name);
			return 1;
		}

		FTSPI020_read_status(&status);
	} while ((g_check_status == check_status_by_sw) && (status & WINBOND_STS_BUSY));

	return 0;
}

struct spi_flash *spi_flash_probe_winbond(uint8_t * code)
{
	uint8_t i;
	uint16_t idcode;
	const struct winbond_spi_flash_params *params;
	struct spi_flash *wsf;

	memcpy(&idcode, (code + 1), 2);
	for (i = 0; i < 2; i++) {
		params = &winbond_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == 3) {
		prints("Winbond: Unsupported DataFlash ID %04x\n", idcode);
		return NULL;
	}

	wsf = malloc(sizeof(struct spi_flash));
	if (!wsf) {
		prints("Winbond: Failed to allocate memory\n");
		return NULL;
	}

	wsf->name = params->name;
	wsf->code = params->idcode1_2;
	wsf->page_size = params->page_size;
	wsf->nr_pages = params->nr_pages ;
	wsf->size = params->page_size * params->nr_pages;
	wsf->erase_sector_size = params->sector_size;

	wsf->max_rd_type = W25_MAX_READ_TYPE;
	wsf->max_wr_type = W25_MAX_WRITE_TYPE;
	wsf->max_er_type = W25_MAX_ERASE_TYPE;

	switch (idcode) {
	case 0x1640:
	case 0x1840:
		prints("Find Flash Name: %s.\n", wsf->name);
		FTSPI020_busy_location(BUSY_BIT0);
		wsf->spi_xfer = spi_xfer_w25;		
		wsf->read = dataflash_read_fast_w25;
		wsf->write = dataflash_write_fast_w25;
		wsf->erase = dataflash_erase_fast_w25;
		wsf->erase_all = dataflash_erase_all_w25;
		wsf->report_status = dataflash_report_status_w25;
		wsf->get_string = w25_action_get_string;
		return wsf;
	default:
		prints("Winbond: Unsupported DataFlash family %u\n", idcode);
		goto err;
	}

      err:
	free(wsf);
	return NULL;

}

#endif
