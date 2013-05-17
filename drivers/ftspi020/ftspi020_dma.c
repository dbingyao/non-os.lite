/**
 * -----------------------------------------------------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2006-2008.  All rights reserved.
 * ------------------------------------------------------------------------------------------------------------------------
 * FILENAME:  ftspi020_dma.c
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:0.1
 * -------------------------------------------------------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2008/07/22    B.J. Luo         API to program DMA Controller. 
 * -------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <dmac020.h>

#include "ftspi020.h"

#ifdef FTSPI020_USE_DMA

#define FTSPI020_REQ_SEL	4

extern clock_t t0_perf;

int FTSPI020_DMA_LLD_STRUCT_ADDR;
static uint32_t ftspi020_data_getDmacMaxSize(uint32_t src_width, uint32_t busrt_size);

/**
 * Attention : If use DMA Controller on A320 board, the burst size must
 * be multiple of sector size. Also check Tx and Rx FIFO threshold.
 * Example : Sector Size 512 bytes, then burst size could be 16.
 * 			 Sector Size 520 bytes, then burst size could be 1.          
 */

static uint32_t ftspi020_data_getDmacMaxSize(uint32_t src_width, uint32_t busrt_size)
{
	uint32_t sz;

	if (busrt_size == 0)
		sz = (4096 - ((1) * (1 << src_width)));
	else
		sz = (4096 - ((1 << (busrt_size + 1)) * (1 << src_width)));

	return sz;

}

/*************************************************************************************
	DMA_NoLLD means there is no Link List Descriptor,
	the size that DMA will work on must smaller than one LLP can move. This function
	only used to configure the registers, not yet starts the DMA. You have to use
	fLib_EnableDMAChannel() to start the DMA. And you can use DMA_WaitIntStatus() to
	wait until fihish.
**************************************************************************************/
static int ftspi020_DMA_NoLLD(uint32_t Channel,	// use which channel for AHB DMA, 0..7
			      uint32_t SrcAddr,	// source begin address
			      uint32_t DstAddr,	// dest begin address
			      uint32_t Size,	// total bytes
			      uint32_t SrcWidth,	// source width 8/16/32 bits -> 0/1/2
			      uint32_t DstWidth,	// dest width 8/16/32 bits -> 0/1/2
			      uint32_t SrcSize,	// source burst size, How many "SrcWidth" will be transmmited at one times ?
			      uint32_t SrcCtrl,	// source address change : Inc/dec/fixed --> 0/1/2
			      uint32_t DstCtrl,	// dest address change : Inc/dec/fixed --> 0/1/2
			      uint32_t Priority)	// priority for this chaanel 0(low)/1/2/3(high)
{
	fLib_DMA_CH_t DMAChannel;

#if 0
	printf("\nCh=%d, SrcAddr=%0.8X, DstAddr=%0.8X, Size=%0.8X\n"
	       "SrcWidth=%d bits, DstWidth=%d bits, SrcSize=%d bytes\n"
	       "SrcCtrl=%s, DstCtrl=%s, Priority=%d, Mode=%s\n",
	       Channel, SrcAddr, DstAddr, Size, 1 << (SrcWidth + 3),
	       1 << (DstWidth + 3), ((SrcSize == 0) ? 1 : 1 << (SrcSize + 1)),
	       ((SrcCtrl ==
		 0) ? "Increment" : ((SrcCtrl == 1) ? "Decrement" : "Fix")),
	       ((DstCtrl == 0) ? "Increment" : ((DstCtrl == 1) ? "Decrement" : "Fix")), Priority,
	       ((Mode == 0) ? "Normal" : "Hardware Handshake"));
#endif

	memset (&DMAChannel, 0, sizeof(fLib_DMA_CH_t));

	/* program channel CSR */
	DMAChannel.csr.enable = 0;	/* not yet enable */
	DMAChannel.csr.tc_msk = 0;	/* no linked list, no mask, there will be interrupt */
	DMAChannel.csr.dst_sel = 0;	/* destination AHB master id */
	DMAChannel.csr.src_sel = 0;	/* source AHB master id */
	DMAChannel.csr.reserved0 = 0;
	DMAChannel.csr.priority = Priority;	/* priority */
	DMAChannel.csr.prot = 0;	/* PROT 1-3 bits */
	DMAChannel.csr.src_size = SrcSize;	/* source burst size */
	DMAChannel.csr.abt = 0;	/* NOT transaction abort */
	DMAChannel.csr.reserved1 = 0;
	DMAChannel.csr.src_width = SrcWidth;	/* source transfer size */
	DMAChannel.csr.dst_width = DstWidth;	/* destination transfer size */
	DMAChannel.csr.mode = 1;	/* Normal mode or Hardware handshake mode */
	DMAChannel.csr.src_ctrl = SrcCtrl;	/* source increment, decrement or fix */
	DMAChannel.csr.dst_ctrl = DstCtrl;	/* destination increment, decrement or fix */

	fLib_SetDMAChannelCfg(Channel, DMAChannel.csr);

	/* program channel CFG */
	DMAChannel.cfg.int_tc_msk = 1;	// Disable terminal count interrupt, if tc then interrupt
	DMAChannel.cfg.int_err_msk = 1;	// Disable error interrupt
	DMAChannel.cfg.int_abt_msk = 1;	// Disable abort interrupt => modify by Jerry
	DMAChannel.cfg.busy = 0;	//busy bit is RO
	DMAChannel.cfg.reserved1 = 0;	//reserved1 bits are RO
	DMAChannel.cfg.llp_cnt = 0;	//llp_cnt bits are RO
	DMAChannel.cfg.reserved2 = 0;	//reserved2 bits are RO

#if defined PLATFORM_AHB
	if (SrcCtrl == 2) {
		DMAChannel.cfg.src_he = 1;
		DMAChannel.cfg.src_rs = FTSPI020_REQ_SEL;
	}
        
        if (DstCtrl == 2){
		DMAChannel.cfg.dst_he = 1;
		DMAChannel.cfg.dst_rs = FTSPI020_REQ_SEL;
	}
#endif
	fLib_SetDMAChannelCnCfg(Channel, DMAChannel.cfg);

	/* program channel llp */
	DMAChannel.link_list_addr = 0;	//no LLP, this is to set link_list_addr = 0
	fLib_DMA_CHLinkList(Channel, DMAChannel.link_list_addr);

	/* porgram address and size */
	fLib_DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, Size / (1 << SrcWidth));

	DMAChannel.link_list_addr = 1;	//this is to set master_id = 1, and link_list_addr = 0

	return 0;
}

