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
static char *_scsipt_c_ident_ = "@(#)scsipt.c	5.17 95/01/31";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#ifdef DI_SCSIPT

extern appdata_t	app_data;
extern FILE		*errfp;

STATIC bool_t	scsipt_run_ab(curstat_t *),
		scsipt_run_sample(curstat_t *),
		scsipt_run_prog(curstat_t *),
		scsipt_run_repeat(curstat_t *),
		scsipt_disc_ready(curstat_t *);
STATIC void	scsipt_stat_poll(curstat_t *),
		scsipt_insert_poll(curstat_t *);

/* Not a CD-ROM error */
bool_t		scsipt_notrom_error = FALSE;

/* VU module entry jump table */
vu_tbl_t	scsipt_vutbl[MAX_VENDORS];


STATIC int	scsipt_stat_interval;		/* Status poll interval */
STATIC long	scsipt_stat_id,			/* Play status poll timer id */
		scsipt_insert_id,		/* Disc insert poll timer id */
		scsipt_search_id;		/* FF/REW timer id */
STATIC byte_t	scsipt_next_sam;		/* Next SAMPLE track */
STATIC bool_t	scsipt_not_open = TRUE,		/* Device not opened yet */
		scsipt_stat_polling,		/* Polling play status */
		scsipt_insert_polling,		/* Polling disc insert */
		scsipt_new_progshuf,		/* New program/shuffle seq */
		scsipt_start_search = FALSE,	/* Start FF/REW play segment */
		scsipt_idx_pause = FALSE,	/* Prev/next index pausing */
		scsipt_fake_stop = FALSE;	/* Force a completion status */
STATIC word32_t	scsipt_ab_start_addr,		/* A->B mode start block */
		scsipt_ab_end_addr,		/* A->B mode end block */
		scsipt_sav_end_addr;		/* Err recov saved end addr */
STATIC msf_t	scsipt_ab_start_msf,		/* A->B mode start MSF */
		scsipt_ab_end_msf,		/* A->B mode end MSF */
		scsipt_sav_end_msf;		/* Err recov saved end MSF */
STATIC byte_t	scsipt_dev_scsiver,		/* Device SCSI version */
		scsipt_sav_end_fmt,		/* Err recov saved end fmt */
		scsipt_route_left,		/* Left channel routing */
		scsipt_route_right;		/* Right channel routing */

/* VU module init jump table */
STATIC vuinit_tbl_t	vuinit[] = {
	NULL, chin_init, hita_init, nec_init, pion_init, sony_init, tosh_init,
};


/***********************
 *  internal routines  *
 ***********************/


/*
 * scsipt_rdsubq
 *	Send SCSI-2 Read Subchannel command to the device
 *
 * Args:
 *	buf - Pointer to the return data buffer
 *	fmt - Subchannel data format code
 *		SUB_ALL		Subchannel-Q data
 *		SUB_CURPOS	CD-ROM Current position data
 *		SUB_CATNO	Media catalog number data
 *		SUB_ISRC	Track Intl Standard Recording Code
 *	subq - Whether the CD-ROM should return subchannel-Q data
 *	trkno - Track number from which the ISRC data is read
 *	msf - Whether to use MSF or logical block address format
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_rdsubq(byte_t *buf, byte_t fmt, byte_t subq, int trkno, bool_t msf)
{
	int	xfer_len;
	bool_t	ret;

	switch (fmt) {
	case SUB_ALL:
		xfer_len = 48;
		break;
	case SUB_CURPOS:
		xfer_len = 16;
		break;
	case SUB_CATNO:
	case SUB_ISRC:
		xfer_len = 24;
		break;
	default:
		return FALSE;
	}

	if (xfer_len > SZ_RDSUBQ)
		xfer_len = SZ_RDSUBQ;

	ret = pthru_send(
		OP_M_RDSUBQ,
		(word32_t) (fmt << 16 | subq << 30),
		buf,
		xfer_len,
		(byte_t) trkno,
		xfer_len,
		(byte_t) (msf << 1),
		0,
		READ_OP,
		TRUE
	);

	if (ret) {
		DBGDUMP("Read Subchannel data bytes", buf, xfer_len);
	}

	return (ret);
}


/*
 * scsipt_modesense
 *	Send SCSI Mode Sense command to the device
 *
 * Args:
 *	buf - Pointer to the return data buffer
 *	pg_ctrl - Defines the type of parameters to be returned:
 *		0: Current values
 *		1: Changeable values
 *		2: Default values
 *	pg_code - Specifies which page or pages to return:
 *		PG_ERRECOV: Error recovery params page
 *		PG_DISCONN: Disconnect/reconnect params page
 *		PG_CDROMCTL: CD-ROM params page
 *		PG_AUDIOCTL: Audio control params page
 *		PG_ALL: All pages
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_modesense(byte_t *buf, byte_t pg_ctrl, byte_t pg_code)
{
	int	xfer_len;
	bool_t	ret;

	switch (pg_code) {
	case PG_ERRECOV:
		xfer_len = 12;
		break;
	case PG_DISCONN:
		xfer_len = 20;
		break;
	case PG_CDROMCTL:
		xfer_len = 12;
		break;
	case PG_AUDIOCTL:
		xfer_len = 20;
		break;
	case PG_ALL:
		xfer_len = 52;
		break;
	default:
		return FALSE;
	}

	if (!app_data.msen_dbd)
		xfer_len += 8;

	ret = pthru_send(
		OP_S_MSENSE,
		(word32_t) ((pg_ctrl << 6 | pg_code) << 8),
		buf,
		xfer_len,
		0,
		xfer_len,
		(byte_t) (app_data.msen_dbd ? 0x08 : 0x00),
		0,
		READ_OP,
		TRUE
	);

	if (ret) {
		DBGDUMP("Mode Sense data bytes", buf, xfer_len);
	}

	return (ret);
}


/*
 * scsipt_modesel
 *	Send SCSI Mode Select command to the device
 *
 * Args:
 *	buf - Pointer to the data buffer
 *	pg_code - Specifies which page or pages to return:
 *		PG_ERRECOV: Error recovery params page
 *		PG_DISCONN: Disconnect/reconnect params page
 *		PG_CDROMCTL: CD-ROM params page
 *		PG_AUDIOCTL: Audio control params page
 *		PG_ALL: All pages
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_modesel(byte_t *buf, byte_t pg_code)
{
	int	xfer_len;

	switch (pg_code) {
	case PG_ERRECOV:
		xfer_len = 12;
		break;
	case PG_DISCONN:
		xfer_len = 20;
		break;
	case PG_CDROMCTL:
		xfer_len = 12;
		break;
	case PG_AUDIOCTL:
		xfer_len = 20;
		break;
	case PG_ALL:
		xfer_len = 52;
		break;
	default:
		return FALSE;
	}

	if (!app_data.msen_dbd)
		xfer_len += 8;

	DBGDUMP("Mode Select data bytes", buf, xfer_len);

	return (
		pthru_send(
			OP_S_MSELECT,
			0,
			buf,
			xfer_len,
			0,
			xfer_len,
			0x10,
			0,
			WRITE_OP,
			TRUE
		)
	);
}


/*
 * scsipt_inquiry
 *	Send SCSI Inquiry command to the device
 *
 * Args:
 *	buf - Pointer to the return data buffer
 *	len - Maximum number of inquiry data bytes to transfer
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_inquiry(byte_t *buf, int len)
{
	bool_t	ret;

	ret = pthru_send(OP_S_INQUIR, 0, buf, len, 0, len, 0, 0, READ_OP, TRUE);

	if (ret) {
		DBGDUMP("Inquiry data bytes", buf, len);
	}

	return (ret);
}


/*
 * scsipt_rdtoc
 *	Send SCSI-2 Read TOC command to the device
 *
 * Args:
 *	buf - Pointer to the return data buffer
 *	msf - Whether to use MSF or logical block address data format
 *	start - Starting track number for which the TOC data is returned
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_rdtoc(byte_t *buf, bool_t msf, int start)
{
	int		xfer_len;
	toc_hdr_t	*thdr;
	bool_t		ret;

	/* Read the TOC header first */
	if (!pthru_send(OP_M_RDTOC, 0, buf, SZ_TOCHDR,
			(byte_t) start, SZ_TOCHDR,
			(byte_t) (msf << 1), 0, READ_OP, TRUE))
		return FALSE;

	thdr = (toc_hdr_t *)(void *) buf;

	if (start == 0)
		start = (int) thdr->first_trk;

	xfer_len = SZ_TOCHDR +
		   (((int) thdr->last_trk - start + 2) * SZ_TOCENT);

	if (xfer_len > SZ_RDTOC)
		xfer_len = SZ_RDTOC;

	/* Read the appropriate number of bytes of the entire TOC */
	ret = pthru_send(
		OP_M_RDTOC,
		0,
		buf,
		xfer_len,
		(byte_t) start,
		xfer_len,
		(byte_t) (msf << 1),
		0,
		READ_OP,
		TRUE
	);

	if (ret) {
		DBGDUMP("Read TOC data bytes", buf, xfer_len);
	}

	return (ret);
}


/*
 * scsipt_tst_unit_rdy
 *	Send SCSI Test Unit Ready command to the device
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure (drive not ready)
 */
bool_t
scsipt_tst_unit_rdy(void)
{
	return (
		pthru_send(
			OP_S_TEST,
			0,
			NULL,
			0,
			0,
			0,
			0,
			0,
			READ_OP,
			app_data.debug
		)
	);
}


