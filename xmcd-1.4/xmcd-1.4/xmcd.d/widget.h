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
#ifndef __WIDGET_H__
#define __WIDGET_H__

#ifndef LINT
static char *_widget_h_ident_ = "@(#)widget.h	5.2 94/12/28";
#endif

/* Holder for all widgets */
typedef struct {
	/* Top-level */
	Widget		toplevel;

	/* Main window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	chkbox_frm;		/* Frame for Lock toggle */
		Widget	check_box;		/* Toggle button checkbox */
		Widget	btnlbl_btn;		/* Button label button */
		Widget	lock_btn;		/* Caddy lock button */
		Widget	repeat_btn;		/* Repeat button */
		Widget	shuffle_btn;		/* Shuffle button */
		Widget	eject_btn;		/* Eject button */
		Widget	poweroff_btn;		/* Power off button */
		Widget	track_ind;		/* Track number indicator */
		Widget	index_ind;		/* Index number indicator */
		Widget	time_ind;		/* Time indicator */
		Widget	rptcnt_ind;		/* Repeat count indicator */
		Widget	dbmode_ind;		/* CD database indicator */
		Widget	progmode_ind;		/* Program mode indicator */
		Widget	timemode_ind;		/* Time mode indicator */
		Widget	playmode_ind;		/* Play mode indicator */
		Widget	dtitle_ind;		/* Disc title indicator */
		Widget	ttitle_ind;		/* Track title indicator */
		Widget	dbprog_btn;		/* CDDB/Program button */
		Widget	options_btn;		/* Options button */
		Widget	time_btn;		/* Time display button */
		Widget	ab_btn;			/* A->B mode button */
		Widget	sample_btn;		/* Sample mode button */
		Widget	keypad_btn;		/* Keypad button */
		Widget	help_btn;		/* Help button */
		Widget	level_scale;		/* Volume control slider */
		Widget	playpause_btn;		/* Play/Pause button */
		Widget	stop_btn;		/* Stop button */
		Widget	prevtrk_btn;		/* Prev Track button */
		Widget	nexttrk_btn;		/* Next Track button */
		Widget	previdx_btn;		/* Prev Index button */
		Widget	nextidx_btn;		/* Next Index button */
		Widget	rew_btn;		/* Search REW button */
		Widget	ff_btn;			/* Search FF button */
	} main;

	/* Keypad window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	keypad_lbl;		/* Keypad label */
		Widget	keypad_ind;		/* Keypad indicator */
		Widget	num_btn[10];		/* Number keys */
		Widget	clear_btn;		/* Clear button */
		Widget	enter_btn;		/* Enter button */
		Widget	warp_lbl;		/* Track warp label */
		Widget	warp_scale;		/* Track warp slider */
		Widget	keypad_sep;		/* Separator bar */
		Widget	cancel_btn;		/* Cancel button */
	} keypad;

	/* Options window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	load_lbl;		/* Load options label */
		Widget	load_chkbox_frm;	/* Load options chkbox frame */
		Widget	load_chkbox;		/* Load options check box */
		Widget	load_lock_btn;		/* Auto Lock button */
		Widget	load_radbox_frm;	/* Load options radbox frame */
		Widget	load_radbox;		/* Load options radio box */
		Widget	load_none_btn;		/* None button */
		Widget	load_spdn_btn;		/* Spin Down button */
		Widget	load_play_btn;		/* Auto Play button */
		Widget	exit_lbl;		/* Exit options label */
		Widget	exit_radbox_frm;	/* Exit options frame */
		Widget	exit_radbox;		/* Exit options radio box */
		Widget	exit_none_btn;		/* None button */
		Widget	exit_stop_btn;		/* Auto Stop button */
		Widget	exit_eject_btn;		/* Auto Eject button */
		Widget	done_lbl;		/* Done options label */
		Widget	done_chkbox_frm;	/* Done options frame */
		Widget	done_chkbox;		/* Done options check box */
		Widget	done_eject_btn;		/* Auto Eject button */
		Widget	eject_lbl;		/* Eject options label */
		Widget	eject_chkbox_frm;	/* Eject options frame */
		Widget	eject_chkbox;		/* Eject options check box */
		Widget	eject_exit_btn;		/* Auto Exit button */
		Widget	chroute_lbl;		/* Mode select label */
		Widget	chroute_radbox_frm;	/* Mode select frame */
		Widget	chroute_radbox;		/* Mode select radio box */
		Widget	chroute_stereo_btn;	/* Stereo button */
		Widget	chroute_rev_btn;	/* Stereo Reverse button */
		Widget	chroute_left_btn;	/* Mono Left-only button */
		Widget	chroute_right_btn;	/* Mono Right-only button */
		Widget	chroute_mono_btn;	/* Mono Left+Right button */
		Widget	vol_lbl;		/* Vol ctrl taper label */
		Widget	vol_radbox_frm;		/* Vol ctrl taper frame */
		Widget	vol_radbox;		/* Vol ctrl taper radio box */
		Widget	vol_linear_btn;		/* Linear button */
		Widget	vol_square_btn;		/* Square button */
		Widget	vol_invsqr_btn;		/* Inverse Square button */
		Widget	bal_lbl;		/* Balance ctrl label */
		Widget	ball_lbl;		/* Balance ctrl left label */
		Widget	balr_lbl;		/* Balance ctrl right label */
		Widget	bal_scale;		/* Balance ctrl scale */
		Widget	balctr_btn;		/* Balance center button */
		Widget	options_sep;		/* Separator bar */
		Widget	reset_btn;		/* Reset button */
		Widget	ok_btn;			/* OK button */
	} options;

	/* CD Database/Program window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	logo_lbl;		/* Logo label */
		Widget	about_btn;		/* About button */
		Widget	dtitle_lbl;		/* Disc title label */
		Widget	dtitle_txt;		/* Disc title text */
		Widget	extd_lbl;		/* Disc ext descr label */
		Widget	extd_btn;		/* Disc ext descr button */
		Widget	dbprog_sep1;		/* Separator bar */
		Widget	trklist_lbl;		/* Track list label */
		Widget	trk_list;		/* Track list */
		Widget	radio_lbl;		/* Time dpy radio box label */
		Widget	radio_frm;		/* Time dpy radio box Frame */
		Widget	radio_box;		/* Time dpy radio box */
		Widget	tottime_btn;		/* Total time button */
		Widget	trktime_btn;		/* Track time button */
		Widget	discid_lbl;		/* Disc ID label */
		Widget	discid_frm;		/* Disc ID frame */
		Widget	discid_ind;		/* Disc ID indicator */
		Widget	ttitle_lbl;		/* Track title label */
		Widget	ttitle_txt;		/* Track title text */
		Widget	extt_lbl;		/* Track ext descr label */
		Widget	extt_btn;		/* Track ext descr button */
		Widget	pgm_lbl;		/* program label */
		Widget	addpgm_btn;		/* Add button */
		Widget	clrpgm_btn;		/* Clear button */
		Widget	pgmseq_lbl;		/* Program sequence label */
		Widget	pgmseq_txt;		/* Program sequence text */
		Widget	send_btn;		/* Send CDDB button */
		Widget	dbprog_sep2;		/* Separator bar */
		Widget	savedb_btn;		/* Save to database button */
		Widget	linkdb_btn;		/* Link database entry button */
		Widget	loaddb_btn;		/* Load from database button */
		Widget	cancel_btn;		/* Cancel button */
	} dbprog;

	/* Extended disc info window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	disc_lbl;		/* Disc ext descr label */
		Widget	disc_txt;		/* Disc ext descr text */
		Widget	dbextd_sep;		/* Separator bar */
		Widget	ok_btn;			/* OK button */
		Widget	clear_btn;		/* Clear button */
		Widget	cancel_btn;		/* Cancel button */
	} dbextd;

	/* Extended track info window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	trk_lbl;		/* Track ext descr label */
		Widget	trk_txt;		/* Track ext descr text */
		Widget	dbextt_sep;		/* Separator bar */
		Widget	ok_btn;			/* OK button */
		Widget	clear_btn;		/* Clear button */
		Widget	cancel_btn;		/* Cancel button */
	} dbextt;

	/* Directory Selector window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	dir_lbl;		/* Directory list label */
		Widget	dir_list;		/* Directory list */
		Widget	dirsel_sep;		/* Separator bar */
		Widget	ok_btn;			/* OK button */
		Widget	cancel_btn;		/* Cancel button */
	} dirsel;

	/* Link Selector window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	link_lbl;		/* Directory list label */
		Widget	link_list;		/* Directory list */
		Widget	linksel_sep;		/* Separator bar */
		Widget	ok_btn;			/* OK button */
		Widget	cancel_btn;		/* Cancel button */
	} linksel;

	/* Help window widgets */
	struct {
		Widget	form;			/* Form container */
		Widget	help_txt;		/* Help text */
		Widget	help_sep;		/* Separator bar */
		Widget	ok_btn;			/* OK button */
	} help;

	/* Dialog windows widgets */
	struct {
		Widget	info;			/* Info popup */
		Widget	warning;		/* Warning popup */
		Widget	fatal;			/* Fatal error popup */
		Widget	confirm;		/* Confirm popup */
		Widget	about;			/* About popup */
	} dialog;
} widgets_t;


