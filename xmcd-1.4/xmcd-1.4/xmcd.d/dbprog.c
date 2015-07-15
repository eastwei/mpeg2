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
static char *_dbprog_c_ident_ = "@(#)dbprog.c	5.5 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "common.d/patchlevel.h"
#include "common.d/util.h"
#include "xmcd.d/xmcd.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/cdfunc.h"
#include "xmcd.d/dbprog.h"
#include "libdi.d/libdi.h"


typedef struct database {
	word32_t	discid;			/* Magic disc ID */
	byte_t		ntrk;			/* Number of disc tracks */
	char		*dbfile;		/* Path to database file */
	char		category[STR_BUF_SZ];	/* CD category */
	char		*dtitle;		/* Disc title */
	char		*trklist[MAXTRACK];	/* Track title list */
	char		*extd;			/* Extended disc info */
	char		*extt[MAXTRACK];	/* Extended track info */
	char		*sav_extt[MAXTRACK];	/* Bkup extended track info */
	char		*playorder;		/* Track play order */
} database_t;


typedef struct linkopts {
	char		*dtitle;		/* Disc title */
	char		idstr[9];		/* Disc id string */
	word32_t	offset;			/* Distance measure */
	struct linkopts	*next;			/* Link to next item */
} linkopts_t;


extern widgets_t	widgets;
extern appdata_t	app_data;
extern char		**dbdirs;
extern FILE		*errfp;

STATIC char		timemode;		/* Time display mode flag */
STATIC int		sel_pos = -1,		/* Track list select position */
			ind_pos = -1,		/* Track list highlight pos */
			linksel_pos = -1,	/* Link select position */
			extt_pos = -1,		/* Ext track info position */
			dirsel_mode = DIRSEL_SAVE,
						/* Directory selector mode */
			trk_off[MAXTRACK];	/* Link track offsets */
STATIC bool_t		title_edited = FALSE,	/* Track title edited flag */
			extt_setup = FALSE,	/* Ext track info setup */
			pgmseq_editing = FALSE,	/* Pgm seq edited flag */
			dbprog_changed = FALSE,	/* Flag to indicate change */
			extd_manage = FALSE,	/* Whether to manage extd */
			extt_manage = FALSE;	/* Whether to manage extt */
STATIC database_t	cur_db;			/* Database entry of CD */
STATIC linkopts_t	*linkhead = NULL;	/* Link options list head */

#ifndef MIN
#define MIN(a,b)	(((a) > (b)) ? (b) : (a))
#endif
#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif



/***********************
 *  internal routines  *
 ***********************/

/*
 * dbprog_strcat
 *	Concatenate two text strings with special handling for newline
 *	and tab character translations.
 *
 * Args:
 *	s1 - The first text string and destination string.
 *	s2 - The second text string.
 *
 * Return:
 *	Pointer to the resultant string, or NULL if failed.
 */
STATIC char *
dbprog_strcat(char *s1, char *s2)
{
	char	*cp = s1;

	if (s1 == NULL || s2 == NULL)
		return NULL;

	/* Concatenate two strings, with special handling for newline
	 * and tab characters.
	 */
	for (s1 += strlen(s1); *s2 != '\0'; s1++, s2++) {
		if (*s2 == '\\') {
			switch (*(s2 + 1)) {
			case 'n':
				*s1 = '\n';
				s2++;
				break;
			case 't':
				*s1 = '\t';
				s2++;
				break;
			default:
				*s1 = *s2;
				break;
			}
		}
		else
			*s1 = *s2;
	}
	*s1 = '\0';

	return (cp);
}


/*
 * dbprog_strcmp
 *	Compare two strings a la strcmp(), except it is case-insensitive.
 *
 * Args:
 *	s1 - The first text string.
 *	s2 - The second text string.
 *
 * Return:
 *	Compare value.  See strcmp(3).
 */
STATIC int
dbprog_strcmp(char *s1, char *s2)
{
	char	*buf1,
		*buf2,
		*p;
	int	ret;

	if (s1 == NULL || s2 == NULL)
		return 0;

	/* Allocate tmp buffers */
	if ((buf1 = (char *) MEM_ALLOC(strlen(s1)+1)) == NULL ||
	    (buf2 = (char *) MEM_ALLOC(strlen(s2)+1)) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return 0;
	}

	/* Convert both strings to lower case and store in tmp buffer */
	for (p = buf1; *s1 != '\0'; s1++, p++)
		*p = (isupper(*s1)) ? tolower(*s1) : *s1;
	*p = '\0';
	for (p = buf2; *s2 != '\0'; s2++, p++)
		*p = (isupper(*s2)) ? tolower(*s2) : *s2;
	*p = '\0';

	ret = strcmp(buf1, buf2);

	MEM_FREE(buf1);
	MEM_FREE(buf2);

	return (ret);
}


/*
 * dbprog_sum
 *	Convert an integer to its text string representation, and
 *	compute its checksum.  Used by dbprog_discid to derive the
 *	disc ID.
 *
 * Args:
 *	n - The integer value.
 *
 * Return:
 *	The integer checksum.
 */
STATIC int
dbprog_sum(int n)
{
	char	buf[12],
		*p;
	int	ret = 0;

	/* For backward compatibility this algorithm must not change */
	sprintf(buf, "%lu", n);
	for (p = buf; *p != '\0'; p++)
		ret += (*p - '0');

	return (ret);
}


/*
 * dbprog_discid
 *	Compute a magic disc ID based on the number of tracks,
 *	the length of each track, and a checksum of the string
 *	that represents the offset of each track.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The integer disc ID.
 */
STATIC word32_t
dbprog_discid(curstat_t *s)
{
	int	i,
		t = 0,
		n = 0;

	/* For backward compatibility this algorithm must not change */
	for (i = 0; i < (int) s->tot_trks; i++) {
		n += dbprog_sum((s->trkinfo[i].min * 60) + s->trkinfo[i].sec);

		t += ((s->trkinfo[i+1].min * 60) + s->trkinfo[i+1].sec) -
		     ((s->trkinfo[i].min * 60) + s->trkinfo[i].sec);
	}

	return ((n % 0xff) << 24 | t << 8 | s->tot_trks);
}


/*
 * dbprog_dpyid
 *	Display the disc ID in the Disc ID Indicator.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_dpyid(void)
{
	XmString	xs;
	char		str[STR_BUF_SZ];
	static char	prev[STR_BUF_SZ];

	if (cur_db.discid != 0)
		sprintf(str, "%s\n%08x", cur_db.category, cur_db.discid);
	else
		sprintf(str, "\n   --   ");

	if (strcmp(str, prev) == 0)
		/* No change */
		return;

	xs = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

	XtVaSetValues(
		widgets.dbprog.discid_ind,
		XmNlabelString,
		xs,
		NULL
	);

	XmStringFree(xs);

	strcpy(prev, str);
}


/*
 * dbprog_parse
 *	Parse the program mode play sequence text string, and
 *	update the playorder table in the curstat_t structure.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	TRUE=success, FALSE=error.
 */
STATIC bool_t
dbprog_parse(curstat_t *s)
{
	int	i,
		j,
		n;
	char	*p,
		*q,
		*tmpbuf;
	bool_t	last = FALSE;

	if (cur_db.playorder == NULL)
		/* Nothing to do */
		return TRUE;

	n = strlen(cur_db.playorder) + 1;
	if ((tmpbuf = (char *) MEM_ALLOC(n)) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return FALSE;
	}

	strcpy(tmpbuf, cur_db.playorder);

	s->prog_tot = 0;

	for (i = 0, p = q = tmpbuf; i < MAXTRACK; i++, p = ++q) {
		/* Skip p to the next digit */
		for (; !isdigit(*p) && *p != '\0'; p++)
			;

		if (*p == '\0')
			/* No more to do */
			break;

		/* Skip q to the next non-digit */
		for (q = p; isdigit(*q); q++)
			;

		if (*q == PGM_SEPCHAR)
			*q = '\0';
		else if (*q == '\0')
			last = TRUE;
		else {
			MEM_FREE(tmpbuf);
			return FALSE;
		}

		if (q > p) {
			/* Update play sequence */
			for (j = 0; j < MAXTRACK; j++) {
				if (s->trkinfo[j].trkno == atoi(p)) {
					s->playorder[i] = j;
					s->prog_tot++;
					break;
				}
			}

			if (j >= MAXTRACK) {
				MEM_FREE(tmpbuf);
				return FALSE;
			}
		}

		if (last)
			break;
	}

	MEM_FREE(tmpbuf);

	return TRUE;
}


/*
 * dbprog_listupd
 *	Update the track list display to reflect the contents of
 *	the trkinfo table in the curstat_t structure.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_listupd(curstat_t *s)
{
	int		i,
			n,
			secs;
	char		*str;
	byte_t		min,
			sec;
	XmString	xs;


	for (i = 0; i < (int) s->tot_trks; i++) {
		if (timemode == TIME_TOTAL) {
			min = (byte_t) s->trkinfo[i].min;
			sec = (byte_t) s->trkinfo[i].sec;
		}
		else {
			secs = ((s->trkinfo[i+1].min * 60 +
				s->trkinfo[i+1].sec) - 
			        (s->trkinfo[i].min * 60 +
				s->trkinfo[i].sec));
			min = (byte_t) (secs / 60);
			sec = (byte_t) (secs % 60);
		}

		n = strlen((cur_db.trklist[i] == NULL) ?
			   UNDEF_STR : cur_db.trklist[i]) + TRKLIST_PFLEN;

		if ((str = (char *) MEM_ALLOC(n)) == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
			return;
		}

		if (cur_db.trklist[i] != NULL)
			sprintf(str, TRKLIST_FMT, s->trkinfo[i].trkno,
				min, sec, cur_db.trklist[i]);
		else
			sprintf(str, TRKLIST_FMT, s->trkinfo[i].trkno,
				min, sec, UNDEF_STR);

		if (s->mode != M_NODISC && s->cur_trk >= 0 &&
		    curtrk_pos(s) == i)
			xs = XmStringCreate(str, CHSET2);
		else
			xs = XmStringCreate(str, CHSET1);

		XmListAddItemUnselected(widgets.dbprog.trk_list, xs, i + 1);

		XmStringFree(xs);
		MEM_FREE(str);
	}

	if (sel_pos > 0)
		/* This item is previously selected */
		XmListSelectPos(widgets.dbprog.trk_list, sel_pos, False);
}


