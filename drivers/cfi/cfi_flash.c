/*
 * (C) Copyright 2002-2004
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Copyright (C) 2004
 * Ed Okerson
 *
 * Copyright (C) 2006
 * Tolunay Orkun <listmember@orkun.us>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/* The prints define must be before common to enable printsging */
/* #define prints	*/

#include <string.h>
#include <common.h>
#include <platform.h>
#include <flash.h>

#include "cfi_flash.h"


/*
 * This file implements a Common Flash Interface (CFI) driver for
 * U-Boot.
 *
 * The width of the port and the width of the chips are determined at
 * initialization.  These widths are used to calculate the address for
 * access CFI data structures.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 * AMD CFI Specification, Release 2.0 December 1, 2001
 * AMD/Spansion Application Note: Migration from Single-byte to Three-byte
 *   Device IDs, Publication Number 25538 Revision A, November 8, 2001
 *
 * Define CONFIG_SYS_WRITE_SWAPPED_DATA, if you have to swap the Bytes between
 * reading and writing ... (yes there is such a Hardware).
 */

static uint32_t flash_offset_cfi[2] = { FLASH_OFFSET_CFI, FLASH_OFFSET_CFI_ALT };

#define flash_verbose 1

flash_info_t flash_info[CFI_MAX_FLASH_BANKS];	/* FLASH chips info */

/*
 * Check if chip width is defined. If not, start detecting with 8bit.
 */
#ifndef CONFIG_SYS_FLASH_CFI_WIDTH
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#endif

/*
 * 0xffff is an undefined value for the configuration register. When
 * this value is returned, the configuration register shall not be
 * written at all (default mode).
 */
static uint16_t cfi_flash_config_reg(int i)
{
#ifdef CONFIG_SYS_CFI_FLASH_CONFIG_REGS
	return ((uint16_t [])CONFIG_SYS_CFI_FLASH_CONFIG_REGS)[i];
#else
	return 0xffff;
#endif
}

#if defined(CONFIG_SYS_MAX_FLASH_BANKS_DETECT)
int cfi_flash_num_flash_banks = CONFIG_SYS_MAX_FLASH_BANKS_DETECT;
#endif

static phys_addr_t __cfi_flash_bank_addr(int i)
{
	return ((phys_addr_t [])CONFIG_SYS_FLASH_BANKS_LIST)[i];
}
phys_addr_t cfi_flash_bank_addr(int i)
	__attribute__((weak, alias("__cfi_flash_bank_addr")));

static unsigned long __cfi_flash_bank_size(int i)
{
#ifdef CONFIG_SYS_FLASH_BANKS_SIZES
	return ((unsigned long [])CONFIG_SYS_FLASH_BANKS_SIZES)[i];
#else
	return 0;
#endif
}
unsigned long cfi_flash_bank_size(int i)
	__attribute__((weak, alias("__cfi_flash_bank_size")));

static void __flash_write8(uint8_t value, void *addr)
{
	outb(value, addr);
}

static void __flash_write16(uint16_t value, void *addr)
{
	outw(value, addr);
}

static void __flash_write32(uint32_t value, void *addr)
{
	outl(value, addr);
}

static void __flash_write64(uint64_t value, void *addr)
{
	/* No architectures currently implement __raw_writeq() */
	*(volatile uint64_t *)addr = value;
}

static uint8_t __flash_read8(void *addr)
{
	return inb(addr);
}

static uint16_t __flash_read16(void *addr)
{
	return inw(addr);
}

static uint32_t __flash_read32(void *addr)
{
	return inl(addr);
}

static uint64_t __flash_read64(void *addr)
{
	/* No architectures currently implement __raw_readq() */
	return *(volatile uint64_t *)addr;
}

#ifdef CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
void flash_write8(uint8_t value, void *addr)__attribute__((weak, alias("__flash_write8")));
void flash_write16(uint16_t value, void *addr)__attribute__((weak, alias("__flash_write16")));
void flash_write32(uint32_t value, void *addr)__attribute__((weak, alias("__flash_write32")));
void flash_write64(uint64_t value, void *addr)__attribute__((weak, alias("__flash_write64")));
uint8_t flash_read8(void *addr)__attribute__((weak, alias("__flash_read8")));
uint16_t flash_read16(void *addr)__attribute__((weak, alias("__flash_read16")));
uint32_t flash_read32(void *addr)__attribute__((weak, alias("__flash_read32")));
uint64_t flash_read64(void *addr)__attribute__((weak, alias("__flash_read64")));
#else
#define flash_write8	__flash_write8
#define flash_write16	__flash_write16
#define flash_write32	__flash_write32
#define flash_write64	__flash_write64
#define flash_read8	__flash_read8
#define flash_read16	__flash_read16
#define flash_read32	__flash_read32
#define flash_read64	__flash_read64
#endif

/*-----------------------------------------------------------------------
 */
flash_info_t *flash_get_info(uint64_t base)
{
	int i;
	flash_info_t *info = NULL;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return info;
}

unsigned long flash_sector_size(flash_info_t *info, flash_sect_t sect)
{
	if (sect != (info->sector_count - 1))
		return info->start[sect + 1] - info->start[sect];
	else
		return info->start[0] + info->size - info->start[sect];
}

/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
static inline void *
flash_map (flash_info_t * info, flash_sect_t sect, uint32_t offset)
{
	unsigned int byte_offset = offset * info->portwidth;

	return (void *)(info->start[sect] + byte_offset);
}

static inline void flash_unmap(flash_info_t *info, flash_sect_t sect,
		unsigned int offset, void *addr)
{
}

/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd(flash_info_t *info, uint32_t cmd, void *cmdbuf)
{
	int i;
	int cword_offset;
	int cp_offset;
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	uint32_t cmd_le = cpu_to_le32(cmd);
#endif
	uint8_t val;
	uint8_t *cp = (uint8_t *) cmdbuf;

	for (i = info->portwidth; i > 0; i--){
		cword_offset = (info->portwidth-i)%info->chipwidth;
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		cp_offset = info->portwidth - i;
		val = *((uint8_t*)&cmd_le + cword_offset);
#else
		cp_offset = i - 1;
		val = *((uint8_t*)&cmd + sizeof(uint32_t) - cword_offset - 1);
#endif
		cp[cp_offset] = (cword_offset >= sizeof(uint32_t)) ? 0x00 : val;
	}
}

#ifdef prints
/*-----------------------------------------------------------------------
 * prints support
 */