static void ftspi020_DMA_FillMemLLD(uint32_t LLPSize, 
				    uint32_t LLPCount,	// total link-list node
				    uint32_t SrcAddr,	// source begin address
				    uint32_t DstAddr,	// dest begin address
				    uint32_t Size,		// total bytes
				    uint32_t SrcWidth,	// source width 8/16/32 bits -> 0/1/2
				    uint32_t DstWidth,	// dest width 8/16/32 bits -> 0/1/2
				    uint32_t SrcCtrl,	// source address change : Inc/dec/fixed --> 0/1/2
				    uint32_t DstCtrl)	// dest address change : Inc/dec/fixed --> 0/1/2                                  
{
	uint32_t i;
	fLib_DMA_LLD_t *LLP = (fLib_DMA_LLD_t *) FTSPI020_DMA_LLD_STRUCT_ADDR;

	for (i = 0; i < LLPCount; i++) {
		if (SrcCtrl == AHBDMA_SrcInc)	// increase
			LLP[i].src_addr = (uint32_t) SrcAddr + ((i + 1) * LLPSize * (1 << SrcWidth));
		else if (SrcCtrl == AHBDMA_SrcDec)	// decrease
			LLP[i].src_addr = (uint32_t) SrcAddr - ((i + 1) * LLPSize * (1 << SrcWidth));
		else if (SrcCtrl == AHBDMA_SrcFix)	// fixed
			LLP[i].src_addr = (uint32_t) SrcAddr;

		if (DstCtrl == AHBDMA_DstInc)
			LLP[i].dst_addr = (uint32_t) DstAddr + ((i + 1) * LLPSize * (1 << SrcWidth));
		else if (DstCtrl == AHBDMA_DstDec)	// Decrease
			LLP[i].dst_addr = (uint32_t) DstAddr - ((i + 1) * LLPSize * (1 << SrcWidth));
		else if (DstCtrl == AHBDMA_DstFix)
			LLP[i].dst_addr = (uint32_t) DstAddr;

		*((uint32_t *) & (LLP[i].llp_ctrl)) = 0;	//init llp_ctrl as 0
		LLP[i].llp_ctrl.dst_sel = 0;	/* destination AHB master id */
		LLP[i].llp_ctrl.src_sel = 0;	/* source AHB master id */
		LLP[i].llp_ctrl.dst_ctrl = DstCtrl;	/* destination increment, decrement or fix */
		LLP[i].llp_ctrl.src_ctrl = SrcCtrl;	/* source increment, decrement or fix */
		LLP[i].llp_ctrl.dst_width = DstWidth;	/* destination transfer size */
		LLP[i].llp_ctrl.src_width = SrcWidth;	/* source transfer size */

		LLP[i].link_list_addr = 0;
		if (i == (LLPCount - 1)) {
			//the last LLP
			LLP[i].link_list_addr = 0;
			LLP[i].llp_ctrl.tc_msk = 0;	// Enable tc status
#if defined PLATFORM_AHB
			LLP[i].size = (Size - LLPSize * (LLPCount) * (1 << SrcWidth)) / (1 << SrcWidth);
#elif defined PLATFORM_A320
			LLP[i].llp_ctrl.size = (Size - LLPSize * (LLPCount) * (1 << SrcWidth)) / (1 << SrcWidth);
#else
#error  "Specify correct platform name."
#endif
		} else {
			LLP[i].link_list_addr = ((uint32_t) & LLP[i + 1]);
			LLP[i].llp_ctrl.tc_msk = 1;
#if defined PLATFORM_AHB
			LLP[i].size = LLPSize;	//the unit is SrcWidth  / (1 << SrcWidth);
#elif defined PLATFORM_A320
			LLP[i].llp_ctrl.size = LLPSize;	//the unit is SrcWidth  / (1 << SrcWidth);
#else
#error  "Specify correct platform name."
#endif
		}
	}

	return;
}