/*
 * scsipt_playmsf
 *	Send SCSI-2 Play Audio MSF command to the device
 *
 * Args:
 *	start - Pointer to the starting position MSF data
 *	end - Pointer to the ending position MSF data
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_playmsf(msf_t *start, msf_t *end)
{
	word32_t	*addr = (word32_t *)(void *) start;
	word16_t	*len = (word16_t *)(void *) &end->sec;
	byte_t		*rsvd = (byte_t *) &end->min;

	if (!app_data.playmsf_supp)
		return FALSE;

	start->res = end->res = 0;

	return (
		pthru_send(
			OP_M_PLAYMSF,
			bswap32(*addr),
			NULL,
			0,
			*rsvd,
			(word32_t) bswap16(*len),
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_play10
 *	Send SCSI-2 Play Audio (10) command to the device
 *
 * Args:
 *	start - The starting logical block address
 *	len - The number of logical blocks to play (max=0xffff)
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_play10(word32_t start, word32_t len)
{
	if (!app_data.play10_supp || len > 0xffff)
		return FALSE;

	return (
		pthru_send(
			OP_M_PLAY,
			start,
			NULL,
			0,
			0,
			len,
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_play12
 *	Send SCSI-2 Play Audio (12) command to the device
 *
 * Args:
 *	start - The starting logical block address
 *	len - The number of logical blocks to play
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_play12(word32_t start, word32_t len)
{
	if (!app_data.play12_supp)
		return FALSE;

	return (
		pthru_send(
			OP_L_PLAY,
			start,
			NULL,
			0,
			0,
			len,
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_prev_allow
 *	Send SCSI Prevent/Allow Medium Removal command to the device
 *
 * Args:
 *	prevent - Whether to prevent or allow medium removal
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_prev_allow(bool_t prevent)
{
	if (!app_data.caddylock_supp)
		return FALSE;

	return (
		pthru_send(
			OP_S_PREVENT,
			0,
			NULL,
			0,
			0,
			prevent,
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_start_stop
 *	Send SCSI Start/Stop Unit command to the device
 *
 * Args:
 *	start - Whether to start unit or stop unit
 *	loej - Whether caddy load/eject operation should be performed
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_start_stop(bool_t start, bool_t loej)
{
	byte_t	ctl;

	if (start)
		ctl = 0x01;
	else
		ctl = 0x00;

	if (loej)
		ctl |= 0x02;

	return (
		pthru_send(
			OP_S_START,
			0,
			NULL,
			0,
			0,
			ctl,
			0x1,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_pause_resume
 *	Send SCSI-2 Pause/Resume command to the device
 *
 * Args:
 *	resume - Whether to resume or pause
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_pause_resume(bool_t resume)
{
	if (!app_data.pause_supp)
		return FALSE;

	return (
		pthru_send(
			OP_M_PAUSE,
			0,
			NULL,
			0,
			0,
			resume,
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_play_trkidx
 *	Send SCSI-2 Play Audio Track/Index command to the device
 *
 * Args:
 *	start_trk - Starting track number
 *	start_idx - Starting index number
 *	end_trk - Ending track number
 *	end_idx - Ending index number
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_play_trkidx(int start_trk, int start_idx, int end_trk, int end_idx)
{
	if (!app_data.playti_supp)
		return FALSE;

	return (
		pthru_send(
			OP_M_PLAYTI,
			(start_trk << 8) | start_idx,
			NULL,
			0,
			0,
			(end_trk << 8) | end_idx,
			0,
			0,
			READ_OP,
			TRUE
		)
	);
}


/*
 * scsipt_do_playaudio
 *	General top-level play audio function
 *
 * Args:
 *	addr_fmt - The address formats specified:
 *		ADDR_BLK: logical block address
 *		ADDR_MSF: MSF address
 *		ADDR_TRKIDX: Track/index numbers
 *		ADDR_OPTEND: Ending address can be ignored
 *	start_addr - Starting logical block address
 *	end_addr - Ending logical block address
 *	start_msf - Pointer to start address MSF data
 *	end_msf - Pointer to end address MSF data
 *	trk - Starting track number
 *	idx - Starting index number
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_do_playaudio(
	byte_t		addr_fmt,
	word32_t	start_addr,
	word32_t	end_addr,
	msf_t		*start_msf,
	msf_t		*end_msf,
	byte_t		trk,
	byte_t		idx
)
{
	msf_t		emsf,
			*emsfp = NULL;
	bool_t		ret = FALSE;


	/* Fix addresses: Some CD-ROM drives will only allow playing to
	 * the last frame minus 1.
	 */
	if (addr_fmt & ADDR_MSF && end_msf != NULL) {
		emsf = *end_msf;	/* Structure copy */
		emsfp = &emsf;

		if (emsfp->frame > 0)
			emsfp->frame--;
		else {
			emsfp->frame = FRAME_PER_SEC - 1;
			if (emsfp->sec > 0)
				emsfp->sec--;
			else {
				emsfp->sec = 59;
				if (emsfp->min > 0)
					emsfp->min--;
			}
		}

		emsfp->res = start_msf->res = 0;

		/* Save end address for error recovery */
		scsipt_sav_end_msf = *end_msf;
	}
	if (addr_fmt & ADDR_BLK) {
		if (end_addr != 0)
			end_addr--;

		/* Save end address for error recovery */
		scsipt_sav_end_addr = end_addr;
	}

	/* Save end address format for error recovery */
	scsipt_sav_end_fmt = addr_fmt;

	if (scsipt_vutbl[app_data.vendor_code].playaudio != NULL) {
		ret = scsipt_vutbl[app_data.vendor_code].playaudio(
			addr_fmt,
			start_addr, end_addr,
			start_msf, emsfp,
			trk, idx
		);
	}

	/* If the device does not claim SCSI-2 compliance, and the
	 * device-specific configuration is not SCSI-2, then don't
	 * attempt to deliver SCSI-2 commands to the device.
	 */
	if (!ret && app_data.vendor_code != VENDOR_SCSI2 &&
	    scsipt_dev_scsiver < 2)
		return FALSE;
	
	if (!ret && (addr_fmt & ADDR_MSF) && app_data.playmsf_supp)
		ret = scsipt_playmsf(start_msf, emsfp);
	
	if (!ret && (addr_fmt & ADDR_BLK) && app_data.play12_supp)
		ret = scsipt_play12(start_addr, end_addr - start_addr);
	
	if (!ret && (addr_fmt & ADDR_BLK) && app_data.play10_supp)
		ret = scsipt_play10(start_addr, end_addr - start_addr);

	if (!ret && (addr_fmt & ADDR_TRKIDX) && app_data.playti_supp)
		ret = scsipt_play_trkidx(trk, idx, trk, idx);

	return (ret);
}


/*
 * scsipt_do_pause_resume
 *	General top-level pause/resume function
 *
 * Args:
 *	resume - Whether to resume or pause
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_do_pause_resume(bool_t resume)
{
	bool_t	ret = FALSE;

	if (scsipt_vutbl[app_data.vendor_code].pause_resume != NULL)
		ret = scsipt_vutbl[app_data.vendor_code].pause_resume(resume);

	/* If the device does not claim SCSI-2 compliance, and the
	 * device-specific configuration is not SCSI-2, then don't
	 * attempt to deliver SCSI-2 commands to the device.
	 */
	if (!ret && app_data.vendor_code != VENDOR_SCSI2 &&
	    scsipt_dev_scsiver < 2)
		return FALSE;

	if (!ret && app_data.pause_supp)
		ret = scsipt_pause_resume(resume);

	return (ret);
}


/*
 * scsipt_do_start_stop
 *	General top-level start/stop function
 *
 * Args:
 *	start - Whether to start unit or stop unit
 *	loej - Whether caddy load/eject operation should be performed
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_do_start_stop(bool_t start, bool_t loej)
{
	bool_t	ret = FALSE;

	if (!app_data.load_supp && start && loej)
		return FALSE;

	if (!app_data.eject_supp)
		loej = 0;

	if (!start && loej &&
	    scsipt_vutbl[app_data.vendor_code].eject != NULL)
		ret = scsipt_vutbl[app_data.vendor_code].eject();

	if (!ret && scsipt_vutbl[app_data.vendor_code].start_stop != NULL)
		ret = scsipt_vutbl[app_data.vendor_code].start_stop(
			start, loej
		);

#ifdef SOL2_VOLMGT
	/* Sun Hack: Under Solaris 2.x with the Volume Manager
	 * we need to use a special SunOS ioctl to eject the CD.
	 */
	if (app_data.sol2_volmgt && !start && loej)
		ret = sol2_volmgt_eject();
#endif

	if (!ret)
		ret = scsipt_start_stop(start, loej);

	return (ret);
}


/*
 * scsipt_get_playstatus
 *	Obtain and update current playback status information
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - Audio playback is in progress
 *	FALSE - Audio playback stopped or command failure
 */
