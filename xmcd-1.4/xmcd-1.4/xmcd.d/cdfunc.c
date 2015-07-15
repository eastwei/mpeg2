/*
 *   xmcd - Motif(tm) CD Audio Player
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
static char *_cdfunc_c_ident_ = "@(#)cdfunc.c	5.15 95/02/01";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "common.d/patchlevel.h"
#include "common.d/util.h"
#include "xmcd.d/xmcd.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/dbprog.h"
#include "xmcd.d/hotkey.h"
#include "xmcd.d/help.h"
#include "xmcd.d/cdfunc.h"
#include "libdi.d/libdi.h"


/* Callback info structure */
typedef struct {
	Widget		widget;
	String		type;
	XtCallbackProc	func;
	XtPointer	data;
} cbinfo_t;


extern widgets_t	widgets;
extern pixmaps_t	pixmaps;
extern appdata_t	app_data;
extern bool_t		exit_flag;
extern FILE		*errfp;

char			**dbdirs = NULL;	/* Database directories */
STATIC char		keystr[3],		/* Keypad number string */
			lockfile[40];		/* Lock file path */
STATIC long		tm_blinkid = -1,	/* Time dpy blink timer ID */
			ab_blinkid = -1;	/* A->B dpy blink timer ID */
STATIC word32_t		warp_offset = 0;	/* Track warp block offset */
STATIC bool_t		devbusy = FALSE,	/* Device busy flag */
			searching = FALSE,	/* Running REW or FF */
			btnlbl_state = FALSE,	/* Button label state */
			popup_ok = FALSE,	/* Are popups allowed? */
			pseudo_warp = FALSE,	/* Warp slider only flag */
			warp_busy = FALSE;	/* Warp function active */


/***********************
 *  internal routines  *
 ***********************/


/*
 * track_rtime
 *	Return the remaining time of the current playing track in seconds.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The track remaining time in seconds.
 */
STATIC sword32_t
track_rtime(curstat_t *s)
{
	sword32_t	i,
			secs,
			tot_sec,
			cur_sec;

	if ((i = curtrk_pos(s)) < 0)
		return 0;

	tot_sec = (s->trkinfo[i+1].min * 60 + s->trkinfo[i+1].sec) -
		  (s->trkinfo[i].min * 60 + s->trkinfo[i].sec);
	cur_sec = s->cur_trk_min * 60 + s->cur_trk_sec;
	secs = tot_sec - cur_sec;

	return ((secs >= 0) ? secs : 0);
}


/*
 * disc_rtime_norm
 *	Return the remaining time of the disc in seconds.  This is
 *	used during normal playback.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The disc remaining time in seconds.
 */
STATIC sword32_t
disc_rtime_norm(curstat_t *s)
{
	sword32_t	secs;

	secs = (s->tot_min * 60 + s->tot_sec) -
		(s->cur_tot_min * 60 + s->cur_tot_sec);

	return ((secs >= 0) ? secs : 0);
}


/*
 * disc_rtime_prog
 *	Return the remaining time of the disc in seconds.  This is
 *	used during shuffle or program mode.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The disc remaining time in seconds.
 */
STATIC sword32_t
disc_rtime_prog(curstat_t *s)
{
	sword32_t	i,
			secs = 0;

	/* Find the time of all unplayed tracks */
	for (i = s->prog_cnt; i < (int) s->prog_tot; i++) {
		secs += ((s->trkinfo[s->playorder[i]+1].min * 60 +
			 s->trkinfo[s->playorder[i]+1].sec) -
		         (s->trkinfo[s->playorder[i]].min * 60 +
			 s->trkinfo[s->playorder[i]].sec));
	}

	/* FInd the remaining time of the current track */
	for (i = 0; i < MAXTRACK; i++) {
		if (s->trkinfo[i].trkno == LEAD_OUT_TRACK)
			break;

		if (s->trkinfo[i].trkno == s->cur_trk) {
			secs += ((s->trkinfo[i+1].min * 60 +
				  s->trkinfo[i+1].sec) -
				 (s->cur_tot_min * 60 + s->cur_tot_sec));

			break;
		}
	}

	return ((secs >= 0) ? secs : 0);
}


/*
 * common_parminit
 *	Read the common configuration file and initialize parameters.
 *
 * Args:
 *	path - Path name to the file to read.
 *
 * Return:
 *	Nothing.
 */
STATIC void
common_parminit(char *path, bool_t priv)
{
	FILE		*fp;
	char		buf[STR_BUF_SZ * 16],
			errstr[ERR_BUF_SZ],
			parm[128];
	static bool_t	dev_cmdline = FALSE,
			force_debug = FALSE;

	if ((fp = fopen(path, "r")) == NULL) {
		if (priv && !di_isdemo()) {
			/* Cannot open system config file. */
			sprintf(errstr, app_data.str_nocfg, path);
			cd_fatal_popup(app_data.str_fatal, errstr);
		}
		return;
	}

	if (priv && app_data.debug)
		force_debug = TRUE;

	/* Read in common parameters */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		/* Skip comments */
		if (buf[0] == '#' || buf[0] == '!' || buf[0] == '\n')
			continue;

		if (sscanf(buf, "device: %s\n", parm) > 0) {
			if (priv && app_data.device != NULL)
				dev_cmdline = TRUE;
			if (!dev_cmdline) {
				if (app_data.device != NULL)
					MEM_FREE(app_data.device);

				app_data.device = (char *) MEM_ALLOC(
					strlen(parm) + 1
				);
				if (app_data.device == NULL) {
					cd_fatal_popup(
						app_data.str_fatal,
						app_data.str_nomemory
					);
				}
				strcpy(app_data.device, parm);
			}
			continue;
		}
		if (sscanf(buf, "dbdir: %s\n", parm) > 0) {
			if (!priv || app_data.dbdir == NULL) {
				if (app_data.dbdir != NULL)
					MEM_FREE(app_data.dbdir);

				app_data.dbdir = (char *) MEM_ALLOC(
					strlen(parm) + 1
				);
				if (app_data.dbdir == NULL) {
					cd_fatal_popup(
						app_data.str_fatal,
						app_data.str_nomemory
					);
				}
				strcpy(app_data.dbdir, parm);
			}
			continue;
		}
		if (sscanf(buf, "maxDbdirs: %s\n", parm) > 0) {
			app_data.max_dbdirs = atoi(parm);
			continue;
		}
		if (sscanf(buf, "dbFileMode: %s\n", parm) > 0) {
			if (!priv || app_data.dbfile_mode == NULL) {
				if (app_data.dbfile_mode != NULL)
					MEM_FREE(app_data.dbfile_mode);

				app_data.dbfile_mode = (char *) MEM_ALLOC(
					strlen(parm) + 1
				);
				if (app_data.dbfile_mode == NULL) {
					cd_fatal_popup(
						app_data.str_fatal,
						app_data.str_nomemory
					);
				}
				strcpy(app_data.dbfile_mode, parm);
			}
			continue;
		}
		if (sscanf(buf, "statusPollInterval: %s\n", parm) > 0) {
			app_data.stat_interval = atoi(parm);
			continue;
		}
		if (sscanf(buf, "insertPollInterval: %s\n", parm) > 0) {
			app_data.ins_interval = atoi(parm);
			continue;
		}
		if (sscanf(buf, "previousThreshold: %s\n", parm) > 0) {
			app_data.prev_threshold = atoi(parm);
			continue;
		}
		if (sscanf(buf, "searchSkipBlocks: %s\n", parm) > 0) {
			app_data.skip_blks = atoi(parm);
			continue;
		}
		if (sscanf(buf, "searchPauseInterval: %s\n", parm) > 0) {
			app_data.skip_pause = atoi(parm);
			continue;
		}
		if (sscanf(buf, "searchSpeedUpCount: %s\n", parm) > 0) {
			app_data.skip_spdup = atoi(parm);
			continue;
		}
		if (sscanf(buf, "searchVolumePercent: %s\n", parm) > 0) {
			app_data.skip_vol = atoi(parm);
			continue;
		}
		if (sscanf(buf, "searchMinVolume: %s\n", parm) > 0) {
			app_data.skip_minvol = atoi(parm);
			continue;
		}
		if (sscanf(buf, "sampleBlocks: %s\n", parm) > 0) {
			app_data.sample_blks = atoi(parm);
			continue;
		}
		if (sscanf(buf, "solaris2VolumeManager: %s\n", parm) > 0) {
			app_data.sol2_volmgt = stob(parm);
			continue;
		}
		if (sscanf(buf, "showScsiErrMsg: %s\n", parm) > 0) {
			app_data.scsierr_msg = stob(parm);
			continue;
		}
		if (sscanf(buf, "debugMode: %s\n", parm) > 0) {
			if (!force_debug)
				app_data.debug = stob(parm);
			continue;
		}
	}

	fclose(fp);
}


/*
 * devspec_parminit
 *	Read the specified device-specific configuration file and
 *	initialize parameters.
 *
 * Args:
 *	path - Path name to the file to read.
 *	priv - Whether the privileged keywords are to be recognized.
 *
 * Return:
 *	Nothing.
 */
STATIC void
devspec_parminit(char *path, bool_t priv)
{
	FILE	*fp;
	char	buf[STR_BUF_SZ * 16],
		errstr[ERR_BUF_SZ],
		parm[128];

	if ((fp = fopen(path, "r")) == NULL) {
		if (priv && !di_isdemo()) {
			/* Cannot open master device-specific
			 * config file.
			 */
			sprintf(errstr, app_data.str_nocfg, path);
			cd_fatal_popup(app_data.str_fatal, errstr);
		}
		return;
	}

	/* Read in device-specific parameters */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		/* Skip comments */
		if (buf[0] == '#' || buf[0] == '!' || buf[0] == '\n')
			continue;

		/* These are privileged parameters and users
		 * cannot overide them in their .xmcdcfg file.
		 */
		if (priv) {
			if (sscanf(buf, "logicalDriveNumber: %s\n",
				   parm) > 0) {
				app_data.devnum = atoi(parm);
				continue;
			}
			if (sscanf(buf, "deviceInterfaceMethod: %s\n",
				   parm) > 0) {
				app_data.di_method = atoi(parm);
				continue;
			}
			if (sscanf(buf, "driveVendorCode: %s\n",
				   parm) > 0) {
				app_data.vendor_code = atoi(parm);
				continue;
			}
			if (sscanf(buf, "scsiAudioVolumeBase: %s\n",
				   parm) > 0) {
				app_data.base_scsivol = atoi(parm);
				continue;
			}
			if (sscanf(buf, "minimumPlayBlocks: %s\n",
				   parm) > 0) {
				app_data.min_playblks = atoi(parm);
				continue;
			}
			if (sscanf(buf, "playAudio10Support: %s\n",
				   parm) > 0) {
				app_data.play10_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "playAudio12Support: %s\n",
				   parm) > 0) {
				app_data.play12_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "playAudioMSFSupport: %s\n",
				   parm) > 0) {
				app_data.playmsf_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "playAudioTISupport: %s\n",
				   parm) > 0) {
				app_data.playti_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "loadSupport: %s\n",
				   parm) > 0) {
				app_data.load_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "ejectSupport: %s\n",
				   parm) > 0) {
				app_data.eject_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "modeSenseSetDBD: %s\n",
				   parm) > 0) {
				app_data.msen_dbd = stob(parm);
				continue;
			}
			if (sscanf(buf, "volumeControlSupport: %s\n",
				   parm) > 0) {
				app_data.mselvol_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "balanceControlSupport: %s\n",
				   parm) > 0) {
				app_data.balance_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "channelRouteSupport: %s\n",
				   parm) > 0) {
				app_data.chroute_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "pauseResumeSupport: %s\n",
				   parm) > 0) {
				app_data.pause_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "caddyLockSupport: %s\n",
				   parm) > 0) {
				app_data.caddylock_supp = stob(parm);
				continue;
			}
			if (sscanf(buf, "curposFormat: %s\n",
				   parm) > 0) {
				app_data.curpos_fmt = stob(parm);
				continue;
			}
			if (sscanf(buf, "noTURWhenPlaying: %s\n",
				   parm) > 0) {
				app_data.play_notur = stob(parm);
				continue;
			}
		}

		/* These are general parameters that can be
		 * changed by the user.
		 */
		if (sscanf(buf, "volumeControlTaper: %s\n", parm) > 0) {
			app_data.vol_taper = atoi(parm);
			continue;
		}
		if (sscanf(buf, "channelRoute: %s\n", parm) > 0) {
			app_data.ch_route = atoi(parm);
			continue;
		}
		if (sscanf(buf, "spinDownOnLoad: %s\n", parm) > 0) {
			app_data.load_spindown = stob(parm);
			continue;
		}
		if (sscanf(buf, "playOnLoad: %s\n", parm) > 0) {
			app_data.load_play = stob(parm);
			continue;
		}
		if (sscanf(buf, "ejectOnDone: %s\n", parm) > 0) {
			app_data.done_eject = stob(parm);
			continue;
		}
		if (sscanf(buf, "ejectOnExit: %s\n", parm) > 0) {
			app_data.exit_eject = stob(parm);
			continue;
		}
		if (sscanf(buf, "stopOnExit: %s\n", parm) > 0) {
			app_data.exit_stop = stob(parm);
			continue;
		}
		if (sscanf(buf, "exitOnEject: %s\n", parm) > 0) {
			app_data.eject_exit = stob(parm);
			continue;
		}
		if (sscanf(buf, "closeOnEject: %s\n", parm) > 0) {
			app_data.eject_close = stob(parm);
			continue;
		}
		if (sscanf(buf, "caddyLock: %s\n", parm) > 0) {
			app_data.caddy_lock = stob(parm);
			continue;
		}
	}

	fclose(fp);

	if (!priv) {
		/* If the drive does not support software eject, then we
		 * can't lock the caddy.
		 */
		if (!app_data.eject_supp) {
			app_data.caddylock_supp = FALSE;
			app_data.done_eject = FALSE;
			app_data.exit_eject = FALSE;
		}

		/* playOnLoad overrides spinDownOnLoad */
		if (app_data.load_play)
			app_data.load_spindown = FALSE;

		/* If the drive does not support locking the caddy, don't
		 * attempt to lock it.
		 */
		if (!app_data.caddylock_supp)
			app_data.caddy_lock = FALSE;

		/* If the drive does not support software volume
		 * control, then it can't support the balance
		 * control either.  Also, force the volume control
		 * taper selector to the linear position.
		 */
		if (!app_data.mselvol_supp) {
			app_data.balance_supp = FALSE;
			app_data.vol_taper = 0;
		}

		/* If the drive does not support channel routing,
		 * force the channel routing setting to normal.
		 */
		if (!app_data.chroute_supp)
			app_data.ch_route = 0;
	}
}


