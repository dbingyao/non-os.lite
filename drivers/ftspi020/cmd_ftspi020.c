/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020.c
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/06   Mike          
 * 2010/08/13   BingYao           Scan all CEs for available flash.
 * 2013/09/03   BingYao           Support SPI NAND flash.
 * -------------------------------------------------------------------------
 */

#include <common.h>

#include "ftspi020.h"
#include "ftspi020_cntr.h"

static int FTSPI020_check_erase(int ce, int offset, int size);

static int FTSPI020_write(int argc, char * const argv[]);
static int FTSPI020_read(int argc, char * const argv[]);
static int FTSPI020_write_spare(int argc, char * const argv[]);
static int FTSPI020_read_spare(int argc, char * const argv[]);
static int FTSPI020_copy(int argc, char * const argv[]);
static int FTSPI020_erase(int argc, char * const argv[]);
static int FTSPI020_erase_all(int argc, char * const argv[]);
static int FTSPI020_reset(int argc, char * const argv[]);
static int FTSPI020_report_status(int argc, char * const argv[]);
static int FTSPI020_burnin(int argc, char * const argv[]);
static int FTSPI020_report_performance(int argc, char * const argv[]);
static int FTSPI020_transfer_type(int argc, char * const argv[]);
static int FTSPI020_use_interrput(int argc, char * const argv[]);
static int FTSPI020_scan_bad_blocks(int argc, char * const argv[]);
static int FTSPI020_list_ce(int argc, char * const argv[]);

static cmd_t ftspi020_cmd_tbl[] = {
	{"wr", "<ce> <type> <offset> <DataCnt> [1:wr with cmp, 2: burn image]", FTSPI020_write},
	{"rd", "<ce> <type> <offset> <DataCnt> [show]", FTSPI020_read},
	{"wr_sp", "<ce> <row> <col> <bytecnt>", FTSPI020_write_spare},
	{"rd_sp", "<ce> <row> <col>", FTSPI020_read_spare},
	{"cp", "<ce> <src_row> <dst_row> <row_count>", FTSPI020_copy},
	{"er", "<ce> <type> <offset> <DataCnt> [er with cmp]", FTSPI020_erase},
	{"er_all", "<ce list> [er with cmp]", FTSPI020_erase_all},
	{"rst", "<ce>", FTSPI020_reset},
	{"sts", "<ce>", FTSPI020_report_status},
	{"burnin", "<ce> <offset> [1: copy]", FTSPI020_burnin},
	{"perf", "<ce> <len>", FTSPI020_report_performance},
	{"tr", "<pio|dma>", FTSPI020_transfer_type},
	{"intr", "<0|1>", FTSPI020_use_interrput},
	{"bbt", "[ce list]", FTSPI020_scan_bad_blocks},
	{"ls", "", FTSPI020_list_ce},
	{"quit", "", 0},
	{0} /* end of FTSPI020c023CmdTbl */
};

struct spi_flash *spi_flash_info[FTSPI020_MAX_CE];

// For choosing the check status is via hw or sw.
char g_check_status = check_status_by_hw;
clock_t t0_perf;

extern int g_cmd_intr_enable;

