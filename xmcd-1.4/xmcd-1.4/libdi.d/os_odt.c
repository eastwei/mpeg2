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
#ifndef LINT
static char *_os_odt_c_ident_ = "@(#)os_odt.c	5.6 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if defined(sco) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

extern appdata_t	app_data;
extern bool_t		scsipt_notrom_error;
extern FILE		*errfp;

STATIC int		pthru_fd = -1;	/* Passthrough device file desc */
STATIC req_sense_data_t	sense_data;	/* Request sense data buffer */


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
	struct scsicmd	sc;
	union scsi_cdb	*cdb;

	
	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&sense_data, 0, sizeof(sense_data));
	memset(&sc, 0, sizeof(sc));
	cdb = (union scsi_cdb *) sc.cdb;

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		cdb->twelve.opcode = opcode;
		cdb->twelve.misc = param;
		cdb->twelve.lun = 0;
		CDB12_BLK(&cdb->twelve, bswap32(addr));
		CDB12_LEN(&cdb->twelve, bswap32(length));
		CDB12_RSV(&cdb->twelve, rsvd);
		CDB12_CTL(&cdb->twelve, control);

		sc.cdb_len = 12;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		cdb->ten.opcode = opcode;
		cdb->ten.misc = param;
		cdb->ten.lun = 0;
		CDB10_BLK(&cdb->ten, bswap32(addr));
		CDB10_LEN(&cdb->ten, bswap16((word16_t) length));
		CDB10_RSV(&cdb->ten, rsvd);
		CDB10_CTL(&cdb->ten, control);

		sc.cdb_len = 10;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		cdb->six.opcode = opcode;
		cdb->six.misc = param;
		cdb->six.lun = 0;
		CDB6_BLK(&cdb->six, bswap16((word16_t) addr));
		CDB6_LEN(&cdb->six, (byte_t) length);
		CDB6_CTL(&cdb->six, control);

		sc.cdb_len = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", (byte_t *) sc.cdb, sc.cdb_len);

	/* set up scsicmd */
	sc.data_ptr = (faddr_t) buf;
	sc.data_len = size;
	sc.is_write = (rw == WRITE_OP);

	/* Send the command down via the "pass-through" interface */
	if (ioctl(pthru_fd, SCSIUSERCMD, &sc) < 0) {
		if (app_data.scsierr_msg && prnerr)
			perror("SCSIUSERCMD ioctl failed");
		return FALSE;
	}

	if (sc.host_sts || sc.target_sts) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp,
				"CD audio: %s %s:\n%s=0x%x %s=0x%x %s=0x%x",
				"SCSI command fault on",
				app_data.device,
				"Opcode",
				opcode,
				"Host_status",
				sc.host_sts,
				"Target_status",
				sc.target_sts);
		}

		/* Send Request Sense command */
		cdb->six.opcode = OP_S_RSENSE;
		cdb->six.misc = 0;
		cdb->six.lun = 0;
		CDB6_BLK(&cdb->six, 0);
		CDB6_LEN(&cdb->six, SZ_RSENSE);
		CDB6_CTL(&cdb->six, 0);
		sc.data_ptr = (faddr_t) &sense_data;
		sc.data_len = SZ_RSENSE;
		sc.is_write = FALSE;
		sc.cdb_len = 6;

		if (ioctl(pthru_fd, SCSIUSERCMD, &sc) < 0 ||
		    sense_data.valid == 0) {
			if (app_data.scsierr_msg && prnerr)
				fprintf(errfp, "\n");
		}
		else if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp, " Key=0x%x Code=0x%x Qual=0x%x\n",
				sense_data.key,
				sense_data.code,
				sense_data.qual);
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
	return ("OS Interface module (for SCO ODT)\n");
}

#endif	/* sco DI_SCSIPT DEMO_ONLY */

