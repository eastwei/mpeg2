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
 *   application to the Linux operating system.
 */
#ifndef __OS_LINUX_H__
#define __OS_LINUX_H__

#if defined(linux) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

#ifndef LINT
static char *_os_linux_h_ident_ = "@(#)os_linux.h	5.5 94/12/28";
#endif


/* Command result word - these should be in a system header file
 * that a user program can include, but they aren't.
 */
#define status_byte(result)		(((result) >> 1) & 0xf)
#define msg_byte(result)		(((result) >> 8) & 0xff)
#define host_byte(result)		(((result) >> 16) & 0xff)
#define driver_byte(result)		(((result) >> 24) & 0xff)

/* Linux SCSI ioctl commands - these should be in a system header file
 * that a user program can include, but they aren't.
 */
#define SCSI_IOCTL_SEND_COMMAND		1
#define SCSI_IOCTL_TEST_UNIT_READY	2


#define OS_MODULE	/* Indicate that this is compiled on a supported OS */
#define SETUID_ROOT	/* Setuid root privilege is required */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* linux DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_LINUX_H__ */