/*
 * dpy_time_blink
 *	Make the time indicator region of the main window blink.
 *	This is used when the disc is paused.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
STATIC void
dpy_time_blink(curstat_t *s)
{
	static bool_t	bstate = TRUE;

	if (bstate) {
		tm_blinkid = cd_timeout(
			app_data.blinkoff_interval,
			dpy_time_blink,
			(byte_t *) s
		);
		dpy_time(s, TRUE);
	}
	else {
		tm_blinkid = cd_timeout(
			app_data.blinkon_interval,
			dpy_time_blink,
			(byte_t *) s
		);
		dpy_time(s, FALSE);
	}
	bstate = !bstate;
}


/*
 * dpy_ab_blink
 *	Make the a->b indicator of the main window blink.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
STATIC void
dpy_ab_blink(curstat_t *s)
{
	static bool_t	bstate = TRUE;

	if (bstate) {
		ab_blinkid = cd_timeout(
			app_data.blinkoff_interval,
			dpy_ab_blink,
			(byte_t *) s
		);
		dpy_playmode(s, TRUE);
	}
	else {
		ab_blinkid = cd_timeout(
			app_data.blinkon_interval,
			dpy_ab_blink,
			(byte_t *) s
		);
		dpy_playmode(s, FALSE);
	}
	bstate = !bstate;
}


/*
 * dpy_keypad_ind
 *	Update the digital track number indicator on the keypad window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dpy_keypad_ind(curstat_t *s)
{
	char		str[16],
			trk[8],
			time[8];
	byte_t		min,
			sec,
			frame;
	XmString	xs;
	static char	prevstr[11];

	if (!XtIsManaged(widgets.keypad.form))
		return;

	if (keystr[0] == '\0') {
		if (warp_busy) {
			blktomsf(warp_offset, &min, &sec, &frame, 0);
			sprintf(trk, "%02u", s->cur_trk);
			sprintf(time, "+%02u:%02u", min, sec);
		}
		else if (curtrk_pos(s) < 0) {
			strcpy(trk, "--");
			strcpy(time, " --:--");
		}
		else if (curtrk_type(s) == TYP_DATA) {
			sprintf(trk, "%02u", s->cur_trk);
			strcpy(time, " --:--");
		}
		else {
			blktomsf(s->cur_trk_addr, &min, &sec, &frame, 0);
			sprintf(trk, "%02u", s->cur_trk);
			sprintf(time, "+%02u:%02u", min, sec);
		}
	}
	else {
		blktomsf(warp_offset, &min, &sec, &frame, 0);
		sprintf(trk, "%02u", atoi(keystr));
		sprintf(time, "+%02u:%02u", min, sec);
	}

	sprintf(str, "%s  %s", trk, time);

	if (strcmp(str, prevstr) == 0) {
		/* No change */
		return;
	}

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.keypad.keypad_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);
	strcpy(prevstr, str);
}


/*
 * dpy_warp
 *	Update the warp slider position.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dpy_warp(curstat_t *s)
{
	int	i;

	if (!XtIsManaged(widgets.keypad.form) || warp_busy)
		return;

	if ((i = curtrk_pos(s)) < 0 || curtrk_type(s) == TYP_DATA)
		set_warp_slider(0, TRUE);
	else
		set_warp_slider(unscale_warp(s, i, s->cur_trk_addr), TRUE);
}


/*
 * set_btn_color
 *	Set the label color of a pushbutton widget
 *
 * Args:
 *	w - The pushbutton widget.
 *	px - The label pixmap, if applicable.
 *	color - The pixel value of the desired color.
 *
 * Return:
 *	Nothing.
 */
STATIC void
set_btn_color(Widget w, Pixmap px, Pixel color)
{
	unsigned char	labtype;

	XtVaGetValues(w, XmNlabelType, &labtype, NULL);

	if (labtype == XmPIXMAP)
		XtVaSetValues(w, XmNlabelPixmap, px, NULL);
	else
		XtVaSetValues(w, XmNforeground, color, NULL);
}


/*
 * set_scale_color
 *	Set the indicator color of a scale widget
 *
 * Args:
 *	w - The scale widget.
 *	color - The pixel value of the desired color.
 *
 * Return:
 *	Nothing.
 */
STATIC void
set_scale_color(Widget w, Pixel color)
{
	XtVaSetValues(w, XmNforeground, color, NULL);
}


/***********************
 *   public routines   *
 ***********************/


/*
 * curtrk_pos
 *	Return the trkinfo table offset location of the current playing
 *	CD track.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Integer offset into the trkinfo table, or -1 if not currently
 *	playing audio.
 */
int
curtrk_pos(curstat_t *s)
{
	int	i;

	if ((int) s->cur_trk <= 0)
		return -1;

	i = (int) s->cur_trk - 1;

	if (s->trkinfo[i].trkno == s->cur_trk)
		return (i);

	for (i = 0; i < MAXTRACK; i++) {
		if (s->trkinfo[i].trkno == s->cur_trk)
			return (i);
	}
	return -1;
}


/*
 * curprog_pos
 *	Return an integer representing the position of the current
 *	program or shuffle mode playing order (0 = first, 1 = second, ...).
 *	This routine should be used only when in program or shuffle play
 *	mode.
 *
 * Arg:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	An integer representing the position of the current program
 *	or shuffle mode playing order, or -1 if not in the appropriate mode.
 */
int
curprog_pos(curstat_t *s)
{
	return ((int) s->playorder[s->prog_cnt]);
}


/*
 * curtrk_type
 *	Return the track type of the currently playing track.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	TYP_AUDIO or TYP_DATA.
 */
byte_t
curtrk_type(curstat_t *s)
{
	sword32_t	i;

	if ((i = curtrk_pos(s)) >= 0)
		return (s->trkinfo[i].type);

	return TYP_AUDIO;
}


/*
 * dpy_track
 *	Update the track number display region of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_track(curstat_t *s)
{
	XmString	xs;
	char		str[4];
	static char	prev[4] = { '\0' };
	static int	sav_trk = -1;


	if (s->cur_trk != sav_trk) {
		/* Update database/program window current track display */
		dbprog_curtrkupd(s);
		/* Update main window track title display */
		dpy_ttitle(s);
		/* Update the keypad indicator */
		dpy_keypad_ind(s);
		/* Update warp slider */
		dpy_warp(s);
	}

	sav_trk = s->cur_trk;

	if (s->cur_trk <= 0 || s->mode == M_NODISC)
		strcpy(str, "--");
	else if (s->time_dpy == T_REMAIN_DISC) {
		if (s->shuffle || s->program)
			sprintf(str, "-%u", s->prog_tot - s->prog_cnt);
		else
			sprintf(str, "-%u", s->tot_trks - curtrk_pos(s) - 1);
	}
	else
		sprintf(str, "%02u", s->cur_trk);

	if (strcmp(str, prev) == 0)
		/* No change, just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.track_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);
}


/*
 * dpy_index
 *	Update the index number display region of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_index(curstat_t *s)
{
	XmString	xs;
	char		str[4];
	static char	prev[4] = { '\0' };

	if (s->cur_idx <= 0 || s->mode == M_NODISC || s->mode == M_STOP)
		strcpy(str, "--");
	else
		sprintf(str, "%02u", s->cur_idx);

	if (strcmp(str, prev) == 0)
		/* No change, just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.index_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);
}


/*
 * dpy_time
 *	Update the time display region of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	blank - Whether the display region should be blanked.
 *
 * Return:
 *	Nothing
 */
void
dpy_time(curstat_t *s, bool_t blank)
{
	sword32_t	time_sec;
	XmString	xs;
	char		str[12];
	static char	prev[12];

	if (blank)
		str[0] = '\0';
	else if (s->mode == M_NODISC) {
		if (devbusy)
			strcpy(str, app_data.str_busy);
		else
			strcpy(str, app_data.str_nodisc);
	}
	else if (s->mode == M_STOP)
		strcpy(str, " --:--");
	else if (curtrk_type(s) == TYP_DATA)
		strcpy(str, app_data.str_data);
	else {
		switch (s->time_dpy) {
		case T_ELAPSED:
			sprintf(str, "%s%02u:%02u",
				(s->cur_idx == 0) ? "-" : "+",
				s->cur_trk_min,
				s->cur_trk_sec);
			break;

		case T_REMAIN_TRACK:
			time_sec = track_rtime(s);

			sprintf(str, "-%02u:%02u",
				time_sec / 60, time_sec % 60);
			break;

		case T_REMAIN_DISC:
			if (s->shuffle || s->program) {
				if (s->cur_idx == 0) {
					strcpy(str, " --:--");
					break;
				}
				else
					time_sec = disc_rtime_prog(s);
			}
			else
				time_sec = disc_rtime_norm(s);

			sprintf(str, "-%02u:%02u",
				time_sec / 60, time_sec % 60);
			break;

		default:
			strcpy(str, "??:??");
			break;
		}
	}

	if (strcmp(str, prev) == 0)
		/* No change: just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.time_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);
	strcpy(prev, str);

	/* Update the keypad indicator */
	dpy_keypad_ind(s);
	/* Update warp slider */
	dpy_warp(s);
}


/*
 * dpy_dtitle
 *	Update the disc title display region of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_dtitle(curstat_t *s)
{
	XmString	xs;
	char		str[TITLEIND_LEN];
	static char	prev[TITLEIND_LEN];

	if (!XtIsManaged(widgets.main.dtitle_ind))
		return;

	strncpy(str, dbprog_curdtitle(s), TITLEIND_LEN);
	str[TITLEIND_LEN - 1] = '\0';

	if (strcmp(str, prev) == 0)
		/* No change: just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.dtitle_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);
}


/*
 * dpy_ttitle
 *	Update the track title display region of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_ttitle(curstat_t *s)
{
	XmString	xs;
	char		str[TITLEIND_LEN];
	static char	prev[TITLEIND_LEN];

	if (!XtIsManaged(widgets.main.ttitle_ind))
		return;

	strncpy(str, dbprog_curttitle(s), TITLEIND_LEN);
	str[TITLEIND_LEN - 1] = '\0';

	if (strcmp(str, prev) == 0)
		/* No change: just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.ttitle_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);
}


/*
 * dpy_rptcnt
 *	Update the repeat count indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_rptcnt(curstat_t *s)
{
	XmString		xs;
	char			str[10];
	static char		prevstr[10];

	if (s->repeat && s->mode == M_PLAY)
		sprintf(str, "%u", s->rptcnt);
	else
		strcpy(str, "-");

	if (strcmp(str, prevstr) == 0)
		/* No change */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.rptcnt_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prevstr, str);
}


