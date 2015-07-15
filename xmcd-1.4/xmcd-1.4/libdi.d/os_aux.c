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
 *   Apple A/UX version 3.x support
 *
 *   Contributing author: Eric Rosen
 *
 *   This software fragment contains code that interfaces the CD player
 *   application to the Apple A/UX operating system.  The name "Apple"
 *   is used here for identification purposes only.
 */
#ifndef LINT
static char *_os_aux_c_ident_ = "@(#)os_aux.c	5.6 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"


#if defined(macII) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

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
	struct userscsireq	sc_cmd;
	uchar			cdb[12];
	req_sense_data_t	sense_data;


	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&sc_cmd, 0, sizeof(sc_cmd));
	memset(cdb, 0, sizeof(cdb));
	memset(&sense_data, 0, sizeof(sense_data));
	sc_cmd.cmdbuf = cdb;

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		sc_cmd.cmdbuf[0] = opcode;
		sc_cmd.cmdbuf[1] = param;
		sc_cmd.cmdbuf[2] = (addr >> 24) & 0xff;
		sc_cmd.cmdbuf[3] = (addr >> 16) & 0xff;
		sc_cmd.cmdbuf[4] = (addr >> 8) & 0xff;
		sc_cmd.cmdbuf[5] = (addr & 0xff);
		sc_cmd.cmdbuf[6] = (length >> 24) & 0xff;
		sc_cmd.cmdbuf[7] = (length >> 16) & 0xff;
		sc_cmd.cmdbuf[8] = (length >> 8) & 0xff;
		sc_cmd.cmdbuf[9] = length & 0xff;
		sc_cmd.cmdbuf[10] = rsvd;
		sc_cmd.cmdbuf[11] = control;

		sc_cmd.cmdlen = 12;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		sc_cmd.cmdbuf[0] = opcode;
		sc_cmd.cmdbuf[1] = param;
		sc_cmd.cmdbuf[2] = (addr >> 24) & 0xff;
		sc_cmd.cmdbuf[3] = (addr >> 16) & 0xff;
		sc_cmd.cmdbuf[4] = (addr >> 8) & 0xff;
		sc_cmd.cmdbuf[5] = addr & 0xff;
		sc_cmd.cmdbuf[6] = rsvd;
		sc_cmd.cmdbuf[7] = (length >> 8) & 0xff;
		sc_cmd.cmdbuf[8] = length & 0xff;
		sc_cmd.cmdbuf[9] = control;

		sc_cmd.cmdlen = 10;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		sc_cmd.cmdbuf[0] = opcode;
		sc_cmd.cmdbuf[1] = param;
		sc_cmd.cmdbuf[2] = (addr >> 8) & 0xFF;
		sc_cmd.cmdbuf[3] = addr & 0xFF;
		sc_cmd.cmdbuf[4] = length & 0xFF;
		sc_cmd.cmdbuf[5] = control;

		sc_cmd.cmdlen = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", (byte_t *) sc_cmd.cmdbuf, sc_cmd.cmdlen);

	sc_cmd.databuf = buf;		/* data to transfer and */
	sc_cmd.datalen = size;		/* number of bytes to transfer */
	sc_cmd.datasent	= 0;

	sc_cmd.sensebuf	= (uchar *) &sense_data;	
	sc_cmd.senselen	= (uchar) SZ_RSENSE;

	if (size != 0 && rw == READ_OP)
		sc_cmd.flags |= SRQ_READ;

	sc_cmd.timeout = 5;		/* allow 5 seconds */

	/* Send the command down via the "pass-through" interface */
	if (ioctl(pthru_fd, SCSISTART, (uchar *) &sc_cmd) < 0) {
		if (app_data.scsierr_msg && prnerr) 
			perror("SCSISTART ioctl failed");
		return FALSE;
	}

	if (sc_cmd.ret != SMG_COMP && sc_cmd.ret != SMG_LNKFLG) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp, "CD audio: %s %s:\n%s=0x%x %s=0x%x",
				"SCSI bus status error on",
				app_data.device,
				"Opcode",
				opcode,
				"Status",
				sc_cmd.msg);

			if (sense_data.valid == 0)
				fprintf(errfp, "\n");
			else {
				fprintf(errfp,
					" Key=0x%x Code=0x%x Qual=0x%x\n",
					sense_data.key,
					sense_data.code,
					sense_data.qual);
			}
		}
		return FALSE;
	}

	if (sc_cmd.stat != 0) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp, "CD audio: %s %s:\n%s=0x%x %s=0x%x\n",
				"adapter status error on",
				app_data.device,
				"Opcode",
				opcode,
				"Status",
				sc_cmd.stat);
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

	if ((pthru_fd = open(path, O_RDONLY, NULL)) < 0) {
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
	return ("OS Interface module (for Apple A/UX 3.0)\n");
}

#endif	/* macII DI_SCSIPT DEMO_ONLY */

