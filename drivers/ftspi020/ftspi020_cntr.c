/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_cntr.c
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2009/07/06   Mike          
 * 2010/08/13   BingJiun	 Scan all CEs for available flash.         
 * -------------------------------------------------------------------------
 */
#include <common.h>
#include <malloc.h>

#include "ftspi020.h"
#include "ftspi020_cntr.h"

extern int FTSPI020_DMA_LLD_STRUCT_ADDR;
int g_spi020_wr_buf_addr;
int g_spi020_wr_buf_length;
int g_spi020_rd_buf_addr;
int g_spi020_rd_buf_length;

int g_cmd_intr_enable;
int debug = 0;

Transfer_type g_trans_mode = PIO;
uint8_t g_divider = 6;
volatile uint32_t ftspi020_cmd_complete;

unsigned long randGen(void);

// ************************* local func *************************
void FTSPI020_dma_enable(uint8_t enable)
{
	FTSPI020_8BIT(INTR_CONTROL_REG) &= ~dma_handshake_enable;

	if (enable)
		FTSPI020_8BIT(INTR_CONTROL_REG) |= dma_handshake_enable;
}

void FTSPI020_cmd_complete_intr_enable(uint8_t enable)
{
	FTSPI020_8BIT(INTR_CONTROL_REG) &= ~cmd_complete_intr_enable;

	if (enable)
		FTSPI020_8BIT(INTR_CONTROL_REG) |= cmd_complete_intr_enable;
}

void FTSPI020_reset_hw(void)
{
	FTSPI020_32BIT(CONTROL_REG) |= BIT8;
}

void FTSPI020_operate_mode(uint8_t mode)
{
	FTSPI020_32BIT(CONTROL_REG) &= ~BIT4;
	FTSPI020_32BIT(CONTROL_REG) |= ((mode << 4) & BIT4);
}

void FTSPI020_busy_location(uint8_t location)
{
	FTSPI020_32BIT(CONTROL_REG) &= ~(BIT18 | BIT17 | BIT16);
	FTSPI020_32BIT(CONTROL_REG) |= ((location << 16) & (BIT18 | BIT17 | BIT16));
}

void FTSPI020_divider(uint8_t divider)
{
	uint8_t val;

	if (divider == 2)
		val = divider_2;
	else if (divider == 4)
		val = divider_4;
	else if (divider == 6)
		val = divider_6;
	else if (divider == 8)
		val = divider_8;
	else {
		prints("Not valid divider value %d\n", divider);
		return ;
	}

	FTSPI020_32BIT(CONTROL_REG) &= ~(BIT1 | BIT0);
	FTSPI020_32BIT(CONTROL_REG) |= ((val) & (BIT1 | BIT0));

}

void FTSPI020_command_based(void)
{
	FTSPI020_32BIT(CONTROL_REG) &= ~BOOT_MODE;
}

void FTSPI020_rxfifo_full(void)
{
	while (!(FTSPI020_32BIT(STATUS_REG) & BIT1)) ;
}

void FTSPI020_txfifo_empty(void)
{
	while (!(FTSPI020_32BIT(STATUS_REG) & BIT0)) ;
}

int FTSPI020_txfifo_depth(void)
{
	// The unit of returning value is byte.
	return ((FTSPI020_8BIT(FEATURE) & 0xFF) << 2);
}

int FTSPI020_rxfifo_depth(void)
{
	// The unit of returning value is byte.
	return ((FTSPI020_8BIT(FEATURE + 1) & 0xFF) << 2);
}

void FTSPI020_check_rx_counter(int len)
{
	if (len != FTSPI020_32BIT(RX_DATA_CNT)) {
		prints("The value of 'rx_data_count' doesn't match the amount of transfer data.\n");
	}
}

void FTSPI020_check_tx_counter(int len)
{
	if (len != FTSPI020_32BIT(TX_DATA_CNT)) {
		prints("The value of 'tx_data_count' doesn't match the amount of transfer data.\n");
	}
}

// *********************** external func ***********************
int FTSPI020_support_dtr_mode(void)
{
	return (FTSPI020_32BIT(FEATURE) & support_dtr_mode);
}

