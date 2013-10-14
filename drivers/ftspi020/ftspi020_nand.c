/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_nand.c
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE         AUTHOR      DESCRIPTION
 * 2013/08/30   BingYao     SPI nand flash driver
 * -------------------------------------------------------------------------
 */
#include <common.h>
#include <malloc.h>

#include "ftspi020.h"

#if defined(FTSPI020_NAND_H)

#include "ftspi020_cntr.h"

static int spinand_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms);

struct spinand_flash_params {
	uint16_t idcode1_2;
	uint16_t page_size;
	uint32_t nr_pages;
	uint32_t block_size;
	const char *name;
};

/**
 * 1 page = (2K + 64) bytes
 *               1 block = (2K + 64) bytes x 64 pages
 *			 = (128K + 4K) bytes
 * A5U12A21ASC : 1 device = 512 blocks x 64 pages
 *			  = 32768 pages
 * GD5F1GQ4U : 1 device = 1024 blocks x 64 pages
 *			  = 65536 pages
 *
 */
static const struct spinand_flash_params nand_spi_flash_table[] = {
	{0x7F20, 2048, 32768, (2048 * 64), "NANDA5U12A"},
	{0x00F1, 2048, 65536, (2048 * 64), "GD5F1GQ4U"},
};

/* bit position is block index, 8 x 64 = 512 bits(blocks) */
static uint8_t bad_block_table[64];

char *spinand_er_string[NAND_MAX_ERASE_TYPE + 1] = {
	"Block Erase",
	"Invalid erase type"
};

char *spinand_wr_string[NAND_MAX_WRITE_TYPE + 1] = {
	"Page Program",
	"Page Program Quad",
	"Invalid write type"
};

char *spinand_rd_string[NAND_MAX_READ_TYPE + 1] = {
	"Read",
	"Read Dual",
	"Read Quad",
	"Read Dual IO",
	"Read Quad IO",
	"Invalid read type"
};

static int spinand_set_ecc_en(struct spi_flash * flash, uint8_t en);
static int spinand_set_block_protect(struct spi_flash * flash, uint8_t bp);
static int spinand_get_feature(struct spi_flash * flash, int fea_addr, uint8_t *status);
static char *spinand_action_get_string(uint32_t act, uint32_t type);

uint32_t spinand_wait_ms;

static char *spinand_action_get_string(uint32_t act, uint32_t type)
{
	if (act == WRITE)
		return spinand_wr_string[type];
	else if (act == READ)
		return spinand_rd_string[type];
	else
		return spinand_er_string[type];
}

static int spinand_get_feature(struct spi_flash * flash, int fea_addr, uint8_t *status)
{
	uint8_t rd_sts_cmd[2];

	rd_sts_cmd[0] = SPINAND_GET_FEATURE;
	rd_sts_cmd[1] = fea_addr;
	if (spi_flash_cmd(flash, rd_sts_cmd, NULL, 0)) {
		return 1;
	}
	FTSPI020_read_status(status);

	return 0;
}

/**
 * Default value BP is 3'b111 means all locked
 */
static int spinand_set_ecc_en(struct spi_flash * flash, uint8_t en)
{
	uint8_t cmd[2];
	uint8_t otp;

	if (spinand_get_feature(flash, SPINAND_FEA_OTP, &otp)) {
		prints("%s: 1-Failed to read OTP register\n", flash->name);
		return 1;
	}
	if (debug > 2)
		prints("OTP register 0x%x \n", otp);

	if ((otp & SPINAND_OTP_ECC_EN) == (en << 4)) {
		prints("ECC enable value the same(%d) \n", en);
		return 0;
	}

	if (en)
		otp |= SPINAND_OTP_ECC_EN;
	else
		otp &= ~SPINAND_OTP_ECC_EN;

	prints("%s: Set OTP register to 0x%x: ", flash->name, otp);

	cmd[0] = SPINAND_SET_FEATURE;
	cmd[1] = SPINAND_FEA_OTP;
	if (spi_flash_cmd_write(flash, cmd, (const void *) &otp, 1)) {
		prints("failed.\n");
		return 1;
	}

	/* Read it back to confirm the value */
	otp = en ? 0 : 0xf;
	if (spinand_get_feature(flash, SPINAND_FEA_OTP, &otp)) {
		prints("2-Failed to read OTP register\n");
		return 1;
	}

	if (en && !(otp & SPINAND_OTP_ECC_EN)) {
		prints("Enable ECC failed.\n");
		return 1;
	}

	if (!en && (otp & SPINAND_OTP_ECC_EN)) {
		prints("Disable ECC failed.\n");
		return 1;
	}

	prints("success.\n");

	return 0;

}
/**
 * Default value BP is 3'b111 means all locked
 */
