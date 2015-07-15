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
 *   application to the HP-UX Release 9.0 operating system.  The name
 *   "HP" and "hpux" are used here for identification purposes only.
 *   This software and its author are not affiliated with the Hewlett-
 *   Packard Company.
 */
#ifndef LINT
static char *_os_hpux_c_ident_ = "@(#)os_hpux.c	5.6 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if defined(__hpux) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

extern appdata_t	app_data;
extern bool_t		scsipt_notrom_error;
extern FILE		*errfp;

STATIC int		pthru_fd = -1;	/* Passthrough device file desc */


/*
 * pthru_send
 *	Build SCSI CDB and send command to the device.
 *
 * Args:
 *	opcode - SCSI command opcode
 *	addr - The "address" portion of the SCSI CDB
 *	buf - Pointer to data buffer
 *	size - Number of bytes to transfer
 *	rsvd - The "reserved" portion of the SCSI CDB
 *	length - The "length" portion of the SCSI CDB
 *	param - The "param" portion of the SCSI CDB
 *	control - The "control" portion of the SCSI CDB
 *	rw - Data transfer direction flag (READ_OP or WRITE_OP)
 *	prnerr - Whether an error message should be displayed
 *		 when a command fails
 *
 * Return:
 *	TRUE - command completed successfully
 *	FALSE - command failed
 */
bool_t
pthru_send(
	byte_t		opcode,
	word32_t	addr,
	byte_t		*buf,
	word32_t	size,
	byte_t		rsvd,
	word32_t	length,
	byte_t		param,
	byte_t		control,
	byte_t		rw,
	bool_t		prnerr
)
{
	struct sctl_io	sctl;

	
	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&sctl, 0, sizeof(sctl));

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		sctl.cdb[0] = opcode;
		sctl.cdb[1] = param;
		sctl.cdb[2] = (addr >> 24) & 0xff;
		sctl.cdb[3] = (addr >> 16) & 0xff;
		sctl.cdb[4] = (addr >> 8) & 0xff;
		sctl.cdb[5] = (addr & 0xff);
		sctl.cdb[6] = (length >> 24) & 0xff;
		sctl.cdb[7] = (length >> 16) & 0xff;
		sctl.cdb[8] = (length >> 8) & 0xff;
		sctl.cdb[9] = length & 0xff;
		sctl.cdb[10] = rsvd;
		sctl.cdb[11] = control;

		sctl.cdb_length = 12;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		sctl.cdb[0] = opcode;
		sctl.cdb[1] = param;
		sctl.cdb[2] = (addr >> 24) & 0xff;
		sctl.cdb[3] = (addr >> 16) & 0xff;
		sctl.cdb[4] = (addr >> 8) & 0xff;
		sctl.cdb[5] = addr & 0xff;
		sctl.cdb[6] = rsvd;
		sctl.cdb[7] = (length >> 8) & 0xff;
		sctl.cdb[8] = length & 0xff;
		sctl.cdb[9] = control;

		sctl.cdb_length = 10;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		sctl.cdb[0] = opcode;
		sctl.cdb[1] = param;
		sctl.cdb[2] = (addr >> 8) & 0xff;
		sctl.cdb[3] = addr & 0xff;
		sctl.cdb[4] = length & 0xff;
		sctl.cdb[5] = control;

		sctl.cdb_length = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", (byte_t *) sctl.cdb, sctl.cdb_length);

	/* set up sctl_io */
	sctl.data = buf;
	sctl.data_length = (unsigned) size;
	if (rw == READ_OP && size > 0)
		sctl.flags = SCTL_READ;
	else
		sctl.flags = 0;

	sctl.max_msecs = 10000;	/* Allow 10 seconds for command */

	/* Send the command down via the "pass-through" interface */
	if (ioctl(pthru_fd, SIOC_IO, &sctl) < 0) {
		if (app_data.scsierr_msg && prnerr)
			perror("SIOC_IO ioctl failed");
		return FALSE;
	}

	if (sctl.cdb_status != S_GOOD) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp,
				"CD audio: %s %s:\n%s=0x%x %s=0x%x %s=0x%x",
				"SCSI command fault on",
				app_data.device,
				"Opcode",
				opcode,
				"Cdb_status",
				sctl.cdb_status,
				"Sense_status",
				sctl.sense_status);

			if (sctl.sense_status == S_GOOD && sctl.sense_xfer > 2)
				fprintf(errfp,
					" Key=0x%x Code=0x%x Qual=0x%x\n",
					sctl.sense[2] & 0x0f,
					sctl.sense[12],
					sctl.sense[13]);
			else
				fprintf(errfp, "\n");
		}

		return FALSE;
	}

	return TRUE;
}


/*
 * pthru_open
 *	Open SCSI pass-through device
 *
 * Args:
 *	path - device path name string
 *
 * Return:
 *	TRUE - open successful
 *	FALSE - open failed
 */
bool_t
pthru_open(char *path)
{
	struct stat	stbuf;
	char		errstr[ERR_BUF_SZ];

	/* Check for validity of device node */
	if (stat(path, &stbuf) < 0) {
		sprintf(errstr, app_data.str_staterr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}
	if (!S_ISCHR(stbuf.st_mode)) {
		sprintf(errstr, app_data.str_noderr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}

	if ((pthru_fd = open(path, O_RDONLY)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", path, errno);
		return FALSE;
	}

	/* Obtain exclusive open */
	if (ioctl(pthru_fd, SIOC_EXCLUSIVE, 1) < 0) {
		DBGPRN(errfp, "Cannot set SIOC_EXCLUSIVE: errno=%d\n", errno);
		close(pthru_fd);
		pthru_fd = -1;
		return FALSE;
	}

	return TRUE;
}


/*
 * pthru_close
 *	Close SCSI pass-through device
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
pthru_close(void)
{
	if (pthru_fd >= 0) {
		/* Relinquish exclusive open */
		ioctl(pthru_fd, SIOC_EXCLUSIVE, 0);

		close(pthru_fd);
		pthru_fd = -1;
	}
}


/*
 * pthru_vers
 *	Return OS Interface Module version string
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Module version text string.
 */
char *
pthru_vers(void)
{
	return ("OS Interface module (for HP-UX)\n");
}

#endif	/* __hpux DI_SCSIPT DEMO_ONLY */