/*
 * dpy_dbmode
 *	Update the cddb indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_dbmode(curstat_t *s)
{
	String		str;
	XmString	xs;
	static bool_t	first = TRUE,
			prev = FALSE;

	if (!first && prev == s->cddb)
		/* No change: just return */
		return;

	first = FALSE;

	if (s->cddb)
		str = app_data.str_dbmode;
	else
		str = "";

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.dbmode_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	prev = s->cddb;
}


/*
 * dpy_progmode
 *	Update the prog indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_progmode(curstat_t *s)
{
	XmString	xs;
	bool_t		state;
	static bool_t	first = TRUE,
			prev = FALSE;

	state = (bool_t) (s->prog_tot > 0 && !s->shuffle);
	if (!first && state == prev)
		/* No change: just return */
		return;

	first = FALSE;

	xs = XmStringCreateSimple(state ? app_data.str_progmode : "");

	XtVaSetValues(
		widgets.main.progmode_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	prev = state;
}


/*
 * dpy_timemode
 *	Update the time mode indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_timemode(curstat_t *s)
{
	String		str;
	XmString	xs;
	static byte_t	prev = 0xff;

	if (prev == s->time_dpy)
		/* No change: just return */
		return;

	switch (s->time_dpy) {
	case T_ELAPSED:
		str = app_data.str_elapse;
		break;

	case T_REMAIN_TRACK:
		str = app_data.str_remaintrk;
		break;

	case T_REMAIN_DISC:
		str = app_data.str_remaindisc;
		break;
	}

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.timemode_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	prev = s->time_dpy;
}


/*
 * dpy_playmode
 *	Update the play mode indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_playmode(curstat_t *s, bool_t blank)
{
	char		*str;
	XmString	xs;
	static char	prev[10];

	if (blank)
		str = "";
	else {
		switch (s->mode) {
		case M_PLAY:
			str = app_data.str_play;
			break;
		case M_PAUSE:
			str = app_data.str_pause;
			break;
		case M_STOP:
			str = app_data.str_ready;
			break;
		case M_A:
			str = "a->?";
			break;
		case M_AB:
			str = "a->b";
			break;
		case M_SAMPLE:
			str = app_data.str_sample;
			break;
		default:
			str = "";
			break;
		}
	}

	if (strcmp(prev, str) == 0)
		/* No change: just return */
		return;

	xs = XmStringCreateSimple(str);

	XtVaSetValues(
		widgets.main.playmode_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);

	if (s->mode == M_A)
		cd_ab_blink(s, TRUE);
	else
		cd_ab_blink(s, FALSE);
}


/*
 * dpy_all
 *	Update all indicator of the main window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing
 */
void
dpy_all(curstat_t *s)
{
	dpy_track(s);
	dpy_index(s);
	dpy_time(s, FALSE);
	dpy_dtitle(s);
	dpy_rptcnt(s);
	dpy_dbmode(s);
	dpy_progmode(s);
	dpy_timemode(s);
	dpy_playmode(s, FALSE);
}


/*
 * reset_curstat
 *	Reset the curstat_t structure to initial defaults.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	clear_toc - Whether the trkinfo CD table-of-contents 
 *		should be cleared.
 *
 * Return:
 *	Nothing.
 */
void
reset_curstat(curstat_t *s, bool_t clear_toc)
{
	sword32_t	i;
	static bool_t	first_time = TRUE;

	s->cur_trk = s->cur_idx = -1;
	s->cur_tot_min = s->cur_tot_sec = s->cur_tot_frame = 0;
	s->cur_trk_min = s->cur_trk_sec = s->cur_trk_frame = 0;
	s->cur_tot_addr = s->cur_trk_addr = 0;
	s->sav_iaddr = 0;
	s->prog_cnt = 0;
	s->program = 0;
	s->rptcnt = 0;

	if (clear_toc) {
		s->mode = M_NODISC;
		s->first_trk = s->last_trk = -1;
		s->tot_min = s->tot_sec = 0;
		s->tot_trks = 0;
		s->tot_addr = 0;
		s->prog_tot = 0;

		for (i = 0; i < MAXTRACK; i++) {
			s->trkinfo[i].trkno = -1;
			s->trkinfo[i].min = 0;
			s->trkinfo[i].sec = 0;
			s->trkinfo[i].frame = 0;
			s->trkinfo[i].addr = 0;
			s->playorder[i] = -1;
		}
	}

	if (first_time) {
		/* These are to be initialized only once */
		first_time = FALSE;

		s->time_dpy = T_ELAPSED;
		s->repeat = s->shuffle = FALSE;
		s->cddb = FALSE;
		s->level = 0;
		s->caddy_lock = FALSE;
		s->vendor[0] = '\0';
		s->prod[0] = '\0';
		s->revnum[0] = '\0';
	}
}


/*
 * reset_shuffle
 *	Recompute a new shuffle play sequence.  Updates the playorder
 *	table in the curstat_t structure.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
reset_shuffle(curstat_t *s)
{
	sword32_t	i,
			j,
			n;

	srand((unsigned) time(NULL));
	s->prog_cnt = 0;
	s->prog_tot = s->tot_trks;

	for (i = 0; i < MAXTRACK; i++) {
		if (i >= (int) s->prog_tot) {
			s->playorder[i] = -1;
			continue;
		}

		do {
			n = rand() % (int) s->prog_tot;
			for (j = 0; j < i; j++) {
				if (n == s->playorder[j])
					break;
			}
		} while (j < i);

		s->playorder[i] = n;
	}


	if (app_data.debug) {
		fprintf(errfp, "\nShuffle tracks: ");

		for (i = 0; i < (int) s->prog_tot; i++)
			fprintf(errfp, "%d ",
				s->trkinfo[s->playorder[i]].trkno);

		fprintf(errfp, "\n");
	}
}


/*
 * set_lock_btn
 *	Set the lock button state
 *
 * Args:
 *	state - TRUE=in, FALSE=out
 *
 * Return:
 *	Nothing.
 */
void
set_lock_btn(bool_t state)
{
	XmToggleButtonSetState(
		widgets.main.lock_btn, (Boolean) state, False
	);
}


/*
 * set_repeat_btn
 *	Set the repeat button state
 *
 * Args:
 *	state - TRUE=in, FALSE=out
 *
 * Return:
 *	Nothing.
 */
void
set_repeat_btn(bool_t state)
{
	XmToggleButtonSetState(
		widgets.main.repeat_btn, (Boolean) state, False
	);
}


/*
 * set_shuffle_btn
 *	Set the shuffle button state
 *
 * Args:
 *	state - TRUE=in, FALSE=out
 *
 * Return:
 *	Nothing.
 */
void
set_shuffle_btn(bool_t state)
{
	XmToggleButtonSetState(
		widgets.main.shuffle_btn, (Boolean) state, False
	);
}


/*
 * set_vol_slider
 *	Set the volume control slider position
 *
 * Args:
 *	val - The value setting.
 *
 * Return:
 *	Nothing.
 */
void
set_vol_slider(int val)
{
	/* Check bounds */
	if (val > 100)
		val = 100;
	if (val < 0)
		val = 0;

	XmScaleSetValue(widgets.main.level_scale, val);
}


/*
 * set_warp_slider
 *	Set the track warp slider position
 *
 * Args:
 *	val - The value setting.
 *	autoupd - This is an auto-update.
 *
 * Return:
 *	Nothing.
 */
void
set_warp_slider(int val, bool_t autoupd)
{
	if (autoupd && keystr[0] != '\0') {
		/* User using keypad: no updates */
		return;
	}

	/* Check bounds */
	if (val > 255)
		val = 255;
	if (val < 0)
		val = 0;

	pseudo_warp = TRUE;
	XmScaleSetValue(widgets.keypad.warp_scale, val);
	pseudo_warp = FALSE;
}


/*
 * set_bal_slider
 *	Set the balance control slider position
 *
 * Args:
 *	val - The value setting.
 *
 * Return:
 *	Nothing.
 */
void
set_bal_slider(int val)
{
	/* Check bounds */
	if (val > 50)
		val = 50;
	if (val < -50)
		val = -50;

	XmScaleSetValue(widgets.options.bal_scale, val);
}


/*
 * set_btn_lbltype
 *	Set the main window pushbuttons label type
 *
 * Args:
 *	type - BTN_PIXMAP or BTN_STRING.
 *
 * Return:
 *	Nothing.
 */
void
set_btn_lbltype(byte_t type)
{
	type = (type == BTN_STRING) ? XmSTRING : XmPIXMAP;

	XtVaSetValues(widgets.main.btnlbl_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.lock_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.repeat_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.shuffle_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.eject_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.poweroff_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.dbprog_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.help_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.options_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.time_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.ab_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.sample_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.keypad_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.playpause_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.stop_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.prevtrk_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.nexttrk_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.previdx_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.nextidx_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.rew_btn,
		XmNlabelType, type,
		NULL
	);
	XtVaSetValues(widgets.main.ff_btn,
		XmNlabelType, type,
		NULL
	);
}


/*
 * taper_vol
 *	Translate the volume level based on the configured taper
 *	characteristics.
 *
 * Args:
 *	v - The linear volume value.
 *
 * Return:
 *	The curved volume value.
 */
int
taper_vol(int v)
{
	switch (app_data.vol_taper) {
	case 1:
		/* squared taper */
		return (SQR(v) / MAX_VOL);
	case 2:
		/* inverse-squared taper */
		return (MAX_VOL - (SQR(MAX_VOL - v) / MAX_VOL));
	case 0:
	default:
		/* linear taper */
		return (v);
	}
	/*NOTREACHED*/
}


/*
 * untaper_vol
 *	Translate the volume level based on the configured taper
 *	characteristics.
 *
 * Args:
 *	v - The curved volume value.
 *
 * Return:
 *	The linear volume value.
 */
int
untaper_vol(int v)
{
	switch (app_data.vol_taper) {
	case 1:
		/* squared taper */
		return (isqrt(v) * 10);
	case 2:
		/* inverse-squared taper */
		return (MAX_VOL - isqrt(SQR(MAX_VOL) - (MAX_VOL * v)));
	case 0:
	default:
		/* linear taper */
		return (v);
	}
	/*NOTREACHED*/
}


/*
 * scale_vol
 *	Scale logical audio volume value (0-100) to an 8-bit value
 *	(0-0xff) range.
 *
 * Args:
 *	v - The logical volume value
 *
 * Return:
 *	The scaled volume value
 */
int
scale_vol(int v)
{
	/* Convert logical audio volume value to 8-bit volume */
	return ((v * (0xff - app_data.base_scsivol) / MAX_VOL) +
	        app_data.base_scsivol);
}


/*
 * unscale_vol
 *	Scale an 8-bit audio volume parameter value (0-0xff) to the
 *	logical volume value (0-100).
 *
 * Args:
 *	v - The 8-bit volume value
 *
 * Return:
 *	The logical volume value
 */
int
unscale_vol(int v)
{
	register int	val;

	/* Convert 8-bit audio volume value to logical volume */
	val = (v - app_data.base_scsivol) * MAX_VOL /
	      (0xff - app_data.base_scsivol);

	return ((val < 0) ? 0 : val);
}



/*
 * scale_warp
 *	Scale track warp value (0-255) to the number of CD logical audio
 *	blocks.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	pos - Track position.
 *	val - Warp value.
 *
 * Return:
 *	The number of CD logical audio blocks
 */
int
scale_warp(curstat_t *s, int pos, int val)
{
	int	n;

	n = val * (s->trkinfo[pos+1].addr - s->trkinfo[pos].addr) / 0xff;
	return ((n > 0) ? (n - 1) : n);
}


/*
 * unscale_warp
 *	Scale the number of CD logical audio blocks to track warp
 *	value (0-255).
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	pos - Track position.
 *	val - Number of logical audio blocks.
 *
 * Return:
 *	The warp value
 */
int
unscale_warp(curstat_t *s, int pos, int val)
{
	return (val * 0xff / (s->trkinfo[pos+1].addr - s->trkinfo[pos].addr));
}