int FTSPI020_init(void)
{
	/* Already init */
	if (g_spi020_rd_buf_addr || g_spi020_wr_buf_addr)
		return 0;

#ifdef FTSPI020_USE_INTERRUPT
	irq_set_type(FTSPI020_IRQ, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler (FTSPI020_IRQ, FTSPI020_Interrupt_Handler, 0);
	irq_set_enable(FTSPI020_IRQ);
#endif
#ifdef FTSPI020_USE_DMA
#if defined PLATFORM_AHB || defined PLATFORM_A320
	//initialize DMA channel
	fLib_InitDMA(0, 0, (1 << FTSPI020_DMA_RD_CHNL) | (1 << FTSPI020_DMA_WR_CHNL));

#if defined PLATFORM_A320
	/* DMA req/ack pair setting for handshake mode */
	*((int *) 0x98100028) = 0x18000;
#endif
#elif defined PLATFORM_AXI
	InitDMA();
#else
#error  "Specify correct platform name."
#endif

	FTSPI020_DMA_LLD_STRUCT_ADDR = (int) malloc(0x2000);
	if (!FTSPI020_DMA_LLD_STRUCT_ADDR) {
		prints("Allocate memory for DMA LLD structure failed\n");
		return 1;
	}
	prints(" DMA LLD struct address 0x%x\n", FTSPI020_DMA_LLD_STRUCT_ADDR); 
	FTSPI020_dma_enable(1);
#endif

	FTSPI020_reset_hw();
	FTSPI020_command_based();
	FTSPI020_operate_mode(mode3);
	FTSPI020_divider(g_divider);

	g_cmd_intr_enable = 0;
	disable_interrupts();
	FTSPI020_cmd_complete_intr_enable(0);

	g_spi020_wr_buf_length =  g_spi020_rd_buf_length = 0x800000;
	g_spi020_wr_buf_addr = (int) malloc(g_spi020_wr_buf_length << 1);
	if (!g_spi020_wr_buf_addr) {
		prints("Allocate memory for read/write buffer failed\n");
		return 1;
	}

	g_spi020_rd_buf_addr = g_spi020_wr_buf_addr + g_spi020_wr_buf_length;

	prints(" Write buffer address 0x%x, read buffer address 0x%x\n", 
		g_spi020_wr_buf_addr, g_spi020_rd_buf_addr);
	
	return 0;
}

struct spi_flash *FTSPI020_probe(uint32_t ce)
{
	int ret;
	uint8_t idcode[3];
	struct spi_flash *flash;
	struct ftspi020_cmd spi_cmd = {0};

	spi_cmd.start_ce = ce;
	spi_cmd.ins_code = CMD_READ_ID;
	spi_cmd.ins_len = instr_1byte;
	spi_cmd.write_en = spi_read;
	spi_cmd.dtr_mode = dtr_disable;
	spi_cmd.spi_mode = spi_operate_serial_mode;
	spi_cmd.data_cnt = 3;

	FTSPI020_issue_cmd(&spi_cmd);
	FTSPI020_data_access(NULL, idcode, sizeof(idcode));
	ret = FTSPI020_wait_cmd_complete(10);
	if (ret) {
		goto err;
	}

	prints("SF: Got idcode %02x %02x %02x\n", idcode[0], idcode[1], idcode[2]);

	switch (idcode[0]) {
#if defined(Winbond_W25Q32BV) || defined(Winbond_W25Q128BV)
	case 0xEF:
		flash = spi_flash_probe_winbond(idcode); 
		break;
#endif
#if defined(Mxic_MX25L12845EM1)
	case 0xC2:
		flash = spi_flash_probe_mxic(idcode);
		break;
#endif
#if defined(Spansion_S25FL032P) || defined(Spansion_S25FL128S)
	case 0x01:
		flash = spi_flash_probe_spansion(idcode);
		break;
#endif
#if defined(Sst_SST25VF080B)
	case 0xBF:
		flash = spi_flash_probe_sst(idcode);
		break;
#endif
	default:
		prints("SF: Unsupported manufacturer 0x%02X @ CE %d\n", idcode[0], ce);
		flash = NULL;
		break;
	}

	if (flash)
		flash->ce = ce;

	return flash;

      err:
	return NULL;
}

int spi_flash_cmd(struct spi_flash *slave, uint8_t * cmd, void *response, size_t len)
{
	int ret;

	if (debug > 2)
		prints(" CMD code 0x%x length %d\n", cmd[0], len);

	ret = slave->spi_xfer(slave, len, cmd, NULL, SPI_XFER_CMD_STATE | SPI_XFER_CHECK_CMD_COMPLETE);
	if (ret) {
		prints("%s: Failed to send command %02x: %d\n", slave->name, cmd[0], ret);
		return ret;
	}

	if (len && response != NULL) {
		ret = slave->spi_xfer(slave, len, NULL, response, SPI_XFER_DATA_STATE); 
		if (ret) {
			prints("%s: Failed to read response (%d bytes): %d\n", slave->name, len, ret);
		}
	} else if ((len && response == NULL) || (!len && response != NULL)) {
		prints("%s: Failed to read response due to the mismatch of len and response (%d bytes): %d\n",
			    slave->name, len, ret);
	}

	return ret;
}

int spi_flash_cmd_write(struct spi_flash *slave, uint8_t * cmd, const void *data, int data_len)
{
	int ret;

	if (debug)
		prints(" CMD write 0x%x 0x%x 0x%x 0x%x\n", cmd[0], cmd[1], cmd[2], cmd[3]);

	ret = slave->spi_xfer(slave, data_len, cmd, NULL, SPI_XFER_CMD_STATE);
	if (ret) {
		prints("%s: Failed to send command %02x\n", slave->name, cmd[0]);
		return ret;
	} else if (data_len != 0) {
		if (debug)
			prints(" CMD data length %d bytes: 0x%x\n", data_len, *((uint32_t *)data));

		ret = slave->spi_xfer(slave, data_len, data, NULL, SPI_XFER_DATA_STATE | SPI_XFER_CHECK_CMD_COMPLETE);
		if (ret) {
			prints("%s: Failed to write data (%d bytes): 0x%x\n", slave->name, data_len, (uint32_t)data);
		}
	}

	return ret;
}

int spi_flash_cmd_read(struct spi_flash *slave, uint8_t * cmd, void *data, int data_len)
{
	int ret;

	if (debug)
		prints(" CMD read 0x%x 0x%x 0x%x 0x%x\n", cmd[0], cmd[1], cmd[2], cmd[3]);

	ret = slave->spi_xfer(slave, data_len, cmd, NULL, SPI_XFER_CMD_STATE);
	if (ret) {
		prints("%s: Failed to send command %02x: %d\n", slave->name, cmd[1], ret);
		return ret;
	} else if (data_len != 0) {
		if (debug)
			prints(" CMD data length %d bytes: 0x%x\n", data_len, *((uint32_t *)data));

		ret = slave->spi_xfer(slave, data_len, NULL, data, SPI_XFER_DATA_STATE | SPI_XFER_CHECK_CMD_COMPLETE);
		if (ret) {
			prints("%s: Failed to read data (%d bytes): 0x%x\n", slave->name, data_len, (uint32_t)data);
		}
	}

	return ret;
}

void FTSPI020_flush_cmd_queue(void)
{
	FTSPI020_8BIT(CONTROL_REG + 1) |= BIT0;
}

void FTSPI020_show_status(void)
{
	prints("status:0x%02x\n", FTSPI020_8BIT(SPI_READ_STATUS));
}

void FTSPI020_read_status(uint8_t * status)
{
	*status = FTSPI020_8BIT(SPI_READ_STATUS);
}

void FTSPI020_show_content(void *data_buf, int len)
{
	int i = 0;
	uint8_t *uint8_t_data_buf = (uint8_t *) (data_buf);

	if (len != 0) {

		do {
			if (i % 16 == 0 && i != 0) {
				prints("\n");
			}
			prints("%02x ", *(uint8_t_data_buf + i));
			i++;

		} while (i != len);
		prints("\n");
	}

}

uint8_t FTSPI020_compare(uint8_t * data1_buf, uint8_t * data2_buf, uint32_t len)
{
	int i;

	/*if(FA520_CPUCheckDCacheEnable())
	   {
	   FA520_CPUInvalidateDCacheAll();
	   FA520_CPUCleanDCacheAll();
	   } */

	for (i = 0; i < len; i++) {
		if (*(data1_buf + i) != *(data2_buf + i)) {
			prints("Compare failed at %d(data1:0x%x but data2:0x%x)\n", i, *(data1_buf + i),
				    *(data2_buf + i));
			return 1;
		}
	}
	return 0;
}

uint8_t FTSPI020_issue_cmd(struct ftspi020_cmd * command)
{
	int cmd_feature1, cmd_feature2;

	if (g_cmd_intr_enable)
		ftspi020_cmd_complete = 0;

	FTSPI020_32BIT(SPI_FLASH_ADDR) = command->spi_addr;

	cmd_feature1 = ((command->conti_read_mode_en & 0x1) << 28) |
	    ((command->ins_len & 0x3) << 24) | ((command->dum_2nd_cyc & 0xFF) << 16) | ((command->addr_len & 0x7) << 0);
	FTSPI020_32BIT(SPI_CMD_FEATURE1) = cmd_feature1;

	FTSPI020_32BIT(SPI_DATA_CNT) = command->data_cnt;

	cmd_feature2 = ((command->ins_code & 0xFF) << 24) |
	    ((command->conti_read_mode_code & 0xFF) << 16) |
	    ((command->start_ce & 0x3) << 8) |
	    ((command->spi_mode & 0x7) << 5) |
	    ((command->dtr_mode & 0x1) << 4) |
	    ((command->read_status & 0x1) << 3) |
	    ((command->read_status_en & 0x1) << 2) | ((command->write_en & 0x1) << 1);

	FTSPI020_32BIT(SPI_CMD_FEATURE2) = cmd_feature2;
#if 0
	prints("************************** Cmd Index:0x%02x *********************\n", command->ins_code);
	prints("Addr        (0x00): 0x%08x, Cmd feature1 (0x04): 0x%08x\n", command->spi_addr, cmd_feature1);
	prints("Data cnt.   (0x08): 0x%08x, Cmd feature2 (0x0C): 0x%08x\n", command->data_cnt, cmd_feature2);
#endif
	return 0;
}

int FTSPI020_wait_cmd_complete(uint32_t wait_ms)
{
	ulong	t0, t1;

	t0 = get_timer(0);	
	do {
		if (g_cmd_intr_enable) {

			if (ftspi020_cmd_complete)
				break;

		} else {
			int intr_status;

			intr_status = FTSPI020_32BIT(INTR_STATUS_REG);

			if (intr_status & cmd_complete) {
				FTSPI020_32BIT(INTR_STATUS_REG) |= cmd_complete;
				if (debug > 2)
					prints(" Wait complete OK \n");
				break;
			}
		}

		t1 = get_timer(0);
		if (t1 - t0 > wait_ms) {
			FTSPI020_flush_cmd_queue();
			prints("Timeout when wait %d ms for cmd complete\n", wait_ms);
			return 1;
		}

	} while (1);

	return 0;
}

void FTSPI020_data_access(uint8_t * dout, uint8_t * din, uint32_t len)
{
#ifdef FTSPI020_USE_DMA
	if (g_trans_mode == DMA) {
		int ret;

		if (dout != NULL) {
			ret = ftspi020_Start_DMA(FTSPI020_DMA_RD_CHNL, (int) dout,
					(int) (FTSPI020_REG_BASE + SPI020_DATA_PORT), len, 0x0,
					0x0, 4, 0, 2, FTSPI020_DMA_PRIORITY);
		} else if (din != NULL) {
			/* Not multiple of 4 bytes */
			if (len & 0x3)
				len = (len + 4) & ~0x3;

			ret = ftspi020_Start_DMA(FTSPI020_DMA_RD_CHNL, (int) (FTSPI020_REG_BASE + SPI020_DATA_PORT),
					(int) din, len, 0x2, 0x2, 2, 2, 0, FTSPI020_DMA_PRIORITY);
		}

		if (ret)
			prints (" DMA transfer error !\n");
	} else {
#endif
#if defined(WORD_ACCESS)
		int access_byte, total_byte = len;

		if (dout != NULL) {
			while (total_byte > 0) {
				FTSPI020_txfifo_empty();
				access_byte = min_t(total_byte, FTSPI020_txfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte >= 4) {
						FTSPI020_32BIT(SPI020_DATA_PORT) = *((int *) dout);
						dout += 4;
						access_byte -= 4;
					} else if (access_byte == 3) {
						FTSPI020_16BIT(SPI020_DATA_PORT) = *((uint16_t *) dout);
						dout += 2;
						FTSPI020_8BIT(SPI020_DATA_PORT) = *((uint8_t *) dout);
						dout += 1;
						access_byte -= 3;
					} else if (access_byte == 2) {
						FTSPI020_16BIT(SPI020_DATA_PORT) = *((uint16_t *) dout);
						dout += 2;
						access_byte -= 2;
					} else {
						FTSPI020_8BIT(SPI020_DATA_PORT) = *((uint8_t *) dout);
						dout += 1;
						access_byte -= 1;
					}
				}
			}
			//FTSPI020_check_tx_counter(len);
		} else if (din != NULL) {
			while (total_byte > 0) {
				FTSPI020_rxfifo_full();
				access_byte = min_t(total_byte, FTSPI020_rxfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte >= 4) {
						*((int *) din) = FTSPI020_32BIT(SPI020_DATA_PORT);
						din += 4;
						access_byte -= 4;
					} else if (access_byte == 3) {
						*((uint16_t *) din) = FTSPI020_16BIT(SPI020_DATA_PORT);
						din += 2;
						*((uint8_t *) din) = FTSPI020_8BIT(SPI020_DATA_PORT);
						din += 1;
						access_byte -= 3;
					} else if (access_byte == 2) {
						*((uint16_t *) din) = FTSPI020_16BIT(SPI020_DATA_PORT);
						din += 2;
						access_byte -= 2;
					} else {
						*((uint8_t *) din) = FTSPI020_8BIT(SPI020_DATA_PORT);
						din += 1;
						access_byte -= 1;
					}
				}
			}
			//FTSPI020_check_rx_counter(len);
		}
#elif defined(HALFWORD_ACCESS)
		int access_byte, total_byte = len;
		if (dout != NULL) {
			while (total_byte > 0) {
				FTSPI020_txfifo_empty();
				access_byte = min_t(total_byte, FTSPI020_txfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte >= 2) {
						FTSPI020_16BIT(SPI020_DATA_PORT) = *((uint16_t *) dout);
						dout += 2;
						access_byte -= 2;
					} else {
						FTSPI020_8BIT(SPI020_DATA_PORT) = *((uint8_t *) dout);
						dout += 1;
						access_byte -= 1;
					}
				}
			}
			//FTSPI020_check_tx_counter(len);
		} else if (din != NULL) {
			while (total_byte > 0) {
				FTSPI020_rxfifo_full();
				access_byte = min_t(total_byte, FTSPI020_rxfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					if (access_byte >= 2) {
						*((uint16_t *) din) = FTSPI020_16BIT(SPI020_DATA_PORT);
						din += 2;
						access_byte -= 2;
					} else {
						*((uint8_t *) din) = FTSPI020_8BIT(SPI020_DATA_PORT);
						din += 1;
						access_byte -= 1;
					}
				}
			}
		}