STATIC bool_t
scsipt_get_playstatus(curstat_t *s)
{
	msf_t		recov_start_msf;
	word32_t	recov_start_addr;
	byte_t		buf[SZ_RDSUBQ],
			audio_status,
			*cp;
	bool_t		done,
			ret = FALSE;
	subq_hdr_t	*h;
	subq_01_t	*p;
	static int	errcnt = 0;
	static word32_t	errblk = 0;
	static bool_t	in_scsipt_get_playstatus = FALSE;


	/* Lock this routine from multiple entry */
	if (in_scsipt_get_playstatus)
		return TRUE;

	in_scsipt_get_playstatus = TRUE;

	if (scsipt_vutbl[app_data.vendor_code].get_playstatus != NULL) {
		ret = scsipt_vutbl[app_data.vendor_code].get_playstatus(
			s, &audio_status
		);
	}

	/* If the device does not claim SCSI-2 compliance, and the
	 * device-specific configuration is not SCSI-2, then don't
	 * attempt to deliver SCSI-2 commands to the device.
	 */
	if (!ret && app_data.vendor_code != VENDOR_SCSI2 &&
	    scsipt_dev_scsiver < 2) {
		in_scsipt_get_playstatus = FALSE;
		return FALSE;
	}

	if (!ret) {
		memset(buf, 0, sizeof(buf));

		if (!scsipt_rdsubq(buf, (byte_t)
				 (app_data.curpos_fmt ? SUB_CURPOS : SUB_ALL),
				 1, 0, TRUE)) {
			/* Check to see if the disc had been manually ejected */
			if (!scsipt_disc_ready(s)) {
				scsipt_sav_end_addr = 0;
				scsipt_sav_end_msf.min = 0;
				scsipt_sav_end_msf.sec = 0;
				scsipt_sav_end_msf.frame = 0;
				scsipt_sav_end_fmt = 0;
				errcnt = 0;
				errblk = 0;

				in_scsipt_get_playstatus = FALSE;
				return FALSE;
			}

			/* The read subchannel command failed for some
			 * unknown reason.  Just return success and
			 * hope the next poll succeeds.  We don't want
			 * to return FALSE here because that would stop
			 * the poll.
			 */
			in_scsipt_get_playstatus = FALSE;
			return TRUE;
		}

		h = (subq_hdr_t *)(void *) buf;

		audio_status = h->audio_status;

		/* Check the subchannel data */
		cp = (byte_t *) h + sizeof(subq_hdr_t);
		switch (*cp) {
		case SUB_ALL:
		case SUB_CURPOS:
			p = (subq_01_t *)(void *) cp;

			if (p->trkno != s->cur_trk) {
				s->cur_trk = p->trkno;
				dpy_track(s);
			}

			if (p->idxno != s->cur_idx) {
				s->cur_idx = p->idxno;
				s->sav_iaddr = s->cur_tot_addr;
				dpy_index(s);
			}

			s->cur_tot_min = p->abs_addr.msf.min;
			s->cur_tot_sec = p->abs_addr.msf.sec;
			s->cur_tot_frame = p->abs_addr.msf.frame;
			msftoblk(
				s->cur_tot_min,
				s->cur_tot_sec,
				s->cur_tot_frame,
				&s->cur_tot_addr,
				MSF_OFFSET(s)
			);

			s->cur_trk_min = p->rel_addr.msf.min;
			s->cur_trk_sec = p->rel_addr.msf.sec;
			s->cur_trk_frame = p->rel_addr.msf.frame;
			msftoblk(
				s->cur_trk_min,
				s->cur_trk_sec,
				s->cur_trk_frame,
				&s->cur_trk_addr,
				0
			);

			break;
		default:
			/* Something is wrong with the data */
			break;
		}
	}

	/* Update time display */
	dpy_time(s, FALSE);


	/* Hack: to work around the fact that some CD-ROM drives
	 * return AUDIO_PAUSED status after issuing a Stop Unit command.
	 * Just treat the status as completed if we get a paused status
	 * and we don't expect the drive to be paused.
	 */
	if (audio_status == AUDIO_PAUSED && s->mode != M_PAUSE &&
	    !scsipt_idx_pause)
		audio_status = AUDIO_COMPLETED;

	/* Force completion status */
	if (scsipt_fake_stop) {
		scsipt_fake_stop = FALSE;
		audio_status = AUDIO_COMPLETED;
	}

	/* Deal with playback status */
	switch (audio_status) {
	case AUDIO_PLAYING:
	case AUDIO_PAUSED:
		done = FALSE;

		/* If we haven't encountered an error for a while, then
		 * clear the error count.
		 */
		if (errcnt > 0 && (s->cur_tot_addr - errblk) > ERR_CLRTHRESH)
			errcnt = 0;
		break;

	case AUDIO_FAILED:
		/* Check to see if the disc had been manually ejected */
		if (!scsipt_disc_ready(s)) {
			scsipt_sav_end_addr = 0;
			scsipt_sav_end_msf.min = 0;
			scsipt_sav_end_msf.sec = 0;
			scsipt_sav_end_msf.frame = 0;
			scsipt_sav_end_fmt = 0;
			errcnt = 0;
			errblk = 0;

			in_scsipt_get_playstatus = FALSE;
			return FALSE;
		}

		/* Audio playback stopped due to a disc error.  We will
		 * try to restart the playback by skipping a few frames
		 * and continuing.  This will cause a glitch in the sound
		 * but is better than just stopping.
		 */
		done = FALSE;

		/* Check for max errors limit */
		if (++errcnt > MAX_RECOVERR) {
			done = TRUE;
			fprintf(errfp, "CD audio: %s\n", app_data.str_maxerr);
		}
		errblk = s->cur_tot_addr;

		if (!done && (scsipt_sav_end_fmt & ADDR_MSF)) {
			if ((int) s->cur_tot_frame <
			    (FRAME_PER_SEC - ERR_SKIPBLKS)) {
				recov_start_msf.min = s->cur_tot_min;
				recov_start_msf.sec = s->cur_tot_sec;
				recov_start_msf.frame =
					s->cur_tot_frame + ERR_SKIPBLKS;
			}
			else if ((int) s->cur_tot_sec < 59) {
				recov_start_msf.min = s->cur_tot_min;
				recov_start_msf.sec = s->cur_tot_sec + 1;
				recov_start_msf.frame = ERR_SKIPBLKS -
					(FRAME_PER_SEC - s->cur_tot_frame);
			}
			else {
				recov_start_msf.min = s->cur_tot_min + 1;
				recov_start_msf.sec = 0;
				recov_start_msf.frame = ERR_SKIPBLKS -
					(FRAME_PER_SEC - s->cur_tot_frame);
			}

			/* Check to see if we have skipped past
			 * the end.
			 */
			if (recov_start_msf.min > scsipt_sav_end_msf.min)
				done = TRUE;
			else if (recov_start_msf.min ==
				 scsipt_sav_end_msf.min) {
				if (recov_start_msf.sec >
				    scsipt_sav_end_msf.sec)
					done = TRUE;
				else if ((recov_start_msf.sec ==
					  scsipt_sav_end_msf.sec) &&
					 (recov_start_msf.frame >
					  scsipt_sav_end_msf.frame)) {
					done = TRUE;
				}
			}
		}
		else {
			recov_start_msf.min = 0;
			recov_start_msf.sec = 0;
			recov_start_msf.frame = 0;
		}

		if (!done && (scsipt_sav_end_fmt & ADDR_BLK)) {
			recov_start_addr = s->cur_tot_addr + ERR_SKIPBLKS;

			/* Check to see if we have skipped past
			 * the end.
			 */
			if (recov_start_addr >= scsipt_sav_end_addr)
				done = TRUE;
		}
		else
			recov_start_addr = 0;


		/* Restart playback */
		if (!done) {
			fprintf(errfp, "CD audio: %s\n",
				app_data.str_recoverr);

			scsipt_do_playaudio(
				scsipt_sav_end_fmt,
				recov_start_addr, scsipt_sav_end_addr,
				&recov_start_msf, &scsipt_sav_end_msf,
				0, 0
			);

			in_scsipt_get_playstatus = FALSE;
			return TRUE;
		}

		/*FALLTHROUGH*/
	case AUDIO_COMPLETED:
	case AUDIO_NOSTATUS:
	case AUDIO_NOTVALID:
		done = TRUE;

		switch (s->mode) {
		case M_SAMPLE:
			done = !scsipt_run_sample(s);
			break;

		case M_AB:
			done = !scsipt_run_ab(s);
			break;

		case M_PLAY:
		case M_PAUSE:
			s->cur_trk_addr = 0;
			s->cur_trk_min = s->cur_trk_sec = s->cur_trk_frame = 0;

			if (s->shuffle || s->program)
				done = !scsipt_run_prog(s);

			if (s->repeat)
				done = !scsipt_run_repeat(s);

			break;
		}

		break;

	default:
		/* Something is wrong with the data. */
		done = FALSE;
	}

	if (done) {
		/* Reset states */
		reset_curstat(s, FALSE);
		s->mode = M_STOP;
		scsipt_sav_end_addr = 0;
		scsipt_sav_end_msf.min = scsipt_sav_end_msf.sec =
			scsipt_sav_end_msf.frame = 0;
		scsipt_sav_end_fmt = 0;
		errcnt = 0;
		errblk = 0;
		dpy_all(s);

		if (app_data.done_eject) {
			/* Eject the disc */
			scsipt_load_eject(s);
		}
		else {
			/* Spin down the disc */
			scsipt_do_start_stop(FALSE, FALSE);
		}

		in_scsipt_get_playstatus = FALSE;
		return FALSE;
	}

	in_scsipt_get_playstatus = FALSE;
	return TRUE;
}


/*
 * scsipt_cfg_vol
 *	Audio volume control function
 *
 * Args:
 *	vol - Logical volume value to set to
 *	s - Pointer to the curstat_t structure
 *	query - If TRUE, query current volume only
 *	warp - Whether to set the volume and balance control slider
 *		thumbs to the appropriate position.
 *		Bits: WARP_VOL WARP_BAL
 *
 * Return:
 *	The current logical volume value, or -1 on failure.
 */
STATIC int
scsipt_cfg_vol(int vol, curstat_t *s, bool_t query, byte_t warp)
{
	int			vol1,
				vol2;
	mode_sense_data_t	*ms_data;
	blk_desc_t		*bdesc;
	audio_pg_t		*audiopg;
	bool_t			ret = FALSE;
	byte_t			buf[SZ_MSENSE];
	static bool_t		muted = FALSE;


	if (scsipt_vutbl[app_data.vendor_code].volume != NULL) {
		vol = scsipt_vutbl[app_data.vendor_code].volume(vol, s, query);

		if (query && vol >= 0) {
			if (warp & WARP_VOL)
				set_vol_slider(vol);

			if (warp & WARP_BAL)
				set_bal_slider(
				    (int) (s->level_right - s->level_left) / 2
				);
		}

		return (vol);
	}

	if (scsipt_vutbl[app_data.vendor_code].mute != NULL) {
		if (!query) {
			if (vol < (int) s->level)
				vol = 0;
			else if (vol > (int) s->level ||
				 (vol != 0 && vol != 100))
				vol = 100;

			ret = scsipt_vutbl[app_data.vendor_code].mute(
				(bool_t) (vol == 0)
			);
			if (ret)
				muted = (vol == 0);
		}

		vol = muted ? 0 : MAX_VOL;

		if (warp & WARP_VOL)
			set_vol_slider(vol);

		if (warp & WARP_BAL)
			set_bal_slider(
				(int) (s->level_right - s->level_left) / 2
			);

		return (vol);
	}

	if (!app_data.mselvol_supp)
		return 0;

	memset(buf, 0, SZ_MSENSE);

	if (!scsipt_modesense(buf, 0, PG_AUDIOCTL))
		return -1;

	ms_data = (mode_sense_data_t *)(void *) buf;
	bdesc = (blk_desc_t *)(void *) ms_data->data;
	audiopg = (audio_pg_t *)(void *)
		&ms_data->data[ms_data->bdescr_len];

	if (audiopg->pg_code == PG_AUDIOCTL) {
		if (query) {
			vol1 = untaper_vol(unscale_vol((int) audiopg->p0_vol));
			vol2 = untaper_vol(unscale_vol((int) audiopg->p1_vol));
			scsipt_route_left = (byte_t) audiopg->p0_ch_ctrl;
			scsipt_route_right = (byte_t) audiopg->p1_ch_ctrl;

			if (vol1 == vol2) {
				s->level_left = s->level_right = 100;
				vol = vol1;
			}
			else if (vol1 > vol2) {
				s->level_left = 100;
				s->level_right = (byte_t) ((vol2 * 100) / vol1);
				vol = vol1;
			}
			else {
				s->level_left = (byte_t) ((vol1 * 100) / vol2);
				s->level_right = 100;
				vol = vol2;
			}

			if (warp & WARP_VOL)
				set_vol_slider(vol);

			if (warp & WARP_BAL)
				set_bal_slider(
				    (int) (s->level_right - s->level_left) / 2
				);

			return (vol);
		}
		else {
			ms_data->data_len = 0;
			if (ms_data->bdescr_len > 0)
				bdesc->num_blks = 0;

			audiopg->p0_vol = scale_vol(
				taper_vol(vol * (int) s->level_left / 100)
			);
			audiopg->p1_vol = scale_vol(
				taper_vol(vol * (int) s->level_right / 100)
			);

			audiopg->p0_ch_ctrl = scsipt_route_left;
			audiopg->p1_ch_ctrl = scsipt_route_right;

			audiopg->sotc = 0;
			audiopg->immed = 1;

			if (scsipt_modesel(buf, PG_AUDIOCTL)) {
				/* Success */
				return (vol);
			}
			else if (audiopg->p0_vol != audiopg->p1_vol) {
				/* Set the balance to the center
				 * and retry.
				 */
				audiopg->p0_vol = audiopg->p1_vol =
					scale_vol(taper_vol(vol));

				if (scsipt_modesel(buf, PG_AUDIOCTL)) {
					/* Success: Warp balance control */
					s->level_left = s->level_right = 100;
					set_bal_slider(0);

					return (vol);
				}

				/* Still failed: just drop through */
			}
		}
	}

	return -1;
}


/*
 * scsipt_vendor_model
 *	Query and update CD-ROM vendor/model/revision information
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_vendor_model(curstat_t *s)
{
	inquiry_data_t	inq;
	char		errstr[ERR_BUF_SZ];

	if (scsipt_inquiry((byte_t *) &inq, sizeof(inq))) {
		strncpy(s->vendor, (char *) inq.vendor, 8);
		s->vendor[8] = '\0';

		strncpy(s->prod, (char *) inq.prod, 16);
		s->prod[16] = '\0';

		strncpy(s->revnum, (char *) inq.revnum, 4);
		s->revnum[4] = '\0';

#ifndef OEM_CDROM
		/* Check for errors.
		 * Note: Some OEM drives identify themselves
		 * as a hard disk instead of a CD-ROM drive
		 * (such as the Toshiba CD-ROM XM revision 1971
		 * OEMed by SGI).  In order to use those units
		 * this file must be compiled with -DOEM_CDROM.
		 */
		if (inq.type != DEV_ROM || !inq.rmb) {
			/* Not a CD-ROM device */
			scsipt_notrom_error = TRUE;
			sprintf(errstr, app_data.str_notrom, app_data.device);
			cd_fatal_popup(app_data.str_fatal, errstr);
			return;
		}
#endif

		/* Check for unsupported drives */
		scsipt_dev_scsiver = (byte_t) (inq.ver & 0x07);
		if (scsipt_dev_scsiver < 2 &&
		    app_data.vendor_code == VENDOR_SCSI2) {
			/* Not SCSI-2 or later */
			sprintf(errstr, app_data.str_notscsi2, app_data.device);
			cd_warning_popup(app_data.str_warning, errstr);
		}
	}
}


