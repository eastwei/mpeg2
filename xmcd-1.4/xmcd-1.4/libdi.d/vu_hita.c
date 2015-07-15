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
 *   The name "Hitachi" is a trademark of Hitachi Corporation, and is
 *   used here for identification purposes only.  This software and its
 *   author are not affiliated in any way with Hitachi.
 *
 */
#ifndef LINT
static char *_vu_hita_c_ident_ = "@(#)vu_hita.c	5.4 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"

#ifdef VENDOR_HITACHI

extern appdata_t	app_data;
extern vu_tbl_t		scsipt_vutbl[];

STATIC bool_t		hita_paused = FALSE,		/* Currently paused */
			hita_playing = FALSE,		/* Currently playing */
			hita_audio_muted = FALSE;
							/* Audio is muted */
STATIC word32_t		hita_pause_addr = 0;		/* Pause addr */
STATIC haudio_arg_t	hita_sav_end;			/* Save addr */


/*
 * Internal functions
 */

/*
 * hita_do_pause
 *	Send a vendor-unique Pause command to the drive
 *
 * Args:
 *	ret_addr - Pointer to a buffer where the paused address will be
 *		   written to.  If NULL, no pause address info will
 *		   returned.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
hita_do_pause(hmsf_t *ret_addr)
{
	hmsf_t	pause_addr;
	bool_t	ret;

	if ((ret = pthru_send(OP_VH_PAUSE, 0, (byte_t *) &pause_addr,
			      SZ_VH_PAUSE, 0, 0, 0, 0, READ_OP,
			      TRUE)) == TRUE) {
		DBGDUMP("hita: Pause address", (byte_t *) &pause_addr,
			sizeof(hmsf_t));

		if (ret_addr != NULL)
			*ret_addr = pause_addr;	/* structure copy */
	}
	return (ret);
}


/*
 * Public functions
 */

/*
 * hita_playaudio
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
hita_playaudio(
	byte_t		addr_fmt,
	word32_t	start_addr,
	word32_t	end_addr,
	msf_t		*start_msf,
	msf_t		*end_msf,
	byte_t		trk,
	byte_t		idx
)
{
	msf_t		istart_msf,
			iend_msf;
	word32_t	addr1 = 0,
			addr2 = 0;
	haudio_arg_t	*a1,
			*a2;
	curstat_t	*s = curstat_addr();
	bool_t		ret = FALSE;

	a1 = (haudio_arg_t *) &addr1;
	a2 = (haudio_arg_t *) &addr2;

	if (!ret && (addr_fmt & ADDR_BLK) && !(addr_fmt & ADDR_MSF)) {
		/* Convert block address to MSF format */
		blktomsf(
			start_addr,
			&istart_msf.min, &istart_msf.sec, &istart_msf.frame,
			MSF_OFFSET(s)
		);

		blktomsf(
			end_addr,
			&iend_msf.min, &iend_msf.sec, &iend_msf.frame,
			MSF_OFFSET(s)
		);

		/* Let the ADDR_MSF code handle the request */
		start_msf = &istart_msf;
		end_msf = &iend_msf;
		addr_fmt |= ADDR_MSF;
		ret = FALSE;
	}

	if (!ret && (addr_fmt & ADDR_MSF)) {
		a1->addr_smin = (byte_t) start_msf->min;
		a1->addr_ssec = (byte_t) start_msf->sec;
		a1->addr_sframe = (byte_t) start_msf->frame;

		a2->addr_emin = hita_sav_end.addr_emin = (byte_t) end_msf->min;
		a2->addr_esec = hita_sav_end.addr_esec = (byte_t) end_msf->sec;
		a2->addr_eframe = hita_sav_end.addr_eframe = (byte_t)
			end_msf->frame;

		/* Send a pause command to cease any current audio playback,
		 * then send the actual play audio command.
		 */
		if ((ret = hita_do_pause(NULL)) == TRUE) {
			ret = pthru_send(
				OP_VH_AUDPLAY, addr1, NULL, 0, 0, addr2,
				(byte_t) (hita_audio_muted ? 0x7 : 0x1),
				0, READ_OP, TRUE
			);
		}
	}

	if (ret) {
		hita_paused = FALSE;
		hita_playing = TRUE;
	}

	return (ret);
}


