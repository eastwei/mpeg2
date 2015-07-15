/*
 *   cda - Command-line CD Audio Player
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

/*
 *   Visual mode support
 *
 *   Contributing author: Philip Le Riche
 *   E-Mail: pleriche@uk03.bull.co.uk
 */

#ifndef LINT
static char *_visual_c_ident_ = "@(#)visual.c	5.14 95/02/06";
#endif

#ifndef NOVISUAL

#define _CDA_

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "common.d/patchlevel.h"
#include "libdi.d/libdi.h"

/* TRUE and FALSE is redefined in curses.h. */
#undef TRUE
#undef FALSE

/* curses.h redefines SYSV - handle with care */
#ifdef SYSV
#undef SYSV
#include <curses.h>
#define SYSV
#else
#if defined(ultrix) || defined(__ultrix)
#include <cursesX.h>
#else
#ifdef __FreeBSD__
#include <ncurses.h>
#else
#include <curses.h>
#endif	/* __FreeBSD__ */
#endif	/* ultrix */
#endif	/* SYSV */

#include <term.h>

#include "cda.d/cda.h"
#include "cda.d/visual.h"

extern appdata_t	app_data;
extern database_t	cur_db;
extern curstat_t	status;
extern char		*errmsg,
			emsg[],
			spipe[],
			rpipe[];
extern int		cda_sfd[],
			cda_rfd[];

STATIC int		scroll_line = 0,	/* 1st line of info window */
			scroll_length,		/* Number of scrollable lines */
			route = 0,		/* Stereo, Mono ... */
			old_route = 1,
			track = -2,		/* Current track no. */
			savch = 0;		/* Saved char for cda_ungetch */
STATIC bool_t		isvisual = FALSE,	/* Currently in visual mode */
			stat_on = FALSE,	/* Visual: cda is "on" */
			ostat_on = TRUE,	/* Previous value */
			refresh_fkeys = TRUE,	/* Refresh funct key labels */
			help = FALSE,		/* Display help in info_win? */
			old_help = TRUE,	/* Previous value */
			refresh_sts = TRUE,	/* Refresh status line */
			echflag = FALSE,	/* Own echo flag */
			savch_echo;		/* Control echo for savch */
STATIC word32_t		oastat0 = (word32_t)-1,	/* Previous status value */
			oastat1 = (word32_t)-1;
STATIC WINDOW		*info_win,		/* Scrolling window for info */
			*status_win;		/* Window for status */


/***********************
 *  internal routines  *
 ***********************/


/*
 * cda_wgetch
 *	Own version of curses wgetch. This interworks with cda_ungetch.
 *
 * Args:
 *	None
 *
 * Return:
 *	Input character or function key token.
 */
STATIC int
cda_wgetch(WINDOW *win)
{
	int	ch;

	if (savch) {
		/* Echo character now if echo on but not yet echoed */
		if (!savch_echo && echflag &&
		    isprint(savch) && !iscntrl(savch)) {
			waddch(win, savch);
			wrefresh(win);
		}
		ch = savch;
		savch = 0;
		return (ch);
	}

	ch = wgetch(win);

	/* Do our own echoing because switching it on and off doesn't
	 * seem to work on some platforms.
	 */
	if (echflag && isprint(ch) && !iscntrl(ch)) {
		waddch(win, ch);
		wrefresh(win);
	}

	return (ch);
}


/*
 * cda_ungetch
 *	Own version of ungetch, because some systems don't have it.
 *	Also, we need to remember the echo status of the ungotten
 *	character.
 *
 * Args:
 *	Char or function key token to return.
 *
 * Return:
 *	Nothing
 */
STATIC void
cda_ungetch(int ch)
{
	savch = ch;
	/* Remember whether the character has been echoed */
	savch_echo = echflag;
}


/*
 * cda_wgetstr
 *	Own version of wgetstr, using cda_wgetch and cda_ungetch.
 *
 * Args:
 *	Buffer to be filled with input string.
 *
 * Return:
 *	Nothing.
 */
STATIC void
cda_wgetstr(WINDOW *win, char *ipbuff, int max)
{
	int	ch,
		n,
		x,
		y;
	char	*p;
	bool_t	eos = FALSE;

	p = ipbuff;
	n = 0;

	while (!eos) {
		if (n > max) {
			beep();
			break;
		}

		ch = cda_wgetch(win);

		switch (ch) {
		case KEY_BACKSPACE:
		case KEY_LEFT:
		case '\010':
			if (n > 0) {
				p--;
				n--;

				/* Echo the effect of backspace */
				getyx(win, y, x);
				wmove(win, y, x-1);
				waddch(win, ' ');
				wmove(win, y, x-1);
			}
			break;

		case KEY_DOWN:
		case '\n':
		case '\r':
			/* End-of-string */
			eos = TRUE;
			break;

		default:
			if (!isprint(ch) || iscntrl(ch))
				beep();
			else {
				*p++ = (char) ch;
				n++;
			}

			break;
		}

		wrefresh(win);
	}

	*p = '\0';
}


