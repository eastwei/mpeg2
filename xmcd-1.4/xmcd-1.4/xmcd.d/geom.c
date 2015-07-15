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
static char *_geom_c_ident_ = "@(#)geom.c	5.2 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/geom.h"


extern appdata_t	app_data;


/***********************
 *  internal routines  *
 ***********************/


/*
 * force_main_geometry
 *	Set the geometry of the widgets in the main window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_main_geometry(widgets_t *m)
{
	XmString	dash = XmStringCreateSimple("-"),
			blank = XmStringCreateSimple("");


	/* Main window widgets */

	XtVaSetValues(m->main.chkbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_CHECKBOX,
		XmNrightPosition, RIGHT_CHECKBOX,
		XmNtopPosition, TOP_CHECKBOX,
		XmNbottomPosition, BOTTOM_CHECKBOX,
		XmNshadowType, XmSHADOW_OUT,
		NULL
	);
	XtVaSetValues(m->main.btnlbl_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	if (!app_data.main_showfocus) {
		XtVaSetValues(m->main.btnlbl_btn,
			XmNhighlightThickness, 0,
			NULL
		);
	}
	XtVaSetValues(m->main.lock_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	if (!app_data.main_showfocus) {
		XtVaSetValues(m->main.lock_btn,
			XmNhighlightThickness, 0,
			NULL
		);
	}
	XtVaSetValues(m->main.repeat_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	if (!app_data.main_showfocus) {
		XtVaSetValues(m->main.repeat_btn,
			XmNhighlightThickness, 0,
			NULL
		);
	}
	XtVaSetValues(m->main.shuffle_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	if (!app_data.main_showfocus) {
		XtVaSetValues(m->main.shuffle_btn,
			XmNhighlightThickness, 0,
			NULL
		);
	}
	XtVaSetValues(m->main.eject_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EJECT,
		XmNrightPosition, RIGHT_EJECT,
		XmNtopPosition, TOP_EJECT,
		XmNbottomPosition, BOTTOM_EJECT,
		NULL
	);
	XtVaSetValues(m->main.poweroff_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_POWEROFF,
		XmNrightPosition, RIGHT_POWEROFF,
		XmNtopPosition, TOP_POWEROFF,
		XmNbottomPosition, BOTTOM_POWEROFF,
		NULL
	);

	XtVaSetValues(m->main.track_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TRACKIND,
		XmNrightPosition, RIGHT_TRACKIND,
		XmNtopPosition, TOP_TRACKIND,
		XmNbottomPosition, BOTTOM_TRACKIND,
		XmNlabelString, dash,
		NULL
	);
	XtVaSetValues(m->main.index_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_INDEXIND,
		XmNrightPosition, RIGHT_INDEXIND,
		XmNtopPosition, TOP_INDEXIND,
		XmNbottomPosition, BOTTOM_INDEXIND,
		XmNlabelString, dash,
		NULL
	);
	XtVaSetValues(m->main.time_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TIMEIND,
		XmNrightPosition, RIGHT_TIMEIND,
		XmNtopPosition, TOP_TIMEIND,
		XmNbottomPosition, BOTTOM_TIMEIND,
		XmNlabelString, dash,
		NULL
	);
	XtVaSetValues(m->main.rptcnt_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_RPTCNTIND,
		XmNrightPosition, RIGHT_RPTCNTIND,
		XmNtopPosition, TOP_RPTCNTIND,
		XmNbottomPosition, BOTTOM_RPTCNTIND,
		XmNlabelString, dash,
		NULL
	);
	XtVaSetValues(m->main.dbmode_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DBMODEIND,
		XmNrightPosition, RIGHT_DBMODEIND,
		XmNtopPosition, TOP_DBMODEIND,
		XmNbottomPosition, BOTTOM_DBMODEIND,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.progmode_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PROGMODEIND,
		XmNrightPosition, RIGHT_PROGMODEIND,
		XmNtopPosition, TOP_PROGMODEIND,
		XmNbottomPosition, BOTTOM_PROGMODEIND,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.timemode_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TIMEMODEIND,
		XmNrightPosition, RIGHT_TIMEMODEIND,
		XmNtopPosition, TOP_TIMEMODEIND,
		XmNbottomPosition, BOTTOM_TIMEMODEIND,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.playmode_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PLAYMODEIND,
		XmNrightPosition, RIGHT_PLAYMODEIND,
		XmNtopPosition, TOP_PLAYMODEIND,
		XmNbottomPosition, BOTTOM_PLAYMODEIND,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.dtitle_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DTITLEIND,
		XmNrightPosition, RIGHT_DTITLEIND,
		XmNtopPosition, TOP_DTITLEIND,
		XmNbottomPosition, BOTTOM_DTITLEIND,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.ttitle_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TTITLEIND,
		XmNrightPosition, RIGHT_TTITLEIND,
		XmNtopPosition, TOP_TTITLEIND,
		XmNbottomPosition, BOTTOM_TTITLEIND,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNlabelString, blank,
		NULL
	);
	XtVaSetValues(m->main.dbprog_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DBPROG,
		XmNrightPosition, RIGHT_DBPROG,
		XmNtopPosition, TOP_DBPROG,
		XmNbottomPosition, BOTTOM_DBPROG,
		NULL
	);
	XtVaSetValues(m->main.options_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_OPTIONS,
		XmNrightPosition, RIGHT_OPTIONS,
		XmNtopPosition, TOP_OPTIONS,
		XmNbottomPosition, BOTTOM_OPTIONS,
		NULL
	);
	XtVaSetValues(m->main.time_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TIME,
		XmNrightPosition, RIGHT_TIME,
		XmNtopPosition, TOP_TIME,
		XmNbottomPosition, BOTTOM_TIME,
		NULL
	);
	XtVaSetValues(m->main.ab_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_AB,
		XmNrightPosition, RIGHT_AB,
		XmNtopPosition, TOP_AB,
		XmNbottomPosition, BOTTOM_AB,
		NULL
	);
	XtVaSetValues(m->main.sample_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_SAMPLE,
		XmNrightPosition, RIGHT_SAMPLE,
		XmNtopPosition, TOP_SAMPLE,
		XmNbottomPosition, BOTTOM_SAMPLE,
		NULL
	);
	XtVaSetValues(m->main.keypad_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEYPAD,
		XmNrightPosition, RIGHT_KEYPAD,
		XmNtopPosition, TOP_KEYPAD,
		XmNbottomPosition, BOTTOM_KEYPAD,
		NULL
	);
	XtVaSetValues(m->main.help_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_HELP,
		XmNrightPosition, RIGHT_HELP,
		XmNtopPosition, TOP_HELP,
		XmNbottomPosition, BOTTOM_HELP,
		NULL
	);
	XtVaSetValues(m->main.level_scale,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LEVEL,
		XmNrightPosition, RIGHT_LEVEL,
		XmNtopPosition, TOP_LEVEL,
		XmNbottomPosition, BOTTOM_LEVEL,
		XmNshowValue, True,
		XmNminimum, 0,
		XmNmaximum, 100,
		XmNorientation, XmHORIZONTAL,
		XmNprocessingDirection, XmMAX_ON_RIGHT,
		NULL
	);
	XtVaSetValues(m->main.playpause_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PLAYPAUSE,
		XmNrightPosition, RIGHT_PLAYPAUSE,
		XmNtopPosition, TOP_PLAYPAUSE,
		XmNbottomPosition, BOTTOM_PLAYPAUSE,
		NULL
	);
	XtVaSetValues(m->main.stop_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_STOP,
		XmNrightPosition, RIGHT_STOP,
		XmNtopPosition, TOP_STOP,
		XmNbottomPosition, BOTTOM_STOP,
		NULL
	);
	XtVaSetValues(m->main.prevtrk_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PREVTRK,
		XmNrightPosition, RIGHT_PREVTRK,
		XmNtopPosition, TOP_PREVTRK,
		XmNbottomPosition, BOTTOM_PREVTRK,
		NULL
	);
	XtVaSetValues(m->main.nexttrk_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_NEXTTRK,
		XmNrightPosition, RIGHT_NEXTTRK,
		XmNtopPosition, TOP_NEXTTRK,
		XmNbottomPosition, BOTTOM_NEXTTRK,
		NULL
	);
	XtVaSetValues(m->main.previdx_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PREVIDX,
		XmNrightPosition, RIGHT_PREVIDX,
		XmNtopPosition, TOP_PREVIDX,
		XmNbottomPosition, BOTTOM_PREVIDX,
		NULL
	);
	XtVaSetValues(m->main.nextidx_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_NEXTIDX,
		XmNrightPosition, RIGHT_NEXTIDX,
		XmNtopPosition, TOP_NEXTIDX,
		XmNbottomPosition, BOTTOM_NEXTIDX,
		NULL
	);
	XtVaSetValues(m->main.rew_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_REW,
		XmNrightPosition, RIGHT_REW,
		XmNtopPosition, TOP_REW,
		XmNbottomPosition, BOTTOM_REW,
		NULL
	);
	XtVaSetValues(m->main.ff_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_FF,
		XmNrightPosition, RIGHT_FF,
		XmNtopPosition, TOP_FF,
		XmNbottomPosition, BOTTOM_FF,
		NULL
	);

	XmStringFree(dash);
	XmStringFree(blank);
}


/*
 * force_keypad_geometry
 *	Set the geometry of the widgets in the keypad window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_keypad_geometry(widgets_t *m)
{
	/* Keypad popup window widgets */

	XtVaSetValues(m->keypad.keypad_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEYPADLBL,
		XmNrightPosition, RIGHT_KEYPADLBL,
		XmNtopPosition, TOP_KEYPADLBL,
		XmNbottomPosition, BOTTOM_KEYPADLBL,
		NULL
	);

	XtVaSetValues(m->keypad.keypad_ind,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEYPADIND,
		XmNrightPosition, RIGHT_KEYPADIND,
		XmNtopPosition, TOP_KEYPADIND,
		XmNbottomPosition, BOTTOM_KEYPADIND,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[0],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY0,
		XmNrightPosition, RIGHT_KEY0,
		XmNtopPosition, TOP_KEY0,
		XmNbottomPosition, BOTTOM_KEY0,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[1],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY1,
		XmNrightPosition, RIGHT_KEY1,
		XmNtopPosition, TOP_KEY1,
		XmNbottomPosition, BOTTOM_KEY1,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[2],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY2,
		XmNrightPosition, RIGHT_KEY2,
		XmNtopPosition, TOP_KEY2,
		XmNbottomPosition, BOTTOM_KEY2,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[3],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY3,
		XmNrightPosition, RIGHT_KEY3,
		XmNtopPosition, TOP_KEY3,
		XmNbottomPosition, BOTTOM_KEY3,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[4],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY4,
		XmNrightPosition, RIGHT_KEY4,
		XmNtopPosition, TOP_KEY4,
		XmNbottomPosition, BOTTOM_KEY4,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[5],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY5,
		XmNrightPosition, RIGHT_KEY5,
		XmNtopPosition, TOP_KEY5,
		XmNbottomPosition, BOTTOM_KEY5,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[6],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY6,
		XmNrightPosition, RIGHT_KEY6,
		XmNtopPosition, TOP_KEY6,
		XmNbottomPosition, BOTTOM_KEY6,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[7],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY7,
		XmNrightPosition, RIGHT_KEY7,
		XmNtopPosition, TOP_KEY7,
		XmNbottomPosition, BOTTOM_KEY7,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[8],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY8,
		XmNrightPosition, RIGHT_KEY8,
		XmNtopPosition, TOP_KEY8,
		XmNbottomPosition, BOTTOM_KEY8,
		NULL
	);

	XtVaSetValues(m->keypad.num_btn[9],
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_KEY9,
		XmNrightPosition, RIGHT_KEY9,
		XmNtopPosition, TOP_KEY9,
		XmNbottomPosition, BOTTOM_KEY9,
		NULL
	);

	XtVaSetValues(m->keypad.clear_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_CLEAR,
		XmNrightPosition, RIGHT_CLEAR,
		XmNtopPosition, TOP_CLEAR,
		XmNbottomPosition, BOTTOM_CLEAR,
		NULL
	);

	XtVaSetValues(m->keypad.enter_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_ENTER,
		XmNrightPosition, RIGHT_ENTER,
		XmNtopPosition, TOP_ENTER,
		XmNbottomPosition, BOTTOM_ENTER,
		NULL
	);

	XtVaSetValues(m->keypad.warp_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_WARPLBL,
		XmNrightPosition, RIGHT_WARPLBL,
		XmNtopPosition, TOP_WARPLBL,
		XmNbottomPosition, BOTTOM_WARPLBL,
		NULL
	);

	XtVaSetValues(m->keypad.warp_scale,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_WARPSCALE,
		XmNrightPosition, RIGHT_WARPSCALE,
		XmNtopPosition, TOP_WARPSCALE,
		XmNbottomPosition, BOTTOM_WARPSCALE,
		XmNshowValue, False,
		XmNminimum, 0,
		XmNmaximum, 255,
		XmNorientation, XmHORIZONTAL,
		XmNprocessingDirection, XmMAX_ON_RIGHT,
		XmNhighlightOnEnter, True,
		NULL
	);

	XtVaSetValues(m->keypad.keypad_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_KEYPADSEP,
		XmNrightPosition, RIGHT_KEYPADSEP,
		XmNtopPosition, TOP_KEYPADSEP,
		NULL
	);

	XtVaSetValues(m->keypad.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_KPCANCEL,
		XmNrightPosition, RIGHT_KPCANCEL,
		XmNtopPosition, TOP_KPCANCEL,
		NULL
	);
}


/*
 * force_options_geometry
 *	Set the geometry of the widgets in the options window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_options_geometry(widgets_t *m)
{
	/* Options popup window widgets */

	XtVaSetValues(m->options.load_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LOAD_LBL,
		XmNrightPosition, RIGHT_LOAD_LBL,
		XmNtopPosition, TOP_LOAD_LBL,
		XmNbottomPosition, BOTTOM_LOAD_LBL,
		NULL
	);

	XtVaSetValues(m->options.load_chkbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNleftPosition, LEFT_LOAD_CHKFRM,
		XmNrightPosition, RIGHT_LOAD_CHKFRM,
		XmNtopPosition, TOP_LOAD_CHKFRM,
		XmNbottomWidget, m->options.load_radbox_frm,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.load_lock_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.load_radbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LOAD_RADFRM,
		XmNrightPosition, RIGHT_LOAD_RADFRM,
		XmNtopPosition, TOP_LOAD_RADFRM,
		XmNbottomPosition, BOTTOM_LOAD_RADFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.load_none_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.load_spdn_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.load_play_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.exit_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXIT_LBL,
		XmNrightPosition, RIGHT_EXIT_LBL,
		XmNtopPosition, TOP_EXIT_LBL,
		XmNbottomPosition, BOTTOM_EXIT_LBL,
		NULL
	);

	XtVaSetValues(m->options.exit_radbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXIT_RADFRM,
		XmNrightPosition, RIGHT_EXIT_RADFRM,
		XmNtopPosition, TOP_EXIT_RADFRM,
		XmNbottomPosition, BOTTOM_EXIT_RADFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.exit_none_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.exit_stop_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.exit_eject_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.done_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DONE_LBL,
		XmNrightPosition, RIGHT_DONE_LBL,
		XmNtopPosition, TOP_DONE_LBL,
		XmNbottomPosition, BOTTOM_DONE_LBL,
		NULL
	);

	XtVaSetValues(m->options.done_chkbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DONE_CHKFRM,
		XmNrightPosition, RIGHT_DONE_CHKFRM,
		XmNtopPosition, TOP_DONE_CHKFRM,
		XmNbottomPosition, BOTTOM_DONE_CHKFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.done_eject_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.eject_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EJECT_LBL,
		XmNrightPosition, RIGHT_EJECT_LBL,
		XmNtopPosition, TOP_EJECT_LBL,
		XmNbottomPosition, BOTTOM_EJECT_LBL,
		NULL
	);

	XtVaSetValues(m->options.eject_chkbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EJECT_CHKFRM,
		XmNrightPosition, RIGHT_EJECT_CHKFRM,
		XmNtopPosition, TOP_EJECT_CHKFRM,
		XmNbottomPosition, BOTTOM_EJECT_CHKFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.eject_exit_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.chroute_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_CHROUTE_LBL,
		XmNrightPosition, RIGHT_CHROUTE_LBL,
		XmNtopPosition, TOP_CHROUTE_LBL,
		XmNbottomPosition, BOTTOM_CHROUTE_LBL,
		NULL
	);

	XtVaSetValues(m->options.chroute_radbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_CHROUTE_RADFRM,
		XmNrightPosition, RIGHT_CHROUTE_RADFRM,
		XmNtopPosition, TOP_CHROUTE_RADFRM,
		XmNbottomPosition, BOTTOM_CHROUTE_RADFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.chroute_stereo_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.chroute_rev_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.chroute_left_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.chroute_right_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.chroute_mono_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.vol_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_VOLTP_LBL,
		XmNrightPosition, RIGHT_VOLTP_LBL,
		XmNtopPosition, TOP_VOLTP_LBL,
		XmNbottomPosition, BOTTOM_VOLTP_LBL,
		NULL
	);

	XtVaSetValues(m->options.vol_radbox_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_VOLTP_RADFRM,
		XmNrightPosition, RIGHT_VOLTP_RADFRM,
		XmNtopPosition, TOP_VOLTP_RADFRM,
		XmNbottomPosition, BOTTOM_VOLTP_RADFRM,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);
	XtVaSetValues(m->options.vol_linear_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.vol_square_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->options.vol_invsqr_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->options.bal_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_BAL_LBL,
		XmNrightPosition, RIGHT_BAL_LBL,
		XmNtopPosition, TOP_BAL_LBL,
		XmNbottomPosition, BOTTOM_BAL_LBL,
		NULL
	);

	XtVaSetValues(m->options.ball_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_BALL_LBL,
		XmNrightPosition, RIGHT_BALL_LBL,
		XmNtopPosition, TOP_BALL_LBL,
		XmNbottomPosition, BOTTOM_BALL_LBL,
		NULL
	);

	XtVaSetValues(m->options.balr_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_BALR_LBL,
		XmNrightPosition, RIGHT_BALR_LBL,
		XmNtopPosition, TOP_BALR_LBL,
		XmNbottomPosition, BOTTOM_BALR_LBL,
		NULL
	);

	XtVaSetValues(m->options.bal_scale,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_BAL_SCALE,
		XmNrightPosition, RIGHT_BAL_SCALE,
		XmNtopPosition, TOP_BAL_SCALE,
		XmNbottomPosition, BOTTOM_BAL_SCALE,
		XmNshowValue, False,
		XmNminimum, -50,
		XmNmaximum, 50,
		XmNvalue, 0,
		XmNorientation, XmHORIZONTAL,
		XmNprocessingDirection, XmMAX_ON_RIGHT,
		XmNhighlightOnEnter, True,
		NULL
	);

	XtVaSetValues(m->options.balctr_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_BALCTR_BTN,
		XmNrightPosition, RIGHT_BALCTR_BTN,
		XmNtopPosition, TOP_BALCTR_BTN,
		XmNbottomPosition, BOTTOM_BALCTR_BTN,
		NULL
	);

	XtVaSetValues(m->options.options_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_OPTSEP,
		XmNrightPosition, RIGHT_OPTSEP,
		XmNtopPosition, TOP_OPTSEP,
		NULL
	);

	XtVaSetValues(m->options.reset_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_RESET_BTN,
		XmNrightPosition, RIGHT_RESET_BTN,
		XmNtopPosition, TOP_RESET_BTN,
		NULL
	);

	XtVaSetValues(m->options.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_OPTOK_BTN,
		XmNrightPosition, RIGHT_OPTOK_BTN,
		XmNtopPosition, TOP_OPTOK_BTN,
		NULL
	);
}


/*
 * force_dbprog_geometry
 *	Set the geometry of the widgets in the database/program window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_dbprog_geometry(widgets_t *m)
{
	/* CD program/database popup window widgets */

	XtVaSetValues(m->dbprog.logo_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LOGO,
		XmNrightPosition, RIGHT_LOGO,
		XmNtopPosition, TOP_LOGO,
		XmNbottomPosition, BOTTOM_LOGO,
		NULL
	);

	XtVaSetValues(m->dbprog.about_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_ABOUT,
		XmNrightPosition, RIGHT_ABOUT,
		XmNtopPosition, TOP_ABOUT,
		XmNbottomPosition, BOTTOM_ABOUT,
		NULL
	);

	XtVaSetValues(m->dbprog.dtitle_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DTITLELBL,
		XmNrightPosition, RIGHT_DTITLELBL,
		XmNtopPosition, TOP_DTITLELBL,
		XmNbottomPosition, BOTTOM_DTITLELBL,
		NULL
	);

	XtVaSetValues(m->dbprog.extd_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTDLBL,
		XmNrightPosition, RIGHT_EXTDLBL,
		XmNtopPosition, TOP_EXTDLBL,
		XmNbottomPosition, BOTTOM_EXTDLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.dtitle_txt,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DTITLE,
		XmNrightPosition, RIGHT_DTITLE,
		XmNtopPosition, TOP_DTITLE,
		XmNbottomPosition, BOTTOM_DTITLE,
		NULL
	);

	XtVaSetValues(m->dbprog.extd_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTD,
		XmNrightPosition, RIGHT_EXTD,
		XmNtopPosition, TOP_EXTD,
		XmNbottomPosition, BOTTOM_EXTD,
		NULL
	);

	XtVaSetValues(m->dbprog.dbprog_sep1,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DBPROGSEP1,
		XmNrightPosition, RIGHT_DBPROGSEP1,
		XmNtopPosition, TOP_DBPROGSEP1,
		NULL
	);

	XtVaSetValues(m->dbprog.trklist_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TRKLISTLBL,
		XmNrightPosition, RIGHT_TRKLISTLBL,
		XmNtopPosition, TOP_TRKLISTLBL,
		XmNbottomPosition, BOTTOM_TRKLISTLBL,
		NULL
	);

	XtVaSetValues(XtParent(m->dbprog.trk_list),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TRKLIST,
		XmNrightPosition, RIGHT_TRKLIST,
		XmNtopPosition, TOP_TRKLIST,
		XmNbottomPosition, BOTTOM_TRKLIST,
		NULL
	);

	XtVaSetValues(m->dbprog.radio_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_RADIOLBL,
		XmNrightPosition, RIGHT_RADIOLBL,
		XmNtopPosition, TOP_RADIOLBL,
		XmNbottomPosition, BOTTOM_RADIOLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.radio_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_RADIOBOX,
		XmNrightPosition, RIGHT_RADIOBOX,
		XmNtopPosition, TOP_RADIOBOX,
		XmNbottomPosition, BOTTOM_RADIOBOX,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);

	XtVaSetValues(m->dbprog.tottime_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);
	XtVaSetValues(m->dbprog.trktime_btn,
		XmNheight, 16,
		XmNrecomputeSize, False,
		NULL
	);

	XtVaSetValues(m->dbprog.discid_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DISCIDLBL,
		XmNrightPosition, RIGHT_DISCIDLBL,
		XmNtopPosition, TOP_DISCIDLBL,
		XmNbottomPosition, BOTTOM_DISCIDLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.discid_frm,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DISCIDFRM,
		XmNrightPosition, RIGHT_DISCIDFRM,
		XmNtopPosition, TOP_DISCIDFRM,
		XmNbottomPosition, BOTTOM_DISCIDFRM,
		NULL
	);

	XtVaSetValues(m->dbprog.ttitle_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TTITLELBL,
		XmNrightPosition, RIGHT_TTITLELBL,
		XmNtopPosition, TOP_TTITLELBL,
		XmNbottomPosition, BOTTOM_TTITLELBL,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		NULL
	);

	XtVaSetValues(m->dbprog.extt_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTTLBL,
		XmNrightPosition, RIGHT_EXTTLBL,
		XmNtopPosition, TOP_EXTTLBL,
		XmNbottomPosition, BOTTOM_EXTTLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.ttitle_txt,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TTITLE,
		XmNrightPosition, RIGHT_TTITLE,
		XmNtopPosition, TOP_TTITLE,
		XmNbottomPosition, BOTTOM_TTITLE,
		NULL
	);

	XtVaSetValues(m->dbprog.extt_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTT,
		XmNrightPosition, RIGHT_EXTT,
		XmNtopPosition, TOP_EXTT,
		XmNbottomPosition, BOTTOM_EXTT,
		NULL
	);

	XtVaSetValues(m->dbprog.pgm_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PGMLBL,
		XmNrightPosition, RIGHT_PGMLBL,
		XmNtopPosition, TOP_PGMLBL,
		XmNbottomPosition, BOTTOM_PGMLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.addpgm_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_ADDPGM,
		XmNrightPosition, RIGHT_ADDPGM,
		XmNtopPosition, TOP_ADDPGM,
		XmNbottomPosition, BOTTOM_ADDPGM,
		NULL
	);

	XtVaSetValues(m->dbprog.clrpgm_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_CLRPGM,
		XmNrightPosition, RIGHT_CLRPGM,
		XmNtopPosition, TOP_CLRPGM,
		XmNbottomPosition, BOTTOM_CLRPGM,
		NULL
	);

	XtVaSetValues(m->dbprog.pgmseq_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PGMSEQLBL,
		XmNrightPosition, RIGHT_PGMSEQLBL,
		XmNtopPosition, TOP_PGMSEQLBL,
		XmNbottomPosition, BOTTOM_PGMSEQLBL,
		NULL
	);

	XtVaSetValues(m->dbprog.pgmseq_txt,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_PGMSEQ,
		XmNrightPosition, RIGHT_PGMSEQ,
		XmNtopPosition, TOP_PGMSEQ,
		XmNbottomPosition, BOTTOM_PGMSEQ,
		NULL
	);

	XtVaSetValues(m->dbprog.send_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_SEND,
		XmNrightPosition, RIGHT_SEND,
		XmNtopPosition, TOP_SEND,
		XmNbottomPosition, BOTTOM_SEND,
		NULL
	);

	XtVaSetValues(m->dbprog.dbprog_sep2,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DBPROGSEP2,
		XmNrightPosition, RIGHT_DBPROGSEP2,
		XmNtopPosition, TOP_DBPROGSEP2,
		NULL
	);

	XtVaSetValues(m->dbprog.savedb_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_SAVEDB,
		XmNrightPosition, RIGHT_SAVEDB,
		XmNtopPosition, TOP_SAVEDB,
		NULL
	);

	XtVaSetValues(m->dbprog.linkdb_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_LINK,
		XmNrightPosition, RIGHT_LINK,
		XmNtopPosition, TOP_LINK,
		NULL
	);

	XtVaSetValues(m->dbprog.loaddb_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_LOADDB,
		XmNrightPosition, RIGHT_LOADDB,
		XmNtopPosition, TOP_LOADDB,
		NULL
	);

	XtVaSetValues(m->dbprog.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DPCANCEL,
		XmNrightPosition, RIGHT_DPCANCEL,
		XmNtopPosition, TOP_DPCANCEL,
		NULL
	);
}


