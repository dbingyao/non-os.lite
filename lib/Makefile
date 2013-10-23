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

include $(TOP_DIR)/config.mk

lib-objs := $(patsubst %.c,%.o,$(wildcard *.c))

libm-objs := $(patsubst %.c,%.o,$(wildcard mylibm/*.c))

subdirs-libc := $(addprefix mylibc/, stdio stdlib string)
subdirs-libc += mylibc
libc-objs := $(foreach dir,$(subdirs-libc),$(patsubst %.c,%.o,$(wildcard $(dir)/*.c)))

depfiles := $(lib-objs:.o=.c)
depfiles += $(libm-objs:.o=.c)
depfiles += $(libc-objs:.o=.c)

Makefile.depend: $(depfiles)
	@find . -name '*.depend' | xargs rm -f
	@echo "Generating Makefile.depend for lib/ ... $(depfiles)"
	@for f in $(depfiles); do \
		$(CC) -MM -MG $(CPPFLAGS) $(CFLAGS) -MT $$f $$f | sed 's/.c:/.o:/g' >> Makefile.depend; \
	done

libc-objs += $(patsubst %.S,%.o,$(wildcard mylibc/string/*.S))

-include Makefile.depend

.PHONY: all
all: pr_objs libgen.a libm.a libc.a

pr_objs:
#	@echo "libm objs: $(libm-objs)"
	@echo "libc objs: $(libc-objs)"

libgen.a: $(lib-objs)
	$(AR) -rs $@ $^
	$(OD) -a $@ > libgen.archive
	$(AR) -rs libUtils.a $^

libm.a: $(libm-objs)
	$(AR) -rs $@ $^
	$(OD) -a $@ > libm.archive
	$(AR) -rs libUtils.a $^

libc.a: $(libc-objs)
	$(AR) -rs $@ $^
	$(OD) -a $@ > libc.archive
	$(AR) -rs libUtils.a $^
	$(OD) -a $@ > libUtils.archive