/*
 * cda_echo
 *	Own versions of curses echo function.
 *
 * Args: None
 *
 * Return: Nothing
 *
 */
STATIC void
cda_echo(void)
{
	echflag = TRUE;
}


/*
 * cda_noecho
 *	Own versions of curses noecho function.
 *
 * Args: None
 *
 * Return: Nothing
 *
 */
STATIC void
cda_noecho(void)
{
	echflag = FALSE;
}


/*
 * cda_screen
 *	Paints the screen in visual mode, geting status and extinfo
 *	as required.
 *
 * Args:
 *	None.
 *
 * Return:
 *	void
 */
/*ARGSUSED*/
STATIC void
cda_screen(int signo)
{
	word32_t	cmd,
			arg[CDA_NARGS],
			astat0,
			astat1;
	int		x,
			y,
			i,
			trkno,
			ntrks,
			min,
			sec,
			rptcnt;
	bool_t		cddb = FALSE,
			playing;
	char		*p;

	/* Need to refresh function key labels? */
	if (refresh_fkeys) {
		refresh_fkeys = FALSE;
		wmove(status_win, 3, 0);
		waddstr(status_win, STATUS_LINE0);
		wmove(status_win, 4, 0);
		waddstr(status_win, STATUS_LINE1);
		wmove(status_win, 5, 0);
		waddstr(status_win, STATUS_LINE2);
		wrefresh(status_win);

		box(status_win, 0, 0);
	}

	/* If daemon running, get status and update display */
	if (!stat_on) {
		/* Daemon not running - just update display to "off" */
		if (stat_on != ostat_on) {
			wmove(status_win, ON_Y, ON_X);
			waddstr(status_win, "On");

			wmove(status_win, OFF_Y, OFF_X);
			wattron(status_win, A_STANDOUT);
			waddstr(status_win, "Off");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, LOAD_Y, LOAD_X);
			waddstr(status_win, "Load");

			wmove(status_win, EJECT_Y, EJECT_X);
			waddstr(status_win, "Eject");

			wmove(status_win, PLAY_Y, PLAY_X);
			waddstr(status_win, "Play");

			wmove(status_win, PAUSE_Y, PAUSE_X);
			waddstr(status_win, "Pause");

			wmove(status_win, STOP_Y, STOP_X);
			waddstr(status_win, "Stop");

			wmove(status_win, LOCK_Y, LOCK_X);
			waddstr(status_win, "Lock");

			wmove(status_win, UNLOCK_Y, UNLOCK_X);
			waddstr(status_win, "Unlock");

			wmove(status_win, SHUFFLE_Y, SHUFFLE_X);
			waddstr(status_win, "Shuffle");

			wmove(status_win, PROGRAM_Y, PROGRAM_X);
			waddstr(status_win, "Program");

			wmove(status_win, REPEAT_ON_Y, REPEAT_ON_X);
			waddstr(status_win, "On");

			wmove(status_win, REPEAT_OFF_Y, REPEAT_OFF_X);
			waddstr(status_win, "Off");
		}
	}
	else {
		/* Daemon running - get status and update display */
		cmd = CDA_STATUS;
		memset(arg, 0, CDA_NARGS * sizeof(word32_t));
		if (!cda_sendcmd(cmd, arg)) {
			cd_quit(&status);
			exit(2);
		}

		wmove(status_win, ON_Y, ON_X);
		wattron(status_win, A_STANDOUT);
		waddstr(status_win, "On");
		wattroff(status_win, A_STANDOUT);

		wmove(status_win, OFF_Y, OFF_X);
		waddstr(status_win, "Off");

		astat0 = arg[0];
		astat1 = arg[1];
		rptcnt = (int) arg[2];

		if (astat0 != oastat0 || astat1 != oastat1) {
			switch (RD_ARG_MODE(astat0)) {
			case M_NODISC:
				wmove(status_win, LOAD_Y, LOAD_X);
				waddstr(status_win, "Load");
				 
				wmove(status_win, EJECT_Y, EJECT_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Eject");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, PLAY_Y, PLAY_X);
				waddstr(status_win, "Play");

				wmove(status_win, PAUSE_Y, PAUSE_X);
				waddstr(status_win, "Pause");

				wmove(status_win, STOP_Y, STOP_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Stop");
				wattroff(status_win, A_STANDOUT);

				break;

			case M_STOP:
				wmove(status_win, LOAD_Y, LOAD_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Load");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, EJECT_Y, EJECT_X);
				waddstr(status_win, "Eject");

				wmove(status_win, PLAY_Y, PLAY_X);
				waddstr(status_win, "Play");

				wmove(status_win, PAUSE_Y, PAUSE_X);
				waddstr(status_win, "Pause");

				wmove(status_win, STOP_Y, STOP_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Stop");
				wattroff(status_win, A_STANDOUT);

				break;

			case M_PLAY:
				wmove(status_win, LOAD_Y, LOAD_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Load");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, EJECT_Y, EJECT_X);
				waddstr(status_win, "Eject");

				wmove(status_win, PLAY_Y, PLAY_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Play");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, PAUSE_Y, PAUSE_X);
				waddstr(status_win, "Pause");

				wmove(status_win, STOP_Y, STOP_X);
				waddstr(status_win, "Stop");

				break;

			case M_PAUSE:
				wmove(status_win, LOAD_Y, LOAD_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Load");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, EJECT_Y, EJECT_X);
				waddstr(status_win, "Eject");

				wmove(status_win, PLAY_Y, PLAY_X);
				waddstr(status_win, "Play");

				wmove(status_win, PAUSE_Y, PAUSE_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Pause");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, STOP_Y, STOP_X);
				waddstr(status_win, "Stop");

				break;
			}

			wmove(status_win, LOCK_Y, LOCK_X);
			if (RD_ARG_LOCK(astat0))
				wattron(status_win, A_STANDOUT);
			waddstr(status_win, "Lock");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, UNLOCK_Y, UNLOCK_X);
			if (!RD_ARG_LOCK(astat0))
				wattron(status_win, A_STANDOUT);
			waddstr(status_win, "Unlock");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, SHUFFLE_Y, SHUFFLE_X);
			if (RD_ARG_SHUF(astat0))
				wattron(status_win, A_STANDOUT);
			waddstr(status_win, "Shuffle");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, PROGRAM_Y, PROGRAM_X);
			if (stat_on && RD_ARG_MODE(astat0) != M_NODISC &&
			    !RD_ARG_SHUF(astat0)) {
				cmd = CDA_PROGRAM;
				arg[0] = 1;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}
				if (arg[0] > 0)
					wattron(status_win, A_STANDOUT);
			}
			waddstr(status_win, "Program");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, REPEAT_ON_Y, REPEAT_ON_X);
			if (RD_ARG_REPT(astat0))
				wattron(status_win, A_STANDOUT);
			waddstr(status_win, "On");
			wattroff(status_win, A_STANDOUT);

			wmove(status_win, REPEAT_OFF_Y, REPEAT_OFF_X);
			if (!RD_ARG_REPT(astat0))
				wattron(status_win, A_STANDOUT);
			waddstr(status_win, "Off");
			wattroff(status_win, A_STANDOUT);
		}

		wmove(status_win, 1, 1);
		if (RD_ARG_MODE(astat0) == M_PLAY ||
		    RD_ARG_MODE(astat0) == M_PAUSE ||
		    RD_ARG_MODE(astat0) == M_NODISC) {

			if (RD_ARG_MODE(astat0) != M_NODISC) {
				wprintw(status_win,
					"Track %02u Index %02u %02u:%02u  ",
					RD_ARG_TRK(astat1),
					RD_ARG_IDX(astat1),
					RD_ARG_MIN(astat1),
					RD_ARG_SEC(astat1));
			}

			cmd = CDA_VOLUME;
			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			wprintw(status_win, "Volume %3u%% ", arg[1]);
			cmd = CDA_BALANCE;
			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			wprintw(status_win, "Balance %3u%%  ", arg[1]);

			switch (route) {
			case 0:
				wprintw(status_win, "Stereo    ");
				break;
			case 1:
				wprintw(status_win, "Reverse   ");
				break;
			case 2:
				wprintw(status_win, "Mono-L    ");
				break;
			case 3:
				wprintw(status_win, "Mono-R    ");
				break;
			case 4:
				wprintw(status_win, "Mono-L+R  ");
				break;
			}

			if (rptcnt >= 0)
				wprintw(status_win, "Count %u", rptcnt);

			getyx(status_win, y, x);
			for (i = x; i < COLS-1; i++)
				waddch(status_win, ' ');

			wmove(status_win, 1, 1);
		}
		else if (refresh_sts) {
			refresh_sts = FALSE;
			for (i = 0; i < COLS-2; i++)
				waddch(status_win, ' ');
			wmove(status_win, 1, 1);

			if (stat_on && RD_ARG_MODE(astat0) != M_NODISC &&
			    !RD_ARG_SHUF(astat0)) {
				cmd = CDA_PROGRAM;
				arg[0] = 1;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}

				if (arg[0] > 0) {
					wprintw(status_win, "Program: ");
					for (i = 0; i < arg[0]; i++) {
						wprintw(status_win, " %02u",
							arg[i+1]);
					}
				}
			}
		}
	}
	wrefresh(status_win);

	/* See if we want to display help info */
	if (help) {
		if (!old_help) {
			wclear(info_win);
			wmove(info_win, 0, 0);
			wprintw(info_win, HELP_INFO);
			scroll_line = 0;
			prefresh(info_win, scroll_line, 0, 0, 0,
				 LINES-8, COLS-1);
			old_help = help;
		}
		signal(SIGALRM, cda_screen);
		alarm(1);
		return;
	}
	else if (old_help) {
		wclear(info_win);
		scroll_line = 0;
		track = -2;	/* force display of version/device */
	}

	/* If state is unchanged since last time, no more to do */
	if (stat_on == ostat_on && old_help == help && old_route == route &&
	    RD_ARG_MODE(astat0) == RD_ARG_MODE(oastat0) &&
	    RD_ARG_TRK(astat1) == RD_ARG_TRK(oastat1) &&
	    RD_ARG_IDX(astat1) == RD_ARG_IDX(oastat1)) {
		ostat_on = stat_on;
		oastat0 = astat0;
		oastat1 = astat1;
		old_help = help;

		/* Call us again - nothing is too much trouble! */
		signal(SIGALRM, cda_screen);
		alarm(1);
		return;
	}

	old_help = help;
	old_route = route;
	ostat_on = stat_on;
	oastat0 = astat0;
	oastat1 = astat1;

	/* Now display data, according to our state: */

	/* Off, or no disc, display version and device */
	if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
		if (track != -1) {
			track = -1;
			wclear(info_win);
			wmove(info_win, 0,0);
			wprintw(info_win,
				"CDA - Command Line CD Audio Player");
			wmove(info_win, 0, COLS-18);
			wprintw(info_win, "Press ");
			wattron(info_win, A_STANDOUT);
			wprintw(info_win, "?");
			wattroff(info_win, A_STANDOUT);
			wprintw(info_win, " for help.\n\n");

			wprintw(info_win, "CD audio        v%s%s PL%d\n",
				VERSION, VERSION_EXT, PATCHLEVEL);
			if (stat_on) {
				cmd = CDA_VERSION;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}
			}
			else sprintf((char *) arg, "%s%s PL%d\n%s",
				VERSION, VERSION_EXT, PATCHLEVEL, di_vers());
			wprintw(info_win, "CD audio daemon v%s\n",
				(char *) arg);
			wprintw(info_win, COPYRIGHT);
				
			wprintw(info_win, "\nDevice: %s\n", app_data.device);
			if (stat_on) {
				cmd = CDA_DEVICE;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}
				wprintw(info_win, "%s\n", (char *) arg);
			}

			prefresh(info_win, scroll_line, 0, 0, 0,
				 LINES-8, COLS-1);
			getyx(info_win, scroll_length, i);
			--scroll_length;
		}
	}
	else if (track != RD_ARG_TRK(astat1)) {
		/* If disc loaded, display extinfo */

		wclear(info_win);
		wmove(info_win, 0, 0);

		/* Get database entry */
		if (RD_ARG_MODE(astat0) == M_PLAY ||
		    RD_ARG_MODE(astat0) == M_PAUSE) {
			track = RD_ARG_TRK(astat1);
		}
		else
			track = -1;

		cmd = CDA_EXTINFO;
		arg[0] = 0;
		arg[1] = track;
		if (!cda_sendcmd(cmd, arg)) {
			cd_quit(&status);
			exit(2);
		}

		/* Load CD database entry */
		dbprog_dbclear(&status);
		cddb = dbprog_dbload(arg[0]);

		ntrks = arg[0] & 0xff;
		if (!cddb || RD_ARG_MODE(astat0) == M_STOP) {
			/* No database entry */
			cmd = CDA_TOC;
			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			wprintw(info_win, "Disc ID: %08x%s",
				arg[0], (cur_db.extd == NULL) ? "" : " *");

			wmove(info_win, 0, COLS-18);
			wprintw(info_win, "Press ");
			wattron(info_win, A_STANDOUT);
			wprintw(info_win, "?");
			wattroff(info_win, A_STANDOUT);
			wprintw(info_win, " for help.\n\n");

			wprintw(info_win, "%s\n\n",
				cddb ? cur_db.dtitle : "(unknown disc title)");

			for (i = 0; i < (int) ntrks; i++) {
				RD_ARG_TOC(arg[i+1], trkno, playing, min, sec);
				wprintw(info_win, "%s%02u %02u:%02u %s %s\n",
					playing ? ">" : " ",
					trkno, min, sec,
					(cur_db.extt[i] == NULL) ? " " : "*",
					cddb ? cur_db.trklist[i] : "??");
			}

			RD_ARG_TOC(arg[i+1], trkno, playing, min, sec);
			wprintw(info_win, "\nTotal Time: %02u:%02u\n",
				min, sec);
		}
		else {
			/* Have database entry */
			if (cur_db.extd == NULL) {
				wprintw(info_win,
				    "No Extended Information for this CD.\n");
			}
			else {
				wprintw(info_win, "%s\n\n", cur_db.dtitle);

				/* Not using wprintw here to avoid a bug
				 * with very long strings
				 */
				p = cur_db.extd;
				for (; *p != '\0'; p++)
				       waddch(info_win, *p);
				waddch(info_win, '\n');
			}

			for (i = 0; i < COLS-1; i++)
				waddch(info_win, ACS_HLINE);
			waddch(info_win, '\n');

			/* If Play or Pause, display track info */
			if (RD_ARG_MODE(astat0) == M_PLAY ||
			    RD_ARG_MODE(astat0) == M_PAUSE) {
				if (cur_db.trklist[arg[2]] == NULL) {
					wprintw(info_win,
						"(No title for track %02u.)\n",
						arg[1]);
				}
				else {
					wprintw(info_win, "%s\n",
						cur_db.trklist[arg[2]]);

					if (cur_db.extt[arg[2]] != NULL) {
						/* Not using wprintw here
						 * to avoid a bug with very
						 * long strings
						 */
						p = cur_db.extt[arg[2]];
						waddch(info_win, '\n');
						for (; *p != '\0'; p++)
						       waddch(info_win, *p);
						waddch(info_win, '\n');
					}
				}
			}
		}
		scroll_line = 0;
		getyx(info_win, scroll_length, i);
		prefresh(info_win, scroll_line, 0, 0, 0, LINES-8, COLS-1);
	}
	oastat0 = astat0;
	oastat1 = astat1;

	/* Come back in 1 sec */
	signal(SIGALRM, cda_screen);
	alarm(1);
}


