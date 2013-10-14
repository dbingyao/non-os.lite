/**
 * -------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp.  All rights reserved.
 * -------------------------------------------------------------------------
 * FILENAME:  ftspi020_cntr.h
 * DEPARTMENT :CTD/SD/SD1
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	  DESCRIPTION
 * 2010/07/07   Mike          
 * 2010/08/13   BingJiun	 Scan all CEs for available flash.         
 * -------------------------------------------------------------------------
 */
#ifndef FTSPI020_CNTR_H
#define FTSPI020_CNTR_H

#define SPI_XFER_BEGIN			0x00000000
#define SPI_XFER_END			0x00000001
#define SPI_XFER_CMD_STATE		0x00000002
#define SPI_XFER_DATA_STATE		0x00000004
#define SPI_XFER_CHECK_CMD_COMPLETE	0x00000008

// ********************* external variable *********************
extern int g_spi020_wr_buf_addr;
extern int g_spi020_wr_buf_length;
extern int g_spi020_rd_buf_addr;
extern int g_spi020_rd_buf_length;
// *********************** external func ***********************
extern int FTSPI020_init(void);
extern int FTSPI020_support_dtr_mode(void);
extern struct spi_flash *FTSPI020_probe(uint32_t ce);
extern int spi_flash_cmd(struct spi_flash *slave, uint8_t * uint8_t_cmd, void *response, size_t len);
extern int spi_flash_cmd_write(struct spi_flash *slave, uint8_t * uint8_t_cmd, const void *data, int data_len);
extern int spi_flash_cmd_read(struct spi_flash *slave, uint8_t * uint8_t_cmd, void *data, int data_len);

extern void FTSPI020_flush_cmd_queue(void);
extern void FTSPI020_show_status(void);
extern void FTSPI020_read_status(uint8_t * status);
extern void FTSPI020_show_content(void *data_buf, int len);
extern uint8_t FTSPI020_compare(uint8_t * data1_buf, uint8_t * data2_buf, uint32_t len);
extern uint8_t FTSPI020_issue_cmd(struct ftspi020_cmd *command);
extern int FTSPI020_wait_cmd_complete(uint32_t wait_ms);
extern int FTSPI020_data_access(uint8_t * dout, uint8_t * din, uint32_t len);
extern void FTSPI020_Interrupt_Handler(void * data);
// ************************* local func *************************
void FTSPI020_dma_enable(uint8_t enable);
void FTSPI020_cmd_complete_intr_enable(uint8_t enable);
void FTSPI020_reset_hw(void);
void FTSPI020_operate_mode(uint8_t mode);
void FTSPI020_busy_location(uint8_t location);
void FTSPI020_divider(uint8_t divider);
int FTSPI020_rxfifo_full(void);
int FTSPI020_txfifo_empty(void);
int FTSPI020_txfifo_depth(void);
int FTSPI020_rxfifo_depth(void);
void FTSPI020_check_rx_counter(int len);
void FTSPI020_check_tx_counter(int len);

#endif