/*
 * scsipt_get_toc
 *	Query and update the CD Table Of Contents
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_get_toc(curstat_t *s)
{
	int			i;
	byte_t			buf[SZ_RDTOC],
				*cp,
				*toc_end;
	bool_t			ret = FALSE;
	toc_hdr_t		*h;
	toc_trk_descr_t		*p;


	if (scsipt_vutbl[app_data.vendor_code].get_toc != NULL)
		ret = scsipt_vutbl[app_data.vendor_code].get_toc(s);

	if (ret)
		return TRUE;

	/* If the device does not claim SCSI-2 compliance, and the
	 * device-specific configuration is not SCSI-2, then don't
	 * attempt to deliver SCSI-2 commands to the device.
	 */
	if (!ret && app_data.vendor_code != VENDOR_SCSI2 &&
	    scsipt_dev_scsiver < 2)
		return FALSE;

	memset(buf, 0, sizeof(buf));

	if (!scsipt_rdtoc(buf, TRUE, 0))
		return FALSE;

	/* Fill curstat structure with TOC data */
	h = (toc_hdr_t *)(void *) buf;
	toc_end = (byte_t *) h + bswap16(h->data_len) + 2;

	s->first_trk = h->first_trk;
	s->last_trk = h->last_trk;

	cp = (byte_t *) h + sizeof(toc_hdr_t);

	for (i = 0; cp < toc_end && i < MAXTRACK; i++) {
		p = (toc_trk_descr_t *)(void *) cp;

		/* Hack: Work around firmware problem on some drives */
		if (i > 0 && s->trkinfo[i-1].trkno == s->last_trk &&
		    p->trkno != LEAD_OUT_TRACK) {
			memset(buf, 0, sizeof(buf));

			if (!scsipt_rdtoc(buf, TRUE, (int) s->last_trk))
				return FALSE;

			cp = (byte_t *) h + sizeof(toc_hdr_t) +
			     sizeof(toc_trk_descr_t);

			toc_end = (byte_t *) h + bswap16(h->data_len) + 2;

			p = (toc_trk_descr_t *)(void *) cp;
		}

		s->trkinfo[i].trkno = p->trkno;
		s->trkinfo[i].type = (p->trktype == 0) ?
			TYP_AUDIO : TYP_DATA;
		s->trkinfo[i].min = p->abs_addr.msf.min;
		s->trkinfo[i].sec = p->abs_addr.msf.sec;
		s->trkinfo[i].frame = p->abs_addr.msf.frame;
		msftoblk(
			s->trkinfo[i].min,
			s->trkinfo[i].sec,
			s->trkinfo[i].frame,
			&s->trkinfo[i].addr,
			MSF_OFFSET(s)
		);

		if (p->trkno == LEAD_OUT_TRACK ||
		    s->trkinfo[i-1].trkno == s->last_trk ||
		    i == (MAXTRACK - 1)) {
			s->tot_min = s->trkinfo[i].min;
			s->tot_sec = s->trkinfo[i].sec;
			s->tot_frame = s->trkinfo[i].frame;
			s->tot_trks = i;
			s->tot_addr = s->trkinfo[i].addr;

			break;
		}

		cp += sizeof(toc_trk_descr_t);
	}

	return TRUE;
}


/*
 * scsipt_start_stat_poll
 *	Start polling the drive for current playback status
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_start_stat_poll(curstat_t *s)
{
	scsipt_stat_polling = TRUE;

	/* Start poll timer */
	scsipt_stat_id = cd_timeout(
		scsipt_stat_interval,
		scsipt_stat_poll,
		(byte_t *) s
	);
}


/*
 * scsipt_stop_stat_poll
 *	Stop polling the drive for current playback status
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_stop_stat_poll(void)
{
	if (scsipt_stat_polling) {
		/* Stop poll timer */
		cd_untimeout(scsipt_stat_id);

		scsipt_stat_polling = FALSE;
	}
}


/*
 * scsipt_start_insert_poll
 *	Start polling the drive for disc insertion
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_start_insert_poll(curstat_t *s)
{
	if (scsipt_insert_polling || s->mode != M_NODISC)
		return;

	scsipt_insert_polling = TRUE;

	/* Start poll timer */
	scsipt_insert_id = cd_timeout(
		app_data.ins_interval,
		scsipt_insert_poll,
		(byte_t *) s
	);
}


/*
 * scsipt_stop_insert_poll
 *	Stop polling the drive for disc insertion
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_stop_insert_poll(void)
{
	if (scsipt_insert_polling) {
		/* Stop poll timer */
		cd_untimeout(scsipt_insert_id);

		scsipt_insert_polling = FALSE;
	}
}


/*
 * stat_poll
 *	The playback status polling function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_stat_poll(curstat_t *s)
{
	if (!scsipt_stat_polling)
		return;

	/* Get current audio playback status */
	if (scsipt_get_playstatus(s)) {
		/* Register next poll interval */
		scsipt_stat_id = cd_timeout(
			scsipt_stat_interval,
			scsipt_stat_poll,
			(byte_t *) s
		);
	}
	else
		scsipt_stat_polling = FALSE;
}


/*
 * insert_poll
 *	The disc insertion polling function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_insert_poll(curstat_t *s)
{
	/* Check to see if a disc is inserted */
	if (!scsipt_disc_ready(s)) {
		/* Register next poll interval */
		scsipt_insert_id = cd_timeout(
			app_data.ins_interval,
			scsipt_insert_poll,
			(byte_t *) s
		);
	}
	else
		scsipt_insert_polling = FALSE;
}


/*
 * scsipt_disc_ready
 *	Check if the disc is loaded and ready for use, and update
 *	curstat table.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - Disc is ready
 *	FALSE - Disc is not ready
 */
STATIC bool_t
scsipt_disc_ready(curstat_t *s)
{
	int		i,
			vol;
	bool_t		err,
			first_open = FALSE;

	/* If device has not been opened, attempt to open it */
	if (scsipt_not_open) {
		/* Check for another copy of the CD player running on
		 * the specified device.
		 */
		if (!cd_devlock(app_data.device)) {
			dpy_time(s, FALSE);
			scsipt_start_insert_poll(s);
			return FALSE;
		}

		/* Open CD-ROM device */
		if (!pthru_open(app_data.device)) {
			dpy_time(s, FALSE);
			scsipt_start_insert_poll(s);
			return FALSE;
		}

		scsipt_not_open = FALSE;
		first_open = TRUE;
	}

	if (app_data.play_notur && s->mode != M_STOP && s->mode != M_NODISC) {
		/* For those drives that returns failure status to
		 * the Test Unit Ready command during audio playback,
		 * we just silently return success if the drive is
		 * supposed to be playing audio.  Shrug.
		 */
		err = FALSE;
	}
	else for (i = 0; i < 5; i++) {
		/* Send Test Unit Ready command to check if the
		 * drive is ready.
		 */
		if ((err = !scsipt_tst_unit_rdy()) == TRUE) {
			s->mode = M_NODISC;
			dbprog_dbclear(s);
		}
		else
			break;
	}

	if (!err && first_open) {
		/* Start up vendor-unique modules */
		if (scsipt_vutbl[app_data.vendor_code].start != NULL)
			scsipt_vutbl[app_data.vendor_code].start();

		/* Fill in inquiry data */
		scsipt_vendor_model(s);

		/* Query current volume and warp volume and balance
		 * sliders to appropriate setting
		 */
		if ((vol = scsipt_cfg_vol(0, s, TRUE, WARP_VOL | WARP_BAL)) >= 0)
			s->level = (byte_t) vol;
		else
			s->level = 0;

		/* Set up channel routing */
		scsipt_route(s);
	}

	/* Read disc table of contents */
	if (err || (s->mode == M_NODISC && !scsipt_get_toc(s))) {
		reset_curstat(s, TRUE);
		dpy_all(s);

		if (app_data.eject_close) {
			/* Close device */
			pthru_close();

			scsipt_not_open = TRUE;
		}

		scsipt_start_insert_poll(s);
		return FALSE;
	}

	if (s->mode == M_NODISC) {
		/* Load CD database entry for this disc */
		dbprog_dbget(s);

		s->mode = M_STOP;
		dpy_all(s);

		/* Disable front-panel eject button if so specified */
		if (app_data.caddy_lock)
			scsipt_lock(s, TRUE);

		if (app_data.load_play) {
			/* Start auto-play */
			scsipt_play_pause(s);
		}
		else if (app_data.load_spindown) {
			/* Spin down disc in case the user isn't going to
			 * play anything for a while.  This reduces wear and
			 * tear on the drive.
			 */
			scsipt_do_start_stop(FALSE, FALSE);
		}
	}

	return TRUE;
}


/*
 * scsipt_run_rew
 *	Run search-rewind operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_run_rew(curstat_t *s)
{
	int		i,
			skip_blks;
	word32_t	addr,
			end_addr;
	msf_t		smsf,
			emsf;
	static word32_t	start_addr,
			seq;

	/* Find out where we are */
	if (!scsipt_get_playstatus(s)) {
		cd_beep();
		return;
	}

	skip_blks = app_data.skip_blks;
	addr = s->cur_tot_addr;

	if (scsipt_start_search) {
		scsipt_start_search = FALSE;
		seq = 0;
		if (skip_blks < addr)
			start_addr = addr - skip_blks;
		else
			start_addr = 0;
	}
	else {
		if (app_data.skip_spdup > 0 && seq > app_data.skip_spdup)
			/* Speed up search */
			skip_blks *= 3;

		if ((int) (start_addr - skip_blks) > 0)
			start_addr -= skip_blks;
		else
			start_addr = 0;
	}

	seq++;

	if (s->shuffle || s->program) {
		if ((i = curtrk_pos(s)) < 0)
			i = 0;
	}
	else
		i = 0;

	if (start_addr < s->trkinfo[i].addr)
		start_addr = s->trkinfo[i].addr;

	end_addr = start_addr + MAX_SRCH_BLKS;

	blktomsf(start_addr, &smsf.min, &smsf.sec, &smsf.frame, MSF_OFFSET(s));
	blktomsf(end_addr, &emsf.min, &emsf.sec, &emsf.frame, MSF_OFFSET(s));

	/* Play next search interval */
	scsipt_do_playaudio(
		ADDR_BLK | ADDR_MSF | ADDR_OPTEND,
		start_addr, end_addr,
		&smsf, &emsf,
		0, 0
	);

	scsipt_search_id = cd_timeout(
		app_data.skip_pause,
		scsipt_run_rew,
		(byte_t *) s
	);
}


