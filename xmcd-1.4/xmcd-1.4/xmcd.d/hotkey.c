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
static char *_hotkey_c_ident_ = "@(#)hotkey.c	5.2 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "xmcd.d/xmcd.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/cdfunc.h"
#include "xmcd.d/hotkey.h"


typedef struct grablist {
	Widget		gr_button;
	KeyCode		gr_keycode;
	Modifiers	gr_modifier;
	struct grablist	*next;
} grablist_t;


typedef struct {
	char		*name;
	Modifiers	mask;
} modtab_t;


extern appdata_t	app_data;
extern widgets_t	widgets;
extern FILE		*errfp;

STATIC modtab_t		modtab[8] = {
	{ "Shift",	ShiftMask	},
	{ "Lock",	LockMask	},
	{ "Ctrl",	ControlMask	},
	{ "Mod1",	Mod1Mask	},
	{ "Mod2",	Mod2Mask	},
	{ "Mod3",	Mod3Mask	},
	{ "Mod4",	Mod4Mask	},
	{ "Mod5",	Mod5Mask	},
};

#define TOTAL_GRABLISTS	2
#define MAIN_LIST	0
#define KEYPAD_LIST	1

STATIC grablist_t	*grablists[TOTAL_GRABLISTS] = {
	NULL, NULL
};


/***********************
 *  internal routines  *
 ***********************/


/*
 * hotkey_label_match
 *	Find a letter in a pushbutton widget label that would
 *	match the hotkey character associated with that button,
 *	and set that letter as the mnemonic (causing the letter
 *	to be displayed with an underscore).
 *
 * Args:
 *	btn - The button widget
 *	keycode - The X keycode of the hotkey character
 *	modifier - The keycode modifier of the hotkey character
 *
 * Return:
 *	Nothing.
 */
STATIC void
hotkey_label_match(Widget btn, KeyCode keycode, unsigned int modifier)
{
	char		*mstr,
			*lstr;
	Display		*display = XtDisplay(widgets.toplevel);
	KeySym		ks;
	XmString	xs;

	ks = XKeycodeToKeysym(display, keycode, 0),
	mstr = XKeysymToString(ks);

	if (mstr == NULL)
		return;

	XtVaGetValues(btn, XmNlabelString, &xs, NULL);

	if (XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &lstr)) {
		/* No need to set mnemonic if the first
		 * button character is a digit.
		 */
		if (isdigit(lstr[0]))
			return;

		/* Make the first letter of the button label
		 * match if possible, even if the
		 * capitalization is wrong.
		 */
		if (toupper(mstr[0]) == toupper(lstr[0]) &&
		    !(modifier & ~ShiftMask)) {
			char	s[2];

			s[0] = lstr[0];
			s[1] = '\0';

			ks = XStringToKeysym(s);
		}
	}

	/* Set the mnemonic */
	XtVaSetValues(btn, XmNmnemonic, ks, NULL);
}


/*
 * hotkey_set_mnemonics
 *	Set the mnemonics of all pushbuttons with hotkey support.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
hotkey_set_mnemonics(void)
{
	int		i;
	grablist_t	*p;
	Widget		prev_btn = (Widget) NULL;

	/* Set mnemonics on all button faces */
	for (i = 0; i < TOTAL_GRABLISTS; i++) {
		for (p = grablists[i]; p != NULL; p = p->next) {
			if (p->gr_button != (Widget) NULL) {
				if (prev_btn == p->gr_button)
					continue;

				hotkey_label_match(
					p->gr_button,
					p->gr_keycode,
					p->gr_modifier
				);
			}
			prev_btn = p->gr_button;
		}
	}
}


/*
 * hotkey_parse_xlat_line
 *	A limited translation string parser.
 *
 * Args:
 *	line - The translation text string line to be parsed
 *	modp - The key modifier return string
 *	keyp - The key event return string
 *	btnp - The button event return string
 *
 * Return:
 *	Nothing.
 */
