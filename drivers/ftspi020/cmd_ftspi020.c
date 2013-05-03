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
 * 2010/08/13   BingJiun	 Scan all CEs for available flash.         
 * -------------------------------------------------------------------------
 */

#include <common.h>

#include "ftspi020.h"
#include "ftspi020_cntr.h"

static int FTSPI020_write(int argc, char * const argv[]);
static int FTSPI020_read(int argc, char * const argv[]);
static int FTSPI020_erase(int argc, char * const argv[]);
static int FTSPI020_erase_all(int argc, char * const argv[]);
static int FTSPI020_report_status(int argc, char * const argv[]);
static int FTSPI020_burnin(int argc, char * const argv[]);
static int FTSPI020_report_performance(int argc, char * const argv[]);
static int FTSPI020_transfer_type(int argc, char * const argv[]);
static int FTSPI020_use_interrput(int argc, char * const argv[]);
static int FTSPI020_list_ce(int argc, char * const argv[]);

static cmd_t ftspi020_cmd_tbl[] = {
	{"wr", "<ce> <type> <rowAddr> <DataCnt> [1:wr with cmp, 2: burn image]", FTSPI020_write},
	{"rd", "<ce> <type> <rowAddr> <DataCnt>", FTSPI020_read},
	{"er", "<ce> <type> <rowAddr> <DataCnt> [er with cmp]", FTSPI020_erase},
	{"er_all", "<ce list> [er with cmp]", FTSPI020_erase_all},
	{"status", "<ce>", FTSPI020_report_status},
	{"burnin", "<ce> <start offset>", FTSPI020_burnin},
	{"perf", "<ce> <len>", FTSPI020_report_performance},
	{"tr", "<pio|dma>", FTSPI020_transfer_type},
	{"intr", "<0|1>", FTSPI020_use_interrput},
	{"ls", "", FTSPI020_list_ce},
	{"quit", "", 0},
	{0} /* end of FTSPI020c023CmdTbl */
};

struct spi_flash *spi_flash_info[FTSPI020_MAX_CE];

// For choosing the check status is via hw or sw.
char g_check_status = check_status_by_hw;

extern int g_cmd_intr_enable;

/* wr <ce> <type> <rowAddr> <DataCnt> [1:wr with cmp, 2: burn image] */
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
	
	ce = atoi(argv[1]);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	}

	if (!str_to_hex(argv[3], &addr, sizeof(addr) * 2))
		return 1;

	if (!str_to_hex(argv[4], &len, sizeof(len) * 2))
		return 1;

	if (argc == 6)
		burn = atoi(argv[5]);

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


	if (spi_flash_info[ce]->write(spi_flash_info[ce], atoi(argv[2]), addr, len, write_buf))
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

/* rd <ce> <type> <rowAddr> <DataCnt> */
int FTSPI020_read(int argc, char * const argv[])
{
	int ce, addr, len;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int *p;

	if (argc != 5) {
		prints("%d arguments for rd isn't allowed\n", argc);
		return 1;
	}

	ce = atoi(argv[1]);
	if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
		prints(" No flash at CE %d\n", ce);
		return 1;
	} 

	if (!str_to_hex(argv[3], &addr, sizeof(addr) * 2))
		return 1;

	if (!str_to_hex(argv[4], &len, sizeof(len) * 2))
		return 1;

	memset(read_buf, 0, len);

	spi_flash_info[ce]->read(spi_flash_info[ce], atoi(argv[2]), addr, len, read_buf);

	p = (int *) read_buf;
	prints("Data content: %x %x %x %x\n", p[0], p[1], p[2], p[3]);

	return 0;
}

/* er <ce> <type> <rowAddr> <DataCnt> [er with rd] */
int FTSPI020_erase(int argc, char * const argv[])
{

	char *golden_buf = (char *) g_spi020_wr_buf_addr;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int ce, type, addr, size;

	if (argc < 5) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = atoi(argv[1]);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        } 

	type = atoi(argv[2]);

	if (!str_to_hex(argv[3], &addr, sizeof(addr) * 2))
		return 1;

	if (!str_to_hex(argv[4], &size, sizeof(size) * 2))
		return 1;

	if (spi_flash_info[ce]->erase(spi_flash_info[ce], type, addr, size)) {
		prints("Erase Sector failed.\n");
		return 0;
	}

	if (argc == 6) {
		int offset, e_offset, len;
		// Prepare the golden buf for comparing.
		memset(golden_buf, 0xFF, g_spi020_rd_buf_length);

		offset = addr;
		e_offset = addr + size;
		len = g_spi020_rd_buf_length;

		while(size > 0) {

			if (offset + len > e_offset)
				len = e_offset - offset;

			memset(read_buf, 0, len);
			spi_flash_info[ce]->read(spi_flash_info[ce], atoi(argv[5]), offset, len, read_buf);
			if (FTSPI020_compare(golden_buf, read_buf, len)) {
				prints("Empty checking failed in erase type %d\n", type);
				return 0;
			}

			size -= len;
			offset += len;
		}
	}

	return 0;
}