/*
 * cd_timeout
 *	Alarm clock callback facility
 *
 * Args:
 *	msec - When msec milliseconds has elapsed, the callback
 *		occurs.
 *	handler - Pointer to the callback function.
 *	arg - An argument passed to the callback function.
 *
 * Return:
 *	Timeout ID.
 */
long
cd_timeout(word32_t msec, void (*handler)(), byte_t *arg)
{
	/* Note: This code assumes that sizeof(XtIntervalId) <= sizeof(long)
	 * If this is not true then cd_timeout/cd_untimeout will not work
	 * correctly.
	 */
	return ((long)
		XtAppAddTimeOut(
			XtWidgetToApplicationContext(widgets.toplevel),
			(unsigned long) msec,
			(XtTimerCallbackProc) handler,
			(XtPointer) arg
		)
	);
}


/*
 * cd_untimeout
 *	Cancel a pending alarm configured with cd_timeout.
 *
 * Args:
 *	id - The timeout ID
 *
 * Return:
 *	Nothing.
 */
void
cd_untimeout(long id)
{
	/* Note: This code assumes that sizeof(XtIntervalId) <= sizeof(long)
	 * If this is not true then cd_timeout/cd_untimeout will not work
	 * correctly.
	 */
	XtRemoveTimeOut((XtIntervalId) id);
}


/*
 * cd_beep
 *	Beep the workstation speaker.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
cd_beep(void)
{
	XBell(XtDisplay(widgets.toplevel), 50);
}


/*
 * cd_pause_blink
 *	Disable or enable the time indicator blinking.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	enable - TRUE: start blink, FALSE: stop blink
 *
 * Return:
 *	Nothing.
 */
void
cd_pause_blink(curstat_t *s, bool_t enable)
{
	static bool_t	blinking = FALSE;

	if (enable) {
		if (!blinking) {
			/* Start time display blink */
			blinking = TRUE;
			dpy_time_blink(s);
		}
	}
	else if (blinking) {
		/* Stop time display blink */
		cd_untimeout(tm_blinkid);

		tm_blinkid = -1;
		blinking = FALSE;
	}
}


/*
 * cd_ab_blink
 *	Disable or enable the a->b indicator blinking.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	enable - TRUE: start blink, FALSE: stop blink
 *
 * Return:
 *	Nothing.
 */
void
cd_ab_blink(curstat_t *s, bool_t enable)
{
	static bool_t	blinking = FALSE;

	if (enable) {
		if (!blinking) {
			/* Start A->B display blink */
			blinking = TRUE;
			dpy_ab_blink(s);
		}
	}
	else if (blinking) {
		/* Stop A->B display blink */
		cd_untimeout(ab_blinkid);

		ab_blinkid = -1;
		blinking = FALSE;
	}
}


/*
 * cd_info_popup
 *	Pop up the information message dialog box.
 *
 * Args:
 *	title - The title bar text string.
 *	msg - The information message text string.
 *
 * Return:
 *	Nothing.
 */
void
cd_info_popup(char *title, char *msg)
{
	XmString	xs;

	if (!popup_ok) {
		fprintf(errfp, "%s %s:\n%s\n", PROGNAME, title, msg);
		return;
	}

	/* Set the dialog box title */
	xs = XmStringCreateSimple(title);
	XtVaSetValues(widgets.dialog.info, XmNdialogTitle, xs, NULL);
	XmStringFree(xs);

	/* Set the dialog box message */
	xs = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(widgets.dialog.info, XmNmessageString, xs, NULL);
	XmStringFree(xs);

	/* Pop up the info dialog */
	if (!XtIsManaged(widgets.dialog.info))
		XtManageChild(widgets.dialog.info);
}


/*
 * cd_warning_popup
 *	Pop up the warning message dialog box.
 *
 * Args:
 *	title - The title bar text string.
 *	msg - The warning message text string.
 *
 * Return:
 *	Nothing.
 */
void
cd_warning_popup(char *title, char *msg)
{
	XmString	xs;

	if (!popup_ok) {
		fprintf(errfp, "%s %s:\n%s\n", PROGNAME, title, msg);
		return;
	}

	/* Set the dialog box title */
	xs = XmStringCreateSimple(title);
	XtVaSetValues(widgets.dialog.warning, XmNdialogTitle, xs, NULL);
	XmStringFree(xs);

	/* Set the dialog box message */
	xs = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(widgets.dialog.warning, XmNmessageString, xs, NULL);
	XmStringFree(xs);

	/* Pop up the warning dialog */
	if (!XtIsManaged(widgets.dialog.warning))
		XtManageChild(widgets.dialog.warning);
}


/*
 * cd_fatal_popup
 *	Pop up the fatal error message dialog box.
 *
 * Args:
 *	title - The title bar text string.
 *	msg - The fatal error message text string.
 *
 * Return:
 *	Nothing.
 */
void
cd_fatal_popup(char *title, char *msg)
{
	XmString	xs;

	if (!popup_ok) {
		fprintf(errfp, "%s %s:\n%s\n", PROGNAME, title, msg);
		exit(1);
	}

	/* Make sure that the cursor is normal */
	cd_busycurs(FALSE);

	if (!XtIsManaged(widgets.dialog.fatal)) {
		/* Set the dialog box title */
		xs = XmStringCreateSimple(title);
		XtVaSetValues(widgets.dialog.fatal, XmNdialogTitle, xs, NULL);
		XmStringFree(xs);

		/* Set the dialog box message */
		xs = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);
		XtVaSetValues(widgets.dialog.fatal, XmNmessageString, xs, NULL);
		XmStringFree(xs);

		/* Pop up the error dialog */
		XtManageChild(widgets.dialog.fatal);
	}
}


/*
 * cd_confirm_popup
 *	Pop up the user-confirmation message dialog box.
 *
 * Args:
 *	title - The title bar text string.
 *	msg - The fatal error message text string.
 *	f_ok - Pointer to the callback function if user selects OK
 *	a_ok - Argument passed to f_ok
 *	f_cancel - Pointer to the callback function if user selects Cancel
 *	a_cancel - Argument passed to f_cancel
 *
 * Return:
 *	Nothing.
 */
void
cd_confirm_popup(
	char *title,
	char *msg,
	XtCallbackProc f_ok,
	XtPointer a_ok,
	XtCallbackProc f_cancel,
	XtPointer a_cancel
)
{
	XmString	xs;
	Widget		ok_btn,
			cancel_btn;
	static cbinfo_t	ok_cbinfo,
			cancel_cbinfo;

	if (!popup_ok)
		/* Not allowed */
		return;

	/* Set the dialog box title */
	xs = XmStringCreateSimple(title);
	XtVaSetValues(widgets.dialog.confirm, XmNdialogTitle, xs, NULL);
	XmStringFree(xs);

	/* Set the dialog box message */
	xs = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(widgets.dialog.confirm, XmNmessageString, xs, NULL);
	XmStringFree(xs);

	/* Add callbacks */
	ok_btn = XmMessageBoxGetChild(
		widgets.dialog.confirm,
		XmDIALOG_OK_BUTTON
	);
	cancel_btn = XmMessageBoxGetChild(
		widgets.dialog.confirm,
		XmDIALOG_CANCEL_BUTTON
	);

	ok_cbinfo.widget = ok_btn;
	ok_cbinfo.type = XmNactivateCallback;
	ok_cbinfo.func = f_ok;
	ok_cbinfo.data = a_ok;
	cancel_cbinfo.widget = cancel_btn;
	cancel_cbinfo.type = XmNactivateCallback;
	cancel_cbinfo.func = f_cancel;
	cancel_cbinfo.data = a_cancel;
	
	if (f_ok != NULL) {
		XtAddCallback(
			ok_btn,
			XmNactivateCallback,
			f_ok,
			a_ok
		);

		XtAddCallback(
			ok_btn,
			XmNactivateCallback,
			(XtCallbackProc) cd_rmcallback,
			(XtPointer) &ok_cbinfo
		);

		XtAddCallback(
			cancel_btn,
			XmNactivateCallback,
			(XtCallbackProc) cd_rmcallback,
			(XtPointer) &ok_cbinfo
		);
	}

	if (f_cancel != NULL) {
		XtAddCallback(
			cancel_btn,
			XmNactivateCallback,
			f_cancel,
			a_cancel
		);

		XtAddCallback(
			cancel_btn,
			XmNactivateCallback,
			(XtCallbackProc) cd_rmcallback,
			(XtPointer) &cancel_cbinfo
		);

		XtAddCallback(
			ok_btn,
			XmNactivateCallback,
			(XtCallbackProc) cd_rmcallback,
			(XtPointer) &cancel_cbinfo
		);
	}

	/* Pop up the error dialog */
	if (!XtIsManaged(widgets.dialog.confirm))
		XtManageChild(widgets.dialog.confirm);
}


/*
 * cd_init
 *	Top level function that initializes all subsystems.  Used on
 *	program startup.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
cd_init(curstat_t *s)
{
	int		i;
	char		*cp,
			*path,
			titlestr[STR_BUF_SZ],
			str[FILE_PATH_SZ + 2];
	XmString	xs;


	DBGPRN(errfp, "XMCD v%s%s PL%d DEBUG MODE\n",
	       VERSION, VERSION_EXT, PATCHLEVEL);

	/* Initialize libutil */
	util_init();

	if ((cp = getenv("XMCD_LIBDIR")) != NULL) {
		app_data.libdir = (char *) MEM_ALLOC(strlen(cp) + 1);
		if (app_data.libdir == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
		}

		strcpy(app_data.libdir, cp);
	}

	if (app_data.libdir == NULL || app_data.libdir[0] == '\0') {
		/* No library directory specified */
		if (di_isdemo()) {
			/* Demo mode: just fake it */
			app_data.libdir = ".";
		}
		else {
			/* Real application: this is a fatal error */
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_libdirerr
			);
		}
	}

	/* Get system common configuration parameters */
	sprintf(str, "%s/config/common.cfg", app_data.libdir);
	common_parminit(str, TRUE);

	/* Get user common configuration parameters */
	sprintf(str, "%s/.xmcdcfg/common.cfg", homedir(get_ouid()));
	common_parminit(str, FALSE);

	/* Sanity check */
	if (app_data.max_dbdirs <= 0 || app_data.max_dbdirs > 100)
		cd_fatal_popup(app_data.str_fatal, app_data.str_dbdirserr);

	/* Allocate memory for the database directories string pointers array */
	dbdirs = (char **)(void *) MEM_ALLOC(
		app_data.max_dbdirs * sizeof(char *)
	);
	if (dbdirs == NULL)
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);

	if ((cp = getenv("XMCD_DBPATH")) != NULL) {
		app_data.dbdir = (char *) MEM_ALLOC(strlen(cp) + 1);
		if (app_data.dbdir == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
		}

		strcpy(app_data.dbdir, cp);
	}

	/* Create the global array of strings each of which is a
	 * path to a CD database directory.
	 */

	i = 0;
	if (app_data.dbdir == NULL || app_data.dbdir[0] == '\0')
		dbdirs[0] = NULL;
	else {
		for (path = app_data.dbdir;
		     (cp = strchr(path, DBPATH_SEPCHAR)) != NULL &&
		     i < app_data.max_dbdirs - 1;
		     path = cp + 1, i++) {
			*cp = '\0';

			if (path[0] == '/') {
				dbdirs[i] = (char *) MEM_ALLOC(
					strlen(path) + 1
				);
			}
			else if (path[0] == '~') {
				dbdirs[i] = (char *) MEM_ALLOC(
					strlen(path) + 128
				);
			}
			else {
				dbdirs[i] = (char *) MEM_ALLOC(
					strlen(path) +
					strlen(app_data.libdir) + 7
				);
			}

			if (dbdirs[i] == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
			}

			if (path[0] == '/') {
				/* Absolute path name specified */
				strcpy(dbdirs[i], path);
			}
			else if (path[0] == '~') {
				/* Perform tilde expansion a la [ck]sh */
				if (path[1] == '/') {
					sprintf(dbdirs[i], "%s%s",
						homedir(get_ouid()), &path[1]);
				}
				else if (path[1] == '\0') {
					strcpy(dbdirs[i], homedir(get_ouid()));
				}
				else {
					char	*cp1;

					cp1 = strchr(path, '/');
					if (cp1 == NULL) {
						strcpy(dbdirs[i],
						       uhomedir(&path[1]));
					}
					else {
						*cp1 = '\0';
						sprintf(dbdirs[i], "%s/%s",
							uhomedir(&path[1]),
							cp1+1);
					}
				}
			}
			else {
				/* Relative path name specified */
				sprintf(dbdirs[i], "%s/cddb/%s",
					app_data.libdir, path);
			}

			*cp = DBPATH_SEPCHAR;

			/* Add path to list in directory selector popup */
			sprintf(str, "  %s", dbdirs[i]);
			xs = XmStringCreateSimple(str);
			XmListAddItemUnselected(
				widgets.dirsel.dir_list,
				xs,
				i + 1
			);
			XmStringFree(xs);
		}

		if (cp != NULL && *cp == DBPATH_SEPCHAR)
			*cp = '\0';

		if (path[0] == '/')
			dbdirs[i] = (char *) MEM_ALLOC(strlen(path) + 1);
		else
			dbdirs[i] = (char *) MEM_ALLOC(
				strlen(path) + strlen(app_data.libdir) + 7
			);

		if (dbdirs[i] == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
		}
		if (path[0] == '/')
			strcpy(dbdirs[i], path);
		else
			sprintf(dbdirs[i], "%s/cddb/%s",
				app_data.libdir, path);

		/* Add path to list in directory selector popup */
		sprintf(str, "  %s", dbdirs[i]);
		xs = XmStringCreateSimple(str);
		XmListAddItemUnselected(widgets.dirsel.dir_list, xs, i + 1);
		XmStringFree(xs);
	}

	for (i++; i < app_data.max_dbdirs; i++)
		dbdirs[i] = NULL;

	lockfile[0] = '\0';

	/* Get system-wide device-specific configuration parameters */
	sprintf(str, "%s/config/%s",
		app_data.libdir, basename(app_data.device));
	devspec_parminit(str, TRUE);

	/* Get user device-specific configuration parameters */
	sprintf(str, "%s/.xmcdcfg/%s",
		homedir(get_ouid()), basename(app_data.device));
	devspec_parminit(str, FALSE);

	/* Initialize the database/program subsystem */
	dbprog_init(s);

	/* Initialize the CD interface subsystem */
	di_init(s);

	/* Set default options */
	cd_options_reset(widgets.options.reset_btn, (XtPointer) s, NULL);

	/* Set the main window title */
	sprintf(titlestr, "%s %d", app_data.main_title, app_data.devnum);
	XStoreName(
		XtDisplay(widgets.toplevel),
		XtWindow(widgets.toplevel),
		titlestr
	);
}