static int ftspi020_DMA_NormalMode(uint32_t Channel,	// use which channel for AHB DMA, 0..7
				   uint32_t LLPSize, 
				   uint32_t LLPCount,// total link-list node, if NO link list is needed, LLPCount is 0
				   uint32_t SrcAddr,	// source begin address
				   uint32_t DstAddr,	// dest begin address
				   uint32_t Size,	// total bytes
				   uint32_t SrcWidth,	// source width 8/16/32 bits -> 0/1/2
				   uint32_t DstWidth,	// dest width 8/16/32 bits -> 0/1/2
				   uint32_t SrcSize,	// source burst size, How many "SrcWidth" will be transmmited at one times ?
				   uint32_t SrcCtrl,	// source address change : Inc/dec/fixed --> 0/1/2
				   uint32_t DstCtrl,	// dest address change : Inc/dec/fixed --> 0/1/2
				   uint32_t Priority)// priority for this chaanel 0(low)/1/2/3(high)
{
	fLib_DMA_CH_t DMAChannel;

/*
    printf("\nCh%d, Src=%08X, Dst=%08X, Size=%08X SrcWidth=%db, DstWidth=%db, SrcSize=%dB\n"
            "SrcCtrl=%s, DstCtrl=%s, Priority=%d, Mode=%s, LLPCnt = %d\n",
            Channel, SrcAddr, DstAddr, Size, 1 << (SrcWidth + 3), 1 << (DstWidth + 3),
            ((SrcSize == 0) ? 1 : 1 << (SrcSize+1)),
            ((SrcCtrl == 0) ? "Inc" : ((SrcCtrl == 1) ? "Dec" : "Fix")),
            ((DstCtrl == 0) ? "Inc" : ((DstCtrl == 1) ? "Dec" : "Fix")),
            Priority, ((Mode == 0) ? "Normal" : "HW"), LLPCount);
*/

	memset (&DMAChannel, 0, sizeof(fLib_DMA_CH_t));
	/* program channel */

	if (LLPCount >= 1) {
		ftspi020_DMA_FillMemLLD(LLPSize, LLPCount, SrcAddr, DstAddr, Size, SrcWidth, DstWidth, SrcCtrl, DstCtrl);
		/* program channel CSR */
		DMAChannel.csr.enable = 0;	/* not yet enable */
		DMAChannel.csr.dst_sel = 0;	/* destination AHB master id */
		DMAChannel.csr.src_sel = 0;	/* source AHB master id */
		DMAChannel.csr.dst_ctrl = DstCtrl;	/* destination increment, decrement or fix */
		DMAChannel.csr.src_ctrl = SrcCtrl;	/* source increment, decrement or fix */
		DMAChannel.csr.mode = 1;	/* Normal mode or Hardware handshake mode */
		DMAChannel.csr.dst_width = DstWidth;	/* destination transfer size */
		DMAChannel.csr.src_width = SrcWidth;	/* source transfer size */
		DMAChannel.csr.reserved1 = 0;
		DMAChannel.csr.abt = 0;	/* NOT transaction abort */
		DMAChannel.csr.src_size = SrcSize;	/* source burst size */
		DMAChannel.csr.prot = 0;	/* protect bits */
		DMAChannel.csr.priority = Priority;	/* priority */
		DMAChannel.csr.reserved0 = 0;
		DMAChannel.csr.tc_msk = 1;	/* disable terminal count */

		fLib_SetDMAChannelCfg(Channel, DMAChannel.csr);

		/* program channel CFG */
		DMAChannel.cfg.int_tc_msk = 1;	// Disable tc status
		DMAChannel.cfg.int_err_msk = 1;
		DMAChannel.cfg.int_abt_msk = 1;

		DMAChannel.cfg.busy = 0;	//busy bit is RO
		DMAChannel.cfg.reserved1 = 0;	//reserved1 bits are RO
		DMAChannel.cfg.llp_cnt = 0;	//llp_cnt bits are RO
		DMAChannel.cfg.reserved2 = 0;	//reserved2 bits are RO

#if defined PLATFORM_AHB
		if (SrcCtrl == 2) {
			DMAChannel.cfg.src_he = 1;
			DMAChannel.cfg.src_rs = FTSPI020_REQ_SEL;
		}
		
		if (DstCtrl == 2){
			DMAChannel.cfg.dst_he = 1;
			DMAChannel.cfg.dst_rs = FTSPI020_REQ_SEL;
		}
#endif
		fLib_SetDMAChannelCnCfg(Channel, DMAChannel.cfg);

		/* program channel llp */
		DMAChannel.link_list_addr = 0;	//Init IP llp
		DMAChannel.link_list_addr = FTSPI020_DMA_LLD_STRUCT_ADDR;
		fLib_DMA_CHLinkList(Channel, DMAChannel.link_list_addr);

		/* porgram address and size */
		fLib_DMA_CHDataCtrl(Channel, SrcAddr, DstAddr, LLPSize);

	} else {		//for LLPCount==0,
		return ftspi020_DMA_NoLLD(Channel, SrcAddr, DstAddr, Size, SrcWidth, DstWidth, SrcSize, SrcCtrl,
				       DstCtrl, Priority);
	}

	return 0;
}