/* wr <ce> <type> <offset> <DataCnt> [1:wr with cmp, 2: burn image] */
int FTSPI020_write(int argc, char * const argv [])
{

	char *write_buf, *read_buf;
	int i, ce, rd_type, addr, len, burn;
	char ch;
	
	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	if (argc < 5) {
		prints("%d arguments for wr isn't allowed\n", argc);
		return 1;
	} 
	
	ce = strtol(argv[1], 0 , 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	if (argc == 6)
		burn = strtol(argv[5], 0 ,0);

	/* Check if bad block for SPI NAND flash */
	if (spi_flash_info[ce]->is_bad_block) {
		if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce], addr)) {
			prints ("You are about to program to bad block 0x%x.\n", addr);
			return 0;
		}
	}

	/* 1:wr with cmp, 2: burn image */
	if (burn == 1) {
		// Prepare the pattern for writing
		for (i = 0; i < len; i++)
			*(write_buf + i) = i;
	} else if (burn == 2){
		prints (" You are about to burn rom image.\n" \
			" Please put rom code to address 0x%x.\n", (uint32_t) write_buf);

		prints (" Press 'c' to continue, 'q' to abort\n");
		while (1) {
			ch = ftuart_kbhit();

			if (ch == 'c')
				break;
			else if (ch == 'q')
				return 1;
		}
	}

	if (spi_flash_info[ce]->write(spi_flash_info[ce], strtol(argv[2], 0, 0), addr, len, write_buf))
		return 0;

	if (argc == 6) {
		rd_type = burn;

		memset(read_buf, 0, len);

		if (spi_flash_info[ce]->read(spi_flash_info[ce], rd_type, addr, len, read_buf))
			return 0;

		if (!FTSPI020_compare(write_buf, read_buf, len)) {
			prints("Compare OK!!\n");
		}
	}

	return 0;
}

/* rd <ce> <type> <offset> <DataCnt> [show] */
int FTSPI020_read(int argc, char * const argv[])
{
	int ce, addr, len;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int *p;

	if (argc < 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	} 

	addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	memset(read_buf, 0, len);

	spi_flash_info[ce]->read(spi_flash_info[ce], strtol(argv[2], 0, 0), addr, len, read_buf);

	if (argc > 5) {
		int	i;

		if (addr & 0xf) {
			prints("0x%08x: ", (int)addr & ~0xf);

			for (i = 0; i < (addr & 0xf); i++) {
				prints("   ");
			}
		}

		/* Write out page data */
		for (i = 0; i < len; i++) {
			if (((addr + i) & 0xf) == 0) {
				if (i != 0) {
					prints("\n");
				}

				prints("0x%08x: ", (int) (addr + i));
			}

			prints("%02x ",  read_buf[i]);
		}


	} else {
		p = (int *) read_buf;
		prints("Data content: %x %x %x %x\n", p[0], p[1], p[2], p[3]);
	}

	return 0;
}