/* Holder for all button-face pixmaps */
typedef struct {
	/* Main window pixmaps */
	struct {
		Pixmap	icon_pixmap;		/* Icon logo */
		Pixmap	btnlbl_pixmap;		/* Button label mode */
		Pixmap	lock_pixmap;		/* Prevent caddy removal */
		Pixmap	repeat_pixmap;		/* Repeat */
		Pixmap	shuffle_pixmap;		/* Shuffle */
		Pixmap	eject_pixmap;		/* Eject */
		Pixmap	eject_hlpixmap;		/* Eject */
		Pixmap	poweroff_pixmap;	/* Power off */
		Pixmap	poweroff_hlpixmap;	/* Power off */
		Pixmap	dbprog_pixmap;		/* CDDB/Prog */
		Pixmap	dbprog_hlpixmap;	/* CDDB/Prog */
		Pixmap	options_pixmap;		/* Options */
		Pixmap	options_hlpixmap;	/* Options */
		Pixmap	time_pixmap;		/* Time */
		Pixmap	time_hlpixmap;		/* Time */
		Pixmap	ab_pixmap;		/* A->B */
		Pixmap	ab_hlpixmap;		/* A->B */
		Pixmap	sample_pixmap;		/* Sample */
		Pixmap	sample_hlpixmap;	/* Sample */
		Pixmap	keypad_pixmap;		/* Keypad */
		Pixmap	keypad_hlpixmap;	/* Keypad */
		Pixmap	help_pixmap;		/* Help */
		Pixmap	help_hlpixmap;		/* Help */
		Pixmap	playpause_pixmap;	/* Play/Pause */
		Pixmap	playpause_hlpixmap;	/* Play/Pause */
		Pixmap	stop_pixmap;		/* Stop */
		Pixmap	stop_hlpixmap;		/* Stop */
		Pixmap	prevtrk_pixmap;		/* Prev Track */
		Pixmap	prevtrk_hlpixmap;	/* Prev Track */
		Pixmap	nexttrk_pixmap;		/* Next Track */
		Pixmap	nexttrk_hlpixmap;	/* Next Track */
		Pixmap	previdx_pixmap;		/* Prev Index */
		Pixmap	previdx_hlpixmap;	/* Prev Index */
		Pixmap	nextidx_pixmap;		/* Next Index */
		Pixmap	nextidx_hlpixmap;	/* Next Index */
		Pixmap	rew_pixmap;		/* Search REW */
		Pixmap	rew_hlpixmap;		/* Search REW */
		Pixmap	ff_pixmap;		/* Search FF */
		Pixmap	ff_hlpixmap;		/* Search FF */
	} main;

	/* CD Database/Program window pixmaps */
	struct {
		Pixmap	logo_pixmap;		/* CD Logo */
	} dbprog;

	/* Dialog windows pixmaps */
	struct {
		Pixmap	xmcd_pixmap;		/* Program logo */
	} dialog;
} pixmaps_t;


/* Mode flags for bm_to_px(): used to set foreground/background colors */
#define BM_PX_NORMAL	0		/* normal/normal */
#define BM_PX_HIGHLIGHT	1		/* highlight/normal */
#define BM_PX_WHITE	2		/* white/normal */
#define BM_PX_BLACK	3		/* black/normal */
#define BM_PX_BW	4		/* black/white */
#define BM_PX_BWREV	5		/* white/black */


/* Public function prototypes */
extern Pixmap	bm_to_px(Widget, void *, int, int, int, int);
extern void	create_widgets(widgets_t *);
extern void	pre_realize_config(widgets_t *);
extern void	post_realize_config(widgets_t *, pixmaps_t *);
extern void	register_callbacks(widgets_t *, curstat_t *);

#endif /* __WIDGET_H__ */

