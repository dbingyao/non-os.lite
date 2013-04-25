
#ifndef __CPE_AHB_DMA_H
#define __CPE_AHB_DMA_H

#include <common.h>

 /* registers */
#define DMA_INT			0x0
#define DMA_INT_TC		0x4
#define DMA_INT_TC_CLR		0x8
#define DMA_INT_ERRABT		0xC
#define DMA_INT_ERRABT_CLR	0x10
#define DMA_TC			0x14
#define DMA_ERRABT		0x18
#define DMA_CH_EN		0x1C
#define DMA_CH_BUSY		0x20
#define DMA_CSR			0x24
#define DMA_SYNC		0x28

#define DMA_C0_DevDtBase	0x40
#define DMA_C0_DevRegBase	0x80

#define DMA_CHANNEL_OFFSET	0x20
#define DMA_CHANNEL0_BASE	0x100
#define DMA_CHANNEL1_BASE	0x120
#define DMA_CHANNEL2_BASE	0x140
#define DMA_CHANNEL3_BASE	0x160
#define DMA_CHANNEL4_BASE	0x180
#define DMA_CHANNEL5_BASE	0x1a0
#define DMA_CHANNEL6_BASE	0x1c0
#define DMA_CHANNEL7_BASE	0x1e0

#define DMA_CHANNEL_CSR_OFFSET		0x0
#define DMA_CHANNEL_CFG_OFFSET		0x4
#define DMA_CHANNEL_SRCADDR_OFFSET	0x8
#define DMA_CHANNEL_DSTADDR_OFFSET	0xc
#define DMA_CHANNEL_LLP_OFFSET		0x10
#define DMA_CHANNEL_SIZE_OFFSET		0x14


/* bit mapping of main configuration status register(CSR) */
#define DMA_CSR_M1ENDIAN		0x00000004
#define DMA_CSR_M0ENDIAN		0x00000002
#define DMA_CSR_DMACEN			0x00000001

/* bit mapping of channel control register */
#define DMA_CSR_TC_MSK			0x80000000
#define DMA_CSR_CHPRJ_HIGHEST		0x00C00000
#define DMA_CSR_CHPRJ_2ND		0x00800000
#define DMA_CSR_CHPRJ_3RD		0x00400000
#define DMA_CSR_PRTO3			0x00200000
#define DMA_CSR_PRTO2			0x00100000
#define DMA_CSR_PRTO1			0x00080000
#define DMA_CSR_SRC_BURST_SIZE_1	0x00000000
#define DMA_CSR_SRC_BURST_SIZE_4	0x00010000
#define DMA_CSR_SRC_BURST_SIZE_8	0x00020000
#define DMA_CSR_SRC_BURST_SIZE_16	0x00030000
#define DMA_CSR_SRC_BURST_SIZE_32	0x00040000
#define DMA_CSR_SRC_BURST_SIZE_64	0x00050000
#define DMA_CSR_SRC_BURST_SIZE_128	0x00060000
#define DMA_CSR_SRC_BURST_SIZE_256	0x00070000

#define DMA_CSR_ABT			0x00008000
#define DMA_CSR_SRC_WIDTH_8		0x00000000
#define DMA_CSR_SRC_WIDTH_16		0x00000800
#define DMA_CSR_SRC_WIDTH_32		0x00001000

#define DMA_CSR_DST_WIDTH_8		0x00000000
#define DMA_CSR_DST_WIDTH_16		0x00000100
#define DMA_CSR_DST_WIDTH_32		0x00000200

#define DMA_CSR_MODE_NORMAL		0x00000000
#define DMA_CSR_MODE_HANDSHAKE		0x00000080

#define DMA_CSR_SRC_INCREMENT		0x00000000
#define DMA_CSR_SRC_DECREMENT		0x00000020
#define DMA_CSR_SRC_FIX			0x00000040

#define DMA_CSR_DST_INCREMENT		0x00000000
#define DMA_CSR_DST_DECREMENT		0x00000008
#define DMA_CSR_DST_FIX			0x00000010

#define DMA_CSR_SRC_SEL			0x00000004
#define DMA_CSR_DST_SEL			0x00000002
#define DMA_CSR_CH_ENABLE		0x00000001	

