/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:DMA.c                                                               *
* Description: DMA C Library routine                                       *
* Author: Fred Chien                                                       *
****************************************************************************/

#include <stdio.h>
#include <common.h>
#include <dmac030.h>


DMA_Reg_st *DMA_Register = (DMA_Reg_st *)DMA030_BASE;
int bWaitIntCB;

void DMA_SetCHCsr(uint32_t Channel, DMA_CH_CSR_t Csr)
{
	DMA_Register->dma_ch[Channel].csr = Csr;
}

DMA_CH_CSR_t DMA_GetCHCsr(uint32_t Channel)
{
	return DMA_Register->dma_ch[Channel].csr;
}

#ifndef DMA_INT_POLLING
void DMA_InterruptHandler(void)
{
	uint32_t	reg, Channel=0;

	bWaitIntCB=0;
	reg=GetDMAIntStatus();
	if(reg>0){
		for(Channel=0;Channel<16;Channel++){
			if((reg>>Channel)&0x01){
				ClearDMAChannelIntStatus(Channel);
				DisableDMAChannel(Channel);
			}
		}
	}
	return;
}

#endif
void DMA_SetCHCfg(uint32_t Channel, DMA_CH_CFG_t Cfg)
{
	DMA_Register->dma_ch[Channel].cfg = Cfg;
}

int DMA_SetLDMAddr(uint32_t Address)
{
	DMA_Register->dma_ldm = Address;
}
int DMA_ISChannelEnable(uint32_t Channel)
{
	DMA_CH_CSR_t reg;
	reg = GetDMAChannelCfg(Channel);
	return ((*(uint32_t*)&reg)&DMA_CSR_CH_EN);
}


uint32_t DMA_WaitIntStatus(uint32_t Channel)
{
	uint32_t choffset;
	uint32_t Status;

#ifndef DMA_INT_POLLING
	//wait interrupt handler to clear the interrupt for hardware interrupt
	choffset = 1 << Channel;

	while(bWaitIntCB);

#else
	choffset = 1 << Channel;
	Status = 0;
	do {
		if(DMA_Register->dma_tc & choffset) {
			DMA_Register->dma_int_tc_clr = choffset;
			Status |= 1;
			break;
		}

		if(DMA_Register->dma_err & choffset) {
			DMA_Register->dma_int_err_clr = choffset;
			Status |= 2;
		}
	} while (!Status);

	return Status;
#endif
}

int DMA_GetAXIBusWidth()
{
	uint32_t Hw_feature,AxiWidth;
	DMA_FEATURE_t *Feature;
	
	Hw_feature=inw((uint32_t *)(DMA030_BASE+DMA_FEATURE));
	Feature=(DMA_FEATURE_t *)&Hw_feature;
	if((Feature->d_width) >2)
	     prints("AXI data bus width greater than 128 bits doesn't support\n");
	AxiWidth=(Feature->d_width)+2;
	return 	AxiWidth;
}

int DMA_GetDepthOfFIFO()
{
	uint32_t 	Hw_feature;
	DMA_FEATURE_t *Feature;
	
	Hw_feature=inw((uint32_t *)(DMA030_BASE+DMA_FEATURE));
	Feature=(DMA_FEATURE_t *)&Hw_feature;
	if((Feature->dfifo_depth) >4)
	     prints("The depth of data FIFO greater than 128!!\n");
	return Feature->dfifo_depth;
}

int IsDMAChannelEnable(uint32_t Channel)
{
	return ((DMA_Register->dma_ch_enable >> Channel) & 0x1);
}

uint32_t GetDMAIntStatus(void)
{
	return DMA_Register->dma_int;
}

uint32_t GetDMAChannelIntStatus(uint32_t Channel)
{
	uint32_t IntStatus = 0;

	if((DMA_Register->dma_int >> Channel) & 0x01)
	{
		if((DMA_Register->dma_int_tc >> Channel) & 0x01)
			IntStatus |= 1;
		if((DMA_Register->dma_int_err >> Channel) & 0x01)
			IntStatus |= 2;
	}

	return IntStatus;
}
/*
int GetDMABusyStatus(void)
{
	return DMA_Register->dma_ch_busy;
}
*///wendy
int GetDMAEnableStatus(void)
{
	return DMA_Register->dma_ch_enable;
}

void EnableDMAChannel(uint32_t Channel)
{
#ifndef	DMA_INT_POLLING
	bWaitIntCB=1;
#endif
	DMA_Register->dma_ch[Channel].csr.ch_en = 1;
}

void DisableDMAChannel(uint32_t Channel)
{
	DMA_Register->dma_ch[Channel].csr.ch_en = 0;
}

void EnableDMAChannelEndianConverter(uint32_t Channel)
{
  volatile uint32_t reg;
  reg=inw(DMA030_BASE+DMA_ENDIAN);
 
  reg |= 1<<Channel;
 
  outl(reg,DMA030_BASE+DMA_ENDIAN);
	
}
 
void DisableDMAChannelEndianConverter(uint32_t Channel)
{
   outl(0<<Channel,DMA030_BASE+DMA_ENDIAN);
}

