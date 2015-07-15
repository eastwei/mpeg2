/*
 *   libdi - CD Audio Player Device Interface Library
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
 *   This software module contains code that interfaces the CD player
 *   application to the SCO Open Desktop operating system.  The name
 *   "SCO" and "ODT" are used here for identification purposes only.
 *   This software and its author are not affiliated with The Santa Cruz
 *   Operation, Inc.
 */
#ifndef __OS_ODT_H__
#define __OS_ODT_H__

#if defined(sco) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_odt_h_ident_ = "@(#)os_odt.h	5.4 94/12/28";
#endif

#include <sys/scsi.h>
#include <sys/scsicmd.h>


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


/* Macros to update various fields of the SCSI CDB structures.
 * These are used to work around byte alignment restrictions and
 * padding with some compilers.  In general, define NO_ALIGN_LIMIT
 * if and only if you are sure that the compiler will not insert
 * pad bytes between any field of the SCSI CDB structure.
 */

#ifdef NO_ALIGN_LIMIT

#define CDB6_BLK(a,d)	(a)->data[0] = (byte_t) (d);		\
			(a)->data[1] = (byte_t) ((d) >> 8);
#define CDB6_LEN(a,d)	(a)->data[2] = (d)
#define CDB6_CTL(a,d)	(a)->control = (d)
#define CDB10_BLK(a,d)	(a)->block = (d)
#define CDB10_LEN(a,d)	(a)->length = (d)
#define CDB10_RSV(a,d)	(a)->reserved = (d)
#define CDB10_CTL(a,d)	(a)->control = (d)
#define CDB12_BLK(a,d)	(a)->block = (d)
#define CDB12_LEN(a,d)	(a)->length = (d)
#define CDB12_RSV(a,d)	(a)->reserved = (d)
#define CDB12_CTL(a,d)	(a)->control = (d)

#else	/* !NO_ALIGN_LIMIT */

#if _BYTE_ORDER_ == _L_ENDIAN_

#define CDB6_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) (a);	\
	p[1] = (d);						\
}
#define CDB6_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[4] = (d);						\
}
#define CDB6_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[5] = (d);						\
}
#define CDB10_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) (a);	\
	p[1] = (word16_t) (d);					\
	p[2] = (word16_t) ((d) >> 16);				\
}
#define CDB10_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[7] = (byte_t) (d);					\
	p[8] = (byte_t) ((d) >> 8);				\
}
#define CDB10_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[6] = (d);						\
}
#define CDB10_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[9] = (d);						\
}
#define CDB12_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) (a);	\
	p[1] = (word16_t) (d);					\
	p[2] = (word16_t) ((d) >> 16);				\
}
#define CDB12_LEN(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) (a);	\
	p[3] = (word16_t) (d);					\
	p[4] = (word16_t) ((d) >> 16);				\
}
#define CDB12_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[10] = (d);						\
}
#define CDB12_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) (a);			\
	p[11] = (d);						\
}

#endif	/* _BYTE_ORDER_ == _L_ENDIAN_ */

#endif	/* !NO_ALIGN_LIMIT */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* sco DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_ODT_H__ */

