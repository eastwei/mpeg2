#
#  @(#) Makefile 1.9, last edit: 6/15/94 16:52:01
#  @(#) Copyright (C) 1993, 1994 Tobias Bading (bading@cs.tu-berlin.de)
#  @(#) Berlin University of Technology
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

#
#  Changes from 1.1 to 1.2:
#    - dependencies described directly, no more makedepend required
#    - all .c source files rename to .cc files
#    - strip command included after a successful compilation
#    - this makefile is now "Star Trek The Next Generation" compatible!
#


CFLAGS = $(COMPILERFLAGS) -DULAW
# debugging: -ggdb -DDEBUG
# profiling: -pg (gprof) or -p (prof)
# strip command has to be removed for debugging or profiling


OBJS = maplay.o ibitstream.o header.o scalefactors.o subband_layer_1.o subband_layer_2.o \
       synthesis_filter.o obuffer.o crc.o ulaw.o


.SUFFIXES:	.cc

install:
		@./configuration.sh

all:		maplay2

maplay2:		$(OBJS)
		$(COMPILER) $(CFLAGS) $(OBJS) -o $@ $(LIBRARIES) -lm
		-strip $@

.cc.o:
		$(COMPILER) -c $(CFLAGS) $(INCLUDEDIRS) $*.cc -o $@

depend:
		makedepend $(INCLUDEDIRS) -w100 *.cc

clean:
		-rm -f maplay2 makedepend_done audio_includes.h Makefile.bak
		-rm -f *.o

tags:
		-rm -f TAGS
		etags *.cc *.h  Makefile configuration.sh

it:
		@echo >/dev/null
so:
		@echo I\'ll do my very best, Captain.
		@make install
		@echo Further orders, Sir\?


crc.o:			all.h crc.h
header.o:		all.h crc.h header.h ibitstream.h
ibitstream.o:		all.h ibitstream.h
maplay.o:		all.h audio_includes.h crc.h header.h ibitstream.h obuffer.h \
			scalefactors.h subband.h subband_layer_1.h subband_layer_2.h \
			synthesis_filter.h ulaw.h
scalefactors.o:		all.h scalefactors.h
ulaw.o:			all.h ulaw.h
obuffer.o:		all.h audio_includes.h crc.h header.h ibitstream.h obuffer.h ulaw.h
subband_layer_1.o:	all.h audio_includes.h crc.h header.h ibitstream.h obuffer.h \
			scalefactors.h subband.h subband_layer_1.h synthesis_filter.h ulaw.h
subband_layer_2.o:	all.h audio_includes.h crc.h header.h ibitstream.h obuffer.h \
			scalefactors.h subband.h subband_layer_2.h synthesis_filter.h ulaw.h
synthesis_filter.o:	all.h audio_includes.h crc.h header.h ibitstream.h obuffer.h \
			synthesis_filter.h ulaw.h


# DO NOT DELETE THIS LINE -- make depend depends on it.
