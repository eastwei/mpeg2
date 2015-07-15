/*
 *   cda - Command-line CD Audio Player
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
#ifndef __CDA_H__
#define __CDA_H__

#ifndef LINT
static char *_cda_h_ident_ = "@(#)cda.h	5.3 95/01/27";
#endif


/* Program name string */
#define PROGNAME		"cda"


/* Copyright message */
#define COPYRIGHT "Copyright (C) 1995 Ti Kan\n\
E-mail: ti@amb.org\n\n\
This is free software and comes with no warranty.\n\
See the GNU General Public License for details.\n"


/* Memory allocator defines */
#define MEM_ALLOC		malloc
#define MEM_REALLOC		realloc
#define MEM_CALLOC		calloc
#define MEM_FREE		free


/* CDA packet magic number */
#define CDA_MAGIC		0x1cda


/* CDA commands */
#define CDA_ON			0x100
#define CDA_OFF			0x101
#define CDA_DISC		0x102
#define CDA_LOCK		0x103
#define CDA_PLAY		0x104
#define CDA_PAUSE		0x105
#define CDA_STOP		0x106
#define CDA_TRACK		0x107
#define CDA_INDEX		0x108
#define CDA_VOLUME		0x109
#define CDA_BALANCE		0x10a
#define CDA_ROUTE		0x10b
#define CDA_PROGRAM		0x10c
#define CDA_SHUFFLE		0x10d
#define CDA_REPEAT		0x10e
#define CDA_STATUS		0x10f
#define CDA_TOC			0x110
#define CDA_EXTINFO		0x111
#define CDA_DEVICE		0x112
#define CDA_VERSION		0x113


/* CDA return code */
#define CDA_UNKNOWN		0x00
#define CDA_OK			0x01
#define CDA_INVALID		0x02
#define CDA_PARMERR		0x03
#define CDA_FAILED		0x04


/* Max number of 32-bit arguments */
#define CDA_NARGS		101


/* CDA_STAT return argument macros */
#define RD_ARG_MODE(x)		(((x) & 0xff000000) >> 24)	/* arg[0] */

#define RD_ARG_LOCK(x)		((x) & 0x00010000)		/* arg[0] */
#define RD_ARG_SHUF(x)		((x) & 0x00020000)		/* arg[0] */
#define RD_ARG_PROG(x)		((x) & 0x00040000)		/* arg[0] */
#define RD_ARG_REPT(x)		((x) & 0x00080000)		/* arg[0] */

#define RD_ARG_SEC(x)		((x) & 0xff)			/* arg[1] */
#define RD_ARG_MIN(x)		(((x) >> 8) & 0xff)		/* arg[1] */
#define RD_ARG_IDX(x)		(((x) >> 16) & 0xff)		/* arg[1] */
#define RD_ARG_TRK(x)		(((x) >> 24) & 0xff)		/* arg[1] */

#define WR_ARG_MODE(x,v)	((x) |= (((v) & 0xff) << 24))	/* arg[0] */

#define WR_ARG_LOCK(x)		((x) |= 0x00010000)		/* arg[0] */
#define WR_ARG_SHUF(x)		((x) |= 0x00020000)		/* arg[0] */
#define WR_ARG_PROG(x)		((x) |= 0x00040000)		/* arg[0] */
#define WR_ARG_REPT(x)		((x) |= 0x00080000)		/* arg[0] */

#define WR_ARG_SEC(x,v)		((x) |= ((v) & 0xff))		/* arg[1] */
#define WR_ARG_MIN(x,v)		((x) |= (((v) & 0xff) << 8))	/* arg[1] */
#define WR_ARG_IDX(x,v)		((x) |= (((v) & 0xff) << 16))	/* arg[1] */
#define WR_ARG_TRK(x,v)		((x) |= (((v) & 0xff) << 24))	/* arg[1] */


/* CDA_TOC return argument macros */
#define RD_ARG_TOC(x,t,p,m,s)	{	\
	(p) = (bool_t) ((x) >> 24);	\
	((t) = ((x) >> 16) & 0xff);	\
	((m) = ((x) >> 8) & 0xff);	\
	((s) = (x) & 0xff);		\
}								/* arg[n] */

#define WR_ARG_TOC(x,t,p,m,s)	{	\
	if ((p) == (t))			\
		(x) |= 1 << 24;		\
	(x) |= (((t) & 0xff) << 16);	\
	(x) |= (((m) & 0xff) << 8);	\
	(x) |= ((s) & 0xff);		\
}								/* arg[n] */


/* CD database information */
typedef struct database {
	word32_t	discid;			/* Magic disc ID */
	char		category[STR_BUF_SZ];	/* CD category */
	char		*dtitle;		/* Disc title */
	char		*trklist[MAXTRACK];	/* Track title list */
	char		*extd;			/* Extended disc info */
	char		*extt[MAXTRACK];	/* Extended track info */
	char		*playorder;		/* Track play order */
} database_t;


/* CDA pipe protocol packet */
typedef struct cda_pkt {
	word32_t	magic;
	word32_t	pktid;
	word32_t	cmd;
	word32_t	retcode;
	word32_t	arg[CDA_NARGS];
} cdapkt_t;


/* Public function prototypes */
extern curstat_t	*curstat_addr(void);
extern bool_t		cda_sendcmd(word32_t, word32_t []);
extern bool_t		dbprog_dbload(word32_t);
extern int		cda_daemon(curstat_t *);

#endif	/* __CDA_H__ */
