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

/*
 *   IBM AIX version 3.2.x and 4.x support
 *
 *   Contributing author: Kurt Brunton
 *   E-Mail: kbrunton@ccd.harris.com
 *
 *   Contributing author: Tom Crawley
 *   E-Mail: tomc@osi.curtin.edu.au
 *
 *   This software fragment contains code that interfaces the CD player
 *   application to the IBM AIX operating system.  The name "IBM" is
 *   used here for identification purposes only.
 *
 */
#ifndef LINT
static char *_os_aix_c_ident_ = "@(#)os_aix.c	5.6 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if defined(_AIX) && defined(DI_SCSIPT) && !defined(DEMO_ONLY)

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
	struct sc_iocmd	sc_cmd;
	int		kstat;

	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&sc_cmd, 0, sizeof(sc_cmd));

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		sc_cmd.scsi_cdb[0] = opcode;
		sc_cmd.scsi_cdb[1] = param;
		sc_cmd.scsi_cdb[2] = (addr >> 24) & 0xff;
		sc_cmd.scsi_cdb[3] = (addr >> 16) & 0xff;
		sc_cmd.scsi_cdb[4] = (addr >> 8) & 0xff;
		sc_cmd.scsi_cdb[5] = (addr & 0xff);
		sc_cmd.scsi_cdb[6] = (length >> 24) & 0xff;
		sc_cmd.scsi_cdb[7] = (length >> 16) & 0xff;
		sc_cmd.scsi_cdb[8] = (length >> 8) & 0xff;
		sc_cmd.scsi_cdb[9] = length & 0xff;
		sc_cmd.scsi_cdb[10] = rsvd;
		sc_cmd.scsi_cdb[11] = control;

		sc_cmd.command_length = 12;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		sc_cmd.scsi_cdb[0] = opcode;
		sc_cmd.scsi_cdb[1] = param;
		sc_cmd.scsi_cdb[2] = (addr >> 24) & 0xff;
		sc_cmd.scsi_cdb[3] = (addr >> 16) & 0xff;
		sc_cmd.scsi_cdb[4] = (addr >> 8) & 0xff;
		sc_cmd.scsi_cdb[5] = addr & 0xff;
		sc_cmd.scsi_cdb[6] = rsvd;
		sc_cmd.scsi_cdb[7] = (length >> 8) & 0xff;
		sc_cmd.scsi_cdb[8] = length & 0xff;
		sc_cmd.scsi_cdb[9] = control;

		sc_cmd.command_length = 10;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		sc_cmd.scsi_cdb[0] = opcode;
		sc_cmd.scsi_cdb[1] = param;
		sc_cmd.scsi_cdb[2] = (addr >> 8) & 0xff;
		sc_cmd.scsi_cdb[3] = addr & 0xff;
		sc_cmd.scsi_cdb[4] = length & 0xff;
		sc_cmd.scsi_cdb[5] = control;

		sc_cmd.command_length = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", (byte_t *) sc_cmd.scsi_cdb,
		sc_cmd.command_length);

	/* set up sc_iocmd */
	sc_cmd.buffer = (caddr_t) buf;
	sc_cmd.data_length = (int) size;
	sc_cmd.timeout_value = 5;	/* Allow 5 seconds */

	sc_cmd.flags = SC_ASYNC; 
	if (size != 0)
		sc_cmd.flags |= (rw == READ_OP) ? B_READ : B_WRITE;

	/* Send the command down via the "pass-through" interface */
	if ((kstat = ioctl(pthru_fd, CDIOCMD, &sc_cmd)) < 0) {
		if (app_data.scsierr_msg && prnerr)
			perror("CDIOCMD ioctl failed");

		return FALSE;
	}

	if ((sc_cmd.status_validity & 0x01) != 0) {
		if (sc_cmd.scsi_bus_status != SC_GOOD_STATUS) {
			if (app_data.scsierr_msg && prnerr) {
				fprintf(errfp,
					"CD audio: %s %s:\n%s=0x%x %s=0x%x\n",
					"SCSI bus status error on",
					app_data.device,
					"Opcode",
					opcode,
					"Status",
					sc_cmd.scsi_bus_status);
			}
		}

		return FALSE;
	}
	
	if ((sc_cmd.status_validity & 0x02) != 0) {
		if (sc_cmd.adapter_status != SC_GOOD_STATUS) {
			if (app_data.scsierr_msg && prnerr) {
				fprintf(errfp,
					"CD audio: %s %s:\n%s=0x%x %s=0x%x\n",
					"adapter status error on",
					app_data.device,
					"Opcode",
					opcode,
					"Status",
					sc_cmd.adapter_status);
			}
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

	if ((pthru_fd = openx(path, O_RDONLY, NULL, SC_DIAGNOSTIC)) < 0) {
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
	return ("OS Interface module (for IBM AIX 3.2.x and 4.x)\n");
}

#endif	/* _AIX DI_SCSIPT DEMO_ONLY */