static void print_longlong (char *str, unsigned long long data)
{
	int i;
	char *cp;

	cp = (char *) &data;
	for (i = 0; i < 8; i++)
		sprintf (&str[i * 2], "%2.2x", *cp++);
}

static void flash_printqry (struct cfi_qry *qry)
{
	uint8_t *p = (uint8_t *)qry;
	int x, y;

	for (x = 0; x < sizeof(struct cfi_qry); x += 16) {
		prints("%02x : ", x);
		for (y = 0; y < 16; y++)
			prints("%2.2x ", p[x + y]);
		prints(" ");
		for (y = 0; y < 16; y++) {
			unsigned char c = p[x + y];
			if (c >= 0x20 && c <= 0x7e)
				prints("%c", c);
			else
				prints(".");
		}
		prints("\n");
	}
}
#endif


/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
static inline uint8_t flash_read_uchar (flash_info_t * info, uint32_t offset)
{
	uint8_t *cp;
	uint8_t retval;

	cp = flash_map (info, 0, offset);
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	retval = flash_read8(cp);
#else
	retval = flash_read8(cp + info->portwidth - 1);
#endif
	flash_unmap (info, 0, offset, cp);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a word at a port width address, assume 16bit bus
 */
static inline uint16_t flash_read_word (flash_info_t * info, uint32_t offset)
{
	uint16_t *addr, retval;

	addr = flash_map (info, 0, offset);
	retval = flash_read16 (addr);
	flash_unmap (info, 0, offset, addr);
	return retval;
}


/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maximum
 * port size word. Swap for ppc format.
 */
static uint64_t flash_read_long (flash_info_t * info, flash_sect_t sect,
			      uint32_t offset)
{
	uint8_t *addr;
	uint64_t retval;

#ifdef prints
	int x;
#endif
	addr = flash_map (info, sect, offset);

#ifdef prints
	prints ("long addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 4 * info->portwidth; x++) {
		prints ("addr[%x] = 0x%x\n", x, flash_read8(addr + x));
	}
#endif
#if defined(__LITTLE_ENDIAN) || defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	retval = ((flash_read8(addr) << 16) |
		  (flash_read8(addr + info->portwidth) << 24) |
		  (flash_read8(addr + 2 * info->portwidth)) |
		  (flash_read8(addr + 3 * info->portwidth) << 8));
#else
	retval = ((flash_read8(addr + 2 * info->portwidth - 1) << 24) |
		  (flash_read8(addr + info->portwidth - 1) << 16) |
		  (flash_read8(addr + 4 * info->portwidth - 1) << 8) |
		  (flash_read8(addr + 3 * info->portwidth - 1)));
#endif
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * Write a proper sized command to the correct address
 */
void flash_write_cmd (flash_info_t * info, flash_sect_t sect,
		      uint32_t offset, uint32_t cmd)
{

	void *addr;
	cfiword_t cword;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		prints ("fwc addr %p cmd %x %x 8bit x %d bit\n", addr, cmd,
		       cword.c, info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write8(cword.c, addr);
		break;
	case FLASH_CFI_16BIT:
		prints ("fwc addr %p cmd %x %4.4x 16bit x %d bit\n", addr,
		       cmd, cword.w,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write16(cword.w, addr);
		break;
	case FLASH_CFI_32BIT:
		prints ("fwc addr %p cmd %x %8.8lx 32bit x %d bit\n", addr,
		       cmd, cword.l,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write32(cword.l, addr);
		break;
	case FLASH_CFI_64BIT:
#ifdef prints
		{
			char str[20];

			print_longlong (str, cword.ll);

			prints ("fwrite addr %p cmd %x %s 64 bit x %d bit\n",
			       addr, cmd, str,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		}
#endif
		flash_write64(cword.ll, addr);
		break;
	}

	flash_unmap(info, sect, offset, addr);
}

static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect)
{
	flash_write_cmd (info, sect, info->addr_unlock1, AMD_CMD_UNLOCK_START);
	flash_write_cmd (info, sect, info->addr_unlock2, AMD_CMD_UNLOCK_ACK);
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal (flash_info_t * info, flash_sect_t sect,
			  uint32_t offset, uint8_t cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);

	prints ("is= cmd %x(%c) addr %p ", cmd, cmd, addr);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		prints ("is= %x %x\n", flash_read8(addr), cword.c);
		retval = (flash_read8(addr) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		prints ("is= %4.4x %4.4x\n", flash_read16(addr), cword.w);
		retval = (flash_read16(addr) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		prints ("is= %8.8x %8.8lx\n", flash_read32(addr), cword.l);
		retval = (flash_read32(addr) == cword.l);
		break;
	case FLASH_CFI_64BIT:
#ifdef prints
		{
			char str1[20];
			char str2[20];

			print_longlong (str1, flash_read64(addr));
			print_longlong (str2, cword.ll);
			prints ("is= %s %s\n", str1, str2);
		}
#endif
		retval = (flash_read64(addr) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_isset (flash_info_t * info, flash_sect_t sect,
			uint32_t offset, uint8_t cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((flash_read8(addr) & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = ((flash_read16(addr) & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = ((flash_read32(addr) & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		retval = ((flash_read64(addr) & cword.ll) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_toggle (flash_info_t * info, flash_sect_t sect,
			 uint32_t offset, uint8_t cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = flash_read8(addr) != flash_read8(addr);
		break;
	case FLASH_CFI_16BIT:
		retval = flash_read16(addr) != flash_read16(addr);
		break;
	case FLASH_CFI_32BIT:
		retval = flash_read32(addr) != flash_read32(addr);
		break;
	case FLASH_CFI_64BIT:
		retval = ( (flash_read32( addr ) != flash_read32( addr )) ||
			   (flash_read32(addr+4) != flash_read32(addr+4)) );
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * flash_is_busy - check to see if the flash is busy
 *
 * This routine checks the status of the chip and returns true if the
 * chip is busy.
 */
static int flash_is_busy (flash_info_t * info, flash_sect_t sect)
{
	int retval;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		retval = !flash_isset (info, sect, 0, FLASH_STATUS_DONE);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
#endif
		retval = flash_toggle (info, sect, 0, AMD_STATUS_TOGGLE);
		break;
	default:
		retval = 0;
	}
	prints ("flash_is_busy: %d\n", retval);
	return retval;
}

/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check (flash_info_t * info, flash_sect_t sector,
			       uint64_t tout, char *prompt)
{
	uint64_t start;

#if CONFIG_SYS_HZ != 1000
	if ((uint64_t)CONFIG_SYS_HZ > 100000)
		tout *= (uint64_t)CONFIG_SYS_HZ / 1000;  /* for a big HZ, avoid overflow */
	else
		tout = DIV_ROUND_UP(tout * (uint64_t)CONFIG_SYS_HZ, 1000);
#endif

	/* Wait for command completion */
#ifdef CONFIG_SYS_LOW_RES_TIMER
	reset_timer();
#endif
	start = get_timer (0);
	while (flash_is_busy (info, sector)) {
		if (get_timer (start) > tout) {
			prints ("Flash %s timeout at address %lx data %lx\n",
				prompt, info->start[sector],
				flash_read_long (info, sector, 0));
			flash_write_cmd (info, sector, 0, info->cmd_reset);
			udelay(1);
			return ERR_TIMOUT;
		}
		udelay (1);		/* also triggers watchdog */
	}
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise
 * do a full status check.
 *
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    uint64_t tout, char *prompt)
{
	int retcode;

	retcode = flash_status_check (info, sector, tout, prompt);
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		if ((retcode != ERR_OK)
		    && !flash_isequal (info, sector, 0, FLASH_STATUS_DONE)) {
			retcode = ERR_INVAL;
			prints ("Flash %s error at address %lx\n", prompt,
				info->start[sector]);
			if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS |
					 FLASH_STATUS_PSLBS)) {
				puts ("Command Sequence Error.\n");
			} else if (flash_isset (info, sector, 0,
						FLASH_STATUS_ECLBS)) {
				puts ("Block Erase Error.\n");
				retcode = ERR_NOT_ERASED;
			} else if (flash_isset (info, sector, 0,
						FLASH_STATUS_PSLBS)) {
				puts ("Locking Error\n");
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_DPS)) {
				puts ("Block locked.\n");
				retcode = ERR_PROTECTED;
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_VPENS))
				puts ("Vpp Low Error.\n");
		}
		flash_write_cmd (info, sector, 0, info->cmd_reset);
		udelay(1);
		break;
	default:
		break;
	}
	return retcode;
}

static int use_flash_status_poll(flash_info_t *info)
{
#ifdef CONFIG_SYS_CFI_FLASH_STATUS_POLL
	if (info->vendor == CFI_CMDSET_AMD_EXTENDED ||
	    info->vendor == CFI_CMDSET_AMD_STANDARD)
		return 1;
#endif
	return 0;
}

static int flash_status_poll(flash_info_t *info, void *src, void *dst,
			     uint64_t tout, char *prompt)
{
#ifdef CONFIG_SYS_CFI_FLASH_STATUS_POLL
	uint64_t start;
	int ready;

#if CONFIG_SYS_HZ != 1000
	if ((uint64_t)CONFIG_SYS_HZ > 100000)
		tout *= (uint64_t)CONFIG_SYS_HZ / 1000;  /* for a big HZ, avoid overflow */
	else
		tout = DIV_ROUND_UP(tout * (uint64_t)CONFIG_SYS_HZ, 1000);
#endif

	/* Wait for command completion */
#ifdef CONFIG_SYS_LOW_RES_TIMER
	reset_timer();
#endif
	start = get_timer(0);
	WATCHDOG_RESET();
	while (1) {
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			ready = flash_read8(dst) == flash_read8(src);
			break;
		case FLASH_CFI_16BIT:
			ready = flash_read16(dst) == flash_read16(src);
			break;
		case FLASH_CFI_32BIT:
			ready = flash_read32(dst) == flash_read32(src);
			break;
		case FLASH_CFI_64BIT:
			ready = flash_read64(dst) == flash_read64(src);
			break;
		default:
			ready = 0;
			break;
		}
		if (ready)
			break;
		if (get_timer(start) > tout) {
			prints("Flash %s timeout at address %lx data %lx\n",
			       prompt, (uint64_t)dst, (uint64_t)flash_read8(dst));
			return ERR_TIMOUT;
		}
		udelay(1);		/* also triggers watchdog */
	}
#endif /* CONFIG_SYS_CFI_FLASH_STATUS_POLL */
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 */
static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uint8_t c)
{
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
	unsigned short	w;
	unsigned int	l;
	unsigned long long ll;
#endif

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->c = c;
		break;
	case FLASH_CFI_16BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		w = c;
		w <<= 8;
		cword->w = (cword->w >> 8) | w;
#else
		cword->w = (cword->w << 8) | c;
#endif
		break;
	case FLASH_CFI_32BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		l = c;
		l <<= 24;
		cword->l = (cword->l >> 8) | l;
#else
		cword->l = (cword->l << 8) | c;
#endif
		break;
	case FLASH_CFI_64BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CONFIG_SYS_WRITE_SWAPPED_DATA)
		ll = c;
		ll <<= 56;
		cword->ll = (cword->ll >> 8) | ll;
#else
		cword->ll = (cword->ll << 8) | c;
#endif
		break;
	}
}

/*
 * Loop through the sector table starting from the previously found sector.
 * Searches forwards or backwards, dependent on the passed address.
 */
static flash_sect_t find_sector (flash_info_t * info, uint64_t addr)
{
	static flash_sect_t saved_sector = 0; /* previously found sector */
	static flash_info_t *saved_info = 0; /* previously used flash bank */
	flash_sect_t sector = saved_sector;

	if ((info != saved_info) || (sector >= info->sector_count))
		sector = 0;

	while ((info->start[sector] < addr)
			&& (sector < info->sector_count - 1))
		sector++;
	while ((info->start[sector] > addr) && (sector > 0))
		/*
		 * also decrements the sector in case of an overshot
		 * in the first loop
		 */
		sector--;

	saved_sector = sector;
	saved_info = info;
	return sector;
}

/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword (flash_info_t * info, uint64_t dest,
				cfiword_t cword)
{
	void *dstaddr = (void *)dest;
	int flag;
	flash_sect_t sect = 0;
	char sect_found = 0;

	/* Check if Flash is (sufficiently) erased */
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((flash_read8(dstaddr) & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		flag = ((flash_read16(dstaddr) & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		flag = ((flash_read32(dstaddr) & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		flag = ((flash_read64(dstaddr) & cword.ll) == cword.ll);
		break;
	default:
		flag = 0;
		break;
	}
	if (!flag)
		return ERR_NOT_ERASED;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		flash_write_cmd (info, 0, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, 0, 0, FLASH_CMD_WRITE);
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
		sect = find_sector(info, dest);
		flash_unlock_seq (info, sect);
		flash_write_cmd (info, sect, info->addr_unlock1, AMD_CMD_WRITE);
		sect_found = 1;
		break;
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
		sect = find_sector(info, dest);
		flash_unlock_seq (info, 0);
		flash_write_cmd (info, 0, info->addr_unlock1, AMD_CMD_WRITE);
		sect_found = 1;
		break;
#endif
	}

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flash_write8(cword.c, dstaddr);
		break;
	case FLASH_CFI_16BIT:
		flash_write16(cword.w, dstaddr);
		break;
	case FLASH_CFI_32BIT:
		flash_write32(cword.l, dstaddr);
		break;
	case FLASH_CFI_64BIT:
		flash_write64(cword.ll, dstaddr);
		break;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	if (!sect_found)
		sect = find_sector (info, dest);

	if (use_flash_status_poll(info))
		return flash_status_poll(info, &cword, dstaddr,
					 info->write_tout, "write");
	else
		return flash_full_status_check(info, sect,
					       info->write_tout, "write");
}

#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE

static int flash_write_cfibuffer (flash_info_t * info, uint64_t dest, uint8_t * cp,
				  int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	void *src = cp;
	void *dst = (void *)dest;
	void *dst2 = dst;
	int flag = 1;
	uint32_t offset = 0;
	unsigned int shift;
	uint8_t write_cmd;

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		shift = 0;
		break;
	case FLASH_CFI_16BIT:
		shift = 1;
		break;
	case FLASH_CFI_32BIT:
		shift = 2;
		break;
	case FLASH_CFI_64BIT:
		shift = 3;
		break;
	default:
		retcode = ERR_INVAL;
		goto out_unmap;
	}

	cnt = len >> shift;

	while ((cnt-- > 0) && (flag == 1)) {
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			flag = ((flash_read8(dst2) & flash_read8(src)) ==
				flash_read8(src));
			src += 1, dst2 += 1;
			break;
		case FLASH_CFI_16BIT:
			flag = ((flash_read16(dst2) & flash_read16(src)) ==
				flash_read16(src));
			src += 2, dst2 += 2;
			break;
		case FLASH_CFI_32BIT:
			flag = ((flash_read32(dst2) & flash_read32(src)) ==
				flash_read32(src));
			src += 4, dst2 += 4;
			break;
		case FLASH_CFI_64BIT:
			flag = ((flash_read64(dst2) & flash_read64(src)) ==
				flash_read64(src));
			src += 8, dst2 += 8;
			break;
		}
	}
	if (!flag) {
		retcode = ERR_NOT_ERASED;
		goto out_unmap;
	}

	src = cp;
	sector = find_sector (info, dest);

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		write_cmd = (info->vendor == CFI_CMDSET_INTEL_PROG_REGIONS) ?
					FLASH_CMD_WRITE_BUFFER_PROG : FLASH_CMD_WRITE_TO_BUFFER;
		flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, sector, 0, FLASH_CMD_READ_STATUS);
		flash_write_cmd (info, sector, 0, write_cmd);
		retcode = flash_status_check (info, sector,
					      info->buffer_write_tout,
					      "write to buffer");
		if (retcode == ERR_OK) {
			/* reduce the number of loops by the width of
			 * the port */
			cnt = len >> shift;
			flash_write_cmd (info, sector, 0, cnt - 1);
			while (cnt-- > 0) {
				switch (info->portwidth) {
				case FLASH_CFI_8BIT:
					flash_write8(flash_read8(src), dst);
					src += 1, dst += 1;
					break;
				case FLASH_CFI_16BIT:
					flash_write16(flash_read16(src), dst);
					src += 2, dst += 2;
					break;
				case FLASH_CFI_32BIT:
					flash_write32(flash_read32(src), dst);
					src += 4, dst += 4;
					break;
				case FLASH_CFI_64BIT:
					flash_write64(flash_read64(src), dst);
					src += 8, dst += 8;
					break;
				default:
					retcode = ERR_INVAL;
					goto out_unmap;
				}
			}
			flash_write_cmd (info, sector, 0,
					 FLASH_CMD_WRITE_BUFFER_CONFIRM);
			retcode = flash_full_status_check (
				info, sector, info->buffer_write_tout,
				"buffer write");
		}

		break;

	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		flash_unlock_seq(info,0);

#ifdef CONFIG_FLASH_SPANSION_S29WS_N
		offset = ((unsigned long)dst - info->start[sector]) >> shift;
#endif
		flash_write_cmd(info, sector, offset, AMD_CMD_WRITE_TO_BUFFER);
		cnt = len >> shift;
		flash_write_cmd(info, sector, offset, cnt - 1);

		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			while (cnt-- > 0) {
				flash_write8(flash_read8(src), dst);
				src += 1, dst += 1;
			}
			break;
		case FLASH_CFI_16BIT:
			while (cnt-- > 0) {
				flash_write16(flash_read16(src), dst);
				src += 2, dst += 2;
			}
			break;
		case FLASH_CFI_32BIT:
			while (cnt-- > 0) {
				flash_write32(flash_read32(src), dst);
				src += 4, dst += 4;
			}
			break;
		case FLASH_CFI_64BIT:
			while (cnt-- > 0) {
				flash_write64(flash_read64(src), dst);
				src += 8, dst += 8;
			}
			break;
		default:
			retcode = ERR_INVAL;
			goto out_unmap;
		}

		flash_write_cmd (info, sector, 0, AMD_CMD_WRITE_BUFFER_CONFIRM);
		if (use_flash_status_poll(info))
			retcode = flash_status_poll(info, src - (1 << shift),
						    dst - (1 << shift),
						    info->buffer_write_tout,
						    "buffer write");
		else
			retcode = flash_full_status_check(info, sector,
							  info->buffer_write_tout,
							  "buffer write");
		break;

	default:
		prints ("Unknown Command Set\n");
		retcode = ERR_INVAL;
		break;
	}

out_unmap:
	return retcode;
}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */


/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	flash_sect_t sect;
	int st;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("Can't erase unknown flash type - aborted\n");
		return 1;
	}
	if ((s_first < 0) || (s_first > s_last)) {
		puts ("- no sectors to erase\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot) {
		prints ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else if (flash_verbose) {
		puts ("\n");
	}


	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			switch (info->vendor) {
			case CFI_CMDSET_INTEL_PROG_REGIONS:
			case CFI_CMDSET_INTEL_STANDARD:
			case CFI_CMDSET_INTEL_EXTENDED:
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_CLEAR_STATUS);
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_BLOCK_ERASE);
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_ERASE_CONFIRM);
				break;
			case CFI_CMDSET_AMD_STANDARD:
			case CFI_CMDSET_AMD_EXTENDED:
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect,
						info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect, 0,
						 AMD_CMD_ERASE_SECTOR);
				break;
#ifdef CONFIG_FLASH_CFI_LEGACY
			case CFI_CMDSET_AMD_LEGACY:
				flash_unlock_seq (info, 0);
				flash_write_cmd (info, 0, info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq (info, 0);
				flash_write_cmd (info, sect, 0,
						AMD_CMD_ERASE_SECTOR);
				break;
#endif
			default:
				prints ("Unkown flash vendor %d\n",
				       info->vendor);
				break;
			}

			if (use_flash_status_poll(info)) {
				cfiword_t cword = (cfiword_t)0xffffffffffffffffULL;
				void *dest;
				dest = flash_map(info, sect, 0);
				st = flash_status_poll(info, &cword, dest,
						       info->erase_blk_tout, "erase");
				flash_unmap(info, sect, 0, dest);
			} else
				st = flash_full_status_check(info, sect,
							     info->erase_blk_tout,
							     "erase");
			if (st)
				rcode = 1;
			else if (flash_verbose)
				puts (".");
		}
	}

	if (flash_verbose)
		puts (" done\n");

	return rcode;
}

#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
static int sector_erased(flash_info_t *info, int i)
{
	int k;
	int size;
	uint32_t *flash;

	/*
	 * Check if whole sector is erased
	 */
	size = flash_sector_size(info, i);
	flash = (uint32_t *)info->start[i];
	/* divide by 4 for longword access */
	size = size >> 2;

	for (k = 0; k < size; k++) {
		if (flash_read32(flash++) != 0xffffffff)
			return 0;	/* not erased */
	}

	return 1;			/* erased */
}
#endif /* CONFIG_SYS_FLASH_EMPTY_INFO */

void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	prints ("%s flash (%d x %d)",
		info->name,
		(info->portwidth << 3), (info->chipwidth << 3));
	if (info->size < 1024*1024)
		prints ("  Size: %ld kB in %d Sectors\n",
			info->size >> 10, info->sector_count);
	else
		prints ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);
	prints ("  ");
	switch (info->vendor) {
		case CFI_CMDSET_INTEL_PROG_REGIONS:
			prints ("Intel Prog Regions");
			break;
		case CFI_CMDSET_INTEL_STANDARD:
			prints ("Intel Standard");
			break;
		case CFI_CMDSET_INTEL_EXTENDED:
			prints ("Intel Extended");
			break;
		case CFI_CMDSET_AMD_STANDARD:
			prints ("AMD Standard");
			break;
		case CFI_CMDSET_AMD_EXTENDED:
			prints ("AMD Extended");
			break;
#ifdef CONFIG_FLASH_CFI_LEGACY
		case CFI_CMDSET_AMD_LEGACY:
			prints ("AMD Legacy");
			break;
#endif
		default:
			prints ("Unknown (%d)", info->vendor);
			break;
	}
	prints (" command set, Manufacturer ID: 0x%02X, Device ID: 0x",
		info->manufacturer_id);
	prints (info->chipwidth == FLASH_CFI_16BIT ? "%04X" : "%02X",
		info->device_id);
	if ((info->device_id & 0xff) == 0x7E) {
		prints(info->chipwidth == FLASH_CFI_16BIT ? "%04X" : "%02X",
		info->device_id2);
	}
	prints ("\n  Erase timeout: %ld ms, write timeout: %ld ms\n",
		info->erase_blk_tout,
		info->write_tout);
	if (info->buffer_size > 1) {
		prints ("  Buffer write timeout: %ld ms, "
			"buffer size: %d bytes\n",
		info->buffer_write_tout,
		info->buffer_size);
	}

	puts ("\n  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			puts("\n");
#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
		/* print empty and read-only info */
		prints ("  %08lX %c %s ",
			info->start[i],
			sector_erased(info, i) ? 'E' : ' ',
			info->protect[i] ? "RO" : "  ");
#else	/* ! CONFIG_SYS_FLASH_EMPTY_INFO */
		prints ("  %08lX   %s ",
			info->start[i],
			info->protect[i] ? "RO" : "  ");
#endif
	}
	puts ("\n");
	return;
}

