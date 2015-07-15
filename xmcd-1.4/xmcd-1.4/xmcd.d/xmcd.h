/*
 *   xmcd - Motif(tm) CD Audio Player
 *
 *   Copyright (C) 1995  Ti Kan
 *   E-mail: ti@amb.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef __XMCD_H__
#define __XMCD_H__

#ifndef LINT
static char *_xmcd_h_ident_ = "@(#)xmcd.h	5.2 94/12/28";
#endif


/* Program name string */
#define PROGNAME	"xmcd"


/* Memory allocator defines */
#define MEM_ALLOC	XtMalloc
#define MEM_REALLOC	XtRealloc
#define MEM_CALLOC	XtCalloc
#define MEM_FREE	XtFree


/* Character set/font */
#define CHSET1		"chset1"
#define CHSET2		"chset2"
#define CHSET3		"chset3"
#define CHSET4		"chset4"


/* Public function prototypes */
extern curstat_t	*curstat_addr(void);

#endif	/* __XMCD_H__ */