/*
 * force_extd_geometry
 *	Set the geometry of the widgets in the extended disc info
 *	window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_extd_geometry(widgets_t *m)
{
	/* Extended Disc Info popup window widgets */

	XtVaSetValues(m->dbextd.disc_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DISCLBL,
		XmNrightPosition, RIGHT_DISCLBL,
		XmNtopPosition, TOP_DISCLBL,
		XmNbottomPosition, BOTTOM_DISCLBL,
		NULL
	);
	
	XtVaSetValues(XtParent(m->dbextd.disc_txt),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTDTXT,
		XmNrightPosition, RIGHT_EXTDTXT,
		XmNtopPosition, TOP_EXTDTXT,
		XmNbottomPosition, BOTTOM_EXTDTXT,
		NULL
	);
	
	XtVaSetValues(m->dbextd.dbextd_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DBEXTDSEP,
		XmNrightPosition, RIGHT_DBEXTDSEP,
		XmNtopPosition, TOP_DBEXTDSEP,
		NULL
	);
	
	XtVaSetValues(m->dbextd.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DDOK,
		XmNrightPosition, RIGHT_DDOK,
		XmNtopPosition, TOP_DDOK,
		NULL
	);
	
	XtVaSetValues(m->dbextd.clear_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DDCLEAR,
		XmNrightPosition, RIGHT_DDCLEAR,
		XmNtopPosition, TOP_DDCLEAR,
		NULL
	);

	XtVaSetValues(m->dbextd.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DDCANCEL,
		XmNrightPosition, RIGHT_DDCANCEL,
		XmNtopPosition, TOP_DDCANCEL,
		NULL
	);
}