/*-----------------------------------------------------------------------
 * This is used in a few places in write_buf() to show programming
 * progress.  Making it a function is nasty because it needs to do side
 * effect updates to digit and dots.  Repeated code is nasty too, so
 * we define it once here.
 */
#ifdef CONFIG_FLASH_SHOW_PROGRESS
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub) \
	if (flash_verbose) { \
		dots -= dots_sub; \
		if ((scale > 0) && (dots <= 0)) { \
			if ((digit % 5) == 0) \
				prints ("%d", digit / 5); \
			else \
				puts ("."); \
			digit--; \
			dots += scale; \
		} \
	}
#else
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub)
#endif

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uint8_t * src, uint64_t addr, uint64_t cnt)
{
	uint64_t wp;
	uint8_t *p;
	int aln;
	cfiword_t cword;
	int i, rc;
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	int buffered_size;
#endif
#ifdef CONFIG_FLASH_SHOW_PROGRESS
	int digit = CONFIG_FLASH_SHOW_PROGRESS;
	int scale = 0;
	int dots  = 0;

	/*
	 * Suppress if there are fewer than CONFIG_FLASH_SHOW_PROGRESS writes.
	 */
	if (cnt >= CONFIG_FLASH_SHOW_PROGRESS) {
		scale = (int)((cnt + CONFIG_FLASH_SHOW_PROGRESS - 1) /
			CONFIG_FLASH_SHOW_PROGRESS);
	}
#endif

	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	if ((aln = addr - wp) != 0) {
		cword.l = 0;
		p = (uint8_t *)wp;
		for (i = 0; i < aln; ++i)
			flash_add_byte (info, &cword, flash_read8(p + i));

		for (; (i < info->portwidth) && (cnt > 0); i++) {
			flash_add_byte (info, &cword, *src++);
			cnt--;
		}
		for (; (cnt == 0) && (i < info->portwidth); ++i)
			flash_add_byte (info, &cword, flash_read8(p + i));

		rc = flash_write_cfiword (info, wp, cword);
		if (rc != 0)
			return rc;

		wp += i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
	}

	/* handle the aligned part */
#ifdef CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	buffered_size = (info->portwidth / info->chipwidth);
	buffered_size *= info->buffer_size;
	while (cnt >= info->portwidth) {
		/* prohibit buffer write when buffer_size is 1 */
		if (info->buffer_size == 1) {
			cword.l = 0;
			for (i = 0; i < info->portwidth; i++)
				flash_add_byte (info, &cword, *src++);
			if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
				return rc;
			wp += info->portwidth;
			cnt -= info->portwidth;
			continue;
		}

		/* write buffer until next buffered_size aligned boundary */
		i = buffered_size - (wp % buffered_size);
		if (i > cnt)
			i = cnt;
		if ((rc = flash_write_cfibuffer (info, wp, src, i)) != ERR_OK)
			return rc;
		i -= i & (info->portwidth - 1);
		wp += i;
		src += i;
		cnt -= i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
	}
#else
	while (cnt >= info->portwidth) {
		cword.l = 0;
		for (i = 0; i < info->portwidth; i++) {
			flash_add_byte (info, &cword, *src++);
		}
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
		FLASH_SHOW_PROGRESS(scale, dots, digit, info->portwidth);
	}
#endif /* CONFIG_SYS_FLASH_USE_BUFFER_WRITE */

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	cword.l = 0;
	p = (uint8_t *)wp;
	for (i = 0; (i < info->portwidth) && (cnt > 0); ++i) {
		flash_add_byte (info, &cword, *src++);
		--cnt;
	}
	for (; i < info->portwidth; ++i)
		flash_add_byte (info, &cword, flash_read8(p + i));

	return flash_write_cfiword (info, wp, cword);
}