/* wr_sp <ce> <row> <col> <bytecnt> */
int FTSPI020_write_spare(int argc, char * const argv[])
{
	int ce, i, row_addr, col_addr, len;
	char *write_buf, *read_buf;
	int *p;

	if (argc != 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	row_addr = strtol(argv[2], 0, 0);
	col_addr = strtol(argv[3], 0, 0);
	len = strtol(argv[4], 0, 0);

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;

	for (i = 0; i < len; i++)
		*(write_buf + i) = i;

	if (spi_flash_info[ce]->write_spare)
		spi_flash_info[ce]->write_spare(spi_flash_info[ce], row_addr, col_addr, len, write_buf);

	memset(read_buf, 0, len);
	if (spi_flash_info[ce]->read_spare)
		spi_flash_info[ce]->read_spare(spi_flash_info[ce], row_addr, col_addr, len, read_buf);

	if (!FTSPI020_compare(write_buf, read_buf, len)) {
		prints("Compare OK!!\n");
	}

	return 0;
}

/* rd_sp <ce> <row> <col> */
int FTSPI020_read_spare(int argc, char * const argv[])
{
	int ce, i, row_addr, col_addr, len;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int *p;

	if (argc != 4) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	row_addr = strtol(argv[2], 0, 0);
	col_addr = strtol(argv[3], 0, 0);
	len = 2112 - col_addr;

	memset(read_buf, 0, len);

	if (spi_flash_info[ce]->read_spare)
		spi_flash_info[ce]->read_spare(spi_flash_info[ce], row_addr, col_addr, len, read_buf);

	if (col_addr & 0xf) {
		prints("%d: ", (int)col_addr & ~0xf);

		for (i = 0; i < (col_addr & 0xf); i++) {
			prints("   ");
		}
	}

	/* Write out page data */
	for (i = 0; i < len; i++) {
		if (((col_addr + i) & 0xf) == 0) {
			if (i != 0) {
				prints("\n");
			}

			prints("%d: ", (int) (col_addr + i));
		}

		prints("%02x ",  read_buf[i]);
	}

	return 0;
}

/* cp <ce> <src_row> <dst_row> <row_count> */
int FTSPI020_copy(int argc, char * const argv[])
{
	int ce, i, src_row, dst_row, row_count, page_size;
	char *write_buf, *read_buf;

	if (argc != 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	if (!spi_flash_info[ce]->copy_data) {
		prints(" CE %d has no copy data function\n", ce);
		return 1;
	}

	src_row = strtol(argv[2], 0, 0);
	dst_row = strtol(argv[3], 0, 0);
	row_count = strtol(argv[4], 0, 0);;

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;
	page_size = spi_flash_info[ce]->page_size;

	for (i=0; i < row_count; i++, src_row++, dst_row++) {
		spi_flash_info[ce]->copy_data(spi_flash_info[ce], src_row, dst_row, 0, 0, 0);

		memset(write_buf, 0, page_size);
		memset(read_buf, 0, page_size);

		spi_flash_info[ce]->read(spi_flash_info[ce], 0, (src_row * page_size), page_size, write_buf);
		spi_flash_info[ce]->read(spi_flash_info[ce], 0, (dst_row * page_size), page_size, read_buf);

		if (!FTSPI020_compare(write_buf, read_buf, page_size)) {
			prints("Compare OK!!\n");
		}
	}

	return 0;
}

static int FTSPI020_check_erase(int ce, int offset, int size)
{
	char *read_buf;
	int i, len;

	read_buf = (char *) ((g_spi020_rd_buf_addr + 0x10) & ~0xF);

	while(size > 0) {

		len = min_t(size, 0x400000);

		memset(read_buf, 0, len);

		spi_flash_info[ce]->read(spi_flash_info[ce], 0, offset, len, read_buf);

		for (i=0; i < len; i++) {
			if (read_buf[i] != 0xFF) {
				prints("Erase: compare data failed at 0x%08x = 0x%x\n",
					(int)(&read_buf[i]), read_buf[i]);
				return 1;
			}
		}

		size -= len;
		offset += len;
	}

	return 0;
}

/* er <ce> <type> <offset> <DataCnt> [er with rd] */
int FTSPI020_erase(int argc, char * const argv[])
{
	int ce, type, addr, size;

	if (argc < 5) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        } 

	type = strtol(argv[2], 0, 0);
	addr = strtol(argv[3], 0, 0);
	size = strtol(argv[4], 0, 0);

	/* Check if bad block for SPI NAND flash */
	if (spi_flash_info[ce]->is_bad_block) {
		if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce], addr)) {
			prints ("You are about to erase bad block 0x%x.\n", addr);
			return 0;
		}
	}

	if (spi_flash_info[ce]->erase(spi_flash_info[ce], type, addr, size)) {
		prints("Erase Sector failed.\n");
		return 0;
	}

	if (argc == 6)
		FTSPI020_check_erase(ce, addr, size);

	return 0;
}

/* er_all <ce list> [er with rd] */
int FTSPI020_erase_all(int argc, char * const argv[])
{

	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int ce, ce_list;

	if (argc < 2) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

	ce_list = strtol(argv[1], 0, 0);

	for (ce = 0; ce < FTSPI020_MAX_CE; ce++) {

		if (!(ce_list & (1 << ce)) || !spi_flash_info[ce])
			continue;

		// Perform the erase operation.
		spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
		
		if (argc == 3)
			FTSPI020_check_erase(ce, 0,  spi_flash_info[ce]->size);
	}

	return 0;
}

