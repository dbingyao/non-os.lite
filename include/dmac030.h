
#ifndef __AHB_DMA030_H
#define __AHB_DMA030_H

#include <platform.h>

#define DMA_INT_POLLING

#define DMA030_BASE		FTDMAC030_REG_BASE
#define DMA030_LDM_BASE		0xB0200000
#define IRQ_DMA030		11

/* registers */
#define DMA_INT			0x0
#define DMA_INT_TC		0x4
#define DMA_INT_TC_CLR		0x8
#define DMA_INT_ERRABT		0xC
#define DMA_INT_ERRABT_CLR	0x10
#define DMA_TC			0x14
#define DMA_ERRABT		0x18
#define DMA_CH_EN		0x1C
#define DMA_CH_SYNC		0x20
#define DMA_LDM_BASE		0x24
#define DMA_WDT			0x28
#define DMA_GE			0x2C
#define DMA_APB_SLAVE_ERR	0x30 
#define DMA_FEATURE		0x38  // new
//#define DMA_C0_DevDtBase	0x40  // new
//#define DMA_C0_DevRegBase	0x80  // new
#define DMA_ENDIAN		0x4c
#define DMA_WRITE_ONLY		0x50 

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
#define DMA_CHANNEL_STRIDE_OFFSET	0x18

/* bit mapping of channel control register 0x100 */
#define DMA_CSR_SRCTCNY_1		0x0
#define DMA_CSR_SRCTCNY_2		BIT29
#define DMA_CSR_SRCTCNY_4		BIT30
#define DMA_CSR_SRCTCNY_8		BIT30 | BIT29
#define DMA_CSR_SRCTCNY_16		BIT31
#define DMA_CSR_SRCTCNY_32		BIT31 | BIT29
#define DMA_CSR_SRCTCNY_64		BIT31 | BIT30
#define DMA_CSR_SRCTCNY_128		BIT31 | BIT30 | BIT29

#define DMA_CSR_SRC_WIDTH_8		0x0
#define DMA_CSR_SRC_WIDTH_16		BIT25
#define DMA_CSR_SRC_WIDTH_32		BIT26
#define DMA_CSR_SRC_WIDTH_64		BIT25 |  BIT26
#define DMA_CSR_SRC_WIDTH_128		BIT27

#define DMA_CSR_DST_WIDTH_8		0x0
#define DMA_CSR_DST_WIDTH_16		BIT22
#define DMA_CSR_DST_WIDTH_32		BIT23
#define DMA_CSR_DST_WIDTH_64		BIT22 | BIT23
#define DMA_CSR_DST_WIDTH_128		BIT24

#define DMA_CSR_SRCCTRL_Fix		BIT20
#define DMA_CSR_DSTCTRL_Fix		BIT18
#define DMA_CSR_WDT_EN			BIT17
#define DMA_CSR_CH_EN			BIT16
#define DMA_CSR_EXP_EN			BIT15
#define DMA_CSR_2D_EN			BIT14
#define DMA_CSR_WEVENT_EN		BIT13
#define DMA_CSR_SEVENT_EN		BIT12
#define DMA_CSR_WSYNC			BIT8
/*END  bit mapping of channel control register 0x100 */

/* bit mapping of channel control register 0x104 */
#define DMA_CSR_UNALIGN			BIT31
#define DMA_CSR_WRITE_ONLY		BIT30
#define DMA_CSR_CHPRI_HI		BIT28
/* hardware feature register*/
#define DMA_FEATURE_UNALIGN		BIT3

/*END  bit mapping of channel control register 0x104 */
#define DMA_CSR_CHPRJ_HIGHEST		0x00C00000
#define DMA_CSR_CHPRJ_2ND		0x00800000
#define DMA_CSR_CHPRJ_3RD		0x00400000
#define DMA_CSR_PRTO3			0x00200000
#define DMA_CSR_PRTO2			0x00100000
#define DMA_CSR_PRTO1			0x00080000

#define DMA_CSR_ABT			0x00008000
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

#define DMA_CSR_DMA_FF_TH		0x07000000   //new
#define DMA_CSR_CHPR1			0x00C00000
#define DMA_CSR_SRC_SIZE		0x00070000
#define DMA_CSR_SRC_WIDTH		0x00003800
#define DMA_CSR_DST_WIDTH		0x00000700
#define DMA_CSR_SRCAD_CTL		0x00000060
#define DMA_CSR_DSTAD_CTL		0x00000018

