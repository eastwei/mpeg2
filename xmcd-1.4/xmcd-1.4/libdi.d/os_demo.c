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
#ifndef LINT
static char *_os_demo_c_ident_ = "@(#)os_demo.c	5.7 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#ifdef DEMO_ONLY

extern appdata_t	app_data;
extern bool_t		scsipt_notrom_error;
extern FILE		*errfp;

int			cdsim_sfd[2] = { -1, -1 },
			cdsim_rfd[2] = { -1, -1 };
STATIC pid_t		cdsim_pid = -1;


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
	simpkt_t	spkt,
			rpkt;
	static word32_t	pktid = 0;

	if (cdsim_rfd[0] < 0 || cdsim_sfd[1] < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&spkt, 0, CDSIM_PKTSZ);
	memset(&rpkt, 0, CDSIM_PKTSZ);

	/* Set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		spkt.cdbsz = 12;
		spkt.cdb[0] = opcode;
		spkt.cdb[1] = param;
		spkt.cdb[2] = (addr >> 24) & 0xff;
		spkt.cdb[3] = (addr >> 16) & 0xff;
		spkt.cdb[4] = (addr >> 8) & 0xff;
		spkt.cdb[5] = (addr & 0xff);
		spkt.cdb[6] = (length >> 24) & 0xff;
		spkt.cdb[7] = (length >> 16) & 0xff;
		spkt.cdb[8] = (length >> 8) & 0xff;
		spkt.cdb[9] = length & 0xff;
		spkt.cdb[10] = rsvd;
		spkt.cdb[11] = control;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		spkt.cdbsz = 10;
		spkt.cdb[0] = opcode;
		spkt.cdb[1] = param;
		spkt.cdb[2] = (addr >> 24) & 0xff;
		spkt.cdb[3] = (addr >> 16) & 0xff;
		spkt.cdb[4] = (addr >> 8) & 0xff;
		spkt.cdb[5] = addr & 0xff;
		spkt.cdb[6] = rsvd;
		spkt.cdb[7] = (length >> 8) & 0xff;
		spkt.cdb[8] = length & 0xff;
		spkt.cdb[9] = control;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		spkt.cdbsz = 6;
		spkt.cdb[0] = opcode;
		spkt.cdb[1] = param;
		spkt.cdb[2] = (addr >> 8) & 0xff;
		spkt.cdb[3] = addr & 0xff;
		spkt.cdb[4] = length & 0xff;
		spkt.cdb[5] = control;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	spkt.len = (size > MAX_DATALEN) ? MAX_DATALEN : size;
	spkt.dir = rw;
	spkt.pktid = ++pktid;

	/* Reset packet id if overflowing */
	if (pktid == 0xffff)
		pktid = 0;

	/* Copy data from user buffer into packet */
	if (rw == WRITE_OP && buf != NULL && spkt.len != 0)
		memcpy(spkt.data, buf, spkt.len);

	/* Send command packet */
	if (!cdsim_sendpkt("pthru", cdsim_sfd[1], &spkt))
		return FALSE;

	/* Get response packet */
	if (!cdsim_getpkt("pthru", cdsim_rfd[0], &rpkt))
		return FALSE;

	/* Sanity check */
	if (rpkt.pktid != spkt.pktid) {
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "pthru: packet sequence error.\n");

		return FALSE;
	}

	/* Check return status */
	if (rpkt.retcode != CDSIM_COMPOK) {
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp,
				"pthru: cmd error (opcode=0x%x status=%d).\n",
				rpkt.cdb[0], rpkt.retcode);

		return FALSE;
	}

	/* Copy data from packet into user buffer */
	if (rw == READ_OP && buf != NULL && rpkt.len != 0)
		memcpy(buf, rpkt.data, rpkt.len);

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
/*ARGSUSED*/
bool_t
pthru_open(char *path)
{
	/* Hard code some capabilities parameters for the
	 * simulated CD-ROM drive.  This overrides the
	 * parameters from the device-specific config files.
	 */
	app_data.device = "(none)";
	app_data.vendor_code = VENDOR_SCSI2;
	app_data.play10_supp = TRUE;
	app_data.play12_supp = TRUE;
	app_data.playmsf_supp = TRUE;
	app_data.playti_supp = TRUE;
	app_data.load_supp = TRUE;
	app_data.eject_supp = TRUE;
	app_data.msen_dbd = FALSE;
	app_data.mselvol_supp = TRUE;
	app_data.balance_supp = TRUE;
	app_data.chroute_supp = TRUE;
	app_data.pause_supp = TRUE;
	app_data.caddylock_supp = TRUE;
	app_data.curpos_fmt = TRUE;

	/* Open pipe for IPC */
	if (pipe(cdsim_sfd) < 0 || pipe(cdsim_rfd) < 0) {
		cd_fatal_popup(app_data.str_fatal, "Cannot open pipe.");
		return FALSE;
	}

	/* Fork the CD simulator child */
	switch (cdsim_pid = fork()) {
	case -1:
		cd_fatal_popup(app_data.str_fatal, "Cannot fork.");
		return FALSE;

	case 0:
		/* Child: run CD simulator */
		cdsim_main();
		exit(0);

	default:
		/* Parent: continue running the CD player */
		DBGPRN(errfp, "pthru: forked cdsim child pid=%d\n", cdsim_pid);
		break;
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
	waitret_t	stat_val;

	/* Close down pipes */
	close(cdsim_sfd[0]);
	close(cdsim_sfd[1]);
	close(cdsim_rfd[0]);
	close(cdsim_rfd[1]);

	/* Shut down child */
	if (cdsim_pid > 0 && kill(cdsim_pid, 0) == 0)
		kill(cdsim_pid, SIGTERM);

	/* Wait for child to exit */
	waitpid(cdsim_pid, &stat_val, 0);
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
	return ("OS Interface module (Demo Dummy)\n");
}

#endif	/* DEMO_ONLY */

