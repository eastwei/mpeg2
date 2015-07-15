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
static char *_widget_c_ident_ = "@(#)widget.c	5.4 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "common.d/patchlevel.h"
#include "xmcd.d/xmcd.h"
#include "xmcd.d/dbprog.h"
#include "xmcd.d/cdfunc.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/geom.h"
#include "xmcd.d/help.h"
#include "xmcd.d/hotkey.h"
#include "libdi.d/libdi.h"

#include "xmcd.d/bitmaps/btnlbl.xbm"
#include "xmcd.d/bitmaps/lock.xbm"
#include "xmcd.d/bitmaps/repeat.xbm"
#include "xmcd.d/bitmaps/shuffle.xbm"
#include "xmcd.d/bitmaps/eject.xbm"
#include "xmcd.d/bitmaps/poweroff.xbm"
#include "xmcd.d/bitmaps/dbprog.xbm"
#include "xmcd.d/bitmaps/help.xbm"
#include "xmcd.d/bitmaps/options.xbm"
#include "xmcd.d/bitmaps/time.xbm"
#include "xmcd.d/bitmaps/ab.xbm"
#include "xmcd.d/bitmaps/sample.xbm"
#include "xmcd.d/bitmaps/keypad.xbm"
#include "xmcd.d/bitmaps/playpaus.xbm"
#include "xmcd.d/bitmaps/stop.xbm"
#include "xmcd.d/bitmaps/prevtrk.xbm"
#include "xmcd.d/bitmaps/nexttrk.xbm"
#include "xmcd.d/bitmaps/previdx.xbm"
#include "xmcd.d/bitmaps/nextidx.xbm"
#include "xmcd.d/bitmaps/rew.xbm"
#include "xmcd.d/bitmaps/ff.xbm"
#include "xmcd.d/bitmaps/logo.xbm"
#include "xmcd.d/bitmaps/xmcd.xbm"


extern appdata_t	app_data;
extern widgets_t	widgets;

STATIC Atom		delw;
STATIC XtCallbackRec	main_chkbox_cblist[] = {
	{ (XtCallbackProc) cd_checkbox,		NULL },
	{ (XtCallbackProc) NULL,		NULL },
};
STATIC XtCallbackRec	options_cblist[] = {
	{ (XtCallbackProc) cd_options,		NULL },
	{ (XtCallbackProc) NULL,		NULL },
};
STATIC XtCallbackRec	dbprog_radbox_cblist[] = {
	{ (XtCallbackProc) dbprog_timedpy,	NULL },
	{ (XtCallbackProc) NULL,		NULL },
};
STATIC XtCallbackRec	help_cblist[] = {
	{ (XtCallbackProc) cd_help_popup,	NULL },
	{ (XtCallbackProc) NULL,		NULL },
};


/***********************
 *  internal routines  *
 ***********************/


/*
 * Action routines
 */

/*
 * focuschg
 *	Widget action routine to handle keyboard focus change events
 *	This is used to change the label color of widgets in the
 *	main window.
 */
/*ARGSUSED*/
STATIC void
focuschg(Widget w, XEvent *ev, String *args, Cardinal *num_args)
{
	Widget	action_widget;
	Boolean	cont;

	if ((int) *num_args != 1)
		return;	/* Error: should have one arg */

	action_widget = XtNameToWidget(widgets.main.form, args[0]);
	if (action_widget == NULL)
		return;	/* Can't find widget */

	cd_btn_focus_chg(action_widget, NULL, ev, &cont);
}


/*
 * mainmap
 *	Widget actin routine to handle the map and unmap events
 *	on the main window.  This is used to perform certain
 *	optimizations when the user iconifies the application.
 */
/*ARGSUSED*/
STATIC void
mainmap(Widget w, XEvent *ev, String *args, Cardinal *num_args)
{
	curstat_t	*s = curstat_addr();

	if (w != widgets.toplevel)
		return;

	if (ev->type == MapNotify)
		cd_icon(s, FALSE);
	else if (ev->type == UnmapNotify)
		cd_icon(s, TRUE);
}


/*
 * Widget-related functions
 */

/*
 * create_main_widgets
 *	Create all widgets in the main window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_main_widgets(widgets_t *m)
{
	int		i;
	Arg		arg[10];
	curstat_t	*s = curstat_addr();

	main_chkbox_cblist[0].closure = (XtPointer) s;

	/* Create form widget as container */
	m->main.form = XmCreateForm(
		m->toplevel,
		"mainForm",
		NULL,
		0
	);

	/* Create frame for check box */
	m->main.chkbox_frm = XmCreateFrame(
		m->main.form,
		"checkBoxFrame",
		NULL,
		0
	);

	/* Create check box widget */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 4); i++;
	XtSetArg(arg[i], XmNspacing, 2); i++;
	XtSetArg(arg[i], XmNmarginHeight, 4); i++;
	XtSetArg(arg[i], XmNentryCallback, main_chkbox_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.check_box = XmCreateSimpleCheckBox(
		m->main.chkbox_frm,
		"checkBox",
		arg,
		i
	);
	m->main.btnlbl_btn = XtNameToWidget(m->main.check_box, "button_0");
	m->main.lock_btn = XtNameToWidget(m->main.check_box, "button_1");
	m->main.repeat_btn = XtNameToWidget(m->main.check_box, "button_2");
	m->main.shuffle_btn = XtNameToWidget(m->main.check_box, "button_3");

	/* Create pushbutton widget for Eject button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.eject_btn = XmCreatePushButton(
		m->main.form,
		"ejectButton",
		arg,
		i
	);

	/* Create pushbutton widget for Power off button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.poweroff_btn = XmCreatePushButton(
		m->main.form,
		"powerOffButton",
		arg,
		i
	);

	/* Create label widget as track indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.track_ind = XmCreateLabel(
		m->main.form,
		"trackIndicator",
		arg,
		i
	);

	/* Create label widget as index indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.index_ind = XmCreateLabel(
		m->main.form,
		"indexIndicator",
		arg,
		i
	);

	/* Create label widget as time indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.time_ind = XmCreateLabel(
		m->main.form,
		"timeIndicator",
		arg,
		i
	);

	/* Create label widget as Repeat count indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.rptcnt_ind = XmCreateLabel(
		m->main.form,
		"repeatCountIndicator",
		arg,
		i
	);

	/* Create label widget as CDDB indicator label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.dbmode_ind = XmCreateLabel(
		m->main.form,
		"dbModeIndicator",
		arg,
		i
	);

	/* Create label widget as Program Mode indicator label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.progmode_ind = XmCreateLabel(
		m->main.form,
		"progModeIndicator",
		arg,
		i
	);

	/* Create label widget as Time Mode indicator label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.timemode_ind = XmCreateLabel(
		m->main.form,
		"timeModeIndicator",
		arg,
		i
	);

	/* Create label widget as Play Mode indicator label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.playmode_ind = XmCreateLabel(
		m->main.form,
		"playModeIndicator",
		arg,
		i
	);

	/* Create label widget as disc title indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.dtitle_ind = XmCreateLabel(
		m->main.form,
		"discTitleIndicator",
		arg,
		i
	);

	/* Create label widget as track title indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.ttitle_ind = XmCreateLabel(
		m->main.form,
		"trackTitleIndicator",
		arg,
		i
	);

	/* Create pushbutton widget for CDDB/Program button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.dbprog_btn = XmCreatePushButton(
		m->main.form,
		"dbprogButton",
		arg,
		i
	);

	/* Create pushbutton widget as Options button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.options_btn = XmCreatePushButton(
		m->main.form,
		"optionsButton",
		arg,
		i
	);

	/* Create pushbutton widget for Time button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.time_btn = XmCreatePushButton(
		m->main.form,
		"timeButton",
		arg,
		i
	);

	/* Create pushbutton widgets for A->B button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.ab_btn = XmCreatePushButton(
		m->main.form,
		"abButton",
		arg,
		i
	);

	/* Create pushbutton widget for Sample button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.sample_btn = XmCreatePushButton(
		m->main.form,
		"sampleButton",
		arg,
		i
	);

	/* Create pushbutton widget for Sample button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.keypad_btn = XmCreatePushButton(
		m->main.form,
		"keypadButton",
		arg,
		i
	);

	/* Create pushbutton widget for Help button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.help_btn = XmCreatePushButton(
		m->main.form,
		"helpButton",
		arg,
		i
	);

	/* Create scale widget for level slider */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.level_scale = XmCreateScale(
		m->main.form,
		"levelScale",
		arg,
		i
	);

	/* Create pushbutton widget for Play-Pause button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.playpause_btn = XmCreatePushButton(
		m->main.form,
		"playPauseButton",
		arg,
		i
	);

	/* Create pushbutton widget for Stop button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.stop_btn = XmCreatePushButton(
		m->main.form,
		"stopButton",
		arg,
		i
	);

	/* Create pushbutton widget for Prev Track button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.prevtrk_btn = XmCreatePushButton(
		m->main.form,
		"prevTrackButton",
		arg,
		i
	);

	/* Create pushbutton widget for Next Track button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.nexttrk_btn = XmCreatePushButton(
		m->main.form,
		"nextTrackButton",
		arg,
		i
	);

	/* Create pushbutton widget for Prev Index button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.previdx_btn = XmCreatePushButton(
		m->main.form,
		"prevIndexButton",
		arg,
		i
	);

	/* Create pushbutton widget for Next Index button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.nextidx_btn = XmCreatePushButton(
		m->main.form,
		"nextIndexButton",
		arg,
		i
	);

	/* Create pushbutton widget for REW button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.rew_btn = XmCreatePushButton(
		m->main.form,
		"rewButton",
		arg,
		i
	);

	/* Create pushbutton widget for FF button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->main.ff_btn = XmCreatePushButton(
		m->main.form,
		"ffButton",
		arg,
		i
	);

	/* Manage the widgets */
	XtManageChild(m->main.form);
	XtManageChild(m->main.chkbox_frm);
	XtManageChild(m->main.check_box);
	XtManageChild(m->main.eject_btn);
	XtManageChild(m->main.poweroff_btn);
	XtManageChild(m->main.track_ind);
	XtManageChild(m->main.index_ind);
	XtManageChild(m->main.time_ind);
	XtManageChild(m->main.rptcnt_ind);
	XtManageChild(m->main.dbmode_ind);
	XtManageChild(m->main.progmode_ind);
	XtManageChild(m->main.timemode_ind);
	XtManageChild(m->main.playmode_ind);
	XtManageChild(m->main.dtitle_ind);
	XtManageChild(m->main.ttitle_ind);
	XtManageChild(m->main.dbprog_btn);
	XtManageChild(m->main.options_btn);
	XtManageChild(m->main.time_btn);
	XtManageChild(m->main.ab_btn);
	XtManageChild(m->main.sample_btn);
	XtManageChild(m->main.keypad_btn);
	XtManageChild(m->main.help_btn);
	XtManageChild(m->main.level_scale);
	XtManageChild(m->main.playpause_btn);
	XtManageChild(m->main.stop_btn);
	XtManageChild(m->main.prevtrk_btn);
	XtManageChild(m->main.nexttrk_btn);
	XtManageChild(m->main.previdx_btn);
	XtManageChild(m->main.nextidx_btn);
	XtManageChild(m->main.rew_btn);
	XtManageChild(m->main.ff_btn);
}