/*
 * force_extt_geometry
 *	Set the geometry of the widgets in the extended track info
 *	window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_extt_geometry(widgets_t *m)
{
	/* Extended Track Info popup window widgets */

	XtVaSetValues(m->dbextt.trk_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_TRKLBL,
		XmNrightPosition, RIGHT_TRKLBL,
		XmNtopPosition, TOP_TRKLBL,
		XmNbottomPosition, BOTTOM_TRKLBL,
		NULL
	);
	
	XtVaSetValues(XtParent(m->dbextt.trk_txt),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_EXTTTXT,
		XmNrightPosition, RIGHT_EXTTTXT,
		XmNtopPosition, TOP_EXTTTXT,
		XmNbottomPosition, BOTTOM_EXTTTXT,
		NULL
	);
	
	XtVaSetValues(m->dbextt.dbextt_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DBEXTTSEP,
		XmNrightPosition, RIGHT_DBEXTTSEP,
		XmNtopPosition, TOP_DBEXTTSEP,
		NULL
	);
	
	XtVaSetValues(m->dbextt.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DTOK,
		XmNrightPosition, RIGHT_DTOK,
		XmNtopPosition, TOP_DTOK,
		NULL
	);
	
	XtVaSetValues(m->dbextt.clear_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DTCLEAR,
		XmNrightPosition, RIGHT_DTCLEAR,
		XmNtopPosition, TOP_DTCLEAR,
		NULL
	);

	XtVaSetValues(m->dbextt.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DTCANCEL,
		XmNrightPosition, RIGHT_DTCANCEL,
		XmNtopPosition, TOP_DTCANCEL,
		NULL
	);
}


