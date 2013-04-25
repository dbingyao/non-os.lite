/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_mxic.c
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

#if defined(FTSPI020_MXIC_H)

#include "ftspi020_cntr.h"

static int mxic_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms);

struct mxic_spi_flash_params {
	uint16_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint16_t sector_size;
	const char *name;
};

static const struct mxic_spi_flash_params mxic_spi_flash_table[] = {

	{0x1820, 256, 65536, 4096, "MX25L12845EM1"},
};

char *mx25_er_string[MX25_MAX_ERASE_TYPE] = {
	"Sector Erase",
	"32KB Block Erase",
	"64KB Block Erase"
};

char *mx25_wr_string[MX25_MAX_WRITE_TYPE] = {
	"Page Program",
	"4xI/O Page Program"
};

char *mx25_rd_string[MX25_MAX_READ_TYPE] = {
	"Read",
	"Fast Read",
	"Fast Double Transfer Rate Read",
	"2xI/O Read Mode",
	"2xI/O Doube Transfer Rate Read",
	"4xI/O Read Mode",
	"4xI/O Doube Transfer Rate Read"
};

uint32_t mx25_wait_ms;

static int mxic_set_quad_enable(struct spi_flash * flash, int en)
{
	uint8_t rd_sts_cmd[1];
	uint8_t wr_sts_cmd[1];
	uint8_t status, tmp;

	mx25_wait_ms = 50;
	tmp = g_check_status;
	g_check_status = check_status_by_sw;

	rd_sts_cmd[0] = MXIC_READ_STATUS;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status);

	if (status & MXIC_STS_WR_STS_DISABLE_BIT) {
		wr_sts_cmd[0] = MXIC_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
			g_check_status = tmp;
			return 1;
		}
		// Clear the protect bit
		status &= ~(MXIC_STS_WR_STS_DISABLE_BIT);
		wr_sts_cmd[0] = MXIC_WRITE_STATUS;
		if (spi_flash_cmd_write(flash, wr_sts_cmd, (const void *) &status, 1)) {
			g_check_status = tmp;
			return 1;
		}
	}

	wr_sts_cmd[0] = MXIC_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}

	if (en)
		status |= (MXIC_STS_QUAD_MODE_BIT);
	else
		status &= ~(MXIC_STS_QUAD_MODE_BIT);
	wr_sts_cmd[0] = MXIC_WRITE_STATUS;
	if (spi_flash_cmd_write(flash, wr_sts_cmd, (const void *) &status, 1)) {
		g_check_status = tmp;
		return 1;
	}

	/* Wait for Write Status Complete */
	g_check_status = check_status_by_hw;
	mxic_check_status_till_ready(flash, 100);

	g_check_status = check_status_by_sw;
	rd_sts_cmd[0] = MXIC_READ_STATUS;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status);

	if (en && !(status & MXIC_STS_QUAD_MODE_BIT)) {
		prints("%s: Enable Quad Mode failed.\n", flash->name);	
		g_check_status = tmp;
		return 1;
	}

	if (!en && (status & MXIC_STS_QUAD_MODE_BIT)) {
		prints("%s: Disable Quad Mode failed.\n", flash->name);	
		g_check_status = tmp;
		return 1;
	}
	g_check_status = tmp;
#if 0
	FTSPI020_show_status();
#endif
	return 0;
}

static char *mx25_action_get_string(uint32_t act, uint32_t type)
{
	if (act == WRITE)
		return mx25_wr_string[type];
	else if (act == READ)
		return mx25_rd_string[type];
	else
		return mx25_er_string[type];
}