/*-----------------------------------------------------------------------
 * Reverse the order of the erase regions in the CFI QRY structure.
 * This is needed for chips that are either a) correctly detected as
 * top-boot, or b) buggy.
 */
static void cfi_reverse_geometry(struct cfi_qry *qry)
{
	unsigned int i, j;
	uint32_t tmp;

	for (i = 0, j = qry->num_erase_regions - 1; i < j; i++, j--) {
		tmp = qry->erase_region_info[i];
		qry->erase_region_info[i] = qry->erase_region_info[j];
		qry->erase_region_info[j] = tmp;
	}
}

/*-----------------------------------------------------------------------
 * read jedec ids from device and set corresponding fields in info struct
 *
 * Note: assume cfi->vendor, cfi->portwidth and cfi->chipwidth are correct
 *
 */
static void cmdset_intel_read_jedec_ids(flash_info_t *info)
{
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
	udelay(1);
	flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */
	info->manufacturer_id = flash_read_uchar (info,
					FLASH_OFFSET_MANUFACTURER_ID);
	info->device_id = (info->chipwidth == FLASH_CFI_16BIT) ?
			flash_read_word (info, FLASH_OFFSET_DEVICE_ID) :
			flash_read_uchar (info, FLASH_OFFSET_DEVICE_ID);
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}

