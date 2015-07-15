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
static char *_os_svr4_c_ident_ = "@(#)os_svr4.c	5.10 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#if defined(SVR4) && !defined(sun) && \
    defined(DI_SCSIPT) && !defined(DEMO_ONLY)

extern appdata_t	app_data;
extern bool_t		scsipt_notrom_error;
extern FILE		*errfp;

STATIC int		pthru_fd = -1;	/* Passthrough device file desc */


#if defined(i386) || (defined(_FTX) && defined(__hppa))
/*
 *   USL UNIX SVR4/x86 support
 *   Stratus UNIX SVR4/PA-RISC FTX 3.x support
 *   Portable Device Interface/SCSI Device Interface
 *
 *   This software fragment contains code that interfaces the
 *   CD player application to the UNIX System V Release 4
 *   operating system for the Intel x86 hardware platforms and
 *   Stratus PA-RISC systems.  The name "USL", "UNIX", "Intel",
 *   "Stratus" and "PA-RISC" are used here for identification
 *   purposes only.
 */


STATIC char		ptpath[FILE_PATH_SZ] = { '\0' };
					/* Passthrough device path */
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
	struct sb	sb;
	struct scb	*scbp;
	union scsi_cdb	cdb;
	
	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	memset(&sense_data, 0, sizeof(sense_data));
	memset(&sb, 0, sizeof(sb));
	memset(&cdb, 0, sizeof(cdb));

	sb.sb_type = ISCB_TYPE;
	scbp = &sb.SCB;

	/* set up SCSI CDB */
	switch (opcode & 0xf0) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		cdb.scl.sl_op = opcode;
		cdb.scl.sl_res1 = param;
		cdb.scl.sl_lun = 0;
		CDB12_BLK(&cdb.scl, bswap32(addr));
		CDB12_LEN(&cdb.scl, bswap32(length));
		CDB12_RSV(&cdb.scl, rsvd);
		CDB12_CTL(&cdb.scl, control);

		scbp->sc_cmdpt = (caddr_t) SCL_AD(&cdb);
		scbp->sc_cmdsz = SCL_SZ;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		cdb.scm.sm_op = opcode;
		cdb.scm.sm_res1 = param;
		cdb.scm.sm_lun = 0;
		CDB10_BLK(&cdb.scm, bswap32(addr));
		CDB10_LEN(&cdb.scm, bswap16((word16_t) length));
		CDB10_RSV(&cdb.scm, rsvd);
		CDB10_CTL(&cdb.scm, control);

		scbp->sc_cmdpt = (caddr_t) SCM_AD(&cdb);
		scbp->sc_cmdsz = SCM_SZ;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		cdb.scs.ss_op = opcode;
		cdb.scs.ss_addr1 = param;
		cdb.scs.ss_lun = 0;
		CDB6_BLK(&cdb.scs, bswap16((word16_t) addr));
		CDB6_LEN(&cdb.scs, (byte_t) length);
		CDB6_CTL(&cdb.scs, control);

		scbp->sc_cmdpt = (caddr_t) SCS_AD(&cdb);
		scbp->sc_cmdsz = SCS_SZ;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		return FALSE;
	}

	DBGDUMP("SCSI CDB bytes", (byte_t *) scbp->sc_cmdpt, scbp->sc_cmdsz);

	/* set up scsicmd */
	scbp->sc_datapt = (caddr_t) buf;
	scbp->sc_datasz = size;
	scbp->sc_mode = (rw == READ_OP) ? SCB_READ : SCB_WRITE;
	scbp->sc_time = 5000;	/* Allow 5 seconds */

	/* Send the command down via the "pass-through" interface */
	while (ioctl(pthru_fd, SDI_SEND, &sb) < 0) {
		if (errno == EAGAIN)
			/* Wait a little while and retry */
			sleep(1);
		else {
			if (app_data.scsierr_msg && prnerr)
				perror("SDI_SEND ioctl failed");
			return FALSE;
		}
	}

	if (scbp->sc_comp_code != SDI_ASW) {
		if (app_data.scsierr_msg && prnerr) {
			fprintf(errfp,
				"CD audio: %s %s:\n%s=0x%x %s=0x%x %s=0x%x",
				"SCSI command fault on",
				app_data.device,
				"Opcode",
				opcode,
				"Completion_code",
				scbp->sc_comp_code,
				"Target_status",
				scbp->sc_status);
		}

		/* Send Request Sense command */
		cdb.scs.ss_op = OP_S_RSENSE;
		cdb.scs.ss_addr1 = 0;
		cdb.scs.ss_lun = 0;
		CDB6_BLK(&cdb.scs, 0);
		CDB6_LEN(&cdb.scs, SZ_RSENSE);
		CDB6_CTL(&cdb.scs, 0);
		scbp->sc_datapt = (caddr_t) &sense_data;
		scbp->sc_datasz = SZ_RSENSE;
		scbp->sc_mode = SCB_READ;
		scbp->sc_cmdpt = (caddr_t) SCS_AD(&cdb);
		scbp->sc_cmdsz = SCS_SZ;

		if (ioctl(pthru_fd, SDI_SEND, &sb) < 0 ||
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
	int		devfd;
	dev_t		ptdev;
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

	/* Open CD-ROM device */
	if ((devfd = open(path, O_RDONLY)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", path, errno);
		return FALSE;
	}

	/* Get pass-through interface device number */
	if (ioctl(devfd, B_GETDEV, &ptdev) < 0) {
		DBGPRN(errfp, "B_GETDEV ioctl failed: errno=%d\n", errno);
		close(devfd);
		return FALSE;
	}

	close(devfd);

	/* Make pass-through interface device node */
	sprintf(ptpath, "%s/pass.%x", TEMP_DIR, ptdev);

	if (mknod(ptpath, S_IFCHR | 0700, ptdev) < 0) {
		if (errno == EEXIST) {
			unlink(ptpath);

			if (mknod(ptpath, S_IFCHR | 0700, ptdev) < 0) {
				DBGPRN(errfp, "Cannot mknod %s: errno=%d\n",
					ptpath, errno);
				return FALSE;
			}
		}
		else {
			DBGPRN(errfp, "Cannot mknod %s: errno=%d\n",
				ptpath, errno);
			return FALSE;
		}
	}

	/* Open pass-through device node */
	if ((pthru_fd = open(ptpath, O_RDONLY)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", ptpath, errno);
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

	if (ptpath[0] != '\0')
		unlink(ptpath);
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
#ifdef _FTX
	return ("OS Interface module (for Stratus FTX 3.x)\n");
#else
	return ("OS Interface module (for UNIX SVR4-PDI/x86)\n");
#endif
}

#endif	/* i386 _FTX __hppa */

#ifdef MOTOROLA
/*
 *   Motorola 88k UNIX SVR4 support
 *
 *   Contributing author: Mark Scott
 *   E-mail: mscott@urbana.mcd.mot.com
 *
 *   Note: Audio CDs sometimes produce "Blank check" warnings on the console, 
 *         just ignore these.
 *
 *   This software fragment contains code that interfaces the CD
 *   player application to the System V Release 4 operating system
 *   from Motorola.  The name "Motorola" is used here for identification
 *   purposes only.
 */


/*
 * pthru_send
 *	Build SCSI CDB and sent command to the device.
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
	char			scsistat = '\0',
				*tmpbuf;
	int			cdb_l = 0,
				i;
	long			residual = 0L;
	unsigned long		errinfo  = 0L,
				ccode = 0L;
	struct scsi_pass	spass,
				*sp = &spass;
	struct ext_sense	sense,
				*esp = &sense;

	if (pthru_fd < 0 || scsipt_notrom_error)
		return FALSE;

	/* Zero out the struct */
	memset(sp, 0, sizeof(struct scsi_pass));
	memset(esp, 0, sizeof(struct ext_sense));

	/* Setup passthru structure */
	sp->resid = &residual;
	sp->sense_data = esp;
	sp->status = &scsistat;
	sp->error_info = &errinfo;
	sp->ctlr_code = &ccode;
	sp->xfer_len = (unsigned long) size;

	/* Align on a page boundary */
	tmpbuf = NULL;
	if (sp->xfer_len > 0) {
		tmpbuf = (char *) malloc(2 * NBPP);
		sp->data = tmpbuf;
		sp->data += NBPP - ((unsigned int) sp->data & (NBPP - 1));
	}
	else
		sp->data = tmpbuf;


	if (rw == WRITE_OP && sp->xfer_len > 0)	/* Write operation */
		memcpy(sp->data, buf, sp->xfer_len);

	/* Set up SCSI CDB */
	switch (opcode & SPT_CDB_LEN) {
	case 0xa0:
	case 0xe0:
		/* 12-byte commands */
		cdb_l = 0xc0;
		sp->cdb[0] = opcode;
		sp->cdb[1] = param;
		sp->cdb[2] = (addr >> 24) & 0xff;
		sp->cdb[3] = (addr >> 16) & 0xff;
		sp->cdb[4] = (addr >> 8) & 0xff;
		sp->cdb[5] = (addr & 0xff);
		sp->cdb[6] = (length >> 24) & 0xff;
		sp->cdb[7] = (length >> 16) & 0xff;
		sp->cdb[8] = (length >> 8) & 0xff;
		sp->cdb[9] = length & 0xff;
		sp->cdb[10] = rsvd;
		sp->cdb[11] = control;
		break;

	case 0xc0:
	case 0xd0:
	case 0x20:
	case 0x30:
	case 0x40:
		/* 10-byte commands */
		cdb_l = 0xa0;
		sp->cdb[0] = opcode;
		sp->cdb[1] = param;
		sp->cdb[2] = (addr >> 24) & 0xff;
		sp->cdb[3] = (addr >> 16) & 0xff;
		sp->cdb[4] = (addr >> 8) & 0xff;
		sp->cdb[5] = addr & 0xff;
		sp->cdb[6] = rsvd;
		sp->cdb[7] = (length >> 8) & 0xff;
		sp->cdb[8] = length & 0xff;
		sp->cdb[9] = control;
		break;

	case 0x00:
	case 0x10:
		/* 6-byte commands */
		cdb_l = 0x60;
		sp->cdb[0] = opcode;
		sp->cdb[1] = param;
		sp->cdb[2] = (addr >> 8) & 0xff;
		sp->cdb[3] = addr & 0xff;
		sp->cdb[4] = length & 0xff;
		sp->cdb[5] = control;
		break;

	default:
		if (app_data.scsierr_msg && prnerr)
			fprintf(errfp, "0x%02x: Unknown SCSI opcode\n",
				opcode);
		if (tmpbuf != NULL)
			free(tmpbuf);

		return FALSE;
	}


	/* Check CDB length & flags */

	if (!SPT_CHK_CDB_LEN(cdb_l))
		fprintf(errfp, "%d: invalid CDB length\n", cdb_l);

	sp->flags = cdb_l | SPT_ERROR_QUIET;
	if (rw == READ_OP)
		sp->flags |= SPT_READ;

	if (SPT_CHK_FLAGS(cdb_l))
		fprintf(errfp, "0x%2x: bad CDB flags\n", sp->flags);

	DBGDUMP("SCSI CDB bytes", (byte_t *) sp->cdb, cdb_l);

	/* Send the command down via the "pass-through" interface */
	if (ioctl(pthru_fd, DKPASSTHRU, sp) < 0) {
		if (app_data.scsierr_msg && prnerr)
			perror("DKPASSTHRU ioctl failed");

		if (tmpbuf != NULL)
			free(tmpbuf);

		return FALSE;
	}

	if (*sp->error_info != SPTERR_NONE) {
		if (*sp->error_info != SPTERR_SCSI && *sp->status != 2 &&
		    app_data.scsierr_msg && prnerr) {

			/* Request Sense is done automatically by the driver */
			fprintf(errfp, "CD audio: %s\n",
				"SCSI command fault on",
				app_data.device);

			fprintf(errfp,
				"CD audio: %s %s:\n%s=0x%x %s=0x%x %s=0x%x\n",
				"SCSI command fault on",
				app_data.device,
				"opcode",
				opcode,
				"xfer_len",
				sp->xfer_len,
				"err_info",
				*sp->error_info);

			fprintf(errfp, "%s=0x%x %s=0x%x %s=0x%x\n",
				"ctlr_code",
				*sp->ctlr_code,
				"status",
				*sp->status,
				"resid",
				*sp->resid);
		}

		if (tmpbuf != NULL)
			free(tmpbuf);

		return FALSE;
	}

	/* pass the data back to caller */
	if (sp->xfer_len > 0 && rw == READ_OP)	/* read operation */
		memcpy(buf, sp->data, sp->xfer_len);

	if (tmpbuf != NULL)
		free(tmpbuf);

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
	int		i;
	bool_t		ret;

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

	/* Open CD-ROM device */
	if ((pthru_fd = open(path, O_RDONLY | O_NDELAY | O_EXCL)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", path, errno);
		return FALSE;
	}

	/* Motorola hack:  The CD-ROM driver allows the open to succeed
	 * even if there is no CD loaded.  We test for the existence of
	 * a disc with scsipt_tst_unit_rdy().
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
	return ("OS Interface module (for Motorola SVR4-m88k)\n");
}

#endif	/* MOTOROLA */

#endif	/* SVR4 sun DI_SCSIPT DEMO_ONLY */