/*
 * scsipt_stop_rew
 *	Stop search-rewind operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
STATIC void
scsipt_stop_rew(curstat_t *s)
{
	cd_untimeout(scsipt_search_id);
}


/*
 * scsipt_run_ff
 *	Run search-fast-forward operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
scsipt_run_ff(curstat_t *s)
{
	int		i,
			skip_blks;
	word32_t	addr,
			end_addr;
	msf_t		smsf,
			emsf;
	static word32_t	start_addr,
			seq;

	/* Find out where we are */
	if (!scsipt_get_playstatus(s)) {
		cd_beep();
		return;
	}

	skip_blks = app_data.skip_blks;
	addr = s->cur_tot_addr;

	if (scsipt_start_search) {
		scsipt_start_search = FALSE;
		seq = 0;
		start_addr = addr + skip_blks;
	}
	else {
		if (app_data.skip_spdup > 0 && seq > app_data.skip_spdup)
			/* Speed up search */
			skip_blks *= 3;

		start_addr += skip_blks;
	}

	seq++;

	if (s->shuffle || s->program) {
		if ((i = curtrk_pos(s)) < 0)
			i = s->tot_trks - 1;
		else if (s->cur_idx == 0)
			/* We're in the lead-in: consider this to be
			 * within the previous track.
			 */
			i--;
	}
	else
		i = s->tot_trks - 1;

	end_addr = start_addr + MAX_SRCH_BLKS;

	if (end_addr >= s->trkinfo[i+1].addr) {
		end_addr = s->trkinfo[i+1].addr;
		start_addr = end_addr - skip_blks;
	}

	blktomsf(start_addr, &smsf.min, &smsf.sec, &smsf.frame, MSF_OFFSET(s));
	blktomsf(end_addr, &emsf.min, &emsf.sec, &emsf.frame, MSF_OFFSET(s));

	/* Play next search interval */
	scsipt_do_playaudio(
		ADDR_BLK | ADDR_MSF | ADDR_OPTEND,
		start_addr, end_addr,
		&smsf, &emsf,
		0, 0
	);

	scsipt_search_id = cd_timeout(
		app_data.skip_pause,
		scsipt_run_ff,
		(byte_t *) s
	);
}


/*
 * scsipt_stop_ff
 *	Stop search-fast-forward operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
STATIC void
scsipt_stop_ff(curstat_t *s)
{
	cd_untimeout(scsipt_search_id);
}


/*
 * scsipt_run_ab
 *	Run a->b segment play operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
/*ARGSUSED*/
STATIC bool_t
scsipt_run_ab(curstat_t *s)
{
	return (
		scsipt_do_playaudio(
			ADDR_BLK | ADDR_MSF,
			scsipt_ab_start_addr, scsipt_ab_end_addr,
			&scsipt_ab_start_msf, &scsipt_ab_end_msf,
			0, 0
		)
	);
}


/*
 * scsipt_run_sample
 *	Run sample play operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_run_sample(curstat_t *s)
{
	word32_t	saddr,
			eaddr;
	msf_t		smsf,
			emsf;

	if (scsipt_next_sam < s->tot_trks) {
		saddr = s->trkinfo[scsipt_next_sam].addr;
		eaddr = saddr + app_data.sample_blks,

		blktomsf(
			saddr,
			&smsf.min,
			&smsf.sec,
			&smsf.frame,
			MSF_OFFSET(s)
		);
		blktomsf(
			eaddr,
			&emsf.min,
			&emsf.sec,
			&emsf.frame,
			MSF_OFFSET(s)
		);

		if (scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
				 saddr, eaddr, &smsf, &emsf, 0, 0)) {
			scsipt_next_sam++;
			return TRUE;
		}
	}

	scsipt_next_sam = 0;
	return FALSE;
}


/*
 * scsipt_run_prog
 *	Run program/shuffle play operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_run_prog(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr,
			end_addr;
	msf_t		start_msf,
			end_msf;
	bool_t		ret;

	if (!s->shuffle && s->prog_tot <= 0)
		return FALSE;

	if (scsipt_new_progshuf) {
		if (s->shuffle)
			/* New shuffle sequence needed */
			reset_shuffle(s);
		else
			/* Program play: simply reset the count */
			s->prog_cnt = 0;

		scsipt_new_progshuf = FALSE;
	}

	if (s->prog_cnt >= s->prog_tot)
		/* Done with program/shuffle play cycle */
		return FALSE;

	if ((i = curprog_pos(s)) < 0)
		return FALSE;

	if (s->trkinfo[i].trkno == LEAD_OUT_TRACK)
		return FALSE;

	s->prog_cnt++;
	s->cur_trk = s->trkinfo[i].trkno;
	s->cur_idx = 1;

	start_addr = s->trkinfo[i].addr + s->cur_trk_addr;
	blktomsf(
		start_addr,
		&s->cur_tot_min,
		&s->cur_tot_sec,
		&s->cur_tot_frame,
		MSF_OFFSET(s)
	);
	start_msf.min = s->cur_tot_min;
	start_msf.sec = s->cur_tot_sec;
	start_msf.frame = s->cur_tot_frame;

	end_addr = s->trkinfo[i+1].addr;
	end_msf.min = s->trkinfo[i+1].min;
	end_msf.sec = s->trkinfo[i+1].sec;
	end_msf.frame = s->trkinfo[i+1].frame;

	s->cur_tot_addr = start_addr;

	if (s->mode != M_PAUSE)
		s->mode = M_PLAY;

	dpy_all(s);

	if (s->trkinfo[i].type == TYP_DATA)
		/* Data track: just fake it */
		return TRUE;

	ret = scsipt_do_playaudio(
		ADDR_BLK | ADDR_MSF,
		start_addr, end_addr,
		&start_msf, &end_msf,
		0, 0
	);

	if (s->mode == M_PAUSE) {
		scsipt_do_pause_resume(FALSE);

		/* Restore volume */
		scsipt_mute_off(s);
	}

	return (ret);
}


/*
 * scsipt_run_repeat
 *	Run repeat play operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
scsipt_run_repeat(curstat_t *s)
{
	msf_t	start_msf,
		end_msf;
	int	ret;

	if (!s->repeat)
		return FALSE;

	if (s->prog_tot > 0) {
		ret = TRUE;

		if (s->prog_cnt < s->prog_tot)
			/* Not done with program/shuffle sequence yet */
			return (ret);

		scsipt_new_progshuf = TRUE;
		s->rptcnt++;
	}
	else {
		s->cur_trk = s->first_trk;
		s->cur_idx = 1;

		s->cur_tot_addr = 0;
		s->cur_tot_min = 0;
		s->cur_tot_sec = 0;
		s->cur_tot_frame = 0;
		s->rptcnt++;
		dpy_all(s);

		start_msf.min = s->trkinfo[0].min;
		start_msf.sec = s->trkinfo[0].sec;
		start_msf.frame = s->trkinfo[0].frame;
		end_msf.min = s->tot_min;
		end_msf.sec = s->tot_sec;
		end_msf.frame = s->tot_frame;

		ret = scsipt_do_playaudio(
			ADDR_BLK | ADDR_MSF,
			s->trkinfo[0].addr, s->tot_addr,
			&start_msf, &end_msf, 0, 0
		);

		if (s->mode == M_PAUSE) {
			scsipt_do_pause_resume(FALSE);

			/* Restore volume */
			scsipt_mute_off(s);
		}

	}

	return (ret);
}


/*
 * scsipt_route_val
 *	Return the channel routing control value used in the
 *	SCSI-2 mode parameter page 0xE (audio parameters).
 *
 * Args:
 *	route_mode - The channel routing mode value.
 *	channel - The channel number desired (0=left 1=right).
 *
 * Return:
 *	The routing control value.
 */
STATIC byte_t
scsipt_route_val(int route_mode, int channel)
{
	switch (channel) {
	case 0:
		switch (route_mode) {
		case 0:
			return 0x1;
			break;
		case 1:
			return 0x2;
			break;
		case 2:
			return 0x1;
			break;
		case 3:
			return 0x2;
			break;
		case 4:
			return 0x3;
			break;
		default:
			/* Invalid value */
			return 0x0;
			break;
		}
		break;

	case 1:
		switch (route_mode) {
		case 0:
			return 0x2;
			break;
		case 1:
			return 0x1;
			break;
		case 2:
			return 0x1;
			break;
		case 3:
			return 0x2;
			break;
		case 4:
			return 0x3;
			break;
		default:
			/* Invalid value */
			return 0x0;
			break;
		}
		break;

	default:
		/* Invalid value */
		return 0x0;
	}
}


/***********************
 *   public routines   *
 ***********************/


/*
 * scsipt_init
 *	Top-level function to initialize the SCSI pass-through and
 *	vendor-unique modules.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_init(curstat_t *s, di_tbl_t *dt)
{
	int	i;

	if (app_data.di_method != DI_SCSIPT)
		/* SCSI pass-through not configured */
		return;

	/* Initialize libdi calling table */
	dt->check_disc = scsipt_check_disc;
	dt->status_upd = scsipt_status_upd;
	dt->lock = scsipt_lock;
	dt->repeat = scsipt_repeat;
	dt->shuffle = scsipt_shuffle;
	dt->load_eject = scsipt_load_eject;
	dt->ab = scsipt_ab;
	dt->sample = scsipt_sample;
	dt->level = scsipt_level;
	dt->play_pause = scsipt_play_pause;
	dt->stop = scsipt_stop;
	dt->prevtrk = scsipt_prevtrk;
	dt->nexttrk = scsipt_nexttrk;
	dt->previdx = scsipt_previdx;
	dt->nextidx = scsipt_nextidx;
	dt->rew = scsipt_rew;
	dt->ff = scsipt_ff;
	dt->warp = scsipt_warp;
	dt->route = scsipt_route;
	dt->mute_on = scsipt_mute_on;
	dt->mute_off = scsipt_mute_off;
	dt->start = scsipt_start;
	dt->icon = scsipt_icon;
	dt->halt = scsipt_halt;
	dt->mode = scsipt_mode;
	dt->vers = scsipt_vers;

	/* Initalize SCSI pass-through module */
	scsipt_stat_polling = FALSE;
	scsipt_stat_interval = app_data.stat_interval;
	scsipt_insert_polling = FALSE;
	scsipt_next_sam = FALSE;
	scsipt_new_progshuf = FALSE;
	scsipt_sav_end_addr = 0;
	scsipt_sav_end_msf.min = scsipt_sav_end_msf.sec =
		scsipt_sav_end_msf.frame = 0;
	scsipt_sav_end_fmt = 0;

#ifdef SETUID_ROOT
#ifdef SOL2_VOLMGT
	if (!app_data.sol2_volmgt)
#endif	/* SOL2_VOLMGT */
	{
		DBGPRN(errfp, "\nSetting uid to 0\n");

		if (setuid(0) < 0 || getuid() != 0) {
			cd_fatal_popup(app_data.str_fatal,
				       app_data.str_moderr);
		}
	}
#endif	/* SETUID_ROOT */

	/* Initialize curstat structure */
	reset_curstat(s, TRUE);

	/* Initialize the SCSI-2 entry of the scsipt_vutbl jump table */
	scsipt_vutbl[VENDOR_SCSI2].vendor = "SCSI-2";
	scsipt_vutbl[VENDOR_SCSI2].playaudio = NULL;
	scsipt_vutbl[VENDOR_SCSI2].pause_resume = NULL;
	scsipt_vutbl[VENDOR_SCSI2].start_stop = NULL;
	scsipt_vutbl[VENDOR_SCSI2].get_playstatus = NULL;
	scsipt_vutbl[VENDOR_SCSI2].volume = NULL;
	scsipt_vutbl[VENDOR_SCSI2].route = NULL;
	scsipt_vutbl[VENDOR_SCSI2].mute = NULL;
	scsipt_vutbl[VENDOR_SCSI2].get_toc = NULL;
	scsipt_vutbl[VENDOR_SCSI2].eject = NULL;
	scsipt_vutbl[VENDOR_SCSI2].start = NULL;
	scsipt_vutbl[VENDOR_SCSI2].halt = NULL;

	/* Initialize all configured vendor-unique modules */
	for (i = 0; i < MAX_VENDORS; i++) {
		if (vuinit[i].init != NULL)
			vuinit[i].init();
	}

	if (app_data.vendor_code != VENDOR_SCSI2 &&
	    vuinit[app_data.vendor_code].init == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_novu);
	}
}


