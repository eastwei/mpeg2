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
 *   application to the SunOS operating systems.  The name "Sun" and
 *   "SunOS" are used here for identification purposes only.  This
 *   software and its author are not affiliated with Sun Microsystems.
 */
#ifndef __OS_SUN_H__
#define __OS_SUN_H__

#if defined(sun) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_sun_h_ident_ = "@(#)os_sun.h	5.4 94/12/28";
#endif

#ifdef SVR4

#include <sys/scsi/impl/uscsi.h>
#include <sys/dkio.h>

#define SOL2_VOLMGT			/* Enable Solaris Vol Mgr support */

#define USCSI_STATUS_GOOD	0

#else	/* !SVR4 */

#include <scsi/impl/uscsi.h>
#undef USCSI_WRITE
#define USCSI_WRITE		0

/* This is a hack to work around a bug in SunOS 4.x's _IOWR macro
 * in <sys/ioccom.h> which makes it incompatible with ANSI compilers.
 * If Sun ever changes the definition of USCSICMD or _IOWR then
 * this will have to change...
 */
#undef _IOWR
#undef USCSICMD

#define _IOWR(x,y,t)	( \
				_IOC_INOUT | \
				((sizeof(t) & _IOCPARM_MASK) << 16) | \
				((x) << 8) | (y) \
			)
#define USCSICMD	_IOWR('u', 1, struct uscsi_cmd)

#endif	/* SVR4 */


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);
extern bool_t	sol2_volmgt_eject(void);

#endif	/* sun DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_SUN_H__ */

