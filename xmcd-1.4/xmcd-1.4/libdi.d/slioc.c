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
 * SunOS/Linux ioctl method module
 *
 * Contributing author: Peter Bauer
 * E-mail: 100136.3530@compuserve.com
 */

#ifndef LINT
static char *_slioc_c_ident_ = "@(#)slioc.c	5.32.1 95/03/04";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/slioc.h"

#define CLIP_FRAMES	10

#if defined(DI_SLIOC) && !defined(DEMO_ONLY)

extern appdata_t	app_data;
extern FILE		*errfp;


STATIC bool_t	slioc_run_ab(curstat_t *),
		slioc_run_sample(curstat_t *),
		slioc_run_prog(curstat_t *),
		slioc_run_repeat(curstat_t *),
		slioc_disc_ready(curstat_t *),
		slioc_disc_present(bool_t),
		slioc_open(char *);
STATIC void	slioc_stat_poll(curstat_t *),
		slioc_insert_poll(curstat_t *),
		slioc_close(void);


STATIC int	slioc_fd = -1,			/* CD-ROM file descriptor */
		slioc_stat_interval;		/* Status poll interval */
STATIC long	slioc_stat_id,			/* Play status poll timer id */
		slioc_insert_id,		/* Disc insert poll timer id */
		slioc_search_id;		/* FF/REW timer id */
STATIC byte_t	slioc_tst_status = M_NODISC,	/* Playback status on load */
		slioc_next_sam;			/* Next SAMPLE track */
STATIC bool_t	slioc_not_open = TRUE,		/* Device not opened yet */
		slioc_stat_polling,		/* Polling play status */
		slioc_insert_polling,		/* Polling disc insert */
		slioc_new_progshuf,		/* New program/shuffle seq */
		slioc_start_search = FALSE,	/* Start FF/REW play segment */
		slioc_idx_pause = FALSE,	/* Prev/next index pausing */
		slioc_fake_stop = FALSE,	/* Force a completion status */
		slioc_playing = FALSE;		/* Currently playing */
STATIC word32_t	slioc_ab_start_addr,		/* A->B mode start block */
		slioc_ab_end_addr,		/* A->B mode end block */
		slioc_sav_end_addr;		/* Err recov saved end addr */
STATIC msf_t	slioc_ab_start_msf,		/* A->B mode start MSF */
		slioc_ab_end_msf,		/* A->B mode end MSF */
		slioc_sav_end_msf;		/* Err recov saved end MSF */
STATIC byte_t	slioc_sav_end_fmt;		/* Err recov saved end fmt */


/* SunOS/Linux CDROM ioctl names */
STATIC iocname_t iname[] = {
	{ CDROMSUBCHNL,		"CDROMSUBCHNL"		},
	{ CDROMREADTOCHDR,	"CDROMREADTOCHDR"	},
	{ CDROMREADTOCENTRY,	"CDROMREADTOCENTRY"	},
#ifdef CDROMLOAD
	{ CDROMLOAD,		"CDROMLOAD"		},
#endif
	{ CDROMEJECT,		"CDROMEJECT"		},
	{ CDROMSTART,		"CDROMSTART"		},
	{ CDROMSTOP,		"CDROMSTOP"		},
	{ CDROMPAUSE,		"CDROMPAUSE"		},
	{ CDROMRESUME,		"CDROMRESUME"		},
	{ CDROMVOLCTRL,		"CDROMVOLCTRL"		},
	{ CDROMPLAYTRKIND,	"CDROMPLAYTRKIND"	},
	{ CDROMPLAYMSF,		"CDROMPLAYMSF"		},
	{ 0,			NULL			},
};



/***********************
 *  internal routines  *
 ***********************/


/*
 * slioc_send
 *	Issue ioctl command.
 *
 * Args:
 *	cmd - ioctl command
 *	arg - ioctl argument
 *	prnerr - whether an error message is to be displayed if the ioctl fails
 *
 * Return:
 *	TRUE - ioctl successful
 *	FALSE - ioctl failed
 */
STATIC bool_t
slioc_send(int cmd, void *arg, bool_t prnerr)
{
	int	i;

	if (slioc_fd < 0)
		return FALSE;

	if (app_data.debug) {
		for (i = 0; iname[i].name != NULL; i++) {
			if (iname[i].cmd == cmd) {
				fprintf(errfp, "\nIOCTL: %s\n",
					iname[i].name);
				break;
			}
		}
		if (iname[i].name == NULL)
			fprintf(errfp, "\nIOCTL: 0x%x\n", cmd);
	}

	if (ioctl(slioc_fd, cmd, arg) < 0) {
		if (prnerr) {
			fprintf(errfp, "CD audio: ioctl error on %s: ",
				app_data.device);

			for (i = 0; iname[i].name != NULL; i++) {
				if (iname[i].cmd == cmd) {
					fprintf(errfp, "cmd=%s errno=%d\n",
						iname[i].name, errno);
					break;
				}
			}
			if (iname[i].name == NULL)
				fprintf(errfp, "cmd=0x%x errno=%d\n",
					cmd, errno);
		}
		return FALSE;
	}

        return TRUE;
}


/*
 * slioc_open
 *	Open CD-ROM device
 *
 * Args:
 *	path - device path name string
 *
 * Return:
 *	TRUE - open successful
 *	FALSE - open failed
 */
STATIC bool_t
slioc_open(char *path)
{
	struct stat	stbuf;
	char		errstr[ERR_BUF_SZ];
	int		i,
			ret;

	/* Force close on eject. */
	app_data.eject_close = TRUE;

	/* Check for validity of device node */
	if (stat(path, &stbuf) < 0) {
		sprintf(errstr, app_data.str_staterr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}

#ifdef linux
	/* Linux CD-ROM device is a block special file! */
	if (!S_ISBLK(stbuf.st_mode))
#else
	if (!S_ISCHR(stbuf.st_mode))
#endif
	{
		sprintf(errstr, app_data.str_noderr, path);
		cd_fatal_popup(app_data.str_fatal, errstr);
		return FALSE;
	}

	if ((slioc_fd = open(path, O_RDONLY | O_EXCL)) < 0) {
		DBGPRN(errfp, "Cannot open %s: errno=%d\n", path, errno);
		return FALSE;
	}

#ifdef linux
	/* Linux hack:  The CD-ROM driver allows the open to succeed
	 * even if there is no CD loaded.  We test for the existence of
	 * a disc with slioc_disc_present().
	 */
	for (i = 0; i < 3; i++) {
		if ((ret = slioc_disc_present(FALSE)) == TRUE)
			break;
	}
	if (ret == FALSE) {
		slioc_close();
		return FALSE;
	}
#endif

	return TRUE;
}


/*
 * slioc_close
 *	Close CD-ROM device
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
slioc_close(void)
{
	if (slioc_fd >= 0) {
		close(slioc_fd);
		slioc_fd = -1;
	}
}


/*
 * slioc_rdsubq
 *	Send Read Subchannel command to the device
 *
 * Args:
 *	buf - Pointer to the return data buffer
 *	msf - Whether to use MSF or logical block address format
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
slioc_rdsubq(struct cdrom_subchnl *sub, byte_t msf)
{
	bool_t	ret;

        sub->cdsc_format = msf;
	ret = slioc_send(CDROMSUBCHNL, sub, TRUE);

	DBGDUMP("cdrom_subchnl data bytes", (byte_t *) sub,
		sizeof(struct cdrom_subchnl));

	return (ret);
}


/*
 * slioc_rdtoc
 *	Send Read TOC command to the device
 *
 * Args:
 *	buf - Pointer to the return data toc header
 *	h - address of pointer to array of toc entrys will be allocated
 *	    by this routine
 *	start - Starting track number for which the TOC data is returned
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
slioc_rdtoc(struct cdrom_tochdr *h, struct cdrom_tocentry **e, int start)
{
	int	i,
		j;

	/* Read the TOC header first */
	if (!slioc_send(CDROMREADTOCHDR, h, TRUE))
		return FALSE;

	DBGDUMP("cdrom_tochdr data bytes", (byte_t *) h,
		sizeof(struct cdrom_tochdr));

	if (start == 0)
		start = h->cdth_trk0;

	if (start > (int) h->cdth_trk1)
		return FALSE;

	h->cdth_trk1++;

	*e = (struct cdrom_tocentry *) malloc(
		(h->cdth_trk1 - start + 1) * sizeof(struct cdrom_tocentry)
	);

	if (*e == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return FALSE;
	}

	for (i = start; i <= (int) h->cdth_trk1; i++) {
		j = i - start;

		(*e)[j].cdte_track =
			(i < (int) h->cdth_trk1) ? i : CDROM_LEADOUT;

		(*e)[j].cdte_format = CDROM_MSF;

		if (!slioc_send(CDROMREADTOCENTRY, &(*e)[j], TRUE)) {
			free(*e);
			return FALSE;
		}

		DBGDUMP("cdrom_tocentry data bytes", (byte_t *) &(*e)[j],
			sizeof(struct cdrom_tocentry));

		/* Sanity check */
		if ((*e)[j].cdte_track == CDROM_LEADOUT &&
		    (*e)[j].cdte_addr.lba == (*e)[0].cdte_addr.lba) {
			free(*e);
			return FALSE;
		}
	}

	return TRUE;
}