/*
 * dbprog_structupd
 *	Update the cur_db structure to match the state of the various
 *	input fields in the database/program window.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_structupd(curstat_t *s)
{
	/* Disc title */
	if (cur_db.dtitle != NULL) {
		XmTextSetString(widgets.dbprog.dtitle_txt, cur_db.dtitle);
		XmTextSetInsertionPosition(
			widgets.dbprog.dtitle_txt,
			strlen(cur_db.dtitle)
		);
	}
	else {
		XmTextSetString(widgets.dbprog.dtitle_txt, UNDEF_STR);
		XmTextSetInsertionPosition(widgets.dbprog.dtitle_txt, 2);
	}


	/* Disc extended info popup */
	if (cur_db.extd != NULL)
		XmTextSetString(widgets.dbextd.disc_txt, cur_db.extd);
	else
		XmTextSetString(widgets.dbextd.disc_txt, "");

	/* Number of tracks */
	cur_db.ntrk = s->tot_trks;

	/* Track title list */
	sel_pos = -1;
	dbprog_listupd(s);

	/* Track extended info popup: This is loaded when the user
	 * pops it up.
	 */
	XmTextSetString(widgets.dbextt.trk_txt, "");

	/* Program sequence */
	if (cur_db.playorder != NULL) {
		XmTextSetString(widgets.dbprog.pgmseq_txt, cur_db.playorder);
		XmTextSetInsertionPosition(
			widgets.dbprog.pgmseq_txt,
			strlen(cur_db.playorder)
		);

		XtSetSensitive(widgets.dbprog.clrpgm_btn, True);
	}
	else {
		XmTextSetString(widgets.dbprog.pgmseq_txt, "");

		XtSetSensitive(widgets.dbprog.clrpgm_btn, False);
	}

	dbprog_changed = FALSE;
	XtSetSensitive(widgets.dbprog.addpgm_btn, False);
	XtSetSensitive(widgets.dbprog.savedb_btn, False);
	XtSetSensitive(widgets.dbprog.extd_btn, True);
	XtSetSensitive(widgets.dbprog.extt_btn, False);

	/* Update display */
	dpy_dbmode(s);

	if (cur_db.dbfile == NULL && dbdirs[0] != NULL && dbdirs[1] == NULL) {
		/* Only one possible database directory: Set
		 * database file path.
		 */
		cur_db.dbfile = (char *) MEM_ALLOC(strlen(dbdirs[0]) + 10);
		if (cur_db.dbfile == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
			return;
		}
		sprintf(cur_db.dbfile, "%s/%08x", dbdirs[0], cur_db.discid);
	}
}


/*
 * dbprog_extdupd
 *	Update the Extended disc info text field in the cur_db structure
 *	to match the contents shown in the text widget.
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_extdupd(void)
{
	/* Update in-core database structure */
	if (cur_db.extd != NULL)
		MEM_FREE(cur_db.extd);

	if ((cur_db.extd = XmTextGetString(widgets.dbextd.disc_txt)) == NULL)
		return;

	if (cur_db.extd[0] == '\0') {
		MEM_FREE(cur_db.extd);
		cur_db.extd = NULL;
	}

	if (dbprog_changed)
		XtSetSensitive(widgets.dbprog.savedb_btn, True);
}


/*
 * dbprog_exttupd
 *	Update the Extended track info text field in the cur_db structure
 *	to match the contents shown in the text widget.
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_exttupd(void)
{
	if (extt_pos < 0)
		return;

	/* Update in-core database structure */
	if (cur_db.extt[extt_pos] != NULL)
		MEM_FREE(cur_db.extt[extt_pos]);

	if ((cur_db.extt[extt_pos] =
		XmTextGetString(widgets.dbextt.trk_txt)) == NULL) {
		extt_pos = -1;
		return;
	}

	if (cur_db.extt[extt_pos][0] == '\0') {
		MEM_FREE(cur_db.extt[extt_pos]);
		cur_db.extt[extt_pos] = NULL;
	}

	extt_pos = -1;

	if (dbprog_changed)
		XtSetSensitive(widgets.dbprog.savedb_btn, True);
}


/*
 * dbprog_putentry
 *	Write one information item into a database file.  Used by
 *	dbprog_dbput to update the CD database file.
 *
 * Args:
 *	fp - file stream handle
 *	idstr - The information identifier keyword text string
 *	entry - The information text string
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_putentry(FILE *fp, char *idstr, char *entry)
{
	int	i,
		n;
	char	*cp;

	if (fp == NULL || idstr == NULL)
		/* Paranoia */
		return;

	if (entry == NULL)
		/* Null entry */
		fprintf(fp, "%s=\n", idstr);
	else {
		/* Write entry to file, splitting into multiple lines
		 * if necessary.  Special handling for newline and tab
		 * characters.
		 */
		cp = entry;

		do {
			fprintf(fp, "%s=", idstr);

			n = MIN((int) strlen(cp), STR_BUF_SZ);

			for (i = 0; i < n; i++, cp++) {
				switch (*cp) {
				case '\n':
					fprintf(fp, "\\n");
					break;
				case '\t':
					fprintf(fp, "\\t");
					break;
				default:
					fprintf(fp, "%c", *cp);
					break;
				}
			}

			fprintf(fp, "\n");

		} while (n == STR_BUF_SZ);
	}
}


/*
 * dbprog_dbput
 *	Write in-core CD database entry to disc file.  This routine
 *	forks a child to do the actual file I/O.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_dbput(curstat_t *s)
{
	char		idstr[12],
			errstr[ERR_BUF_SZ];
	int		i;
	waitret_t	stat_val;
	int		dbfile_mode;
	pid_t		cpid;
	FILE		*fp;

	if (cur_db.dbfile == NULL)
		/* Output file undefined */
		return;

	/* Update structures if necessary */
	if (XtIsManaged(widgets.dbextd.form))
		dbprog_extdupd();
	if (XtIsManaged(widgets.dbextt.form))
		dbprog_exttupd();

	/* Change to watch cursor */
	cd_busycurs(TRUE);

	switch (cpid = fork()) {
	case 0:
		/* Child process */
		break;
	case -1:
		sprintf(errstr, app_data.str_saverr_fork, errno);
		cd_warning_popup(app_data.str_warning, errstr);
		return;
	default:
		/* Parent process: wait for child to exit */
		while (waitpid(cpid, &stat_val, 0) != cpid)
			;

		/* Change to normal cursor */
		cd_busycurs(FALSE);

		if (WIFEXITED(stat_val)) {
			switch (WEXITSTATUS(stat_val)) {
			case SETUID_ERR:
				sprintf(errstr, app_data.str_saverr_suid,
					get_ouid(), get_ogid());
				cd_warning_popup(app_data.str_warning, errstr);
				return;

			case OPEN_ERR:
				sprintf(errstr, app_data.str_saverr_open);
				cd_warning_popup(app_data.str_warning, errstr);
				return;

			case CLOSE_ERR:
				sprintf(errstr, app_data.str_saverr_close);
				cd_warning_popup(app_data.str_warning, errstr);
				return;

			default:
				break;
			}
		}
		else if (WIFSIGNALED(stat_val)) {
			sprintf(errstr, app_data.str_saverr_killed,
				WTERMSIG(stat_val));
			cd_warning_popup(app_data.str_warning, errstr);
			return;
		}

		/* Database mode is on */
		s->cddb = TRUE;

		/* All edits have been saved, so clear flag */
		dbprog_changed = FALSE;

		/* Update display */
		dpy_dbmode(s);
		dbprog_dpyid();

		XtSetSensitive(widgets.dbprog.send_btn, True);
		XtSetSensitive(widgets.dbprog.linkdb_btn, False);
		XtSetSensitive(widgets.dbprog.savedb_btn, False);
		XmProcessTraversal(
			widgets.dbprog.cancel_btn,
			XmTRAVERSE_CURRENT
		);
		return;
	}

	DBGPRN(errfp, "\nSetting uid to %d, gid to %d\n",
		get_ouid(), get_ogid());

	/* Force uid and gid to original setting */
	if (setuid(get_ouid()) < 0 || setgid(get_ogid()) < 0)
		exit(SETUID_ERR);

	DBGPRN(errfp, "\nWriting CD database file %s\n", cur_db.dbfile);

	if ((fp = fopen(cur_db.dbfile, "w")) == NULL)
		exit(OPEN_ERR);

	/* File header */
	fprintf(fp, "# xmcd %s CD database file\n", VERSION);
	fprintf(fp, "# Copyright (C) 1995 Ti Kan\n");

	fprintf(fp, "#\n# Track frame offsets:\n");
	for (i = 0; i < (int) cur_db.ntrk; i++)
		fprintf(fp, "#\t%u\n", s->trkinfo[i].addr + MSF_OFFSET(s));

	fprintf(fp, "#\n# Disc length: %u seconds\n#\n",
		s->trkinfo[(int) cur_db.ntrk].min * 60 +
		s->trkinfo[(int) cur_db.ntrk].sec);

	/* Disc ID magic number */
	fprintf(fp, "DISCID=%08x\n", cur_db.discid);

	/* Disc artist/title */
	dbprog_putentry(fp, "DTITLE", cur_db.dtitle);

	/* Track titles */
	for (i = 0; i < (int) cur_db.ntrk; i++) {
		sprintf(idstr, "TTITLE%u", i);
		dbprog_putentry(fp, idstr, cur_db.trklist[i]);
	}

	/* Extended disc information */
	dbprog_putentry(fp, "EXTD", cur_db.extd);

	/* Extended track information */
	for (i = 0; i < (int) cur_db.ntrk; i++) {
		sprintf(idstr, "EXTT%u", i);
		dbprog_putentry(fp, idstr, cur_db.extt[i]);
	}

	/* Track program sequence */
	dbprog_putentry(fp, "PLAYORDER", cur_db.playorder);

	if (fclose(fp) != 0)
		exit(CLOSE_ERR);

	/* Set the database file permissions */
	sscanf(app_data.dbfile_mode, "%o", &dbfile_mode);

	/* Make sure the file is at least readable to the user just
	 * in case dbfile_mode is bogus.
	 */
	dbfile_mode |= S_IRUSR;

	/* Turn off extraneous bits */
	dbfile_mode &= ~(S_ISUID | S_ISGID | S_IXUSR | S_IXGRP | S_IXOTH);

	/* Set file permission */
	chmod(cur_db.dbfile, (mode_t) dbfile_mode);

	/* Child exits here. */
	exit(0);
}