/*
 * cd_start
 *	Start up I/O to the CD player.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
cd_start(curstat_t *s)
{
	struct stat	stbuf;
	char		errmsg[ERR_BUF_SZ];

	/* Debug information */
	if (app_data.debug) {
		fprintf(errfp, "\ndevnum=%d\n",
			app_data.devnum);
		fprintf(errfp, "device=%s\n",
			app_data.device);
		fprintf(errfp, "libdir=%s\n",
			app_data.libdir);
		fprintf(errfp, "dbdir=%s\n",
			app_data.dbdir);
		fprintf(errfp, "maxDbdirs=%d\n",
			app_data.max_dbdirs);
		fprintf(errfp, "dbFileMode=%s\n",
			app_data.dbfile_mode);
		fprintf(errfp, "deviceInterfaceMethod=%d\n",
			app_data.di_method);
		fprintf(errfp, "statusPollInterval=%d\n",
			app_data.stat_interval);
		fprintf(errfp, "insertPollInterval=%d\n",
			app_data.ins_interval);
		fprintf(errfp, "previousThreshold=%d\n",
			app_data.prev_threshold);
		fprintf(errfp, "searchSkipBlocks=%d\n",
			app_data.skip_blks);
		fprintf(errfp, "searchPauseInterval=%d\n",
			app_data.skip_pause);
		fprintf(errfp, "searchSpeedUpCount=%d\n",
			app_data.skip_spdup);
		fprintf(errfp, "searchVolumePercent=%d\n",
			app_data.skip_vol);
		fprintf(errfp, "searchMinVolume=%d\n",
			app_data.skip_minvol);
		fprintf(errfp, "sampleBlocks=%d\n",
			app_data.sample_blks);
		fprintf(errfp, "minimumPlayBlocks=%d\n",
			app_data.min_playblks);
		fprintf(errfp, "displayBlinkOnInterval=%d\n",
			app_data.blinkon_interval);
		fprintf(errfp, "displayBlinkOffInterval=%d\n",
			app_data.blinkoff_interval);
		fprintf(errfp, "scsiAudioVolumeBase=%d\n",
			app_data.base_scsivol);
		fprintf(errfp, "volumeControlTaper=%d\n",
			app_data.vol_taper);
		fprintf(errfp, "channelRoute=%d\n",
			app_data.ch_route);
		fprintf(errfp, "driveVendorCode=%d\n",
			app_data.vendor_code);
		fprintf(errfp, "playAudio10Support=%d\n",
			app_data.play10_supp);
		fprintf(errfp, "playAudio12Support=%d\n",
			app_data.play12_supp);
		fprintf(errfp, "playAudioMSFSupport=%d\n",
			app_data.playmsf_supp);
		fprintf(errfp, "playAudioTISupport=%d\n",
			app_data.playti_supp);
		fprintf(errfp, "loadSupport=%d\n",
			app_data.load_supp);
		fprintf(errfp, "ejectSupport=%d\n",
			app_data.eject_supp);
		fprintf(errfp, "modeSenseSetDBD=%d\n",
			app_data.msen_dbd);
		fprintf(errfp, "volumeControlSupport=%d\n",
			app_data.mselvol_supp);
		fprintf(errfp, "balanceControlSupport=%d\n",
			app_data.balance_supp);
		fprintf(errfp, "channelRouteSupport=%d\n",
			app_data.chroute_supp);
		fprintf(errfp, "pauseResumeSupport=%d\n",
			app_data.pause_supp);
		fprintf(errfp, "caddyLockSupport=%d\n",
			app_data.caddylock_supp);
		fprintf(errfp, "curposFormat=%d\n",
			app_data.curpos_fmt);
		fprintf(errfp, "noTURWhenPlaying=%d\n",
			app_data.play_notur);
		fprintf(errfp, "spinDownOnLoad=%d\n",
			app_data.load_spindown);
		fprintf(errfp, "ejectOnExit=%d\n",
			app_data.exit_eject);
		fprintf(errfp, "stopOnExit=%d\n",
			app_data.exit_stop);
		fprintf(errfp, "exitOnEject=%d\n",
			app_data.eject_exit);
		fprintf(errfp, "closeOnEject=%d\n",
			app_data.eject_close);
		fprintf(errfp, "caddyLock=%d\n",
			app_data.caddy_lock);
		fprintf(errfp, "solaris2VolumeManager=%d\n",
			app_data.sol2_volmgt);
		fprintf(errfp, "showScsiErrMsg=%d\n",
			app_data.scsierr_msg);
		fprintf(errfp, "mainShowFocus=%d\n",
			app_data.main_showfocus);

		fprintf(errfp, "\n");
	}

	/* Make temporary directory, if needed */
	sprintf(errmsg, app_data.str_tmpdirerr, TEMP_DIR);
	if (stat(TEMP_DIR, &stbuf) < 0) {
		if (errno == ENOENT) {
			int	omask;

			/* The permissions should be writable by all */
			omask = umask(0);
			if (mkdir(TEMP_DIR, 0777) < 0) {
				(void) umask(omask);
				cd_fatal_popup(app_data.str_fatal, errmsg);
				return;
			}
			(void) umask(omask);
		}
		else {
			cd_fatal_popup(app_data.str_fatal, errmsg);
			return;
		}
	}
	else if (!S_ISDIR(stbuf.st_mode)) {
		cd_fatal_popup(app_data.str_fatal, errmsg);
		return;
	}

	/* Allow popup dialogs from here on */
	popup_ok = TRUE;

	/* Start up I/O interface */
	di_start(s);
}


/*
 * cd_icon
 *	Main window iconification/deiconification handler.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	iconified - Whether the main window is iconified.
 *
 * Return:
 *	Nothing.
 */
void
cd_icon(curstat_t *s, bool_t iconified)
{
	di_icon(s, iconified);
}


/*
 * cd_halt
 *	Top level function to shut down all subsystems.  Used when
 *	closing the application.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
cd_halt(curstat_t *s)
{
	di_halt(s);
}


/*
 * cd_quit
 *	Close the application.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
cd_quit(curstat_t *s)
{
	XmAnyCallbackStruct	p;

	if (XtIsRealized(widgets.toplevel))
		XtUnmapWidget(widgets.toplevel);

	/* Shut down all xmcd subsystems */
	cd_halt(s);

	/* Uninstall current keyboard grabs */
	p.reason = XmCR_FOCUS;
	cd_form_focus_chg(
		widgets.toplevel,
		(XtPointer) widgets.toplevel,
		(XtPointer) &p
	);

	if (!devbusy && lockfile[0] != '\0')
		unlink(lockfile);

	exit_flag = TRUE;
}


/*
 * cd_devlock
 *	Create a lock to prevent another xmcd process from accessing
 *	the same CD-ROM device.
 *
 * Args:
 *	path - The lock file path name.
 *
 * Return:
 *	TRUE if the lock was successful.  If FALSE, then it indicates
 *	that another xmcd process currently has the lock.
 */
bool_t
cd_devlock(char *path)
{
	int		fd;
	pid_t		pid,
			mypid;
	char		buf[12];
	struct stat	stbuf;

	if (di_isdemo())
		return TRUE;	/* No locking needed in demo mode */

	if (stat(path, &stbuf) < 0)
		return FALSE;

	sprintf(lockfile, "%s/lock.%x", TEMP_DIR, stbuf.st_rdev);
	mypid = getpid();

	for (;;) {
		fd = open(lockfile, O_CREAT | O_EXCL | O_WRONLY);
		if (fd < 0) {
			if (errno == EEXIST) {
				if ((fd = open(lockfile, O_RDONLY)) < 0)
					return FALSE;

				if (read(fd, buf, 12) > 0)
					pid = (pid_t) atoi(buf);
				else {
					close(fd);
					return FALSE;
				}

				close(fd);

				if (pid == mypid)
					/* Our own lock */
					return TRUE;

				if (pid <= 0 ||
				    (kill(pid, 0) < 0 && errno == ESRCH)) {
					/* Pid died, steal its lockfile */
					unlink(lockfile);
				}
				else {
					/* Pid still running: clash */
					devbusy = TRUE;
					return FALSE;
				}
			}
			else
				return FALSE;
		}
		else {
			sprintf(buf, "%d\n", mypid);
			write(fd, buf, strlen(buf));

			close(fd);
			chmod(lockfile, 0644);

			devbusy = FALSE;
			return TRUE;
		}
	}
}


/*
 * cd_busycurs
 *	Enable/disable the watch cursor.
 *
 * Args:
 *	Boolean value indicating whether to enable or disable the watch
 *	cursor.
 *
 * Return:
 *	Nothing.
 */