/* bit mapping of channel configuration register 0x104 */
#define DMA_CFG_INT_ABT_MSK	        0x00000004   // new
#define DMA_CFG_INT_ERR_MSK	        0x00000002   // new
#define DMA_CFG_INT_TC_MSK	        0x00000001   // new

///////////////////////////////////////////////////
/* bit mapping of Linked List Control Descriptor */

#define DMA_LLP_DMA_FF_TH		0xE0000000   // new

#define DMA_LLP_TC_MSK			0x10000000

#define DMA_LLP_SRC_WIDTH_8		0x00000000
#define DMA_LLP_SRC_WIDTH_16		0x02000000
#define DMA_LLP_SRC_WIDTH_32		0x04000000
#define DMA_LLP_SRC_WIDTH_64		0x06000000   // new

#define DMA_LLP_DST_WIDTH_8		0x00000000
#define DMA_LLP_DST_WIDTH_16		0x00400000
#define DMA_LLP_DST_WIDTH_32		0x00800000
#define DMA_LLP_DST_WIDTH_64		0x00C00000   // new

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
#define AHBDMA_SrcFix		2

#define AHBDMA_DstInc		0
#define AHBDMA_DstFix		2

#define AHBDMA_PriorityLow	0
//#define AHBDMA_Priority3rd	1
//#define AHBDMA_Priority2nd	2
#define AHBDMA_PriorityHigh	1


// feature register
typedef struct
{
	uint32_t ch_num		:3;
	uint32_t reserved3	:1;
	uint32_t d_width	:2;
	uint32_t reserved6	:2;
	uint32_t dfifo_depth	:3;
	uint32_t reserved11	:1;
	uint32_t pri_on		:1;
	uint32_t reserved13	:3;
	uint32_t pri_num	:4;
	uint32_t ldm_on		:1;
	uint32_t reserved21	:3;
	uint32_t ldm_depth	:2;
	uint32_t reserved26	:2;
	uint32_t cmd_depth	:2;
	uint32_t reserved30	:2;
} DMA_FEATURE_t;	

typedef struct
{
	uint32_t ch_wevent:8;
	uint32_t wsync:1;
	uint32_t ch_sevent:3;
	uint32_t sevent_en:1;
	uint32_t wevent_en:1;
	uint32_t twod_en:1;
	uint32_t exp_en:1;
	uint32_t ch_en:1;
	uint32_t wdt_en:1;
	uint32_t dst_ctrl:2;
	uint32_t src_ctrl:2;
	uint32_t dst_width:3;
	uint32_t src_width:3;
	uint32_t tc_msk:1;
	uint32_t src_tcnt:3;
}DMA_CH_CSR_t;//DMAC030

typedef struct
{
	uint32_t int_tc_msk:1;
	uint32_t int_err_msk:1;
	uint32_t int_abt_msk:1;   
	uint32_t src_rs:4;	
	uint32_t src_hen:1;
	uint32_t reserved:1;
	uint32_t dst_rs:4;
	uint32_t dst_hen:1;
	uint32_t reserved1:2;
	uint32_t llp_cnt:4;
	uint32_t ch_gntwin:8;
	uint32_t ch_pri:1;
	uint32_t reserved29:1;
	uint32_t wo_mode:1;
	uint32_t Unalign_Mode:1;
}DMA_CH_CFG_t;//DMAC030

typedef struct
{
	uint32_t link_list_addr:32;
}DMA_CH_LLP_t;

typedef struct
{
	uint32_t ch_wevent:8;
	uint32_t wsync:1;
	uint32_t ch_sevent:3;
	uint32_t sevent_en:1;
	uint32_t wevent_en:1;
	uint32_t twod_en:1;
	uint32_t exp_en:1;
	uint32_t ch_en:1;
	uint32_t wdt_en:1;
	uint32_t dst_ctrl:2;
	uint32_t src_ctrl:2;
	uint32_t dst_width:3;
	uint32_t src_width:3;
	uint32_t tc_msk:1;
	uint32_t src_tcnt:3;		//new
}DMA_LLP_CTRL_t;