/*
 * create_keypad_widgets
 *	Create all widgets in the keypad window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_keypad_widgets(widgets_t *m)
{
	int		i, j;
	Arg		arg[10];
	char		btn_name[20];

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->keypad.form = XmCreateFormDialog(
		m->toplevel,
		"keypadForm",
		arg,
		i
	);

	/* Create label widget as keypad label */
	m->keypad.keypad_lbl = XmCreateLabel(
		m->keypad.form,
		"keypadLabel",
		NULL,
		0
	);

	/* Create label widget as keypad indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.keypad_ind = XmCreateLabel(
		m->keypad.form,
		"keypadIndicator",
		arg,
		i
	);

	/* Create pushbutton widgets as number keys */
	for (j = 0; j < 10; j++) {
		sprintf(btn_name, "keypadNumButton%u", j);

		i = 0;
		XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
		m->keypad.num_btn[j] = XmCreatePushButton(
			m->keypad.form,
			btn_name,
			arg,
			i
		);
	}

	/* Create pushbutton widget as clear button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.clear_btn = XmCreatePushButton(
		m->keypad.form,
		"keypadClearButton",
		arg,
		i
	);

	/* Create pushbutton widget as enter button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.enter_btn = XmCreatePushButton(
		m->keypad.form,
		"keypadEnterButton",
		arg,
		i
	);

	/* Create label widget as track warp label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.warp_lbl = XmCreateLabel(
		m->keypad.form,
		"trackWarpLabel",
		arg,
		i
	);

	/* Create scale widget for track warp slider */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.warp_scale = XmCreateScale(
		m->keypad.form,
		"trackWarpScale",
		arg,
		i
	);

	/* Create separator bar widget */
	m->keypad.keypad_sep = XmCreateSeparator(
		m->keypad.form,
		"keypadSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->keypad.cancel_btn = XmCreatePushButton(
		m->keypad.form,
		"keypadCancelButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->keypad.keypad_lbl);
	XtManageChild(m->keypad.keypad_ind);
	for (i = 0; i < 10; i++)
		XtManageChild(m->keypad.num_btn[i]);
	XtManageChild(m->keypad.clear_btn);
	XtManageChild(m->keypad.enter_btn);
	XtManageChild(m->keypad.warp_lbl);
	XtManageChild(m->keypad.warp_scale);
	XtManageChild(m->keypad.keypad_sep);
	XtManageChild(m->keypad.cancel_btn);
}


