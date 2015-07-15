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
 *   application to the DEC Ultrix and DEC OSF/1 operating systems.
 *   The term DEC, Ultrix and OSF/1 are used here for identification
 *   purposes only.
 */
#ifndef __OS_DEC_H__
#define __OS_DEC_H__

#if ((defined(__alpha) && defined(__osf__)) || \
     defined(ultrix) || defined(__ultrix)) && \
    defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_dec_h_ident_ = "@(#)os_dec.h	5.4 94/12/28";
#endif

#ifdef __osf__
#include <io/common/iotypes.h>
#else /* must be ultrix */
#include <sys/devio.h>
#endif
#include <io/cam/cam.h>
#include <io/cam/uagt.h> 
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>  


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* setuid root privilege is required */


#define DEV_CAM		"/dev/cam"	/* CAM device path */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* __alpha __osf__ ultrix __ultrix DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_DEC_H__ */

