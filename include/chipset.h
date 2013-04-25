
/* DO NOT EDIT!! - this file automatically generated
 *                 from .s file by awk -f s2h.awk
 */
/****************************************************************************
 * Copyright  Faraday Technology Corp 2002-2003.  All rights reserved.      *
 *--------------------------------------------------------------------------*
 * Name:cpe.s                                                               *
 * Description: CPE board specfic define                                    *
 * Author: Fred Chien                                                       *
 ****************************************************************************
 */

/* 
 *   CPE address map;
 * 
 *                +==========================================
 *     0x00000000 | ROM 
 *                |
 *     0x00080000 |==========================================
 *                | SRAM
 *                |==========================================
 *     0x10000000 | SDRAM 
 *     0x8FFFFFFF |
 *                |==========================================
 *     0x90000000 |Controller's reg				
 *                |						
 *                |0x90100000 Arbiter/Decoder             						
 *                |0x90200000 SRAM controller reg  
 *        		|0x902ffffc Debug Numberic LED					
 *                |0x90900000 SDRAM controller reg        						
 *                |0x90400000 DMA controller reg          						
 *                |0x90500000 AHB2APB bridge              						
 *                |0x90600000 Reserved                    						
 *                |0x91000000-91F00000 DMA0-DMA15 Device; 						
 *                |0x92400000 DSP                         						
 *                |0x92500000 Reserved                         						
 *                |0x96500000 LCD                         						
 *                |0x96600000 Bluetooth                        						
 *                |0x96700000 MAC        						
 *                |0x96800000 PCI                       						
 *                |0x96900000 USB2.0 host                       
 *                |0x98000000-9AFFFFFF APB Device                         ;               |
 *     0x98000000 |==========================================
 *                | APB Device's Reg
 *                |
 *                |0x98000000 Reserved
 *                |0x98100000 Power Managemnet
 *                |0x98200000 UART1
 *                |0x98300000 UART2/IrDA
 *                |0x98400000 Timer
 *                |0x98500000 Watchdog Timer
 *                |0x98600000 RTC
 *                |0x98700000 GPIO
 *                |0x98800000 INTC
 *                |0x98900000 UART3
 *                |0x98A00000 I2C
 *                |0x98B00000 SSP1
 *                |0x98C00000 USB Device
 *                |0x98D00000 Compact Flash
 *                |0x98E00000 Secure Digital
 *                |0x98F00000 SMC
 *                |0x99000000 MS
 *                |0x99100000 SCI
 *                |0x99200000 ECP/EPP
 *                |0x99300000 KBC                 
 *                |0x99400000 I2S                 
 *                |0x99500000 AC97                 
 *                |0x99600000 SSP2                 
 *                |0x99700000 Mouse                 
 *                |0x9AFFFFFF Reserved                             
 *                |          
 *                +==========================================
 */


/*  -------------------------------------------------------------------------------
 *   CPE system registers
 *  ------------------------------------------------------------------------------- 
 * -------------------------------------------------------------------------------
 *  Decoder definitions
 * -------------------------------------------------------------------------------
 */
#ifndef __CHIPSET_H
#define __CHIPSET_H

#define AHB_SLAVE0_REG              0x00
#define AHB_SLAVE1_REG              0x04
#define AHB_SLAVE2_REG              0x08
#define AHB_SLAVE3_REG              0x0c
#define AHB_SLAVE4_REG              0x10
#define AHB_SLAVE5_REG              0x14
#define AHB_SLAVE6_REG              0x18
#define AHB_SLAVE7_REG              0x1c
#define AHB_SLAVE8_REG              0x20
#define AHB_SLAVE9_REG              0x24
#define AHB_SLAVE10_REG             0x28

#define CPE_PRIORITY_REG            0x80
#define CPE_DEFAULT_MASTER_REG      0x84
#define CPE_REMAP_REG               0x88 


/* -------------------------------------------------------------------------------
 *  AHB2APB Bridge definitions
 * -------------------------------------------------------------------------------
 */


#define APB_SLAVE0_REG              0x0
#define APB_SLAVE1_REG              0x4
#define APB_SLAVE2_REG              0x8
#define APB_SLAVE3_REG              0xc
#define APB_SLAVE4_REG              0x10
#define APB_SLAVE5_REG              0x14
#define APB_SLAVE6_REG              0x18
#define APB_SLAVE7_REG              0x1c
#define APB_SLAVE8_REG              0x20
#define APB_SLAVE9_REG              0x24
#define APB_SLAVE10_REG             0x28
#define APB_SLAVE11_REG             0x2c
#define APB_SLAVE12_REG             0x30
#define APB_SLAVE13_REG             0x34
#define APB_SLAVE14_REG             0x38
#define APB_SLAVE15_REG             0x3c
#define APB_SLAVE16_REG             0x40
#define APB_SLAVE17_REG             0x44
#define APB_SLAVE18_REG             0x48
#define APB_SLAVE19_REG             0x4c  

#define APB_DMA_SRC_A               0x80
#define APB_DMA_DEST_A              0x84
#define APB_DMA_CYCLE_A             0x88
#define APB_DMA_CMD_A               0x8c
#define APB_DMA_SRC_B               0x90
#define APB_DMA_DEST_B              0x94
#define APB_DMA_CYCLE_B             0x98
#define APB_DMA_CMD_B               0x9c
#define APB_DMA_SRC_C               0xa0
#define APB_DMA_DEST_C              0xa4
#define APB_DMA_CYCLE_C             0xa8
#define APB_DMA_CMD_C               0xac
#define APB_DMA_SRC_D               0xb0
#define APB_DMA_DEST_D              0xb4
#define APB_DMA_CYCLE_D             0xb8
#define APB_DMA_CMD_D               0xbc

#endif
/* 	END */