/*
 * dbprog_dbsend
 *	Send current CD database entry to archive site via e-mail.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_dbsend(curstat_t *s)
{
	pid_t		cpid;
	waitret_t	stat_val;
	char		*p,
			*q,
			subject[STR_BUF_SZ],
			cmd[256];

	/* Change to the watch cursor */
	cd_busycurs(TRUE);

	/* Create child process to send mail */
	switch (cpid = fork()) {
	case 0:
		/* Child process */
		break;
	case -1:
		cd_warning_popup(app_data.str_warning, app_data.str_mailerr);
		return;
	default:
		/* Parent process: wait for child to exit */
		while (waitpid(cpid, &stat_val, 0) != cpid)
			;

		/* Change to normal cursor */
		cd_busycurs(FALSE);

		if ((WIFEXITED(stat_val) && WEXITSTATUS(stat_val) != 0) ||
		    WIFSIGNALED(stat_val)) {
			cd_warning_popup(
				app_data.str_warning,
				app_data.str_mailerr
			);
		}
		else {
			/* Make the send button insensitive */
			XtSetSensitive(widgets.dbprog.send_btn, False);
		}

		return;
	}

	DBGPRN(errfp, "\nSetting uid to %d, gid to %d\n",
		get_ouid(), get_ogid());

	/* Force uid and gid to original setting */
	if (setuid(get_ouid()) < 0 || setgid(get_ogid()) < 0)
		exit(SETUID_ERR);


	/* Mail command */
	for (p = cmd, q = app_data.cddb_mailcmd; *q != '\0'; p++, q++) {
		if (*q ==  '%') {
			/* Support the special meanings of %S, %A and %F */
			switch (*(q+1)) {
			case 'S':
				/* Mail subject */
				sprintf(subject, "cddb %s %08x",
					cur_db.category[0] == '\0' ?
					"unknown" : cur_db.category,
					cur_db.discid);
				strcpy(p, subject);
				p += strlen(subject) - 1;
				q++;
				break;

			case 'A':
				/* Mail address */
				strcpy(p, app_data.cddb_mailsite);
				p += strlen(app_data.cddb_mailsite) - 1;
				q++;
				break;

			case 'F':
				/* CD database file path */
				strcpy(p, cur_db.dbfile);
				p += strlen(cur_db.dbfile) - 1;
				q++;
				break;

			default:
				p++;
				q++;
				sprintf(p, "%%c", *q);
				break;
			}
		}
		else {
			*p = *q;
		}
	}
	*p = '\0';

	DBGPRN(errfp, "\nSend CDDB: [%s]\n", cmd);

	/* Send the mail */
	if (di_isdemo()) {
		/* Don't send mail if in demo mode */
		fprintf(errfp, "DEMO mode: mail not sent.\n");
	}
	else if (system(cmd) != 0)
		exit(1);

	/* Child exits here */
	exit(0);
}


/*
 * dbprog_add_linkent
 *	Add an entry to the link-search list in sorted order.  Used
 *	by dbprog_bld_linkopts.
 *
 * Args:
 *	dtitle - Disc artist/title text string
 *	idstr - A text string representation of the magic number
 *	offset - A measure how good the track addresses match
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_add_linkent(char *dtitle, char *idstr, word32_t offset)
{
	int		n;
	linkopts_t	*p,
			*q,
			*r;

	if (dtitle == NULL || idstr == NULL)
		/* Paranoia */
		return;

	if ((p = (linkopts_t *)(void *)
		 MEM_ALLOC(sizeof(linkopts_t))) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}
	if ((p->dtitle = (char *) MEM_ALLOC(strlen(dtitle) + 1)) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}

	strcpy(p->idstr, idstr);
	strcpy(p->dtitle, dtitle);
	p->offset = offset;

	if (linkhead == NULL) {
		/* This is the first element */
		linkhead = p;
		p->next = NULL;
	}
	else {
		/* Add to list in sorted order */
		for (q = linkhead, r = NULL; q != NULL; q = q->next) {
			if ((n = dbprog_strcmp(dtitle, q->dtitle)) == 0) {
				/* Already in list: no need to add */
				MEM_FREE(p->dtitle);
				MEM_FREE((char *) p);
				return;
			}
			else if (n < 0) {
				/* Track timings outside of criteria:
				 * just sort alphabetically.
				 */
				break;
			}
			r = q;
		}
		if (r == NULL) {
			p->next = linkhead;
			linkhead = p;
		}
		else {
			p->next = r->next;
			r->next = p;
		}
	}
}


/*
 * dbprog_bld_linkopts
 *	Build a sorted linked list of track titles which are to be
 *	used to present to the user for database search-link.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC linkopts_t *
dbprog_bld_linkopts(curstat_t *s)
{
	int		i,
			ntrk;
	word32_t	offset,
			trkaddr;
	char		*dbdir,
			*bname,
			tmppath[FILE_PATH_SZ],
			buf[STR_BUF_SZ + 16];
	FILE		*fp;
	DIR		*dp;
	struct dirent	*de;
	bool_t		found;

	/* Warning: This code is SYSV-ish.  Porting to other
	 * environment may require some modification here.
	 */

	if (cur_db.dbfile == NULL)
		/* Error */
		return NULL;

	dbdir = dirname(cur_db.dbfile);
	bname = basename(cur_db.dbfile);

	if ((dp = opendir(dbdir)) == NULL)
		return NULL;

	while ((de = readdir(dp)) != NULL) {
		/* Find all entries in this directory with the same
		 * number of tracks as this disc.
		 */
		if (strncmp(de->d_name + 6, bname + 6, 2) != 0)
			continue;

		sprintf(tmppath, "%s/%s", dbdir, de->d_name);
		if ((fp = fopen(tmppath, "r")) == NULL)
			continue;
		
		/* Read first line of database file */
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			fclose(fp);
			continue;
		}

		/* Database file signature check */
		if (strncmp(buf, "# xmcd ", 7) != 0) {
			/* Not a supported database file */
			fclose(fp);
			continue;
		}

		ntrk = 0;
		found = FALSE;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			/* Look for track addresses of possible links */
			if (strncmp(buf, "# Track frame offsets", 21) == 0) {
				found = TRUE;
				continue;
			}
			else if (strncmp(buf, "# Disc length", 13) == 0) {
				i = sscanf(buf, "# Disc length: %u seconds\n",
					   &trkaddr);
				if (i > 0) {
					trk_off[ntrk] =
						(trkaddr * FRAME_PER_SEC) -
						s->trkinfo[ntrk].addr -
						MSF_OFFSET(s);
				}
				else {
					/* File format error */
					ntrk = 0;
				}
				found = FALSE;
				continue;
			}

			if (found &&
			    (i = sscanf(buf, "# %u\n", &trkaddr)) > 0) {
				trk_off[ntrk] =
					trkaddr - s->trkinfo[ntrk].addr -
					MSF_OFFSET(s);
				ntrk++;
			}

			/* Look for disk title */
			if (strncmp(buf, "DTITLE=", 7) == 0) {
				/* Eat newline */
				i = strlen(buf) - 1;
				if (buf[i] == '\n')
					buf[i] = '\0';
				
				/* Check whether a valid offset can be
				 * calculated (compare # of tracks).
				 */
				if (ntrk == (int) cur_db.ntrk) {
					/* Compute the average block
					 * number difference per track.
					 */
					offset = 0;
					for (i = 0; i <= ntrk; i++) {
						if (trk_off[i] < 0)
						    trk_off[i] = -trk_off[i];
						offset += trk_off[i];
					}
					offset /= ntrk;
				}
				else {
					/* Track offsets not specified or
					 * not valid in database file.
					 */
					offset = OFFSET_UNKN;
				}

				/* Add to list in sorted order */
				dbprog_add_linkent(buf + 7, de->d_name, offset);

				break;
			}
		}

		fclose(fp);
	}
	closedir(dp);

	return (linkhead);
}


/*
 * dbprog_free_linkopts
 *	Dismantle the sorted linked list of track titles created by
 *	dbprog_bld_linkopts.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
dbprog_free_linkopts(void)
{
	linkopts_t	*p,
			*q;

	for (p = q = linkhead; p != NULL; p = q) {
		q = p->next;
		if (p->dtitle != NULL)
			MEM_FREE(p->dtitle);
		MEM_FREE((char *) p);
	}
	linkhead = NULL;
}


/*
 * dbprog_dblink
 *	Pop up the search-link popup window, to let the user pick
 *	an existing CD database file entry to link the current disc to.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
STATIC void
dbprog_dblink(curstat_t *s)
{
	int		i;
	linkopts_t	*p;
	XmString	xs;
	byte_t		min,
			sec,
			frame;
	char		buf[STR_BUF_SZ * 4];

	if (cur_db.dbfile == NULL)
		/* Output file undefined */
		return;

	/* Change to the watch cursor */
	cd_busycurs(TRUE);

	/* Search directory for possible alternatives, and allow
	 * user to select a file to link to.
	 */

	XmListDeleteAllItems(widgets.linksel.link_list);

	linkhead = dbprog_bld_linkopts(s);

	for (i = 0, p = linkhead; p != NULL; i++, p = p->next) {
		if (p->offset == OFFSET_UNKN) {
			sprintf(buf, "??:??  %s", p->dtitle);
			xs = XmStringCreate(buf, CHSET1);
		}
		else {
			blktomsf(p->offset, &min, &sec, &frame, 0);
			sprintf(buf, "%02u:%02u  %s", min, sec, p->dtitle);
			if (p->offset < OFFSET_THRESH)
				xs = XmStringCreate(buf, CHSET2);
			else
				xs = XmStringCreate(buf, CHSET1);
		}

		XmListAddItemUnselected(widgets.linksel.link_list, xs, i + 1);
		XmStringFree(xs);
	}

	/* Change to the normal cursor */
	cd_busycurs(FALSE);

	if (i == 0) {
		cd_info_popup(app_data.str_info, app_data.str_nolink);
	}
	else if (!XtIsManaged(widgets.linksel.form)) {
		linksel_pos = 0;

		/* Pop up the link selector window */
		XtManageChild(widgets.linksel.form);
	}
}


/***********************
 *   public routines   *
 ***********************/