#define DMA_CSR_CHPR1			0x00C00000
#define DMA_CSR_SRC_SIZE		0x00070000
#define DMA_CSR_SRC_WIDTH		0x00003800
#define DMA_CSR_DST_WIDTH		0x00000700	
#define DMA_CSR_SRCAD_CTL		0x00000060
#define DMA_CSR_DSTAD_CTL		0x00000018


#define DMA_MAX_SIZE			0x10000
#define DAM_CHANNEL_NUMBER		8

/* bit mapping of channel configuration register */
#define DMA_CFG_INT_ERR_MSK_Disable	0x00000000
#define DMA_CFG_INT_TC_MSK_Disable	0x00000000

/* bit mapping of Linked List Control Descriptor */
#define DMA_LLP_TC_MSK			0x10000000

#define DMA_LLP_SRC_WIDTH_8		0x00000000
#define DMA_LLP_SRC_WIDTH_16		0x02000000
#define DMA_LLP_SRC_WIDTH_32		0x04000000

#define DMA_LLP_DST_WIDTH_8		0x00000000
#define DMA_LLP_DST_WIDTH_16		0x00400000
#define DMA_LLP_DST_WIDTH_32		0x00800000

#define DMA_LLP_SRC_INCREMENT		0x00000000
#define DMA_LLP_SRC_DECREMENT		0x00100000
#define DMA_LLP_SRC_FIX			0x00200000

#define DMA_LLP_DST_INCREMENT		0x00000000
#define DMA_LLP_DST_DECREMENT		0x00040000
#define DMA_LLP_DST_FIX			0x00080000

#define DMA_LLP_SRC_SEL			0x00020000
#define DMA_LLP_DST_SEL			0x00010000

/////////////////////////// AHB DMA Define //////////////////////////////////
#define AHBDMA_Channel0		0
#define AHBDMA_Channel1		1
#define AHBDMA_Channel2		2	
#define AHBDMA_Channel3		3	
#define AHBDMA_Channel4		4		
#define AHBDMA_Channel5		5			
#define AHBDMA_Channel6		6				
#define AHBDMA_Channel7		7	

#define AHBDMA_CH_SD		0
#define AHBDMA_CH_I2S_TX	1
#define AHBDMA_CH_I2S_RX	2
#define AHBDMA_CH_ADC		3
#define AHBDMA_CH_IDE_TX	4
#define AHBDMA_CH_IDE_RX	5
#define AHBDMA_CH_SPI2_TX	6
#define AHBDMA_CH_SPI2_RX	7

#define AHBDMA_SrcWidth_Byte	0					
#define AHBDMA_SrcWidth_Word	1					
#define AHBDMA_SrcWidth_DWord	2		

#define AHBDMA_DstWidth_Byte	0					
#define AHBDMA_DstWidth_Word	1					
#define AHBDMA_DstWidth_DWord	2					

#define AHBDMA_Burst1		0					
#define AHBDMA_Burst4		1					
#define AHBDMA_Burst8		2					
#define AHBDMA_Burst16		3					
#define AHBDMA_Burst32		4					
#define AHBDMA_Burst64		5					
#define AHBDMA_Burst128		6					
#define AHBDMA_Burst256		7					

#define AHBDMA_NormalMode	0					
#define AHBDMA_HwHandShakeMode	1					

#define AHBDMA_SrcInc		0					
#define AHBDMA_SrcDec		1					
#define AHBDMA_SrcFix		2	

#define AHBDMA_DstInc		0					
#define AHBDMA_DstDec		1					
#define AHBDMA_DstFix		2	

#define AHBDMA_PriorityLow	0					
#define AHBDMA_Priority3rd	1					
#define AHBDMA_Priority2nd	2	
#define AHBDMA_PriorityHigh	3	

////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint enable:1;
	uint dst_sel:1;
	uint src_sel:1;
	uint dst_ctrl:2;
	uint src_ctrl:2;
	uint mode:1;
	uint dst_width:3;		
	uint src_width:3;
	uint reserved1:1;
	uint abt:1;
	uint src_size:3;
	uint prot:3;
	uint priority:2;
