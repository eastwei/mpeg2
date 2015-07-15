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
 */
#ifndef __OS_DEMO_H__
#define __OS_DEMO_H__

#if defined(DI_SCSIPT) && defined(DEMO_ONLY)

#ifndef LINT
static char *_os_demo_h_ident_ = "@(#)os_demo.h	5.4 94/12/28";
#endif


#include "libdi.d/os_cdsim.h"	/* CD-ROM simulator header file */


/* Public function prototypes */
extern bool_t	pthru_send(byte_t, word32_t, byte_t *, word32_t, byte_t,
			word32_t, byte_t, byte_t, byte_t, bool_t);
extern bool_t	pthru_open(char *);
extern void	pthru_close(void);
extern char	*pthru_vers(void);

#endif	/* DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_DEMO_H__ */

