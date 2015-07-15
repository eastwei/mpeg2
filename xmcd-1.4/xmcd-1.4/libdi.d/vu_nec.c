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
 *   The name "NEC" is a trademark of NEC Corporation, and is used here
 *   for identification purposes only.  This software and its author
 *   are not affiliated in any way with NEC.
 *
 */
#ifndef LINT
static char *_vu_nec_c_ident_ = "@(#)vu_nec.c	5.4 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#ifdef VENDOR_NEC

extern appdata_t	app_data;
extern vu_tbl_t		scsipt_vutbl[];

STATIC bool_t		nec_audio_muted = FALSE;	/* Is audio muted? */


/*
 * nec_playaudio
 *	Play audio function: send vendor-unique play audio command
 *	to the drive.
 *
 * Args:
 *	addr_fmt - Flags indicating which address formats are passed in
 *	If ADDR_BLK, then:
 *	    start_addr - The logical block starting address
 *	    end_addr - The logical block ending address
 *	If ADD_MSF, then:
 *	    start_msf - Pointer to the starting MSF address structure
 *	    end_msf - Pointer to the ending MSF address structure
 *	If ADDR_TRKIDX, then:
 *	    trk - The starting track number
 *	    idx - The starting index number
 *	If ADDR_OPTEND, then the ending address, if specified, can be
 *	ignored if possible.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
/*ARGSUSED*/
bool_t
nec_playaudio(
	byte_t		addr_fmt,
	word32_t	start_addr,
	word32_t	end_addr,
	msf_t		*start_msf,
	msf_t		*end_msf,
	byte_t		trk,
	byte_t		idx
)
{
	bool_t		ret = FALSE;
	word32_t	addr = 0;
	naudio_arg_t	*p;

	p = (naudio_arg_t *) &addr;

	if (!ret && addr_fmt & ADDR_MSF) {
		/* Position laser head at desired location
		 * and start play.
		 */
		p->addr_min = (byte_t) ltobcd(start_msf->min);
		p->addr_sec = (byte_t) ltobcd(start_msf->sec);
		p->addr_frame = (byte_t) ltobcd(start_msf->frame);

		ret = pthru_send(
			OP_VN_AUDSRCH,
			addr, NULL, 0, 0, 0,
			0x1, 0x1 << 6, READ_OP, TRUE
		);

		if (ret && !(addr_fmt & ADDR_OPTEND)) {
			/* Specify end location, muting, and start play */
			p->addr_min = (byte_t) ltobcd(end_msf->min);
			p->addr_sec = (byte_t) ltobcd(end_msf->sec);
			p->addr_frame = (byte_t) ltobcd(end_msf->frame);

			ret = pthru_send(
				OP_VN_AUDPLAY,
				addr, NULL, 0, 0, 0,
				(byte_t) (nec_audio_muted ? 0x0 : 0x3),
				0x1 << 6, READ_OP, TRUE
			);
		}
	}

	if (!ret && addr_fmt & ADDR_BLK) {
		/* Position laser head at desired location
		 * and start play.
		 */
		p->addr_logical = start_addr;

		ret = pthru_send(
			OP_VN_AUDSRCH,
			addr, NULL, 0, 0, 0,
			0x1, 0x0, READ_OP, TRUE
		);

		if (ret && !(addr_fmt & ADDR_OPTEND)) {
			/* Specify end location, muting, and start play */
			p->addr_logical = end_addr;

			ret = pthru_send(
				OP_VN_AUDPLAY,
				addr, NULL, 0, 0, 0,
				(byte_t) (nec_audio_muted ? 0x0 : 0x3),
				0x0, READ_OP, TRUE
			);
		}
	}

	return (ret);
}