//	uint reserved0:7;		//FIE7020
	uint ff_th:3;			//FIE7021
	uint reserved0:4;		//FIE7021
	uint tc_msk:1;	
}fLib_DMA_CH_CSR_t;

typedef struct
{
	uint int_tc_msk:1;
	uint int_err_msk:1;
	uint int_abt_msk:1;		
	uint src_rs:4;	
	uint src_he:1;
	uint busy:1;
	uint dst_rs:4;
	uint dst_he:1;
	uint reserved1:2;
	uint llp_cnt:4;	
	uint reserved2:12;
}fLib_DMA_CH_CFG_t;
	
typedef struct
{
#if defined PLATFORM_AHB
	uint reserved:16;
#elif defined PLATFORM_A320
	uint size:12;
	uint reserved:4;
#else           
#error  "Specify correct platform name."
#endif
	uint dst_sel:1;
	uint src_sel:1;
	uint dst_ctrl:2;
	uint src_ctrl:2;
	uint dst_width:3;
	uint src_width:3;
	uint tc_msk:1;
	uint ff_th:3;
}fLib_DMA_LLP_CTRL_t;

typedef struct
{
	fLib_DMA_CH_CSR_t csr;
	fLib_DMA_CH_CFG_t cfg;
	uint src_addr;
	uint dst_addr;
	uint link_list_addr;
	uint size;
	uint dummy[2];
}fLib_DMA_CH_t;

typedef struct
{
	uint src_addr;
	uint dst_addr;
	uint link_list_addr;
	fLib_DMA_LLP_CTRL_t llp_ctrl;
#if defined PLATFORM_AHB
	uint size;			//FIE7021
#endif
}fLib_DMA_LLD_t;


typedef struct
{
	uint dma_int;
	uint dma_int_tc;
	uint dma_int_tc_clr;
	uint dma_int_err;
	uint dma_int_err_clr;
	uint dma_tc;
	uint dma_err;
	uint dma_ch_enable;
	uint dma_ch_busy;
	uint dma_csr;
	uint dma_sync;
	uint dummy0[5];
	uint dma_ch_dev_dt_base[8];
	
	uint dummy1[8];
	
	uint dma_ch_dev_reg_base[8];
	
	uint dummy2[24];
	
	fLib_DMA_CH_t dma_ch[8];
}fLib_DMA_Reg_t;

extern volatile fLib_DMA_Reg_t *DMAReg;

/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */
 
extern int    fLib_IsDMAChannelBusy(uint Channel);
extern int    fLib_IsDMAChannelEnable(uint Channel);
extern uint fLib_GetDMAIntStatus(void);
extern uint fLib_GetDMAChannelIntStatus(uint Channel);
extern int    fLib_GetDMABusyStatus(void);
extern int    fLib_GetDMAEnableStatus(void);

extern void   fLib_InitDMA(uint M0_BigEndian, uint M1_BigEndian, uint Sync);
extern void   fLib_EnableDMAChannel(uint Channel);

extern void   fLib_DisableDMAChannel(uint Channel);  // add by jerry

extern void   fLib_ClearDMAChannelIntStatus(uint Channel);

extern void   fLib_SetDMAChannelCfg(uint Channel, fLib_DMA_CH_CSR_t Csr);
extern void   fLib_SetDMAChannelCnCfg(uint Channel, fLib_DMA_CH_CFG_t CnCfg);
extern fLib_DMA_CH_CSR_t fLib_GetDMAChannelCfg(uint Channel);
extern void   fLib_DMA_CHIntMask(uint Channel, fLib_DMA_CH_CFG_t Mask);
extern void   fLib_DMA_CHLinkList(uint Channel, uint link_list_addr);
extern void   fLib_DMA_CHDataCtrl(uint Channel, uint SrcAddr, uint DstAddr, uint Size);

extern void fLib_DMA_SetInterrupt(uint channel, uint tcintr, uint errintr, uint abtintr);
extern void fLib_DMA_ResetChannel(unchar channel);
extern void fLib_DMA_ClearAllInterrupt(void);

#endif