void
cd_busycurs(bool_t busy)
{
	Display		*dpy = XtDisplay(widgets.toplevel);
	Window		win;
	static Cursor	wcur = (Cursor) 0;
	static bool_t	state = FALSE;

	if (wcur == (Cursor) 0)
		wcur = XCreateFontCursor(dpy, XC_watch);

	if (state == busy)
		return;

	state = busy;

	if (busy) {
		if ((win = XtWindow(widgets.main.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.keypad.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.options.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.dbprog.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.dbextd.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.dbextt.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.dirsel.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.linksel.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
		if ((win = XtWindow(widgets.help.form)) != (Window) 0)
			XDefineCursor(dpy, win, wcur);
	}
	else {
		if ((win = XtWindow(widgets.main.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.keypad.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.options.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.dbprog.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.dbextd.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.dbextt.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.dirsel.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.linksel.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
		if ((win = XtWindow(widgets.help.form)) != (Window) 0)
			XUndefineCursor(dpy, win);
	}
	XFlush(dpy);
}


/*
 * onsig
 *	Signal handler.  Causes the application to shut down gracefully.
 *
 * Args:
 *	sig - The signal number received.
 *
 * Return:
 *	Nothing.
 */
void
onsig(int sig)
{
	signal(sig, SIG_IGN);
	cd_quit(curstat_addr());
}


/**************** vv Callback routines vv ****************/

/*
 * cd_checkbox
 *	Main window checkbox callback function
 */
/*ARGSUSED*/
void
cd_checkbox(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmRowColumnCallbackStruct
			*p = (XmRowColumnCallbackStruct *)(void *) call_data;
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (p->reason != XmCR_ACTIVATE)
		return;

	DBGPRN(errfp, "\n* CHKBOX: ");

	if (p->widget == widgets.main.btnlbl_btn) {
		DBGPRN(errfp, "disp\n");

		btnlbl_state = !btnlbl_state;
		set_btn_lbltype((byte_t)
			(btnlbl_state ? BTN_STRING : BTN_PIXMAP)
		);
	}
	else if (p->widget == widgets.main.lock_btn) {
		DBGPRN(errfp, "lock\n");

		di_lock(s, (bool_t) !s->caddy_lock);
	}
	else if (p->widget == widgets.main.repeat_btn) {
		DBGPRN(errfp, "repeat\n");

		di_repeat(s, (bool_t) !s->repeat);
	}
	else if (p->widget == widgets.main.shuffle_btn) {
		DBGPRN(errfp, "shuffle\n");

		di_shuffle(s, (bool_t) !s->shuffle);
	}
}


/*
 * cd_load_eject
 *	Main window load/eject button callback function
 */
/*ARGSUSED*/
void
cd_load_eject(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	DBGPRN(errfp, "\n* LOAD_EJECT\n");

	if (searching) {
		cd_beep();
		return;
	}
	if (s->mode == M_PAUSE)
		cd_pause_blink(s, FALSE);

	di_load_eject(s);
}


/*
 * cd_poweroff
 *	Main window quit button callback function
 */
/*ARGSUSED*/
void
cd_poweroff(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* QUIT\n");

	cd_confirm_popup(
		app_data.str_confirm,
		app_data.str_quit,
		(XtCallbackProc) cd_exit,
		client_data,
		(XtCallbackProc) NULL,
		NULL
	);
}


/*
 * cd_dbprog
 *	Main window dbprog button callback function
 */
void
cd_dbprog(Widget w, XtPointer client_data, XtPointer call_data)
{
	static bool_t	first = TRUE;

	if (XtIsManaged(widgets.dbprog.form)) {
		/* Pop down the Database/Program window */
		dbprog_cancel(w, client_data, call_data);
		return;
	}

	/* Pop up the Database/Program window */
	dbprog_popup(w, client_data, call_data);

	if (first) {
		first = FALSE;
		XmProcessTraversal(
			widgets.dbprog.cancel_btn,
			XmTRAVERSE_CURRENT
		);
	}
}


/*
 * cd_time
 *	Main window time mode button callback function
 */
/*ARGSUSED*/
void
cd_time(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	switch (s->time_dpy) {
	case T_ELAPSED:
		s->time_dpy = T_REMAIN_TRACK;
		break;

	case T_REMAIN_TRACK:
		s->time_dpy = T_REMAIN_DISC;
		break;

	case T_REMAIN_DISC:
		s->time_dpy = T_ELAPSED;
		break;
	}

	dpy_timemode(s);
	dpy_track(s);
	dpy_time(s, FALSE);
}


/*
 * cd_ab
 *	Main window a->b mode button callback function
 */
/*ARGSUSED*/
void
cd_ab(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* A->B\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_ab((curstat_t *)(void *) client_data);
}


/*
 * cd_sample
 *	Main window sample mode button callback function
 */
/*ARGSUSED*/
void
cd_sample(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* SAMPLE\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_sample((curstat_t *)(void *) client_data);
}


/*
 * cd_level
 *	Main window volume control slider callback function
 */
/*ARGSUSED*/
void
cd_level(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScaleCallbackStruct
			*p = (XmScaleCallbackStruct *)(void *) call_data;

	DBGPRN(errfp, "\n* VOL\n");

	di_level(
		(curstat_t *)(void *) client_data,
		(byte_t) p->value,
		(bool_t) (p->reason != XmCR_VALUE_CHANGED)
	);
}


/*
 * cd_play_pause
 *	Main window play/pause button callback function
 */
/*ARGSUSED*/
void
cd_play_pause(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	DBGPRN(errfp, "\n* PLAY_PAUSE\n");

	if (searching) {
		cd_beep();
		return;
	}

	di_play_pause(s);

	switch (s->mode) {
	case M_PAUSE:
		cd_pause_blink(s, TRUE);
		break;
	case M_PLAY:
	case M_STOP:
	case M_A:
	case M_AB:
	case M_SAMPLE:
		cd_pause_blink(s, FALSE);

		cd_keypad_clear(w, client_data, NULL);
		warp_busy = 0;
		break;
	}
}


/*
 * cd_stop
 *	Main window stop button callback function
 */
/*ARGSUSED*/
void
cd_stop(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	DBGPRN(errfp, "\n* STOP\n");

	if (searching) {
		cd_beep();
		return;
	}
	if (s->mode == M_PAUSE)
		cd_pause_blink(s, FALSE);

	di_stop(s, TRUE);
}


/*
 * cd_prevtrk
 *	Main window prev track button callback function
 */
/*ARGSUSED*/
void
cd_prevtrk(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* PREVTRK\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_prevtrk((curstat_t *)(void *) client_data);
}


/*
 * cd_nexttrk
 *	Main window next track button callback function
 */
/*ARGSUSED*/
void
cd_nexttrk(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* NEXTTRK\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_nexttrk((curstat_t *)(void *) client_data);
}


/*
 * cd_previdx
 *	Main window prev index button callback function
 */
/*ARGSUSED*/
void
cd_previdx(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* PREVIDX\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_previdx((curstat_t *)(void *) client_data);
}


/*
 * cd_previdx
 *	Main window next index button callback function
 */
/*ARGSUSED*/
void
cd_nextidx(Widget w, XtPointer client_data, XtPointer call_data)
{
	DBGPRN(errfp, "\n* NEXTIDX\n");

	if (searching) {
		cd_beep();
		return;
	}
	di_nextidx((curstat_t *)(void *) client_data);
}


/*
 * cd_rew
 *	Main window search rewind button callback function
 */
/*ARGSUSED*/
void
cd_rew(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmPushButtonCallbackStruct
			*p = (XmPushButtonCallbackStruct *)(void *) call_data;
	curstat_t	*s = (curstat_t *)(void *) client_data;
	bool_t		start;
	static bool_t	rew_running = FALSE;

	if (p->reason == XmCR_ARM) {
		DBGPRN(errfp, "\n* REW: down\n");

		if (!rew_running) {
			if (searching) {
				/* Release running FF */
				XtCallActionProc(
					widgets.main.ff_btn,
					"Activate",
					p->event,
					NULL,
					0
				);
				XtCallActionProc(
					widgets.main.ff_btn,
					"Disarm",
					p->event,
					NULL,
					0
				);
			}

			rew_running = TRUE;
			searching = TRUE;
			start = TRUE;
		}
		else
			/* Already running REW */
			return;
	}
	else {
		DBGPRN(errfp, "\n* REW: up\n");

		if (rew_running) {
			rew_running = FALSE;
			searching = FALSE;
			start = FALSE;
		}
		else
			/* Not running REW */
			return;
	}

	di_rew(s, start);

	if (s->mode == M_PAUSE)
		cd_pause_blink(s, (bool_t) !start);
}


/*
 * cd_ff
 *	Main window search fast-forward button callback function
 */
/*ARGSUSED*/
void
cd_ff(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmPushButtonCallbackStruct
			*p = (XmPushButtonCallbackStruct *)(void *) call_data;
	curstat_t	*s = (curstat_t *)(void *) client_data;
	bool_t		start;
	static bool_t	ff_running = FALSE;

	if (p->reason == XmCR_ARM) {
		DBGPRN(errfp, "\n* FF: down\n");

		if (!ff_running) {
			if (searching) {
				/* Release running REW */
				XtCallActionProc(
					widgets.main.rew_btn,
					"Activate",
					p->event,
					NULL,
					0
				);
				XtCallActionProc(
					widgets.main.rew_btn,
					"Disarm",
					p->event,
					NULL,
					0
				);
			}

			ff_running = TRUE;
			searching = TRUE;
			start = TRUE;
		}
		else
			/* Already running FF */
			return;
	}
	else {
		DBGPRN(errfp, "\n* FF: up\n");

		if (ff_running) {
			ff_running = FALSE;
			searching = FALSE;
			start = FALSE;
		}
		else
			/* Not running FF */
			return;
	}

	di_ff(s, start);

	if (s->mode == M_PAUSE)
		cd_pause_blink(s, (bool_t) !start);
}


/*
 * cd_keypad_popup
 *	Main window keypad button callback function
 */
void
cd_keypad_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
	static bool_t	first = TRUE;

	if (XtIsManaged(widgets.keypad.form)) {
		/* Pop down keypad window */
		cd_keypad_popdown(w, client_data, call_data);
		return;
	}

	/* Pop up keypad window */
	XtManageChild(widgets.keypad.form);

	/* Reset keypad */
	cd_keypad_clear(w, client_data, NULL);

	/* Update warp slider */
	dpy_warp((curstat_t *)(void *) client_data);

	if (first) {
		first = FALSE;
		XmProcessTraversal(
			widgets.keypad.cancel_btn,
			XmTRAVERSE_CURRENT
		);
	}
}


/*
 * cd_keypad_popdown
 *	Keypad window popdown callback function
 */
/*ARGSUSED*/
void
cd_keypad_popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down keypad window */
	if (XtIsManaged(widgets.keypad.form))
		XtUnmanageChild(widgets.keypad.form);
}


/*
 * cd_keypad_num
 *	Keypad window number button callback function
 */
/*ARGSUSED*/
void
cd_keypad_num(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = curstat_addr();
	sword32_t	sav_cur_trk;
	char		tmpstr[2];

	/* The user entered a digit */
	if (strlen(keystr) >= sizeof(keystr) - 1) {
		cd_beep();
		return;
	}

	sprintf(tmpstr, "%u", (unsigned int) client_data);
	strcat(keystr, tmpstr);

	sav_cur_trk = s->cur_trk;
	s->cur_trk = (sword32_t) atoi(keystr);

	if (curtrk_pos(s) < 0) {
		/* Illegal track entered */
		cd_keypad_clear(w, (XtPointer) s, NULL);
		s->cur_trk = sav_cur_trk;

		cd_beep();
		return;
	}

	warp_offset = 0;
	set_warp_slider(0, FALSE);
	dpy_keypad_ind(s);
}


/*
 * cd_keypad_clear
 *	Keypad window clear button callback function
 */
/*ARGSUSED*/
void
cd_keypad_clear(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	/* Reset keypad */
	keystr[0] = '\0';

	/* Hack: if the third arg is NULL, then it's an internal
	 * call rather than a callback.  We want to set s->cur_trk
	 * to -1 only for callbacks, so that the keypad indicator
	 * display gets updated correctly.
	 */
	if (call_data != NULL)
		s->cur_trk = -1;

	warp_offset = 0;
	set_warp_slider(0, FALSE);
	dpy_keypad_ind(s);
}


/*
 * cd_keypad_enter
 *	Keypad window enter button callback function
 */
void
cd_keypad_enter(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	int		i;
	sword32_t	sav_cur_trk;
	bool_t		paused = FALSE;

	/* The user activated the Enter key */
	if (keystr[0] == '\0' && s->cur_trk < 0) {
		cd_beep();
		return;
	}

	if (s->shuffle) {
		/* Disable shuffle mode */
		di_shuffle(s, FALSE);
		set_shuffle_btn(FALSE);
	}

	if (s->prog_tot > 0 || !di_check_disc(s)) {
		/* Cannot go to a track while in program mode,
		 * or when the disc is not ready.
		 */
		cd_keypad_clear(w, client_data, NULL);
		cd_beep();
		return;
	}

	if (keystr[0] != '\0')
		s->cur_trk = (sword32_t) atoi(keystr);

	if ((i = curtrk_pos(s)) < 0) {
		cd_beep();
		return;
	}

	switch (s->mode) {
	case M_PAUSE:
		/* Mute sound */
		di_mute_on(s);
		paused = TRUE;

		/*FALLTHROUGH*/
	case M_PLAY:
	case M_A:
	case M_AB:
	case M_SAMPLE:
		sav_cur_trk = s->cur_trk;

		/* Set play status to stop */
		di_stop(s, FALSE);

		/* Restore s->cur_trk because di_stop resets it */
		s->cur_trk = sav_cur_trk;

		break;

	default:
		break;
	}

	s->cur_trk_addr = warp_offset;
	blktomsf(s->cur_trk_addr,
		 &s->cur_trk_min,
		 &s->cur_trk_sec,
		 &s->cur_trk_frame,
		 0
	);
	s->cur_tot_addr = s->trkinfo[i].addr + warp_offset;
	blktomsf(s->cur_tot_addr,
		 &s->cur_tot_min,
		 &s->cur_tot_sec,
		 &s->cur_tot_frame,
		 MSF_OFFSET(s)
	);

	/* Start playback at new position */
	cd_play_pause(w, client_data, call_data);

	if (paused) {
		/* This will cause the playback to pause */
		cd_play_pause(w, client_data, call_data);

		/* Restore sound */
		di_mute_off(s);
	}
}


/*
 * cd_warp
 *	Track warp function
 */
void
cd_warp(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	XmScaleCallbackStruct
			*p = (XmScaleCallbackStruct *)(void *) call_data;
	int		i;
	bool_t		paused = FALSE;

	if (pseudo_warp) {
		warp_busy = FALSE;
		return;
	}

	if (s->mode == M_NODISC) {
		warp_offset = 0;
		warp_busy = FALSE;
		set_warp_slider(0, FALSE);
		return;
	}

	/* Translate slider position to block offset */
	if (keystr[0] != '\0') {
		if (s->shuffle) {
			/* Disable shuffle mode */
			di_shuffle(s, FALSE);
			set_shuffle_btn(FALSE);
		}
		else if (s->prog_tot > 0){
			/* Don't allow changing tracks in program mode */
			cd_beep();
			warp_busy = FALSE;
			set_warp_slider(0, FALSE);
			cd_keypad_clear(w, client_data, NULL);
			return;
		}

		/* Use track number selected on keypad */
		s->cur_trk = atoi(keystr);
	}

	if ((i = curtrk_pos(s)) < 0) {
		warp_offset = 0;
		warp_busy = FALSE;
		set_warp_slider(0, FALSE);
		return;
	}

	warp_offset = (word32_t) scale_warp(s, i, p->value);

	if (p->reason == XmCR_VALUE_CHANGED) {
		DBGPRN(errfp, "\n* TRACK WARP\n");

		s->cur_trk_addr = warp_offset;
		blktomsf(s->cur_trk_addr,
			 &s->cur_trk_min,
			 &s->cur_trk_sec,
			 &s->cur_trk_frame,
			 0
		);
		s->cur_tot_addr = s->trkinfo[i].addr + warp_offset;
		blktomsf(s->cur_tot_addr,
			 &s->cur_tot_min,
			 &s->cur_tot_sec,
			 &s->cur_tot_frame,
			 MSF_OFFSET(s)
		);

		if (s->mode == M_STOP) {
			warp_busy = FALSE;
			return;
		}

		/* Start playback at new position */
		di_warp(s);

		cd_keypad_clear(w, client_data, NULL);
		warp_offset = 0;
		warp_busy = FALSE;

		/* Update display */
		dpy_track(s);
		dpy_index(s);
		dpy_time(s, FALSE);
	}
	else {
		warp_busy = TRUE;
	}

	dpy_keypad_ind(s);
}


/*
 * cd_options_popup
 *	Options window popup callback function
 */
/*ARGSUSED*/
void
cd_options_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
	static bool_t	first = TRUE;

	if (XtIsManaged(widgets.options.form)) {
		/* Pop down options window */
		cd_options_popdown(w, client_data, call_data);
		return;
	}

	/* Pop up keypad window */
	XtManageChild(widgets.options.form);

	if (first) {
		first = FALSE;
		XmProcessTraversal(
			widgets.options.ok_btn,
			XmTRAVERSE_CURRENT
		);
	}
}


/*
 * cd_options_popdown
 *	Options window popdown callback function
 */
/*ARGSUSED*/
void
cd_options_popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down options window */
	if (XtIsManaged(widgets.options.form))
		XtUnmanageChild(widgets.options.form);
}


/*
 * cd_options_reset
 *	Options window reset button callback function
 */
/*ARGSUSED*/
void
cd_options_reset(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	char		str[FILE_PATH_SZ + 2];

	if (call_data != NULL) {
		/* Re-read defaults */

		/* Get system-wide device-specific configuration parameters */
		sprintf(str, "%s/config/%s",
			app_data.libdir, basename(app_data.device));
		devspec_parminit(str, FALSE);

		/* Get user device-specific configuration parameters */
		sprintf(str, "%s/.xmcdcfg/%s",
			homedir(get_ouid()), basename(app_data.device));
		devspec_parminit(str, FALSE);

		/* Set the channel routing */
		di_route(s);

		/* Set the volume level */
		di_level(s, (byte_t) s->level, TRUE);
	}

	XmToggleButtonSetState(
		widgets.options.load_lock_btn,
		(Boolean) app_data.caddy_lock,
		False
	);

	XmToggleButtonSetState(
		widgets.options.load_none_btn,
		(Boolean) (!app_data.load_spindown && !app_data.load_play),
		False
	);
	XmToggleButtonSetState(
		widgets.options.load_spdn_btn,
		(Boolean) app_data.load_spindown,
		False
	);
	XmToggleButtonSetState(
		widgets.options.load_play_btn,
		(Boolean) app_data.load_play,
		False
	);

	XmToggleButtonSetState(
		widgets.options.exit_none_btn,
		(Boolean) (!app_data.exit_stop && !app_data.exit_eject),
		False
	);
	XmToggleButtonSetState(
		widgets.options.exit_stop_btn,
		(Boolean) app_data.exit_stop,
		False
	);
	XmToggleButtonSetState(
		widgets.options.exit_eject_btn,
		(Boolean) app_data.exit_eject,
		False
	);

	XmToggleButtonSetState(
		widgets.options.done_eject_btn,
		(Boolean) app_data.done_eject,
		False
	);

	XmToggleButtonSetState(
		widgets.options.eject_exit_btn,
		(Boolean) app_data.eject_exit,
		False
	);

	XmToggleButtonSetState(
		widgets.options.vol_linear_btn,
		(Boolean) (app_data.vol_taper == 0),
		False
	);
	XmToggleButtonSetState(
		widgets.options.vol_square_btn,
		(Boolean) (app_data.vol_taper == 1),
		False
	);
	XmToggleButtonSetState(
		widgets.options.vol_invsqr_btn,
		(Boolean) (app_data.vol_taper == 2),
		False
	);

	XmToggleButtonSetState(
		widgets.options.chroute_stereo_btn,
		(Boolean) (app_data.ch_route == 0),
		False
	);
	XmToggleButtonSetState(
		widgets.options.chroute_rev_btn,
		(Boolean) (app_data.ch_route == 1),
		False
	);
	XmToggleButtonSetState(
		widgets.options.chroute_left_btn,
		(Boolean) (app_data.ch_route == 2),
		False
	);
	XmToggleButtonSetState(
		widgets.options.chroute_right_btn,
		(Boolean) (app_data.ch_route == 3),
		False
	);
	XmToggleButtonSetState(
		widgets.options.chroute_mono_btn,
		(Boolean) (app_data.ch_route == 4),
		False
	);
}


/*
 * cd_options
 *	Options window toggle button callback function
 */
void
cd_options(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmRowColumnCallbackStruct	*p =
		(XmRowColumnCallbackStruct *)(void *) call_data;
	XmToggleButtonCallbackStruct	*q;
	curstat_t			*s = (curstat_t *)(void *) client_data;

	if (p != NULL)
		q = (XmToggleButtonCallbackStruct *)(void *) p->callbackstruct;

	if (w == widgets.options.load_chkbox) {
		if (p->widget == widgets.options.load_lock_btn) {
			if (app_data.caddylock_supp) {
				DBGPRN(errfp, "\n* OPTION: caddyLock=%d\n",
				       q->set);
				app_data.caddy_lock = (bool_t) q->set;
			}
			else {
				DBGPRN(errfp, "\n* OPTION: caddyLock=0\n");
				cd_beep();
				XmToggleButtonSetState(
					p->widget,
					False,
					False
				);
			}
		}
	}
	else if (w == widgets.options.load_radbox) {
		if (p->widget == widgets.options.load_spdn_btn) {
			DBGPRN(errfp, "\n* OPTION: spinDownOnLoad=%d\n",
			       q->set);
			app_data.load_spindown = (bool_t) q->set;
		}
		else if (p->widget == widgets.options.load_play_btn) {
			DBGPRN(errfp, "\n* OPTION: playOnLoad=%d\n", q->set);
			app_data.load_play = (bool_t) q->set;
		}
	}
	else if (w == widgets.options.exit_radbox) {
		if (p->widget == widgets.options.exit_stop_btn) {
			DBGPRN(errfp, "\n* OPTION: stopOnExit=%d\n", q->set);
			app_data.exit_stop = (bool_t) q->set;
		}
		else if (p->widget == widgets.options.exit_eject_btn) {
			if (app_data.eject_supp) {
				DBGPRN(errfp, "\n* OPTION: ejectOnExit=%d\n",
				       q->set);
				app_data.exit_eject = (bool_t) q->set;
			}
			else {
				DBGPRN(errfp, "\n* OPTION: ejectOnExit=0\n");
				cd_beep();
				XmToggleButtonSetState(
					p->widget,
					False,
					False
				);
				if (app_data.exit_stop) {
					XmToggleButtonSetState(
						widgets.options.exit_stop_btn,
						True,
						False
					);
				}
				else {
					XmToggleButtonSetState(
						widgets.options.exit_none_btn,
						True,
						False
					);
				}
			}
		}
	}
	else if (w == widgets.options.done_chkbox) {
		if (p->widget == widgets.options.done_eject_btn) {
			if (app_data.eject_supp) {
				DBGPRN(errfp, "\n* OPTION: ejectOnDone=%d\n",
				       q->set);
				app_data.done_eject = (bool_t) q->set;
			}
			else {
				DBGPRN(errfp, "\n* OPTION: ejectOnDone=0\n");
				cd_beep();
				XmToggleButtonSetState(
					p->widget,
					False,
					False
				);
			}
		}
	}
	else if (w == widgets.options.eject_chkbox) {
		if (p->widget == widgets.options.eject_exit_btn) {
			if (app_data.eject_supp) {
				DBGPRN(errfp, "\n* OPTION: exitOnEject=%d\n",
				       q->set);
				app_data.eject_exit = (bool_t) q->set;
			}
			else {
				DBGPRN(errfp, "\n* OPTION: exitOnEject=0\n");
				cd_beep();
				XmToggleButtonSetState(
					p->widget,
					False,
					False
				);
			}
		}
	}
	else if (w == widgets.options.chroute_radbox) {
		if (!q->set)
			return;

		if (!app_data.chroute_supp) {
			/* Channel routing not supported: force to
			 * normal setting.
			 */
			cd_beep();
			XmToggleButtonSetState(
				p->widget,
				False,
				False
			);
			XmToggleButtonSetState(
				widgets.options.chroute_stereo_btn,
				True,
				False
			);
			return;
		}

		if (p->widget == widgets.options.chroute_stereo_btn) {
			DBGPRN(errfp, "\n* OPTION: channelRoute=0\n");
			app_data.ch_route = 0;
		}
		else if (p->widget == widgets.options.chroute_rev_btn) {
			DBGPRN(errfp, "\n* OPTION: channelRoute=1\n");
			app_data.ch_route = 1;
		}
		else if (p->widget == widgets.options.chroute_left_btn) {
			DBGPRN(errfp, "\n* OPTION: channelRoute=2\n");
			app_data.ch_route = 2;
		}
		else if (p->widget == widgets.options.chroute_right_btn) {
			DBGPRN(errfp, "\n* OPTION: channelRoute=3\n");
			app_data.ch_route = 3;
		}
		else if (p->widget == widgets.options.chroute_mono_btn) {
			DBGPRN(errfp, "\n* OPTION: channelRoute=4\n");
			app_data.ch_route = 4;
		}

		di_route(s);
	}
	else if (w == widgets.options.vol_radbox) {
		if (!q->set)
			return;

		if (!app_data.mselvol_supp) {
			/* Volume control not supported: force to
			 * linear setting.
			 */
			cd_beep();
			XmToggleButtonSetState(
				p->widget,
				False,
				False
			);
			XmToggleButtonSetState(
				widgets.options.vol_linear_btn,
				True,
				False
			);
			return;
		}

		if (p->widget == widgets.options.vol_linear_btn) {
			DBGPRN(errfp, "\n* OPTION: volumeControlTaper=0\n");
			app_data.vol_taper = 0;
		}
		else if (p->widget == widgets.options.vol_square_btn) {
			DBGPRN(errfp, "\n* OPTION: volumeControlTaper=1\n");
			app_data.vol_taper = 1;
		}
		else if (p->widget == widgets.options.vol_invsqr_btn) {
			DBGPRN(errfp, "\n* OPTION: volumeControlTaper=2\n");
			app_data.vol_taper = 2;
		}

		di_level(s, (byte_t) s->level, TRUE);
	}
}


/*
 * cd_balance
 *	Balance control slider callback function
 */
/*ARGSUSED*/
void
cd_balance(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScaleCallbackStruct
			*p = (XmScaleCallbackStruct *)(void *) call_data;
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (!app_data.balance_supp) {
		if (p->reason == XmCR_VALUE_CHANGED)
			set_bal_slider(0);
		return;
	}

	if (p->value == 0) {
		/* Center setting */
		s->level_left = s->level_right = 100;
	}
	else if (p->value < 0) {
		/* Attenuate the right channel */
		s->level_left = 100;
		s->level_right = 100 + (p->value * 2);
	}
	else {
		/* Attenuate the left channel */
		s->level_left = 100 - (p->value * 2);
		s->level_right = 100;
	}

	di_level(
		s,
		(byte_t) s->level,
		(bool_t) (p->reason != XmCR_VALUE_CHANGED)
	);
}


/*
 * cd_balance_center
 *	Balance control center button callback function
 */
/*ARGSUSED*/
void
cd_balance_center(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScaleCallbackStruct	d;

	/* Force the balance control to the center position */
	set_bal_slider(0);

	/* Force a callback */
	d.reason = XmCR_VALUE_CHANGED;
	d.value = 0;
	cd_balance(widgets.options.bal_scale, client_data, (XtPointer) &d);
}


/*
 * cd_about
 *	Program information popup callback function
 */
/*ARGSUSED*/
void
cd_about(Widget w, XtPointer client_data, XtPointer call_data)
{
	int		i;
	char		txt[2048];
	XmString	xs_progname,
			xs_desc,
			xs_info,
			xs_tmp,
			xs;
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (XtIsManaged(widgets.dialog.about)) {
		/* Pop down the about dialog box */
		XtUnmanageChild(widgets.dialog.about);
		return;
	}

	xs_progname = XmStringCreateLtoR(PROGNAME, CHSET1);

	sprintf(txt, "   v%s%s PL%d\n%s\n%s\n%s\n\n",
		VERSION,
		VERSION_EXT,
		PATCHLEVEL,
		"Motif(tm) CD Audio Player",
		"Copyright (C) 1995  Ti Kan",
		"E-mail: ti@amb.org");

	xs_desc = XmStringCreateLtoR(txt, CHSET2);

	sprintf(txt, "%s\n%s%s %s %s%s%s\n%s%s\n%s%s\n\n%s",
		di_vers(),
		"CD-ROM: ",
		(s->vendor[0] == '\0') ? "??" : s->vendor,
		s->prod,
		(s->revnum[0] == '\0') ? "" : "(",
		s->revnum,
		(s->revnum[0] == '\0') ? "" : ")",
		"Device: ",
		app_data.device,
		"Mode:   ",
		di_mode(),
		"CD Database directories:"
	);

	for (i = 0; i < app_data.max_dbdirs && dbdirs[i] != NULL; i++)
		sprintf(txt, "%s\n    %s", txt, dbdirs[i]);

	if (i == 0)
		strcat(txt, " None");

	sprintf(txt, "%s\n\n%s\n%s", txt,
		"This is free software and comes with no warranty.",
		"See the GNU General Public License for details.");

	xs_info = XmStringCreateLtoR(txt, CHSET3);

	/* Set the dialog box title */
	xs = XmStringCreateSimple(app_data.str_about);
	XtVaSetValues(widgets.dialog.about, XmNdialogTitle, xs, NULL);
	XmStringFree(xs);

	/* Set the dialog box message */
	xs_tmp = XmStringConcat(xs_progname, xs_desc);
	xs = XmStringConcat(xs_tmp, xs_info);
	XtVaSetValues(widgets.dialog.about, XmNmessageString, xs, NULL);
	XmStringFree(xs_progname);
	XmStringFree(xs_desc);
	XmStringFree(xs_info);
	XmStringFree(xs_tmp);
	XmStringFree(xs);

	/* Pop up the about dialog box */
	XtManageChild(widgets.dialog.about);
}


/*
 * cd_help
 *	Program help window popup callback function
 */
/*ARGSUSED*/
void
cd_help_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
	if (w == widgets.main.help_btn && XtIsManaged(widgets.help.form)) {
		/* Pop down help window */
		XtUnmanageChild(widgets.help.form);

		return;
	}

	/* Pop up help window */
	help_popup(w);
}


/*
 * cd_help_popdown
 *	Program help window popdown callback function
 */
/*ARGSUSED*/
void
cd_help_popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down help window */
	if (XtIsManaged(widgets.help.form))
		XtUnmanageChild(widgets.help.form);
}