/*
 * slioc_disc_present
 *	Check if a CD is loaded.  Much of the complication in this
 *	routine stems from the fact that the SunOS and the Linux
 *	CD-ROM drivers behave differently when a CD is ejected.
 *	In fact, the scsi, mcd, sbpcd, cdu31a drivers under Linux
 *	are also inconsistent amongst each other (Argh!).
 *
 * Args:
 *	savstat - Whether to save start-up status in slioc_tst_status.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure (drive not ready)
 */
STATIC bool_t
slioc_disc_present(bool_t savstat)
{
	struct cdrom_subchnl	sub;
	struct cdrom_tochdr	h;
	struct cdrom_tocentry	e;
	word32_t		a1,
				a2;
	static int		tot_trks = 0;
	static word32_t		sav_a1 = 0,
				sav_a2 = 0;

	if (savstat)
		slioc_tst_status = M_NODISC;

	/* Fake it with CDROMSUBCHNL */
	memset((byte_t *) &sub, 0, sizeof(sub));
	sub.cdsc_format = CDROM_MSF;
	if (!slioc_send(CDROMSUBCHNL, &sub, app_data.debug))
		return FALSE;

	switch (sub.cdsc_audiostatus) {
	case CDROM_AUDIO_PLAY:
		if (savstat) {
			DBGPRN(errfp, "\nstatus=CDROM_AUDIO_PLAY\n");
			slioc_tst_status = M_PLAY;
			return TRUE;
		}
		break;
	case CDROM_AUDIO_PAUSED:
		if (savstat) {
			DBGPRN(errfp, "\nstatus=CDROM_AUDIO_PAUSED\n");
			slioc_tst_status = M_PAUSE;
			return TRUE;
		}
		break;
	case CDROM_AUDIO_ERROR:
		DBGPRN(errfp, "\nstatus=CDROM_AUDIO_ERROR\n");
		break;
	case CDROM_AUDIO_COMPLETED:
		DBGPRN(errfp, "\nstatus=CDROM_AUDIO_COMPLETED\n");
		break;
	case CDROM_AUDIO_NO_STATUS:
		DBGPRN(errfp, "\nstatus=CDROM_AUDIO_NO_STATUS\n");
		break;
	case CDROM_AUDIO_INVALID:
		DBGPRN(errfp, "\nstatus=CDROM_AUDIO_INVALID\n");
		break;
	default:
		DBGPRN(errfp, "\nstatus=unknown (%d)\n", sub.cdsc_audiostatus);
		return FALSE;
	}

	if (savstat)
		slioc_tst_status = M_STOP;

	/* CDROMSUBCHNL didn't give useful info.
	 * Try CDROMREADTOCHDR and CDROMREADTOCENTRY.
	 */
	memset((byte_t *) &h, 0, sizeof(h));
	if (!slioc_send(CDROMREADTOCHDR, &h, app_data.debug))
		return FALSE;

	if ((h.cdth_trk1 - h.cdth_trk0 + 1) != tot_trks) {
		/* Disk changed */
		tot_trks = h.cdth_trk1 - h.cdth_trk0 + 1;
		return FALSE;
	}

	memset((byte_t *) &e, 0, sizeof(e));
	e.cdte_format = CDROM_MSF;
	e.cdte_track = h.cdth_trk0;
	if (!slioc_send(CDROMREADTOCENTRY, &e, app_data.debug))
		return FALSE;

	a1 = (word32_t) e.cdte_addr.lba;

	memset((byte_t *) &e, 0, sizeof(e));
	e.cdte_format = CDROM_MSF;
	e.cdte_track = h.cdth_trk1;
	if (!slioc_send(CDROMREADTOCENTRY, &e, app_data.debug))
		return FALSE;

	a2 = (word32_t) e.cdte_addr.lba;

	DBGPRN(errfp, "\na1=0x%x a2=0x%x\n", a1, a2);

	if (a1 != sav_a1 || a2 != sav_a2) {
		/* Disk changed */
		sav_a1 = a1;
		sav_a2 = a2;
		return FALSE;
	}

	if (tot_trks > 1 && a1 == a2)
		return FALSE;

	return TRUE;
}