static int cmdset_intel_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = FLASH_CMD_RESET;

	cmdset_intel_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

#ifdef CONFIG_SYS_FLASH_PROTECTION
	/* read legacy lock/unlock bit from intel flash */
	if (info->ext_addr) {
		info->legacy_unlock = flash_read_uchar (info,
				info->ext_addr + 5) & 0x08;
	}
#endif

	return 0;
}

static void cmdset_amd_read_jedec_ids(flash_info_t *info)
{
	uint16_t bankId = 0;
	uint8_t  manuId;

	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	flash_unlock_seq(info, 0);
	flash_write_cmd(info, 0, info->addr_unlock1, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */

	manuId = flash_read_uchar (info, FLASH_OFFSET_MANUFACTURER_ID);
	/* JEDEC JEP106Z specifies ID codes up to bank 7 */
	while (manuId == FLASH_CONTINUATION_CODE && bankId < 0x800) {
		bankId += 0x100;
		manuId = flash_read_uchar (info,
			bankId | FLASH_OFFSET_MANUFACTURER_ID);
	}
	info->manufacturer_id = manuId;

	switch (info->chipwidth){
	case FLASH_CFI_8BIT:
		info->device_id = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID);
		if (info->device_id == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		break;
	case FLASH_CFI_16BIT:
		info->device_id = flash_read_word (info,
						FLASH_OFFSET_DEVICE_ID);
		if ((info->device_id & 0xff) == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		break;
	default:
		break;
	}
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	udelay(1);
}

static int cmdset_amd_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = AMD_CMD_RESET;

	cmdset_amd_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

	return 0;
}

#ifdef CONFIG_FLASH_CFI_LEGACY
static void flash_read_jedec_ids (flash_info_t * info)
{
	info->manufacturer_id = 0;
	info->device_id       = 0;
	info->device_id2      = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_PROG_REGIONS:
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		cmdset_intel_read_jedec_ids(info);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		cmdset_amd_read_jedec_ids(info);
		break;
	default:
		break;
	}
}