/*
 * nec_pause_resume
 *	Pause/resume function: send vendor-unique commands to implement
 *	the pause and resume capability.
 *
 * Args:
 *	resume - TRUE: resume, FALSE: pause
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
nec_pause_resume(bool_t resume)
{
	if (resume) {
		return (
			pthru_send(
				OP_VN_AUDPLAY, 0, NULL, 0, 0, 0,
				(byte_t) (nec_audio_muted ? 0x0 : 0x3),
				0x3 << 6, READ_OP, TRUE
			)
		);
	}
	else {
		return (
			pthru_send(
				OP_VN_STILL, 0, NULL, 0, 0, 0,
				0, 0, READ_OP, TRUE
			)
		);
	}
}


/*
 * nec_get_playstatus
 *	Send vendor-unique command to obtain current audio playback
 *	status.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *	audio_status - Address where a current status code (SCSI-2
 *		       style) is to be returned.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
nec_get_playstatus(curstat_t *s, byte_t *audio_status)
{
	int		i,
			trkno,
			idxno;
	byte_t		buf[sizeof(nsubq_data_t)];
	nsubq_data_t	*d;


	memset(buf, 0, sizeof(buf));

	if (!pthru_send(OP_VN_RDSUBQ, 0, buf, SZ_VN_RDSUBQ, 0, 0,
		       SZ_VN_RDSUBQ, 0, READ_OP, TRUE))
		return FALSE;

	DBGDUMP("nec: Read Subchannel data bytes", buf, SZ_VN_RDSUBQ);

	d = (nsubq_data_t *)(void *) buf;

	trkno = bcdtol((word32_t) d->trkno);
	if (s->cur_trk != trkno) {
		s->cur_trk = trkno;
		dpy_track(s);
	}

	idxno = bcdtol((word32_t) d->idxno);
	if (s->cur_idx != idxno) {
		s->cur_idx = idxno;
		s->sav_iaddr = s->cur_tot_addr;
		dpy_index(s);
	}

	if ((i = curtrk_pos(s)) >= 0)
		s->trkinfo[i].type = (d->trktype == 0) ? TYP_AUDIO : TYP_DATA;

	s->cur_tot_min = (byte_t) bcdtol(d->abs_min);
	s->cur_tot_sec = (byte_t) bcdtol(d->abs_sec);
	s->cur_tot_frame = (byte_t) bcdtol(d->abs_frame);
	s->cur_trk_min = (byte_t) bcdtol(d->rel_min);
	s->cur_trk_sec = (byte_t) bcdtol(d->rel_sec);
	s->cur_trk_frame = (byte_t) bcdtol(d->rel_frame);
	msftoblk(
		s->cur_tot_min, s->cur_tot_sec, s->cur_tot_frame,
		&s->cur_tot_addr, MSF_OFFSET(s)
	);
	msftoblk(
		s->cur_trk_min, s->cur_trk_sec, s->cur_trk_frame,
		&s->cur_trk_addr, 0
	);

	/* Translate NEC audio status to SCSI-2 audio status */
	switch (d->audio_status) {
	case NAUD_PLAYING:
		*audio_status = AUDIO_PLAYING;
		break;

	case NAUD_PAUSED:
	case NAUD_SRCH_PAUSED:
		*audio_status = AUDIO_PAUSED;
		break;

	case NAUD_COMPLETED:
		*audio_status = AUDIO_COMPLETED;
		break;
	}

	return TRUE;
}