/*
 * slioc_playmsf
 *	Send Play Audio MSF command to the device
 *
 * Args:
 *	start - Pointer to the starting position MSF data
 *	end - Pointer to the ending position MSF data
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
slioc_playmsf(msf_t *start, msf_t *end)
{
	struct cdrom_msf	m;

	m.cdmsf_min0 = start->min;
	m.cdmsf_sec0 = start->sec;
	m.cdmsf_frame0 = start->frame;
	m.cdmsf_min1 = end->min;
	m.cdmsf_sec1 = end->sec;
	m.cdmsf_frame1 = end->frame;

	DBGDUMP("cdrom_msf data bytes", (byte_t *) &m,
		sizeof(struct cdrom_msf));

	return (slioc_send(CDROMPLAYMSF, &m, TRUE));
}


/*
 * slioc_start_stop
 *	Send Start/Stop Unit command to the device
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
slioc_start_stop(bool_t start, bool_t loej)
{
        bool_t	ret;

	if (start) {
		if (loej)
#ifdef CDROMLOAD
			ret = slioc_send(CDROMLOAD, NULL, TRUE);
#else
			ret = FALSE;
#endif
		else
			ret = slioc_send(CDROMSTART, NULL, TRUE);
	}
	else {
		slioc_playing = FALSE;

		if (loej)
			ret = slioc_send(CDROMEJECT, NULL, TRUE);
		else
			ret = slioc_send(CDROMSTOP, NULL, TRUE);
	}

	return (ret);

}


/*
 * slioc_pause_resume
 *	Send Pause/Resume command to the device
 *
 * Args:
 *	resume - Whether to resume or pause
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
slioc_pause_resume(bool_t resume)
{
	return (slioc_send(resume ? CDROMRESUME : CDROMPAUSE, NULL, TRUE));
}


/*
 * slioc_play_trkidx
 *	Send Play Audio Track/Index command to the device
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
slioc_play_trkidx(int start_trk, int start_idx, int end_trk, int end_idx)
{
	struct cdrom_ti	t;

	t.cdti_trk0 = start_trk;
	t.cdti_ind0 = start_idx;
	t.cdti_trk1 = end_trk;
	t.cdti_ind1 = end_idx;

	DBGDUMP("cdrom_ti data bytes", (byte_t *) &t, sizeof(struct cdrom_ti));

	return (slioc_send(CDROMPLAYTRKIND, &t, TRUE));
}


/*
 * slioc_do_playaudio
 *	General top-level play audio function
 *
 * Args:
 *	addr_fmt - The address formats specified:
 *		ADDR_BLK: logical block address (not supported)
 *		ADDR_MSF: MSF address
 *		ADDR_TRKIDX: Track/index numbers
 *		ADDR_OPTEND: Ending address can be ignored
 *	start_addr - Starting logical block address (not supported)
 *	end_addr - Ending logical block address (not supported)
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
slioc_do_playaudio(
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
	 * the last frame minus a few frames.
	 */
	if (addr_fmt & ADDR_MSF && end_msf != NULL) {
		emsf = *end_msf;	/* Structure copy */
		emsfp = &emsf;

		if (emsfp->frame >= CLIP_FRAMES)
			emsfp->frame -= CLIP_FRAMES;
		else {
			if (emsfp->sec > 0)
				emsfp->sec--;
			else {
				emsfp->sec = 59;
				if (emsfp->min > 0)
					emsfp->min--;
			}
			emsfp->frame = FRAME_PER_SEC -
				(CLIP_FRAMES - emsfp->frame);
		}

		emsfp->res = start_msf->res = 0;

		/* Save end address for error recovery */
		slioc_sav_end_msf = *end_msf;
	}
	if (addr_fmt & ADDR_BLK) {
		if (end_addr >= CLIP_FRAMES)
			end_addr -= CLIP_FRAMES;
		else if (end_addr > 0)
			end_addr = 0;

		/* Save end address for error recovery */
		slioc_sav_end_addr = end_addr;
	}

	/* Save end address format for error recovery */
	slioc_sav_end_fmt = addr_fmt;

	if (slioc_playing)
		/* Pause playback first */
		(void) slioc_send(CDROMPAUSE, NULL, TRUE);
	else
		/* Spin up CD */
		(void) slioc_send(CDROMSTART, NULL, TRUE);

	slioc_playing = TRUE;

	if ((addr_fmt & ADDR_MSF) && app_data.playmsf_supp)
		ret = slioc_playmsf(start_msf, emsfp);
	
	if (!ret && (addr_fmt & ADDR_TRKIDX) && app_data.playti_supp)
		ret = slioc_play_trkidx(trk, idx, trk, idx);

	return (ret);
}


/*
 * slioc_get_playstatus
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
slioc_get_playstatus(curstat_t *s)
{
	struct cdrom_subchnl	sub;
	msf_t			recov_start_msf;
	word32_t		recov_start_addr;
	byte_t			audio_status;
	bool_t			done;
	static int		errcnt = 0;
	static word32_t		errblk = 0;
	static bool_t		in_slioc_get_playstatus = FALSE;


	/* Lock this routine from multiple entry */
	if (in_slioc_get_playstatus)
		return TRUE;

	in_slioc_get_playstatus = TRUE;

	memset((byte_t *) &sub, 0, sizeof(sub));

	if (!slioc_rdsubq(&sub, CDROM_MSF)) {
		/* Check to see if the disc had been manually ejected */
		if (!slioc_disc_ready(s)) {
			slioc_sav_end_addr = 0;
			slioc_sav_end_msf.min = 0;
			slioc_sav_end_msf.sec = 0;
			slioc_sav_end_msf.frame = 0;
			slioc_sav_end_fmt = 0;
			errcnt = 0;
			errblk = 0;

			in_slioc_get_playstatus = FALSE;
			return FALSE;
		}

		/* The read subchannel command failed for some
		 * unknown reason.  Just return success and
		 * hope the next poll succeeds.  We don't want
		 * to return FALSE here because that would stop
		 * the poll.
		 */
		in_slioc_get_playstatus = FALSE;
		return TRUE;
	}

	/* Check the subchannel data */
	audio_status = sub.cdsc_audiostatus;

	if (sub.cdsc_trk != s->cur_trk) {
		s->cur_trk = sub.cdsc_trk;
		dpy_track(s);
	}

	if (sub.cdsc_ind != s->cur_idx) {
		s->cur_idx = sub.cdsc_ind;
		s->sav_iaddr = s->cur_tot_addr;
		dpy_index(s);
	}

	s->cur_tot_frame = sub.cdsc_absaddr.msf.frame;
	s->cur_tot_sec = sub.cdsc_absaddr.msf.second;
	s->cur_tot_min = sub.cdsc_absaddr.msf.minute;
	msftoblk(
		s->cur_tot_min,
		s->cur_tot_sec,
		s->cur_tot_frame,
		&s->cur_tot_addr,
		MSF_OFFSET(s)
	);

	s->cur_trk_frame = sub.cdsc_reladdr.msf.frame;
	s->cur_trk_sec = sub.cdsc_reladdr.msf.second;
	s->cur_trk_min = sub.cdsc_reladdr.msf.minute;
	msftoblk(
		s->cur_trk_min,
		s->cur_trk_sec,
		s->cur_trk_frame,
		&s->cur_trk_addr,
		0
	);

	/* Update time display */
	dpy_time(s, FALSE);

	/* Hack: to work around the fact that some CD-ROM drives
	 * return CDROM_AUDIO_PAUSED status after issuing a Stop Unit command.
	 * Just treat the status as completed if we get a paused status
	 * and we don't expect the drive to be paused.
	 */
	if (audio_status == CDROM_AUDIO_PAUSED && s->mode != M_PAUSE &&
	    !slioc_idx_pause)
		audio_status = CDROM_AUDIO_COMPLETED;

	/* Force completion status */
	if (slioc_fake_stop)
		audio_status = CDROM_AUDIO_COMPLETED;

	/* Deal with playback status */
	switch (audio_status) {
	case CDROM_AUDIO_PLAY:
	case CDROM_AUDIO_PAUSED:
		done = FALSE;

		/* If we haven't encountered an error for a while, then
		 * clear the error count.
		 */
		if (errcnt > 0 && (s->cur_tot_addr - errblk) > ERR_CLRTHRESH)
			errcnt = 0;
		break;

	case CDROM_AUDIO_ERROR:
		/* Check to see if the disc had been manually ejected */
		if (!slioc_disc_ready(s)) {
			slioc_sav_end_addr = 0;
			slioc_sav_end_msf.min = 0;
			slioc_sav_end_msf.sec = 0;
			slioc_sav_end_msf.frame = 0;
			slioc_sav_end_fmt = 0;
			errcnt = 0;
			errblk = 0;

			in_slioc_get_playstatus = FALSE;
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

		if (!done && (slioc_sav_end_fmt & ADDR_MSF)) {
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
			if (recov_start_msf.min > slioc_sav_end_msf.min)
				done = TRUE;
			else if (recov_start_msf.min ==
				 slioc_sav_end_msf.min) {
				if (recov_start_msf.sec >
				    slioc_sav_end_msf.sec)
					done = TRUE;
				else if ((recov_start_msf.sec ==
					  slioc_sav_end_msf.sec) &&
					 (recov_start_msf.frame >
					  slioc_sav_end_msf.frame)) {
					done = TRUE;
				}
			}
		}
		else {
			recov_start_msf.min = 0;
			recov_start_msf.sec = 0;
			recov_start_msf.frame = 0;
		}

		if (!done && (slioc_sav_end_fmt & ADDR_BLK)) {
			recov_start_addr = s->cur_tot_addr + ERR_SKIPBLKS;

			/* Check to see if we have skipped past
			 * the end.
			 */
			if (recov_start_addr >= slioc_sav_end_addr)
				done = TRUE;
		}
		else
			recov_start_addr = 0;


		/* Restart playback */
		if (!done) {
			fprintf(errfp, "CD audio: %s\n",
				app_data.str_recoverr);

			slioc_do_playaudio(
				slioc_sav_end_fmt,
				recov_start_addr, slioc_sav_end_addr,
				&recov_start_msf, &slioc_sav_end_msf,
				0, 0
			);

			in_slioc_get_playstatus = FALSE;
			return TRUE;
		}

		/*FALLTHROUGH*/
	case CDROM_AUDIO_COMPLETED:
	case CDROM_AUDIO_NO_STATUS:
	case CDROM_AUDIO_INVALID:
		done = TRUE;

		if (!slioc_fake_stop)
			slioc_playing = FALSE;

		slioc_fake_stop = FALSE;

		switch (s->mode) {
		case M_SAMPLE:
			done = !slioc_run_sample(s);
			break;

		case M_AB:
			done = !slioc_run_ab(s);
			break;

		case M_PLAY:
		case M_PAUSE:
			s->cur_trk_addr = 0;
			s->cur_trk_min = s->cur_trk_sec = s->cur_trk_frame = 0;

			if (s->shuffle || s->program)
				done = !slioc_run_prog(s);

			if (s->repeat)
				done = !slioc_run_repeat(s);

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
		slioc_sav_end_addr = 0;
		slioc_sav_end_msf.min = slioc_sav_end_msf.sec =
			slioc_sav_end_msf.frame = 0;
		slioc_sav_end_fmt = 0;
		errcnt = 0;
		errblk = 0;
		dpy_all(s);

		if (app_data.done_eject) {
			/* Eject the disc */
			slioc_load_eject(s);
		}
		else {
			/* Spin down the disc */
			slioc_start_stop(FALSE, FALSE);
		}

		in_slioc_get_playstatus = FALSE;
		return FALSE;
	}

	in_slioc_get_playstatus = FALSE;
	return TRUE;
}


/*
 * slioc_cfg_vol
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
slioc_cfg_vol(int vol, curstat_t *s, bool_t query, byte_t warp)
{
	struct cdrom_volctrl	volctrl;
	static bool_t		first = TRUE;

	memset((byte_t *) &volctrl, 0, sizeof(volctrl));

	if (query) {
		if (first) {
			first = FALSE;

			/* SunOS/Linux doesn't give a way to read volume
			 * setting via CDROM ioctl.
			 * Force the setting to maximum.
			 */
			vol = 100;
			s->level_left = s->level_right = 100;

			if (warp & WARP_VOL)
				set_vol_slider(vol);

			if (warp & WARP_BAL)
				set_bal_slider(0);

			(void) slioc_cfg_vol(vol, s, FALSE, FALSE);
		}
		return (vol);
	}
	else {
                volctrl.channel0 = scale_vol(
			taper_vol(vol * (int) s->level_left / 100)
		);
                volctrl.channel1 = scale_vol(
			taper_vol(vol * (int) s->level_right / 100)
		);

		DBGDUMP("cdrom_volctrl data bytes", (byte_t *) &volctrl,
			sizeof(struct cdrom_volctrl));

                if (slioc_send(CDROMVOLCTRL, &volctrl, TRUE))
                        return (vol);
		else if (volctrl.channel0 != volctrl.channel1) {
			/* Set the balance to the center
			 * and retry.
			 */
			volctrl.channel0 = volctrl.channel1 =
				scale_vol(taper_vol(vol));

			DBGDUMP("cdrom_volctrl data bytes",
				(byte_t *) &volctrl,
				sizeof(struct cdrom_volctrl));

			if (slioc_send(CDROMVOLCTRL, &volctrl, TRUE)) {
				/* Success: Warp balance control */
				s->level_left = s->level_right = 100;
				set_bal_slider(0);

				return (vol);
			}

			/* Still failed: just drop through */
		}
	}

	return -1;
}