#if defined(SIGTSTP) && defined(SIGCONT)
/*
 * ontstp
 *	Handler for job control stop signal
 *
 * Args:
 *	signo - The signal number
 *
 * Return:
 *	Nothing
 */
void
ontstp(int signo)
{
	if (signo != SIGTSTP)
		return;

	/* Cancel alarms */
	alarm(0);

	/* Put screen in sane state */
	move(LINES-1, 0);
	printw("\r\n\n");
	putp(cursor_normal);
	refresh();
	reset_shell_mode();

	/* Now stop the process */
	signal(SIGTSTP, SIG_DFL);
	kill(getpid(), SIGTSTP);
}


/*
 * oncont
 *	Handler for job control continue signal
 *
 * Args:
 *	signo - The signal number
 *
 * Return:
 *	Nothing
 */
void
oncont(int signo)
{
	if (signo != SIGCONT)
		return;

	signal(SIGTSTP, ontstp);
	signal(SIGCONT, oncont);

	/* Restore visual attributes */
	reset_prog_mode();
	putp(cursor_invisible);

	/* Set up for auto refresh */
	wclear(info_win);
	wclear(status_win);
	oastat0 = oastat1 = (word32_t) -1;
	ostat_on = !stat_on;
	old_help = !help;
	old_route = !route;
	refresh_fkeys = TRUE;
}
#endif	/* SIGTSTP SIGCONT */


