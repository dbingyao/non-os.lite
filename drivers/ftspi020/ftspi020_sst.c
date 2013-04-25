/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_sst.c
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

#if defined(FTSPI020_SST_H)

#include "ftspi020_cntr.h"

static int sst_check_status_til_ready(struct spi_flash * flash, uint32_t wait_ms);
static int  sst_check_write_protect(struct spi_flash * flash);
// For the special AAI two-bytes programming in SST
uint8_t g_AAI_state;

struct sst_spi_flash_params {
	uint16_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint16_t sector_size;
	const char *name;
};

static const struct sst_spi_flash_params sst_spi_flash_table[] = {

	{0x8E25, 2, 524288, 4096, "SST25VF080B"},
};


char *sst25_er_string[SST25_MAX_ERASE_TYPE] = {
	"Sector Erase",
	"32KB Block Erase",
	"64KB Block Erase"
};

char *sst25_wr_string[SST25_MAX_WRITE_TYPE] = {
	"Byte Program",
	"Auto Address Increment Program"
};

char *sst25_rd_string[SST25_MAX_READ_TYPE] = {
	"Read",
	"High-Speed Read",
};

uint32_t sst25_wait_ms;

static char *sst25_action_get_string(uint32_t act, uint32_t type)
{
	if (act == WRITE)
		return sst25_wr_string[type];
	else if (act == READ)
		return sst25_rd_string[type];
	else
		return sst25_er_string[type];
}

int spi_xfer_25vf(struct spi_flash * flash, unsigned int len, const void *dout, void *din, unsigned long flags)
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
		case SST_WRITE_ENABLE:
		case SST_WRITE_DISABLE:
		case SST_WRITE_STATUS_ENABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SST_WRITE_STATUS:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = 1;
			break;
		case SST_ERASE_SECTOR:
		case SST_ERASE_32K_BLOCK:
		case SST_ERASE_64K_BLOCK:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SST_ERASE_CHIP:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SST_READ_STATUS:
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
		case SST_BYTE_PROGRAM:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = 1;
			spi_cmd.write_en = spi_write;
			spi_cmd.read_status_en = read_status_disable;
			spi_cmd.dtr_mode = dtr_disable;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SST_AUTO_ADDR_INC_PROGRAM:
			if (g_AAI_state == 0) {
				spi_cmd.spi_addr =
				    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
				spi_cmd.addr_len = addr_3byte;
				spi_cmd.ins_len = instr_1byte;
				spi_cmd.data_cnt = 2;
				spi_cmd.write_en = spi_write;
				spi_cmd.read_status_en = read_status_disable;
				spi_cmd.dtr_mode = dtr_disable;
				spi_cmd.spi_mode = spi_operate_serial_mode;
				g_AAI_state = 1;
			} else {
				spi_cmd.ins_len = instr_1byte;
				spi_cmd.data_cnt = 2;
				spi_cmd.write_en = spi_write;
				spi_cmd.read_status_en = read_status_disable;
				spi_cmd.dtr_mode = dtr_disable;
				spi_cmd.spi_mode = spi_operate_serial_mode;
			}
			break;
		case SST_READ_DATA:
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
		case SST_READ_DATA_HIGH_SPEED:
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
		default:
			break;
		}
		FTSPI020_issue_cmd(&spi_cmd);
	} else if (flags & SPI_XFER_DATA_STATE) {
		FTSPI020_data_access((uint8_t *) dout, (uint8_t *) din, len);

	}
	// For the series of write cmd(The cmd_complete will be set when the data is written)
	if (flags & SPI_XFER_CHECK_CMD_COMPLETE) {
		if (FTSPI020_wait_cmd_complete(sst25_wait_ms))
			return 1;
	}

	return 0;
}

static int dataflash_read_fast_25vf(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	int ret;
	uint8_t rd_cmd[4];

	if (offset + len > flash->size) {
		prints("%s: %s: Read %zu bytes @ 0x%x out of range\n",  flash->name, 
			    (g_trans_mode == PIO) ? "PIO":"DMA", len, offset);
		return 1;
	}

	sst_check_status_til_ready(flash, 100);
	switch (type) {
	case SST25_READ_DATA:
		rd_cmd[0] = SST_READ_DATA;
		break;
	case SST25_READ_DATA_HIGH_SPEED:
		rd_cmd[0] = SST_READ_DATA_HIGH_SPEED;
		break;
	default:
		prints("%s: %s: Unknown read type %zu bytes @ %d\n", flash->name, 
			    (g_trans_mode == PIO) ? "PIO":"DMA",len, offset);
		return 1;
	}

	rd_cmd[1] = offset & 0xFF;
	rd_cmd[2] = ((offset & 0xFF00) >> 8);
	rd_cmd[3] = ((offset & 0xFF0000) >> 16);
	ret = spi_flash_cmd_read(flash, rd_cmd, buf, len);
	//FTSPI020_show_content(buf, len);
	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    sst25_rd_string[type], len, offset);
	return ret;
}