/* er_all <ce list> [er with rd] */
int FTSPI020_erase_all(int argc, char * const argv[])
{

	char *golden_buf = (char *) g_spi020_wr_buf_addr;
	char *read_buf = (char *) g_spi020_rd_buf_addr;
	int rd_size_each_times;
	int ce, ce_list, offset;

	if (argc < 2) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

	if (!str_to_hex(argv[1], &ce_list, sizeof(ce_list) * 2))
		return 1;

	for (ce = 0; ce < FTSPI020_MAX_CE; ce++) {

		if (!(ce_list & (1 << ce)) || !spi_flash_info[ce])
			continue;

		// Perform the erase operation.
		spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
		
		offset = 0;
		if (argc == 3) {
			rd_size_each_times = g_spi020_rd_buf_length;
			// Prepare the golden buf for comparing.
			memset(golden_buf, 0xFF, rd_size_each_times);

			do {
				memset(read_buf, 0, rd_size_each_times);
				spi_flash_info[ce]->read(spi_flash_info[ce], atoi(argv[2]), offset, 
							 rd_size_each_times, read_buf);
				if (FTSPI020_compare(golden_buf, read_buf, rd_size_each_times))
					break;
				offset += rd_size_each_times;
			} while (offset + rd_size_each_times <= spi_flash_info[ce]->size);
		}
	}
	return 0;
}

/* status <ce> */
int FTSPI020_report_status(int argc, char * const argv[])
{
	int ce;

	if (argc < 2) {
		prints("%d arguments for status isn't allowed\n", argc);
		return 1;
	}
        
	ce = atoi(argv[1]);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }
	
	spi_flash_info[ce]->report_status(spi_flash_info[ce]);

	return 0;
}

/* burnin <ce> <start offset> */
int FTSPI020_burnin(int argc, char * const argv[])
{
	int wr_byte, wr_type, rd_type, er_type;
	int start_pos;
	int i, err, times = 0;
	int wr_tp_cnt[2] = { 0 };
	int rd_tp_cnt[7] = { 0 };
	int er_tp_cnt[4] = { 0 };
	char *write_buf, *read_buf;
	char mode;
	int ce;
	uint32_t offset;

	if (argc != 3) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = atoi(argv[1]);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }

	if (!str_to_hex(argv[2], &start_pos, sizeof(start_pos) * 2))
		return 1;

	write_buf = (char *) g_spi020_wr_buf_addr;
	read_buf = (char *) g_spi020_rd_buf_addr;
	er_type = 0;
	mode = mode3;
	g_divider = 2;
	offset = start_pos;

	prints("Round: 0\n");
	do {
		// Get the random num from 1 to 262144
		wr_byte = (rand() % 262144) + 1;
		if (offset + wr_byte > spi_flash_info[ce]->size)
			wr_byte = spi_flash_info[ce]->size - offset;
		wr_type = rand() % spi_flash_info[ce]->max_wr_type;
		// Get the read type(Read page, Fast read, and Fast read dual output)
		rd_type = rand() % spi_flash_info[ce]->max_rd_type;

#if defined(Sst_SST25VF080B)
		if (spi_flash_info[ce]->code == 0x8E25) 
			wr_byte &= ~1;
#endif

#if defined(Winbond_W25Q32BV) || defined(Winbond_W25Q128BV)
		/* Temp solution for Winbond specific read type */
		if ( ((spi_flash_info[ce]->code == 0x1640) || (spi_flash_info[ce]->code == 0x1840)) && 
		     rd_type == W25_WORD_READ_QUAD_IO) {
			if (offset % 2 != 0)
				offset++;

			if (offset + wr_byte > spi_flash_info[ce]->size)
				wr_byte = spi_flash_info[ce]->size - offset;
		}
#endif

		// Prepare the pattern for writing & reading
		for (i = 0; i < wr_byte; i++)
			*(write_buf + i) = i;
		memset(read_buf, 0, wr_byte);

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
			times++;
			prints("Round: %d\n", times);

			if (er_type == spi_flash_info[ce]->max_er_type) {
				if (start_pos == 0)
					err = spi_flash_info[ce]->erase_all(spi_flash_info[ce]);
				else
					err = spi_flash_info[ce]->erase(spi_flash_info[ce], 0, start_pos,
								spi_flash_info[ce]->size);
			 } else 
				err = spi_flash_info[ce]->erase(spi_flash_info[ce], er_type, start_pos,
								spi_flash_info[ce]->size);

			er_tp_cnt[er_type]++;
			er_type++;

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

/* perf <ce> <len> */
int FTSPI020_report_performance(int argc, char * const argv[])
{
	int bytes, i;
	clock_t t0;
	char *read_buf/*, *write_buf*/;
	int ce;

	if (argc != 3) {
		prints("%d arguments for er isn't allowed\n", argc);
		return 1;
	}

        ce = atoi(argv[1]);
        if ((ce >= FTSPI020_MAX_CE) || !spi_flash_info[ce]) {
                prints(" No flash at CE %d\n", ce);
                return 1;
        }

	bytes = atoi(argv[2]);
#if 0
	// Prepare the pattern for writing & reading
	write_buf = (chard *) g_spi020_wr_buf_addr;
	for (i = 0; i < bytes; i++)
		*(write_buf + i) = i;
	// Because we only want to measure performance, don't care for error.
	spi_flash_info[ce]->write(spi_flash_info[ce], 0, 0, bytes, write_buf);
#endif
	read_buf = (char *) g_spi020_rd_buf_addr;

	for (i = 0; i < spi_flash_info[ce]->max_rd_type; i++) {
		t0 = get_timer(0);
		spi_flash_info[ce]->read(spi_flash_info[ce], i, 0, bytes, read_buf);
		prints("Rd:%d ms.\n", get_timer(0) - t0);

#if 0
		//Error message print inside the function
		FTSPI020_compare(write_buf, read_buf, bytes);
#endif
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
	g_cmd_intr_enable = atoi(argv[1]);
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