/*
 * scsipt_check_disc
 *	Check if disc is ready for use
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
scsipt_check_disc(curstat_t *s)
{
	return (scsipt_disc_ready(s));
}


/*
 * scsipt_status_upd
 *	Force update of playback status
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_status_upd(curstat_t *s)
{
	scsipt_get_playstatus(s);
}


/*
 * scsipt_lock
 *	Caddy lock function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	enable - whether to enable/disable caddy lock
 *
 * Return:
 *	Nothing.
 */
void
scsipt_lock(curstat_t *s, bool_t enable)
{
	if (s->mode == M_NODISC || !scsipt_prev_allow(enable)) {
		/* Cannot lock/unlock caddy */
		cd_beep();
		set_lock_btn((bool_t) !enable);
		return;
	}

	s->caddy_lock = enable;
	set_lock_btn((bool_t) enable);
}


/*
 * scsipt_repeat
 *	Repeat mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	enable - whether to enable/disable repeat mode
 *
 * Return:
 *	Nothing.
 */
void
scsipt_repeat(curstat_t *s, bool_t enable)
{
	s->repeat = enable;
	dpy_rptcnt(s);
}


/*
 * scsipt_shuffle
 *	Shuffle mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	enable - whether to enable/disable shuffle mode
 *
 * Return:
 *	Nothing.
 */
void
scsipt_shuffle(curstat_t *s, bool_t enable)
{
	switch (s->mode) {
	case M_STOP:
	case M_NODISC:
		if (s->prog_tot > 0 && !s->shuffle) {
			/* Currently in program mode: can't enable shuffle */
			cd_beep();
			set_shuffle_btn((bool_t) !enable);
			return;
		}
		break;
	default:
		if (enable) {
			/* Can't enable shuffle unless when stopped */
			cd_beep();
			set_shuffle_btn((bool_t) !enable);
			return;
		}
		break;
	}

	s->shuffle = enable;
	if (!s->shuffle)
		s->prog_tot = 0;
}


/*
 * scsipt_load_eject
 *	CD caddy load and eject function.  If disc caddy is not
 *	loaded, it will attempt to load it.  Otherwise, it will be
 *	ejected.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_load_eject(curstat_t *s)
{
	bool_t	ret = FALSE;

	if (!scsipt_disc_ready(s)) {
		/* Disc not ready: try loading the disc */
		if (!scsipt_do_start_stop(TRUE, TRUE))
			cd_beep();

		return;
	}

	/* Eject the disc */

	if (!app_data.eject_supp) {
		cd_beep();

		scsipt_stop_stat_poll();
		reset_curstat(s, TRUE);
		s->mode = M_NODISC;

		dbprog_dbclear(s);
		dpy_all(s);

		if (app_data.eject_close) {
			/* Close device */
			pthru_close();

			scsipt_not_open = TRUE;
		}

		scsipt_start_insert_poll(s);
		return;
	}

	/* Unlock caddy if necessary */
	if (s->caddy_lock)
		scsipt_lock(s, FALSE);

	scsipt_stop_stat_poll();
	reset_curstat(s, TRUE);
	s->mode = M_NODISC;

	dbprog_dbclear(s);
	dpy_all(s);

	scsipt_do_start_stop(FALSE, TRUE);

	if (app_data.eject_exit)
		cd_quit(s);
	else {
		if (app_data.eject_close) {
			/* Close device */
			pthru_close();

			scsipt_not_open = TRUE;
		}

		scsipt_start_insert_poll(s);
	}
}


/*
 * scsipt_ab
 *	A->B segment play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_ab(curstat_t *s)
{
	switch (s->mode) {
	case M_SAMPLE:
	case M_PLAY:
		/* Get current location */
		if (!scsipt_get_playstatus(s)) {
			cd_beep();
			break;
		}

		scsipt_ab_start_addr = s->cur_tot_addr;
		scsipt_ab_start_msf.min = s->cur_tot_min;
		scsipt_ab_start_msf.sec = s->cur_tot_sec;
		scsipt_ab_start_msf.frame = s->cur_tot_frame;

		s->mode = M_A;
		dpy_playmode(s, FALSE);
		break;

	case M_A:
		/* Get current location */
		if (!scsipt_get_playstatus(s)) {
			cd_beep();
			break;
		}

		scsipt_ab_end_addr = s->cur_tot_addr;
		scsipt_ab_end_msf.min = s->cur_tot_min;
		scsipt_ab_end_msf.sec = s->cur_tot_sec;
		scsipt_ab_end_msf.frame = s->cur_tot_frame;

		/* Make sure that the A->B play interval is no less
		 * than a user-configurable minimum.
		 */
		if ((scsipt_ab_end_addr - scsipt_ab_start_addr) <
		    app_data.min_playblks) {
			scsipt_ab_end_addr = scsipt_ab_start_addr +
					     app_data.min_playblks;
			blktomsf(
				scsipt_ab_end_addr,
				&scsipt_ab_end_msf.min,
				&scsipt_ab_end_msf.sec,
				&scsipt_ab_end_msf.frame,
				MSF_OFFSET(s)
			);
		}

		if (!scsipt_run_ab(s)) {
			cd_beep();
			return;
		}

		s->mode = M_AB;
		dpy_playmode(s, FALSE);
		break;

	case M_AB:
		/* Currently doing A->B playback, just call scsipt_play_pause
		 * to resume normal playback.
		 */
		scsipt_play_pause(s);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_sample
 *	Sample play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_sample(curstat_t *s)
{
	int	i;

	if (!scsipt_disc_ready(s)) {
		cd_beep();
		return;
	}

	if (s->shuffle || s->prog_tot > 0) {
		/* Sample is not supported in program/shuffle mode */
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_STOP:
		scsipt_start_stat_poll(s);
		/*FALLTHROUGH*/
	case M_A:
	case M_AB:
	case M_PLAY:
		/* If already playing a track, start sampling the track after
		 * the current one.  Otherwise, sample from the beginning.
		 */
		if (s->cur_trk > 0 && s->cur_trk != s->last_trk) {
			i = curtrk_pos(s) + 1;
			s->cur_trk = s->trkinfo[i].trkno;
			scsipt_next_sam = i;
		}
		else {
			s->cur_trk = s->first_trk;
			scsipt_next_sam = 0;
		}
		
		s->cur_idx = 1;

		s->mode = M_SAMPLE;
		dpy_all(s);

		if (!scsipt_run_sample(s))
			return;

		break;

	case M_SAMPLE:
		/* Currently doing Sample playback, just call scsipt_play_pause
		 * to resume normal playback.
		 */
		scsipt_play_pause(s);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_level
 *	Audio volume control function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	level - The volume level to set to
 *	drag - Whether this is an update due to the user dragging the
 *		volume control slider thumb.  If this is FALSE, then
 *		a final volume setting has been found.
 *
 * Return:
 *	Nothing.
 */
void
scsipt_level(curstat_t *s, byte_t level, bool_t drag)
{
	int	actual;
	byte_t	warpflg;

	if (drag && app_data.vendor_code != VENDOR_SCSI2 &&
	    scsipt_vutbl[app_data.vendor_code].volume == NULL)
		return;

	if (drag)
		warpflg = WARP_VOL;
	else
		warpflg = WARP_VOL | WARP_BAL;

	/* Set volume level */
	if ((actual = scsipt_cfg_vol((int) level, s, FALSE, warpflg)) >= 0)
		s->level = (byte_t) actual;
}


/*
 * scsipt_play_pause
 *	Audio playback and pause function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_play_pause(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;

	if (!scsipt_disc_ready(s)) {
		cd_beep();
		return;
	}

	if (s->mode == M_NODISC)
		s->mode = M_STOP;

	switch (s->mode) {
	case M_PLAY:
		/* Currently playing: go to pause mode */

		if (!scsipt_do_pause_resume(FALSE)) {
			cd_beep();
			return;
		}
		scsipt_stop_stat_poll();
		s->mode = M_PAUSE;
		dpy_playmode(s, FALSE);
		break;

	case M_PAUSE:
		/* Currently paused: resume play */

		if (!scsipt_do_pause_resume(TRUE)) {
			cd_beep();
			return;
		}
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		scsipt_start_stat_poll(s);
		break;

	case M_STOP:
		/* Currently stopped: start play */

		if (s->shuffle || s->prog_tot > 0) {
			scsipt_new_progshuf = TRUE;

			/* Start shuffle/program play */
			if (!scsipt_run_prog(s)) {
				s->program = FALSE;
				return;
			}

			s->program = !s->shuffle;
		}
		else {
			/* Start normal play */
			if ((i = curtrk_pos(s)) < 0 || s->cur_trk <= 0) {
				/* Start play from the beginning */
				i = 0;
				s->cur_trk = s->first_trk;
				start_addr = s->trkinfo[0].addr +
					     s->cur_trk_addr;
				blktomsf(start_addr,
					 &start_msf.min,
					 &start_msf.sec,
					 &start_msf.frame,
					 MSF_OFFSET(s)
				);
			}
			else {
				/* User has specified a starting track */
				start_addr = s->trkinfo[i].addr +
					     s->cur_trk_addr;
			}

			blktomsf(start_addr,
				 &start_msf.min,
				 &start_msf.sec,
				 &start_msf.frame,
				 MSF_OFFSET(s)
			);

			end_msf.min = s->tot_min;
			end_msf.sec = s->tot_sec;
			end_msf.frame = s->tot_frame;

			s->cur_idx = 1;
			s->mode = M_PLAY;
			if (s->trkinfo[i].type == TYP_DATA)
				dpy_time(s, FALSE);

			if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				s->mode = M_STOP;
				return;
			}
		}

		dpy_all(s);
		scsipt_start_stat_poll(s);
		break;

	case M_A:
		/* Just reset mode to play and continue */
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		break;

	case M_AB:
	case M_SAMPLE:
		/* Force update of curstat */
		if (!scsipt_get_playstatus(s)) {
			cd_beep();
			return;
		}

		/* Currently doing a->b or sample playback: just resume play */
		if (s->shuffle || s->program) {
			if ((i = curtrk_pos(s)) < 0 ||
			    s->trkinfo[i].trkno == LEAD_OUT_TRACK)
				return;

			start_msf.min = s->cur_tot_min;
			start_msf.sec = s->cur_tot_sec;
			start_msf.frame = s->cur_tot_frame;
			end_msf.min = s->trkinfo[i+1].min;
			end_msf.sec = s->trkinfo[i+1].sec;
			end_msf.frame = s->trkinfo[i+1].frame;

			if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
					  s->cur_tot_addr,
					  s->trkinfo[i+1].addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				return;
			}
		}
		else {
			start_msf.min = s->cur_tot_min;
			start_msf.sec = s->cur_tot_sec;
			start_msf.frame = s->cur_tot_frame;
			end_msf.min = s->tot_min;
			end_msf.sec = s->tot_sec;
			end_msf.frame = s->tot_frame;

			if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
					  s->cur_tot_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				return;
			}
		}
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_stop
 *	Stop function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	stop_disc - Whether to actually spin down the disc or just
 *		update status.
 *
 * Return:
 *	Nothing.
 */
