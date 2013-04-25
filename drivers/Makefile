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

subdirs-y = console timer interrupt dma ftspi020 ftsmc020 cfi

incl-makefile := $(patsubst %,%/Makefile ,$(subdirs-y))

drvobjs-y := 
include $(incl-makefile)

depfiles := $(drvobjs-y:.o=.c)

Makefile.depend: $(depfiles)
	@find . -name '*.depend' | xargs rm -f
	@echo "Generating Makefile.depend for drivers/ ... $(depfiles)"
	@for f in $(depfiles); do \
		$(CC) -MM -MG $(CPPFLAGS) $(CFLAGS) -MT $$f $$f | sed 's/.c:/.o:/g' >> Makefile.depend; \
	done

-include Makefile.depend

.PHONY: all
all: drivers.a

drivers.a: $(drvobjs-y)
	$(AR) -rs $@ $^
	$(OD) -a $@ > drivers.archive