/*
 * dbprog_curtrkupd
 *	Update the track list display to show the current playing
 *	track entry in bold font.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
dbprog_curtrkupd(curstat_t *s)
{
	int			n,
				pos,
				secs;
	char			*str;
	XmString		xs;
	static int		sav_pos = -1;
	static sword32_t	sav_trkno;
	static byte_t		sav_tot_min,
				sav_tot_sec,
				sav_trk_min,
				sav_trk_sec;

	if (sav_pos >= 0) {
		n = strlen((cur_db.trklist[sav_pos] == NULL) ?
			   UNDEF_STR : cur_db.trklist[sav_pos]) + TRKLIST_PFLEN;

		if ((str = (char *) MEM_ALLOC(n)) == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
			return;
		}

		if (cur_db.trklist[sav_pos] != NULL) {
			sprintf(str, TRKLIST_FMT,
				sav_trkno,
				(timemode == TIME_TOTAL) ?
					sav_tot_min : sav_trk_min,
				(timemode == TIME_TOTAL) ?
					sav_tot_sec : sav_trk_sec,
				cur_db.trklist[sav_pos]);
		}
		else {
			sprintf(str, TRKLIST_FMT,
				sav_trkno,
				(timemode == TIME_TOTAL) ?
					sav_tot_min : sav_trk_min,
				(timemode == TIME_TOTAL) ?
					sav_tot_sec : sav_trk_sec,
				UNDEF_STR);
		}

		/* Restore previous playing track to original font */

		xs = XmStringCreate(str, CHSET1);

		XmListReplaceItemsPos(widgets.dbprog.trk_list, &xs,
				      1, sav_pos + 1);

		XmStringFree(xs);
		MEM_FREE(str);

		if (sel_pos == sav_pos + 1)
			/* This item is previously selected */
			XmListSelectPos(widgets.dbprog.trk_list,
					sel_pos, False);
	}

	if (s->cur_trk <= 0 || s->mode == M_NODISC) {
		sav_pos = -1;
		return;
	}

	sav_pos = pos = curtrk_pos(s);

	secs = ((s->trkinfo[pos+1].min * 60 +
		s->trkinfo[pos+1].sec) - 
	        (s->trkinfo[pos].min * 60 +
		s->trkinfo[pos].sec));

	sav_trkno = s->trkinfo[pos].trkno;

	sav_tot_min = s->trkinfo[pos].min;
	sav_tot_sec = s->trkinfo[pos].sec;
	sav_trk_min = (byte_t) (secs / 60);
	sav_trk_sec = (byte_t) (secs % 60);

	n = strlen((cur_db.trklist[pos] == NULL) ?
		   UNDEF_STR : cur_db.trklist[pos]) + TRKLIST_PFLEN;

	if ((str = (char *) MEM_ALLOC(n)) == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}

	if (cur_db.trklist[pos] != NULL) {
		sprintf(str, TRKLIST_FMT,
			sav_trkno,
			(timemode == TIME_TOTAL) ? sav_tot_min : sav_trk_min,
			(timemode == TIME_TOTAL) ? sav_tot_sec : sav_trk_sec,
			cur_db.trklist[pos]);
	}
	else {
		sprintf(str, TRKLIST_FMT,
			sav_trkno,
			(timemode == TIME_TOTAL) ? sav_tot_min : sav_trk_min,
			(timemode == TIME_TOTAL) ? sav_tot_sec : sav_trk_sec,
			UNDEF_STR);
	}

	/* Change current playing track to new font */

	xs = XmStringCreate(str, CHSET2);

	XmListReplaceItemsPos(widgets.dbprog.trk_list, &xs, 1, pos + 1);

	XmStringFree(xs);
	MEM_FREE(str);

	if (sel_pos == pos + 1)
		/* This item is previously selected */
		XmListSelectPos(widgets.dbprog.trk_list, sel_pos, False);
}


/*
 * dbprog_dbclear
 *	Clear in-core CD database entry.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
dbprog_dbclear(curstat_t *s)
{
	int		i;
	static bool_t	first_time = TRUE;

	/* Pop down the extd and extt popups if necessary */
	if (XtIsManaged(widgets.dbextd.form)) {
		dbprog_extd_cancel(
			widgets.dbextd.cancel_btn,
			(XtPointer) s,
			(XtPointer) NULL
		);
	}
	if (XtIsManaged(widgets.dbextt.form)) {
		dbprog_extt_cancel(
			widgets.dbextt.cancel_btn,
			(XtPointer) s,
			(XtPointer) NULL
		);
	}

	/* Clear database entry structure */

	cur_db.category[0] = '\0';

	if (cur_db.dbfile != NULL) {
		MEM_FREE(cur_db.dbfile);
		cur_db.dbfile = NULL;
	}

	if (cur_db.dtitle != NULL) {
		MEM_FREE(cur_db.dtitle);
		cur_db.dtitle = NULL;
	}

	if (cur_db.extd != NULL) {
		MEM_FREE(cur_db.extd);
		cur_db.extd = NULL;
	}

	for (i = MAXTRACK-1; i >= 0; i--) {
		if (cur_db.trklist[i] != NULL) {
			MEM_FREE(cur_db.trklist[i]);
			cur_db.trklist[i] = NULL;
		}

		if (cur_db.extt[i] != NULL) {
			MEM_FREE(cur_db.extt[i]);
			cur_db.extt[i] = NULL;
		}

		if (cur_db.sav_extt[i] != NULL) {
			MEM_FREE(cur_db.sav_extt[i]);
			cur_db.sav_extt[i] = NULL;
		}
	}

	if (cur_db.playorder != NULL) {
		MEM_FREE(cur_db.playorder);
		cur_db.playorder = NULL;
	}

	cur_db.discid = 0;

	if (first_time || cur_db.ntrk != 0) {
		first_time = FALSE;

		cur_db.ntrk = 0;

		/* Update disc ID indicator */
		dbprog_dpyid();

		/* Update database/program display */
		XmTextSetString(widgets.dbprog.dtitle_txt, "");
		XmListDeleteAllItems(widgets.dbprog.trk_list);
		XmTextSetString(widgets.dbprog.ttitle_txt, "");
		XmTextSetString(widgets.dbprog.pgmseq_txt, "");
		XmTextSetString(widgets.dbextd.disc_txt, "");
		XmTextSetString(widgets.dbextt.trk_txt, "");

		/* Make some buttons insensitive */
		XtSetSensitive(widgets.dbprog.addpgm_btn, False);
		XtSetSensitive(widgets.dbprog.clrpgm_btn, False);
		XtSetSensitive(widgets.dbprog.send_btn, False);
		XtSetSensitive(widgets.dbprog.savedb_btn, False);
		XtSetSensitive(widgets.dbprog.linkdb_btn, False);
		XtSetSensitive(widgets.dbprog.extd_btn, False);
		XtSetSensitive(widgets.dbprog.extt_btn, False);
	}

	/* Clear database flag */
	s->cddb = FALSE;
	dbprog_changed = FALSE;
}


/*
 * dbprog_dbget
 *	Read in the CD database file entry pertaining to the
 *	currently loaded disc, if available.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
dbprog_dbget(curstat_t *s)
{
	int		i,
			pos,
			lineno;
	word32_t	discid;
	FILE		*fp = NULL;
	char		dbfile[FILE_PATH_SZ],
			buf[STR_BUF_SZ + 16],
			tmpbuf[STR_BUF_SZ + 16];

	/* Get magic disc identifier */
	if ((discid = dbprog_discid(s)) == 0)
		/* Invalid identifier */
		return;

	if (cur_db.discid == discid)
		/* Database entry already loaded: just return. */
		return;

	cur_db.discid = discid;

	/* Change to watch cursor */
	cd_busycurs(TRUE);

	/* Loop through all the database directories
	 * and try to open the matching file for reading.
	 */
	for (fp = NULL, i = 0; i < app_data.max_dbdirs; i++) {
		if (dbdirs[i] == NULL)
			break;

		sprintf(dbfile, "%s/%08x", dbdirs[i], discid);

		if ((fp = fopen(dbfile, "r")) != NULL)
			break;
	}

	if (fp == NULL) {
		/* File does not exist or not readable */

		/* Update list widget */
		dbprog_structupd(s);

		/* Make the link button sensitive */
		XtSetSensitive(widgets.dbprog.linkdb_btn, True);

		/* Change to normal cursor */
		cd_busycurs(FALSE);

		/* Update disc ID indicator */
		dbprog_dpyid();

		return;
	}

	/* Record the category */
	strcpy(cur_db.category, basename(dbdirs[i]));

	/* Update disc ID indicator */
	dbprog_dpyid();

	/* Record the path to the database file. */
	cur_db.dbfile = (char *) MEM_ALLOC(strlen(dbfile) + 1);
	if (cur_db.dbfile == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}
	strcpy(cur_db.dbfile, dbfile);

	/* Read first line of database file */
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);

		/* Change to normal cursor */
		cd_busycurs(FALSE);
		return;
	}

	/* Database file signature check */
	if (strncmp(buf, "# xmcd ", 7) != 0) {
		/* Not a supported database file */
		fclose(fp);

		/* Change to normal cursor */
		cd_busycurs(FALSE);
		return;
	}

	/* Read the rest of the database file */
	for (lineno = 0; fgets(buf, sizeof(buf), fp) != NULL; lineno++) {
		/* Comment line */
		if (buf[0] == '#') {
			lineno--;
			continue;
		}

		buf[strlen(buf)-1] = '\n';

		/* Disk ID sanity check */
		if (lineno == 0) {
			if (strncmp(buf, "DISCID=", 7) == 0)
				/* Okay */
				continue;

			/* Sanity check failed */
			fclose(fp);

			/* Change to normal cursor */
			cd_busycurs(FALSE);
			return;
		}

		/* Disk title */
		if (sscanf(buf, "DTITLE=%[^\n]\n", tmpbuf) > 0) {
			if (cur_db.dtitle == NULL) {
				cur_db.dtitle = (char *)
					MEM_ALLOC(strlen(tmpbuf) + 1);

				if (cur_db.dtitle != NULL)
					cur_db.dtitle[0] = '\0';
			}
			else {
				cur_db.dtitle = (char *)
					MEM_REALLOC(cur_db.dtitle,
						strlen(cur_db.dtitle) +
						strlen(tmpbuf) + 1);
			}

			if (cur_db.dtitle == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				break;
			}

			dbprog_strcat(cur_db.dtitle, tmpbuf);
			continue;
		}

		/* Track title */
		if (sscanf(buf, "TTITLE%u=%[^\n]\n", &pos, tmpbuf) >= 2) {
			if (pos >= (int) s->tot_trks)
				continue;

			if (cur_db.trklist[pos] == NULL) {
				cur_db.trklist[pos] = (char *)
					MEM_ALLOC(strlen(tmpbuf) + 1);

				if (cur_db.trklist[pos] != NULL)
					cur_db.trklist[pos][0] = '\0';
				
			}
			else {
				cur_db.trklist[pos] = (char *)
					MEM_REALLOC(cur_db.trklist[pos],
						strlen(cur_db.trklist[pos]) +
						strlen(tmpbuf) + 1);
			}

			if (cur_db.trklist[pos] == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				break;
			}

			dbprog_strcat(cur_db.trklist[pos], tmpbuf);
			continue;
		}

		/* Disk extended info */
		if (sscanf(buf, "EXTD=%[^\n]\n", tmpbuf) > 0) {
			if (cur_db.extd == NULL) {
				cur_db.extd = (char *)
					MEM_ALLOC(strlen(tmpbuf) + 1);

				if (cur_db.extd != NULL)
					cur_db.extd[0] = '\0';
			}
			else {
				cur_db.extd = (char *)
					MEM_REALLOC(cur_db.extd,
						strlen(cur_db.extd) +
						strlen(tmpbuf) + 1);
			}

			if (cur_db.extd == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				break;
			}

			dbprog_strcat(cur_db.extd, tmpbuf);
			continue;
		}

		/* Track extended info */
		if (sscanf(buf, "EXTT%u=%[^\n]\n", &pos, tmpbuf) >= 2) {
			if (pos >= (int) s->tot_trks)
				continue;

			if (cur_db.extt[pos] == NULL) {
				cur_db.extt[pos] = (char *)
					MEM_ALLOC(strlen(tmpbuf) + 1);

				if (cur_db.extt[pos] != NULL)
					cur_db.extt[pos][0] = '\0';
			}
			else {
				cur_db.extt[pos] = (char *)
					MEM_REALLOC(cur_db.extt[pos],
						strlen(cur_db.extt[pos]) +
						strlen(tmpbuf) + 1);
			}

			if (cur_db.extt[pos] == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				break;
			}

			dbprog_strcat(cur_db.extt[pos], tmpbuf);
			continue;
		}

		/* Play order */
		if (sscanf(buf, "PLAYORDER=%[^\n]\n", tmpbuf) > 0) {
			if (cur_db.playorder == NULL) {
				cur_db.playorder = (char *)
					MEM_ALLOC(strlen(tmpbuf) + 1);

				if (cur_db.playorder != NULL)
					cur_db.playorder[0] = '\0';
					
			}
			else {
				cur_db.playorder = (char *)
					MEM_REALLOC(cur_db.playorder,
						strlen(cur_db.playorder) +
						strlen(tmpbuf) + 1);
			}

			if (cur_db.playorder == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				break;
			}

			dbprog_strcat(cur_db.playorder, tmpbuf);
			continue;
		}
	}

	fclose(fp);

	s->cddb = TRUE;
	XtSetSensitive(widgets.dbprog.send_btn, True);
	XtSetSensitive(widgets.dbprog.savedb_btn, False);
	XtSetSensitive(widgets.dbprog.linkdb_btn, False);

	/* Update list widget */
	dbprog_structupd(s);

	/* Disable shuffle mode */
	if (s->shuffle) {
		di_shuffle(s, FALSE);
		set_shuffle_btn(FALSE);
	}

	/* Parse play order string and set the play order */
	if (!dbprog_parse(s))
		cd_warning_popup(app_data.str_warning, app_data.str_seqfmterr);

	/* Update display */
	dpy_progmode(s);

	/* Change to normal cursor */
	cd_busycurs(FALSE);
}