/* rst <ce> */
int FTSPI020_reset(int argc, char * const argv[])
{
	int ce;
	struct ftspi020_cmd spi_cmd = {0};

	if (argc != 2) {
		prints("%d arguments for status isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }

	spi_cmd.start_ce = ce;
	spi_cmd.ins_code = CMD_RESET;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_write;

	FTSPI020_issue_cmd(&spi_cmd);

	if (FTSPI020_wait_cmd_complete(10)){
		FTSPI020_reset_hw();
	}

	return 0;
}

/* sts <ce> */
int FTSPI020_report_status(int argc, char * const argv[])
{
	int ce;

	if (argc < 2) {
		prints("%d arguments for status isn't allowed\n", argc);
		return 1;
	}
        
	ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }
	
	spi_flash_info[ce]->report_status(spi_flash_info[ce]);

	return 0;
}

int FTSPI020_burnin_copy(int ce)
{
	int wr_byte, page_size, max_size, i, j, times, err;
	uint32_t offset, dst_offset, src_row, dst_row;
	uint32_t modify_row, modify_col;
	size_t modify_len;
	char *write_buf, *src_buf, *dst_buf, *modify_buf;

	write_buf = (char *) g_spi020_wr_buf_addr;
	src_buf = (char *) g_spi020_rd_buf_addr;
	dst_buf = (char *) (g_spi020_rd_buf_addr + 262144);

	page_size = spi_flash_info[ce]->page_size;
	max_size = spi_flash_info[ce]->size - (page_size << 1);
	offset = times = 0;

	prints("Round: 0\n");
	do {
		wr_byte = (rand() % 262144) + page_size;

		/* spi nand flash:
		 * Each 512-bytes sector can only be written once,
		 * so increase the row_addr to next 512 bytes
		 */
		if (offset)
			offset = (offset + 512) & ~511;

		if (offset + (wr_byte << 1) >= max_size) {
			wr_byte = (max_size - offset) >> 1;

			if (wr_byte < (page_size << 2))
				goto erase;
		}

		/* Prepare the pattern for writing & reading */
		for (i = 0; i < wr_byte; i++)
			*(write_buf + i) = i;

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
		check_bad_block:
			if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce], offset)) {
				prints ("Skip program to bad block 0x%x.\n", offset);
				/* increment to next block */
				offset += spi_flash_info[ce]->erase_sector_size;
				offset &= ~(spi_flash_info[ce]->erase_sector_size-1);

				/* Is it a bad block too */
				goto check_bad_block;
			}
		}

		/* write pattern to source row address */
		if (spi_flash_info[ce]->write(spi_flash_info[ce], 0, offset, wr_byte, write_buf))
			break;

		/* Compare the source data */
		memset(src_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0, offset, wr_byte, src_buf);
		if (err || FTSPI020_compare(write_buf, src_buf, wr_byte))
			return 1;


		src_row = offset / page_size;
		dst_row = j = (offset + wr_byte + page_size) / page_size;

		/* Random data load test */
		modify_row = src_row + (rand() % (j - src_row));
		if (modify_row == src_row)
			modify_row = src_row + 1;

		/* Random load happens at last row of source */
		if (modify_row == (j-1)) {
			i = (offset + wr_byte) % page_size;
		} else {
			i = page_size;
		}

		modify_col = rand() % i;
		if (!modify_col)
			modify_col = 1;

		modify_len = i - modify_col;
		modify_buf = src_buf + page_size - (offset % page_size);
		modify_buf += ((modify_row - src_row - 1) * page_size);
		modify_buf +=  modify_col;

		/* Alter the pattern for random data load */
		for (i = 0; i < modify_len; i++)
			*(modify_buf + i) = (i % 2 == 0) ? 0xAA : 0x55;

		while (src_row < dst_row) {
			if (src_row == modify_row) {
				if (spi_flash_info[ce]->copy_data(spi_flash_info[ce],
								  src_row, j, modify_col, 
								  modify_buf, modify_len))
					return 1;

			} else {
				if (spi_flash_info[ce]->copy_data(spi_flash_info[ce],
								  src_row, j, 0, 0, 0))
					return 1;

			}
			src_row++;
			j++;
		}

		/* column address at source row address */
		i = offset % page_size;
		dst_offset = (dst_row * page_size) + i;

		memset(dst_buf, 0, wr_byte);
		err = spi_flash_info[ce]->read(spi_flash_info[ce], 0, dst_offset, wr_byte, dst_buf);
		if (err || FTSPI020_compare(src_buf, dst_buf, wr_byte))
			return 1;

		offset = dst_offset + wr_byte;
		if (offset >= max_size) {
erase:
			times++;
			prints("Round: %d\n", times);

			err = spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
			if (err || FTSPI020_check_erase(ce, 0, spi_flash_info[ce]->size))
				return 1;

#ifdef FTSPI020_USE_DMA
			g_trans_mode = (g_trans_mode == PIO) ? DMA : PIO;
#else
			g_trans_mode == PIO;
#endif
			offset = 0;

			g_divider += 2;
			if (g_divider > 8)
				g_divider = 2;
			FTSPI020_divider(g_divider);
			prints("Set divider to %d\n", g_divider);

		}
		// Press 'q' to leave the burnin
		if ('q' == ftuart_kbhit())
			break;
	} while (1);

	return 0;
}