static int spinand_set_block_protect(struct spi_flash * flash, uint8_t bp)
{
	uint8_t cmd[2];
	uint8_t bl_lock;

	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &bl_lock)) {
		prints("%s: 1-Failed to read Block lock register\n", flash->name);
		return 1;
	}
	if (debug > 2)
		prints("Block Protect register 0x%x \n", bl_lock);

	bp &= 0x7;
	bl_lock &= ~(SPINAND_BL_BP0TO2 | SPINAND_BL_BRWD);
	bl_lock |= (bp << 3);
	prints("%s: Set Block Protect register 0x%x: ", flash->name, bl_lock);

	cmd[0] = SPINAND_SET_FEATURE;
	cmd[1] = SPINAND_FEA_PROTECTION;
	if (spi_flash_cmd_write(flash, cmd, (const void *) &bl_lock, 1)) {
		prints("failed.\n");
		return 1;
	}

	/* Read it back to confirm the value */
	bl_lock = 0xf;
	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &bl_lock)) {
		prints("failed to read Block lock register\n");
		return 1;
	}

	if (((bl_lock >> 3) & 0x7) == bp)
		prints(" success\n");
	else
		prints("read back value wrong.(0x%x)\n", bl_lock);

	return 0;

}

static int spinand_check_status_till_ready(struct spi_flash *flash, uint32_t wait_ms)
{
	uint8_t rd_sts_cmd[1];
	uint8_t status;

	spinand_wait_ms = wait_ms;
	do {
		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &status)) {
			prints("%s: Failed to check status by SW\n", flash->name);
			return 1;
		}
	} while (status & SPINAND_STS_BUSY);

	return 0;
}

/**
 * return 1 if offset is located at bad block
 */
static int dataflash_is_bad_block_spinand(struct spi_flash * flash, uint32_t offset)
{
	int row_addr, bad, blk_idx, blk_shift;

	row_addr = offset / flash->page_size;
	blk_idx = row_addr / 8;
	blk_shift = row_addr % 8;

	/* for ease debugging */
	bad = bad_block_table[blk_idx] & (1 << blk_shift);

	return bad;
}

/**
 * Identifying Initial invalid Blocks by check the
 * data at the column address 2048 of page 0 and page 1
 * from block 0 to the last block.
 *
 * Do not erase or program factory-marked bad blocks.
 */
static int dataflash_scan_bad_blocks_spinand(struct spi_flash * flash)
{
	char buf;
	int offset, row_addr, i = 0;

	for (offset = 0; offset < flash->size; offset += flash->erase_sector_size) {
		int blk_idx, blk_shift;

		row_addr = offset / flash->page_size;
		prints("%s: Scan bbt block %d off %d row %d ...\n", flash->name, ++i, offset, row_addr);
		buf = 0;
		if (flash->read_spare(flash, row_addr, 2048, 1, &buf))
			prints("%s: Scan bad blocks, read spare failed at addr 0x%x.\n",
				flash->name, row_addr);

		if (buf != 0xff)
			goto mark_bad_block;

		buf = 0;
		if (flash->read_spare(flash, (row_addr + 1), 2048, 1, &buf))
			prints("%s: Scan bad blocks, read spare failed at addr 0x%x.\n",
				flash->name, (row_addr + 1));

		if (buf == 0xff)
			continue;

mark_bad_block:
		blk_idx = row_addr / 8;
		blk_shift = row_addr % 8;
		bad_block_table[blk_idx] |= (1 << blk_shift);
		prints("%s: Bad block %d.\n", flash->name, blk_idx);
	}

	return 0;
}