static int spi_xfer_mx25(struct spi_flash * flash, unsigned int len, const void *dout, void *din, unsigned long flags)
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
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = 3;
			break;
		case MXIC_WRITE_ENABLE:
		case MXIC_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_ERASE_SECTOR:
		case MXIC_ERASE_32K_BLOCK:
		case MXIC_ERASE_64K_BLOCK:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status = read_status_by_hw;
			break;
		case MXIC_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_READ_STATUS:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_enable;
			if (g_check_status == check_status_by_sw) {
				spi_cmd.read_status = read_status_by_sw;
			} else {
				spi_cmd.read_status = read_status_by_hw;
			}
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_WRITE_STATUS:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;
			break;
		case MXIC_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_4xIO_WRITE_PAGE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
			break;
		case MXIC_READ_DATA:
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
		case MXIC_FAST_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_FAST_READ_DT:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 6;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case MXIC_2xIO_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 4;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case MXIC_2xIO_READ_DT:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 6;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			spi_cmd.spi_mode = spi_operate_dualio_mode;
			break;
		case MXIC_4xIO_READ:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 4;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
			spi_cmd.conti_read_mode_en = op_code_1byte;
			/* Enhanced Performance Mode */
			spi_cmd.conti_read_mode_code = 0; //0xF0;
			break;
		case MXIC_4xIO_READ_DT:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.dum_2nd_cyc = 7;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_enable;
			spi_cmd.spi_mode = spi_operate_quadio_mode;
			spi_cmd.conti_read_mode_en = op_code_1byte;
			/* Enhanced Performance Mode */
			spi_cmd.conti_read_mode_code = 0;//0xF0;
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
		if (FTSPI020_wait_cmd_complete(mx25_wait_ms))
			return 1;
	}

	return 0;
}

static int dataflash_read_fast_mx25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int ret, tmp;
	uint8_t rd_cmd[4];

	if ((type == MX25_FAST_READ_DT || type == MX25_x2_IO_READ_DT || type == MX25_x4_IO_READ_DT) 
	    && !FTSPI020_support_dtr_mode())
		return 2;

	if (type == MX25_x4_IO_READ || type == MX25_x4_IO_READ_DT)
		mxic_set_quad_enable(flash, 1);
	else
		mxic_set_quad_enable(flash, 0);
	
	if (type == MX25_FAST_READ_DT || type == MX25_x2_IO_READ_DT || type == MX25_x4_IO_READ_DT) {
		tmp = g_divider << 1;
		if (tmp > 8)
			tmp = 8;

		FTSPI020_divider(tmp);
	} else
		FTSPI020_divider(g_divider);

	mxic_check_status_till_ready(flash, 100);
	switch (type) {
	case MX25_READ_DATA:
		rd_cmd[0] = MXIC_READ_DATA;
		break;
	case MX25_FAST_READ:
		rd_cmd[0] = MXIC_FAST_READ;
		break;
	case MX25_FAST_READ_DT:
		rd_cmd[0] = MXIC_FAST_READ_DT;
		break;
	case MX25_x2_IO_READ:
		rd_cmd[0] = MXIC_2xIO_READ;
		break;
	case MX25_x2_IO_READ_DT:
		rd_cmd[0] = MXIC_2xIO_READ_DT;
		break;
	case MX25_x4_IO_READ:
		rd_cmd[0] = MXIC_4xIO_READ;
		break;
	case MX25_x4_IO_READ_DT:
		rd_cmd[0] = MXIC_4xIO_READ_DT;
		break;
	default:
		prints("%s: Unknown read type %zu bytes @ %d\n", flash->name, len, offset);
		return 1;
	}

	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);
//      FTSPI020_show_content(buf, len);
	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    mx25_rd_string[type], len, offset);
	return ret;
}

static int dataflash_write_fast_mx25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
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
	   This judgement, "if(len + offset_in_start_page <= wsf->params->page_size)"
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

	if (type == MX25_x4_IO_PAGE_PROGRAM)
		ret = mxic_set_quad_enable(flash, 1);
	else
		ret = mxic_set_quad_enable(flash, 0);
	if (ret)
		return ret;
	mxic_check_status_till_ready(flash, 100);

	wr_en_cmd[0] = MXIC_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
		return 1;

	//wr_cmd[0] = mxic_WRITE_PAGE;
	switch (type) {
	case MX25_PAGE_PROGRAM:
		wr_cmd[0] = MXIC_WRITE_PAGE;
		break;
	case MX25_x4_IO_PAGE_PROGRAM:
		wr_cmd[0] = MXIC_4xIO_WRITE_PAGE;
		break;
	default:
		prints("%s: Unknown write type %zu bytes @ %d\n", flash->name, len, offset);
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

		mxic_check_status_till_ready(flash, 100);

		wr_en_cmd[0] = MXIC_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		if (type == MX25_PAGE_PROGRAM)
			wr_cmd[0] = MXIC_WRITE_PAGE;
		else // x4_IO_PAGE_PROGRAM:
			wr_cmd[0] = MXIC_4xIO_WRITE_PAGE;

		wr_cmd[1] = offset & 0xFF;
		wr_cmd[2] = ((offset & 0xFF00) >> 8);
		wr_cmd[3] = ((offset & 0xFF0000) >> 16);

		//      prints("offset:0x%x len:0x%x\n", offset, len_each_times);
		if (spi_flash_cmd_write(flash, wr_cmd, alignment_buf, len_each_times))
			return 1;
		buff = buff + len_each_times;

	} while (1);

	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    mx25_wr_string[type], original_len, original_offset);

	free(alignment_buf);
	return 0;
}