/* burnin <ce> <offset> [1:copy] */
int FTSPI020_burnin(int argc, char * const argv[])
{
	int wr_byte, wr_type, rd_type, er_type;
	int start_pos;
	int i, err, nop, times = 0;
	int wr_tp_cnt[2] = { 0 };
	int rd_tp_cnt[7] = { 0 };
	int er_tp_cnt[4] = { 0 };
	char *write_buf, *read_buf;
	char mode;
	int ce;
	uint32_t offset;

	if (argc < 3) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

	ce = strtol(argv[1], 0, 0);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	if (argc == 4) {
		FTSPI020_burnin_copy(ce);
		return 0;
	}

	start_pos = strtol(argv[2], 0, 0);

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;
	er_type = nop = 0;
	mode = mode3;
	g_divider = 2;
	offset = start_pos;

	prints("Round: 0\n");
	do {
		// Get the random num from 1 to 262144
		wr_byte = (rand() % 262144) + 1;
		if ( ((spi_flash_info[ce]->code == 0x1640) || (spi_flash_info[ce]->code == 0x1840)) && 
		     rd_type == W25_WORD_READ_QUAD_IO) {
			/* Temp solution for Winbond specific read type */
			if (offset % 2 != 0)
				offset++;

		} else if ((spi_flash_info[ce]->code == 0x7F20) || (spi_flash_info[ce]->code == 0x00F1)) {
			/* spi nand flash:
			 * Each 512-bytes sector can only be written once,
			 * so increase the row_addr to next 512 bytes
			 */
			offset = (offset + 512) & ~511;

		} else if (spi_flash_info[ce]->code == 0x8E25) {
			wr_byte &= ~1;

		}

		if (offset + wr_byte > spi_flash_info[ce]->size)
			wr_byte = spi_flash_info[ce]->size - offset;

		if (wr_byte < 512)
			goto erase;

		wr_type = rand() % spi_flash_info[ce]->max_wr_type;
		// Get the read type(Read page, Fast read, and Fast read dual output)
		rd_type = rand() % spi_flash_info[ce]->max_rd_type;


		// Prepare the pattern for writing & reading
		for (i = 0; i < wr_byte; i++)
			*(write_buf + i) = i;
		memset(read_buf, 0, wr_byte);

		/* Check if bad block for SPI NAND flash */
		if (spi_flash_info[ce]->is_bad_block) {
		check_bad_block:
			if (spi_flash_info[ce]->is_bad_block(spi_flash_info[ce], offset)) {
				prints ("Skip program to bad block 0x%x.\n", offset);
				/* increment to next block */
				offset += spi_flash_info[ce]->erase_sector_size;
				offset &= ~(spi_flash_info[ce]->erase_sector_size-1);

				/* Is it a bad block too */
				goto check_bad_block;
			}
		}

		if (spi_flash_info[ce]->write(spi_flash_info[ce], wr_type, offset, wr_byte, write_buf))
			break;
		wr_tp_cnt[wr_type]++;

		/* Some read type is not support due to controller setting */
		err = spi_flash_info[ce]->read(spi_flash_info[ce], rd_type, offset, wr_byte, read_buf);
		if (!err) {
			if (FTSPI020_compare(write_buf, read_buf, wr_byte))
				break;

			prints("Compare OK!!\n");
			rd_tp_cnt[rd_type]++;
		} else {
			if (err == 1)
				break;
			else if (err == 2)
				prints(" %s not support by HW.\n", spi_flash_info[ce]->get_string(READ, rd_type));
		}

		offset = offset + wr_byte;
		if (offset >= spi_flash_info[ce]->size) {
erase:
			times++;
			prints("Round: %d\n", times);

			if (er_type == spi_flash_info[ce]->max_er_type) {
				if (start_pos == 0)
					err = spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
				else
					err = spi_flash_info[ce]->erase(spi_flash_info[ce], 0, start_pos,
								(spi_flash_info[ce]->size - start_pos));
			 } else 
				err = spi_flash_info[ce]->erase(spi_flash_info[ce], er_type, start_pos,
								(spi_flash_info[ce]->size - start_pos));

			er_tp_cnt[er_type]++;
			er_type++;

			err = FTSPI020_check_erase(ce, start_pos, spi_flash_info[ce]->size);

			if (err)
				break;

#ifdef FTSPI020_USE_DMA
			g_trans_mode = (g_trans_mode == PIO) ? DMA : PIO;
#else
			g_trans_mode == PIO;
#endif
			FTSPI020_operate_mode((mode == mode3) ? mode0 : mode3);

			if (er_type > spi_flash_info[ce]->max_er_type) {
				er_type = 0;
			}

			offset = start_pos;
	
			g_divider += 2;
			if (g_divider > 8)
				g_divider = 2;
			FTSPI020_divider(g_divider);
			prints("Set divider to %d\n", g_divider);

		}
		// Press 'q' to leave the burnin
		if ('q' == ftuart_kbhit())
			break;
	} while (1);

	prints(" Final position at  %d.\n", offset);
	for (i = 0; i < spi_flash_info[ce]->max_rd_type; i++) {
		prints(" %s: %d times.\n", spi_flash_info[ce]->get_string(READ, i), rd_tp_cnt[i]);
	}

	for (i = 0; i < spi_flash_info[ce]->max_wr_type; i++) {
		prints(" %s: %d times.\n", spi_flash_info[ce]->get_string(WRITE, i), wr_tp_cnt[i]);
	}

	for (i = 0; i <= spi_flash_info[ce]->max_er_type; i++) {
		if (i == spi_flash_info[ce]->max_er_type)
			prints(" Erase Chip: %d times.\n", er_tp_cnt[i]);
		else
			prints(" %s: %d times.\n", spi_flash_info[ce]->get_string(ERASE, i), er_tp_cnt[i]);
	}

	return 0;
}