static int spi_xfer_spinand(struct spi_flash * flash, unsigned int len, const void *dout,
			    void *din, unsigned long flags)
{

	struct ftspi020_cmd spi_cmd;
	uint8_t *data_out = (uint8_t *) dout;

	memset(&spi_cmd, 0, sizeof(struct ftspi020_cmd));

	if (flags & SPI_XFER_CMD_STATE) {
		spi_cmd.start_ce = flash->ce;
		spi_cmd.ins_code = *data_out;

		switch (spi_cmd.ins_code) {
		case CMD_RESET:	/* Reset the Flash */
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_WRITE_ENABLE:
		case SPINAND_WRITE_DISABLE:
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_ERASE_BLOCK:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_SET_FEATURE:
			spi_cmd.spi_addr = *(data_out + 1);
			spi_cmd.addr_len = addr_1byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			spi_cmd.data_cnt = len;
			break;
		case SPINAND_GET_FEATURE:
			spi_cmd.spi_addr = *(data_out + 1);
			spi_cmd.addr_len = addr_1byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_read;
			spi_cmd.read_status_en = read_status_enable;
			spi_cmd.read_status = read_status_by_sw;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_RANDOM_WRITE_PAGE:
		case SPINAND_QUAD_RANDOM_WRITE_PAGE:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) | *(data_out + 1);
			spi_cmd.addr_len = addr_2byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			if (spi_cmd.ins_code == SPINAND_RANDOM_WRITE_PAGE)
				spi_cmd.spi_mode = spi_operate_serial_mode;
			else
				spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case SPINAND_PROGRAM_EXECUTE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write;
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_PAGE_LOAD:
		case SPINAND_PAGE_LOAD_QUAD:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) | *(data_out + 1);
			spi_cmd.addr_len = addr_2byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_write;
			if (spi_cmd.ins_code == SPINAND_PAGE_LOAD)
				spi_cmd.spi_mode = spi_operate_serial_mode;
			else
				spi_cmd.spi_mode = spi_operate_quad_mode;
			break;
		case SPINAND_READ_TO_CACHE:
			spi_cmd.spi_addr =
			    (*(data_out + 3) << 16) | (*(data_out + 2) << 8) | (*(data_out + 1));;
			spi_cmd.addr_len = addr_3byte;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.write_en = spi_write; /* no return data */
			spi_cmd.spi_mode = spi_operate_serial_mode;
			break;
		case SPINAND_READ_FROM_CACHE:
		case SPINAND_READ_FROM_CACHE_DUAL:
		case SPINAND_READ_FROM_CACHE_QUAD:
			spi_cmd.spi_addr = (*(data_out + 2) << 8) | *(data_out + 1);
			spi_cmd.addr_len = addr_2byte;
			spi_cmd.dum_2nd_cyc = 8;
			spi_cmd.ins_len = instr_1byte;
			spi_cmd.data_cnt = len;
			spi_cmd.write_en = spi_read;
			if (spi_cmd.ins_code == SPINAND_READ_FROM_CACHE)
				spi_cmd.spi_mode = spi_operate_serial_mode;
			else if (spi_cmd.ins_code == SPINAND_READ_FROM_CACHE_DUAL)
				spi_cmd.spi_mode = spi_operate_dual_mode;
			else if (spi_cmd.ins_code == SPINAND_READ_FROM_CACHE_QUAD)
				spi_cmd.spi_mode = spi_operate_quad_mode;
			else if (spi_cmd.ins_code == SPINAND_READ_FROM_CACHE_DUAL_IO)
				spi_cmd.spi_mode = spi_operate_dualio_mode;
			else
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
		if (FTSPI020_wait_cmd_complete(spinand_wait_ms))
			return 1;
	}

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache) -> row address
 *  2) 0Fh (GET FEATURE command to read the status)
 *  3) 0Bh or 03h (READ FROM CACHE x1) / 3Bh (x2) / 6Bh (x4) -> column address
 */