/*
 * create_options_widgets
 *	Create all widgets in the options window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_options_widgets(widgets_t *m)
{
	int		i, j;
	Arg		arg[10];
	curstat_t	*s = curstat_addr();

	options_cblist[0].closure = (XtPointer) s;

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->options.form = XmCreateFormDialog(
		m->toplevel,
		"optionsForm",
		arg,
		i
	);

	/* Create label widget as load options label */
	m->options.load_lbl = XmCreateLabel(
		m->options.form,
		"onLoadLabel",
		NULL,
		0
	);

	/* Create frame for load options check box */
	m->options.load_chkbox_frm = XmCreateFrame(
		m->options.form,
		"onLoadCheckBoxFrame",
		NULL,
		0
	);

	/* Create check box widget as load options checkbox */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 1); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.load_chkbox = XmCreateSimpleCheckBox(
		m->options.load_chkbox_frm,
		"onLoadCheckBox",
		arg,
		i
	);
	m->options.load_lock_btn =
		XtNameToWidget(m->options.load_chkbox, "button_0");

	/* Create frame for load options radio box */
	m->options.load_radbox_frm = XmCreateFrame(
		m->options.form,
		"onLoadRadioBoxFrame",
		NULL,
		0
	);

	/* Create radio box widget as load options selector */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 3); i++;
	XtSetArg(arg[i], XmNbuttonSet, 0); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.load_radbox = XmCreateSimpleRadioBox(
		m->options.load_radbox_frm,
		"onLoadRadioBox",
		arg,
		i
	);
	m->options.load_none_btn =
		XtNameToWidget(m->options.load_radbox, "button_0");
	m->options.load_spdn_btn =
		XtNameToWidget(m->options.load_radbox, "button_1");
	m->options.load_play_btn =
		XtNameToWidget(m->options.load_radbox, "button_2");

	/* Create label widget as exit options label */
	m->options.exit_lbl = XmCreateLabel(
		m->options.form,
		"onExitLabel",
		NULL,
		0
	);

	/* Create frame for exit options radio box */
	m->options.exit_radbox_frm = XmCreateFrame(
		m->options.form,
		"onExitRadioBoxFrame",
		NULL,
		0
	);

	/* Create radio box widget as exit options selector */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 3); i++;
	XtSetArg(arg[i], XmNbuttonSet, 0); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.exit_radbox = XmCreateSimpleRadioBox(
		m->options.exit_radbox_frm,
		"onExitRadioBox",
		arg,
		i
	);
	m->options.exit_none_btn =
		XtNameToWidget(m->options.exit_radbox, "button_0");
	m->options.exit_stop_btn =
		XtNameToWidget(m->options.exit_radbox, "button_1");
	m->options.exit_eject_btn =
		XtNameToWidget(m->options.exit_radbox, "button_2");

	/* Create label widget as done options label */
	m->options.done_lbl = XmCreateLabel(
		m->options.form,
		"onDoneLabel",
		NULL,
		0
	);

	/* Create frame for done options radio box */
	m->options.done_chkbox_frm = XmCreateFrame(
		m->options.form,
		"onDoneCheckBoxFrame",
		NULL,
		0
	);

	/* Create check box widget as done options checkbox */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 1); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.done_chkbox = XmCreateSimpleCheckBox(
		m->options.done_chkbox_frm,
		"onDoneCheckBox",
		arg,
		i
	);
	m->options.done_eject_btn =
		XtNameToWidget(m->options.done_chkbox, "button_0");

	/* Create label widget as eject options label */
	m->options.eject_lbl = XmCreateLabel(
		m->options.form,
		"onEjectLabel",
		NULL,
		0
	);

	/* Create frame for done options radio box */
	m->options.eject_chkbox_frm = XmCreateFrame(
		m->options.form,
		"onEjectCheckBoxFrame",
		NULL,
		0
	);

	/* Create check box widget as eject options checkbox */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 1); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.eject_chkbox = XmCreateSimpleCheckBox(
		m->options.eject_chkbox_frm,
		"onEjectCheckBox",
		arg,
		i
	);
	m->options.eject_exit_btn =
		XtNameToWidget(m->options.eject_chkbox, "button_0");

	/* Create label widget as channel route options label */
	m->options.chroute_lbl = XmCreateLabel(
		m->options.form,
		"channelRouteLabel",
		NULL,
		0
	);

	/* Create frame for channel route options radio box */
	m->options.chroute_radbox_frm = XmCreateFrame(
		m->options.form,
		"channelRouteRadioBoxFrame",
		NULL,
		0
	);

	/* Create radio box widget as channel route options selector */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 5); i++;
	XtSetArg(arg[i], XmNbuttonSet, 0); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.chroute_radbox = XmCreateSimpleRadioBox(
		m->options.chroute_radbox_frm,
		"channelRouteRadioBox",
		arg,
		i
	);
	m->options.chroute_stereo_btn =
		XtNameToWidget(m->options.chroute_radbox, "button_0");
	m->options.chroute_rev_btn =
		XtNameToWidget(m->options.chroute_radbox, "button_1");
	m->options.chroute_left_btn =
		XtNameToWidget(m->options.chroute_radbox, "button_2");
	m->options.chroute_right_btn =
		XtNameToWidget(m->options.chroute_radbox, "button_3");
	m->options.chroute_mono_btn =
		XtNameToWidget(m->options.chroute_radbox, "button_4");

	/* Create label widget as vol taper options label */
	m->options.vol_lbl = XmCreateLabel(
		m->options.form,
		"volTaperLabel",
		NULL,
		0
	);

	/* Create frame for vol taper options radio box */
	m->options.vol_radbox_frm = XmCreateFrame(
		m->options.form,
		"volTaperRadioBoxFrame",
		NULL,
		0
	);

	/* Create radio box widget as vol taper options selector */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 3); i++;
	XtSetArg(arg[i], XmNbuttonSet, 0); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, options_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.vol_radbox = XmCreateSimpleRadioBox(
		m->options.vol_radbox_frm,
		"volTaperRadioBox",
		arg,
		i
	);
	m->options.vol_linear_btn =
		XtNameToWidget(m->options.vol_radbox, "button_0");
	m->options.vol_square_btn =
		XtNameToWidget(m->options.vol_radbox, "button_1");
	m->options.vol_invsqr_btn =
		XtNameToWidget(m->options.vol_radbox, "button_2");

	/* Create label widget as balance control label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.bal_lbl = XmCreateLabel(
		m->options.form,
		"balanceLabel",
		arg,
		i
	);

	/* Create label widget as balance control L label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.ball_lbl = XmCreateLabel(
		m->options.form,
		"balanceLeftLabel",
		arg,
		i
	);

	/* Create label widget as balance control R label */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.balr_lbl = XmCreateLabel(
		m->options.form,
		"balanceRightLabel",
		arg,
		i
	);

	/* Create scale widget for balance control slider */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.bal_scale = XmCreateScale(
		m->options.form,
		"balanceScale",
		arg,
		i
	);

	/* Create pushbutton widget as balance center button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.balctr_btn = XmCreatePushButton(
		m->options.form,
		"balanceCenterButton",
		arg,
		i
	);

	/* Create separator bar widget */
	m->options.options_sep = XmCreateSeparator(
		m->options.form,
		"optionsSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as default button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.reset_btn = XmCreatePushButton(
		m->options.form,
		"resetButton",
		arg,
		i
	);

	/* Create pushbutton widget as OK button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->options.ok_btn = XmCreatePushButton(
		m->options.form,
		"okButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->options.load_lbl);
	XtManageChild(m->options.load_chkbox_frm);
	XtManageChild(m->options.load_chkbox);
	XtManageChild(m->options.load_radbox_frm);
	XtManageChild(m->options.load_radbox);
	XtManageChild(m->options.exit_lbl);
	XtManageChild(m->options.exit_radbox_frm);
	XtManageChild(m->options.exit_radbox);
	XtManageChild(m->options.done_lbl);
	XtManageChild(m->options.done_chkbox_frm);
	XtManageChild(m->options.done_chkbox);
	XtManageChild(m->options.eject_lbl);
	XtManageChild(m->options.eject_chkbox_frm);
	XtManageChild(m->options.eject_chkbox);
	XtManageChild(m->options.chroute_lbl);
	XtManageChild(m->options.chroute_radbox_frm);
	XtManageChild(m->options.chroute_radbox);
	XtManageChild(m->options.vol_lbl);
	XtManageChild(m->options.vol_radbox_frm);
	XtManageChild(m->options.vol_radbox);
	XtManageChild(m->options.bal_lbl);
	XtManageChild(m->options.ball_lbl);
	XtManageChild(m->options.balr_lbl);
	XtManageChild(m->options.bal_scale);
	XtManageChild(m->options.balctr_btn);
	XtManageChild(m->options.options_sep);
	XtManageChild(m->options.reset_btn);
	XtManageChild(m->options.ok_btn);
}


/*
 * create_dbprog_widgets
 *	Create all widgets in the database/program window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_dbprog_widgets(widgets_t *m)
{
	int		i;
	Arg		arg[10];
	curstat_t	*s = curstat_addr();

	dbprog_radbox_cblist[0].closure = (XtPointer) s;

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->dbprog.form = XmCreateFormDialog(
		m->toplevel,
		"dbprogForm",
		arg,
		i
	);

	/* Create label widget as logo */
	m->dbprog.logo_lbl = XmCreateLabel(
		m->dbprog.form,
		"logoLabel",
		NULL,
		0
	);

	/* Create pushbutton widget as about button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.about_btn = XmCreatePushButton(
		m->dbprog.form,
		"aboutButton",
		arg,
		i
	);

	/* Create label widget as disc title display/editor label */
	m->dbprog.dtitle_lbl = XmCreateLabel(
		m->dbprog.form,
		"discTitleLabel",
		NULL,
		0
	);

	/* Create label widget as disc ext descr label */
	m->dbprog.extd_lbl = XmCreateLabel(
		m->dbprog.form,
		"discLabel",
		NULL,
		0
	);

	/* Create text widget as disc title display/editor */
	i = 0;
	XtSetArg(arg[i], XmNeditable, True); i++;
	XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, True); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.dtitle_txt = XmCreateText(
		m->dbprog.form,
		"discTitleText",
		arg,
		i
	);

	/* Create pushbutton widget as disc title ext descr popup button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.extd_btn = XmCreatePushButton(
		m->dbprog.form,
		"extDiscInfoButton",
		arg,
		i
	);

	/* Create separator bar widget */
	m->dbprog.dbprog_sep1 = XmCreateSeparator(
		m->dbprog.form,
		"dbprogSeparator1",
		NULL,
		0
	);

	/* Create label widget as track list label */
	m->dbprog.trklist_lbl = XmCreateLabel(
		m->dbprog.form,
		"trackListLabel",
		NULL,
		0
	);

	/* Create scrolled list widget as track list */
	i = 0;
	XtSetArg(arg[i], XmNautomaticSelection, False); i++;
	XtSetArg(arg[i], XmNselectionPolicy, XmBROWSE_SELECT); i++;
	XtSetArg(arg[i], XmNlistSizePolicy, XmCONSTANT); i++;
	XtSetArg(arg[i], XmNscrollBarDisplayPolicy, XmSTATIC); i++;
	XtSetArg(arg[i], XmNscrolledWindowMarginWidth, 2); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.trk_list = XmCreateScrolledList(
		m->dbprog.form,
		"trackList",
		arg,
		i
	);

	/* Create label widget as time mode selector label */
	m->dbprog.radio_lbl = XmCreateLabel(
		m->dbprog.form,
		"timeSelectLabel",
		NULL,
		0
	);

	/* Create frame for radio box */
	m->dbprog.radio_frm = XmCreateFrame(
		m->dbprog.form,
		"timeSelectFrame",
		NULL,
		0
	);

	/* Create radio box widget as time mode selector */
	i = 0;
	XtSetArg(arg[i], XmNbuttonCount, 2); i++;
	XtSetArg(arg[i], XmNbuttonSet, 1); i++;
	XtSetArg(arg[i], XmNspacing, 1); i++;
	XtSetArg(arg[i], XmNmarginHeight, 3); i++;
	XtSetArg(arg[i], XmNentryCallback, dbprog_radbox_cblist); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.radio_box = XmCreateSimpleRadioBox(
		m->dbprog.radio_frm,
		"timeSelectBox",
		arg,
		i
	);
	m->dbprog.tottime_btn = XtNameToWidget(m->dbprog.radio_box, "button_0");
	m->dbprog.trktime_btn = XtNameToWidget(m->dbprog.radio_box, "button_1");

	/* Create label widget as disc ID indicator label */
	m->dbprog.discid_lbl = XmCreateLabel(
		m->dbprog.form,
		"discIdLabel",
		NULL,
		0
	);

	/* Create frame for disc ID indicator */
	m->dbprog.discid_frm = XmCreateFrame(
		m->dbprog.form,
		"discIdFrame",
		NULL,
		0
	);

	/* Create label widget as disc ID indicator */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.discid_ind = XmCreateLabel(
		m->dbprog.discid_frm,
		"discIdIndicator",
		arg,
		i
	);

	/* Create label widget as track title display/editor label */
	m->dbprog.ttitle_lbl = XmCreateLabel(
		m->dbprog.form,
		"trackTitleLabel",
		NULL,
		0
	);

	/* Create label widget as disc ext descr label */
	m->dbprog.extt_lbl = XmCreateLabel(
		m->dbprog.form,
		"trackLabel",
		NULL,
		0
	);

	/* Create text widget as track title display/editor */
	i = 0;
	XtSetArg(arg[i], XmNeditable, True); i++;
	XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, True); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.ttitle_txt = XmCreateText(
		m->dbprog.form,
		"trackTitleText",
		arg,
		i
	);

	/* Create pushbutton widget as track title ext descr popup button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.extt_btn = XmCreatePushButton(
		m->dbprog.form,
		"extTrackInfoButton",
		arg,
		i
	);

	/* Create label widget as program pushbuttons label */
	m->dbprog.pgm_lbl = XmCreateLabel(
		m->dbprog.form,
		"programLabel",
		NULL,
		0
	);

	/* Create pushbutton widget as Add PGM button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.addpgm_btn = XmCreatePushButton(
		m->dbprog.form,
		"addProgramButton",
		arg,
		i
	);

	/* Create pushbutton widget as Clear PGM button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.clrpgm_btn = XmCreatePushButton(
		m->dbprog.form,
		"clearProgramButton",
		arg,
		i
	);

	/* Create label widget as program sequence display/editor label */
	m->dbprog.pgmseq_lbl = XmCreateLabel(
		m->dbprog.form,
		"programSequenceLabel",
		NULL,
		0
	);

	/* Create text widget as program sequence display/editor */
	i = 0;
	XtSetArg(arg[i], XmNeditable, True); i++;
	XtSetArg(arg[i], XmNeditMode, XmSINGLE_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, True); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.pgmseq_txt = XmCreateText(
		m->dbprog.form,
		"programSequenceText",
		arg,
		i
	);

	/* Create pushbutton widget as Send CDDB button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.send_btn = XmCreatePushButton(
		m->dbprog.form,
		"sendButton",
		arg,
		i
	);

	/* Create separator bar widget */
	m->dbprog.dbprog_sep2 = XmCreateSeparator(
		m->dbprog.form,
		"dbprogSeparator2",
		NULL,
		0
	);

	/* Create pushbutton widget as Save DB button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.savedb_btn = XmCreatePushButton(
		m->dbprog.form,
		"saveDatabaseButton",
		arg,
		i
	);

	/* Create pushbutton widget as Link DB button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.linkdb_btn = XmCreatePushButton(
		m->dbprog.form,
		"linkDatabaseButton",
		arg,
		i
	);

	/* Create pushbutton widget as Load DB button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.loaddb_btn = XmCreatePushButton(
		m->dbprog.form,
		"loadDatabaseButton",
		arg,
		i
	);

	/* Create pushbutton widget as Cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbprog.cancel_btn = XmCreatePushButton(
		m->dbprog.form,
		"dbprogCancelButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->dbprog.logo_lbl);
	XtManageChild(m->dbprog.about_btn);
	XtManageChild(m->dbprog.dtitle_lbl);
	XtManageChild(m->dbprog.dtitle_txt);
	XtManageChild(m->dbprog.extd_lbl);
	XtManageChild(m->dbprog.extd_btn);
	XtManageChild(m->dbprog.dbprog_sep1);
	XtManageChild(m->dbprog.trklist_lbl);
	XtManageChild(m->dbprog.trk_list);
	XtManageChild(m->dbprog.radio_lbl);
	XtManageChild(m->dbprog.radio_frm);
	XtManageChild(m->dbprog.radio_box);
	XtManageChild(m->dbprog.discid_lbl);
	XtManageChild(m->dbprog.discid_frm);
	XtManageChild(m->dbprog.discid_ind);
	XtManageChild(m->dbprog.ttitle_lbl);
	XtManageChild(m->dbprog.ttitle_txt);
	XtManageChild(m->dbprog.extt_lbl);
	XtManageChild(m->dbprog.extt_btn);
	XtManageChild(m->dbprog.pgm_lbl);
	XtManageChild(m->dbprog.addpgm_btn);
	XtManageChild(m->dbprog.clrpgm_btn);
	XtManageChild(m->dbprog.pgmseq_lbl);
	XtManageChild(m->dbprog.pgmseq_txt);
	XtManageChild(m->dbprog.send_btn);
	XtManageChild(m->dbprog.dbprog_sep2);
	XtManageChild(m->dbprog.savedb_btn);
	XtManageChild(m->dbprog.linkdb_btn);
	XtManageChild(m->dbprog.loaddb_btn);
	XtManageChild(m->dbprog.cancel_btn);
}


/*
 * create_exttxt_widgets
 *	Create all widgets in the extended disc information and
 *	extended track information windows.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_exttxt_widgets(widgets_t *m)
{
	int	i;
	Arg	arg[10];

	/* Extended disc info window */

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->dbextd.form = XmCreateFormDialog(
		m->dbprog.extd_btn,
		"extDiscInfoForm",
		arg,
		i
	);

	/* Create label widget as extended disc info label */
	m->dbextd.disc_lbl = XmCreateLabel(
		m->dbextd.form,
		"extDiscInfoLabel",
		NULL,
		0
	);

	/* Create text widget as extended disc info editor/viewer */
	i = 0;
	XtSetArg(arg[i], XmNeditable, True); i++;
	XtSetArg(arg[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, True); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextd.disc_txt = XmCreateScrolledText(
		m->dbextd.form,
		"extDiscInfoText",
		arg,
		i
	);

	/* Create separator bar widget */
	m->dbextd.dbextd_sep = XmCreateSeparator(
		m->dbextd.form,
		"extDiscInfoSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as OK button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextd.ok_btn = XmCreatePushButton(
		m->dbextd.form,
		"extDiscInfoOkButton",
		arg,
		i
	);

	/* Create pushbutton widget as Clear button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextd.clear_btn = XmCreatePushButton(
		m->dbextd.form,
		"extDiscInfoClearButton",
		arg,
		i
	);

	/* Create pushbutton widget as Cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextd.cancel_btn = XmCreatePushButton(
		m->dbextd.form,
		"extDiscInfoCancelButton",
		arg,
		i
	);

	/* Extended track info window */

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->dbextt.form = XmCreateFormDialog(
		m->dbprog.extt_btn,
		"extTrackInfoForm",
		arg,
		i
	);

	/* Create label widget as extended disc info label */
	m->dbextt.trk_lbl = XmCreateLabel(
		m->dbextt.form,
		"extTrackInfoLabel",
		NULL,
		0
	);

	/* Create text widget as extended disc info editor/viewer */
	i = 0;
	XtSetArg(arg[i], XmNeditable, True); i++;
	XtSetArg(arg[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, True); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextt.trk_txt = XmCreateScrolledText(
		m->dbextt.form,
		"extTrackInfoText",
		arg,
		i
	);

	/* Create separator bar widget */
	m->dbextt.dbextt_sep = XmCreateSeparator(
		m->dbextt.form,
		"extTrackInfoSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as OK button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextt.ok_btn = XmCreatePushButton(
		m->dbextt.form,
		"extTrackInfoOkButton",
		arg,
		i
	);

	/* Create pushbutton widget as Clear button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextt.clear_btn = XmCreatePushButton(
		m->dbextt.form,
		"extTrackInfoClearButton",
		arg,
		i
	);

	/* Create pushbutton widget as Cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dbextt.cancel_btn = XmCreatePushButton(
		m->dbextt.form,
		"extTrackInfoCancelButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->dbextd.disc_lbl);
	XtManageChild(m->dbextd.disc_txt);
	XtManageChild(m->dbextd.dbextd_sep);
	XtManageChild(m->dbextd.ok_btn);
	XtManageChild(m->dbextd.clear_btn);
	XtManageChild(m->dbextd.cancel_btn);

	XtManageChild(m->dbextt.trk_lbl);
	XtManageChild(m->dbextt.trk_txt);
	XtManageChild(m->dbextt.dbextt_sep);
	XtManageChild(m->dbextt.ok_btn);
	XtManageChild(m->dbextt.clear_btn);
	XtManageChild(m->dbextt.cancel_btn);
}


/*
 * create_dirsel_widgets
 *	Create all widgets in the CD database directory selector window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_dirsel_widgets(widgets_t *m)
{
	int	i;
	Arg	arg[10];

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
	m->dirsel.form = XmCreateFormDialog(
		m->dbprog.savedb_btn,
		"dirSelectForm",
		arg,
		i
	);

	/* Create label widget as database directory selector label */
	m->dirsel.dir_lbl = XmCreateLabel(
		m->dirsel.form,
		"dirSelectLabel",
		NULL,
		0
	);

	/* Create scrolled list widget as directory list */
	i = 0;
	XtSetArg(arg[i], XmNautomaticSelection, False); i++;
	XtSetArg(arg[i], XmNselectionPolicy, XmBROWSE_SELECT); i++;
	XtSetArg(arg[i], XmNlistSizePolicy, XmCONSTANT); i++;
	XtSetArg(arg[i], XmNscrollBarDisplayPolicy, XmSTATIC); i++;
	XtSetArg(arg[i], XmNscrolledWindowMarginWidth, 2); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dirsel.dir_list = XmCreateScrolledList(
		m->dirsel.form,
		"dirSelectList",
		arg,
		i
	);

	/* Create separator bar widget */
	m->dirsel.dirsel_sep = XmCreateSeparator(
		m->dirsel.form,
		"dirSelectSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as OK button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dirsel.ok_btn = XmCreatePushButton(
		m->dirsel.form,
		"dirSelectOkButton",
		arg,
		i
	);

	/* Create pushbutton widget as Cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->dirsel.cancel_btn = XmCreatePushButton(
		m->dirsel.form,
		"dirSelectCancelButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->dirsel.dir_lbl);
	XtManageChild(m->dirsel.dir_list);
	XtManageChild(m->dirsel.dirsel_sep);
	XtManageChild(m->dirsel.ok_btn);
	XtManageChild(m->dirsel.cancel_btn);
}


/*
 * create_linksel_widgets
 *	Create all widgets in the CD database search-link window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_linksel_widgets(widgets_t *m)
{
	int	i;
	Arg	arg[10];

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
	m->linksel.form = XmCreateFormDialog(
		m->dbprog.linkdb_btn,
		"linkSelectForm",
		arg,
		i
	);

	/* Create label widget as cddb link selector label */
	m->linksel.link_lbl = XmCreateLabel(
		m->linksel.form,
		"linkSelectLabel",
		NULL,
		0
	);

	/* Create scrolled list widget as disc titles list */
	i = 0;
	XtSetArg(arg[i], XmNautomaticSelection, False); i++;
	XtSetArg(arg[i], XmNselectionPolicy, XmBROWSE_SELECT); i++;
	XtSetArg(arg[i], XmNlistSizePolicy, XmCONSTANT); i++;
	XtSetArg(arg[i], XmNscrollBarDisplayPolicy, XmSTATIC); i++;
	XtSetArg(arg[i], XmNscrolledWindowMarginWidth, 2); i++;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->linksel.link_list = XmCreateScrolledList(
		m->linksel.form,
		"linkSelectList",
		arg,
		i
	);

	/* Create separator bar widget */
	m->linksel.linksel_sep = XmCreateSeparator(
		m->linksel.form,
		"linkSelectSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as OK button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->linksel.ok_btn = XmCreatePushButton(
		m->linksel.form,
		"linkSelectOkButton",
		arg,
		i
	);

	/* Create pushbutton widget as Cancel button */
	i = 0;
	XtSetArg(arg[i], XmNhelpCallback, help_cblist); i++;
	m->linksel.cancel_btn = XmCreatePushButton(
		m->linksel.form,
		"linkSelectCancelButton",
		arg,
		i
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->linksel.link_lbl);
	XtManageChild(m->linksel.link_list);
	XtManageChild(m->linksel.linksel_sep);
	XtManageChild(m->linksel.ok_btn);
	XtManageChild(m->linksel.cancel_btn);
}


/*
 * create_help_widgets
 *	Create all widgets in the help text display window.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_help_widgets(widgets_t *m)
{
	int	i;
	Arg	arg[10];

	/* Help popup window */

	/* Create form widget as container */
	i = 0;
	XtSetArg(arg[i], XmNautoUnmanage, False); i++;
	XtSetArg(arg[i], XmNresizePolicy, XmRESIZE_NONE); i++;
	m->help.form = XmCreateFormDialog(
		m->main.form,
		"helpForm",
		arg,
		i
	);

	/* Create text widget as help text viewer */
	i = 0;
	XtSetArg(arg[i], XmNeditable, False); i++;
	XtSetArg(arg[i], XmNeditMode, XmMULTI_LINE_EDIT); i++;
	XtSetArg(arg[i], XmNcursorPositionVisible, False); i++;
	XtSetArg(arg[i], XmNcursorPosition, 0); i++;
	m->help.help_txt = XmCreateScrolledText(
		m->help.form,
		"helpText",
		arg,
		i
	);

	/* Create separator bar widget */
	m->help.help_sep = XmCreateSeparator(
		m->help.form,
		"helpSeparator",
		NULL,
		0
	);

	/* Create pushbutton widget as OK button */
	m->help.ok_btn = XmCreatePushButton(
		m->help.form,
		"helpOkButton",
		NULL,
		0
	);

	/* Manage the widgets (except the form) */
	XtManageChild(m->help.help_txt);
	XtManageChild(m->help.help_sep);
	XtManageChild(m->help.ok_btn);
}


