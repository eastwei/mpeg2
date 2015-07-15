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
 */
#ifndef __OS_SVR4_H__
#define __OS_SVR4_H__

#if defined(SVR4) && !defined(sun) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_svr4_h_ident_ = "@(#)os_svr4.h	5.4 94/12/28";
#endif

#if defined(i386) || (defined(_FTX) && defined(__hppa))
/*
 *   USL UNIX SVR4/x86 support
 *   Stratus UNIX SVR4/PA-RISC FTX 3.x support
 *   Portable Device Interface/SCSI Device Interface
 *
 *   This software fragment contains code that interfaces the
 *   CD player application to the UNIX System V Release 4
 *   operating system for the Intel x86 hardware platforms and
 *   Stratus PA-RISC systems.  The name "USL", "UNIX", "Intel",
 *   "Stratus" and "PA-RISC" are used here for identification
 *   purposes only.
 */

#include <sys/scsi.h>
#include <sys/sdi_edt.h>
#include <sys/sdi.h>


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


#ifndef SCL_AD

/* SCSI 12-byte CDB */
#define SCL_AD(x)	((byte_t *) (x) + 2)
#define SCL_SZ		12

struct scl {
	unsigned int	sl_pad0:16;	/* pad for alignment */
	unsigned int	sl_op:8;	/* Opcode */
	unsigned int	sl_res1:5;	/* Reserved field */
	unsigned int	sl_lun:3;	/* Logical unit number */

	word32_t	sl_addr;	/* Block address */
	word32_t	sl_len;		/* Transfer length */

	byte_t		sl_res2;	/* Reserved field */
	byte_t		sl_cont;	/* Control byte */
};
 
#endif	/* SCL_AD */


/* SCSI Command Descriptor Block union */
union scsi_cdb {
	struct scs	scs;
	struct scm	scm;
	struct scl	scl;
};


/* Macros to update various fields of the SCSI CDB structures.
 * These are used to work around byte alignment restrictions and
 * padding with some compilers.  In general, define NO_ALIGN_LIMIT
 * if and only if you are sure that the compiler will not insert
 * pad bytes between any field of the SCSI CDB structure.
 */

#ifdef NO_ALIGN_LIMIT

#define CDB6_BLK(a,d)	(a)->ss_addr = (d)
#define CDB6_LEN(a,d)	(a)->ss_len = (d)
#define CDB6_CTL(a,d)	(a)->ss_cont = (d)
#define CDB10_BLK(a,d)	(a)->sm_addr = (d)
#define CDB10_LEN(a,d)	(a)->sm_len = (d)
#define CDB10_RSV(a,d)	(a)->sm_res2 = (d)
#define CDB10_CTL(a,d)	(a)->sm_cont = (d)
#define CDB12_BLK(a,d)	(a)->sl_addr = (d)
#define CDB12_LEN(a,d)	(a)->sl_len = (d)
#define CDB12_RSV(a,d)	(a)->sl_res2 = (d)
#define CDB12_CTL(a,d)	(a)->sl_cont = (d)

#else	/* !NO_ALIGN_LIMIT */

#if _BYTE_ORDER_ == _L_ENDIAN_

#define CDB6_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCS_AD(a);	\
	p[1] = (d);						\
}
#define CDB6_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) SCS_AD(a);		\
	p[4] = (d);						\
}
#define CDB6_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCS_AD(a);		\
	p[5] = (d);						\
}
#define CDB10_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCM_AD(a);	\
	p[1] = (word16_t) (d);					\
	p[2] = (word16_t) ((d) >> 16);				\
}
#define CDB10_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[7] = (byte_t) (d);					\
	p[8] = (byte_t) ((d) >> 8);				\
}
#define CDB10_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[6] = (d);						\
}
#define CDB10_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[9] = (d);						\
}
#define CDB12_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCL_AD(a);	\
	p[1] = (word16_t) (d);					\
	p[2] = (word16_t) ((d) >> 16);				\
}
#define CDB12_LEN(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCL_AD(a);	\
	p[3] = (word16_t) (d);					\
	p[4] = (word16_t) ((d) >> 16);				\
}
#define CDB12_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) SCL_AD(a);		\
	p[10] = (d);						\
}
#define CDB12_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCL_AD(a);		\
	p[11] = (d);						\
}

#else	/* _BYTE_ORDER_ */

#define CDB6_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCS_AD(a);	\
	p[1] = (d);						\
}
#define CDB6_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) SCS_AD(a);		\
	p[4] = (d);						\
}
#define CDB6_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCS_AD(a);		\
	p[5] = (d);						\
}
#define CDB10_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCM_AD(a);	\
	p[1] = (word16_t) ((d) >> 16);				\
	p[2] = (word16_t) (d);					\
}
#define CDB10_LEN(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[7] = (byte_t) ((d) >> 8);				\
	p[8] = (byte_t) (d);					\
}
#define CDB10_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[6] = (d);						\
}
#define CDB10_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCM_AD(a);		\
	p[9] = (d);						\
}
#define CDB12_BLK(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCL_AD(a);	\
	p[1] = (word16_t) ((d) >> 16);				\
	p[2] = (word16_t) (d);					\
}
#define CDB12_LEN(a,d)	{					\
	register word16_t *p = (word16_t *)(void *) SCL_AD(a);	\
	p[3] = (word16_t) ((d) >> 16);				\
	p[4] = (word16_t) (d);					\
}
#define CDB12_RSV(a,d)	{					\
	register byte_t *p = (byte_t *) SCL_AD(a);		\
	p[10] = (d);						\
}
#define CDB12_CTL(a,d)	{					\
	register byte_t *p = (byte_t *) SCL_AD(a);		\
	p[11] = (d);						\
}

#endif	/* _BYTE_ORDER_ */

#endif	/* !NO_ALIGN_LIMIT */

#endif	/* i386 _FTX __hppa */


#ifdef MOTOROLA
/*
 *   Motorola 88k UNIX SVR4 support
 *
 *   This software fragment contains code that interfaces the CD
 *   player application to the System V Release 4 operating system
 *   from Motorola.  The name "Motorola" is used here for identification
 *   purposes only.
 */

#include <sys/param.h>
#include <sys/dk.h>


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


#endif	/* MOTOROLA */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* SVR4 sun DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_SVR4_H__ */