/*
 * dbprog_init
 *	Initialize the CD database/program subsystem.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
void
dbprog_init(curstat_t *s)
{
	timemode = TIME_TRACK;

	/* Clear the in-core structure */
	dbprog_dbclear(s);
}


/*
 * dbprog_curdtitle
 *	Return the current disc title string.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Disc title text string, or the null string if there
 *	is no title available.
 */
char *
dbprog_curdtitle(curstat_t *s)
{
	if (s->mode == M_NODISC)
		return ("");

	return (cur_db.dtitle == NULL ? app_data.str_unkndisc : cur_db.dtitle);
}


/*
 * dbprog_curttitle
 *	Return the current track title string.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Track title text string, or the null string if there
 *	is no title available.
 */
char *
dbprog_curttitle(curstat_t *s)
{
	int	n = curtrk_pos(s);

	if (s->mode == M_NODISC || (int) s->cur_trk < 0)
		return ("");

	if (n < 0 || cur_db.trklist[n] == NULL)
		return (app_data.str_unkntrk);

	return (cur_db.trklist[n]);
}


/**************** vv Callback routines vv ****************/

/*
 * dbprog_popup
 *	Pop up the database/program subsystem window.
 */
void
dbprog_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
	if (!XtIsManaged(widgets.dbprog.form)) {
		/* Pop up the dbprog window */
		XtManageChild(widgets.dbprog.form);

		/* Pop up the extd/extt windows if necessary */
		if (extd_manage)
			dbprog_extd(w, client_data, call_data);

		if (extt_manage)
			dbprog_extt(w, (XtPointer) FALSE, call_data);
	}
}


/*
 * dbprog_dtitle_new
 *	Disc title editor text widget callback function.
 */
/*ARGSUSED*/
void
dbprog_dtitle_new(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;
	char			*str;
	XmString		xs;

	if (p->reason != XmCR_ACTIVATE && p->reason != XmCR_VALUE_CHANGED)
		return;

	if ((str = XmTextGetString(w)) == NULL)
		return;

	if (strcmp(str, UNDEF_STR) == 0) {
		if (cur_db.dtitle != NULL) {
			MEM_FREE(cur_db.dtitle);
			cur_db.dtitle = NULL;
		}
		XtFree(str);
		return;
	}

	dbprog_changed = TRUE;
	XtSetSensitive(widgets.dbprog.savedb_btn, True);

	if (cur_db.dtitle != NULL)
		MEM_FREE(cur_db.dtitle);

	if (str[0] == '\0') {
		XtFree(str);
		cur_db.dtitle = NULL;
	}
	else
		cur_db.dtitle = str;

	/* Update the extd window if necessary */
	if (XtIsManaged(widgets.dbextd.form)) {
		if (cur_db.dtitle == NULL)
			xs = XmStringCreateSimple("Untitled");
		else
			xs = XmStringCreateSimple(cur_db.dtitle);

		XtVaSetValues(widgets.dbextd.disc_lbl,
			XmNlabelString, xs,
			NULL
		);

		XmStringFree(xs);
	}

	/* Update main window title display */
	dpy_dtitle(s);
}


/*
 * dbprog_trklist_play
 *	Track list entry selection default action callback.
 */