/*
 * slioc_vendor_model
 *	Query and update CD-ROM vendor/model/revision information
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_vendor_model(curstat_t *s)
{
	/*
	 * There is currently no way to get this info,
	 * so just fill in some default info.
	 */
	strcpy(s->vendor, "standard");
	strcpy(s->prod, "CD-ROM drive    ");
	strcpy(s->revnum, " -- ");
}


/*
 * slioc_get_toc
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
slioc_get_toc(curstat_t *s)
{
	int			i;
	bool_t			ret = FALSE;
	struct cdrom_tochdr	h;
	struct cdrom_tocentry	*e,
				*p;

	if (!slioc_rdtoc(&h, &e, 0))
		return FALSE;

	/* Fill curstat structure with TOC data */
	s->first_trk = h.cdth_trk0;
	s->last_trk = h.cdth_trk1;

	p = e;

	for (i = 0; i < (int) (h.cdth_trk1 - h.cdth_trk0 + 1); i++) {
		s->trkinfo[i].trkno = p->cdte_track;
		s->trkinfo[i].type = TYP_AUDIO;

		s->trkinfo[i].min = p->cdte_addr.msf.minute;
		s->trkinfo[i].sec = p->cdte_addr.msf.second;
		s->trkinfo[i].frame = p->cdte_addr.msf.frame;
		msftoblk(
			s->trkinfo[i].min,
			s->trkinfo[i].sec,
			s->trkinfo[i].frame,
			&s->trkinfo[i].addr,
			MSF_OFFSET(s)
		);

		if (p->cdte_track == CDROM_LEADOUT ||
		    s->trkinfo[i].trkno == s->last_trk ||
		    i == (MAXTRACK - 1)) {
			s->tot_min = s->trkinfo[i].min;
			s->tot_sec = s->trkinfo[i].sec;
			s->tot_frame = s->trkinfo[i].frame;
			s->tot_trks = i;
			s->tot_addr = s->trkinfo[i].addr;

			break;
		}

		p++;
	}

        free(e);
	return TRUE;
}


/*
 * slioc_start_stat_poll
 *	Start polling the drive for current playback status
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_start_stat_poll(curstat_t *s)
{
	slioc_stat_polling = TRUE;

	/* Start poll timer */
	slioc_stat_id = cd_timeout(
		slioc_stat_interval,
		slioc_stat_poll,
		(byte_t *) s
	);
}


/*
 * slioc_stop_stat_poll
 *	Stop polling the drive for current playback status
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_stop_stat_poll(void)
{
	if (slioc_stat_polling) {
		/* Stop poll timer */
		cd_untimeout(slioc_stat_id);

		slioc_stat_polling = FALSE;
	}
}


/*
 * slioc_start_insert_poll
 *	Start polling the drive for disc insertion
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_start_insert_poll(curstat_t *s)
{
	if (slioc_insert_polling || s->mode != M_NODISC)
		return;

	slioc_insert_polling = TRUE;

	/* Start poll timer */
	slioc_insert_id = cd_timeout(
		app_data.ins_interval,
		slioc_insert_poll,
		(byte_t *) s
	);
}


/*
 * slioc_stop_insert_poll
 *	Stop polling the drive for disc insertion
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_stop_insert_poll(void)
{
	if (slioc_insert_polling) {
		/* Stop poll timer */
		cd_untimeout(slioc_insert_id);

		slioc_insert_polling = FALSE;
	}
}


/*
 * slioc_stat_poll
 *	The playback status polling function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_stat_poll(curstat_t *s)
{
	if (!slioc_stat_polling)
		return;

	/* Get current audio playback status */
	if (slioc_get_playstatus(s)) {
		/* Register next poll interval */
		slioc_stat_id = cd_timeout(
			slioc_stat_interval,
			slioc_stat_poll,
			(byte_t *) s
		);
	}
	else
		slioc_stat_polling = FALSE;
}