typedef struct
{
	volatile DMA_CH_CSR_t csr;
	volatile DMA_CH_CFG_t cfg;
	volatile uint32_t src_addr;
	volatile uint32_t dst_addr;
	volatile DMA_CH_LLP_t llp;
	volatile uint32_t size;		//the unit is SrcWidth
	volatile uint32_t stride;
	volatile uint32_t dummy;
}DMA_CH_t;

typedef struct
{
	uint32_t src_addr;
	uint32_t dst_addr;
	DMA_CH_LLP_t llp;
	DMA_LLP_CTRL_t llp_ctrl;
	uint32_t TotalSize;		//new, the unit is SrcWidth, only 21 bits is used
	uint32_t Stride;
	uint32_t Dummy[2];		//this is for program to look nice.
}DMA_LLD_t;

typedef struct
{
	volatile uint32_t dma_int;
	volatile uint32_t dma_int_tc;
	volatile uint32_t dma_int_tc_clr;
	volatile uint32_t dma_int_err;
	volatile uint32_t dma_int_err_clr;
	volatile uint32_t dma_tc;
	volatile uint32_t dma_err;
	volatile uint32_t dma_ch_enable;
	volatile uint32_t dma_sync_pi;
	volatile uint32_t dma_ldm;
	volatile uint32_t dma_wdt;
	volatile uint32_t dma_ge;
	volatile uint32_t dma_plverr;
	volatile uint32_t dma_rn;
	volatile uint32_t dma_hf;
	volatile uint32_t dma_ldm_flag[4];
	volatile uint32_t dma_big_endian;
	volatile uint32_t dma_writeonly;

	volatile uint32_t dummy1[43];

	volatile DMA_CH_t dma_ch[8];
}DMA_Reg_st;


/*  -------------------------------------------------------------------------------
 *   API
 *  -------------------------------------------------------------------------------
 */

extern int IsDMAChannelEnable(uint32_t Channel);
extern uint32_t GetDMAIntStatus(void);
extern uint32_t GetDMAChannelIntStatus(uint32_t Channel);
extern int GetDMABusyStatus(void);
extern int GetDMAEnableStatus(void);

extern void InitDMA(void);
extern void EnableDMAChannel(uint32_t Channel);
extern void DisableDMAChannel(uint32_t Channel);  
extern void EnableDMAChannelEndianConverter(uint32_t Channel);
extern void DisableDMAChannelEndianConverter(uint32_t Channel);

extern void ClearDMAChannelIntStatus(uint32_t Channel);

extern void SetDMAChannelCfg(uint32_t Channel, DMA_CH_CSR_t Csr);
extern DMA_CH_CSR_t GetDMAChannelCfg(uint32_t Channel);
extern void DMA_CHIntMask(uint32_t Channel, DMA_CH_CFG_t Mask);
extern void DMA_CHLinkList(uint32_t Channel, DMA_CH_LLP_t LLP);
extern void DMA_CHDataCtrl(uint32_t Channel, uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size);
extern void DMA_CHDataCtrl_2D(uint32_t Channel, uint32_t SrcAddr, uint32_t DstAddr, uint32_t XTcnt, 
			      uint32_t YTcnt, uint32_t DstStride, uint32_t SrcStride);
extern void EnableDMAChannelUnalign(uint32_t Channel);

extern void DMA_SetInterrupt(uint32_t channel, uint32_t tcintr, uint32_t errintr, uint32_t abtintr);
extern void DMA_ResetChannel(uint8_t channel);
extern void DMA_ClearAllInterrupt(void);
extern void DMA_SetWriteOnlyValue(uint32_t Value);

uint32_t DMA_WaitIntStatus(uint32_t Channel);
int DMA_ISChannelEnable(uint32_t Channel);
int DMATest_new(uint32_t mode);
int CompareMemory(uint32_t *SrcAddr, uint32_t *DstAddr, uint32_t Size);
int CheckContent(uint32_t *DstAddr, uint32_t Size);

extern void DMA_SetCHCsr(uint32_t Channel, DMA_CH_CSR_t Csr);
extern void DMA_SetCHCfg(uint32_t Channel, DMA_CH_CFG_t Cfg);
#endif