void
scsipt_stop(curstat_t *s, bool_t stop_disc)
{
	/* The stop_disc parameter will cause the disc to spin down.
	 * This is usually set to TRUE, but can be FALSE if the caller
	 * just wants to set the current state to stop but will
	 * immediately go into play state again.  Not spinning down
	 * the drive makes things a little faster...
	 */

	if (!scsipt_disc_ready(s)) {
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_PLAY:
	case M_PAUSE:
	case M_A:
	case M_AB:
	case M_SAMPLE:
	case M_STOP:
		/* Currently playing or paused: stop */

		if (stop_disc && !scsipt_do_start_stop(FALSE, FALSE)) {
			cd_beep();
			return;
		}
		scsipt_stop_stat_poll();

		reset_curstat(s, FALSE);
		s->mode = M_STOP;

		dpy_all(s);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_prevtrk
 *	Previous track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_prevtrk(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;
	bool_t		go_prev;

	if (!scsipt_disc_ready(s)) {
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		/* Find appropriate track to start */
		if (s->prog_tot > 0) {
			if (s->prog_cnt > 0) {
				s->prog_cnt--;
				scsipt_new_progshuf = FALSE;
			}
			i = curprog_pos(s);
		}
		else
			i = curtrk_pos(s);

		if (i < 0)
			return;

		start_addr = s->trkinfo[i].addr;
		start_msf.min = s->trkinfo[i].min;
		start_msf.sec = s->trkinfo[i].sec;
		start_msf.frame = s->trkinfo[i].frame;
		s->cur_trk = s->trkinfo[i].trkno;
		s->cur_idx = 1;

		/* If the current track has been playing for less
		 * than app_data.prev_threshold blocks, then go
		 * to the beginning of the previous track (if we
		 * are not already on the first track).
		 */
		go_prev = FALSE;
		if ((s->cur_tot_addr - start_addr) < app_data.prev_threshold)
			go_prev = TRUE;

		if (go_prev) {
			if (s->prog_tot > 0) {
				if (s->prog_cnt > 0) {
					s->prog_cnt--;
					scsipt_new_progshuf = FALSE;
				}
				if ((i = curprog_pos(s)) < 0)
					return;

				start_addr = s->trkinfo[i].addr;
				start_msf.min = s->trkinfo[i].min;
				start_msf.sec = s->trkinfo[i].sec;
				start_msf.frame = s->trkinfo[i].frame;
				s->cur_trk = s->trkinfo[i].trkno;
			}
			else if (s->prog_tot <= 0 && i > 0) {
				start_addr = s->trkinfo[i-1].addr;
				start_msf.min = s->trkinfo[i-1].min;
				start_msf.sec = s->trkinfo[i-1].sec;
				start_msf.frame = s->trkinfo[i-1].frame;
				s->cur_trk = s->trkinfo[i-1].trkno;
			}
		}

		if (s->mode == M_PAUSE)
			/* Mute: so we don't get a transient */
			scsipt_mute_on(s);

		if (s->prog_tot > 0) {
			/* Program/Shuffle mode: just stop the playback
			 * and let scsipt_run_prog go to the previous track
			 */
			scsipt_fake_stop = TRUE;

			/* Force status update */
			scsipt_get_playstatus(s);
		}
		else {
			end_msf.min = s->tot_min;
			end_msf.sec = s->tot_sec;
			end_msf.frame = s->tot_frame;

			s->cur_tot_addr = start_addr;
			s->cur_tot_min = start_msf.min;
			s->cur_tot_sec = start_msf.sec;
			s->cur_tot_frame = start_msf.frame;
			s->cur_trk_addr = 0;
			s->cur_trk_min = s->cur_trk_sec = s->cur_trk_frame = 0;

			dpy_track(s);
			dpy_index(s);
			dpy_time(s, FALSE);

			if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();

				/* Restore volume */
				scsipt_mute_off(s);
				return;
			}

			if (s->mode == M_PAUSE) {
				scsipt_do_pause_resume(FALSE);

				/* Restore volume */
				scsipt_mute_off(s);
			}
		}

		break;

	case M_STOP:
		if (s->prog_tot > 0) {
			/* Pre-selecting tracks not supported in shuffle
			 * or program mode.
			 */
			cd_beep();
			return;
		}

		/* Find previous track */
		if (s->cur_trk <= 0) {
			s->cur_trk = s->trkinfo[0].trkno;
			dpy_track(s);
		}
		else {
			i = curtrk_pos(s);

			if (i > 0) {
				s->cur_trk = s->trkinfo[i-1].trkno;
				dpy_track(s);
			}
		}
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_nexttrk
 *	Next track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_nexttrk(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;

	if (!scsipt_disc_ready(s)) {
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (s->prog_tot > 0) {
			if (s->prog_cnt >= s->prog_tot) {
				/* Disallow advancing beyond current
				 * shuffle/program sequence if
				 * repeat mode is not on.
				 */
				if (s->repeat)
					scsipt_new_progshuf = TRUE;
				else
					return;
			}

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				scsipt_mute_on(s);

			/* Program/Shuffle mode: just stop the playback
			 * and let scsipt_run_prog go to the next track.
			 */
			scsipt_fake_stop = TRUE;

			/* Force status update */
			scsipt_get_playstatus(s);

			return;
		}

		/* Find next track */
		if ((i = curtrk_pos(s)) < 0)
			return;

		if (i < (MAXTRACK - 1) &&
		    s->trkinfo[i+1].trkno >= 0 &&
		    s->trkinfo[i+1].trkno != LEAD_OUT_TRACK) {

			start_addr = s->trkinfo[i+1].addr;
			start_msf.min = s->trkinfo[i+1].min;
			start_msf.sec = s->trkinfo[i+1].sec;
			start_msf.frame = s->trkinfo[i+1].frame;
			s->cur_trk = s->trkinfo[i+1].trkno;
			s->cur_idx = 1;

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				scsipt_mute_on(s);

			end_msf.min = s->tot_min;
			end_msf.sec = s->tot_sec;
			end_msf.frame = s->tot_frame;

			s->cur_tot_addr = start_addr;
			s->cur_tot_min = start_msf.min;
			s->cur_tot_sec = start_msf.sec;
			s->cur_tot_frame = start_msf.frame;
			s->cur_trk_addr = 0;
			s->cur_trk_min = s->cur_trk_sec = s->cur_trk_frame = 0;

			dpy_track(s);
			dpy_index(s);
			dpy_time(s, FALSE);

			if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				return;
			}

			if (s->mode == M_PAUSE) {
				scsipt_do_pause_resume(FALSE);

				/* Restore volume */
				scsipt_mute_off(s);
			}
		}

		break;

	case M_STOP:
		if (s->prog_tot > 0) {
			/* Pre-selecting tracks not supported in shuffle
			 * or program mode.
			 */
			cd_beep();
			return;
		}

		/* Find next track */
		if (s->cur_trk <= 0) {
			s->cur_trk = s->trkinfo[0].trkno;
			dpy_track(s);
		}
		else {
			i = curtrk_pos(s);

			if (i >= 0 &&
			    s->trkinfo[i+1].trkno != LEAD_OUT_TRACK) {
				s->cur_trk = s->trkinfo[i+1].trkno;
				dpy_track(s);
			}
		}
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_previdx
 *	Previous index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_previdx(curstat_t *s)
{
	msf_t		start_msf,
			end_msf;
	byte_t		idx;

	if (s->prog_tot > 0) {
		/* Index search is not supported in program/shuffle mode */
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		/* Find appropriate index to start */
		if (s->cur_idx > 1 &&
		    (s->cur_tot_addr - s->sav_iaddr) < app_data.prev_threshold)
			idx = s->cur_idx - 1;
		else
			idx = s->cur_idx;
		
		/* This is a Hack...
		 * Since there is no standard SCSI-2 command to start
		 * playback on an index boundary and then go on playing
		 * until the end of the disc, we will use the PLAY AUDIO
		 * TRACK/INDEX command to go to where we want to start,
		 * immediately followed by a PAUSE.  We then find the
		 * current block position and issue a PLAY AUDIO MSF
		 * or PLAY AUDIO(12) command to start play there.
		 * We mute the audio in between these operations to
		 * prevent unpleasant transients.
		 */

		/* Mute */
		scsipt_mute_on(s);

		if (!scsipt_do_playaudio(ADDR_TRKIDX, 0, 0, NULL, NULL,
				  (byte_t) s->cur_trk, idx)) {
			/* Restore volume */
			scsipt_mute_off(s);
			cd_beep();
			return;
		}

		scsipt_idx_pause = TRUE;

		if (!scsipt_do_pause_resume(FALSE)) {
			/* Restore volume */
			scsipt_mute_off(s);
			scsipt_idx_pause = FALSE;
			return;
		}

		/* Use scsipt_get_playstatus to update the current status */
		if (!scsipt_get_playstatus(s)) {
			/* Restore volume */
			scsipt_mute_off(s);
			scsipt_idx_pause = FALSE;
			return;
		}

		/* Save starting block addr of this index */
		s->sav_iaddr = s->cur_tot_addr;

		if (s->mode != M_PAUSE)
			/* Restore volume */
			scsipt_mute_off(s);

		start_msf.min = s->cur_tot_min;
		start_msf.sec = s->cur_tot_sec;
		start_msf.frame = s->cur_tot_frame;
		end_msf.min = s->tot_min;
		end_msf.sec = s->tot_sec;
		end_msf.frame = s->tot_frame;

		if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
				  s->cur_tot_addr, s->tot_addr,
				  &start_msf, &end_msf, 0, 0)) {
			cd_beep();
			scsipt_idx_pause = FALSE;
			return;
		}

		scsipt_idx_pause = FALSE;

		if (s->mode == M_PAUSE) {
			scsipt_do_pause_resume(FALSE);

			/* Restore volume */
			scsipt_mute_off(s);

			/* Force update of curstat */
			scsipt_get_playstatus(s);
		}

		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_nextidx
 *	Next index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_nextidx(curstat_t *s)
{
	msf_t		start_msf,
			end_msf;

	if (s->prog_tot > 0) {
		/* Index search is not supported in program/shuffle mode */
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		/* Find appropriate index to start */
		
		/* This is a Hack...
		 * Since there is no standard SCSI-2 command to start
		 * playback on an index boundary and then go on playing
		 * until the end of the disc, we will use the PLAY AUDIO
		 * TRACK/INDEX command to go to where we want to start,
		 * immediately followed by a PAUSE.  We then find the
		 * current block position and issue a PLAY AUDIO MSF
		 * or PLAY AUDIO(12) command to start play there.
		 * We mute the audio in between these operations to
		 * prevent unpleasant transients.
		 */

		/* Mute */
		scsipt_mute_on(s);

		if (!scsipt_do_playaudio(ADDR_TRKIDX, 0, 0, NULL, NULL,
				  (byte_t) s->cur_trk,
				  (byte_t) (s->cur_idx + 1))) {
			/* Restore volume */
			scsipt_mute_off(s);
			cd_beep();
			return;
		}

		scsipt_idx_pause = TRUE;

		if (!scsipt_do_pause_resume(FALSE)) {
			/* Restore volume */
			scsipt_mute_off(s);
			scsipt_idx_pause = FALSE;
			return;
		}

		/* Use scsipt_get_playstatus to update the current status */
		if (!scsipt_get_playstatus(s)) {
			/* Restore volume */
			scsipt_mute_off(s);
			scsipt_idx_pause = FALSE;
			return;
		}

		/* Save starting block addr of this index */
		s->sav_iaddr = s->cur_tot_addr;

		if (s->mode != M_PAUSE)
			/* Restore volume */
			scsipt_mute_off(s);

		start_msf.min = s->cur_tot_min;
		start_msf.sec = s->cur_tot_sec;
		start_msf.frame = s->cur_tot_frame;
		end_msf.min = s->tot_min;
		end_msf.sec = s->tot_sec;
		end_msf.frame = s->tot_frame;

		if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
				  s->cur_tot_addr, s->tot_addr,
				  &start_msf, &end_msf, 0, 0)) {
			cd_beep();
			scsipt_idx_pause = FALSE;
			return;
		}

		scsipt_idx_pause = FALSE;

		if (s->mode == M_PAUSE) {
			scsipt_do_pause_resume(FALSE);

			/* Restore volume */
			scsipt_mute_off(s);

			/* Force update of curstat */
			scsipt_get_playstatus(s);
		}

		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * scsipt_rew
 *	Search-rewind function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_rew(curstat_t *s, bool_t start)
{
	sword32_t	i;
	msf_t		start_msf,
			end_msf;
	byte_t		vol;

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		/* Go to normal play mode first */
		scsipt_play_pause(s);

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (start) {
			/* Button press */

			if (s->mode == M_PLAY)
				scsipt_stop_stat_poll();

			/* Reduce volume */
			vol = (byte_t) ((int) s->level *
				app_data.skip_vol / 100);

			(void) scsipt_cfg_vol((int)
				((vol < (byte_t)app_data.skip_minvol) ?
				 (byte_t) app_data.skip_minvol : vol),
				s,
				FALSE,
				0
			);

			/* Start search rewind */
			scsipt_start_search = TRUE;
			scsipt_run_rew(s);
		}
		else {
			/* Button release */

			scsipt_stop_rew(s);

			/* Update display */
			scsipt_get_playstatus(s);

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				scsipt_mute_on(s);
			else
				/* Restore volume */
				scsipt_mute_off(s);

			if (s->shuffle || s->program) {
				if ((i = curtrk_pos(s)) < 0 ||
				    s->trkinfo[i].trkno == LEAD_OUT_TRACK) {
					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}

				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->trkinfo[i+1].min;
				end_msf.sec = s->trkinfo[i+1].sec;
				end_msf.frame = s->trkinfo[i+1].frame;

				if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->trkinfo[i+1].addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}
			}
			else {
				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->tot_min;
				end_msf.sec = s->tot_sec;
				end_msf.frame = s->tot_frame;

				if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->tot_addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}
			}

			if (s->mode == M_PAUSE) {
				scsipt_do_pause_resume(FALSE);

				/* Restore volume */
				scsipt_mute_off(s);
			}
			else
				scsipt_start_stat_poll(s);
		}
		break;

	default:
		if (start)
			cd_beep();
		break;
	}
}


/*
 * scsipt_ff
 *	Search-fast-forward function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_ff(curstat_t *s, bool_t start)
{
	sword32_t	i;
	msf_t		start_msf,
			end_msf;
	byte_t		vol;

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		/* Go to normal play mode first */
		scsipt_play_pause(s);

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (start) {
			/* Button press */

			if (s->mode == M_PLAY)
				scsipt_stop_stat_poll();

			/* Reduce volume */
			vol = (byte_t) ((int) s->level *
				app_data.skip_vol / 100);

			(void) scsipt_cfg_vol((int)
				((vol < (byte_t)app_data.skip_minvol) ?
				 (byte_t) app_data.skip_minvol : vol),
				s,
				FALSE,
				0
			);

			/* Start search forward */
			scsipt_start_search = TRUE;
			scsipt_run_ff(s);
		}
		else {
			/* Button release */

			scsipt_stop_ff(s);

			/* Update display */
			scsipt_get_playstatus(s);

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				scsipt_mute_on(s);
			else
				/* Restore volume */
				scsipt_mute_off(s);

			if (s->shuffle || s->program) {
				if ((i = curtrk_pos(s)) < 0 ||
				    s->trkinfo[i].trkno == LEAD_OUT_TRACK) {
					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}

				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->trkinfo[i+1].min;
				end_msf.sec = s->trkinfo[i+1].sec;
				end_msf.frame = s->trkinfo[i+1].frame;

				if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->trkinfo[i+1].addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}
			}
			else {
				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->tot_min;
				end_msf.sec = s->tot_sec;
				end_msf.frame = s->tot_frame;

				if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->tot_addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					scsipt_mute_off(s);
					return;
				}
			}
			if (s->mode == M_PAUSE) {
				scsipt_do_pause_resume(FALSE);

				/* Restore volume */
				scsipt_mute_off(s);
			}
			else
				scsipt_start_stat_poll(s);
		}
		break;

	default:
		if (start)
			cd_beep();
		break;
	}
}