#else
		int access_byte, total_byte = len;
		if (dout != NULL) {
			while (total_byte > 0) {
				FTSPI020_txfifo_empty();
				access_byte = min_t(total_byte, FTSPI020_txfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					FTSPI020_8BIT(SPI020_DATA_PORT) = *((uint8_t *) dout);
					dout += 1;
					access_byte -= 1;
				}
			}
			//FTSPI020_check_tx_counter(len);
		} else if (din != NULL) {
			while (total_byte > 0) {
				FTSPI020_rxfifo_full();
				access_byte = min_t(total_byte, FTSPI020_rxfifo_depth());
				total_byte -= access_byte;
				while (access_byte > 0) {
					*((uint8_t *) din) = FTSPI020_8BIT(SPI020_DATA_PORT);
					din += 1;
					access_byte -= 1;
				}
			}
		}
#endif
#ifdef FTSPI020_USE_DMA
	}
#endif
}

void FTSPI020_Interrupt_Handler(void *data)
{
	int intr_status = FTSPI020_32BIT(INTR_STATUS_REG);

	if (intr_status & cmd_complete) {
		if (debug)
			prints(" ISR: Command Complete\n");

		ftspi020_cmd_complete = 1;
		FTSPI020_32BIT(INTR_STATUS_REG) |= cmd_complete;
	} else {
		prints(" ISR: No status\n");
	}
}