/*
 * force_help_geometry
 *	Set the geometry of the widgets in the help display window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_help_geometry(widgets_t *m)
{
	XtVaSetValues(XtParent(m->help.help_txt),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_HELPTXT,
		XmNrightPosition, RIGHT_HELPTXT,
		XmNtopPosition, TOP_HELPTXT,
		XmNbottomPosition, BOTTOM_HELPTXT,
		NULL
	);
	
	XtVaSetValues(m->help.help_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_HELPSEP,
		XmNrightPosition, RIGHT_HELPSEP,
		XmNtopPosition, TOP_HELPSEP,
		NULL
	);
	
	XtVaSetValues(m->help.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_HELPOK,
		XmNrightPosition, RIGHT_HELPOK,
		XmNtopPosition, TOP_HELPOK,
		NULL
	);
}


/*
 * force_dirsel_geometry
 *	Set the geometry of the widgets in the CD database directory
 *	selector window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_dirsel_geometry(widgets_t *m)
{
	/* Directory selector popup widgets */

	XtVaSetValues(m->dirsel.dir_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DIRLBL,
		XmNrightPosition, RIGHT_DIRLBL,
		XmNtopPosition, TOP_DIRLBL,
		XmNbottomPosition, BOTTOM_DIRLBL,
		NULL
	);
	
	XtVaSetValues(XtParent(m->dirsel.dir_list),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_DIRLIST,
		XmNrightPosition, RIGHT_DIRLIST,
		XmNtopPosition, TOP_DIRLIST,
		XmNbottomPosition, BOTTOM_DIRLIST,
		NULL
	);
	
	XtVaSetValues(m->dirsel.dirsel_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DIRSELSEP,
		XmNrightPosition, RIGHT_DIRSELSEP,
		XmNtopPosition, TOP_DIRSELSEP,
		NULL
	);
	
	XtVaSetValues(m->dirsel.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DSOK,
		XmNrightPosition, RIGHT_DSOK,
		XmNtopPosition, TOP_DSOK,
		NULL
	);
	
	XtVaSetValues(m->dirsel.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_DSCANCEL,
		XmNrightPosition, RIGHT_DSCANCEL,
		XmNtopPosition, TOP_DSCANCEL,
		NULL
	);
}