static int dataflash_write_fast_25vf(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len, void *buf)
{
	uint32_t tx_offset = offset;
	uint32_t tx_len = 0;
	uint8_t *buff = (uint8_t *) buf;
	uint8_t wr_en_cmd[1], wr_cmd[4];

	if (offset + len > flash->size) {
		prints("%s: %s: Write %zu bytes @ 0x%x out of range\n",  flash->name, 
			    (g_trans_mode == PIO) ? "PIO":"DMA", len, offset);
		return 1;
	}

	if (sst_check_write_protect(flash))
		return 1;

	switch (type) {
	case SST25_BYTE_PROGRAM:
		do {
			sst_check_status_til_ready(flash, 10);

			wr_en_cmd[0] = SST_WRITE_ENABLE;
			if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
				return 1;

			wr_cmd[0] = SST_BYTE_PROGRAM;
			wr_cmd[1] = tx_offset & 0xFF;
			wr_cmd[2] = ((tx_offset & 0xFF00) >> 8);
			wr_cmd[3] = ((tx_offset & 0xFF0000) >> 16);

			if (spi_flash_cmd_write(flash, wr_cmd, buff, 1))
				return 1;
			buff++;
			tx_offset++;
			tx_len++;
		} while (tx_len < len);
		break;
	case SST25_AUTO_ADDR_INC_PROGRAM:
		sst_check_status_til_ready(flash, 10);

		wr_en_cmd[0] = SST_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_en_cmd, NULL, 0))
			return 1;

		// Reset the AAI programming state.
		g_AAI_state = 0;
		wr_cmd[0] = SST_AUTO_ADDR_INC_PROGRAM;
		wr_cmd[1] = tx_offset & 0xFF;
		wr_cmd[2] = ((tx_offset & 0xFF00) >> 8);
		wr_cmd[3] = ((tx_offset & 0xFF0000) >> 16);
		if (spi_flash_cmd_write(flash, wr_cmd, buff, 2))
			return 1;

		buff += 2;
		tx_len += 2;

		do {
			sst_check_status_til_ready(flash, 10);
			wr_cmd[0] = SST_AUTO_ADDR_INC_PROGRAM;
			if (spi_flash_cmd_write(flash, wr_cmd, buff, 2))
				return 1;

			buff += 2;
			tx_len += 2;
		} while (tx_len < len);

		sst_check_status_til_ready(flash, 10);

		wr_cmd[0] = SST_WRITE_DISABLE;
		if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
			return 1;

		break;
	default:
		prints("%s: %s: Unknown write type %zu bytes @ %d\n", flash->name, 
			    (g_trans_mode == PIO) ? "PIO":"DMA", len, offset);
		return 1;
	}

	prints("%s: %s: %s %zu bytes @ %d\n", flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", 
		    sst25_wr_string[type], len, offset);
	return 0;
}

static int dataflash_erase_fast_25vf(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, erase_size;
	uint8_t er_cmd[4], cmd_code;
	uint32_t wait_t;

	if (sst_check_write_protect(flash))
		return 1;

	if (type ==  SST25_SECTOR_ERASE) {
		cmd_code =  SST_ERASE_SECTOR;
		erase_size = flash->erase_sector_size;
		wait_t = 100;
	} else if (type == SST25_BLOCK_32K_ERASE) {
		cmd_code = SST_ERASE_32K_BLOCK;
		erase_size = 32 << 10;
		wait_t = 200;
	} else {
		cmd_code = SST_ERASE_64K_BLOCK;
		erase_size = 64 << 10;
		wait_t = 800;
	}

	offset = offset + erase_size + ~(erase_size - 1);

	if (sst_check_status_til_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		er_cmd[0] = SST_WRITE_ENABLE;
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

		sst_check_status_til_ready(flash, wait_t);

		er_cmd[0] = SST_WRITE_DISABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		prints("%s: %s @ 0x%x\n", flash->name, sst25_er_string[type], addr);

	}

	return ret;
}

static int dataflash_report_status_25vf(struct spi_flash *flash)
{
	int ret;
	uint8_t rd_sts_cmd[1];

	sst_check_status_til_ready(flash, 10);

	rd_sts_cmd[0] = SST_READ_STATUS;
	ret = spi_flash_cmd(flash, rd_sts_cmd, NULL, 0);
	FTSPI020_show_status();

	prints("%s: Successfully read status\n", flash->name);
	return ret;
}