static int dataflash_read_spare_spinand(struct spi_flash *flash, uint32_t row_addr,
					uint32_t col_addr, size_t total_len, char *buf)
{
	uint8_t rd_cmd[4];

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	if (col_addr < 2048) {
		prints("%s: Spare area start at byte 2048.\n", flash->name);
		col_addr = 2048;
	}

	if (total_len > 64) {
		prints("%s: Spare area has only 64 bytes.\n", flash->name);
		total_len = 64;
	}

	if ((col_addr + total_len) > 2112) {
		prints("%s: Spare area read start plus length exceeds 2010 bytes.\n", flash->name);
		total_len = 2112 - col_addr;
	}

	rd_cmd[0] = SPINAND_READ_TO_CACHE;
	rd_cmd[1] = row_addr & 0xFF;
	rd_cmd[2] = ((row_addr & 0xFF00) >> 8);
	rd_cmd[3] = ((row_addr & 0xFF0000) >> 16);
	if (spi_flash_cmd(flash, rd_cmd, NULL, 0)) {
		prints("%s: %s: READ(SPARE) TO CACHE @ row %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (spinand_check_status_till_ready(flash, 1)) {
		prints("%s: %s: Wait READ(SPARE) TO CACHE ready @ row %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	rd_cmd[0] = SPINAND_READ_FROM_CACHE;
	rd_cmd[1] = col_addr & 0xFF;
	rd_cmd[2] = ((col_addr & 0xFF00) >> 8);
	if (spi_flash_cmd_read(flash, rd_cmd, buf, total_len)) {
		prints("%s: %s: READ(SPARE) FROM CACHE %d bytes @ col %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", total_len, col_addr);
		return 1;
	}

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 06h (WRITE ENABLE)
 *  2) 02h (PROGRAM LOAD x1) / 32h (x4)
 *  3) 10h (PROGRAM EXECUTE)
 *  4) 0Fh (GET FEATURE command to read the status)
 */
static int dataflash_write_spare_spinand(struct spi_flash *flash, uint32_t row_addr,
					 uint32_t col_addr, size_t total_len, void *buf)
{
	uint8_t sts;
	uint8_t wr_cmd[4];

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	if (col_addr < 2048) {
		prints("%s: Spare area start at byte 2048.\n", flash->name);
		col_addr = 2048;
	}

	if (total_len > 64) {
		prints("%s: Spare area has only 64 bytes.\n", flash->name);
		total_len = 64;
	}

	if ((col_addr + total_len) > 2112) {
		prints("%s: Spare area read start plus length exceeds 2010 bytes.\n", flash->name);
		total_len = 2112 - col_addr;
	}

	wr_cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
		return 1;

	wr_cmd[0] = SPINAND_PAGE_LOAD;
	wr_cmd[1] = col_addr & 0xFF;
	wr_cmd[2] = ((col_addr & 0xFF00) >> 8);
	if (spi_flash_cmd_write(flash, wr_cmd, buf, total_len)) {
		prints("%s: %s: PAGE LOAD(SPARE) %d bytes @ col %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", total_len, col_addr);
		return 1;
	}

	wr_cmd[0] = SPINAND_PROGRAM_EXECUTE;
	wr_cmd[1] = row_addr & 0xFF;
	wr_cmd[2] = ((row_addr & 0xFF00) >> 8);
	wr_cmd[3] = ((row_addr & 0xFF0000) >> 16);
	if (spi_flash_cmd(flash, wr_cmd, NULL, 0)) {
		prints("%s: %s: PROGRAM EXECUTE(SPARE) @ row %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (spinand_check_status_till_ready(flash, 1)) {
		prints("%s: %s: Wait PROGRAM EXECUTE(SPARE) ready @ row %d failed\n", flash->name,
			(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
		return 1;
	}

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
		prints("%s: Failed to check page %d program(spare) status.\n",
			flash->name, row_addr);
		return 1;
	}

	if (sts & SPINAND_STS_PROGRAM_FAIL)
		prints("%s: Page %d check status program(spare) failed.\n",
			flash->name, row_addr);

	return 0;
}
/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache) -> row address
 *  2) 0Fh (GET FEATURE command to read the status)
 *  3) 0Bh or 03h (READ FROM CACHE x1) / 3Bh (x2) / 6Bh (x4) -> column address
 */