void
dbprog_trklist_play(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct	*p = (XmListCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;

	if (p->reason != XmCR_DEFAULT_ACTION)
		return;

	sel_pos = p->item_position;

	dbprog_clrpgm(w, (XtPointer) s, (XtPointer) p);

	/* Set this again because dbprog_clrpgm() changes it */
	sel_pos = p->item_position;

	/* Add selected track to program */
	dbprog_addpgm(w, (XtPointer) s, (XtPointer) p);

	/* Stop current playback */
	if (s->mode != M_STOP)
		di_stop(s, FALSE);

	/* Play selected track */
	cd_play_pause(w, (XtPointer) s, (XtPointer) p);

	XmListDeselectPos(w, sel_pos);
	sel_pos = -1;
	XmTextSetString(widgets.dbprog.ttitle_txt, "");

	XtSetSensitive(widgets.dbprog.addpgm_btn, False);
	XtSetSensitive(widgets.dbprog.extt_btn, False);
}


/*
 * dbprog_trklist_select
 *	Track list entry selection callback.
 */
void
dbprog_trklist_select(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct	*p = (XmListCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;
	int			n,
				secs;
	char			*cp,
				*str;
	byte_t			min,
				sec;
	XmString		xs;

	if (p->reason != XmCR_BROWSE_SELECT)
		return;

	if (!di_check_disc(s) || s->mode == M_NODISC)
		return;

	if (title_edited) {
		title_edited = FALSE;

		if ((cp = XmTextGetString(widgets.dbprog.ttitle_txt)) == NULL)
			return;

		if (timemode == TIME_TOTAL) {
			min = s->trkinfo[p->item_position-1].min;
			sec = s->trkinfo[p->item_position-1].sec;
		}
		else {
			secs = ((s->trkinfo[p->item_position].min * 60 +
				s->trkinfo[p->item_position].sec) - 
				(s->trkinfo[p->item_position-1].min * 60 +
				s->trkinfo[p->item_position-1].sec));
			min = (byte_t) (secs / 60);
			sec = (byte_t) (secs % 60);
		}

		n = strlen(cp) + TRKLIST_PFLEN;
		if ((str = (char *) MEM_ALLOC(n)) == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
			return;
		}

		sprintf(str, TRKLIST_FMT,
			s->trkinfo[p->item_position-1].trkno,
			min, sec, cp);

		if (s->cur_trk >= 0 && curtrk_pos(s) == p->item_position-1)
			xs = XmStringCreate(str, CHSET2);
		else
			xs = XmStringCreate(str, CHSET1);

		XmListReplaceItemsPos(w, &xs, 1, p->item_position);

		XmStringFree(xs);
		MEM_FREE(str);

		XmListDeselectPos(widgets.dbprog.trk_list, sel_pos);
		sel_pos = -1;
		XmTextSetString(widgets.dbprog.ttitle_txt, "");

		if (cur_db.trklist[p->item_position-1] != NULL)
			MEM_FREE(cur_db.trklist[p->item_position-1]);

		cur_db.trklist[p->item_position-1] = cp;

		dbprog_changed = TRUE;
		XtSetSensitive(widgets.dbprog.savedb_btn, True);
		XtSetSensitive(widgets.dbprog.addpgm_btn, False);
		XtSetSensitive(widgets.dbprog.extt_btn, False);

		/* Update the extt window if necessary */
		if (extt_pos == p->item_position-1 &&
		    XtIsManaged(widgets.dbextt.form)) {
			if (cur_db.trklist[p->item_position-1] == NULL)
				xs = XmStringCreateSimple("Untitled");
			else
				xs = XmStringCreateSimple(
					cur_db.trklist[p->item_position-1]
				);

			XtVaSetValues(widgets.dbextt.trk_lbl,
				XmNlabelString, xs,
				NULL
			);

			XmStringFree(xs);
		}

		/* Update the main window if necessary */
		if (curtrk_pos(s) == p->item_position-1)
			dpy_ttitle(s);

		/* Return the input focus to the track title editor */
		XmProcessTraversal(
			widgets.dbprog.ttitle_txt,
			XmTRAVERSE_CURRENT
		);
	}
	else if (sel_pos == p->item_position) {
		/* This item is already selected: deselect it */

		XmListDeselectPos(w, p->item_position);
		sel_pos = ind_pos = -1;
		XmTextSetString(widgets.dbprog.ttitle_txt, "");

		XtSetSensitive(widgets.dbprog.addpgm_btn, False);
		XtSetSensitive(widgets.dbprog.extt_btn, False);
	}
	else {
		sel_pos = p->item_position;

		if (sel_pos > 0)
			XmListSelectPos(w, sel_pos, False);

		if (cur_db.trklist[p->item_position-1] == NULL) {
			XmTextSetString(widgets.dbprog.ttitle_txt, UNDEF_STR);
			XmTextSetInsertionPosition(
				widgets.dbprog.ttitle_txt,
				strlen(UNDEF_STR)
			);
		}
		else {
			XmTextSetString(widgets.dbprog.ttitle_txt,
					cur_db.trklist[p->item_position-1]);
			XmTextSetInsertionPosition(
				widgets.dbprog.ttitle_txt,
				strlen(cur_db.trklist[p->item_position-1])
			);
		}

		XtSetSensitive(widgets.dbprog.addpgm_btn, True);
		XtSetSensitive(widgets.dbprog.extt_btn, True);

		/* Warp the extt window to the new selected track, if
		 * it is popped up.
		 */
		if (XtIsManaged(widgets.dbextt.form))
			dbprog_extt(w, (XtPointer) FALSE, call_data);
	}
}


/*
 * dbprog_ttitle_focuschg
 *	Track title editor text widget keyboard focus change callback.
 */
/*ARGSUSED*/
void
dbprog_ttitle_focuschg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;
	int			i;

	if (p->reason == XmCR_FOCUS) {
		if (sel_pos < 0) {
			for (i = 0; i < (int) s->tot_trks; i++) {
				if (cur_db.trklist[i] == NULL) {
					ind_pos = i + 1;

					XmListSelectPos(
						widgets.dbprog.trk_list,
						ind_pos,
						False
					);
					XmListSetBottomPos(
						widgets.dbprog.trk_list,
						ind_pos
					);
					break;
				}
			}
		}
	}
	else if (p->reason == XmCR_LOSING_FOCUS) {
		if (ind_pos >= 0) {
			XmListDeselectPos(widgets.dbprog.trk_list, ind_pos);
			ind_pos = -1;
		}
	}
}


/*
 * dbprog_ttitle_new
 *	Track title editor text widget callback function.
 */
void
dbprog_ttitle_new(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;
	char			*cp,
				*str;
	int			*pos,
				secs,
				i,
				n;
	byte_t			min,
				sec;
	XmString		xs;
	XmListCallbackStruct	cb;

	if (p->reason == XmCR_VALUE_CHANGED) {
		if ((cp = XmTextGetString(w)) == NULL)
			return;

		if (cp[0] == '\0')
			title_edited = FALSE;
		else if (sel_pos < 0)
			title_edited = TRUE;

		XtFree(cp);
		return;
	}
	else if (p->reason != XmCR_ACTIVATE)
		return;

	if (sel_pos >= 0 &&
	    XmListGetSelectedPos(widgets.dbprog.trk_list, &pos, &i)) {
		if ((cp = XmTextGetString(w)) == NULL)
			return;

		if (pos == NULL) {
			XtFree(cp);
			return;
		}

		if (i == 1) {
			if (timemode == TIME_TOTAL) {
				min = s->trkinfo[(*pos)-1].min;
				sec = s->trkinfo[(*pos)-1].sec;
			}
			else {
				secs = ((s->trkinfo[*pos].min * 60 +
					s->trkinfo[*pos].sec) - 
					(s->trkinfo[(*pos)-1].min * 60 +
					s->trkinfo[(*pos)-1].sec));
				min = (byte_t) (secs / 60);
				sec = (byte_t) (secs % 60);
			}

			n = strlen(cp) + TRKLIST_PFLEN;
			if ((str = (char *) MEM_ALLOC(n)) == NULL) {
				cd_fatal_popup(
					app_data.str_fatal,
					app_data.str_nomemory
				);
				return;
			}

			sprintf(str, TRKLIST_FMT,
				s->trkinfo[(*pos)-1].trkno,
				min, sec, cp);

			if (s->mode != M_NODISC && s->cur_trk >= 0 &&
			    curtrk_pos(s) == ((*pos)-1))
				xs = XmStringCreate(str, CHSET2);
			else
				xs = XmStringCreate(str, CHSET1);

			XmListReplaceItemsPos(widgets.dbprog.trk_list,
					      &xs, 1, *pos);
			XmStringFree(xs);
			MEM_FREE(str);

			XmListDeselectPos(widgets.dbprog.trk_list, sel_pos);
			sel_pos = -1;

			if (cur_db.trklist[(*pos)-1] != NULL)
				MEM_FREE(cur_db.trklist[(*pos)-1]);

			cur_db.trklist[(*pos)-1] = cp;

			dbprog_changed = TRUE;
			XtSetSensitive(widgets.dbprog.savedb_btn, True);
			XtSetSensitive(widgets.dbprog.addpgm_btn, False);
			XtSetSensitive(widgets.dbprog.extt_btn, False);

			/* Update the extt window if necessary */
			if (extt_pos == (*pos)-1 &&
			    XtIsManaged(widgets.dbextt.form)) {
				if (cur_db.trklist[(*pos)-1] == NULL)
					xs = XmStringCreateSimple("Untitled");
				else
					xs = XmStringCreateSimple(
						cur_db.trklist[(*pos)-1]
					);

				XtVaSetValues(widgets.dbextt.trk_lbl,
					XmNlabelString, xs,
					NULL
				);

				XmStringFree(xs);
			}

			/* Update the main window if necessary */
			if (curtrk_pos(s) == (*pos)-1)
				dpy_ttitle(s);

		}

		XmTextSetString(w, "");

		XtFree((XtPointer) pos);
	}
	else {
		/* Pressing Return in this case is equivalent to clicking
		 * on the first title-less track on the track list.
		 */
		for (i = 0; i < (int) s->tot_trks; i++) {
			if (cur_db.trklist[i] == NULL) {
				cb.item_position = i + 1;
				cb.reason = XmCR_BROWSE_SELECT;
				cb.event = p->event;

				dbprog_trklist_select(
					widgets.dbprog.trk_list,
					(XtPointer) s,
					(XtPointer) &cb
				);
				break;
			}
		}
	}

	for (i = 0; i < (int) s->tot_trks; i++) {
		if (cur_db.trklist[i] == NULL) {
			ind_pos = i + 1;

			XmListSelectPos(
				widgets.dbprog.trk_list,
				ind_pos,
				False
			);
			XmListSetBottomPos(
				widgets.dbprog.trk_list,
				ind_pos
			);
			break;
		}
	}
}


/*
 * dbprog_pgmseq_verify
 *	Play sequence editor text widget user-input verification callback.
 */
/*ARGSUSED*/
void
dbprog_pgmseq_verify(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmTextVerifyCallbackStruct
		*p = (XmTextVerifyCallbackStruct *)(void *) call_data;
	int	i;

	if (p->reason != XmCR_MODIFYING_TEXT_VALUE)
		return;

	p->doit = True;

	/* If not changing via direct edit in the text widget,
	 * then there is nothing to do here.
	 */
	if (!pgmseq_editing)
		return;

	if (p->startPos != p->endPos)
		return;

	switch (p->text->format) {
	case FMT8BIT:
		if (p->text->length != 1) {
			p->doit = False;
			return;
		}

		if (p->text->ptr[0] == PGM_SEPCHAR) {
			if (cur_db.playorder == NULL ||
			    (i = strlen(cur_db.playorder)) == 0 ||
			    cur_db.playorder[i-1] == PGM_SEPCHAR ||
			    p->currInsert != p->newInsert)
				p->doit = False;
		}
		else if (!isdigit(p->text->ptr[0]))
			p->doit = False;
		break;

	case FMT16BIT:
		/* Don't know how to handle 16-bit character sets yet */
		p->doit = False;
		return;
	}
}


/*
 * dbprog_pgmseq_txtchg
 *	Play sequence editor text widget text changed callback.
 */
/*ARGSUSED*/
void
dbprog_pgmseq_txtchg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;
	curstat_t		*s = (curstat_t *)(void *) client_data;

	if (p->reason != XmCR_VALUE_CHANGED)
		return;

	/* If not changing via direct edit in the text widget,
	 * then there is nothing to do here.
	 */
	if (!pgmseq_editing)
		return;

	if (cur_db.playorder != NULL)
		MEM_FREE(cur_db.playorder);

	cur_db.playorder = XmTextGetString(w);

	/* Disable shuffle mode */
	if (s->shuffle) {
		di_shuffle(s, FALSE);
		set_shuffle_btn(FALSE);
	}

	/* Parse play order string and set the play order */
	if (!dbprog_parse(s))
		cd_warning_popup(app_data.str_warning, app_data.str_seqfmterr);

	/* Update display */
	dpy_progmode(s);

	dbprog_changed = TRUE;
	XtSetSensitive(widgets.dbprog.savedb_btn, True);
	XtSetSensitive(widgets.dbprog.clrpgm_btn, True);
}


/*
 * dbprog_pgmseq_focuschg
 *	Play sequence editor text widget keyboard focus change callback.
 */
/*ARGSUSED*/
void
dbprog_pgmseq_focuschg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;

	if (p->reason == XmCR_FOCUS)
		pgmseq_editing = TRUE;
	else if (p->reason == XmCR_LOSING_FOCUS)
		pgmseq_editing = FALSE;
}


/*
 * dbprog_addpgm
 *	Program Add button callback.
 */
/*ARGSUSED*/
void
dbprog_addpgm(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	char		*cp,
			tmpbuf[6];

	if (sel_pos < 0 || !di_check_disc(s) || s->mode == M_NODISC) {
		cd_beep();
		return;
	}

	if (cur_db.playorder == NULL) {
		sprintf(tmpbuf, "%u", s->trkinfo[sel_pos-1].trkno);
		cp = (char *) MEM_ALLOC(strlen(tmpbuf) + 1);
	}
	else {
		sprintf(tmpbuf, "%c%u",
			PGM_SEPCHAR, s->trkinfo[sel_pos-1].trkno);
		cp = (char *) MEM_ALLOC(
			strlen(cur_db.playorder) + strlen(tmpbuf) + 1
		);
	}

	if (cp == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}

	sprintf(cp, "%s%s",
		(cur_db.playorder == NULL) ? "" : cur_db.playorder,
		tmpbuf);

	MEM_FREE(cur_db.playorder);
	cur_db.playorder = cp;

	XmTextSetString(widgets.dbprog.pgmseq_txt, cur_db.playorder);
	XmTextSetInsertionPosition(
		widgets.dbprog.pgmseq_txt,
		strlen(cur_db.playorder)
	);

	/* Disable shuffle mode */
	if (s->shuffle) {
		di_shuffle(s, FALSE);
		set_shuffle_btn(FALSE);
	}

	/* Parse play order string and set the play order */
	if (!dbprog_parse(s))
		cd_warning_popup(app_data.str_warning, app_data.str_seqfmterr);

	/* Update display */
	dpy_progmode(s);

	dbprog_changed = TRUE;
	XmListDeselectPos(widgets.dbprog.trk_list, sel_pos);
	sel_pos = -1;
	XmTextSetString(widgets.dbprog.ttitle_txt, "");
	XtSetSensitive(widgets.dbprog.savedb_btn, True);
	XtSetSensitive(widgets.dbprog.clrpgm_btn, True);
	XtSetSensitive(widgets.dbprog.addpgm_btn, False);
	XtSetSensitive(widgets.dbprog.extt_btn, False);
}