/*
 * create_dialog_widgets
 *	Create all widgets in the dialog box windows.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
create_dialog_widgets(widgets_t *m)
{
	int	i;
	Arg	arg[10];
	Widget	help_btn,
		cancel_btn;

	/* Create info dialog widget for information messages */
	i = 0;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_MODELESS); i++;
	m->dialog.info = XmCreateInformationDialog(
		m->toplevel,
		"infoPopup",
		arg,
		i
	);

	/* Remove unused buttons in the info dialog widget */
	help_btn = XmMessageBoxGetChild(
		m->dialog.info,
		XmDIALOG_HELP_BUTTON
	);
	cancel_btn = XmMessageBoxGetChild(
		m->dialog.info,
		XmDIALOG_CANCEL_BUTTON
	);

	XtUnmanageChild(help_btn);
	XtUnmanageChild(cancel_btn);

	/* Create warning dialog widget for warning messages */
	i = 0;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
	m->dialog.warning = XmCreateWarningDialog(
		m->toplevel,
		"warningPopup",
		arg,
		i
	);

	/* Remove unused buttons in the warning dialog widget */
	help_btn = XmMessageBoxGetChild(
		m->dialog.warning,
		XmDIALOG_HELP_BUTTON
	);
	cancel_btn = XmMessageBoxGetChild(
		m->dialog.warning,
		XmDIALOG_CANCEL_BUTTON
	);

	XtUnmanageChild(help_btn);
	XtUnmanageChild(cancel_btn);

	/* Create error dialog widget for fatal error messages */
	i = 0;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
	m->dialog.fatal = XmCreateErrorDialog(
		m->toplevel,
		"fatalPopup",
		arg,
		i
	);

	/* Remove unused buttons in the error dialog widget */
	help_btn = XmMessageBoxGetChild(
		m->dialog.fatal,
		XmDIALOG_HELP_BUTTON
	);
	cancel_btn = XmMessageBoxGetChild(
		m->dialog.fatal,
		XmDIALOG_CANCEL_BUTTON
	);

	XtUnmanageChild(help_btn);
	XtUnmanageChild(cancel_btn);

	/* Create question dialog widget for confirm messages */
	i = 0;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
	m->dialog.confirm = XmCreateQuestionDialog(
		m->toplevel,
		"questionPopup",
		arg,
		i
	);

	/* Remove unused buttons in the question dialog widget */
	help_btn = XmMessageBoxGetChild(
		m->dialog.confirm,
		XmDIALOG_HELP_BUTTON
	);

	XtUnmanageChild(help_btn);

	/* Create info dialog widget for the About popup */
	i = 0;
	XtSetArg(arg[i], XmNdialogStyle, XmDIALOG_MODELESS); i++;
	m->dialog.about = XmCreateInformationDialog(
		m->toplevel,
		"aboutPopup",
		arg,
		i
	);

	/* Remove unused buttons in the about popup */
	help_btn = XmMessageBoxGetChild(
		m->dialog.about,
		XmDIALOG_HELP_BUTTON
	);
	cancel_btn = XmMessageBoxGetChild(
		m->dialog.about,
		XmDIALOG_CANCEL_BUTTON
	);

	XtUnmanageChild(help_btn);
	XtUnmanageChild(cancel_btn);
}


