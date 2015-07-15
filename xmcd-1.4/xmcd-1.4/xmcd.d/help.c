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
static char *_help_c_ident_ = "@(#)help.c	5.2 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "xmcd.d/xmcd.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/cdfunc.h"
#include "xmcd.d/help.h"


/* This structure is used to map widgets to associated help files.
 * Instead of using XtName(), this mechanism allows us to map multiple
 * widgets to a common help file.  Also, we can use arbitrary lengths
 * for the widget name and still have help files with less than 14 chars
 * in its name (necessary for compatibility with some systems).
 */
typedef struct {
	Widget	widget;
	char	*hlpname;
} wname_t;


extern appdata_t	app_data;
extern widgets_t	widgets;

STATIC wname_t		wname[MAX_HELP_WIDGETS];
STATIC char		tmpbuf[STR_BUF_SZ * 2];


/***********************
 *  internal routines  *
 ***********************/

/*
 * help_mapinit
 *	Initialize the widget-to-helpfile_name mapping table.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
help_mapinit(void)
{
	int	i = 0;

	wname[i].widget = widgets.main.check_box;
	wname[i++].hlpname = "CheckBox";
	wname[i].widget = widgets.main.eject_btn;
	wname[i++].hlpname = "EjectBtn";
	wname[i].widget = widgets.main.poweroff_btn;
	wname[i++].hlpname = "PowerBtn";
	wname[i].widget = widgets.main.dbprog_btn;
	wname[i++].hlpname = "DbProgBtn";
	wname[i].widget = widgets.main.options_btn;
	wname[i++].hlpname = "OptionsBtn";
	wname[i].widget = widgets.main.time_btn;
	wname[i++].hlpname = "TimeBtn";
	wname[i].widget = widgets.main.ab_btn;
	wname[i++].hlpname = "AbBtn";
	wname[i].widget = widgets.main.sample_btn;
	wname[i++].hlpname = "SampleBtn";
	wname[i].widget = widgets.main.keypad_btn;
	wname[i++].hlpname = "KeypadBtn";
	wname[i].widget = widgets.main.help_btn;
	wname[i++].hlpname = "HelpBtn";
	wname[i].widget = widgets.main.level_scale;
	wname[i++].hlpname = "LevelScale";
	wname[i].widget = widgets.main.playpause_btn;
	wname[i++].hlpname = "PlayPauseBtn";
	wname[i].widget = widgets.main.stop_btn;
	wname[i++].hlpname = "StopBtn";
	wname[i].widget = widgets.main.prevtrk_btn;
	wname[i++].hlpname = "PrevTrkBtn";
	wname[i].widget = widgets.main.nexttrk_btn;
	wname[i++].hlpname = "NextTrkBtn";
	wname[i].widget = widgets.main.previdx_btn;
	wname[i++].hlpname = "PrevIdxBtn";
	wname[i].widget = widgets.main.nextidx_btn;
	wname[i++].hlpname = "NextIdxBtn";
	wname[i].widget = widgets.main.rew_btn;
	wname[i++].hlpname = "RewBtn";
	wname[i].widget = widgets.main.ff_btn;
	wname[i++].hlpname = "FfBtn";
	wname[i].widget = widgets.main.track_ind;
	wname[i++].hlpname = "TrackInd";
	wname[i].widget = widgets.main.index_ind;
	wname[i++].hlpname = "IndexInd";
	wname[i].widget = widgets.main.time_ind;
	wname[i++].hlpname = "TimeInd";
	wname[i].widget = widgets.main.rptcnt_ind;
	wname[i++].hlpname = "RptCntInd";
	wname[i].widget = widgets.main.dbmode_ind;
	wname[i++].hlpname = "DbModeInd";
	wname[i].widget = widgets.main.progmode_ind;
	wname[i++].hlpname = "ProgModeInd";
	wname[i].widget = widgets.main.timemode_ind;
	wname[i++].hlpname = "TimeModeInd";
	wname[i].widget = widgets.main.playmode_ind;
	wname[i++].hlpname = "PlayModeInd";
	wname[i].widget = widgets.main.dtitle_ind;
	wname[i++].hlpname = "DiscTitleInd";
	wname[i].widget = widgets.main.ttitle_ind;
	wname[i++].hlpname = "TrackTitleInd";
	wname[i].widget = widgets.keypad.keypad_ind;
	wname[i++].hlpname = "KeypadInd";
	wname[i].widget = widgets.keypad.num_btn[0];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[1];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[2];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[3];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[4];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[5];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[6];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[7];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[8];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.num_btn[9];
	wname[i++].hlpname = "KpNumBtn";
	wname[i].widget = widgets.keypad.clear_btn;
	wname[i++].hlpname = "KpClearBtn";
	wname[i].widget = widgets.keypad.enter_btn;
	wname[i++].hlpname = "KpEnterBtn";
	wname[i].widget = widgets.keypad.warp_lbl;
	wname[i++].hlpname = "KpWarpScale";
	wname[i].widget = widgets.keypad.warp_scale;
	wname[i++].hlpname = "KpWarpScale";
	wname[i].widget = widgets.keypad.cancel_btn;
	wname[i++].hlpname = "KpCancelBtn";
	wname[i].widget = widgets.options.load_chkbox;
	wname[i++].hlpname = "OpLoadChkBox";
	wname[i].widget = widgets.options.load_radbox;
	wname[i++].hlpname = "OpLoadRadBox";
	wname[i].widget = widgets.options.exit_radbox;
	wname[i++].hlpname = "OpExitRadBox";
	wname[i].widget = widgets.options.done_chkbox;
	wname[i++].hlpname = "OpDoneChkBox";
	wname[i].widget = widgets.options.eject_chkbox;
	wname[i++].hlpname = "OpEjectChkBox";
	wname[i].widget = widgets.options.chroute_radbox;
	wname[i++].hlpname = "OpChRtRadBox";
	wname[i].widget = widgets.options.vol_radbox;
	wname[i++].hlpname = "OpVolTprRadBox";
	wname[i].widget = widgets.options.bal_lbl;
	wname[i++].hlpname = "OpBalScale";
	wname[i].widget = widgets.options.bal_scale;
	wname[i++].hlpname = "OpBalScale";
	wname[i].widget = widgets.options.ball_lbl;
	wname[i++].hlpname = "OpBalScale";
	wname[i].widget = widgets.options.balr_lbl;
	wname[i++].hlpname = "OpBalScale";
	wname[i].widget = widgets.options.balctr_btn;
	wname[i++].hlpname = "OpBalCtrBtn";
	wname[i].widget = widgets.options.reset_btn;
	wname[i++].hlpname = "OpResetBtn";
	wname[i].widget = widgets.options.ok_btn;
	wname[i++].hlpname = "OpOkBtn";
	wname[i].widget = widgets.dbprog.about_btn;
	wname[i++].hlpname = "DpAboutBtn";
	wname[i].widget = widgets.dbprog.dtitle_txt;
	wname[i++].hlpname = "DpDTitleTxt";
	wname[i].widget = widgets.dbprog.extd_btn;
	wname[i++].hlpname = "DpDExtBtn";
	wname[i].widget = widgets.dbprog.trk_list;
	wname[i++].hlpname = "DpTrkList";
	wname[i].widget = widgets.dbprog.addpgm_btn;
	wname[i++].hlpname = "DpAddPgmBtn";
	wname[i].widget = widgets.dbprog.clrpgm_btn;
	wname[i++].hlpname = "DpClrPgmBtn";
	wname[i].widget = widgets.dbprog.radio_box;
	wname[i++].hlpname = "DpTimeSelBtn";
	wname[i].widget = widgets.dbprog.discid_ind;
	wname[i++].hlpname = "DiscIdInd";
	wname[i].widget = widgets.dbprog.ttitle_txt;
	wname[i++].hlpname = "DpTTitleTxt";
	wname[i].widget = widgets.dbprog.extt_btn;
	wname[i++].hlpname = "DpTExtBtn";
	wname[i].widget = widgets.dbprog.pgmseq_txt;
	wname[i++].hlpname = "DpPgmSeqTxt";
	wname[i].widget = widgets.dbprog.send_btn;
	wname[i++].hlpname = "DpSendBtn";
	wname[i].widget = widgets.dbprog.savedb_btn;
	wname[i++].hlpname = "DpSaveBtn";
	wname[i].widget = widgets.dbprog.linkdb_btn;
	wname[i++].hlpname = "DpLinkBtn";
	wname[i].widget = widgets.dbprog.loaddb_btn;
	wname[i++].hlpname = "DpLoadBtn";
	wname[i].widget = widgets.dbprog.cancel_btn;
	wname[i++].hlpname = "DpCancelBtn";
	wname[i].widget = widgets.dbextd.disc_txt;
	wname[i++].hlpname = "DdDiscTxt";
	wname[i].widget = widgets.dbextd.ok_btn;
	wname[i++].hlpname = "DdOkBtn";
	wname[i].widget = widgets.dbextd.clear_btn;
	wname[i++].hlpname = "DdClrBtn";
	wname[i].widget = widgets.dbextd.cancel_btn;
	wname[i++].hlpname = "DdCancelBtn";
	wname[i].widget = widgets.dbextt.trk_txt;
	wname[i++].hlpname = "DtTrackTxt";
	wname[i].widget = widgets.dbextt.ok_btn;
	wname[i++].hlpname = "DtOkBtn";
	wname[i].widget = widgets.dbextt.clear_btn;
	wname[i++].hlpname = "DtClrBtn";
	wname[i].widget = widgets.dbextt.cancel_btn;
	wname[i++].hlpname = "DtCancelBtn";
	wname[i].widget = widgets.dirsel.dir_list;
	wname[i++].hlpname = "DsDirList";
	wname[i].widget = widgets.dirsel.ok_btn;
	wname[i++].hlpname = "DsOkBtn";
	wname[i].widget = widgets.dirsel.cancel_btn;
	wname[i++].hlpname = "DsCancelBtn";
	wname[i].widget = widgets.linksel.link_list;
	wname[i++].hlpname = "LsLinkList";
	wname[i].widget = widgets.linksel.ok_btn;
	wname[i++].hlpname = "LsOkBtn";
	wname[i].widget = widgets.linksel.cancel_btn;
	wname[i++].hlpname = "LsCancelBtn";
	wname[i].widget = (Widget) NULL;
	wname[i].hlpname = NULL;
}


/*
 * help_getname
 *	Given a widget, return the associated help file name.
 *
 * Args:
 *	w - The widget
 *
 * Return:
 *	The help file name text string.
 */
