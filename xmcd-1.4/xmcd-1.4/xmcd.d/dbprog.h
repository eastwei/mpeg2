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
#ifndef __DBPROG_H__
#define __DBPROG_H__

#ifndef LINT
static char *_dbprog_h_ident_ = "@(#)dbprog.h	5.3 94/12/28";
#endif


#define TRKLIST_FMT	" %02u  %02u:%02u  %s "
#define TRKLIST_PFLEN	16
#define UNDEF_STR	"??"
#define DBPATH_SEPCHAR	':'
#define PGM_SEPCHAR	','
#define TIME_TOTAL	1
#define TIME_TRACK	2
#define DIRSEL_SAVE	1
#define DIRSEL_LINK	2
#define OPEN_ERR	50
#define SETUID_ERR	51
#define LINK_ERR	52
#define CLOSE_ERR	53
#define OFFSET_UNKN	0xffffffff
#define OFFSET_THRESH	750


/* Public functions */
extern void	dbprog_curtrkupd(curstat_t *);
extern void	dbprog_dbclear(curstat_t *);
extern void	dbprog_dbget(curstat_t *);
extern void	dbprog_init(curstat_t *);
extern char	*dbprog_curdtitle(curstat_t *s);
extern char	*dbprog_curttitle(curstat_t *s);

/* Callback functions */
extern void	dbprog_popup(Widget, XtPointer, XtPointer);
extern void	dbprog_dtitle_new(Widget, XtPointer, XtPointer);
extern void	dbprog_trklist_play(Widget, XtPointer, XtPointer);
extern void	dbprog_trklist_select(Widget, XtPointer, XtPointer);
extern void	dbprog_ttitle_focuschg(Widget, XtPointer, XtPointer);
extern void	dbprog_ttitle_new(Widget, XtPointer, XtPointer);
extern void	dbprog_pgmseq_verify(Widget, XtPointer, XtPointer);
extern void	dbprog_pgmseq_txtchg(Widget, XtPointer, XtPointer);
extern void	dbprog_pgmseq_focuschg(Widget, XtPointer, XtPointer);
extern void	dbprog_addpgm(Widget, XtPointer, XtPointer);
extern void	dbprog_clrpgm(Widget, XtPointer, XtPointer);
extern void	dbprog_send(Widget, XtPointer, XtPointer);
extern void	dbprog_savedb(Widget, XtPointer, XtPointer);
extern void	dbprog_loaddb(Widget, XtPointer, XtPointer);
extern void	dbprog_link(Widget, XtPointer, XtPointer);
extern void	dbprog_cancel(Widget, XtPointer, XtPointer);
extern void	dbprog_timedpy(Widget, XtPointer, XtPointer);
extern void	dbprog_extd(Widget, XtPointer, XtPointer);
extern void	dbprog_extt(Widget, XtPointer, XtPointer);
extern void	dbprog_set_changed(Widget, XtPointer, XtPointer);
extern void	dbprog_extd_ok(Widget, XtPointer, XtPointer);
extern void	dbprog_extd_clear(Widget, XtPointer, XtPointer);
extern void	dbprog_extd_cancel(Widget, XtPointer, XtPointer);
extern void	dbprog_extt_ok(Widget, XtPointer, XtPointer);
extern void	dbprog_extt_clear(Widget, XtPointer, XtPointer);
extern void	dbprog_extt_cancel(Widget, XtPointer, XtPointer);
extern void	dbprog_dirsel_select(Widget, XtPointer, XtPointer);
extern void	dbprog_dirsel_ok(Widget, XtPointer, XtPointer);
extern void	dbprog_dirsel_cancel(Widget, XtPointer, XtPointer);
extern void	dbprog_linksel_select(Widget, XtPointer, XtPointer);
extern void	dbprog_linksel_ok(Widget, XtPointer, XtPointer);
extern void	dbprog_linksel_cancel(Widget, XtPointer, XtPointer);

#endif	/* __DBPROG_H__ */