/* perf <ce> <len> [type] */
int FTSPI020_report_performance(int argc, char * const argv[])
{
	int bytes, i;
	char *read_buf/*, *write_buf*/;
	int ce, type;

	if (argc < 3) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = strtol(argv[1], 0, 0);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }


	bytes = strtol(argv[2], 0, 0);
#if 0
	// Prepare the pattern for writing & reading
	write_buf = (chard *) g_spi020_wr_buf_addr;
	for (i = 0; i < bytes; i++)
		*(write_buf + i) = i;
	// Because we only want to measure performance, don't care for error.
	spi_flash_info[ce]->write(spi_flash_info[ce], 0, 0, bytes, write_buf);
#endif
	read_buf = (char *) g_spi020_rd_buf_addr;

	g_divider = 2;
	FTSPI020_divider(g_divider);

	if (argc == 4) {
		type = strtol(argv[3], 0, 0);
		spi_flash_info[ce]->read(spi_flash_info[ce], type, 0, bytes, read_buf);
		prints("Rd:%d ms.\n", get_timer(0) - t0_perf);
	} else {
		for (i = 0; i < spi_flash_info[ce]->max_rd_type ; i++) {
			spi_flash_info[ce]->read(spi_flash_info[ce], i, 0, bytes, read_buf);
			prints("Rd:%d ms.\n", get_timer(0) - t0_perf);

	#if 0
			//Error message print inside the function
			FTSPI020_compare(write_buf, read_buf, bytes);
	#endif
		}
	}

	return 0;
}