STATIC char *
help_getname(Widget w)
{
	int	i;

	for (i = 0; wname[i].widget != NULL; i++) {
		if (w == wname[i].widget)
			return (wname[i].hlpname);
	}
	return NULL;
}


/***********************
 *   public routines   *
 ***********************/


/*
 * help_setup
 *	Top level function to set up the help subsystem.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
help_setup(widgets_t *m)
{
	int		i;
	char		xlat_str[40];
	XtTranslations	xtab1,
			xtab2;

	sprintf(xlat_str, "<Btn3Down>,<Btn3Up>: Help()\n");
	xtab1 = XtParseTranslationTable(xlat_str);
	sprintf(xlat_str, "<Btn3Down>,<Btn3Up>: PrimitiveHelp()\n");
	xtab2 = XtParseTranslationTable(xlat_str);

	/* Translations for the main window widgets */
	XtOverrideTranslations(m->main.check_box, xtab1);
	XtOverrideTranslations(m->main.eject_btn, xtab1);
	XtOverrideTranslations(m->main.poweroff_btn, xtab1);
	XtOverrideTranslations(m->main.track_ind, xtab1);
	XtOverrideTranslations(m->main.index_ind, xtab1);
	XtOverrideTranslations(m->main.time_ind, xtab1);
	XtOverrideTranslations(m->main.rptcnt_ind, xtab1);
	XtOverrideTranslations(m->main.dbmode_ind, xtab1);
	XtOverrideTranslations(m->main.progmode_ind, xtab1);
	XtOverrideTranslations(m->main.timemode_ind, xtab1);
	XtOverrideTranslations(m->main.playmode_ind, xtab1);
	XtOverrideTranslations(m->main.dtitle_ind, xtab1);
	XtOverrideTranslations(m->main.ttitle_ind, xtab1);
	XtOverrideTranslations(m->main.dbprog_btn, xtab1);
	XtOverrideTranslations(m->main.options_btn, xtab1);
	XtOverrideTranslations(m->main.time_btn, xtab1);
	XtOverrideTranslations(m->main.ab_btn, xtab1);
	XtOverrideTranslations(m->main.sample_btn, xtab1);
	XtOverrideTranslations(m->main.keypad_btn, xtab1);
	XtOverrideTranslations(m->main.help_btn, xtab1);
	XtOverrideTranslations(m->main.level_scale, xtab1);
	XtOverrideTranslations(m->main.playpause_btn, xtab1);
	XtOverrideTranslations(m->main.stop_btn, xtab1);
	XtOverrideTranslations(m->main.previdx_btn, xtab1);
	XtOverrideTranslations(m->main.nextidx_btn, xtab1);
	XtOverrideTranslations(m->main.prevtrk_btn, xtab1);
	XtOverrideTranslations(m->main.nexttrk_btn, xtab1);
	XtOverrideTranslations(m->main.rew_btn, xtab1);
	XtOverrideTranslations(m->main.ff_btn, xtab1);

	/* Translations for the keypad window widgets */
	XtOverrideTranslations(m->keypad.keypad_ind, xtab1);
	for (i = 0; i < 10; i++)
		XtOverrideTranslations(m->keypad.num_btn[i], xtab1);
	XtOverrideTranslations(m->keypad.clear_btn, xtab1);
	XtOverrideTranslations(m->keypad.enter_btn, xtab1);
	XtOverrideTranslations(m->keypad.warp_lbl, xtab1);
	XtOverrideTranslations(m->keypad.warp_scale, xtab1);
	XtOverrideTranslations(m->keypad.cancel_btn, xtab1);

	/* Translations for the options window widgets */
	XtOverrideTranslations(m->options.load_chkbox, xtab1);
	XtOverrideTranslations(m->options.load_radbox, xtab1);
	XtOverrideTranslations(m->options.exit_radbox, xtab1);
	XtOverrideTranslations(m->options.done_chkbox, xtab1);
	XtOverrideTranslations(m->options.eject_chkbox, xtab1);
	XtOverrideTranslations(m->options.chroute_radbox, xtab1);
	XtOverrideTranslations(m->options.vol_radbox, xtab1);
	XtOverrideTranslations(m->options.bal_lbl, xtab1);
	XtOverrideTranslations(m->options.bal_scale, xtab1);
	XtOverrideTranslations(m->options.ball_lbl, xtab1);
	XtOverrideTranslations(m->options.balr_lbl, xtab1);
	XtOverrideTranslations(m->options.balctr_btn, xtab1);
	XtOverrideTranslations(m->options.reset_btn, xtab1);
	XtOverrideTranslations(m->options.ok_btn, xtab1);

	/* Translations for the dbprog window widgets */
	XtOverrideTranslations(m->dbprog.about_btn, xtab1);
	XtOverrideTranslations(m->dbprog.dtitle_txt, xtab1);
	XtOverrideTranslations(m->dbprog.extd_btn, xtab1);
	XtOverrideTranslations(m->dbprog.trk_list, xtab2);
	XtOverrideTranslations(m->dbprog.radio_box, xtab1);
	XtOverrideTranslations(m->dbprog.discid_ind, xtab1);
	XtOverrideTranslations(m->dbprog.ttitle_txt, xtab1);
	XtOverrideTranslations(m->dbprog.extt_btn, xtab1);
	XtOverrideTranslations(m->dbprog.addpgm_btn, xtab1);
	XtOverrideTranslations(m->dbprog.clrpgm_btn, xtab1);
	XtOverrideTranslations(m->dbprog.pgmseq_txt, xtab1);
	XtOverrideTranslations(m->dbprog.send_btn, xtab1);
	XtOverrideTranslations(m->dbprog.savedb_btn, xtab1);
	XtOverrideTranslations(m->dbprog.linkdb_btn, xtab1);
	XtOverrideTranslations(m->dbprog.loaddb_btn, xtab1);
	XtOverrideTranslations(m->dbprog.cancel_btn, xtab1);

	/* Translations for the extd window widgets */
	XtOverrideTranslations(m->dbextd.disc_txt, xtab1);
	XtOverrideTranslations(m->dbextd.ok_btn, xtab1);
	XtOverrideTranslations(m->dbextd.clear_btn, xtab1);
	XtOverrideTranslations(m->dbextd.cancel_btn, xtab1);

	/* Translations for the extt window widgets */
	XtOverrideTranslations(m->dbextt.trk_txt, xtab1);
	XtOverrideTranslations(m->dbextt.ok_btn, xtab1);
	XtOverrideTranslations(m->dbextt.clear_btn, xtab1);
	XtOverrideTranslations(m->dbextt.cancel_btn, xtab1);

	/* Translations for the dirsel window widgets */
	XtOverrideTranslations(m->dirsel.dir_list, xtab2);
	XtOverrideTranslations(m->dirsel.ok_btn, xtab1);
	XtOverrideTranslations(m->dirsel.cancel_btn, xtab1);

	/* Translations for the linksel window widgets */
	XtOverrideTranslations(m->linksel.link_list, xtab2);
	XtOverrideTranslations(m->linksel.ok_btn, xtab1);
	XtOverrideTranslations(m->linksel.cancel_btn, xtab1);

	/* Initialize helpfile mappings */
	help_mapinit();
}