static int dataflash_erase_all_25vf(struct spi_flash *flash)
{
	uint8_t er_all_cmd[1];

	if (sst_check_write_protect(flash))
		return 1;

	sst_check_status_til_ready(flash, 10);

	er_all_cmd[0] = SST_WRITE_ENABLE;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	er_all_cmd[0] = SST_ERASE_CHIP;
	if (spi_flash_cmd(flash, er_all_cmd, NULL, 0))
		return 1;

	prints("%s: Successfully erase the whole chip\n", flash->name);

	prints("%s: Wait for busy bit cleared\n", flash->name);
	sst_check_status_til_ready(flash, 50);

	return 0;
}

static int sst_check_status_til_ready(struct spi_flash * flash, uint32_t wait_ms)
{
	uint8_t rd_sts_cmd[1];
	uint8_t status;

	sst25_wait_ms = wait_ms;

	rd_sts_cmd[0] = SST_READ_STATUS;
	do {
		if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
			prints("%s: Failed to check status by SW\n", flash->name);
			return 1;
		}

		FTSPI020_read_status(&status);
	} while ((g_check_status == check_status_by_sw) && (status & MXIC_STS_BUSY_BIT));

	return 0;
}

static int sst_check_write_protect(struct spi_flash * flash)
{
	uint8_t rd_sts_cmd[1];
	uint8_t wr_sts_cmd[1];
	uint8_t status, tmp;

	sst25_wait_ms = 50;
	tmp = g_check_status;
	g_check_status = check_status_by_sw;

	sst_check_status_til_ready(flash, 10);

	rd_sts_cmd[0] = SST_READ_STATUS;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status);

#if 0
	FTSPI020_show_status();
#endif

	if (status & SST_STS_PROTECT_BITS) {
		wr_sts_cmd[0] = SST_WRITE_STATUS_ENABLE;
		if (spi_flash_cmd(flash, wr_sts_cmd, NULL, 0)) {
			g_check_status = tmp;
			return 1;
		}

		// Set the BP3~BP0 to be readable if it's read-only
		if (status & SST_STS_PROTECT_MODE_BIT) {
			status = status & ~(SST_STS_PROTECT_MODE_BIT);
		}
		// Clear the protect bit
		status = status & ~(SST_STS_PROTECT_BITS);

		wr_sts_cmd[0] = SST_WRITE_STATUS;
		if (spi_flash_cmd_write(flash, wr_sts_cmd, &status, 1)){
			g_check_status = tmp;
			return 1;
		}
	}

	rd_sts_cmd[0] = SST_READ_STATUS;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		g_check_status = tmp;
		return 1;
	}
	FTSPI020_read_status(&status);
	if (status & SST_STS_PROTECT_BITS) {
		prints("%s: Disable Protect bits failed\n", flash->name);
		g_check_status = tmp;
		return 1;
	}
	
	g_check_status = tmp;
#if 0
	FTSPI020_show_status();
#endif
	return 0;
}

struct spi_flash *spi_flash_probe_sst(uint8_t * idcode)
{
	uint8_t i;
	uint16_t u16_idcode;
	const struct sst_spi_flash_params *params;
	struct spi_flash *ssf;

	memcpy(&u16_idcode, (idcode + 1), 2);

	for (i = 0; i < 1; i++) {
		params = &sst_spi_flash_table[i];

		if (params->idcode1_2 == u16_idcode)
			break;
	}

	if (i == 1) {
		prints("SST: Unsupported DataFlash ID %04x\n", u16_idcode);
		return NULL;
	}

	ssf = malloc(sizeof(struct spi_flash));
	if (!ssf) {
		prints("SST: Failed to allocate memory\n");
		return NULL;
	}

	ssf->name = params->name;
	ssf->code = params->idcode1_2;
	ssf->page_size = params->page_size;
	ssf->nr_pages = params->nr_pages;
	ssf->size = params->page_size * params->nr_pages;
	ssf->erase_sector_size = params->sector_size;

	ssf->max_rd_type = SST25_MAX_READ_TYPE;
	ssf->max_wr_type = SST25_MAX_WRITE_TYPE;
	ssf->max_er_type = SST25_MAX_ERASE_TYPE;

	switch (u16_idcode) {
	case 0x8E25:
		prints("Find Flash Name: %s\n", ssf->name);
		FTSPI020_busy_location(0);
		ssf->spi_xfer = spi_xfer_25vf;
		ssf->read = dataflash_read_fast_25vf;
		ssf->write = dataflash_write_fast_25vf;
		ssf->erase = dataflash_erase_fast_25vf;
		ssf->erase_all = dataflash_erase_all_25vf;
		ssf->report_status = dataflash_report_status_25vf;
		ssf->get_string = sst25_action_get_string;
		return ssf;
		break;
	default:
		prints("SST: Unsupported DataFlash family %u\n", u16_idcode);
		goto err;
		break;
	}

      err:
	free(ssf);
	return NULL;

}

#endif
