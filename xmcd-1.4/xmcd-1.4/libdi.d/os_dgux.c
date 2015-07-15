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

/*
 *   Data General DG/UX support
 *
 *   Contributing author: Karl Owen
 *   E-mail: owen@dg-rtp.dg.com
 *
 *   This software fragment contains code that interfaces the CD player
 *   application to the Data General DG/UX operating system.  The name
 *   "Data General" and DG/UX are used here for identification purposes
 *   only.
 */
#ifndef LINT
static char *_os_dgux_c_ident_ = "@(#)os_dgux.c	5.6 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"


#if defined(DGUX) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

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
	byte_t				cdb[12];
	struct user_scsi_param_blk	param_blk;


	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&cdb, 0, sizeof(cdb));
	memset(&param_blk, 0, sizeof(param_blk));

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		cdb[0] = opcode;
		cdb[1] = param;
		cdb[2] = (addr >> 24) & 0xff;
		cdb[3] = (addr >> 16) & 0xff;
		cdb[4] = (addr >> 8) & 0xff;
		cdb[5] = (addr & 0xff);
		cdb[6] = (length >> 24) & 0xff;
		cdb[7] = (length >> 16) & 0xff;
		cdb[8] = (length >> 8) & 0xff;
		cdb[9] = length & 0xff;
		cdb[10] = rsvd;
		cdb[11] = control;

		param_blk.cdb_size = 12;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		cdb[0] = opcode;
		cdb[1] = param;
		cdb[2] = (addr >> 24) & 0xff;
		cdb[3] = (addr >> 16) & 0xff;
		cdb[4] = (addr >> 8) & 0xff;
		cdb[5] = addr & 0xff;
		cdb[6] = rsvd;
		cdb[7] = (length >> 8) & 0xff;
		cdb[8] = length & 0xff;
		cdb[9] = control;

		param_blk.cdb_size = 10;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		cdb[0] = opcode;
		cdb[1] = param;
		cdb[2] = (addr >> 8) & 0xff;
		cdb[3] = addr & 0xff;
		cdb[4] = length & 0xff;
		cdb[5] = control;

		param_blk.cdb_size = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", cdb, param_blk.cdb_size);

	param_blk.cdb_ptr = (union user_scsi_cdb *)(void *) cdb;
	param_blk.data_size = size;
	param_blk.data_ptr = buf;
	param_blk.flags.data_write = (WRITE_OP == rw);

        if (ioctl(pthru_fd, USER_SCSI_CMD, &param_blk) < 0) {
		if (app_data.scsierr_msg && prnerr) {
			perror("USER_SCSI_CMD ioctl failed");
			extended_perror("USER_SCSI_CMD ioctl failed");
		}
                return FALSE;
        }

        if (param_blk.scsi_status != USER_SCSI_STATUS_GOOD) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp,
				"CD audio: %s %s:\n%s=0x%x %s=0x%x\n",
				"SCSI command fault on",
				app_data.device,
				"Opcode",
				opcode,
				"Status",
				param_blk.scsi_status);
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
	return ("OS Interface module (for Data General DG/UX)\n");
}

#endif /* DGUX DI_SCSIPT DEMO_ONLY */