/*-----------------------------------------------------------------------
 * Call board code to request info about non-CFI flash.
 * board_flash_get_legacy needs to fill in at least:
 * info->portwidth, info->chipwidth and info->interface for Jedec probing.
 */
static int flash_detect_legacy(phys_addr_t base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];

	if (board_flash_get_legacy(base, banknum, info)) {
		/* board code may have filled info completely. If not, we
		   use JEDEC ID probing. */
		if (!info->vendor) {
			int modes[] = {
				CFI_CMDSET_AMD_STANDARD,
				CFI_CMDSET_INTEL_STANDARD
			};
			int i;

			for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
				info->vendor = modes[i];
				info->start[0] = (uint64_t)base;

				if (info->portwidth == FLASH_CFI_8BIT
					&& info->interface == FLASH_CFI_X8X16) {
					info->addr_unlock1 = 0x2AAA;
					info->addr_unlock2 = 0x5555;
				} else {
					info->addr_unlock1 = 0x5555;
					info->addr_unlock2 = 0x2AAA;
				}
				flash_read_jedec_ids(info);
				prints("JEDEC PROBE: ID %x %x %x\n",
						info->manufacturer_id,
						info->device_id,
						info->device_id2);
				if (jedec_flash_match(info, info->start[0]))
					break;

			}
		}

		switch(info->vendor) {
		case CFI_CMDSET_INTEL_PROG_REGIONS:
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			info->cmd_reset = FLASH_CMD_RESET;
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
		case CFI_CMDSET_AMD_LEGACY:
			info->cmd_reset = AMD_CMD_RESET;
			break;
		}
		info->flash_id = FLASH_MAN_CFI;
		return 1;
	}
	return 0; /* use CFI */
}
#else
static inline int flash_detect_legacy(phys_addr_t base, int banknum)
{
	return 0; /* use CFI */
}
#endif

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 */
static void flash_read_cfi (flash_info_t *info, void *buf,
		unsigned int start, size_t len)
{
	uint8_t *p = buf;
	unsigned int i;

	for (i = 0; i < len; i++)
		p[i] = flash_read_uchar(info, start + i);
}

