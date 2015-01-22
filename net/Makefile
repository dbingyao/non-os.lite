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

net-objs := $(patsubst %.c,%.o,$(wildcard *.c))

depfiles := $(net-objs:.o=.c)

Makefile.depend: $(depfiles)
	@find . -name '*.depend' | xargs rm -f
	@echo "Generating Makefile.depend for net/ ... $(depfiles)"
	@for f in $(depfiles); do \
		$(CC) -MM -MG $(CPPFLAGS) $(CFLAGS) -MT $$f $$f | sed 's/.c:/.o:/g' >> Makefile.depend; \
	done

-include Makefile.depend
.PHONY: all
all: net.a

net.a: $(net-objs)
	$(AR) -rs $@ $^
	$(OD) -a $@ > net.archive