static int dataflash_read_spinand(struct spi_flash *flash, uint8_t type, uint32_t offset,
				  size_t total_len, void *buf)
{
	uint8_t cmd_code, sts;
	uint8_t rd_cmd[4];
	uint32_t row_addr, col_addr;
	size_t len;

	if (type >= flash->max_rd_type) {
		prints("%s: Unknown read %d type %d bytes @ %d\n",
			flash->name, type, total_len, offset);
		return 1;
	}

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	switch (type) {
	case NAND_READ:
		cmd_code = SPINAND_READ_FROM_CACHE;
		break;
	case NAND_READ_DUAL:
		cmd_code = SPINAND_READ_FROM_CACHE_DUAL;
		break;
	case NAND_READ_QUAD:
		cmd_code = SPINAND_READ_FROM_CACHE_QUAD;
		break;
	case NAND_READ_DUAL_IO:
		cmd_code = SPINAND_READ_FROM_CACHE_DUAL_IO;
		break;
	case NAND_READ_QUAD_IO:
		cmd_code = SPINAND_READ_FROM_CACHE_QUAD_IO;
		break;
	default:
		prints("%s: Unknown read %d type %d bytes @ %d\n", flash->name,
			type, total_len, offset);
		return 1;
	}

	prints("%s: %s: %s %d bytes @ %d\n", flash->name,
		(g_trans_mode == PIO) ? "PIO":"DMA", spinand_rd_string[type],
		total_len, offset);

	do {
		row_addr = offset / flash->page_size;
		col_addr = offset % flash->page_size;
		len = total_len;

		/* exceed one page size */
		if (col_addr + len > flash->page_size)
			len = flash->page_size - col_addr;

		rd_cmd[0] = SPINAND_READ_TO_CACHE;
		rd_cmd[1] = row_addr & 0xFF;
		rd_cmd[2] = ((row_addr & 0xFF00) >> 8);
		rd_cmd[3] = ((row_addr & 0xFF0000) >> 16);
		if (spi_flash_cmd(flash, rd_cmd, NULL, 0)) {
			prints("%s: %s: READ TO CACHE @ row %d failed\n", flash->name,
				(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
			return 1;
		}

		if (spinand_check_status_till_ready(flash, 1)) {
			prints("%s: %s: Wait READ TO CACHE ready @ row %d failed\n", flash->name,
				(g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
			return 1;
		}

		rd_cmd[0] = cmd_code;
		rd_cmd[1] = col_addr & 0xFF;
		rd_cmd[2] = ((col_addr & 0xFF00) >> 8);
		if (spi_flash_cmd_read(flash, rd_cmd, buf, len)) {
			prints("%s: %s: %s %d bytes @ col %d failed\n", flash->name,
				(g_trans_mode == PIO) ? "PIO":"DMA", spinand_rd_string[type],
				len, col_addr);
			return 1;
		}

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check read row %d col %d ECC.\n",
				flash->name, row_addr, col_addr);
			return 1;
		}

		if ((sts & SPINAND_STS_ECC_STS0_1) == 0x20)
			prints("%s: Read row %d col %d len %d ECC failed.\n",
				flash->name, row_addr, col_addr, len);

		total_len -= len;
		buf += len;
		offset += len;

	} while (total_len);

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 06h (WRITE ENABLE)
 *  2) 02h (PROGRAM LOAD x1) / 32h (x4)
 *  3) 10h (PROGRAM EXECUTE)
 *  4) 0Fh (GET FEATURE command to read the status)
 */
