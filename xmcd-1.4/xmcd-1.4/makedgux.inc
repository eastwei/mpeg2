#
#   @(#)makedgux.inc	5.3 94/12/28
#
#   make.inc for xmcd, cda and libdi (DG/UX AViiON-specific version)
#
#	xmcd  - Motif(tm) CD Audio Player
#	cda   - Command-line CD Audio Player
#	libdi - CD Audio Player Device Interface Library
#
#   NOTE: You should use this only if you don't have imake!
#
#   Copyright (C) 1995  Ti Kan
#   E-mail: ti@amb.org
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

#
# Modify these as you see fit.  You will most certainly need to
# make some changes here in order to successfully build xmcd/cda
# on your platform.
#
SHELL=		/bin/sh
MAKE=		make
CC=		gcc
RANLIB=		/bin/true
AR=		ar cq
RM=		rm -f
CFLAGS=		-O $(DEFINES) $(INCDIRS) $(ARCH) -fwritable-strings
# ARCH=
INCDIRS=	-I..
XLIBS=		-lXm -lXt -lX11 -lPW
CURSESLIB=	-lcurses
BINDIR=		/usr/bin/X11
LIBDIR=		/usr/lib/X11
MANDIR=		/usr/man/man1
MANSUF=		.1
