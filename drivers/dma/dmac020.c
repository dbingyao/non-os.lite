/***************************************************************************
* Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
*--------------------------------------------------------------------------*
* Name:DMA.c                                                               *
* Description: DMA C Library routine                                       *
* Author: Fred Chien                                                       *
****************************************************************************/

#include <platform.h>
#include <dmac020.h>

volatile fLib_DMA_Reg_t *DMAReg = (fLib_DMA_Reg_t *)FTDMAC020_REG_BASE;

int fLib_IsDMAChannelBusy(uint Channel)
{
	return ((DMAReg->dma_ch_busy >> Channel) & 0x1);
}

int fLib_IsDMAChannelEnable(uint Channel)
{
	return ((DMAReg->dma_ch_enable >> Channel) & 0x1);
}

int fLib_GetDMABusyStatus(void)
{
	return DMAReg->dma_ch_busy;
}

int fLib_GetDMAEnableStatus(void)
{
	return DMAReg->dma_ch_enable;
}

uint fLib_GetDMAIntStatus(void)
{
	return DMAReg->dma_int;
}



uint fLib_GetDMAChannelIntStatus(uint Channel)
{
	volatile uint IntStatus = 0;
	
	if((DMAReg->dma_int >> Channel) & 0x01)
	{
		if((DMAReg->dma_int_tc >> Channel) & 0x01)
			IntStatus |= 1;
		if((DMAReg->dma_int_err >> Channel) & 0x01)
			IntStatus |= 2;
	}
	
	
	
	
	return IntStatus;
}

void fLib_InitDMA(uint M0_BigEndian, uint M1_BigEndian, uint Sync)
{		
	DMAReg->dma_csr = (M0_BigEndian ? DMA_CSR_M0ENDIAN : 0) | 
	(M1_BigEndian ? DMA_CSR_M1ENDIAN : 0) | DMA_CSR_DMACEN;
	
	DMAReg->dma_sync = Sync; 
}

void fLib_EnableDMAChannel(uint Channel)
{
	uint reg;
		
	reg = *(uint *)&DMAReg->dma_ch[Channel].csr;
	reg |= DMA_CSR_CH_ENABLE;
	*(uint *)&DMAReg->dma_ch[Channel].csr = reg;
}

void fLib_DisableDMAChannel(uint Channel)
{
	uint reg;
	
	reg = *(uint *)&DMAReg->dma_ch[Channel].csr;
	reg &= ~DMA_CSR_CH_ENABLE;
	*(uint *)&DMAReg->dma_ch[Channel].csr = reg;
}

// added by Shawn 2005.5.18
void fLib_EnableDMA(unsigned int value)
{
	unsigned int reg;
	
	value &= 0x1;	// check input
	
	reg = DMAReg->dma_csr;
	
	reg &= ~(0x1);
	reg |= value;
	
	DMAReg->dma_csr = reg;
}

// This function was modified by jerry
void fLib_ClearDMAChannelIntStatus(uint Channel)
{
	uint ctrl;
	
	ctrl = 1 << Channel;
	DMAReg->dma_int_tc_clr = 1 << Channel;
	
	ctrl = (1<<Channel) | (1<<(Channel+16));
	DMAReg->dma_int_err_clr = 1 << Channel;

}


void fLib_SetDMAChannelCfg(uint Channel, fLib_DMA_CH_CSR_t Csr)
{
	DMAReg->dma_ch[Channel].csr = Csr;
}

fLib_DMA_CH_CSR_t fLib_GetDMAChannelCfg(uint Channel)
{
	return DMAReg->dma_ch[Channel].csr;
}


void fLib_SetDMAChannelCnCfg(uint Channel, fLib_DMA_CH_CFG_t CnCfg)
{
	DMAReg->dma_ch[Channel].cfg = CnCfg;
}

fLib_DMA_CH_CFG_t fLib_GetDMAChannelCnCfg(uint Channel)
{
	return DMAReg->dma_ch[Channel].cfg;
}

void fLib_DMA_CHIntMask(uint Channel, fLib_DMA_CH_CFG_t Mask)
{
	DMAReg->dma_ch[Channel].cfg = Mask;
}

void fLib_DMA_CHLinkList(uint Channel, uint link_list_addr)
{
	DMAReg->dma_ch[Channel].link_list_addr = link_list_addr;
}

void fLib_DMA_CHDataCtrl(uint Channel, uint SrcAddr, uint DstAddr, uint Size)
{
	DMAReg->dma_ch[Channel].src_addr = SrcAddr;
	DMAReg->dma_ch[Channel].dst_addr = DstAddr;
	DMAReg->dma_ch[Channel].size = Size;
}

void fLib_DMA_SetInterrupt(uint channel, uint tcintr, uint errintr, uint abtintr)
{
	fLib_DMA_CH_CFG_t cfg;
	int i;
	
	cfg =  fLib_GetDMAChannelCnCfg(channel); //ycmo091007 add
	
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
	
	fLib_DMA_CHIntMask(channel, cfg);	

}

void fLib_DMA_ResetChannel(unchar channel)
{
#if 0
	uint base = FTDMAC020_REG_BASE + DMA_CHANNEL0_BASE + channel*DMA_CHANNEL_OFFSET;
	
	outl(base+DMA_CHANNEL_CSR_OFFSET,0);
	outl(base+DMA_CHANNEL_CFG_OFFSET,7);
	outl(base+DMA_CHANNEL_SRCADDR_OFFSET,0);
	outl(base+DMA_CHANNEL_DSTADDR_OFFSET,0);
	outl(base+DMA_CHANNEL_LLP_OFFSET,0);
	outl(base+DMA_CHANNEL_SIZE_OFFSET,0);
#endif

	memset(&DMAReg->dma_ch[channel], 0, sizeof(fLib_DMA_CH_t));

}

void fLib_DMA_ClearAllInterrupt()
{
	// Clear all interrupt source
	outl(FTDMAC020_REG_BASE + DMA_INT_TC_CLR,0xFF);
	outl(FTDMAC020_REG_BASE + DMA_INT_ERRABT_CLR,0xFF00FF);	
}

void fLib_DMA_WaitIntStatus(uint Channel)
{
	uint choffset;
	volatile uint status;
	
	choffset = 1 << Channel;

	while((inl(FTDMAC020_REG_BASE + DMA_TC) & choffset)==0)
    	;

	
	fLib_DisableDMAChannel(Channel);
}