STATIC bool_t
hotkey_parse_xlat_line(
	char *line,
	char **modp,
	char **keyp,
	char **btnp
)
{
	char		*p,
			*q;
	static char	modstr[12],
			keystr[12],
			btnstr[48];

	/* Get modifier specification */
	p = line;
	q = strchr(p, '<');

	if (q == NULL)
		return FALSE;
	else if (q > p) {
		*q = '\0';
		strcpy(modstr, p);
		*modp = modstr;
		*q = '<';
	}

	/* Get event specification */
	p = q + 1;
	q = strchr(p, '>');

	if (q == NULL || q == p)
		return FALSE;
	else {
		*q = '\0';

		/* We are interested only in key events here */
		if (strncmp(p, "Key", 3) != 0)
			return FALSE;

		*q = '>';
	}

	/* Get key specification */
	p = q + 1;
	q = strchr(p, ':');

	if (q == NULL || q == p)
		return FALSE;
	else {
		*q = '\0';
		strcpy(keystr, p);
		*keyp = keystr;
		*q = ':';
	}

	/* Get associated button */
	p = q + 1;
	q = strchr(p, '(');

	if (q == NULL || q == p)
		return FALSE;
	else {
		p = q + 1;
		q = strchr(p, ',');

		if (q == NULL || q == p)
			return FALSE;
		else {
			*q = '\0';
			strcpy(btnstr, p);
			*btnp = btnstr;
			*q = ',';
		}
	}

	return TRUE;
}


/*
 * hotkey_build_grablist
 *	Build a linked list of widgets which have associated
 *	hotkey support, and information about the hotkey.  These
 *	keys will be grabbed when the parent form window has input
 *	focus.
 *
 * Args:
 *	form - The parent form widget
 *	str - The translation string specifying the hotkey
 *	listhead - The list head (return)
 *
 * Return:
 *	Nothing.
 */
STATIC void
hotkey_build_grablist(Widget form, char *str, grablist_t **listhead)
{
	int		i;
	char		*p,
			*q,
			*end,
			*modstr,
			*keystr,
			*btnstr;
	grablist_t	*g;

	p = str;
	end = p + strlen(p);

	do {
		while (isspace(*p))
			p++;
		q = strchr(p, '\n');

		if (p >= end)
			break;

		if (q == NULL) {
			if (q > end)
				break;
		}
		else
			*q = '\0';

		modstr = keystr = btnstr = NULL;

		/* Parse translation line */
		if (*p != '#' &&
		    hotkey_parse_xlat_line(p, &modstr, &keystr, &btnstr) &&
		    keystr != NULL) {

			/* Allocate new list element */
			if (*listhead == NULL) {
				*listhead = g = (grablist_t *)(void *)
					MEM_ALLOC(sizeof(grablist_t));

				if (g == NULL) {
					cd_fatal_popup(
						app_data.str_fatal,
						app_data.str_nomemory
					);
				}
			}
			else {
				g->next = (grablist_t *)(void *)
					MEM_ALLOC(sizeof(grablist_t));

				if (g->next == NULL) {
					cd_fatal_popup(
						app_data.str_fatal,
						app_data.str_nomemory
					);
				}

				g = g->next;
			}
			g->next = NULL;

			g->gr_keycode = XKeysymToKeycode(
				XtDisplay(widgets.toplevel),
				XStringToKeysym(keystr)
			);

			g->gr_modifier = 0;
			if (modstr != NULL) {
				for (i = 0; i < 8; i++) {
					if (strcmp(modtab[i].name,
						   modstr) == 0) {
						g->gr_modifier =
							modtab[i].mask;
						break;
					}
				}
			}

			g->gr_button = (Widget) NULL;
			if (btnstr != NULL)
				g->gr_button = XtNameToWidget(form, btnstr);
		}

		if (q != NULL) {
			*q = '\n';
			p = q + 1;
		}
		else
			p = end;

	} while (p < end);
}


/***********************
 *   public routines   *
 ***********************/


/*
 * hotkey_setup
 *	Top level setup function for the hotkey subsystem.  Called
 *	once at program startup.
 *
 * Args:
 *	m - The main widgets structure.
 *
 * Return:
 *	Nothing.
 */
