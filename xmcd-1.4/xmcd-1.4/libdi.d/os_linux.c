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
#ifndef LINT
static char *_os_linux_c_ident_ = "@(#)os_linux.c	5.11 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if defined(linux) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

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
	byte_t			*ptbuf;
	byte_t			cdb[12];
	int			ptbufsz,
				cdblen,
				ret;
	req_sense_data_t	*rp;

	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	/* Linux hack: use SCSI_IOCTL_TEST_UNIT_READY instead of
	 * SCSI_IOCTL_SEND_COMMAND for test unit ready commands.
	 */
	if (opcode == OP_S_TEST) {
		DBGPRN(errfp, "\nSending SCSI_IOCTL_TEST_UNIT_READY\n");

		ret = ioctl(pthru_fd, SCSI_IOCTL_TEST_UNIT_READY, NULL);
		if (ret == 0)
			return TRUE;

		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp,
				"SCSI_IOCTL_TEST_UNIT_READY failed: ret=0x%x\n",
				ret
			);
		}
		return FALSE;
	}

	memset(cdb, 0, sizeof(cdb));

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

		cdblen = 12;
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

		cdblen = 10;
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

		cdblen = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", cdb, cdblen);


	/* Set up SCSI pass-through command/data buffer */
	ptbufsz = (sizeof(int) << 1) + cdblen;
	if (buf != NULL && size > 0)
		ptbufsz += size;

	if ((ptbuf = (byte_t *) malloc(ptbufsz)) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return FALSE;
	}

	memset(ptbuf, 0, ptbufsz);
	memcpy(ptbuf + (sizeof(int) << 1), cdb, cdblen);

	if (buf != NULL && size > 0) {
		if (rw == WRITE_OP) {
			*((int *) ptbuf) = size;
			memcpy(ptbuf + (sizeof(int) << 1) + cdblen, buf, size);
		}
		else
			*(((int *) ptbuf) + 1) = size;
	}

	/* Send the command down via the "pass-through" interface */
	if ((ret = ioctl(pthru_fd, SCSI_IOCTL_SEND_COMMAND, ptbuf)) != 0) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp, "CD audio: %s %s:\n",
				"SCSI command error on", app_data.device);
			fprintf(errfp,
				"%s=0x%x %s=0x%x %s=0x%x %s=0x%x %s=0x%x\n",
				"Opcode", opcode,
				"Status", status_byte(ret),
				"Msg", msg_byte(ret),
				"Host", host_byte(ret),
				"Driver", driver_byte(ret));

			rp = (req_sense_data_t *)(void *)
				(ptbuf + (sizeof(int) << 1));

			if (rp->valid) {
				fprintf(errfp,
					"Key=0x%x Code=0x%x Qual=0x%x\n",
					rp->key, rp->code, rp->qual);
			}
		}

		free(ptbuf);
		return FALSE;
	}

	if (buf != NULL && rw == READ_OP && size > 0)
		memcpy(buf, ptbuf + (sizeof(int) << 1), size);

	free(ptbuf);
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
	int		i,
			ret;

	/* Force close on eject.  This is because Linux's
	 * SCSI_IOCTL_SEND_COMMAND ioctl does not work well
	 * when there is no CD loaded...
	 */
	app_data.eject_close = TRUE;

	/* Check for validity of device node */
	if (stat(path, &stbuf) < 0) {
		sprintf(errstr, app_data.str_staterr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}

	/* Linux CD-ROM device is a block special file! */
	if (!S_ISBLK(stbuf.st_mode)) {
		sprintf(errstr, app_data.str_noderr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}

	if ((pthru_fd = open(path, O_RDONLY | O_EXCL)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", path, errno);
		return FALSE;
	}

	/* Linux hack:  The CD-ROM driver allows the open to succeed
	 * even if there is no CD loaded.  We test for the existence of
	 * a disc with SCSI_IOCTL_TEST_UNIT_READY.
	 */
	for (i = 0; i < 3; i++) {
		DBGPRN(errfp, "\nSending SCSI_IOCTL_TEST_UNIT_READY\n");

		ret = ioctl(pthru_fd, SCSI_IOCTL_TEST_UNIT_READY, NULL);
		if (ret == 0)
			break;

		if (app_data.scsierr_msg && app_data.debug) {
			fprintf(errfp,
				"SCSI_IOCTL_TEST_UNIT_READY failed: ret=0x%x\n",
				ret
			);
		}
	}
	if (ret != 0) {
		/* No CD loaded */
		pthru_close();
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
	return ("OS Interface module (for Linux)\n");
}

#endif	/* linux DI_SCSIPT DEMO_ONLY */

