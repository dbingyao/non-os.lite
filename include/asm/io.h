/*
 *  linux/include/asm-arm/io.h
 *
 *  Copyright (C) 2012 Dante Su
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/sizes.h>

static inline void *virt_to_unca(void *va)
{
#ifdef CONFIG_USE_MMU
	unsigned long base = 0xffffffff - CONFIG_RAM_SIZE + 1;	/* un-cached base address */

# ifdef CONFIG_USE_IRQ
	if ((unsigned long)va < SZ_1M)
		return (void *)(base + (unsigned long)va);
# endif

	if ((unsigned long)va >= CONFIG_RAM_BASE && 
		(unsigned long)va < (CONFIG_RAM_BASE + CONFIG_RAM_SIZE))
		return (void *)(base + ((unsigned long)va - CONFIG_RAM_BASE));

#endif	/* CONFIG_USE_MMU */
	return va;
}

static inline phys_addr_t virt_to_phys(void * vaddr)
{
	unsigned long phys = (unsigned long)(vaddr);
	
#ifdef CONFIG_USE_MMU
	unsigned long base = 0xffffffff - CONFIG_RAM_SIZE + 1;	/* un-cached base address */

	if (phys >= base)
		phys = CONFIG_RAM_BASE + (phys - base);
# if defined(CONFIG_USE_IRQ) && (CONFIG_RAM_BASE != 0)
	else if (phys < SZ_1M)
		phys = CONFIG_RAM_BASE + (phys);
# endif

#endif	/* CONFIG_USE_MMU */

	return (phys_addr_t)phys;
}

/*
 * Generic virtual read/write.
 */

#ifndef REG32
#define REG32(addr)		*(volatile uint32_t *)(addr)
#endif

#ifndef REG16
#define REG16(addr)		*(volatile uint16_t *)(addr)
#endif

#ifndef REG8
#define REG8(addr)		*(volatile uint8_t *)(addr)
#endif

#endif	/* __ASM_ARM_IO_H */