static int dataflash_write_spinand(struct spi_flash *flash, uint8_t type, uint32_t offset,
				   size_t total_len, void *buf)
{
	uint8_t cmd_code, sts;
	uint8_t wr_cmd[4];
	uint32_t row_addr, col_addr, blk_idx, blk_shift;
	size_t len;

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	switch (type) {
	case NAND_PAGE_PROGRAM:
		cmd_code = SPINAND_PAGE_LOAD;
		break;
	case NAND_QUAD_PROGRAM:
		cmd_code = SPINAND_PAGE_LOAD_QUAD;
		break;
	default:
		prints("%s: Unknown write %d type %d bytes @ %d\n", flash->name, type, total_len, offset);
		return 1;
	}

	prints("%s: %s: %s %d bytes @ %d\n", flash->name,
		(g_trans_mode == PIO) ? "PIO":"DMA", spinand_wr_string[type],
		total_len, offset);

	do {
		row_addr = offset / flash->page_size;
		col_addr = offset % flash->page_size;
		len = total_len;

		/* exceed one page size */
		if (col_addr + len > flash->page_size)
			len = flash->page_size - col_addr;

		/* Do not program factory-marked bad blocks. */
		blk_idx = row_addr / 8;
		blk_shift = row_addr % 8;
		if (bad_block_table[blk_idx] & (1 << blk_shift)) {
			prints("%s: %s: Skip bad block %d @ page 0x%x (offset 0x%x).\n",
				flash->name, spinand_wr_string[type], blk_idx, row_addr, offset);
			return 1;
		}

		wr_cmd[0] = SPINAND_WRITE_ENABLE;
		if (spi_flash_cmd(flash, wr_cmd, NULL, 0))
			return 1;

		wr_cmd[0] = cmd_code;
		wr_cmd[1] = col_addr & 0xFF;
		wr_cmd[2] = ((col_addr & 0xFF00) >> 8);
		if (spi_flash_cmd_write(flash, wr_cmd, buf, len)) {
			prints("%s: %s: %s %d bytes @ col %d failed\n",
				flash->name, (g_trans_mode == PIO) ? "PIO":"DMA",
				spinand_wr_string[type], len, col_addr);
			return 1;
		}

		wr_cmd[0] = SPINAND_PROGRAM_EXECUTE;
		wr_cmd[1] = row_addr & 0xFF;
		wr_cmd[2] = ((row_addr & 0xFF00) >> 8);
		wr_cmd[3] = ((row_addr & 0xFF0000) >> 16);
		if (spi_flash_cmd(flash, wr_cmd, NULL, 0)) {
			prints("%s: %s: PROGRAM EXECUTE @ row %d failed\n",
				flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
			return 1;
		}

		if (spinand_check_status_till_ready(flash, 1)) {
			prints("%s: %s: Wait PROGRAM EXECUTE ready @ row %d failed\n",
				flash->name, (g_trans_mode == PIO) ? "PIO":"DMA", row_addr);
			return 1;
		}

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check page %d program status.\n",
				flash->name, row_addr);
			return 1;
		}

		if (sts & SPINAND_STS_PROGRAM_FAIL)
			prints("%s: %s Page %d check status program failed.\n", flash->name,
				spinand_wr_string[type], row_addr);


		total_len -= len;
		buf += len;
		offset += len;

	} while (total_len);

	return 0;
}

static int dataflash_erase_spinand(struct spi_flash *flash, uint8_t type, uint32_t offset, size_t len)
{
	int ret, addr, row_addr, erase_size;
	uint8_t er_cmd[4];
	uint8_t cmd_code, sts;
	uint32_t wait_t, blk_idx, blk_shift;

	if (type == NAND_BLOCK_ERASE) {
		cmd_code = SPINAND_ERASE_BLOCK;
		erase_size = flash->erase_sector_size;
		wait_t = 10; /* Max Block erase time 10 ms */
	} else {
		prints("%s: %s @ 0x%x\n", flash->name, spinand_er_string[type], offset);
		return 1;
	}

	offset &=  ~(erase_size - 1);

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	for (addr = offset; addr < (offset + len); addr += erase_size) {
		if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &sts)) {
			prints("%s: Failed to read block protection status.\n",
				flash->name);
			return 1;
		}

		if (sts & SPINAND_BL_BP0TO2) {
			if (spinand_set_block_protect(flash, 0))
				return 1;
		}

		er_cmd[0] = SPINAND_WRITE_ENABLE;
		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		row_addr = addr / flash->page_size;

		/* Do not erase factory-marked bad blocks. */
		blk_idx = row_addr / 8;
		blk_shift = row_addr % 8;
		if (bad_block_table[blk_idx] & (1 << blk_shift)) {
			prints("%s: %s: Skip bad block %d @ page 0x%x (offset 0x%x).\n",
				flash->name, spinand_er_string[type], (row_addr >> 6),
				row_addr, addr);
			continue;
		}

		er_cmd[0] = cmd_code;
		er_cmd[1] = row_addr & 0xFF;
		er_cmd[2] = ((row_addr & 0xFF00) >> 8);
		er_cmd[3] = ((row_addr & 0xFF0000) >> 16);

		ret = spi_flash_cmd(flash, er_cmd, NULL, 0);
		if (ret)
			break;

		if (spinand_check_status_till_ready(flash, wait_t))
			return 1;

		if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
			prints("%s: Failed to check erase block %d status.\n",
				flash->name, (row_addr >> 6));
			return 1;
		}

		if (sts & SPINAND_STS_ERASE_FAIL)
			prints("%s: %s: Erase block %d check status 0x%x failed.\n", flash->name,
				spinand_er_string[type], (row_addr >> 6), sts);

		prints("%s: %s @ 0x%x\n", flash->name, spinand_er_string[type], row_addr);

	}

	return ret;
}