/*
 * dbprog_clrpgm
 *	Program Clear button callback.
 */
/*ARGSUSED*/
void
dbprog_clrpgm(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (cur_db.playorder != NULL) {
		MEM_FREE(cur_db.playorder);
		cur_db.playorder = NULL;

		XmTextSetString(widgets.dbprog.pgmseq_txt, "");
	}

	XmListDeselectPos(widgets.dbprog.trk_list, sel_pos);
	sel_pos = -1;
	XmTextSetString(widgets.dbprog.ttitle_txt, "");

	dbprog_changed = TRUE;
	XtSetSensitive(widgets.dbprog.savedb_btn, True);
	XtSetSensitive(widgets.dbprog.addpgm_btn, False);
	XtSetSensitive(widgets.dbprog.clrpgm_btn, False);
	XtSetSensitive(widgets.dbprog.extt_btn, False);

	s->prog_tot = 0;

	/* Update display */
	dpy_progmode((curstat_t *)(void *) client_data);
}


/*
 * dbprog_send
 *	CDDB Send pushbutton callback.
 */
/*ARGSUSED*/
void
dbprog_send(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	char		msg[128];

	if (s->mode == M_NODISC || cur_db.discid == 0 ||
	    cur_db.dbfile == NULL) {
		/* Nothing to send */
		cd_beep();
		return;
	}

	if (app_data.cddb_mailsite == NULL ||
	    app_data.cddb_mailsite[0] == '\0' ||
	    app_data.cddb_mailcmd == NULL ||
	    app_data.cddb_mailcmd[0] == '\0') {
		/* No mail site or mail command specified */
		cd_beep();
		return;
	}

	sprintf(msg, app_data.str_send, app_data.cddb_mailsite);

	cd_confirm_popup(
		app_data.str_confirm,
		msg,
		(XtCallbackProc) dbprog_dbsend,
		client_data,
		(XtCallbackProc) NULL,
		NULL
	);
}


/*
 * dbprog_savedb
 *	In-core CD database SAVE button callback.
 */
/*ARGSUSED*/
void
dbprog_savedb(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	static bool_t	first = TRUE;

	if (cur_db.discid == 0) {
		cd_beep();
		return;
	}

	if (dbdirs[0] == NULL) {
		/* No database directory */
		cd_info_popup(app_data.str_info, app_data.str_nodb);
		return;
	}

	dirsel_mode = DIRSEL_SAVE;

	if (cur_db.dbfile == NULL) {
		/* Pop up the database directory selection popup */
		XmListDeselectAllItems(widgets.dirsel.dir_list);
		XmTextSetString(widgets.dbprog.ttitle_txt, "");

		XtManageChild(widgets.dirsel.form);

		if (first) {
			first = FALSE;
			XmProcessTraversal(
				widgets.dirsel.ok_btn,
				XmTRAVERSE_CURRENT
			);
		}
	}
	else
		/* Save to file directly */
		dbprog_dbput(s);
}


/*
 * dbprog_loaddb
 *	CD database file LOAD button callback.
 */
/*ARGSUSED*/
void
dbprog_loaddb(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (!di_check_disc(s) || s->mode == M_NODISC) {
		cd_beep();
		return;
	}

	/* Clear the in-core entry */
	dbprog_dbclear(s);

	/* Re-load from database file */
	dbprog_dbget(s);
}


/*
 * dbprog_link
 *	CD Database file search-link button callback.
 */
/*ARGSUSED*/
void
dbprog_link(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	static bool_t	first = TRUE;

	if (cur_db.discid == 0) {
		cd_beep();
		return;
	}

	dirsel_mode = DIRSEL_LINK;

	if (cur_db.dbfile != NULL) {
		MEM_FREE(cur_db.dbfile);
		cur_db.dbfile = NULL;
	}

	if (dbdirs[0] == NULL) {
		/* No database directory */
		cd_info_popup(app_data.str_info, app_data.str_nodb);
		return;
	}

	if (dbdirs[1] == NULL) {
		/* Only one possible database directory: Set
		 * database file path.
		 */
		cur_db.dbfile = (char *) MEM_ALLOC(strlen(dbdirs[0]) + 10);
		if (cur_db.dbfile == NULL) {
			cd_fatal_popup(
				app_data.str_fatal,
				app_data.str_nomemory
			);
			return;
		}
		sprintf(cur_db.dbfile, "%s/%08x", dbdirs[0], cur_db.discid);

		dbprog_dblink(s);
		return;
	}

	/* Pop up the database directory selection popup */
	XmListDeselectAllItems(widgets.dirsel.dir_list);
	XmTextSetString(widgets.dbprog.ttitle_txt, "");

	if (first) {
		first = FALSE;
		XmProcessTraversal(
			widgets.dirsel.ok_btn,
			XmTRAVERSE_CURRENT
		);
	}

	XtManageChild(widgets.dirsel.form);
}


/*
 * dbprog_cancel
 *	Pop down CD database/program window.
 */
/*ARGSUSED*/
void
dbprog_cancel(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmPushButtonCallbackStruct
			*p = (XmPushButtonCallbackStruct *)(void *) call_data;
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (XtIsManaged(widgets.dbextd.form)) {
		/* Force a popdown of the extd window */
		dbprog_extd_cancel(
			widgets.dbextd.cancel_btn,
			(XtPointer) s,
			(XtPointer) p
		);

		extd_manage = TRUE;
	}
	if (XtIsManaged(widgets.dbextt.form)) {
		/* Force a popdown of the extt window */
		dbprog_extt_cancel(
			widgets.dbextt.cancel_btn,
			(XtPointer) s,
			(XtPointer) p
		);

		extt_manage = TRUE;
	}

	/* Pop down the database/program dialog */
	XtUnmanageChild(widgets.dbprog.form);
}


/*
 * dbprog_timedpy
 *	Toggle the time display mode in the track list.
 */
/*ARGSUSED*/
void
dbprog_timedpy(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmRowColumnCallbackStruct	*p =
		(XmRowColumnCallbackStruct *)(void *) call_data;
	XmToggleButtonCallbackStruct	*q;
	curstat_t			*s = (curstat_t *)(void *) client_data;

	if (p != NULL)
		q = (XmToggleButtonCallbackStruct *)(void *) p->callbackstruct;

	if (!q->set)
		return;

	if (p->widget == widgets.dbprog.tottime_btn) {
		if (timemode == TIME_TOTAL)
			return;	/* No change */

		timemode = TIME_TOTAL;
	}
	else if (p->widget == widgets.dbprog.trktime_btn) {
		if (timemode == TIME_TRACK)
			return;	/* No change */

		timemode = TIME_TRACK;
	}
	else
		return;	/* Invalid widget */

	if (di_check_disc(s) && s->mode != M_NODISC) {
		XmListDeleteAllItems(widgets.dbprog.trk_list);
		dbprog_listupd(s);
	}
}


/*
 * dbprog_extd
 *	Pop up/down the disc extended info window.
 */
void
dbprog_extd(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmString	xs;
	static bool_t	first = TRUE;

	if (XtIsManaged(widgets.dbextd.form)) {
		/* Pop down the Disc Extended Info window */
		dbprog_extd_ok(w, client_data, call_data);
		return;
	}

	if (cur_db.dtitle == NULL)
		xs = XmStringCreateSimple("Untitled");
	else
		xs = XmStringCreateSimple(cur_db.dtitle);

	XtVaSetValues(widgets.dbextd.disc_lbl,
		XmNlabelString, xs,
		NULL
	);

	XmStringFree(xs);

	/* Pop up the Disc Extended Info window */
	XtManageChild(widgets.dbextd.form);

	if (first) {
		first = FALSE;
		XmProcessTraversal(
			widgets.dbextd.ok_btn,
			XmTRAVERSE_CURRENT
		);
	}

	extd_manage = FALSE;
}


/*
 * dbprog_extt
 *	Pop up/down the track extended info window.
 */
void
dbprog_extt(Widget w, XtPointer client_data, XtPointer call_data)
{
	int		i,
			n,
			*pos;
	XmString	xs;
	bool_t		from_main = (bool_t)(int) client_data;
	static bool_t	first = TRUE;

	if (XtIsManaged(widgets.dbextt.form)) {
		if (from_main) {
			/* Pop down the Track Extended Info window */
			dbprog_extt_ok(w, client_data, call_data);

			return;
		}
		else {
			/* Update structures */
			dbprog_exttupd();
		}
	}

	if (sel_pos >= 0 &&
	    XmListGetSelectedPos(widgets.dbprog.trk_list, &pos, &n)) {

		/* Enter extt setup mode */
		extt_setup = TRUE;

		if (n != 1) {
			/* This shouldn't happen error */
			cd_beep();
			return;
		}

		if (cur_db.trklist[(*pos)-1] == NULL)
			xs = XmStringCreateSimple("Untitled");
		else
			xs = XmStringCreateSimple(cur_db.trklist[(*pos)-1]);

		XtVaSetValues(widgets.dbextt.trk_lbl,
			XmNlabelString, xs,
			NULL
		);

		XmStringFree(xs);

		/* Track extended info text */
		if (cur_db.extt[(*pos)-1] != NULL)
			XmTextSetString(widgets.dbextt.trk_txt,
				        cur_db.extt[(*pos)-1]);
		else
			XmTextSetString(widgets.dbextt.trk_txt, "");

		extt_pos = (*pos)-1;

		if (from_main) {
			/* Save a backup copy of the text in case the user
			 * wants to abort.  This code will ensure that the
			 * data is not saved twice while the extended track
			 * info window is popped up.
			 */
			for (i = 0; i < MAXTRACK; i++) {
				if (cur_db.sav_extt[i] == NULL &&
				    cur_db.extt[i] != NULL) {
					cur_db.sav_extt[i] = (char *)
						MEM_ALLOC(
						    strlen(cur_db.extt[i]) + 1
						);
					if (cur_db.sav_extt[i] == NULL) {
						cd_fatal_popup(
							app_data.str_fatal,
							app_data.str_nomemory
						);
						return;
					}
					strcpy(
						cur_db.sav_extt[i],
						cur_db.extt[i]
					);
				}
			}
		}

		/* Pop up the Track Extended Info popup */
		XtManageChild(widgets.dbextt.form);

		if (first) {
			first = FALSE;
			XmProcessTraversal(
				widgets.dbextt.ok_btn,
				XmTRAVERSE_CURRENT
			);
		}

		extt_manage = FALSE;

		/* Exit extt setup mode */
		extt_setup = FALSE;
	}
}