/*
 * nec_get_toc
 *	Send vendor-unique command to obtain the disc table-of-contents
 *
 * Args:
 *	s - Pointer to the curstat_t structure, which contains the TOC
 *	    table to be updated.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
nec_get_toc(curstat_t *s)
{
	int		i,
			j;
	byte_t		buf[SZ_VN_RDTOC];
	ninfo_00_t	*t0;
	ninfo_01_t	*t1;
	ninfo_02_t	*t2;


	memset(buf, 0, sizeof(buf));

	/* Find number of tracks */
	if (!pthru_send(OP_VN_RDTOC, 0, buf, SZ_VN_RDTOC,
			0, 0, 0, 0, READ_OP, TRUE))
		return FALSE;

	DBGDUMP("nec: Read TOC data bytes", buf, SZ_VN_RDTOC);

	t0 = (ninfo_00_t *) buf;
	s->first_trk = (byte_t) bcdtol(t0->first_trk);
	s->last_trk = (byte_t) bcdtol(t0->last_trk);

	/* Get the starting position of each track */
	for (i = 0, j = (int) s->first_trk; j <= (int) s->last_trk; i++, j++) {
		memset(buf, 0, sizeof(buf));

		if (!pthru_send(OP_VN_RDTOC, ltobcd(j) << 24,
				buf, SZ_VN_RDTOC, 0, 0, 2,
				0, READ_OP, TRUE))
			return FALSE;

		DBGDUMP("nec: Read TOC data bytes", buf, SZ_VN_RDTOC);

		t2 = (ninfo_02_t *) buf;

		s->trkinfo[i].trkno = j;
		s->trkinfo[i].min = (byte_t) bcdtol(t2->min);
		s->trkinfo[i].sec = (byte_t) bcdtol(t2->sec);
		s->trkinfo[i].frame = (byte_t) bcdtol(t2->frame);
		msftoblk(
			s->trkinfo[i].min,
			s->trkinfo[i].sec,
			s->trkinfo[i].frame,
			&s->trkinfo[i].addr,
			MSF_OFFSET(s)
		);
	}
	s->tot_trks = (byte_t) i;

	memset(buf, 0, sizeof(buf));

	/* Get the lead out track position */
	if (!pthru_send(OP_VN_RDTOC, 0,
			buf, SZ_VN_RDTOC, 0, 0, 1,
			0, READ_OP, TRUE))
		return FALSE;

	DBGDUMP("nec: Read TOC data bytes", buf, SZ_VN_RDTOC);

	t1 = (ninfo_01_t *) buf;

	s->trkinfo[i].trkno = LEAD_OUT_TRACK;
	s->tot_min = s->trkinfo[i].min = (byte_t) bcdtol(t1->min);
	s->tot_sec = s->trkinfo[i].sec = (byte_t) bcdtol(t1->sec);
	s->tot_frame = s->trkinfo[i].frame = (byte_t) bcdtol(t1->frame);
	msftoblk(
		s->trkinfo[i].min,
		s->trkinfo[i].sec,
		s->trkinfo[i].frame,
		&s->trkinfo[i].addr,
		MSF_OFFSET(s)
	);
	s->tot_addr = s->trkinfo[i].addr;

	return TRUE;
}


/*
 * nec_mute
 *	Send vendor-unique command to mute/unmute the audio
 *
 * Args:
 *	mute - TRUE: mute audio, FALSE: unmute audio
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
nec_mute(bool_t mute)
{
	curstat_t	*s = curstat_addr();

	if (nec_audio_muted != mute) {
		switch (s->mode) {
		case M_NODISC:
		case M_STOP:
		case M_PAUSE:
			break;

		default:
			if (!pthru_send(OP_VN_AUDPLAY, 0, NULL, 0, 0, 0,
					(byte_t) (mute ? 0x0 : 0x3),
					0x3 << 6, READ_OP, TRUE))
				return FALSE;
			break;
		}

		nec_audio_muted = mute;
	}

	return TRUE;
}


/*
 * nec_eject
 *	Send vendor-unique command to eject the caddy
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
nec_eject(void)
{
	return (pthru_send(OP_VN_EJECT, 0, NULL, 0, 0, 0, 1, 0, READ_OP, TRUE));
}


/*
 * nec_init
 *	Initialize the vendor-unique support module
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
nec_init(void)
{
	/* Register vendor_unique module entry points */
	scsipt_vutbl[VENDOR_NEC].vendor = "NEC";
	scsipt_vutbl[VENDOR_NEC].playaudio = nec_playaudio;
	scsipt_vutbl[VENDOR_NEC].pause_resume = nec_pause_resume;
	scsipt_vutbl[VENDOR_NEC].start_stop = NULL;
	scsipt_vutbl[VENDOR_NEC].get_playstatus = nec_get_playstatus;
	scsipt_vutbl[VENDOR_NEC].volume = NULL;
	scsipt_vutbl[VENDOR_NEC].route = NULL;
	scsipt_vutbl[VENDOR_NEC].mute = nec_mute;
	scsipt_vutbl[VENDOR_NEC].get_toc = nec_get_toc;
	scsipt_vutbl[VENDOR_NEC].eject = nec_eject;
	scsipt_vutbl[VENDOR_NEC].start = NULL;
	scsipt_vutbl[VENDOR_NEC].halt = NULL;
}


#endif	/* VENDOR_NEC */