/*
 * cd_warning_popdown
 *	Warning message dialog box popdown callback function
 */
/*ARGSUSED*/
void
cd_warning_popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down the warning dialog */
	if (XtIsManaged(widgets.dialog.warning))
		XtUnmanageChild(widgets.dialog.warning);
}


/*
 * cd_fatal_popdown
 *	Fatal error message dialog box popdown callback function.
 *	This causes the application to terminate.
 */
void
cd_fatal_popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down the error dialog */
	if (XtIsManaged(widgets.dialog.fatal))
		XtUnmanageChild(widgets.dialog.fatal);

	/* Quit */
	cd_quit((curstat_t *) client_data);
}


/*
 * cd_rmcallback
 *	Remove callback function specified in cdinfo_t.
 */
/*ARGSUSED*/
void
cd_rmcallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	cbinfo_t	*cb = (cbinfo_t *)(void *) client_data;

	XtRemoveCallback(
		cb->widget,
		cb->type,
		(XtCallbackProc) cb->func,
		(XtPointer) cb->data
	);
}


/*
 * cd_form_focus_chg
 *	Focus change callback.  Used to implement keyboard grabs for
 *	hotkey handling.
 */
/*ARGSUSED*/
void
cd_form_focus_chg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;
	Widget			form = (Widget) client_data;
	static Widget		prev_form = (Widget) NULL;

	if (p->reason != XmCR_FOCUS || form == (Widget) NULL)
		return;

	if (prev_form != NULL) {
		if (form == prev_form)
			return;
		else
			hotkey_ungrabkeys(prev_form);
	}

	if (form != widgets.toplevel) {
		hotkey_grabkeys(form);
		prev_form = form;
	}
}