int ftspi020_Start_DMA(uint32_t Channel,	// use which channel for AHB DMA, 0..7
		       uint32_t SrcAddr,	// source begin address
		       uint32_t DstAddr,	// dest begin address
		       uint32_t Size,	// total bytes
		       uint32_t SrcWidth,	// source width 8/16/32 bits -> 0/1/2
		       uint32_t DstWidth,	// dest width 8/16/32 bits -> 0/1/2
		       uint32_t SrcSize,	// source burst size, How many "SrcWidth" will be transmmited at one times ?
		       uint32_t SrcCtrl,	// source address change : Inc/dec/fixed --> 0/1/2
		       uint32_t DstCtrl,	// dest address change : Inc/dec/fixed --> 0/1/2
		       uint32_t Priority)	// priority for this chaanel 0(low)/1/2/3(high)
{
	uint32_t LLPSize, Count, intrStatus;

	fLib_DisableDMAChannel(Channel);

	LLPSize = ftspi020_data_getDmacMaxSize(SrcWidth, SrcSize);
	if ((Size / (1 << SrcWidth)) > LLPSize) { //the Size is too large to move in one LLP in the IP
		Count = ((Size + (LLPSize - 1) * (1 << SrcWidth)) / (1 << SrcWidth)) / LLPSize;
	} else {
		Count = 1;
	}

	if (ftspi020_DMA_NormalMode
	    (Channel, LLPSize, Count - 1, SrcAddr, DstAddr, Size, SrcWidth, DstWidth, SrcSize, SrcCtrl,
	     DstCtrl, Priority)) {
		return 1;
	}

	t0_perf = get_timer(0);
	fLib_EnableDMAChannel(Channel);

	intrStatus = 0;
	do {
		if (DMAReg->dma_tc & (1 << Channel)) {
			DMAReg->dma_int_tc_clr = (1 << Channel);
			intrStatus |= 1;
			break;
		}

		if (DMAReg->dma_err & (1 << Channel)) {
			DMAReg->dma_int_err_clr =  (1 << Channel);
			intrStatus |= 2;
		}
	} while (!intrStatus);

	fLib_DMA_ResetChannel(Channel);
	fLib_ClearDMAChannelIntStatus(Channel);

	if (intrStatus & 0x2)
		return 1;

	return 0;
}
#endif