/*
 * slioc_insert_poll
 *	The disc insertion polling function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_insert_poll(curstat_t *s)
{
	/* Check to see if a disc is inserted */
	if (!slioc_disc_ready(s)) {
		/* Register next poll interval */
		slioc_insert_id = cd_timeout(
			app_data.ins_interval,
			slioc_insert_poll,
			(byte_t *) s
		);
	}
	else
		slioc_insert_polling = FALSE;
}


/*
 * slioc_disc_ready
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
slioc_disc_ready(curstat_t *s)
{
	int		i,
			vol;
	bool_t		err,
			first_open = FALSE;
	static bool_t	opened_once = FALSE;

	/* If device has not been opened, attempt to open it */
	if (slioc_not_open) {
		/* Check for another copy of the CD player running on
		 * the specified device.
		 */
		if (!cd_devlock(app_data.device)) {
			dpy_time(s, FALSE);
			slioc_start_insert_poll(s);
			return FALSE;
		}

		/* Open CD-ROM device */
		if (!slioc_open(app_data.device)) {
			dpy_time(s, FALSE);
			slioc_start_insert_poll(s);
			return FALSE;
		}

		if (!opened_once)
			first_open = TRUE;

		slioc_not_open = FALSE;
		opened_once = TRUE;
	}

	for (i = 0; i < 5; i++) {
		/* Check if a CD is loaded */
		if ((err = !slioc_disc_present(first_open)) == TRUE) {
			s->mode = M_NODISC;
			dbprog_dbclear(s);
		}
		else
			break;
	}

	if (!err && first_open) {
		/* Fill in inquiry data */
		slioc_vendor_model(s);

		/* Query current volume and warp volume and balance
		 * sliders to appropriate setting
		 */
		if ((vol = slioc_cfg_vol(0, s, TRUE, WARP_VOL | WARP_BAL)) >= 0)
			s->level = (byte_t) vol;
		else
			s->level = 0;
	}

	/* Read disc table of contents */
	if (err || (s->mode == M_NODISC && !slioc_get_toc(s))) {
		reset_curstat(s, TRUE);
		dpy_all(s);

		if (app_data.eject_close) {
			/* Close device */
			slioc_close();

			slioc_not_open = TRUE;
		}

		slioc_start_insert_poll(s);
		return FALSE;
	}

	if (s->mode == M_NODISC) {
		/* Load CD database entry for this disc */
		dbprog_dbget(s);

		s->mode = M_STOP;
		dpy_all(s);

		if (app_data.load_play) {
			/* Start auto-play */
			slioc_play_pause(s);
		}
		else if (app_data.load_spindown) {
			/* Spin down disc in case the user isn't going to
			 * play anything for a while.  This reduces wear and
			 * tear on the drive.
			 */
			slioc_start_stop(FALSE, FALSE);
		}
		else {
			switch (slioc_tst_status) {
			case M_PLAY:
			case M_PAUSE:
				/* Drive is current playing audio or paused:
				 * act appropriately.
				 */
				s->mode = slioc_tst_status;
				slioc_get_playstatus(s);
				dpy_all(s);
				if (s->mode == M_PAUSE)
					cd_pause_blink(s, TRUE);
				else
					slioc_start_stat_poll(s);
				break;
			default:
				/* Drive is stopped: do nothing */
				break;
			}
		}
	}

	return TRUE;
}


/*
 * slioc_run_rew
 *	Run search-rewind operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_run_rew(curstat_t *s)
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
	if (!slioc_get_playstatus(s)) {
		cd_beep();
		return;
	}

	skip_blks = app_data.skip_blks;
	addr = s->cur_tot_addr;

	if (slioc_start_search) {
		slioc_start_search = FALSE;
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
	slioc_do_playaudio(
		ADDR_BLK | ADDR_MSF | ADDR_OPTEND,
		start_addr, end_addr,
		&smsf, &emsf,
		0, 0
	);

	slioc_search_id = cd_timeout(
		app_data.skip_pause,
		slioc_run_rew,
		(byte_t *) s
	);
}


/*
 * slioc_stop_rew
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
slioc_stop_rew(curstat_t *s)
{
	cd_untimeout(slioc_search_id);
}


/*
 * slioc_run_ff
 *	Run search-fast-forward operation
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
slioc_run_ff(curstat_t *s)
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
	if (!slioc_get_playstatus(s)) {
		cd_beep();
		return;
	}

	skip_blks = app_data.skip_blks;
	addr = s->cur_tot_addr;

	if (slioc_start_search) {
		slioc_start_search = FALSE;
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
	slioc_do_playaudio(
		ADDR_BLK | ADDR_MSF | ADDR_OPTEND,
		start_addr, end_addr,
		&smsf, &emsf,
		0, 0
	);

	slioc_search_id = cd_timeout(
		app_data.skip_pause,
		slioc_run_ff,
		(byte_t *) s
	);
}


/*
 * slioc_stop_ff
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
slioc_stop_ff(curstat_t *s)
{
	cd_untimeout(slioc_search_id);
}


/*
 * slioc_run_ab
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
slioc_run_ab(curstat_t *s)
{
	return (
		slioc_do_playaudio(
			ADDR_BLK | ADDR_MSF,
			slioc_ab_start_addr, slioc_ab_end_addr,
			&slioc_ab_start_msf, &slioc_ab_end_msf,
			0, 0
		)
	);
}


/*
 * slioc_run_sample
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
slioc_run_sample(curstat_t *s)
{
	word32_t	saddr,
			eaddr;
	msf_t		smsf,
			emsf;

	if (slioc_next_sam < s->tot_trks) {
		saddr = s->trkinfo[slioc_next_sam].addr;
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

		if (slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
				 saddr, eaddr, &smsf, &emsf, 0, 0)) {
			slioc_next_sam++;
			return TRUE;
		}
	}

	slioc_next_sam = 0;
	return FALSE;
}


/*
 * slioc_run_prog
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
slioc_run_prog(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr,
			end_addr;
	msf_t		start_msf,
			end_msf;
	bool_t		ret;

	if (!s->shuffle && s->prog_tot <= 0)
		return FALSE;

	if (slioc_new_progshuf) {
		if (s->shuffle)
			/* New shuffle sequence needed */
			reset_shuffle(s);
		else
			/* Program play: simply reset the count */
			s->prog_cnt = 0;

		slioc_new_progshuf = FALSE;
	}

	if (s->prog_cnt >= s->prog_tot)
		/* Done with program/shuffle play cycle */
		return FALSE;

	if ((i = curprog_pos(s)) < 0)
		return FALSE;

	if (s->trkinfo[i].trkno == CDROM_LEADOUT)
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

	ret = slioc_do_playaudio(
		ADDR_BLK | ADDR_MSF,
		start_addr, end_addr,
		&start_msf, &end_msf,
		0, 0
	);

	if (s->mode == M_PAUSE) {
		slioc_pause_resume(FALSE);

		/* Restore volume */
		slioc_mute_off(s);
	}

	return (ret);
}


/*
 * slioc_run_repeat
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
slioc_run_repeat(curstat_t *s)
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

		slioc_new_progshuf = TRUE;
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

		ret = slioc_do_playaudio(
			ADDR_BLK | ADDR_MSF,
			s->trkinfo[0].addr, s->tot_addr,
			&start_msf, &end_msf, 0, 0
		);

		if (s->mode == M_PAUSE) {
			slioc_pause_resume(FALSE);

			/* Restore volume */
			slioc_mute_off(s);
		}

	}

	return (ret);
}


