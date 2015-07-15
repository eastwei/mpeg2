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
static char *_libdi_c_ident_ = "@(#)libdi.c	5.8 95/01/27";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"
#include "libdi.d/slioc.h"


extern appdata_t	app_data;


/* libdi module init routines */
diinit_tbl_t		diinit[] = {
	scsipt_init,			/* SCSI pass-through method */
	slioc_init,			/* SunOS/Linux ioctl method */
	NULL				/* List terminator */
};


/* libdi interface calling table */
di_tbl_t		ditbl[MAX_METHODS];


/*
 * di_init
 *	Top-level function to initialize the libdi modules.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_init(curstat_t *s)
{
	int	i;

#if defined(DI_SCSIPT) && defined(DEMO_ONLY)
	app_data.di_method = DI_SCSIPT;
#else
	/* Sanity check */
	if (app_data.di_method < 0 || app_data.di_method >= MAX_METHODS) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomethod);
		return;
	}
#endif

	/* Initialize the libdi modules */
	for (i = 0; i < MAX_METHODS; i++) {
		if (diinit[i].init != NULL)
			diinit[i].init(s, &ditbl[i]);
	}

	/* Sanity check again */
	if (ditbl[app_data.di_method].mode == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomethod);
		return;
	}
}


/*
 * di_check_disc
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
di_check_disc(curstat_t *s)
{
	if (ditbl[app_data.di_method].check_disc != NULL)
		return (ditbl[app_data.di_method].check_disc(s));

	return FALSE;
}


/*
 * di_status_upd
 *	Force update of playback status
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_status_upd(curstat_t *s)
{
	if (ditbl[app_data.di_method].status_upd != NULL)
		ditbl[app_data.di_method].status_upd(s);
}


/*
 * di_lock
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
di_lock(curstat_t *s, bool_t enable)
{
	if (ditbl[app_data.di_method].lock != NULL)
		ditbl[app_data.di_method].lock(s, enable);
}


/*
 * di_repeat
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
di_repeat(curstat_t *s, bool_t enable)
{
	if (ditbl[app_data.di_method].repeat != NULL)
		ditbl[app_data.di_method].repeat(s, enable);
}


/*
 * di_shuffle
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
di_shuffle(curstat_t *s, bool_t enable)
{
	if (ditbl[app_data.di_method].shuffle != NULL)
		ditbl[app_data.di_method].shuffle(s, enable);
}


/*
 * di_load_eject
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
di_load_eject(curstat_t *s)
{
	if (ditbl[app_data.di_method].load_eject != NULL)
		ditbl[app_data.di_method].load_eject(s);
}


/*
 * di_ab
 *	A->B segment play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_ab(curstat_t *s)
{
	if (ditbl[app_data.di_method].ab != NULL)
		ditbl[app_data.di_method].ab(s);
}


/*
 * di_sample
 *	Sample play mode function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_sample(curstat_t *s)
{
	if (ditbl[app_data.di_method].sample != NULL)
		ditbl[app_data.di_method].sample(s);
}


/*
 * di_level
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
di_level(curstat_t *s, byte_t level, bool_t drag)
{
	if (ditbl[app_data.di_method].level != NULL)
		ditbl[app_data.di_method].level(s, level, drag);
}


/*
 * di_play_pause
 *	Audio playback and pause function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_play_pause(curstat_t *s)
{
	if (ditbl[app_data.di_method].play_pause != NULL)
		ditbl[app_data.di_method].play_pause(s);
}


/*
 * di_stop
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
di_stop(curstat_t *s, bool_t stop_disc)
{
	if (ditbl[app_data.di_method].stop != NULL)
		ditbl[app_data.di_method].stop(s, stop_disc);
}


/*
 * di_prevtrk
 *	Previous track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_prevtrk(curstat_t *s)
{
	if (ditbl[app_data.di_method].prevtrk != NULL)
		ditbl[app_data.di_method].prevtrk(s);
}


/*
 * di_nexttrk
 *	Next track function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_nexttrk(curstat_t *s)
{
	if (ditbl[app_data.di_method].nexttrk != NULL)
		ditbl[app_data.di_method].nexttrk(s);
}


/*
 * di_previdx
 *	Previous index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_previdx(curstat_t *s)
{
	if (ditbl[app_data.di_method].previdx != NULL)
		ditbl[app_data.di_method].previdx(s);
}


/*
 * di_nextidx
 *	Next index function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_nextidx(curstat_t *s)
{
	if (ditbl[app_data.di_method].nextidx != NULL)
		ditbl[app_data.di_method].nextidx(s);
}


/*
 * di_rew
 *	Search-rewind function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_rew(curstat_t *s, bool_t start)
{
	if (ditbl[app_data.di_method].rew != NULL)
		ditbl[app_data.di_method].rew(s, start);
}


/*
 * di_ff
 *	Search-fast-forward function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_ff(curstat_t *s, bool_t start)
{
	if (ditbl[app_data.di_method].ff != NULL)
		ditbl[app_data.di_method].ff(s, start);
}


/*
 * di_warp
 *	Track warp function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_warp(curstat_t *s)
{
	if (ditbl[app_data.di_method].warp != NULL)
		ditbl[app_data.di_method].warp(s);
}


/*
 * di_route
 *	Channel routing function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_route(curstat_t *s)
{
	if (ditbl[app_data.di_method].route != NULL)
		ditbl[app_data.di_method].route(s);
}


/*
 * di_mute_on
 *	Mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_mute_on(curstat_t *s)
{
	if (ditbl[app_data.di_method].mute_on != NULL)
		ditbl[app_data.di_method].mute_on(s);
}


/*
 * di_mute_off
 *	Un-mute audio function
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_mute_off(curstat_t *s)
{
	if (ditbl[app_data.di_method].mute_off != NULL)
		ditbl[app_data.di_method].mute_off(s);
}


/*
 * di_start
 *	Start the SCSI pass-through module.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_start(curstat_t *s)
{
	if (ditbl[app_data.di_method].start != NULL)
		ditbl[app_data.di_method].start(s);
}


/*
 * di_icon
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
di_icon(curstat_t *s, bool_t iconified)
{
	if (ditbl[app_data.di_method].icon != NULL)
		ditbl[app_data.di_method].icon(s, iconified);
}


/*
 * di_halt
 *	Shut down the SCSI pass-through and vendor-unique modules.
 *
 * Args:
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
di_halt(curstat_t *s)
{
	if (ditbl[app_data.di_method].halt != NULL)
		ditbl[app_data.di_method].halt(s);
}


/*
 * di_mode
 *	Return a text string indicating the current operating mode.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Mode text string.
 */
char *
di_mode(void)
{
	if (ditbl[app_data.di_method].mode != NULL)
		return (ditbl[app_data.di_method].mode());

	return ("");
}


/*
 * di_vers
 *	Return a text string indicating active module's version number
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Version text string.
 */
char *
di_vers(void)
{
	if (ditbl[app_data.di_method].vers != NULL)
		return (ditbl[app_data.di_method].vers());

	return ("");
}


/*
 * di_isdemo
 *	Query if this is a demo-only version of the CD player.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	TRUE - demo-only version.
 *	FALSE - real version.
 */
bool_t
di_isdemo(void)
{
#ifdef DEMO_ONLY
	return TRUE;
#else
	return FALSE;
#endif
}