void
hotkey_setup(widgets_t *m)
{
	char	xlat_str[MAX_TRANSLATIONS_SZ];

	/* Translations for the main window form */
	sprintf(xlat_str,
		"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		app_data.btnlbl_key,
		app_data.lock_key,
		app_data.repeat_key,
		app_data.shuffle_key,
		app_data.eject_key,
		app_data.poweroff_key,
		app_data.dbprog_key,
		app_data.help_key,
		app_data.options_key,
		app_data.time_key,
		app_data.ab_key,
		app_data.sample_key,
		app_data.keypad_key,
		app_data.playpause_key,
		app_data.stop_key,
		app_data.prevtrk_key,
		app_data.nexttrk_key,
		app_data.previdx_key,
		app_data.nextidx_key,
		app_data.rew_key,
		app_data.ff_key);

	XtOverrideTranslations(
		m->main.form,
		XtParseTranslationTable(xlat_str)
	);

	hotkey_build_grablist(
		m->main.form,
		xlat_str,
		&grablists[MAIN_LIST]
	);

	/* Translations for the keypad window form */
	sprintf(xlat_str,
		"%s%s%s%s%s%s%s%s%s%s%s%s%s",
		app_data.keypad0_key,
		app_data.keypad1_key,
		app_data.keypad2_key,
		app_data.keypad3_key,
		app_data.keypad4_key,
		app_data.keypad5_key,
		app_data.keypad6_key,
		app_data.keypad7_key,
		app_data.keypad8_key,
		app_data.keypad9_key,
		app_data.keypadclear_key,
		app_data.keypadenter_key,
		app_data.keypadcancel_key);

	XtOverrideTranslations(
		m->keypad.form,
		XtParseTranslationTable(xlat_str)
	);

	hotkey_build_grablist(
		m->keypad.form,
		xlat_str,
		&grablists[KEYPAD_LIST]
	);

	/* Set key label mnemonics */
	hotkey_set_mnemonics();
}


/*
 * hotkey_grabkeys
 *	Grab all keys used as hotkeys in the specified window form.
 *
 * Args:
 *	form - The parent form widget.
 *
 * Return:
 *	Nothing.
 */
void
hotkey_grabkeys(Widget form)
{
	grablist_t	*list,
			*p;

	if (form == widgets.main.form)
		list = grablists[MAIN_LIST];
	else if (form == widgets.keypad.form)
		list = grablists[KEYPAD_LIST];
	else
		list = NULL;

	for (p = list; p != NULL; p = p->next) {
		XtGrabKey(
			form,
			p->gr_keycode,
			p->gr_modifier,
			True,
			GrabModeAsync,
			GrabModeAsync
		);
	}
}


/*
 * hotkey_ungrabkeys
 *	Ungrab all keys used as hotkeys in the specified window form.
 *
 * Args:
 *	form - The parent form widget.
 *
 * Return:
 *	Nothing.
 */
void
hotkey_ungrabkeys(Widget form)
{
	grablist_t	*list,
			*p;

	if (form == widgets.main.form)
		list = grablists[MAIN_LIST];
	else if (form == widgets.keypad.form)
		list = grablists[KEYPAD_LIST];
	else
		list = NULL;

	for (p = list; p != NULL; p = p->next)
		XtUngrabKey(form, p->gr_keycode, p->gr_modifier);
}


/*
 * hotkey
 *	Widget action routine to handle hotkey events
 */
void
hotkey(Widget w, XEvent *ev, String *args, Cardinal *num_args)
{
	int	i;
	Widget	action_widget;

	if ((int) *num_args <= 0)
		return;	/* Error: should have at least one arg */

	if ((action_widget = XtNameToWidget(w, args[0])) == (Widget) NULL)
		return;	/* Can't find widget */

	/* Switch keyboard focus to the widget of interest */
	XmProcessTraversal(action_widget, XmTRAVERSE_CURRENT);

	for (i = 1; i < (int) *num_args; i++)
		/* Invoke the named action of the specified widget */
		XtCallActionProc(action_widget, args[i], ev, NULL, 0);
}