/***********************
 *   public routines   *
 ***********************/


/*
 * slioc_init
 *	Top-level function to initialize the SunOS/Linux ioctl method.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_init(curstat_t *s, di_tbl_t *dt)
{
	int	i;

	if (app_data.di_method != DI_SLIOC)
		/* SunOS/Linux ioctl method not configured */
		return;

	/* Initialize libdi calling table */
	dt->check_disc = slioc_check_disc;
	dt->status_upd = slioc_status_upd;
	dt->lock = slioc_lock;
	dt->repeat = slioc_repeat;
	dt->shuffle = slioc_shuffle;
	dt->load_eject = slioc_load_eject;
	dt->ab = slioc_ab;
	dt->sample = slioc_sample;
	dt->level = slioc_level;
	dt->play_pause = slioc_play_pause;
	dt->stop = slioc_stop;
	dt->prevtrk = slioc_prevtrk;
	dt->nexttrk = slioc_nexttrk;
	dt->previdx = slioc_previdx;
	dt->nextidx = slioc_nextidx;
	dt->rew = slioc_rew;
	dt->ff = slioc_ff;
	dt->warp = slioc_warp;
	dt->route = NULL;
	dt->mute_on = slioc_mute_on;
	dt->mute_off = slioc_mute_off;
	dt->start = slioc_start;
	dt->icon = slioc_icon;
	dt->halt = slioc_halt;
	dt->mode = slioc_mode;
	dt->vers = slioc_vers;

	/* Hardwire some unsupported features */
	app_data.caddylock_supp = FALSE;
	app_data.caddy_lock = FALSE;
	app_data.chroute_supp = FALSE;

	/* Initalize SunOS/Linux ioctl method */
	slioc_stat_polling = FALSE;
	slioc_stat_interval = app_data.stat_interval;
	slioc_insert_polling = FALSE;
	slioc_next_sam = FALSE;
	slioc_new_progshuf = FALSE;
	slioc_sav_end_addr = 0;
	slioc_sav_end_msf.min = slioc_sav_end_msf.sec =
		slioc_sav_end_msf.frame = 0;
	slioc_sav_end_fmt = 0;

	/* Initialize curstat structure */
	reset_curstat(s, TRUE);
}


/*
 * slioc_check_disc
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
slioc_check_disc(curstat_t *s)
{
	return (slioc_disc_ready(s));
}


/*
 * slioc_status_upd
 *	Force update of playback status
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_status_upd(curstat_t *s)
{
	slioc_get_playstatus(s);
}


/*
 * slioc_lock
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
slioc_lock(curstat_t *s, bool_t enable)
{
	/* Caddy lock function currently not supported
	 * under SunOS/Linux ioctl method
	 */
	if (enable) {
		cd_beep();
		set_lock_btn(FALSE);
	}
}


/*
 * slioc_repeat
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
slioc_repeat(curstat_t *s, bool_t enable)
{
	s->repeat = enable;
	dpy_rptcnt(s);
}


/*
 * slioc_shuffle
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
slioc_shuffle(curstat_t *s, bool_t enable)
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
 * slioc_load_eject
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
slioc_load_eject(curstat_t *s)
{
	bool_t	ret = FALSE;

	if (!slioc_disc_ready(s)) {
		/* Disc not ready: try loading the disc */
		if (!slioc_start_stop(TRUE, TRUE))
			cd_beep();

		return;
	}

	/* Eject the disc */

	if (!app_data.eject_supp) {
		cd_beep();

		slioc_stop_stat_poll();
		reset_curstat(s, TRUE);
		s->mode = M_NODISC;

		dbprog_dbclear(s);
		dpy_all(s);

		if (app_data.eject_close) {
			/* Close device */
			slioc_close();

			slioc_not_open = TRUE;
		}

		slioc_start_insert_poll(s);
		return;
	}

	slioc_stop_stat_poll();
	reset_curstat(s, TRUE);
	s->mode = M_NODISC;

	dbprog_dbclear(s);
	dpy_all(s);

	slioc_start_stop(FALSE, TRUE);

	if (app_data.eject_exit)
		cd_quit(s);
	else {
		if (app_data.eject_close) {
			/* Close device */
			slioc_close();

			slioc_not_open = TRUE;
		}
			
		slioc_start_insert_poll(s);
	}
}