void EnableDMAChannelWriteOnly(uint32_t Channel)
{
	volatile uint32_t reg;

	reg = *(uint32_t *)&DMA_Register->dma_ch[Channel].cfg;
	reg |= DMA_CSR_WRITE_ONLY;
	*(uint32_t *)&DMA_Register->dma_ch[Channel].cfg = reg;
}

void EnableDMAChannelUnalign(uint32_t Channel)
{
	volatile uint32_t reg;
       // printk("\nEnable unalign mode");
	reg = *(uint32_t *)&DMA_Register->dma_ch[Channel].cfg;
	reg |= DMA_CSR_UNALIGN;
	*(uint32_t *)&DMA_Register->dma_ch[Channel].cfg = reg;
}

// This function was modified by jerry
void ClearDMAChannelIntStatus(uint32_t Channel)
{
	uint32_t ctrl;

	ctrl = 1 << Channel;
	DMA_Register->dma_int_tc_clr = 1 << Channel;

	ctrl = (1<<Channel) | (1<<(Channel+16));
	DMA_Register->dma_int_err_clr = 1 << Channel;

}


void SetDMAChannelCfg(uint32_t Channel, DMA_CH_CSR_t Csr)
{
	DMA_Register->dma_ch[Channel].csr = Csr;
}

DMA_CH_CSR_t GetDMAChannelCfg(uint32_t Channel)
{
	return DMA_Register->dma_ch[Channel].csr;
}

void DMA_CHIntMask(uint32_t Channel, DMA_CH_CFG_t Mask)
{
	DMA_Register->dma_ch[Channel].cfg = Mask;
}

void DMA_CHLinkList(uint32_t Channel, DMA_CH_LLP_t LLP)
{
	DMA_Register->dma_ch[Channel].llp = LLP;
}

void DMA_CHDataCtrl(uint32_t Channel, uint32_t SrcAddr, uint32_t DstAddr, uint32_t Size)
{
	DMA_Register->dma_ch[Channel].src_addr = SrcAddr;
	DMA_Register->dma_ch[Channel].dst_addr = DstAddr;
	DMA_Register->dma_ch[Channel].size = Size;
}

void DMA_CHDataCtrl_2D(uint32_t Channel, uint32_t SrcAddr, uint32_t DstAddr, uint32_t XTcnt, 
		       uint32_t YTcnt, uint32_t DstStride, uint32_t SrcStride)
{
	DMA_Register->dma_ch[Channel].src_addr = SrcAddr;
	DMA_Register->dma_ch[Channel].dst_addr = DstAddr;
	DMA_Register->dma_ch[Channel].size = (YTcnt << 16) | XTcnt;
	DMA_Register->dma_ch[Channel].stride = (DstStride << 16) | SrcStride;
}


void DMA_SetInterrupt(uint32_t channel, uint32_t tcintr, uint32_t errintr, uint32_t abtintr)
{
	DMA_CH_CFG_t cfg;

	if(tcintr)
		cfg.int_tc_msk = 0;	// Enable terminal count interrupt
	else
		cfg.int_tc_msk = 1;	// Disable terminal count interrupt

	if(errintr)
		cfg.int_err_msk = 0;	// Enable error interrupt
	else
		cfg.int_err_msk = 1;	// Disable error interrupt

	if(abtintr)
		cfg.int_abt_msk = 0;	// Enable abort interrupt
	else
		cfg.int_abt_msk = 1;	// Disable abort interrupt

	DMA_CHIntMask(channel, cfg);
}

void DMA_ResetChannel(uint8_t channel)
{
	uint32_t base = DMA030_BASE+DMA_CHANNEL0_BASE+channel*DMA_CHANNEL_OFFSET;

	outl(0,base+DMA_CHANNEL_CSR_OFFSET);
	outl(7,base+DMA_CHANNEL_CFG_OFFSET);
	outl(0,base+DMA_CHANNEL_SRCADDR_OFFSET);
	outl(0,base+DMA_CHANNEL_DSTADDR_OFFSET);
	outl(0,base+DMA_CHANNEL_LLP_OFFSET);
	outl(0,base+DMA_CHANNEL_SIZE_OFFSET);
}

void DMA_ClearAllInterrupt(void)
{
	// Clear all interrupt source
	outl(0xFF,DMA030_BASE+DMA_INT_TC_CLR);
	outl(0xFF00FF,DMA030_BASE+DMA_INT_ERRABT_CLR);
}

void DMA_SetWriteOnlyValue(uint32_t Value)
{
	// set constant value 
	outl(Value,DMA030_BASE+DMA_WRITE_ONLY);
}

void DMA_EnableFeatureUnalign(void)
{
	volatile uint32_t reg;
	reg=inw(DMA030_BASE+DMA_FEATURE);
	reg |= DMA_FEATURE_UNALIGN;
	outl(reg,DMA030_BASE+DMA_FEATURE);
}	

uint32_t DMA_UnalignFeature(void)
{
	volatile uint32_t reg;
	reg=inw(DMA030_BASE+DMA_FEATURE);
	reg &= DMA_FEATURE_UNALIGN;
	if(reg)
	   return 1;
	else
	   return 0;
}	
	
void InitDMA(void)
{
	DMA_Register->dma_plverr = 0; //Disable DMA apb slave error response
}