void __flash_cmd_reset(flash_info_t *info)
{
	/*
	 * We do not yet know what kind of commandset to use, so we issue
	 * the reset command in both Intel and AMD variants, in the hope
	 * that AMD flash roms ignore the Intel command.
	 */
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	udelay(1);
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}
void flash_cmd_reset(flash_info_t *info)
	__attribute__((weak,alias("__flash_cmd_reset")));

static int __flash_detect_cfi (flash_info_t * info, struct cfi_qry *qry)
{
	int cfi_offset;

	/* Issue FLASH reset command */
	flash_cmd_reset(info);

	for (cfi_offset=0;
	     cfi_offset < sizeof(flash_offset_cfi) / sizeof(uint32_t);
	     cfi_offset++) {
		flash_write_cmd (info, 0, flash_offset_cfi[cfi_offset],
				 FLASH_CMD_CFI);
		if (flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP, 'Q')
		    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 1, 'R')
		    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 2, 'Y')) {
			flash_read_cfi(info, qry, FLASH_OFFSET_CFI_RESP,
					sizeof(struct cfi_qry));
			info->interface	= qry->interface_desc;

			info->cfi_offset = flash_offset_cfi[cfi_offset];
			prints ("device interface is %d\n",
			       info->interface);
			prints ("found port %d chip %d ",
			       info->portwidth, info->chipwidth);
			prints ("port %d bits chip %d bits\n",
			       info->portwidth << CFI_FLASH_SHIFT_WIDTH,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);

			/* calculate command offsets as in the Linux driver */
			info->addr_unlock1 = 0x555;
			info->addr_unlock2 = 0x2aa;

			/*
			 * modify the unlock address if we are
			 * in compatibility mode
			 */
			if (	/* x8/x16 in x8 mode */
				((info->chipwidth == FLASH_CFI_BY8) &&
					(info->interface == FLASH_CFI_X8X16)) ||
				/* x16/x32 in x16 mode */
				((info->chipwidth == FLASH_CFI_BY16) &&
					(info->interface == FLASH_CFI_X16X32)))
			{
				info->addr_unlock1 = 0xaaa;
				info->addr_unlock2 = 0x555;
			}

			info->name = "CFI conformant";
			return 1;
		}
	}

	return 0;
}

static int flash_detect_cfi (flash_info_t * info, struct cfi_qry *qry)
{
	prints ("flash detect cfi\n");

	for (info->portwidth = CONFIG_SYS_FLASH_CFI_WIDTH;
	     info->portwidth <= FLASH_CFI_64BIT; info->portwidth <<= 1) {
		for (info->chipwidth = FLASH_CFI_BY8;
		     info->chipwidth <= info->portwidth;
		     info->chipwidth <<= 1)
			if (__flash_detect_cfi(info, qry))
				return 1;
	}
	prints ("not found\n");
	return 0;
}

/*
 * Manufacturer-specific quirks. Add workarounds for geometry
 * reversal, etc. here.
 */
static void flash_fixup_amd(flash_info_t *info, struct cfi_qry *qry)
{
	/* check if flash geometry needs reversal */
	if (qry->num_erase_regions > 1) {
		/* reverse geometry if top boot part */
		if (info->cfi_version < 0x3131) {
			/* CFI < 1.1, try to guess from device id */
			if ((info->device_id & 0x80) != 0)
				cfi_reverse_geometry(qry);
		} else if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			cfi_reverse_geometry(qry);
		}
	}
}

static void flash_fixup_atmel(flash_info_t *info, struct cfi_qry *qry)
{
	int reverse_geometry = 0;

	/* Check the "top boot" bit in the PRI */
	if (info->ext_addr && !(flash_read_uchar(info, info->ext_addr + 6) & 1))
		reverse_geometry = 1;

	/* AT49BV6416(T) list the erase regions in the wrong order.
	 * However, the device ID is identical with the non-broken
	 * AT49BV642D they differ in the high byte.
	 */
	if (info->device_id == 0xd6 || info->device_id == 0xd2)
		reverse_geometry = !reverse_geometry;

	if (reverse_geometry)
		cfi_reverse_geometry(qry);
}

static void flash_fixup_stm(flash_info_t *info, struct cfi_qry *qry)
{
	/* check if flash geometry needs reversal */
	if (qry->num_erase_regions > 1) {
		/* reverse geometry if top boot part */
		if (info->cfi_version < 0x3131) {
			/* CFI < 1.1, guess by device id */
			if (info->device_id == 0x22CA || /* M29W320DT */
			    info->device_id == 0x2256 || /* M29W320ET */
			    info->device_id == 0x22D7) { /* M29W800DT */
				cfi_reverse_geometry(qry);
			}
		} else if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			cfi_reverse_geometry(qry);
		}
	}
}

/*
 * The following code cannot be run from FLASH!
 *
 */