/*
 * slioc_ab
 *	A->B segment play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_ab(curstat_t *s)
{
	switch (s->mode) {
	case M_SAMPLE:
	case M_PLAY:
		/* Get current location */
		if (!slioc_get_playstatus(s)) {
			cd_beep();
			break;
		}

		slioc_ab_start_addr = s->cur_tot_addr;
		slioc_ab_start_msf.min = s->cur_tot_min;
		slioc_ab_start_msf.sec = s->cur_tot_sec;
		slioc_ab_start_msf.frame = s->cur_tot_frame;

		s->mode = M_A;
		dpy_playmode(s, FALSE);
		break;

	case M_A:
		/* Get current location */
		if (!slioc_get_playstatus(s)) {
			cd_beep();
			break;
		}

		slioc_ab_end_addr = s->cur_tot_addr;
		slioc_ab_end_msf.min = s->cur_tot_min;
		slioc_ab_end_msf.sec = s->cur_tot_sec;
		slioc_ab_end_msf.frame = s->cur_tot_frame;

		/* Make sure that the A->B play interval is no less
		 * than a user-configurable minimum.
		 */
		if ((slioc_ab_end_addr - slioc_ab_start_addr) <
		    app_data.min_playblks) {
			slioc_ab_end_addr = slioc_ab_start_addr +
					    app_data.min_playblks;
			blktomsf(
				slioc_ab_end_addr,
				&slioc_ab_end_msf.min,
				&slioc_ab_end_msf.sec,
				&slioc_ab_end_msf.frame,
				MSF_OFFSET(s)
			);
		}

		if (!slioc_run_ab(s)) {
			cd_beep();
			return;
		}

		s->mode = M_AB;
		dpy_playmode(s, FALSE);
		break;

	case M_AB:
		/* Currently doing A->B playback, just call slioc_play_pause
		 * to resume normal playback.
		 */
		slioc_play_pause(s);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * slioc_sample
 *	Sample play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_sample(curstat_t *s)
{
	int	i;

	if (!slioc_disc_ready(s)) {
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
		slioc_start_stat_poll(s);
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
			slioc_next_sam = i;
		}
		else {
			s->cur_trk = s->first_trk;
			slioc_next_sam = 0;
		}
		
		s->cur_idx = 1;

		s->mode = M_SAMPLE;
		dpy_all(s);

		if (!slioc_run_sample(s))
			return;

		break;

	case M_SAMPLE:
		/* Currently doing Sample playback, just call slioc_play_pause
		 * to resume normal playback.
		 */
		slioc_play_pause(s);
		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * slioc_level
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
slioc_level(curstat_t *s, byte_t level, bool_t drag)
{
	int	actual;
	byte_t	warpflg;

	if (drag)
		warpflg = WARP_VOL;
	else
		warpflg = WARP_VOL | WARP_BAL;

	/* Set volume level */
	if ((actual = slioc_cfg_vol((int) level, s, FALSE, warpflg)) >= 0)
		s->level = (byte_t) actual;
}


/*
 * slioc_play_pause
 *	Audio playback and pause function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_play_pause(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;

	if (!slioc_disc_ready(s)) {
		cd_beep();
		return;
	}

	if (s->mode == M_NODISC)
		s->mode = M_STOP;

	switch (s->mode) {
	case M_PLAY:
		/* Currently playing: go to pause mode */

		if (!slioc_pause_resume(FALSE)) {
			cd_beep();
			return;
		}
		slioc_stop_stat_poll();
		s->mode = M_PAUSE;
		dpy_playmode(s, FALSE);
		break;

	case M_PAUSE:
		/* Currently paused: resume play */

		if (!slioc_pause_resume(TRUE)) {
			cd_beep();
			return;
		}
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		slioc_start_stat_poll(s);
		break;

	case M_STOP:
		/* Currently stopped: start play */

		if (s->shuffle || s->prog_tot > 0) {
			slioc_new_progshuf = TRUE;

			/* Start shuffle/program play */
			if (!slioc_run_prog(s)) {
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

			if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				s->mode = M_STOP;
				return;
			}
		}

		dpy_all(s);
		slioc_start_stat_poll(s);
		break;

	case M_A:
		/* Just reset mode to play and continue */
		s->mode = M_PLAY;
		dpy_playmode(s, FALSE);
		break;

	case M_AB:
	case M_SAMPLE:
		/* Force update of curstat */
		if (!slioc_get_playstatus(s)) {
			cd_beep();
			return;
		}

		/* Currently doing a->b or sample playback: just resume play */
		if (s->shuffle || s->program) {
			if ((i = curtrk_pos(s)) < 0 ||
			    s->trkinfo[i].trkno == CDROM_LEADOUT)
				return;

			start_msf.min = s->cur_tot_min;
			start_msf.sec = s->cur_tot_sec;
			start_msf.frame = s->cur_tot_frame;
			end_msf.min = s->trkinfo[i+1].min;
			end_msf.sec = s->trkinfo[i+1].sec;
			end_msf.frame = s->trkinfo[i+1].frame;

			if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
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

			if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
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
 * slioc_stop
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
slioc_stop(curstat_t *s, bool_t stop_disc)
{
	/* The stop_disc parameter will cause the disc to spin down.
	 * This is usually set to TRUE, but can be FALSE if the caller
	 * just wants to set the current state to stop but will
	 * immediately go into play state again.  Not spinning down
	 * the drive makes things a little faster...
	 */

	if (!slioc_disc_ready(s)) {
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

		if (stop_disc && !slioc_start_stop(FALSE, FALSE)) {
			cd_beep();
			return;
		}
		slioc_stop_stat_poll();

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
 * slioc_prevtrk
 *	Previous track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_prevtrk(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;
	bool_t		go_prev;

	if (!slioc_disc_ready(s)) {
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
				slioc_new_progshuf = FALSE;
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
					slioc_new_progshuf = FALSE;
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
			slioc_mute_on(s);

		if (s->prog_tot > 0) {
			/* Program/Shuffle mode: just stop the playback
			 * and let slioc_run_prog go to the previous track
			 */
			slioc_fake_stop = TRUE;

			/* Force status update */
			slioc_get_playstatus(s);
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

			if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();

				/* Restore volume */
				slioc_mute_off(s);
				return;
			}

			if (s->mode == M_PAUSE) {
				slioc_pause_resume(FALSE);

				/* Restore volume */
				slioc_mute_off(s);
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
 * slioc_nexttrk
 *	Next track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_nexttrk(curstat_t *s)
{
	sword32_t	i;
	word32_t	start_addr;
	msf_t		start_msf,
			end_msf;

	if (!slioc_disc_ready(s)) {
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
					slioc_new_progshuf = TRUE;
				else
					return;
			}

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				slioc_mute_on(s);

			/* Program/Shuffle mode: just stop the playback
			 * and let slioc_run_prog go to the next track.
			 */
			slioc_fake_stop = TRUE;

			/* Force status update */
			slioc_get_playstatus(s);

			return;
		}

		/* Find next track */
		if ((i = curtrk_pos(s)) < 0)
			return;

		if (i < (MAXTRACK - 1) &&
		    s->trkinfo[i+1].trkno >= 0 &&
		    s->trkinfo[i+1].trkno != CDROM_LEADOUT) {

			start_addr = s->trkinfo[i+1].addr;
			start_msf.min = s->trkinfo[i+1].min;
			start_msf.sec = s->trkinfo[i+1].sec;
			start_msf.frame = s->trkinfo[i+1].frame;
			s->cur_trk = s->trkinfo[i+1].trkno;
			s->cur_idx = 1;

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				slioc_mute_on(s);

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

			if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
					  start_addr, s->tot_addr,
					  &start_msf, &end_msf, 0, 0)) {
				cd_beep();
				return;
			}

			if (s->mode == M_PAUSE) {
				slioc_pause_resume(FALSE);

				/* Restore volume */
				slioc_mute_off(s);
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
			    s->trkinfo[i+1].trkno != CDROM_LEADOUT) {
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
 * slioc_previdx
 *	Previous index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_previdx(curstat_t *s)
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
		 * Since there is no standard command to start
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
		slioc_mute_on(s);

		if (!slioc_do_playaudio(ADDR_TRKIDX, 0, 0, NULL, NULL,
				  (byte_t) s->cur_trk, idx)) {
			/* Restore volume */
			slioc_mute_off(s);
			cd_beep();
			return;
		}

		slioc_idx_pause = TRUE;

		if (!slioc_pause_resume(FALSE)) {
			/* Restore volume */
			slioc_mute_off(s);
			slioc_idx_pause = FALSE;
			return;
		}

		/* Use slioc_get_playstatus to update the current status */
		if (!slioc_get_playstatus(s)) {
			/* Restore volume */
			slioc_mute_off(s);
			slioc_idx_pause = FALSE;
			return;
		}

		/* Save starting block addr of this index */
		s->sav_iaddr = s->cur_tot_addr;

		if (s->mode != M_PAUSE)
			/* Restore volume */
			slioc_mute_off(s);

		start_msf.min = s->cur_tot_min;
		start_msf.sec = s->cur_tot_sec;
		start_msf.frame = s->cur_tot_frame;
		end_msf.min = s->tot_min;
		end_msf.sec = s->tot_sec;
		end_msf.frame = s->tot_frame;

		if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
				  s->cur_tot_addr, s->tot_addr,
				  &start_msf, &end_msf, 0, 0)) {
			cd_beep();
			slioc_idx_pause = FALSE;
			return;
		}

		slioc_idx_pause = FALSE;

		if (s->mode == M_PAUSE) {
			slioc_pause_resume(FALSE);

			/* Restore volume */
			slioc_mute_off(s);

			/* Force update of curstat */
			slioc_get_playstatus(s);
		}

		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * slioc_nextidx
 *	Next index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_nextidx(curstat_t *s)
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
		 * Since there is no standard command to start
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
		slioc_mute_on(s);

		if (!slioc_do_playaudio(ADDR_TRKIDX, 0, 0, NULL, NULL,
				  (byte_t) s->cur_trk,
				  (byte_t) (s->cur_idx + 1))) {
			/* Restore volume */
			slioc_mute_off(s);
			cd_beep();
			return;
		}

		slioc_idx_pause = TRUE;

		if (!slioc_pause_resume(FALSE)) {
			/* Restore volume */
			slioc_mute_off(s);
			slioc_idx_pause = FALSE;
			return;
		}

		/* Use slioc_get_playstatus to update the current status */
		if (!slioc_get_playstatus(s)) {
			/* Restore volume */
			slioc_mute_off(s);
			slioc_idx_pause = FALSE;
			return;
		}

		/* Save starting block addr of this index */
		s->sav_iaddr = s->cur_tot_addr;

		if (s->mode != M_PAUSE)
			/* Restore volume */
			slioc_mute_off(s);

		start_msf.min = s->cur_tot_min;
		start_msf.sec = s->cur_tot_sec;
		start_msf.frame = s->cur_tot_frame;
		end_msf.min = s->tot_min;
		end_msf.sec = s->tot_sec;
		end_msf.frame = s->tot_frame;

		if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
				  s->cur_tot_addr, s->tot_addr,
				  &start_msf, &end_msf, 0, 0)) {
			cd_beep();
			slioc_idx_pause = FALSE;
			return;
		}

		slioc_idx_pause = FALSE;

		if (s->mode == M_PAUSE) {
			slioc_pause_resume(FALSE);

			/* Restore volume */
			slioc_mute_off(s);

			/* Force update of curstat */
			slioc_get_playstatus(s);
		}

		break;

	default:
		cd_beep();
		break;
	}
}


/*
 * slioc_rew
 *	Search-rewind function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_rew(curstat_t *s, bool_t start)
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
		slioc_play_pause(s);

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (start) {
			/* Button press */

			if (s->mode == M_PLAY)
				slioc_stop_stat_poll();

			/* Reduce volume */
			vol = (byte_t) ((int) s->level *
				app_data.skip_vol / 100);

			(void) slioc_cfg_vol((int)
				((vol < (byte_t)app_data.skip_minvol) ?
				 (byte_t) app_data.skip_minvol : vol),
				s,
				FALSE,
				0
			);

			/* Start search rewind */
			slioc_start_search = TRUE;
			slioc_run_rew(s);
		}
		else {
			/* Button release */

			slioc_stop_rew(s);

			/* Update display */
			slioc_get_playstatus(s);

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				slioc_mute_on(s);
			else
				/* Restore volume */
				slioc_mute_off(s);

			if (s->shuffle || s->program) {
				if ((i = curtrk_pos(s)) < 0 ||
				    s->trkinfo[i].trkno == CDROM_LEADOUT) {
					/* Restore volume */
					slioc_mute_off(s);
					return;
				}

				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->trkinfo[i+1].min;
				end_msf.sec = s->trkinfo[i+1].sec;
				end_msf.frame = s->trkinfo[i+1].frame;

				if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->trkinfo[i+1].addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					slioc_mute_off(s);
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

				if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->tot_addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					slioc_mute_off(s);
					return;
				}
			}

			if (s->mode == M_PAUSE) {
				slioc_pause_resume(FALSE);

				/* Restore volume */
				slioc_mute_off(s);
			}
			else
				slioc_start_stat_poll(s);
		}
		break;

	default:
		if (start)
			cd_beep();
		break;
	}
}