/* tr <pio|dma> */
int FTSPI020_transfer_type(int argc, char * const argv[])
{
	if (argc != 2)
		return 1;
	
	if (strcmp(argv[1], "pio") == 0) 
		g_trans_mode = PIO;	
	else
		g_trans_mode = DMA;

	return 0;
}

/* intr <0|1> */
int FTSPI020_use_interrput(int argc, char * const argv[])
{
	if (argc != 2)
		return 1;

#ifdef FTSPI020_USE_INTERRUPT
	g_cmd_intr_enable = strtol(argv[1], 0, 0);
#else
	g_cmd_intr_enable = 0;
#endif

	if (g_cmd_intr_enable) {
		enable_interrupts(); /* Clear CPU I bit */
		FTSPI020_cmd_complete_intr_enable(1);
	} else {
		disable_interrupts();
		FTSPI020_cmd_complete_intr_enable(0);
	}

	prints(" %s interrupt\n", g_cmd_intr_enable ? "Use" : "Not use");

	return 0;
}

/* bbt [ce list] */
int FTSPI020_scan_bad_blocks(int argc, char * const argv[])
{
	int ce_list, ce;

	if (argc == 2)
		ce_list = strtol(argv[1], 0, 0);
	else
		ce_list = 0xf;

	for (ce = 0; ce < FTSPI020_MAX_CE; ce++) {

		if (!(ce_list & (1 << ce)) ||
		    !spi_flash_info[ce] ||
		    !spi_flash_info[ce]->scan_bad_blocks)
			continue;

		/* Build bad block table */
		spi_flash_info[ce]->scan_bad_blocks(spi_flash_info[ce]);
	}
}

/* ls */
int FTSPI020_list_ce(int argc, char * const argv[])
{
	int i;
	
	for (i=0 ; i < FTSPI020_MAX_CE; i++) {
		prints(" CE %d:\n", i);
		if (!spi_flash_info[i])
			prints(" - No Flash \n");
		else {
			prints(" - Name: %s.\n", spi_flash_info[i]->name);
			prints(" - Code: 0x%04x.\n", spi_flash_info[i]->code);
			prints(" - Page Size: %d bytes.\n", spi_flash_info[i]->page_size);
			prints(" - Number of Page: %d.\n", spi_flash_info[i]->nr_pages);
			prints(" - Sector Size: %d bytes.\n", spi_flash_info[i]->erase_sector_size);
			prints(" - Chip Size: %d bytes.\n", spi_flash_info[i]->size);
		}
	}

	return 0;
}

int FTSPI020_main(int argc, char * const argv[])
{
	int i;
	char cmdstr[CMDLEN];

#if defined ROM_SPI && defined COPY_DATA_ONLY
	prints("\n Program runs at SPI Flash.\n");
	prints(" FTSPI020 can not be used as command mode.\n");
	return 0;
#endif
	prints("\nFTSPI020 FPGA Non-OS verification code %s\n", FTSPI020_FW_VERSION);

	if (!spi_flash_info[0]) {
		prints(" HW. Revision:0x%08x\n", FTSPI020_32BIT(REVISION));
		if (FTSPI020_init()) {
			prints(" Init failed\n");
		}
	}

	if (g_cmd_intr_enable)
		enable_interrupts(); /* Clear CPU I bit */
	else
		disable_interrupts();

	for (i=0 ; i < FTSPI020_MAX_CE; i++) {
		prints(" Scanning CE %d ... ", i);
		spi_flash_info[i] = FTSPI020_probe(i);
	}

	while (1) {
		puts("\nftspi020:> ");

		scan_string(cmdstr);

		if (cmd_exec(cmdstr, ftspi020_cmd_tbl))
			break;
	}
}