/*
 * hita_pause_resume
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
hita_pause_resume(bool_t resume)
{
	word32_t	addr1 = 0,
			addr2 = 0;
	haudio_arg_t	*a1,
			*a2;
	hmsf_t		*a;
	bool_t		ret = FALSE;


	a1 = (haudio_arg_t *) &addr1;
	a2 = (haudio_arg_t *) &addr2;
	a = (hmsf_t *) &hita_pause_addr;

        if (resume) {
		if (!hita_paused)
			return TRUE;

		a1->addr_smin = a->min;
		a1->addr_ssec = a->sec;
		a1->addr_sframe = a->frame;
		a2->addr_emin = hita_sav_end.addr_emin;
		a2->addr_esec = hita_sav_end.addr_esec;
		a2->addr_eframe = hita_sav_end.addr_eframe;

		ret = pthru_send(
			OP_VH_AUDPLAY, addr1, NULL, 0, 0, addr2,
			(byte_t) (hita_audio_muted ? 0x7 : 0x1),
			0, READ_OP, TRUE
		);
        }
        else {
		if (hita_paused)
			return TRUE;

		ret = hita_do_pause(a);
	}

	if (ret) {
		hita_paused = !resume;
		hita_playing = !hita_paused;
	}

	return (ret);
}


/*
 * hita_start_stop
 *	Start/stop function: When playing audio, the Hitachi drive must
 *	first be paused before sending a Start/Stop Unit command to
 *	stop it.
 *
 * Args:
 *	start - TRUE: start unit, FALSE: stop unit
 *	loej - TRUE: eject caddy, FALSE: do not eject
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
hita_start_stop(bool_t start, bool_t loej)
{
	byte_t	param = 0x00;

	/* If audio playback is in progress, pause the playback.
	 * Then, return to caller (do_start_stop() in scsipt.c)
	 * and issue a start/stop unit command from there.
	 */
	if (!start && hita_playing && !hita_do_pause(NULL))
		return FALSE;

	hita_paused = FALSE;

	if (start)
		param = 0x01;
	else if (loej)
		param = 0x02;

	return (
	    pthru_send(OP_S_START, 0, NULL, 0, 0, param, 0, 0, READ_OP, TRUE)
	);
}


/*
 * hita_get_playstatus
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
hita_get_playstatus(curstat_t *s, byte_t *audio_status)
{
	int		trkno,
			idxno;
	byte_t		buf[SZ_VH_RDSTAT];
	haudstat_t	*d;


	memset(buf, 0, sizeof(buf));

	if (!pthru_send(OP_VH_RDSTAT, 0, buf, SZ_VH_RDSTAT, 0, 0, 0, 0,
			READ_OP, TRUE))
		return FALSE;

	DBGDUMP("hita: Read Status data bytes", buf, SZ_VH_RDSTAT);

	d = (haudstat_t *)(void *) buf;

	trkno = (word32_t) d->trkno;
	if (s->cur_trk != trkno) {
		s->cur_trk = trkno;
		dpy_track(s);
	}
	idxno = 1;	/* Fudge */
	if (s->cur_idx != idxno) {
		s->cur_idx = idxno;
		dpy_index(s);
	}

	s->cur_tot_min = (byte_t) d->abs_addr.min;
	s->cur_tot_sec = (byte_t) d->abs_addr.sec;
	s->cur_tot_frame = (byte_t) d->abs_addr.frame;
	s->cur_trk_min = (byte_t) d->rel_addr.min;
	s->cur_trk_sec = (byte_t) d->rel_addr.sec;
	s->cur_trk_frame = (byte_t) d->rel_addr.frame;
	msftoblk(
		s->cur_tot_min, s->cur_tot_sec, s->cur_tot_frame,
		&s->cur_tot_addr, MSF_OFFSET(s)
	);
	msftoblk(
		s->cur_trk_min, s->cur_trk_sec, s->cur_trk_frame,
		&s->cur_trk_addr, 0
	);

	/* Make up SCSI-2 style audio status */
	if (hita_paused)
		*audio_status = AUDIO_PAUSED;
	else if (d->playing)
		*audio_status = AUDIO_PLAYING;
	else {
		*audio_status = AUDIO_COMPLETED;
		hita_playing = FALSE;
	}

	return TRUE;
}


