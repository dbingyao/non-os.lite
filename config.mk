###
 # (C) Copyright 2013 Faraday Technology
 # BingYao Luo <bjluo@faraday-tech.com>
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 2 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program; if not, write to the Free Software
 # Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ##

# AHB, AXI, or A320
PLATFORM=AXI

# ROM_NOR,ROM_SPI or RAM
IMAGE=RAM

#text or data
COPYSECT=text

CROSS_COMPILE		= arm-none-eabi-
export CC	= $(CROSS_COMPILE)gcc
export AS	= $(CROSS_COMPILE)as
export LD	= $(CROSS_COMPILE)ld
export AR	= $(CROSS_COMPILE)ar
export OBJCOPY	= $(CROSS_COMPILE)objcopy
export OD	= $(CROSS_COMPILE)objdump

CFLAGS := -g -mfloat-abi=soft
CPPFLAGS := -I$(TOP_DIR)/include -I$(TOP_DIR)/include/linux -I$(TOP_DIR)/include/libc \
	    -I$(TOP_DIR)/include/asm -isystem $(shell $(CC) -print-file-name=include)

LDFLAGS := -LBIN -g -v -nostartfiles -nostdlib
ASFLAGS := -g -mfloat-abi=soft

CFLAGS += -DPLATFORM_$(PLATFORM)

CFLAGS += -D$(IMAGE)

ifeq ($(IMAGE), RAM)
else
ifeq ($(COPYSECT), data)
CFLAGS += -DCOPY_DATA_ONLY
endif
endif

ifeq ($(PLATFORM), A320)
CFLAGS += -mcpu=fa526
LDFLAGS += --fix-v4bx
endif

%.o: %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

#Drivers
CONFIG_FTTMR010=y
CONFIG_FTUART=y
CONFIG_FTINTC010=y
CONFIG_FTSPI020=n
CONFIG_FTSATA100=y
CONFIG_FTGMAC100=y
CONFIG_FTSMC020=n
CONFIG_FTDMAC020=n
CONFIG_FTDMAC030=y

#SATA100 at AXI platform does not have LED
ifeq ($(CONFIG_FTSATA100), y)
CFLAGS += -DCONFIG_FTSATA100
endif

ifeq ($(CONFIG_FTSPI020), y)
CFLAGS += -DCONFIG_FTSPI020
endif

ifeq ($(PLATFORM), AXI)
CONFIG_FTDMAC030=y
else
CONFIG_FTDMAC020=y
endif