uint64_t flash_get_size (phys_addr_t base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int i, j;
	flash_sect_t sect_cnt;
	phys_addr_t sector;
	unsigned long tmp;
	int size_ratio;
	uint8_t num_erase_regions;
	int erase_region_size;
	int erase_region_count;
	struct cfi_qry qry;
	unsigned long max_size;

	memset(&qry, 0, sizeof(qry));

	info->ext_addr = 0;
	info->cfi_version = 0;
#ifdef CONFIG_SYS_FLASH_PROTECTION
	info->legacy_unlock = 0;
#endif

	info->start[0] = (uint64_t)base;

	if (flash_detect_cfi (info, &qry)) {
		info->vendor = qry.p_id;
		info->ext_addr = qry.p_adr;
		num_erase_regions = qry.num_erase_regions;

		if (info->ext_addr) {
			info->cfi_version = (uint16_t) flash_read_uchar (info,
						info->ext_addr + 3) << 8;
			info->cfi_version |= (uint16_t) flash_read_uchar (info,
						info->ext_addr + 4);
		}

#ifdef prints
		flash_printqry (&qry);
#endif

		switch (info->vendor) {
		case CFI_CMDSET_INTEL_PROG_REGIONS:
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			cmdset_intel_init(info, &qry);
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			cmdset_amd_init(info, &qry);
			break;
		default:
			prints("CFI: Unknown command set 0x%x\n",
					info->vendor);
			/*
			 * Unfortunately, this means we don't know how
			 * to get the chip back to Read mode. Might
			 * as well try an Intel-style reset...
			 */
			flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
			return 0;
		}

		/* Do manufacturer-specific fixups */
		switch (info->manufacturer_id) {
		case 0x0001: /* AMD */
		case 0x0037: /* AMIC */
			flash_fixup_amd(info, &qry);
			break;
		case 0x001f:
			flash_fixup_atmel(info, &qry);
			break;
		case 0x0020:
			flash_fixup_stm(info, &qry);
			break;
		}

		prints ("manufacturer is %d\n", info->vendor);
		prints ("manufacturer id is 0x%x\n", info->manufacturer_id);
		prints ("device id is 0x%x\n", info->device_id);
		prints ("device id2 is 0x%x\n", info->device_id2);
		prints ("cfi version is 0x%04x\n", info->cfi_version);

		size_ratio = info->portwidth / info->chipwidth;
		/* if the chip is x8/x16 reduce the ratio by half */
		if ((info->interface == FLASH_CFI_X8X16)
		    && (info->chipwidth == FLASH_CFI_BY8)) {
			size_ratio >>= 1;
		}
		prints ("size_ratio %d port %d bits chip %d bits\n",
		       size_ratio, info->portwidth << CFI_FLASH_SHIFT_WIDTH,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		info->size = 1 << qry.dev_size;
		/* multiply the size by the number of chips */
		info->size *= size_ratio;
		max_size = cfi_flash_bank_size(banknum);
		if (max_size && (info->size > max_size)) {
			prints("[truncated from %ldMiB]", info->size >> 20);
			info->size = max_size;
		}
		prints ("found %d erase regions\n", num_erase_regions);
		sect_cnt = 0;
		sector = base;
		for (i = 0; i < num_erase_regions; i++) {
			if (i > NUM_ERASE_REGIONS) {
				prints ("%d erase regions found, only %d used\n",
					num_erase_regions, NUM_ERASE_REGIONS);
				break;
			}

			tmp = qry.erase_region_info[i];
			prints("erase region %u: 0x%08lx\n", i, tmp);

			erase_region_count = (tmp & 0xffff) + 1;
			tmp >>= 16;
			erase_region_size =
				(tmp & 0xffff) ? ((tmp & 0xffff) * 256) : 128;
			prints ("erase_region_count = %d erase_region_size = %d\n",
				erase_region_count, erase_region_size);
			for (j = 0; j < erase_region_count; j++) {
				if (sector - base >= info->size)
					break;
				if (sect_cnt >= CONFIG_SYS_MAX_FLASH_SECT) {
					prints("ERROR: too many flash sectors\n");
					break;
				}
				info->start[sect_cnt] = (uint64_t)sector;
				sector += (erase_region_size * size_ratio);

				/*
				 * Only read protection status from
				 * supported devices (intel...)
				 */
				switch (info->vendor) {
				case CFI_CMDSET_INTEL_PROG_REGIONS:
				case CFI_CMDSET_INTEL_EXTENDED:
				case CFI_CMDSET_INTEL_STANDARD:
					/*
					 * Set flash to read-id mode. Otherwise
					 * reading protected status is not
					 * guaranteed.
					 */
					flash_write_cmd(info, sect_cnt, 0,
							FLASH_CMD_READ_ID);
					info->protect[sect_cnt] =
						flash_isset (info, sect_cnt,
							     FLASH_OFFSET_PROTECT,
							     FLASH_STATUS_PROTECT);
					break;
				default:
					/* default: not protected */
					info->protect[sect_cnt] = 0;
				}

				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		info->buffer_size = 1 << qry.max_buf_write_size;
		tmp = 1 << qry.block_erase_timeout_typ;
		info->erase_blk_tout = tmp *
			(1 << qry.block_erase_timeout_max);
		tmp = (1 << qry.buf_write_timeout_typ) *
			(1 << qry.buf_write_timeout_max);

		/* round up when converting to ms */
		info->buffer_write_tout = (tmp + 999) / 1000;
		tmp = (1 << qry.word_write_timeout_typ) *
			(1 << qry.word_write_timeout_max);
		/* round up when converting to ms */
		info->write_tout = (tmp + 999) / 1000;
		info->flash_id = FLASH_MAN_CFI;
		if ((info->interface == FLASH_CFI_X8X16) &&
		    (info->chipwidth == FLASH_CFI_BY8)) {
			/* XXX - Need to test on x8/x16 in parallel. */
			info->portwidth >>= 1;
		}

		flash_write_cmd (info, 0, 0, info->cmd_reset);
	}

	return (info->size);
}

static void cfi_flash_set_config_reg(uint32_t base, uint16_t val)
{
#ifdef CONFIG_SYS_CFI_FLASH_CONFIG_REGS
	/*
	 * Only set this config register if really defined
	 * to a valid value (0xffff is invalid)
	 */
	if (val == 0xffff)
		return;

	/*
	 * Set configuration register. Data is "encrypted" in the 16 lower
	 * address bits.
	 */
	flash_write16(FLASH_CMD_SETUP, (void *)(base + (val << 1)));
	flash_write16(FLASH_CMD_SET_CR_CONFIRM, (void *)(base + (val << 1)));

	/*
	 * Finally issue reset-command to bring device back to
	 * read-array mode
	 */
	flash_write16(FLASH_CMD_RESET, (void *)base);
#endif
}

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

#ifdef CONFIG_SYS_FLASH_PROTECTION
	/* read environment from EEPROM */
	char s[64];
	getenv_f("unlock", s, sizeof(s));
#endif

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;

		/* Optionally write flash configuration register */
		cfi_flash_set_config_reg(cfi_flash_bank_addr(i),
					 cfi_flash_config_reg(i));

		if (!flash_detect_legacy(cfi_flash_bank_addr(i), i))
			flash_get_size(cfi_flash_bank_addr(i), i);
		size += flash_info[i].size;
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifndef CONFIG_SYS_FLASH_QUIET_TEST
			prints ("## Unknown flash on Bank %d "
				"- Size = 0x%08lx = %ld MB\n",
				i+1, flash_info[i].size,
				flash_info[i].size >> 20);
#endif /* CONFIG_SYS_FLASH_QUIET_TEST */
		}
	}

	return (size);
}
