/*
 *   util.h - Common utility routines for xmcd, cda and libdi.
 *
 *   xmcd  - Motif(tm) CD Audio Player
 *   cda   - Command-line CD Audio Player
 *   libdi - CD Audio Player Device Interface Library
 *
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
#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef LINT
static char *_util_h_ident_ = "@(#)util.h	5.3 94/12/28";
#endif


/* Utility macros */
#define SQR(x)		((x) * (x))	/* Compute the square of a number */
#define DBGPRN		if (app_data.debug) fprintf
#define DBGDUMP		if (app_data.debug) dbgdump


/* Public function prototypes */
extern void		util_init(void);
extern uid_t		get_ouid(void);
extern gid_t		get_ogid(void);
extern sword32_t	ltobcd(sword32_t);
extern sword32_t	bcdtol(sword32_t);
extern bool_t		stob(char *);
extern char		*basename(char *);
extern char		*dirname(char *);
extern char		*homedir(uid_t);
extern char		*uhomedir(char *);
extern int		isqrt(int);
extern void		blktomsf(word32_t, byte_t *, byte_t *, byte_t *,
				 word32_t);
extern void		msftoblk(byte_t, byte_t, byte_t, word32_t *, word32_t);
extern word16_t		bswap16(word16_t);
extern word32_t		bswap24(word32_t);
extern word32_t		bswap32(word32_t);
extern word16_t		lswap16(word16_t);
extern word32_t		lswap24(word32_t);
extern word32_t		lswap32(word32_t);
extern void		dbgdump(char *, byte_t *, int);

#endif	/* __UTIL_H__ */