/*
 * hita_get_toc
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
hita_get_toc(curstat_t *s)
{
	int		i,
			j,
			xfer_len;
	byte_t		buf[SZ_VH_RDXINFO];
	hxdiscinfo_t	*p;
	hxmsf_t		*a;


	if (hita_playing)
		return FALSE;	/* Drive is busy */

	memset(buf, 0, sizeof(buf));

	/* Read the TOC header first */
	if (!pthru_send(OP_VH_RDXINFO, 0, buf, SZ_VH_XTOCHDR,
			SZ_VH_XTOCHDR & 0xff, SZ_VH_XTOCHDR >> 8, 0, 0,
			READ_OP, TRUE))
		return FALSE;

	p = (hxdiscinfo_t *)(void *) buf;

	s->first_trk = (byte_t) p->first_trk;
	s->last_trk = (byte_t) p->last_trk;

	xfer_len = SZ_VH_XTOCHDR +
		   ((int) (p->last_trk - p->first_trk + 2) * SZ_VH_XTOCENT);

	if (xfer_len > SZ_VH_RDXINFO)
		xfer_len = SZ_VH_RDXINFO;

	/* Read the appropriate number of bytes of the entire TOC */
	if (!pthru_send(OP_VH_RDXINFO, 0, buf, xfer_len,
			(byte_t) (xfer_len & 0xff), xfer_len >> 8, 0, 0,
			READ_OP, TRUE))
		return FALSE;

	DBGDUMP("hita: Read Extended Disc Info data bytes", buf, xfer_len);

	/* Get the starting position of each track */
	for (i = 0, j = (int) s->first_trk; j <= (int) s->last_trk; i++, j++) {
		a = (hxmsf_t *)(void *) &p->xmsfdata[(i+1) * SZ_VH_XTOCENT];
		s->trkinfo[i].trkno = j;
		s->trkinfo[i].min = (byte_t) a->min;
		s->trkinfo[i].sec = (byte_t) a->sec;
		s->trkinfo[i].frame = (byte_t) a->frame;
		msftoblk(
			s->trkinfo[i].min,
			s->trkinfo[i].sec,
			s->trkinfo[i].frame,
			&s->trkinfo[i].addr,
			MSF_OFFSET(s)
		);
		s->trkinfo[i].type = (a->trktype == 0) ? TYP_AUDIO : TYP_DATA;
	}
	s->tot_trks = (byte_t) i;

	/* Get the lead-out track position */
	a = (hxmsf_t *)(void *) &p->xmsfdata[0];
	s->trkinfo[i].trkno = LEAD_OUT_TRACK;
	s->tot_min = s->trkinfo[i].min = (byte_t) a->min;
	s->tot_sec = s->trkinfo[i].sec = (byte_t) a->sec;
	s->tot_frame = s->trkinfo[i].frame = (byte_t) a->frame;
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
 * hita_mute
 *	Send vendor-unique command to mute/unmute the audio
 *
 * Args:
 *	mute - TRUE: mute audio, FALSE: un-mute audio
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
hita_mute(bool_t mute)
{
	word32_t	addr1 = 0,
			addr2 = 0;
	haudio_arg_t	*a1,
			*a2;
	curstat_t	*s = curstat_addr();


	if (mute == hita_audio_muted)
		return TRUE;

	a1 = (haudio_arg_t *) &addr1;
	a2 = (haudio_arg_t *) &addr2;

	if (hita_playing) {
		/* Pause the playback first */
        	if (!hita_do_pause(NULL))
			return FALSE;

		a1->addr_smin = (byte_t) s->cur_tot_min;
		a1->addr_ssec = (byte_t) s->cur_tot_sec;
		a1->addr_sframe = (byte_t) s->cur_tot_frame;
		a2->addr_emin = hita_sav_end.addr_emin;
		a2->addr_esec = hita_sav_end.addr_esec;
		a2->addr_eframe = hita_sav_end.addr_eframe;

		if (!pthru_send(OP_VH_AUDPLAY, addr1, NULL, 0, 0, addr2,
				(byte_t) (mute ? 0x7 : 0x1), 0, READ_OP, TRUE))
			return FALSE;
	}

	hita_audio_muted = mute;

	return TRUE;
}


/*
 * hita_eject
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
hita_eject(void)
{
	/* If audio playback is in progress, pause the playback first */
	if (hita_playing && !hita_do_pause(NULL))
		return FALSE;

	hita_playing = hita_paused = FALSE;

	/* Eject the caddy */
	return (
	    pthru_send(OP_VH_EJECT, 0, NULL, 0, 0x1, 0, 0, 0, READ_OP, TRUE)
	);
}


/*
 * hita_init
 *	Initialize the vendor-unique support module
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
hita_init(void)
{
	/* Register vendor_unique module entry points */
	scsipt_vutbl[VENDOR_HITACHI].vendor = "Hitachi";
	scsipt_vutbl[VENDOR_HITACHI].playaudio = hita_playaudio;
	scsipt_vutbl[VENDOR_HITACHI].pause_resume = hita_pause_resume;
	scsipt_vutbl[VENDOR_HITACHI].start_stop = hita_start_stop;
	scsipt_vutbl[VENDOR_HITACHI].get_playstatus = hita_get_playstatus;
	scsipt_vutbl[VENDOR_HITACHI].volume = NULL;
	scsipt_vutbl[VENDOR_HITACHI].route = NULL;
	scsipt_vutbl[VENDOR_HITACHI].mute = hita_mute;
	scsipt_vutbl[VENDOR_HITACHI].get_toc = hita_get_toc;
	scsipt_vutbl[VENDOR_HITACHI].eject = hita_eject;
	scsipt_vutbl[VENDOR_HITACHI].start = NULL;
	scsipt_vutbl[VENDOR_HITACHI].halt = NULL;
}


#endif	/* VENDOR_HITACHI */

