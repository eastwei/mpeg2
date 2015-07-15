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
 *   This software fragment contains code that interfaces the CD player
 *   application to the Apple A/UX operating system.  The name "Apple"
 *   is used here for identification purposes only.
 */

#ifndef __OS_AUX_H__
#define __OS_AUX_H__

#if defined(macII) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_aux_h_ident_ = "@(#)os_aux.h	5.4 94/12/28";
#endif


#include <sys/scsiccs.h>
#include <sys/scsireq.h>


#define	SCSISTART	_IOWR('S', 1, struct userscsireq)

typedef	unsigned char	uchar;
	
struct userscsireq {
	uchar	*cmdbuf;	/* buffer containing CDB */
	uchar	cmdlen;		/* length of CDB */
	uchar	*databuf;	/* buffer containing data to send or receive */
	ulong	datalen;	/* length of the data buffer (8 KB maximum) */
	ulong	datasent;	/* length of data actually sent or received */
	uchar	*sensebuf;	/* optional error result from sense command */
	uchar	senselen;	/* optional sense buf length (100 byte max) */
	uchar	sensesent;	/* length of sense data received */
	ushort	flags;		/* read/write flags */
	uchar	msg;		/* mode byte returned on command complete */
	uchar	stat;		/* completion status byte */
	uchar	ret;		/* return code from SCSI Manager */
	short	timeout;	/* maximum seconds inactivity for request */
};


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* macII DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_AUX_H__ */

