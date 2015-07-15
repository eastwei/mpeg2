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
 *   DEC Ultrix and OSF/1 support
 *
 *   Contributing author: Matt Thomas
 *   E-Mail: thomas@lkg.dec.com
 *
 *   This software module contains code that interfaces the CD player
 *   application to the DEC Ultrix and DEC OSF/1 operating systems.
 *   The term DEC, Ultrix and OSF/1 are used here for identification
 *   purposes only.
 */

#ifndef LINT
static char *_os_dec_c_ident_ = "@(#)os_dec.c	5.7 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if ((defined(__alpha) && defined(__osf__)) || \
     defined(ultrix) || defined(__ultrix)) && \
    defined(DI_SCSIPT) && !defined(DEMO_ONLY)

extern appdata_t	app_data;
extern bool_t		scsipt_notrom_error;
extern FILE		*errfp;

STATIC int		pthru_fd = -1,	/* Passthrough device file desc */
			bus = -1,
			target = -1,
			lun = -1;


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
	UAGT_CAM_CCB	uagt;
	CCB_SCSIIO	ccb;
	byte_t		*cdb;


	if ((pthru_fd < 0 && pthru_open(app_data.device) == FALSE) ||
	    scsipt_notrom_error)
		return FALSE;

	memset(&uagt, 0, sizeof(uagt));
	memset(&ccb, 0, sizeof(ccb));

	cdb = (byte_t *) ccb.cam_cdb_io.cam_cdb_bytes;

	/* Set up SCSI CDB */
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

		ccb.cam_cdb_len = 12;
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

		ccb.cam_cdb_len = 10;
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

		ccb.cam_cdb_len = 6;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	/* Setup the user agent ccb */
	uagt.uagt_ccb = (CCB_HEADER *) &ccb;
	uagt.uagt_ccblen = sizeof(CCB_SCSIIO);

	/* Setup the scsi ccb */
	ccb.cam_ch.my_addr = (CCB_HEADER *) &ccb;
	ccb.cam_ch.cam_ccb_len = sizeof(CCB_SCSIIO);
	ccb.cam_ch.cam_func_code = XPT_SCSI_IO;

	if (buf != NULL && size > 0) {
		ccb.cam_ch.cam_flags |=
			(rw == READ_OP) ? CAM_DIR_IN : CAM_DIR_OUT;
		uagt.uagt_buffer = (u_char *) buf;
		uagt.uagt_buflen = size;
	}
	else
		ccb.cam_ch.cam_flags |= CAM_DIR_NONE;
	
	ccb.cam_ch.cam_flags |= CAM_DIS_AUTOSENSE;
	ccb.cam_data_ptr = uagt.uagt_buffer;
	ccb.cam_dxfer_len = uagt.uagt_buflen;
	ccb.cam_timeout = 5;

	ccb.cam_ch.cam_path_id = bus;
	ccb.cam_ch.cam_target_id = target;
	ccb.cam_ch.cam_target_lun = lun;
    
	DBGDUMP("SCSI CDB bytes", cdb, ccb.cam_cdb_len);

	if (ioctl(pthru_fd, UAGT_CAM_IO, (caddr_t) &uagt) < 0) {
		if (app_data.scsierr_msg && prnerr)
			perror("UAGT_CAM_IO ioctl failed");

		pthru_close();
		return FALSE;
    	}

	/* Check return status */
	if ((ccb.cam_ch.cam_status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
		if (ccb.cam_ch.cam_status & CAM_SIM_QFRZN) {
			memset(&ccb, 0, sizeof(ccb));
			memset(&uagt, 0, sizeof(uagt));

			/* Setup the user agent ccb */
			uagt.uagt_ccb = (CCB_HEADER  *) &ccb;
			uagt.uagt_ccblen = sizeof(CCB_RELSIM);

			/* Setup the scsi ccb */
			ccb.cam_ch.my_addr = (struct ccb_header *) &ccb;
			ccb.cam_ch.cam_ccb_len = sizeof(CCB_RELSIM);
			ccb.cam_ch.cam_func_code = XPT_REL_SIMQ;

			ccb.cam_ch.cam_path_id = bus;
			ccb.cam_ch.cam_target_id = target;
			ccb.cam_ch.cam_target_lun = lun;

			if (ioctl(pthru_fd, UAGT_CAM_IO, (caddr_t) &uagt) < 0) {
				pthru_close();
				return FALSE;
			}
		}

		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp, "CD audio: %s %s:\n%s=0x%x %s=0x%x\n",
				"SCSI command fault on",
				app_data.device,
				"Opcode",
				opcode,
				"Status",
				ccb.cam_scsi_status);
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
	int		i,
			saverr,
			ret;

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

	if ((pthru_fd = open(path, O_RDONLY | O_NDELAY, 0)) >= 0) {
		struct devget	devget;

		if (ioctl(pthru_fd, DEVIOCGET, &devget) >= 0) {
#ifdef __osf__
			lun = devget.slave_num % 8;
			devget.slave_num /= 8;
#else
			lun = 0;
#endif
			target = devget.slave_num % 8;
			devget.slave_num /= 8;
			bus = devget.slave_num % 8;
			close(pthru_fd);

			if ((pthru_fd = open(DEV_CAM, O_RDWR, 0)) >= 0 ||
			    (pthru_fd = open(DEV_CAM, O_RDONLY, 0)) >= 0) {

				/* DEC hack:  The CAM driver allows
				 * the open to succeed even if there
				 * is no CD loaded.  We test for the
				 * existence of a disc with
				 * scsipt_tst_unit_rdy().
				 */
				for (i = 0; i < 3; i++) {
				    if ((ret = scsipt_tst_unit_rdy()) == TRUE)
					break;
				}
				if (!ret) {
					/* No CD loaded */
					pthru_close();
					return FALSE;
				}

				return TRUE;
			}

			DBGPRN(errfp, "Cannot open %s: errno=%d\n",
			       DEV_CAM, errno);
		}
		else {
			close(pthru_fd);

			DBGPRN(errfp,
			       "DEVIOCGET ioctl failed on %s: errno=%d\n",
			       path, errno);
		}
	}
	else {
		saverr = errno;
		sprintf(errstr, "Cannot open %s: errno=%d", path, saverr);

		if (saverr != EIO)
			cd_fatal_popup(app_data.str_fatal, errstr);

		DBGPRN(errfp, "%s\n", errstr);
	}

	pthru_fd = bus = target = lun = -1;
	return FALSE;
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
		pthru_fd = bus = target = lun = -1;
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
	return ("OS Interface module (for DEC OSF/1 & Ultrix)\n");
}

#endif	/* __alpha __osf__ ultrix __ultrix DI_SCSIPT DEMO_ONLY */