static int dataflash_erase_fast_mx25(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[4], cmd_code;
	uint32_t wait_t;

	if (type ==  MX25_SECTOR_ERASE) {
		cmd_code =  MXIC_ERASE_SECTOR;
		erase_size = flash->erase_sector_size;
		wait_t = 300;
	} else if (type == MX25_BLOCK_32K_ERASE) {
		cmd_code = MXIC_ERASE_32K_BLOCK;
		erase_size = 32 << 10;
		wait_t = 2000;
	} else {
		cmd_code = MXIC_ERASE_64K_BLOCK;
		erase_size = 64 << 10;
		wait_t = 2000;
	}

	offset = offset + erase_size + ~(erase_size - 1);

	if (mxic_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = MXIC_WRITE_ENABLE;
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

		mxic_check_status_till_ready(flash, wait_t);

		prints("%s: %s @ 0x%x\n", flash->name, mx25_er_string[type], addr);

	}

	return ret;
}

static int dataflash_report_status_mx25(struct spi_flash *flash)
{
	int ret;
	uint8_t rd_sts_cmd[1];

	mx25_wait_ms = 100;
	rd_sts_cmd[0] = MXIC_READ_STATUS;
	ret = spi_flash_cmd(flash, rd_sts_cmd, NULL, 0);
	FTSPI020_show_status();

	prints("%s: Successfully read status\n", flash->name);
	return ret;
}

static int dataflash_erase_all_mx25(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	er_all_cmd[0] = MXIC_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = MXIC_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	mxic_check_status_till_ready(flash, 80000);
	return 0;
}

static int mxic_check_status_till_ready(struct spi_flash * flash, uint32_t wait_ms)
{
	uint8_t rd_sts_cmd[1];
	uint8_t status;

	mx25_wait_ms = wait_ms;

	rd_sts_cmd[0] = MXIC_READ_STATUS;
	do {
		if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
			prints("%s: Failed to check status by SW\n", flash->name);
			return 1;
		}

		FTSPI020_read_status(&status);
	} while ((g_check_status == check_status_by_sw) && (status & MXIC_STS_BUSY_BIT));

	return 0;
}

struct spi_flash *spi_flash_probe_mxic(uint8_t * code)
{
	uint8_t i;
	uint16_t idcode;
	const struct mxic_spi_flash_params *params;
	struct spi_flash *msf;

	memcpy(&idcode, (code + 1), 2);
	for (i = 0; i < 2; i++) {
		params = &mxic_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == 3) {
		prints("MXIC: Unsupported DataFlash ID %04x\n", idcode);
		return NULL;
	}

	msf = malloc(sizeof(struct spi_flash));
	if (!msf) {
		prints("MXIC: Failed to allocate memory\n");
		return NULL;
	}

	msf->name = params->name;
	msf->code = params->idcode1_2;
	msf->page_size = params->page_size;
	msf->nr_pages = params->nr_pages ;
	msf->size = params->page_size * params->nr_pages;
	msf->erase_sector_size = params->sector_size;

	msf->max_rd_type = MX25_MAX_READ_TYPE;
        msf->max_wr_type = MX25_MAX_WRITE_TYPE;
        msf->max_er_type = MX25_MAX_ERASE_TYPE;

	switch (idcode) {
	case 0x1820:
		prints("Find Flash Name: %s\n", msf->name);
		FTSPI020_busy_location(0);
		msf->spi_xfer = spi_xfer_mx25;
		msf->read = dataflash_read_fast_mx25;
		msf->write = dataflash_write_fast_mx25;
		msf->erase = dataflash_erase_fast_mx25;
		msf->erase_all = dataflash_erase_all_mx25;
		msf->report_status = dataflash_report_status_mx25;
		msf->get_string = mx25_action_get_string;
		return msf;
	default:
		prints("MXIC: Unsupported DataFlash family 0x%u\n", idcode);
		goto err;
	}

      err:
	free(msf);
	return NULL;

}
#endif