static int dataflash_erase_all_spinand(struct spi_flash *flash)
{
	if (dataflash_erase_spinand(flash, NAND_BLOCK_ERASE, 0, flash->size))
		return 1;

	return 0;
}

static int dataflash_report_status_spinand(struct spi_flash *flash)
{
	int ret;
	uint8_t status;

	if (spinand_check_status_till_ready(flash, 100))
		return 1;

	if (spinand_get_feature(flash, SPINAND_FEA_PROTECTION, &status)) {
		prints("%s: Failed to read Block Lock Register.\n", flash->name);
	}
	prints("%s: Block Lock Register: 0x%x.\n", flash->name, status);

	if (spinand_get_feature(flash, SPINAND_FEA_OTP, &status)) {
		prints("%s: Failed to read One Time Program Register.\n", flash->name);
	}
	prints("%s: OTP Register: 0x%x.\n", flash->name, status);

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &status)) {
		prints("%s: Failed to read Status Register.\n", flash->name);
	}
	prints("%s: Status Register: 0x%x.\n", flash->name, status);

	if (spinand_get_feature(flash, SPINAND_FEA_DRV_STRENGTH, &status)) {
		prints("%s: Failed to read Driver Strength Register.\n", flash->name);
	}
	prints("%s: Driver Strength Register: 0x%x.\n", flash->name, status);

	return 0;
}

/**
 * The command sequence is follows:
 *  1) 13h (PAGE READ to cache)
 *  2) 06h (WRITE ENABLE)
 *  3) 84h (PROGRAM LOAD RANDOM DATA x1) / 34h (x4); this is OPTIONAL in sequence.
 *  4) 10h (PROGRAM EXECUTE)
 *  5) 0Fh (GET FEATURE command to read the status)
 *
 * Move maximum one page content from one block to another block each time.
 * If user want to move more than one page, call this function more than one time.
 */
static int dataflash_intern_move_spinand(struct spi_flash *flash, uint32_t src_row,
					 uint32_t dst_row, uint32_t dst_col,
					 void * buf, size_t total_len)
{
	uint8_t cmd[4], sts;

	prints("%s: Copy data from row %d to %d \n", flash->name,
		src_row, dst_row);

	cmd[0] = SPINAND_READ_TO_CACHE;
	cmd[1] = src_row & 0xFF;
	cmd[2] = ((src_row & 0xFF00) >> 8);
	cmd[3] = ((src_row & 0xFF0000) >> 16);
	if (spi_flash_cmd(flash, cmd, NULL, 0)) {
		prints("%s: READ TO CACHE @ row %d failed\n", flash->name, src_row);
		return 1;
	}

	if (spinand_check_status_till_ready(flash, 1)) {
		prints("%s: Wait READ TO CACHE ready @ %d failed\n", flash->name, src_row);
		return 1;
	}

	cmd[0] = SPINAND_WRITE_ENABLE;
	if (spi_flash_cmd(flash, cmd, NULL, 0))
		return 1;

	if (buf && total_len) {
		prints("%s: Random data load %d bytes to col %d\n", flash->name,
			total_len, dst_col);
		cmd[0] = SPINAND_RANDOM_WRITE_PAGE;
		cmd[1] = dst_col & 0xFF;
		cmd[2] = ((dst_col & 0xFF00) >> 8);
		if (spi_flash_cmd_write(flash, cmd, buf, total_len)) {
			prints("%s: %s: Load Random Data: %d bytes @ col %d failed\n",
				flash->name, (g_trans_mode == PIO) ? "PIO":"DMA",
				total_len, dst_col);
			return 1;
		}
	}

	cmd[0] = SPINAND_PROGRAM_EXECUTE;
	cmd[1] = dst_row & 0xFF;
	cmd[2] = ((dst_row & 0xFF00) >> 8);
	cmd[3] = ((dst_row & 0xFF0000) >> 16);
	if (spi_flash_cmd(flash, cmd, NULL, 0)) {
		prints("%s: PROGRAM EXECUTE @ row %d failed\n",
			flash->name, dst_row);
		return 1;
	}

	if (spinand_check_status_till_ready(flash, 1)) {
		prints("%s: Wait PROGRAM EXECUTE ready @ %d failed\n",
			flash->name, dst_row);
		return 1;
	}

	if (spinand_get_feature(flash, SPINAND_FEA_STATUS, &sts)) {
		prints("%s: Failed to check page %d program status.\n",
			flash->name, dst_row);
		return 1;
	}

	if (sts & SPINAND_STS_PROGRAM_FAIL)
		prints("%s: Page %d program failed.\n", flash->name, dst_row);

	return 0;
}