/*
 * make_pixmaps
 *	Create pixmaps from bitmap data and set up various widgets to
 *	use them.
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure
 *	p - The main pixmaps placeholder structure
 *	depth - The desired depth of the pixmap
 *
 * Return:
 *	Nothing.
 */
STATIC void
make_pixmaps(widgets_t *m, pixmaps_t *p, int depth)
{
	/* Set icon pixmap */
	p->main.icon_pixmap = bm_to_px(
		m->toplevel,
		logo_bits,
		logo_width,
		logo_height,
		1,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->toplevel, XmNiconPixmap, p->main.icon_pixmap, NULL);
					
	/*
	 * The following puts proper pixmaps on button faces
	 */

	p->main.btnlbl_pixmap = bm_to_px(
		m->main.check_box,
		btnlbl_bits,
		btnlbl_width,
		btnlbl_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->main.btnlbl_btn,
		XmNlabelPixmap, p->main.btnlbl_pixmap,
		NULL
	);

	p->main.lock_pixmap = bm_to_px(
		m->main.check_box,
		lock_bits,
		lock_width,
		lock_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->main.lock_btn,
		XmNlabelPixmap, p->main.lock_pixmap,
		NULL
	);

	p->main.repeat_pixmap = bm_to_px(
		m->main.check_box,
		repeat_bits,
		repeat_width,
		repeat_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->main.repeat_btn,
		XmNlabelPixmap, p->main.repeat_pixmap,
		NULL
	);

	p->main.shuffle_pixmap = bm_to_px(
		m->main.check_box,
		shuffle_bits,
		shuffle_width,
		shuffle_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->main.shuffle_btn,
		XmNlabelPixmap, p->main.shuffle_pixmap,
		NULL
	);

	p->main.eject_pixmap = bm_to_px(
		m->main.eject_btn,
		eject_bits,
		eject_width,
		eject_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.eject_hlpixmap = bm_to_px(
		m->main.eject_btn,
		eject_bits,
		eject_width,
		eject_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.eject_btn,
		XmNlabelPixmap, p->main.eject_pixmap,
		NULL
	);

	p->main.poweroff_pixmap = bm_to_px(
		m->main.poweroff_btn,
		poweroff_bits,
		poweroff_width,
		poweroff_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.poweroff_hlpixmap = bm_to_px(
		m->main.poweroff_btn,
		poweroff_bits,
		poweroff_width,
		poweroff_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.poweroff_btn,
		XmNlabelPixmap, p->main.poweroff_pixmap,
		NULL
	);

	p->main.dbprog_pixmap = bm_to_px(
		m->main.dbprog_btn,
		dbprog_bits,
		dbprog_width,
		dbprog_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.dbprog_hlpixmap = bm_to_px(
		m->main.dbprog_btn,
		dbprog_bits,
		dbprog_width,
		dbprog_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.dbprog_btn,
		XmNlabelPixmap, p->main.dbprog_pixmap,
		NULL
	);

	p->main.options_pixmap = bm_to_px(
		m->main.options_btn,
		options_bits,
		options_width,
		options_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.options_hlpixmap = bm_to_px(
		m->main.options_btn,
		options_bits,
		options_width,
		options_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.options_btn,
		XmNlabelPixmap, p->main.options_pixmap,
		NULL
	);

	p->main.time_pixmap = bm_to_px(
		m->main.time_btn,
		time_bits,
		time_width,
		time_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.time_hlpixmap = bm_to_px(
		m->main.time_btn,
		time_bits,
		time_width,
		time_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.time_btn,
		XmNlabelPixmap, p->main.time_pixmap,
		NULL
	);

	p->main.ab_pixmap = bm_to_px(
		m->main.ab_btn,
		ab_bits,
		ab_width,
		ab_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.ab_hlpixmap = bm_to_px(
		m->main.ab_btn,
		ab_bits,
		ab_width,
		ab_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.ab_btn,
		XmNlabelPixmap, p->main.ab_pixmap,
		NULL
	);

	p->main.sample_pixmap = bm_to_px(
		m->main.sample_btn,
		sample_bits,
		sample_width,
		sample_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.sample_hlpixmap = bm_to_px(
		m->main.sample_btn,
		sample_bits,
		sample_width,
		sample_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.sample_btn,
		XmNlabelPixmap, p->main.sample_pixmap,
		NULL
	);

	p->main.keypad_pixmap = bm_to_px(
		m->main.keypad_btn,
		keypad_bits,
		keypad_width,
		keypad_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.keypad_hlpixmap = bm_to_px(
		m->main.keypad_btn,
		keypad_bits,
		keypad_width,
		keypad_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.keypad_btn,
		XmNlabelPixmap, p->main.keypad_pixmap,
		NULL
	);

	p->main.help_pixmap = bm_to_px(
		m->main.help_btn,
		help_bits,
		help_width,
		help_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.help_hlpixmap = bm_to_px(
		m->main.help_btn,
		help_bits,
		help_width,
		help_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.help_btn,
		XmNlabelPixmap, p->main.help_pixmap,
		NULL
	);

	p->main.playpause_pixmap = bm_to_px(
		m->main.playpause_btn,
		playpause_bits,
		playpause_width,
		playpause_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.playpause_hlpixmap = bm_to_px(
		m->main.playpause_btn,
		playpause_bits,
		playpause_width,
		playpause_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.playpause_btn,
		XmNlabelPixmap, p->main.playpause_pixmap,
		NULL
	);

	p->main.stop_pixmap = bm_to_px(
		m->main.stop_btn,
		stop_bits,
		stop_width,
		stop_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.stop_hlpixmap = bm_to_px(
		m->main.stop_btn,
		stop_bits,
		stop_width,
		stop_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.stop_btn,
		XmNlabelPixmap, p->main.stop_pixmap,
		NULL
	);

	p->main.prevtrk_pixmap = bm_to_px(
		m->main.prevtrk_btn,
		prevtrk_bits,
		prevtrk_width,
		prevtrk_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.prevtrk_hlpixmap = bm_to_px(
		m->main.prevtrk_btn,
		prevtrk_bits,
		prevtrk_width,
		prevtrk_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.prevtrk_btn,
		XmNlabelPixmap, p->main.prevtrk_pixmap,
		NULL
	);

	p->main.nexttrk_pixmap = bm_to_px(
		m->main.nexttrk_btn,
		nexttrk_bits,
		nexttrk_width,
		nexttrk_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.nexttrk_hlpixmap = bm_to_px(
		m->main.nexttrk_btn,
		nexttrk_bits,
		nexttrk_width,
		nexttrk_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.nexttrk_btn,
		XmNlabelPixmap, p->main.nexttrk_pixmap,
		NULL
	);

	p->main.previdx_pixmap = bm_to_px(
		m->main.previdx_btn,
		previdx_bits,
		previdx_width,
		previdx_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.previdx_hlpixmap = bm_to_px(
		m->main.previdx_btn,
		previdx_bits,
		previdx_width,
		previdx_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.previdx_btn,
		XmNlabelPixmap, p->main.previdx_pixmap,
		NULL
	);

	p->main.nextidx_pixmap = bm_to_px(
		m->main.nextidx_btn,
		nextidx_bits,
		nextidx_width,
		nextidx_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.nextidx_hlpixmap = bm_to_px(
		m->main.nextidx_btn,
		nextidx_bits,
		nextidx_width,
		nextidx_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.nextidx_btn,
		XmNlabelPixmap, p->main.nextidx_pixmap,
		NULL
	);

	p->main.rew_pixmap = bm_to_px(
		m->main.rew_btn,
		rew_bits,
		rew_width,
		rew_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.rew_hlpixmap = bm_to_px(
		m->main.rew_btn,
		rew_bits,
		rew_width,
		rew_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.rew_btn,
		XmNlabelPixmap, p->main.rew_pixmap,
		NULL
	);

	p->main.ff_pixmap = bm_to_px(
		m->main.ff_btn,
		ff_bits,
		ff_width,
		ff_height,
		depth,
		BM_PX_NORMAL
	);
	p->main.ff_hlpixmap = bm_to_px(
		m->main.ff_btn,
		ff_bits,
		ff_width,
		ff_height,
		depth,
		BM_PX_HIGHLIGHT
	);
	XtVaSetValues(m->main.ff_btn,
		XmNlabelPixmap, p->main.ff_pixmap,
		NULL
	);

	p->dbprog.logo_pixmap = bm_to_px(
		m->main.dbprog_btn,
		logo_bits,
		logo_width,
		logo_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->dbprog.logo_lbl,
		XmNlabelType, XmPIXMAP,
		XmNlabelPixmap, p->dbprog.logo_pixmap,
		XmNlabelInsensitivePixmap, p->dbprog.logo_pixmap,
		NULL
	);

	p->dialog.xmcd_pixmap = bm_to_px(
		m->main.dtitle_ind,
		xmcd_bits,
		xmcd_width,
		xmcd_height,
		depth,
		BM_PX_NORMAL
	);
	XtVaSetValues(m->dialog.about,
		XmNsymbolPixmap, p->dialog.xmcd_pixmap,
		NULL
	);
}


/*
 * register_main_callbacks
 *	Register all callback routines for widgets in the main window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_main_callbacks(widgets_t *m, curstat_t *s)
{
	/* Main window callbacks */
	XtAddCallback(
		m->main.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->main.form
	);

	XtAddCallback(
		m->main.eject_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_load_eject,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.eject_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.poweroff_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_poweroff,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.poweroff_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.dbprog_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_dbprog,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.dbprog_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.options_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_options_popup,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.options_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.time_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_time,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.time_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.ab_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_ab,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.ab_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.sample_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_sample,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.sample_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.keypad_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_keypad_popup,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.keypad_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.help_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_help_popup,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.help_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.level_scale,
		XmNvalueChangedCallback,
		(XtCallbackProc) cd_level,
		(XtPointer) s
	);
	XtAddCallback(
		m->main.level_scale,
		XmNdragCallback,
		(XtCallbackProc) cd_level,
		(XtPointer) s
	);

	XtAddCallback(
		m->main.playpause_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_play_pause,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.playpause_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.stop_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_stop,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.stop_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.prevtrk_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_prevtrk,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.prevtrk_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.nexttrk_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_nexttrk,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.nexttrk_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.previdx_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_previdx,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.previdx_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.nextidx_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_nextidx,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.nextidx_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.rew_btn,
		XmNarmCallback,
		(XtCallbackProc) cd_rew,
		(XtPointer) s
	);
	XtAddCallback(
		m->main.rew_btn,
		XmNdisarmCallback,
		(XtCallbackProc) cd_rew,
		(XtPointer) s
	);
	XtAddCallback(
		m->main.rew_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_rew,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.rew_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	XtAddCallback(
		m->main.ff_btn,
		XmNarmCallback,
		(XtCallbackProc) cd_ff,
		(XtPointer) s
	);
	XtAddCallback(
		m->main.ff_btn,
		XmNdisarmCallback,
		(XtCallbackProc) cd_ff,
		(XtPointer) s
	);
	XtAddCallback(
		m->main.ff_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_ff,
		(XtPointer) s
	);
	XtAddEventHandler(
		m->main.ff_btn,
		FocusChangeMask,
		False,
		(XtEventHandler) cd_btn_focus_chg,
		(XtPointer) NULL
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		m->toplevel,
		delw,
		(XtCallbackProc) cd_exit,
		(XtPointer) s
	);
}


/*
 * register_keypad_callbacks
 *	Register all callback routines for widgets in the keypad window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_keypad_callbacks(widgets_t *m, curstat_t *s)
{
	int	i;

	XtAddCallback(
		m->keypad.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->keypad.form
	);
	/* Keypad popup callbacks */
	for (i = 0; i < 10; i++) {
		XtAddCallback(
			m->keypad.num_btn[i],
			XmNactivateCallback,
			(XtCallbackProc) cd_keypad_num,
			(XtPointer) i
		);
	}
	XtAddCallback(
		m->keypad.clear_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_keypad_clear,
		(XtPointer) s
	);
	XtAddCallback(
		m->keypad.enter_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_keypad_enter,
		(XtPointer) s
	);
	XtAddCallback(
		m->keypad.warp_scale,
		XmNvalueChangedCallback,
		(XtCallbackProc) cd_warp,
		(XtPointer) s
	);
	XtAddCallback(
		m->keypad.warp_scale,
		XmNdragCallback,
		(XtCallbackProc) cd_warp,
		(XtPointer) s
	);
	XtAddCallback(
		m->keypad.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_keypad_popdown,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->keypad.form),
		delw,
		(XtCallbackProc) cd_keypad_popdown,
		(XtPointer) s
	);
}


/*
 * register_options_callbacks
 *	Register all callback routines for widgets in the options window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_options_callbacks(widgets_t *m, curstat_t *s)
{
	XtAddCallback(
		m->options.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->options.form
	);

	XtAddCallback(
		m->options.bal_scale,
		XmNvalueChangedCallback,
		(XtCallbackProc) cd_balance,
		(XtPointer) s
	);
	XtAddCallback(
		m->options.bal_scale,
		XmNdragCallback,
		(XtCallbackProc) cd_balance,
		(XtPointer) s
	);

	XtAddCallback(
		m->options.balctr_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_balance_center,
		(XtPointer) s
	);

	XtAddCallback(
		m->options.reset_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_options_reset,
		(XtPointer) s
	);

	XtAddCallback(
		m->options.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_options_popdown,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->options.form),
		delw,
		(XtCallbackProc) cd_options_popdown,
		(XtPointer) s
	);
}


/*
 * register_dbprog_callbacks
 *	Register all callback routines for widgets in the
 *	database/program window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_dbprog_callbacks(widgets_t *m, curstat_t *s)
{
	/* Database/program popup callbacks */
	XtAddCallback(
		m->dbprog.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->dbprog.form
	);
	XtAddCallback(
		m->dbprog.about_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_about,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.dtitle_txt,
		XmNvalueChangedCallback,
		(XtCallbackProc) dbprog_dtitle_new,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.dtitle_txt,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_dtitle_new,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.extd_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extd,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.trk_list,
		XmNdefaultActionCallback,
		(XtCallbackProc) dbprog_trklist_play,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.trk_list,
		XmNbrowseSelectionCallback,
		(XtCallbackProc) dbprog_trklist_select,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.ttitle_txt,
		XmNvalueChangedCallback,
		(XtCallbackProc) dbprog_ttitle_new,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.ttitle_txt,
		XmNfocusCallback,
		(XtCallbackProc) dbprog_ttitle_focuschg,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.ttitle_txt,
		XmNlosingFocusCallback,
		(XtCallbackProc) dbprog_ttitle_focuschg,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.ttitle_txt,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_ttitle_new,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.extt_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extt,
		(XtPointer) TRUE
	);
	XtAddCallback(
		m->dbprog.pgmseq_txt,
		XmNmodifyVerifyCallback,
		(XtCallbackProc) dbprog_pgmseq_verify,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.pgmseq_txt,
		XmNvalueChangedCallback,
		(XtCallbackProc) dbprog_pgmseq_txtchg,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.pgmseq_txt,
		XmNfocusCallback,
		(XtCallbackProc) dbprog_pgmseq_focuschg,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.pgmseq_txt,
		XmNlosingFocusCallback,
		(XtCallbackProc) dbprog_pgmseq_focuschg,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.addpgm_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_addpgm,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.clrpgm_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_clrpgm,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.send_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_send,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.savedb_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_savedb,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.linkdb_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_link,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.loaddb_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_loaddb,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbprog.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_cancel,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->dbprog.form),
		delw,
		(XtCallbackProc) dbprog_cancel,
		(XtPointer) s
	);
}


/*
 * register_extd_callbacks
 *	Register all callback routines for widgets in the extended
 *	disc information window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_extd_callbacks(widgets_t *m, curstat_t *s)
{
	XtAddCallback(
		m->dbextd.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->dbextd.form
	);
	XtAddCallback(
		m->dbextd.disc_txt,
		XmNvalueChangedCallback,
		(XtCallbackProc) dbprog_set_changed,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextd.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extd_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextd.clear_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extd_clear,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextd.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extd_cancel,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->dbextd.form),
		delw,
		(XtCallbackProc) dbprog_extd_cancel,
		(XtPointer) s
	);
}


/*
 * register_extt_callbacks
 *	Register all callback routines for widgets in the extended
 *	track information window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_extt_callbacks(widgets_t *m, curstat_t *s)
{
	XtAddCallback(
		m->dbextt.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->dbextt.form
	);
	XtAddCallback(
		m->dbextt.trk_txt,
		XmNvalueChangedCallback,
		(XtCallbackProc) dbprog_set_changed,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextt.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extt_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextt.clear_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extt_clear,
		(XtPointer) s
	);
	XtAddCallback(
		m->dbextt.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_extt_cancel,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->dbextt.form),
		delw,
		(XtCallbackProc) dbprog_extt_cancel,
		(XtPointer) s
	);
}


/*
 * register_dirsel_callbacks
 *	Register all callback routines for widgets in the CD database
 *	directory selector window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_dirsel_callbacks(widgets_t *m, curstat_t *s)
{
	/* Directory selector popup callbacks */
	XtAddCallback(
		m->dirsel.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->dirsel.form
	);
	XtAddCallback(
		m->dirsel.dir_list,
		XmNdefaultActionCallback,
		(XtCallbackProc) dbprog_dirsel_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->dirsel.dir_list,
		XmNbrowseSelectionCallback,
		(XtCallbackProc) dbprog_dirsel_select,
		(XtPointer) s
	);
	XtAddCallback(
		m->dirsel.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_dirsel_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->dirsel.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_dirsel_cancel,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->dirsel.form),
		delw,
		(XtCallbackProc) dbprog_dirsel_cancel,
		(XtPointer) s
	);
}


/*
 * register_linksel_callbacks
 *	Register all callback routines for widgets in the CD database
 *	search-link window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_linksel_callbacks(widgets_t *m, curstat_t *s)
{
	/* Link selector popup callbacks */
	XtAddCallback(
		m->linksel.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->linksel.form
	);
	XtAddCallback(
		m->linksel.link_list,
		XmNdefaultActionCallback,
		(XtCallbackProc) dbprog_linksel_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->linksel.link_list,
		XmNbrowseSelectionCallback,
		(XtCallbackProc) dbprog_linksel_select,
		(XtPointer) s
	);
	XtAddCallback(
		m->linksel.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_linksel_ok,
		(XtPointer) s
	);
	XtAddCallback(
		m->linksel.cancel_btn,
		XmNactivateCallback,
		(XtCallbackProc) dbprog_linksel_cancel,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->linksel.form),
		delw,
		(XtCallbackProc) dbprog_linksel_cancel,
		(XtPointer) s
	);
}


/*
 * register_help_callbacks
 *	Register all callback routines for widgets in the help text
 *	display window
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_help_callbacks(widgets_t *m, curstat_t *s)
{
	/* Help popup window callbacks */
	XtAddCallback(
		m->help.form,
		XmNfocusCallback,
		(XtCallbackProc) cd_form_focus_chg,
		(XtPointer) m->help.form
	);
	XtAddCallback(
		m->help.ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_help_popdown,
		(XtPointer) s
	);

	/* Install WM_DELETE_WINDOW handler */
	XmAddWMProtocolCallback(
		XtParent(m->help.form),
		delw,
		(XtCallbackProc) cd_help_popdown,
		(XtPointer) s
	);
}


/*
 * register_dialog_callbacks
 *	Register all callback routines for widgets in the dialog
 *	box windows
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
STATIC void
register_dialog_callbacks(widgets_t *m, curstat_t *s)
{
	Widget	ok_btn;

	ok_btn = XmMessageBoxGetChild(m->dialog.warning, XmDIALOG_OK_BUTTON);
	XtAddCallback(
		ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_warning_popdown,
		(XtPointer) s
	);

	ok_btn = XmMessageBoxGetChild(m->dialog.fatal, XmDIALOG_OK_BUTTON);
	XtAddCallback(
		ok_btn,
		XmNactivateCallback,
		(XtCallbackProc) cd_fatal_popdown,
		(XtPointer) s
	);
}


/***********************
 *   public routines   *
 ***********************/


/*
 * bm_to_px
 *	Convert a bitmap into a pixmap.
 *
 * Args:
 *	w - A widget the pixmap should be associated with
 *	bits - Pointer to the raw bitmap data
 *	width, height - The resultant pixmap dimensions
 *	depth - The depth of the desired pixmap
 *	mode - The desired color characteristics of the pixmap
 *		BM_PX_BW	foreground: black, background: white
 *		BM_PX_BWREV 	foreground: white, background: black
 *		BM_PX_WHITE 	foreground: white, background: bg of w
 *		BM_PX_BLACK 	foreground: black, background: bg of w
 *		BM_PX_HIGHLIGHT	foreground: hl of w, background: bg of w
 *		BM_PX_NORMAL	foreground: fg of w, background: bg of w
 *
 * Return:
 *	The pixmap ID, or NULL if failure.
 */
Pixmap
bm_to_px(
	Widget w,
	void *bits,
	int width,
	int height,
	int depth,
	int mode
)
{
	Display		*display = XtDisplay(w);
	Window		window	 = XtWindow(w);
	int		screen	 = DefaultScreen(display);
	GC		pixmap_gc;
	XGCValues	val;
	Pixmap		tmp_bitmap;
	static Pixmap	ret_pixmap;

	tmp_bitmap = XCreateBitmapFromData(display, window,
					   (char *) bits, width, height);
	if (tmp_bitmap == (Pixmap) NULL)
		return ((Pixmap) NULL);

	if (depth == 1) {
		ret_pixmap = tmp_bitmap;
		return (ret_pixmap);
	}
		
	/* Create pixmap with depth */
	ret_pixmap = XCreatePixmap(display, window, width, height, depth);
	if (ret_pixmap == (Pixmap) NULL)
		return ((Pixmap) NULL);

	/* Allocate colors for pixmap if on color screen */
	if (DisplayCells(display, screen) > 2) {
		/* Get pixmap color configuration */
		switch (mode) {
		case BM_PX_BW:
			val.foreground = BlackPixel(display, screen);
			val.background = WhitePixel(display, screen);
			break;

		case BM_PX_BWREV:
			val.foreground = WhitePixel(display, screen);
			val.background = BlackPixel(display, screen);
			break;

		case BM_PX_WHITE:
			val.foreground = WhitePixel(display, screen);
			XtVaGetValues(w,
				XmNbackground, &val.background,
				NULL
			);
			break;

		case BM_PX_BLACK:
			val.foreground = BlackPixel(display, screen);
			XtVaGetValues(w,
				XmNbackground, &val.background,
				NULL
			);
			break;

		case BM_PX_HIGHLIGHT:
			XtVaGetValues(w,
				XmNhighlightColor, &val.foreground,
				XmNbackground, &val.background,
				NULL
			);
			break;

		case BM_PX_NORMAL:
		default:
			XtVaGetValues(w,
				XmNforeground, &val.foreground,
				XmNbackground, &val.background,
				NULL
			);
			break;
		}

		/* Create GC for pixmap */
		pixmap_gc = XCreateGC(display, window,
				    GCForeground | GCBackground, &val);
	}
	else
		pixmap_gc = DefaultGC(display, screen);
		
	/* Copy bitmap into pixmap */
	XCopyPlane(display, tmp_bitmap, ret_pixmap, pixmap_gc,
		   0, 0, width, height, 0, 0, 1);

	/* No need for the bitmap any more, so free it */
	XFreePixmap(display, tmp_bitmap);

	return (ret_pixmap);
}


/*
 * create_widgets
 *	Top-level function to create all widgets
 *
 * Args:
 *	m - Pointer to the main widgets placeholder structure.
 *
 * Return:
 *	Nothing.
 */
void
create_widgets(widgets_t *m)
{
	create_main_widgets(m);
	create_keypad_widgets(m);
	create_options_widgets(m);
	create_dbprog_widgets(m);
	create_exttxt_widgets(m);
	create_dirsel_widgets(m);
	create_linksel_widgets(m);
	create_help_widgets(m);
	create_dialog_widgets(m);
}


/*
 * pre_realize_config
 *	Top-level function to perform set-up and initialization tasks
 *	prior to realizing all widgets.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
void
pre_realize_config(widgets_t *m)
{
	unsigned char		labtype;
	static XtActionsRec	actions[] = {
		{ "hotkey",	hotkey },
		{ "focuschg",	focuschg },
		{ "mainmap",	mainmap }
	};

	/* Set geometry and location of all widgets */
	force_geometry(m);

	/* Make main window toggle buttons have the same label setting as
	 * the pushbuttons.
	 */
	XtVaGetValues(m->main.playpause_btn, XmNlabelType, &labtype, NULL);
	XtVaSetValues(m->main.btnlbl_btn, XmNlabelType, labtype, NULL);
	XtVaSetValues(m->main.lock_btn, XmNlabelType, labtype, NULL);
	XtVaSetValues(m->main.repeat_btn, XmNlabelType, labtype, NULL);
	XtVaSetValues(m->main.shuffle_btn, XmNlabelType, labtype, NULL);

	/* Register action routines */
	XtAppAddActions(
		XtWidgetToApplicationContext(m->toplevel),
		actions,
		XtNumber(actions)
	);

	/* Add translations for iconification handling */
	XtOverrideTranslations(
		m->toplevel,
		XtParseTranslationTable(
			"<MapNotify>: mainmap()\n<UnmapNotify>: mainmap()"
		)
	);

	/* Add translations for shortcut keys */
	hotkey_setup(m);

	/* Add translations for widget-sensitive help popup */
	help_setup(m);

#if defined(EDITRES) && (XtSpecificationRelease >= 5)
	/* Enable editres interaction (see editres(1)) */
	{
		extern void _XEditResCheckMessages();

		XtAddEventHandler(
			widgets.toplevel,
			(EventMask) 0,
			True,
			_XEditResCheckMessages,
			(XtPointer) NULL
		);
	}
#endif

}


/*
 * post_realize_config
 *	Top-level function to perform set-up and initialization tasks
 *	after realizing all widgets.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
void
post_realize_config(widgets_t *m, pixmaps_t *p)
{
	Display		*display = XtDisplay(m->toplevel);
	int		depth = DefaultDepth(display, DefaultScreen(display));

	/* Make pixmaps for all the button tops */
	make_pixmaps(m, p, depth);

	/* Get WM_DELETE_WINDOW atom */
	delw = XmInternAtom(display, "WM_DELETE_WINDOW", False);

	XmProcessTraversal(m->main.playpause_btn, XmTRAVERSE_CURRENT);
}


/*
 * register_callbacks
 *	Top-level function to register all callback routines
 *
 * Args:
 *	m - Pointer to the main widgets structure
 *	s - Pointer to the curstat_t structure
 *
 * Return:
 *	Nothing.
 */
void
register_callbacks(widgets_t *m, curstat_t *s)
{
	register_main_callbacks(m, s);
	register_keypad_callbacks(m, s);
	register_options_callbacks(m, s);
	register_dbprog_callbacks(m, s);
	register_extd_callbacks(m, s);
	register_extt_callbacks(m, s);
	register_dirsel_callbacks(m, s);
	register_linksel_callbacks(m, s);
	register_help_callbacks(m, s);
	register_dialog_callbacks(m, s);
}


