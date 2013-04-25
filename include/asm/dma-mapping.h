/*
 * (C) Copyright 2007
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_ARM_DMA_MAPPING_H
#define __ASM_ARM_DMA_MAPPING_H

#include <asm/io.h>

enum dma_data_direction {
	DMA_BIDIRECTIONAL	= 0,
	DMA_TO_DEVICE		= 1,
	DMA_FROM_DEVICE		= 2,
};

static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	void *va = memalign(64, len);
	
	if (va == NULL)
		return NULL;

	if (handle)
		*handle  = virt_to_phys(va);

	/* invalidate the buffer */
	dcache_invalidate((uint32_t)va, (uint32_t)len);

	/* convert the va to un-cached version */
	va = virt_to_unca(va);

	return va;
}

static inline unsigned long dma_map_single(volatile void *vaddr, size_t len,
					   enum dma_data_direction dir)
{
	switch(dir) {
	case DMA_BIDIRECTIONAL:
	case DMA_TO_DEVICE:
		dcache_flush((uint32_t)vaddr, (uint32_t)len);
		break;

	case DMA_FROM_DEVICE:
		dcache_invalidate((uint32_t)vaddr, (uint32_t)len);
		break;
	}

	return virt_to_phys((void *)vaddr);
}

static inline void dma_unmap_single(volatile void *vaddr, size_t len,
				    unsigned long paddr)
{
}

#endif /* __ASM_ARM_DMA_MAPPING_H */