struct spi_flash *spi_flash_probe_spinand(uint32_t ce)
{
	uint8_t i;
	uint16_t idcode;
	const struct spinand_flash_params *params;
	struct spi_flash *flsh;
	struct ftspi020_cmd spi_cmd = {0};
	uint32_t buf;

	spi_cmd.start_ce = ce;
	spi_cmd.ins_code = CMD_READ_ID;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_read;
	spi_cmd.dtr_mode = dtr_disable;
	spi_cmd.spi_mode = spi_operate_serial_mode;
	spi_cmd.data_cnt = 3;

	/* After READ ID opCode, GigaDevice sends
	 * address 00h/01h, zentel sends dummy byte.
	 */
	spi_cmd.spi_addr = 0;
	spi_cmd.addr_len = addr_1byte;

	FTSPI020_issue_cmd(&spi_cmd);
	if (FTSPI020_data_access(NULL, (uint8_t *)(&buf), 3)){
		prints("spi nand:%d: READ JEDEC ID, data access failed\n", ce);
		return NULL;
	}

	if (FTSPI020_wait_cmd_complete(10)) {
		prints("spi nand:%d: READ JEDEC ID, wait cmd complete failed\n", ce);
		return NULL;
	}

	if ((buf & 0xFF) != 0xC8) {
		prints("spi nand:%d: READ JEDEC ID, manufacture id 0x%x not support.\n",
			ce, buf);
		return NULL;
	}

	idcode = (uint16_t) ((buf >> 8) & 0xFFFF);
	for (i = 0; i < 2; i++) {
		params = &nand_spi_flash_table[i];

		if (params->idcode1_2 == idcode)
			break;
	}

	if (i == 3) {
		prints("spi nand:%d: Unsupported DataFlash ID 0x%04x\n", ce, buf);
		return NULL;
	}

	flsh = malloc(sizeof(struct spi_flash));
	if (!flsh) {
		prints("spi nand: Failed to allocate memory\n");
		return NULL;
	}

	flsh->name = params->name;
	flsh->code = params->idcode1_2;
	flsh->page_size = params->page_size;
	flsh->nr_pages = params->nr_pages ;
	flsh->size = params->page_size * params->nr_pages;
	flsh->erase_sector_size = params->block_size;

	flsh->max_wr_type = NAND_MAX_WRITE_TYPE;
	flsh->max_er_type = NAND_MAX_ERASE_TYPE;

	switch (idcode) {
	case 0x7F20: /* zentel */
	case 0x00F1: /* GigaDevice */
		if (idcode == 0x7F20)
			flsh->max_rd_type = ZNTL_MAX_RD_TYPE;
		else if (idcode == 0x00F1)
			flsh->max_rd_type = GDV_MAX_RD_TYPE;

		prints("Find Flash Name: %s.\n", flsh->name);
		FTSPI020_busy_location(BUSY_BIT0);
		flsh->spi_xfer = spi_xfer_spinand;
		flsh->read = dataflash_read_spinand;
		flsh->write = dataflash_write_spinand;
		flsh->read_spare = dataflash_read_spare_spinand;
		flsh->write_spare = dataflash_write_spare_spinand;
		flsh->copy_data = dataflash_intern_move_spinand;
		flsh->is_bad_block = dataflash_is_bad_block_spinand;
		flsh->scan_bad_blocks = dataflash_scan_bad_blocks_spinand;
		flsh->erase = dataflash_erase_spinand;
		flsh->erase_all = dataflash_erase_all_spinand;
		flsh->report_status = dataflash_report_status_spinand;
		flsh->get_string = spinand_action_get_string;

		/* Because the default setting is lock all blocks,
		 * to ease the test, just unlock all blocks.
		 */
		spinand_set_block_protect(flsh, 0);

		/* Enable ECC */
		spinand_set_ecc_en(flsh, 1);

		return flsh;
	default:
		prints("spi nand: Unsupported DataFlash family 0x%02x\n", idcode);
	}

	free(flsh);
	return NULL;

}

#endif