/*
 * force_linksel_geometry
 *	Set the geometry of the widgets in the search-link selector
 *	list window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
force_linksel_geometry(widgets_t *m)
{
	/* Link selector popup widgets */

	XtVaSetValues(m->linksel.link_lbl,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LINKLBL,
		XmNrightPosition, RIGHT_LINKLBL,
		XmNtopPosition, TOP_LINKLBL,
		XmNbottomPosition, BOTTOM_LINKLBL,
		NULL
	);
	
	XtVaSetValues(XtParent(m->linksel.link_list),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNleftPosition, LEFT_LINKLIST,
		XmNrightPosition, RIGHT_LINKLIST,
		XmNtopPosition, TOP_LINKLIST,
		XmNbottomPosition, BOTTOM_LINKLIST,
		NULL
	);
	
	XtVaSetValues(m->linksel.linksel_sep,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_LINKSELSEP,
		XmNrightPosition, RIGHT_LINKSELSEP,
		XmNtopPosition, TOP_LINKSELSEP,
		NULL
	);
	
	XtVaSetValues(m->linksel.ok_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_LSOK,
		XmNrightPosition, RIGHT_LSOK,
		XmNtopPosition, TOP_LSOK,
		NULL
	);
	
	XtVaSetValues(m->linksel.cancel_btn,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNbottomAttachment, XmATTACH_NONE,
		XmNleftPosition, LEFT_LSCANCEL,
		XmNrightPosition, RIGHT_LSCANCEL,
		XmNtopPosition, TOP_LSCANCEL,
		NULL
	);
}


/*
 * force_dialog_geometry
 *	Set the geometry of the widgets in the dialog box windows.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
STATIC void
force_dialog_geometry(widgets_t *m)
{
	/* For future expansion */
}


/***********************
 *   public routines   *
 ***********************/


/*
 * force_geometry
 *	Top level function to set the geometry of the widgets in each
 *	main and sub-window.
 *
 * Args:
 *	m - Pointer to the main widgets structure.
 *
 * Return:
 *	Nothing.
 */
void
force_geometry(widgets_t *m)
{
	force_main_geometry(m);
	force_keypad_geometry(m);
	force_options_geometry(m);
	force_dbprog_geometry(m);
	force_extd_geometry(m);
	force_extt_geometry(m);
	force_help_geometry(m);
	force_dirsel_geometry(m);
	force_linksel_geometry(m);
	force_dialog_geometry(m);
}