/*
 * help_popup
 *	Pop up the help window and display appropriate help text.
 *
 * Args:
 *	w - The widget which the help info is being displayed about.
 *
 * Return:
 *	Nothing.
 */
void
help_popup(Widget w)
{
	char		hlpfile[FILE_PATH_SZ],
			*hlpname;
	FILE		*fp;
	static char	*helptext = NULL;

	if ((hlpname = help_getname(w)) == NULL)
		return;

	sprintf(hlpfile, "%s/help/%s", app_data.libdir, hlpname);

	if ((fp = fopen(hlpfile, "r")) == NULL) {
		/* Can't read help file on this widget */
		XmTextSetString(widgets.help.help_txt, app_data.str_nohelp);
		XtManageChild(widgets.help.form);

		return;
	}

	if (helptext != NULL) {
		MEM_FREE(helptext);
		helptext = NULL;
	}

	while (fgets(tmpbuf, sizeof(tmpbuf), fp) != NULL) {
		if (tmpbuf[0] == '#')
			/* Comment */
			continue;

		if (helptext == NULL) {
			helptext = (char *) MEM_ALLOC(strlen(tmpbuf) + 1);

			if (helptext != NULL)
				*helptext = '\0';
		}
		else {
			helptext = (char *) MEM_REALLOC(
				helptext,
				strlen(helptext) + strlen(tmpbuf) + 1
			);
		}

		if (helptext == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
		}

		strcat(helptext, tmpbuf);
	}

	fclose(fp);

	XmTextSetString(widgets.help.help_txt, helptext);
	XtManageChild(widgets.help.form);
}