/*
 * cd_exit
 *	Shut down the application gracefully.
 */
/*ARGSUSED*/
void
cd_exit(Widget w, XtPointer client_data, XtPointer call_data)
{
	cd_quit((curstat_t *)(void *) client_data);
}


/**************** ^^ Callback routines ^^ ****************/

/***************** vv Event Handlers vv ******************/


/*
 * cd_btn_focus_chg
 *	Pushbutton keyboard focus change event handler.  Used to change
 *	the main window pushbuttons' label color.
 */
/*ARGSUSED*/
void
cd_btn_focus_chg(Widget w, XtPointer client_data, XEvent *ev, Boolean *cont)
{
	unsigned char	focuspolicy;		
	static bool_t	first = TRUE;
	static int	count = 0;
	static Pixel	fg,
			hl;

	if (!app_data.main_showfocus)
		return;

	if (first) {
		first = FALSE;

		XtVaGetValues(
			widgets.toplevel,
			XmNkeyboardFocusPolicy,
			&focuspolicy,
			NULL
		);
		if (focuspolicy != XmEXPLICIT) {
			app_data.main_showfocus = FALSE;
			return;
		}

		XtVaGetValues(w, XmNforeground, &fg, NULL);
		XtVaGetValues(w, XmNhighlightColor, &hl, NULL);
	}

	if (ev->xfocus.mode != NotifyNormal ||
	    ev->xfocus.detail != NotifyAncestor)
		return;

	if (ev->type == FocusOut) {
		if (count <= 0)
			return;

		/* Restore original forground pixmap */
		if (w == widgets.main.eject_btn)
			set_btn_color(w, pixmaps.main.eject_pixmap, fg);
		else if (w == widgets.main.poweroff_btn)
			set_btn_color(w, pixmaps.main.poweroff_pixmap, fg);
		else if (w == widgets.main.dbprog_btn)
			set_btn_color(w, pixmaps.main.dbprog_pixmap, fg);
		else if (w == widgets.main.help_btn)
			set_btn_color(w, pixmaps.main.help_pixmap, fg);
		else if (w == widgets.main.options_btn)
			set_btn_color(w, pixmaps.main.options_pixmap, fg);
		else if (w == widgets.main.time_btn)
			set_btn_color(w, pixmaps.main.time_pixmap, fg);
		else if (w == widgets.main.ab_btn)
			set_btn_color(w, pixmaps.main.ab_pixmap, fg);
		else if (w == widgets.main.sample_btn)
			set_btn_color(w, pixmaps.main.sample_pixmap, fg);
		else if (w == widgets.main.keypad_btn)
			set_btn_color(w, pixmaps.main.keypad_pixmap, fg);
		else if (w == widgets.main.level_scale)
			set_scale_color(w, fg);
		else if (w == widgets.main.playpause_btn)
			set_btn_color(w, pixmaps.main.playpause_pixmap, fg);
		else if (w == widgets.main.stop_btn)
			set_btn_color(w, pixmaps.main.stop_pixmap, fg);
		else if (w == widgets.main.prevtrk_btn)
			set_btn_color(w, pixmaps.main.prevtrk_pixmap, fg);
		else if (w == widgets.main.nexttrk_btn)
			set_btn_color(w, pixmaps.main.nexttrk_pixmap, fg);
		else if (w == widgets.main.previdx_btn)
			set_btn_color(w, pixmaps.main.previdx_pixmap, fg);
		else if (w == widgets.main.nextidx_btn)
			set_btn_color(w, pixmaps.main.nextidx_pixmap, fg);
		else if (w == widgets.main.rew_btn)
			set_btn_color(w, pixmaps.main.rew_pixmap, fg);
		else if (w == widgets.main.ff_btn)
			set_btn_color(w, pixmaps.main.ff_pixmap, fg);

		count--;
	}
	else if (ev->type == FocusIn) {
		if (count >= 1)
			return;

		/* Set new highlighted forground pixmap */
		if (w == widgets.main.eject_btn)
			set_btn_color(w, pixmaps.main.eject_hlpixmap, hl);
		else if (w == widgets.main.poweroff_btn)
			set_btn_color(w, pixmaps.main.poweroff_hlpixmap, hl);
		else if (w == widgets.main.dbprog_btn)
			set_btn_color(w, pixmaps.main.dbprog_hlpixmap, hl);
		else if (w == widgets.main.help_btn)
			set_btn_color(w, pixmaps.main.help_hlpixmap, hl);
		else if (w == widgets.main.options_btn)
			set_btn_color(w, pixmaps.main.options_hlpixmap, hl);
		else if (w == widgets.main.time_btn)
			set_btn_color(w, pixmaps.main.time_hlpixmap, hl);
		else if (w == widgets.main.ab_btn)
			set_btn_color(w, pixmaps.main.ab_hlpixmap, hl);
		else if (w == widgets.main.sample_btn)
			set_btn_color(w, pixmaps.main.sample_hlpixmap, hl);
		else if (w == widgets.main.keypad_btn)
			set_btn_color(w, pixmaps.main.keypad_hlpixmap, hl);
		else if (w == widgets.main.level_scale)
			set_scale_color(w, hl);
		else if (w == widgets.main.playpause_btn)
			set_btn_color(w, pixmaps.main.playpause_hlpixmap, hl);
		else if (w == widgets.main.stop_btn)
			set_btn_color(w, pixmaps.main.stop_hlpixmap, hl);
		else if (w == widgets.main.prevtrk_btn)
			set_btn_color(w, pixmaps.main.prevtrk_hlpixmap, hl);
		else if (w == widgets.main.nexttrk_btn)
			set_btn_color(w, pixmaps.main.nexttrk_hlpixmap, hl);
		else if (w == widgets.main.previdx_btn)
			set_btn_color(w, pixmaps.main.previdx_hlpixmap, hl);
		else if (w == widgets.main.nextidx_btn)
			set_btn_color(w, pixmaps.main.nextidx_hlpixmap, hl);
		else if (w == widgets.main.rew_btn)
			set_btn_color(w, pixmaps.main.rew_hlpixmap, hl);
		else if (w == widgets.main.ff_btn)
			set_btn_color(w, pixmaps.main.ff_hlpixmap, hl);

		count++;
	}
}

/***************** ^^ Event Handlers ^^ ******************/