/***********************
 *   public routines   *
 ***********************/


/*
 * cda_vtidy
 *	Tidy up and go home for visual mode.
 *
 * Args:
 *	None
 *
 * Return:
 *	Nothing
 */
void
cda_vtidy(void)
{
	if (isvisual) {
		keypad(stdscr, FALSE);
		putp(cursor_normal);
		clear();

		move(LINES-1, 0);
		refresh();
		echo();
		nocbreak();
		endwin();
	}

	printf("%s\n", errmsg != NULL ? errmsg : "Goodbye!");
}


/*
 * cda_visual
 *	Visual (curses mode) interface.
 *
 * Args:
 *	None.
 *
 * Return:
 *	Return code for exit()
 */
void
cda_visual(void)
{
	word32_t	cmd;
	word32_t	arg[CDA_NARGS];
	word32_t	astat0,
			astat1;
	int		inp,
			i,
			j,
			mins,
			secs;
	char		ipbuff[80],
			*p;

	stat_on = FALSE;
	 
	/* Open FIFOs - command side */
	cda_sfd[1] = open(spipe, O_WRONLY);
	if (cda_sfd[1] >= 0) {
		stat_on = TRUE;

		cda_rfd[1] = open(rpipe, O_RDONLY);
		if (cda_rfd[1] < 0)
			cd_fatal_popup(NULL, "cannot open recv pipe");
	}

#if defined(SIGTSTP) && defined(SIGCONT)
	/* Handle job control */
	signal(SIGTSTP, ontstp);
	signal(SIGCONT, oncont);
#endif

	/* Initialize curses and paint initial screen */
	initscr();
	isvisual = TRUE;

	noecho();
	cbreak();
	putp(cursor_invisible);

	if ((info_win = newpad(MAXTRACK * 2, COLS)) == (WINDOW *) NULL) {
		cd_quit(&status);
		exit(2);
	}

	keypad(info_win, TRUE);

	if ((status_win = newwin(7, COLS, LINES-7, 0)) == (WINDOW *) NULL) {
		cd_quit(&status);
		exit(2);
	}

	keypad(status_win, TRUE);

	wmove(status_win, 3, 0);
	waddstr(status_win, STATUS_LINE0);
	wmove(status_win, 4, 0);
	waddstr(status_win, STATUS_LINE1);
	wmove(status_win, 5, 0);
	waddstr(status_win, STATUS_LINE2);
	wrefresh(status_win);

	box(status_win, 0, 0);

	/* Paint the screen every second */
	cda_screen(0);

	/* Main processing loop */
	while ((inp = cda_wgetch(status_win)) != KEY_F(8)) {
		if (inp == 'q' || inp == 'Q')
			break;

		/* Cancel alarm so we don't nest */
		alarm(0);

		/* Get current status */
		if (stat_on) {
			cmd = CDA_STATUS;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			astat0 = arg[0];
			astat1 = arg[1];
		}

		switch (inp) {
		case KEY_F(1):	/* On/Off */
		case 'o':
		case 'O':
			if (!stat_on) {
				if (cda_daemon(&status)) {
					cd_quit(&status);
					exit(0);
				}

				stat_on = FALSE;

				/* Open FIFOs - command side */
				cda_sfd[1] = open(spipe, O_WRONLY);
				if (cda_sfd[1] >= 0) {
					stat_on = TRUE;

					cda_rfd[1] = open(rpipe, O_RDONLY);
					if (cda_rfd[1] < 0) {
						cd_fatal_popup(
							NULL,
							"cannot open recv pipe"
						);
					}
				}

				cmd = CDA_ON;
				arg[0] = 0;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}

				wmove(status_win, 1, 1);
				for (i = 0; i < COLS-2; i++)
					waddch(status_win, ' ');
				wmove(status_win, 1, 1);
				wprintw(status_win,
				    "CD audio daemon pid=%d dev=%s started.",
				    arg[0], app_data.device);
				wrefresh(status_win);
			}
			else {
				cmd = CDA_OFF;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}

				wmove(status_win, 1, 1);
				for (i = 0; i < COLS-2; i++)
					waddch(status_win, ' ');
				wmove(status_win, 1, 1);
				wprintw(status_win,
				    "CD audio daemon pid=%d dev=%s exited.",
				    arg[0], app_data.device);
				wrefresh(status_win);

				stat_on = FALSE;
				close(cda_sfd[1]);
				close(cda_rfd[1]);
				cda_sfd[1] = cda_rfd[1] = -1;
			}
			track = -2; /* force redisplay of version info */
			scroll_line = 0;
			break;

		case KEY_F(2):	/* Load/Eject */
		case 'j':
		case 'J':
			if (!stat_on) {
				beep();
				break;
			}

			cmd = CDA_DISC;
			arg[0] = (RD_ARG_MODE(astat0) == M_NODISC) ? 0 : 1;

			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			track = -2; /* force redisplay of version info */
			refresh_sts = TRUE;
			break;

		case KEY_F(3): /* Play/Pause */
		case 'p':
		case 'P':
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
				beep();
				break;
			}
			if (RD_ARG_MODE(astat0) == M_PLAY)
				cmd = CDA_PAUSE;
			else
				cmd = CDA_PLAY;

			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case KEY_F(4): /* Stop */
		case 's':
		case 'S':
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
				beep();
				break;
			}

			if (RD_ARG_MODE(astat0) != M_PLAY &&
			    RD_ARG_MODE(astat0) != M_PAUSE) {
				beep();
				break;
			}

			cmd = CDA_STOP;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			refresh_sts = TRUE;
			break;

		case KEY_F(5):	/* Lock/Unlock */
		case 'k':
		case 'K':
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
				beep();
				break;
			}

			cmd = CDA_LOCK;
			arg[0] = RD_ARG_LOCK(astat0) ? 0 : 1;

			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case KEY_F(6):	/* Shuffle/Program */
		case 'u':
		case 'U':
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
				beep();
				break;
			}

			/* Not allowed if play or pause */
			if (RD_ARG_MODE(astat0) == M_PLAY ||
			    RD_ARG_MODE(astat0) == M_PAUSE) {
				beep();
				break;
			}

			/* See if program on */
			cmd = CDA_PROGRAM;
			arg[0] = 1;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			/* If neither program nor shuffle, set shuffle */
			if (!RD_ARG_SHUF(astat0) && arg[0] == 0) {
				cmd = CDA_SHUFFLE;
				arg[0] = 1;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}
				break;
			}
			else if (RD_ARG_SHUF(astat0)) {
				/* If shuffle, turn it off and prompt
				 * for program
				 */
				cmd = CDA_SHUFFLE;
				arg[0] = 0;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}

				wmove(status_win, SHUFFLE_Y, SHUFFLE_X);
				waddstr(status_win, "Shuffle");

				wmove(status_win, PROGRAM_Y, PROGRAM_X);
				wattron(status_win, A_STANDOUT);
				waddstr(status_win, "Program");
				wattroff(status_win, A_STANDOUT);

				wmove(status_win, 1, 1);
				for (i = 0; i < COLS-2; i++)
					waddch(status_win, ' ');

				wmove(status_win, 1, 1);
				wprintw(status_win, "Program: ");
				cda_echo();
				putp(cursor_normal);
				wrefresh(status_win);

				/* Before reading the program list, check for
				 * F6 or "u", and dismiss prog mode if found
				 */
				i = cda_wgetch(status_win);
				if (i != KEY_F(6) && i != 'u') {
					cda_ungetch(i);

					cda_wgetstr(status_win, ipbuff,
						    COLS-12);

					/* Is the string just read was
					 * terminated by F6, it will
					 * have been ungotten, but must be
					 * thrown away or it will cause
					 * return to shuffle mode.
					 */
					if (savch == KEY_F(6))
						savch = 0;

					j = 0;
					p = ipbuff;
					while ((i = strtol(p, &p, 10)) != 0)
					{
						arg[++j] = i;

						if (p == NULL)
							break;

						while (*p != '\0' &&
						       !isdigit(*p))
							p++;
					}

					arg[0] = (word32_t) -j;
					cmd = CDA_PROGRAM;
					if (!cda_sendcmd(cmd, arg)) {
						errmsg = NULL;
						beep();
					}
				}

				cda_noecho();
				putp(cursor_invisible);

				refresh_sts = TRUE;
				break;
			}
			else {
				/* Program is on - reset it */
				arg[0] = 0;
				cmd = CDA_PROGRAM;
				if (!cda_sendcmd(cmd, arg)) {
					cd_quit(&status);
					exit(2);
				}
				refresh_sts = TRUE;
			}
			break;

		case KEY_F(7):	/* Repeat On/Off */
		case 'e':
		case 'E':
			if (!stat_on) {
				beep();
				break;
			}

			cmd = CDA_REPEAT;
			arg[0] = RD_ARG_REPT(astat0) ? 0 : 1;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case KEY_LEFT:	/* Prev track */
		case KEY_RIGHT:	/* Next track */
		case 'C':
		case 'c':
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC ||
			    RD_ARG_MODE(astat0) != M_PLAY) {
				beep();
				break;
			}

			arg[0] = (inp == KEY_LEFT || inp == 'C') ? 0 : 1;
			cmd = CDA_TRACK;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case '<':	/* Prev index */
		case '>':	/* Next index */
			if (!stat_on || RD_ARG_MODE(astat0) == M_NODISC) {
				beep();
				break;
			}

			arg[0] = (inp == '<') ? 0 : 1;
			cmd = CDA_INDEX;
			if (!cda_sendcmd(cmd, arg)) {
				errmsg = NULL;
				beep();
			}
			break;

		case '+':	/* Vol up */
		case '-':	/* Vol down */
			if (!stat_on) {
				beep();
				break;
			}

			cmd = CDA_VOLUME;
			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			if (inp == '+') {
				if (arg[1] <= 95)
					arg[1] += 5;
			}
			else if (arg[1] >= 5) {
				arg[1] -= 5;
			}
			arg[0] = 1;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case 'l':	/* Bal left */
		case 'L':
		case 'r':	/* Bal right */
		case 'R':
			if (!stat_on) {
				beep();
				break;
			}

			cmd = CDA_BALANCE;
			arg[0] = 0;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}

			if (inp == 'r' || inp == 'R') {
				if (arg[1] <= 95)
					arg[1] += 5;
			}
			else if (arg[1] >= 5) {
				arg[1] -= 5;
			}
			arg[0] = 1;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		case '?':	/* Help */
			help = TRUE;
			scroll_length = HELP_SCROLL_LENGTH;
			break;

		case ' ':	/* Return from help */
			help = FALSE;
			break;

		case '\014':	/* ^l - repaint screen */
		case '\022':	/* ^r - repaint screen */
			wclear(info_win);
			wclear(status_win);
			oastat0 = oastat1 = (word32_t) -1;
			ostat_on = !stat_on;
			old_help = !help;
			old_route = !route;
			refresh_fkeys = TRUE;
			break;

		case KEY_UP:	/* Scroll up */
		case KEY_DOWN:	/* Scroll down */
		case '^':
		case 'v':
		case 'V':
			if (inp == KEY_UP || inp == '^')
				scroll_line--;
			else
				scroll_line++;

			if (scroll_line < 0) {
				beep();
				scroll_line = 0;
			}
			if (scroll_line > (scroll_length - 1)) {
				beep();
				scroll_line = scroll_length - 1;
			}
			prefresh(info_win, scroll_line, 0, 0, 0,
				 LINES-8, COLS-1);
			break;

		case '\011':	/* Route */
			if (!stat_on) {
				beep();
				break;
			}

			if (--route < 0)
				route = 4;

			cmd = CDA_ROUTE;
			arg[0] = 1;
			arg[1] = route;
			if (!cda_sendcmd(cmd, arg)) {
				cd_quit(&status);
				exit(2);
			}
			break;

		default:
			if (isdigit(inp)) {
				if (!stat_on) {
					beep();
					break;
				}

				cda_ungetch(inp);

				wmove(status_win, 1, 1);
				for (i = 0; i < COLS-2; i++)
					waddch(status_win, ' ');

				wmove(status_win, 1, 1);
				wprintw(status_win, "Track n [mins secs] : ");
				cda_echo();
				putp(cursor_normal);
				wrefresh(status_win);

				cda_wgetstr(status_win, ipbuff, 20);
				i = strtol(ipbuff, &p, 10);
				if (i != 0) {
					mins = secs = 0;
					if (p != NULL) {
						while (*p != '\0' &&
						       !isdigit(*p))
							p++;

						mins = strtol(p, &p, 10);

						if (p != NULL) {
							while (*p != '\0' &&
							       !isdigit(*p))
								p++;

							secs = strtol(p,&p,10);
						}
					}

					cmd = CDA_PLAY;
					arg[0] = i;
					arg[1] = mins;
					arg[2] = secs;
					if (!cda_sendcmd(cmd, arg)) {
						errmsg = NULL;
						beep();
					}
				}
				else
					beep(); /* i = 0 */

				cda_noecho();
				putp(cursor_invisible);
				refresh_sts = TRUE;

				break;

			}
			break;
		}

		/* Repaint screen */
		cda_screen(0);
	}

	/* Tidy up and go home */
	alarm(0);
	errmsg = NULL;

	if (stat_on) {
		cmd = CDA_ON;
		arg[0] = 0;
		if (!cda_sendcmd(cmd, arg)) {
			cd_quit(&status);
			exit(2);
		}
		sprintf(emsg, "CD audio daemon pid=%d still running.", arg[0]);
		errmsg = emsg;
	}

	cd_quit(&status);
	exit(0);
}


#endif	/* NOVISUAL */