/*
 * slioc_ff
 *	Search-fast-forward function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_ff(curstat_t *s, bool_t start)
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
		slioc_play_pause(s);

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_PAUSE:
		if (start) {
			/* Button press */

			if (s->mode == M_PLAY)
				slioc_stop_stat_poll();

			/* Reduce volume */
			vol = (byte_t) ((int) s->level *
				app_data.skip_vol / 100);

			(void) slioc_cfg_vol((int)
				((vol < (byte_t)app_data.skip_minvol) ?
				 (byte_t) app_data.skip_minvol : vol),
				s,
				FALSE,
				0
			);

			/* Start search forward */
			slioc_start_search = TRUE;
			slioc_run_ff(s);
		}
		else {
			/* Button release */

			slioc_stop_ff(s);

			/* Update display */
			slioc_get_playstatus(s);

			if (s->mode == M_PAUSE)
				/* Mute: so we don't get a transient */
				slioc_mute_on(s);
			else
				/* Restore volume */
				slioc_mute_off(s);

			if (s->shuffle || s->program) {
				if ((i = curtrk_pos(s)) < 0 ||
				    s->trkinfo[i].trkno == CDROM_LEADOUT) {
					/* Restore volume */
					slioc_mute_off(s);
					return;
				}

				start_msf.min = s->cur_tot_min;
				start_msf.sec = s->cur_tot_sec;
				start_msf.frame = s->cur_tot_frame;
				end_msf.min = s->trkinfo[i+1].min;
				end_msf.sec = s->trkinfo[i+1].sec;
				end_msf.frame = s->trkinfo[i+1].frame;

				if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->trkinfo[i+1].addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					slioc_mute_off(s);
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

				if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
						  s->cur_tot_addr,
						  s->tot_addr,
						  &start_msf, &end_msf,
						  0, 0)) {
					cd_beep();

					/* Restore volume */
					slioc_mute_off(s);
					return;
				}
			}
			if (s->mode == M_PAUSE) {
				slioc_pause_resume(FALSE);

				/* Restore volume */
				slioc_mute_off(s);
			}
			else
				slioc_start_stat_poll(s);
		}
		break;

	default:
		if (start)
			cd_beep();
		break;
	}
}


/*
 * slioc_warp
 *	Track warp function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_warp(curstat_t *s)
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
		slioc_play_pause(s);

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
			slioc_mute_on(s);

		if (!slioc_do_playaudio(ADDR_BLK | ADDR_MSF,
				  start_addr, end_addr,
				  &start_msf, &end_msf,
				  0, 0)) {
			cd_beep();

			/* Restore volume */
			slioc_mute_off(s);
			return;
		}

		if (s->mode == M_PAUSE) {
			slioc_pause_resume(FALSE);

			/* Restore volume */
			slioc_mute_off(s);
		}

		break;

	default:
		break;
	}
}


/*
 * slioc_mute_on
 *	Mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_mute_on(curstat_t *s)
{
	(void) slioc_cfg_vol(0, s, FALSE, 0);
}


/*
 * slioc_mute_off
 *	Un-mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_mute_off(curstat_t *s)
{
	(void) slioc_cfg_vol((int) s->level, s, FALSE, 0);
}


/*
 * slioc_start
 *	Start the SunOS/Linux ioctl method.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_start(curstat_t *s)
{
	/* Check to see if disc is ready */
	(void) slioc_disc_ready(s);

	/* Update display */
	dpy_all(s);
}


/*
 * slioc_icon
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
slioc_icon(curstat_t *s, bool_t iconified)
{
	/* This function attempts to reduce the status polling frequency
	 * when possible to cut down on CPU and bus usage.  This is
	 * done when the CD player is iconified.
	 */

	/* Increase status polling interval by 4 seconds when iconified */
	if (iconified)
		slioc_stat_interval = app_data.stat_interval + 4000;
	else
		slioc_stat_interval = app_data.stat_interval;

	/* Check disc status */
	if (!slioc_disc_ready(s))
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
		slioc_stat_interval = app_data.stat_interval;
		break;

	case M_PLAY:
		if (!iconified) {
			/* Force an immediate update */
			slioc_stop_stat_poll();
			slioc_start_stat_poll(s);
		}
		break;
	}
}


/*
 * slioc_halt
 *	Shut down the SunOS/Linux ioctl method.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
slioc_halt(curstat_t *s)
{
	int	i;

	if (s->mode != M_NODISC) {
		if (app_data.exit_eject && app_data.eject_supp) {
			/* User closing application: Eject disc */
			slioc_start_stop(FALSE, TRUE);
		}
		else {
			if (app_data.exit_stop)
				/* User closing application: Stop disc */
				slioc_start_stop(FALSE, FALSE);

			switch (s->mode) {
			case M_PLAY:
			case M_PAUSE:
			case M_A:
			case M_AB:
			case M_SAMPLE:
				slioc_stop_stat_poll();
				break;
			}
		}
	}

	/* Close device */
	slioc_close();
}


/*
 * slioc_mode
 *	Return a text string indicating the current method.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Method text string.
 */
char *
slioc_mode(void)
{
#ifdef linux
	return ("Linux ioctl method");
#else
	return ("SunOS ioctl method");
#endif
}


/*
 * slioc_vers
 *	Return a text string indicating the method's version number.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Version text string.
 */
char *
slioc_vers(void)
{
	return ("");
}

#endif	/* DI_SLIOC DEMO_ONLY */