/*
 * scsipt_warp
 *	Track warp function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_warp(curstat_t *s)
{
	word32_t	start_addr,
			end_addr;
	msf_t		start_msf,
			end_msf;
	int		i;

	start_addr = s->cur_tot_addr;
	start_msf.min = s->cur_tot_min;
	start_msf.sec = s->cur_tot_sec;
	start_msf.frame = s->cur_tot_frame;

	switch (s->mode) {
	case M_A:
	case M_AB:
	case M_SAMPLE:
		/* Go to normal play mode first */
		scsipt_play_pause(s);

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (s->shuffle || s->program) {
			if ((i = curtrk_pos(s)) < 0) {
				cd_beep();
				return;
			}

			end_addr = s->trkinfo[i+1].addr;
			end_msf.min = s->trkinfo[i+1].min;
			end_msf.sec = s->trkinfo[i+1].sec;
			end_msf.frame = s->trkinfo[i+1].frame;
		}
		else {
			end_addr = s->tot_addr;
			end_msf.min = s->tot_min;
			end_msf.sec = s->tot_sec;
			end_msf.frame = s->tot_frame;
		}

		if (s->mode == M_PAUSE)
			/* Mute: so we don't get a transient */
			scsipt_mute_on(s);

		if (!scsipt_do_playaudio(ADDR_BLK | ADDR_MSF,
				  start_addr, end_addr,
				  &start_msf, &end_msf,
				  0, 0)) {
			cd_beep();

			/* Restore volume */
			scsipt_mute_off(s);
			return;
		}

		if (s->mode == M_PAUSE) {
			scsipt_do_pause_resume(FALSE);

			/* Restore volume */
			scsipt_mute_off(s);
		}

		break;

	default:
		break;
	}
}


/*
 * scsipt_route
 *	Channel routing function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_route(curstat_t *s)
{
	byte_t	val0,
		val1;
	bool_t	ret;

	if (!app_data.chroute_supp)
		return;

	if (scsipt_vutbl[app_data.vendor_code].route != NULL) {
		(void) scsipt_vutbl[app_data.vendor_code].route(s);
		return;
	}

	val0 = scsipt_route_val(app_data.ch_route, 0);
	val1 = scsipt_route_val(app_data.ch_route, 1);

	if (val0 == scsipt_route_left && val1 == scsipt_route_right)
		/* No change: just return */
		return;

	scsipt_route_left = val0;
	scsipt_route_right = val1;

	/* With SCSI-2, channel routing is done with the volume control */
	(void) scsipt_cfg_vol(s->level, s, FALSE, 0);
}


/*
 * scsipt_mute_on
 *	Mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_mute_on(curstat_t *s)
{
	(void) scsipt_cfg_vol(0, s, FALSE, 0);
}


/*
 * scsipt_mute_off
 *	Un-mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_mute_off(curstat_t *s)
{
	(void) scsipt_cfg_vol((int) s->level, s, FALSE, 0);
}


/*
 * scsipt_start
 *	Start the SCSI pass-through module.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_start(curstat_t *s)
{
	/* Check to see if disc is ready */
	(void) scsipt_disc_ready(s);

	/* Update display */
	dpy_all(s);
}


/*
 * scsipt_icon
 *	Handler for main window iconification/de-iconification
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	iconified - Whether the main window is iconified
 *
 * Return:
 *	Nothing.
 */
void
scsipt_icon(curstat_t *s, bool_t iconified)
{
	/* This function attempts to reduce the status polling frequency
	 * when possible to cut down on CPU and SCSI bus usage.  This is
	 * done when the CD player is iconified.
	 */

	/* Increase status polling interval by 4 seconds when iconified */
	if (iconified)
		scsipt_stat_interval = app_data.stat_interval + 4000;
	else
		scsipt_stat_interval = app_data.stat_interval;

	/* Check disc status */
	if (!scsipt_disc_ready(s))
		return;

	switch (s->mode) {
	case M_STOP:
	case M_NODISC:
	case M_PAUSE:
		break;

	case M_A:
	case M_AB:
	case M_SAMPLE:
		/* No optimization in these modes */
		scsipt_stat_interval = app_data.stat_interval;
		break;

	case M_PLAY:
		if (!iconified) {
			/* Force an immediate update */
			scsipt_stop_stat_poll();
			scsipt_start_stat_poll(s);
		}
		break;
	}
}


/*
 * scsipt_halt
 *	Shut down the SCSI pass-through and vendor-unique modules.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
scsipt_halt(curstat_t *s)
{
	int	i;

	/* Re-enable front-panel eject button */
	if (s->caddy_lock)
		scsipt_lock(s, FALSE);

	if (s->mode != M_NODISC) {
		if (app_data.exit_eject && app_data.eject_supp) {
			/* User closing application: Eject disc */
			scsipt_do_start_stop(FALSE, TRUE);
		}
		else {
			if (app_data.exit_stop)
				/* User closing application: Stop disc */
				scsipt_do_start_stop(FALSE, FALSE);

			switch (s->mode) {
			case M_PLAY:
			case M_PAUSE:
			case M_A:
			case M_AB:
			case M_SAMPLE:
				scsipt_stop_stat_poll();
				break;
			}
		}
	}

	/* Shut down the vendor unique modules */
	for (i = 0; i < MAX_VENDORS; i++) {
		if (scsipt_vutbl[i].halt != NULL)
			scsipt_vutbl[i].halt();
	}

	/* Close device */
	pthru_close();
}


/*
 * scsipt_mode
 *	Return a text string indicating the current SCSI mode
 *	("SCSI-2" or a particular vendor-unique string).
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	SCSI mode text string.
 */
char *
scsipt_mode(void)
{
	static char	str[STR_BUF_SZ];

	sprintf(str, "%s%s", scsipt_vutbl[app_data.vendor_code].vendor,
		app_data.vendor_code == VENDOR_SCSI2 ? "" : " vendor unique");

	return (str);
}


/*
 * scsipt_vers
 *	Return a text string indicating the SCSI pass-through module's
 *	version number and which SCSI-1 vendor-unique modes are
 *	supported in this binary.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Version text string.
 */
char *
scsipt_vers(void)
{
	int		i;
	bool_t		vusupp = FALSE;
	static char	vers[256];

	sprintf(vers, "%s\n%s",
		"SCSI-2 Pass-through method",
		"SCSI-1 Vendor-unique support:");

	for (i = 0; i < MAX_VENDORS; i++) {
		if (scsipt_vutbl[i].vendor != NULL && i != VENDOR_SCSI2) {
			sprintf(vers, "%s%s%s",
				vers,
				vusupp ? ", " : "\n   ",
				scsipt_vutbl[i].vendor);
			vusupp = TRUE;
		}
	}

	sprintf(vers, "%s%s\n%s", vers, vusupp ? "" : " none", pthru_vers());

	return (vers);
}

#endif	/* DI_SCSIPT */