/*
 * dbprog_set_changed
 *	Set the flag indicating that the user has made changes to the
 *	in-core CD database entry.
 */
/*ARGSUSED*/
void
dbprog_set_changed(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct	*p = (XmAnyCallbackStruct *)(void *) call_data;

	if (p->reason != XmCR_VALUE_CHANGED)
		return;

	/* Setup of the extt window is not a user change */
	if (!extt_setup) {
		dbprog_changed = TRUE;
		XtSetSensitive(widgets.dbprog.savedb_btn, True);
	}
}


/*
 * dbprog_extd_ok
 *	Extended disc info window OK button callback.
 */
/*ARGSUSED*/
void
dbprog_extd_ok(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down the Disc Extended Info popup */
	XtUnmanageChild(widgets.dbextd.form);

	/* Update structures */
	dbprog_extdupd();
}


/*
 * dbprog_extd_clear
 *	Extended disc info window Clear button callback.
 */
/*ARGSUSED*/
void
dbprog_extd_clear(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmTextSetString(widgets.dbextd.disc_txt, "");
}


/*
 * dbprog_extd_cancel
 *	Extended disc info window Cancel button callback.
 */
/*ARGSUSED*/
void
dbprog_extd_cancel(Widget w, XtPointer client_data, XtPointer call_data)
{
	bool_t	sav_chg;

	/* Pop down the Disc Extended Info popup */
	XtUnmanageChild(widgets.dbextd.form);

	sav_chg = dbprog_changed;

	/* Restore original text */
	if (cur_db.extd == NULL)
		XmTextSetString(widgets.dbextd.disc_txt, "");
	else
		XmTextSetString(widgets.dbextd.disc_txt, cur_db.extd);

	dbprog_changed = sav_chg;

	if (!dbprog_changed)
		XtSetSensitive(widgets.dbprog.savedb_btn, False);
}


/*
 * dbprog_extt_ok
 *	Extended track info window OK button callback.
 */
/*ARGSUSED*/
void
dbprog_extt_ok(Widget w, XtPointer client_data, XtPointer call_data)
{
	int	i;

	/* Pop down the Track Extended Info popup */
	XtUnmanageChild(widgets.dbextt.form);

	/* Update structures */
	dbprog_exttupd();

	/* Delete backup text */
	for (i = 0; i < MAXTRACK; i++) {
		if (cur_db.sav_extt[i] != NULL) {
			MEM_FREE(cur_db.sav_extt[i]);
			cur_db.sav_extt[i] = NULL;
		}
	}
}


/*
 * dbprog_extt_clear
 *	Extended track info window Clear button callback.
 */
/*ARGSUSED*/
void
dbprog_extt_clear(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmTextSetString(widgets.dbextt.trk_txt, "");
}


/*
 * dbprog_extt_cancel
 *	Extended track info window Cancel button callback.
 */
/*ARGSUSED*/
void
dbprog_extt_cancel(Widget w, XtPointer client_data, XtPointer call_data)
{
	int	i;

	/* Pop down the Track Extended Info popup */
	XtUnmanageChild(widgets.dbextt.form);

	/* Restore backup text */
	for (i = 0; i < MAXTRACK; i++) {
		if (cur_db.extt[i] != NULL)
			MEM_FREE(cur_db.extt[i]);

		cur_db.extt[i] = cur_db.sav_extt[i];
		cur_db.sav_extt[i] = NULL;
	}
}


/*
 * dbprog_dirsel_select
 *	CD Database directory selection list callback.
 */
/*ARGSUSED*/
void
dbprog_dirsel_select(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct	*p = (XmListCallbackStruct *)(void *) call_data;

	if (p->reason != XmCR_BROWSE_SELECT)
		return;

	if (cur_db.dbfile != NULL)
		MEM_FREE(cur_db.dbfile);

	cur_db.dbfile = (char *)
		MEM_ALLOC(strlen(dbdirs[p->item_position-1]) + 10);

	if (cur_db.dbfile == NULL) {
		cd_fatal_popup(app_data.str_fatal, app_data.str_nomemory);
		return;
	}

	sprintf(cur_db.dbfile, "%s/%08x",
		dbdirs[p->item_position-1], cur_db.discid);
	strcpy(cur_db.category, basename(dbdirs[p->item_position-1]));
}


/*
 * dbprog_dirsel_ok
 *	CD Database directory selection window OK button callback.
 */
/*ARGSUSED*/
void
dbprog_dirsel_ok(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;

	if (cur_db.dbfile == NULL) {
		/* User has not selected a directory yet */
		cd_beep();
		return;
	}

	/* Pop down the database directory selector popup dialog */
	XtUnmanageChild(widgets.dirsel.form);

	switch (dirsel_mode) {
	case DIRSEL_SAVE:
		/* Save the database entry to output file */
		dbprog_dbput(s);
		break;

	case DIRSEL_LINK:
		/* Link the database entry to another file */
		dbprog_dblink(s);
		break;

	default:
		/* Shouldn't get here */
		break;
	}
}


/*
 * dbprog_dirsel_cancel
 *	CD Database directory selection window Cancel button callback.
 */
/*ARGSUSED*/
void
dbprog_dirsel_cancel(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down the database directory selector popup dialog */
	XtUnmanageChild(widgets.dirsel.form);

	/* Clear database file path */
	if (cur_db.dbfile != NULL) {
		MEM_FREE(cur_db.dbfile);
		cur_db.dbfile = NULL;
	}
}


/*
 * dbprog_linksel_select
 *	Search-link selector list user-selection callback.
 */
/*ARGSUSED*/
void
dbprog_linksel_select(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmListCallbackStruct	*p = (XmListCallbackStruct *)(void *) call_data;

	if (p->reason != XmCR_BROWSE_SELECT)
		return;

	linksel_pos = p->item_position;
}


/*
 * dbprog_linksel_ok
 *	Search-link selector window OK button callback.
 */
void
dbprog_linksel_ok(Widget w, XtPointer client_data, XtPointer call_data)
{
	curstat_t	*s = (curstat_t *)(void *) client_data;
	char		errstr[ERR_BUF_SZ],
			ltarget[FILE_PATH_SZ];
	int		i;
	waitret_t	stat_val;
	linkopts_t	*q;
	pid_t		cpid;

	if (linksel_pos <= 0) {
		/* User has not selected a link target yet */
		cd_beep();
		return;
	}

	/* Pop down the link selector popup dialog */
	XtUnmanageChild(widgets.linksel.form);

	/* Change to watch cursor */
	cd_busycurs(TRUE);

	/* Do the link */
	switch (cpid = fork()) {
	case 0:
		/* Child process */
		break;
	case -1:
		sprintf(errstr, app_data.str_saverr_fork, errno);
		cd_warning_popup(app_data.str_warning, errstr);
		return;
	default:
		/* Parent process: wait for child to exit */
		while (waitpid(cpid, &stat_val, 0) != cpid)
			;

		/* Change to normal cursor */
		cd_busycurs(FALSE);

		/* Free link options list */
		dbprog_free_linkopts();

		if (WIFEXITED(stat_val)) {
			switch (WEXITSTATUS(stat_val)) {
			case SETUID_ERR:
				sprintf(errstr, app_data.str_lnkerr_suid,
					get_ouid(), get_ogid());
				cd_warning_popup(app_data.str_warning, errstr);
				return;

			case LINK_ERR:
				sprintf(errstr, app_data.str_lnkerr_link);
				cd_warning_popup(app_data.str_warning, errstr);
				return;

			default:
				break;
			}
		}
		else if (WIFSIGNALED(stat_val)) {
			sprintf(errstr, app_data.str_saverr_killed,
				WTERMSIG(stat_val));
			cd_warning_popup(app_data.str_warning, errstr);
			return;
		}

		/* Load new database entry */
		dbprog_loaddb(w, client_data, call_data);

		/* Database mode is now on */
		s->cddb = TRUE;

		/* All edits have been saved, so clear flag */
		dbprog_changed = FALSE;

		/* Update display */
		dpy_dbmode(s);
		dbprog_dpyid();

		XtSetSensitive(widgets.dbprog.send_btn, True);
		XtSetSensitive(widgets.dbprog.linkdb_btn, False);
		XtSetSensitive(widgets.dbprog.savedb_btn, False);
		XmProcessTraversal(
			widgets.dbprog.cancel_btn,
			XmTRAVERSE_CURRENT
		);
		return;
	}

	DBGPRN(errfp, "\nSetting uid to %d, gid to %d\n",
		get_ouid(), get_ogid());

	/* Force uid and gid to original setting */
	if (setuid(get_ouid()) < 0 || setgid(get_ogid()) < 0)
		exit(SETUID_ERR);

	for (i = 0, q = linkhead; q != NULL; i++, q = q->next)
		if (i == linksel_pos - 1)
			break;

	if (q == NULL)
		/* This should not happen */
		exit(LINK_ERR);

	sprintf(ltarget, "%s/%s", dirname(cur_db.dbfile), q->idstr);

#ifdef USE_SYMLINK
	DBGPRN(errfp, "\nSymbolic Linking CD database file %s -> %s\n",
	       ltarget, cur_db.dbfile);

	if (symlink(ltarget, cur_db.dbfile) < 0)
#else
	DBGPRN(errfp, "\nHard Linking CD database file %s -> %s\n",
	       ltarget, cur_db.dbfile);

	if (link(ltarget, cur_db.dbfile) < 0)
#endif
		exit(LINK_ERR);

	/* Child exits here. */
	exit(0);
}


/*
 * dbprog_linksel_cancel
 *	Search-link selector window Cancel button callback.
 */
/*ARGSUSED*/
void
dbprog_linksel_cancel(Widget w, XtPointer client_data, XtPointer call_data)
{
	/* Pop down the link selector popup dialog */
	XtUnmanageChild(widgets.linksel.form);

	/* Free link options list */
	dbprog_free_linkopts();

	/* Clear database file path */
	if (cur_db.dbfile != NULL) {
		MEM_FREE(cur_db.dbfile);
		cur_db.dbfile = NULL;
	}
}

/**************** ^^ Callback routines ^^ ****************/

