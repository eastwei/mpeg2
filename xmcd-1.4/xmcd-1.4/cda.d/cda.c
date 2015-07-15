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
#ifndef LINT
static char *_cda_c_ident_ = "@(#)cda.c	5.19 95/01/29";
#endif

#define _CDA_

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "common.d/patchlevel.h"
#include "libdi.d/libdi.h"
#include "cda.d/cda.h"
#include "cda.d/visual.h"


#define DBPATH_SEPCHAR	':'			/* Database path separator */
#define PGM_SEPCHAR	','			/* Program seq separator */


extern char		*ttyname();

appdata_t		app_data;		/* Option settings */
database_t		cur_db;			/* Database entry of CD */
curstat_t		status;			/* Current CD player status */
char			*errmsg = NULL,		/* Error msg for use on exit */
			emsg[ERR_BUF_SZ],	/* Error message buffer */
			spipe[FILE_PATH_SZ],	/* Send pipe path */
			rpipe[FILE_PATH_SZ];	/* Receive pipe path */
int			cda_sfd[2] = {-1,-1},	/* Send pipe file desc */
			cda_rfd[2] = {-1,-1};	/* Receive pipe file desc */
FILE			*errfp = stderr;	/* Error message stream */

STATIC char		lockfile[FILE_PATH_SZ],	/* Lock file path */
			**dbdirs = NULL;	/* CD database directories */
STATIC int		cont_delay = 1;		/* Status display interval */
STATIC dev_t		cd_rdev;		/* CD device number */
STATIC bool_t		visual = FALSE,		/* Visual (curses) mode */
			isdaemon = FALSE,	/* Am I the daemon process */
			stat_cont = FALSE;	/* Continuous display status */
STATIC FILE		*ttyfp;			/* /dev/tty */



/***********************
 *  internal routines  *
 ***********************/


/*
 * onsig
 *	Signal handler to shut down application.
 *
 * Args:
 *	signo - The signal number.
 *
 * Return:
 *	Nothing.
 */
/* ARGSUSED */
STATIC void
onsig(int signo)
{
	cd_quit(&status);
	exit(3);
}


/*
 * onintr
 *	Signal handler to stop continuous status display.
 *
 * Args:
 *	signo - The signal number.
 *
 * Return:
 *	Nothing.
 */
/* ARGSUSED */
STATIC void
onintr(int signo)
{
	signal(signo, SIG_IGN);
	stat_cont = FALSE;
	printf("\r");
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
			parm[128];
	static bool_t	dev_cmdline = FALSE,
			force_debug = FALSE;

	/* Default maximum number of CD database directories */
	app_data.max_dbdirs = MAX_DBDIRS;

	/* Initialize error messages */
	app_data.str_nomethod = STR_NOMETHOD;
	app_data.str_novu = STR_NOVU;
	app_data.str_nomemory = STR_NOMEMORY;
	app_data.str_nocfg = STR_NOCFG;
	app_data.str_notrom = STR_NOTROM;
	app_data.str_notscsi2 = STR_NOTSCSI2;
	app_data.str_moderr = STR_MODERR;
	app_data.str_staterr = STR_STATERR;
	app_data.str_noderr = STR_NODERR;
	app_data.str_dbdirserr = STR_DBDIRSERR;
	app_data.str_recoverr = STR_RECOVERR;
	app_data.str_maxerr = STR_MAXERR;
	app_data.str_tmpdirerr = STR_TMPDIRERR;

	if (di_isdemo())
		app_data.device = "(none)";

	/* Open common config file */
	if ((fp = fopen(path, "r")) == NULL) {
		if (priv && !di_isdemo()) {
			/* Cannot open config file. */
			sprintf(emsg, app_data.str_nocfg, path);
			cd_fatal_popup(NULL, emsg);
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

		if (!di_isdemo() && sscanf(buf, "device: %s\n", parm) > 0) {
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
						NULL,
						app_data.str_nomemory
					);
				}
				strcpy(app_data.device, parm);
			}
			continue;
		}
		if (sscanf(buf, "dbdir: %s\n", parm) > 0) {
			app_data.dbdir = (char *) MEM_ALLOC(strlen(parm) + 1);
			if (app_data.dbdir == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			strcpy(app_data.dbdir, parm);
			continue;
		}
		if (sscanf(buf, "maxDbdirs: %s\n", parm) > 0) {
			app_data.max_dbdirs = atoi(parm);
			continue;
		}
		if (sscanf(buf, "previousThreshold: %s\n", parm) > 0) {
			app_data.prev_threshold = atoi(parm);
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
		parm[128];

	if ((fp = fopen(path, "r")) == NULL) {
		if (priv && !di_isdemo()) {
			/* Cannot open master device-specific
			 * config file.
			 */
			sprintf(emsg, app_data.str_nocfg, path);
			cd_fatal_popup(NULL, emsg);
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
		 * * force the channel routing setting to normal.
		 */
		if (!app_data.chroute_supp)
			app_data.ch_route = 0;
	}
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
 * dbprog_parse
 *	Parse the shuffle/program mode play sequence text string, and
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
		return FALSE;

	n = strlen(cur_db.playorder) + 1;
	if ((tmpbuf = (char *) MEM_ALLOC(n)) == NULL)
		cd_fatal_popup(NULL, app_data.str_nomemory);

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
 * cda_init
 *	Initialize the CD interface subsystem.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
cda_init(curstat_t *s)
{
	char	str[FILE_PATH_SZ];

	/* Get system-wide device-specific configuration parameters */
	sprintf(str, "%s/config/%s",
		app_data.libdir, basename(app_data.device));
	devspec_parminit(str, TRUE);

	/* Get user device-specific configuration parameters */
	sprintf(str, "%s/.xmcdcfg/%s",
		homedir(get_ouid()), basename(app_data.device));
	devspec_parminit(str, FALSE);

	/* Initialize the CD hardware */
	di_init(s);
}


/*
 * cda_start
 *	Start the CD interface subsystem.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
cda_start(curstat_t *s)
{
	/* Debug information */
	if (app_data.debug) {
		fprintf(errfp, "\ndevnum=%d\n",
			app_data.devnum);
		fprintf(errfp, "device=%s\n",
			app_data.device);
		fprintf(errfp, "libdir=%s\n",
			app_data.libdir);
		fprintf(errfp, "deviceInterfaceMethod=%d\n",
			app_data.di_method);
		fprintf(errfp, "minimumPlayBlocks=%d\n",
			app_data.min_playblks);
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

		fprintf(errfp, "\n");
	}

	/* Start up I/O interface */
	di_start(s);

	/* Open FIFOs - daemon side */
	if ((cda_sfd[0] = open(spipe, O_RDONLY)) < 0) {
		perror("CD audio daemon: cannot open send pipe");
		cd_quit(s);
		exit(4);
	}
	if ((cda_rfd[0] = open(rpipe, O_WRONLY)) < 0) {
		perror("CD audio daemon: cannot open recv pipe");
		cd_quit(s);
		exit(5);
	}
}


/*
 * cda_poll
 *	Periodic polling function - used to manage program, shuffle,
 *	and repeat modes.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
STATIC void
cda_poll(curstat_t *s)
{
	static int	n = 0;

	if (++n > 100)
		n = 0;

	if (s->mode != M_PLAY || n % 2)
		return;

	if (s->prog_tot > 0 || s->repeat) {
		if (di_check_disc(s))
			di_status_upd(s);
	}
}


/*
 * cda_sendpkt
 *	Write a CDA packet down the pipe.
 *
 * Args:
 *	name - The text string describing the caller module
 *	fd - Pipe file descriptor
 *	s - Pointer to the packet data
 *
 * Return:
 *	TRUE - pipe write successful
 *	FALSE - pipe write failed
 */
STATIC bool_t
cda_sendpkt(char *name, int fd, cdapkt_t *s)
{
	byte_t	*p = (byte_t *) s;
	int	i,
		ret;

	if (fd < 0)
		return FALSE;

	/* Brand packet with magic number */
	s->magic = CDA_MAGIC;

	/* Send a packet */
	i = sizeof(cdapkt_t);
	while ((ret = write(fd, p, i)) < i) {
		if (ret < 0) {
			if (errno == EBADF || errno == EINTR) {
				/* Avoid hogging CPU */
				sleep(1);
			}
			else {
				sprintf(emsg,
					"%s: packet write error (errno=%d)\n",
					name, errno);
				cd_warning_popup(NULL, emsg);
				return FALSE;
			}
		}
		else if (ret == 0) {
			/* Avoid hogging CPU */
			sleep(1);
		}
		else {
			i -= ret;
			p += ret;
		}
	}

	return TRUE;
}


/*
 * cda_getpkt
 *	Read a CDA packet from the pipe.
 *
 * Args:
 *	name - The text string describing the caller module
 *	fd - Pipe file descriptor
 *	s - Pointer to the packet data
 *
 * Return:
 *	TRUE - pipe read successful
 *	FALSE - pipe read failed
 */
STATIC bool_t
cda_getpkt(char *name, int fd, cdapkt_t *r)
{
	byte_t	*p = (byte_t *) r;
	int	i,
		ret;

	if (fd < 0)
		return FALSE;

	/* Get a packet */
	i = sizeof(cdapkt_t);
	while ((ret = read(fd, p, i)) < i) {
		if (ret < 0) {
			if (errno == EBADF || errno == EINTR) {
				/* Avoid hogging CPU */
				sleep(1);
			}
			else {
				sprintf(emsg,
					"%s: packet read error (errno=%d)\n",
					name, errno);
				cd_warning_popup(NULL, emsg);
				return FALSE;
			}
		}
		else if (ret == 0) {
			/* Use this occasion to perform polling function */
			cda_poll(&status);

			/* Avoid hogging CPU */
			sleep(1);
		}
		else {
			i -= ret;
			p += ret;
		}
	}

	/* Check packet for magic number */
	if (r->magic != CDA_MAGIC) {
		sprintf(emsg, "%s: bad packet magic number.", name);
		cd_warning_popup(NULL, emsg);
		return FALSE;
	}

	return TRUE;
}


/*
 * cda_docmd
 *	Perform the command received.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *	p - Pointer to the CDA packet structure.
 *
 * Return:
 *	TRUE - Received a CDA_OFF command: daemon should shut down
 *	FALSE - Received normal command.
 */
STATIC bool_t
cda_docmd(curstat_t *s, cdapkt_t *p)
{
	int		i,
			j,
			min,
			sec;
	word32_t	blkno;
	bool_t		stopfirst,
			offsets;

	/* Update CD status */
	if (di_check_disc(s) && p->cmd != CDA_OFF)
		di_status_upd(s);

	/* Default status */
	p->retcode = CDA_OK;

	switch (p->cmd) {
	case CDA_ON:
		p->arg[0] = getpid();
		break;

	case CDA_OFF:
		p->arg[0] = getpid();
		return TRUE;

	case CDA_DISC:
		if (p->arg[0] == 0) {
			/* Load */
			if (s->mode == M_NODISC)
				di_load_eject(s);
			else
				p->retcode = CDA_INVALID;
		}
		else {
			/* Eject */
			if (s->mode != M_NODISC)
				di_load_eject(s);
			else
				p->retcode = CDA_INVALID;
		}

		break;

	case CDA_LOCK:
		if (s->mode == M_NODISC)
			p->retcode = CDA_INVALID;
		else if (p->arg[0] == 0) {
			/* Unlock */
			if (s->caddy_lock) {
				s->caddy_lock = FALSE;
				di_lock(s, FALSE);
			}
			else
				p->retcode = CDA_INVALID;
		}
		else {
			/* Lock */
			if (!s->caddy_lock) {
				s->caddy_lock = TRUE;
				di_lock(s, TRUE);
			}
			else
				p->retcode = CDA_INVALID;
		}

		break;

	case CDA_PLAY:
		switch (s->mode) {
		case M_PLAY:
			stopfirst = TRUE;
			break;
		case M_NODISC:
			p->retcode = CDA_INVALID;
			return FALSE;
		default:
			stopfirst = FALSE;
			break;
		}

		/* Starting track number */
		i = -1;
		if (p->arg[0] != 0) {
			if (s->shuffle || s->prog_cnt > 0) {
				p->retcode = CDA_INVALID;
				break;
			}

			for (i = 0; i < (int) s->tot_trks; i++) {
				if (s->trkinfo[i].trkno == p->arg[0])
					break;
			}

			if (i >= (int) s->tot_trks) {
				/* Invalid track specified */
				p->retcode = CDA_PARMERR;
				break;
			}

			s->cur_trk = p->arg[0];
		}

		if (stopfirst) {
			/* Stop current playback first */
			di_stop(s, TRUE);

			/*
			 * Restore s->cur_trk value because di_stop() zaps it
			 */
			if (p->arg[0] != 0)
				s->cur_trk = p->arg[0];
		}

		if (p->arg[0] != 0 &&
		    (int) p->arg[1] >= 0 && (int) p->arg[2] >= 0) {
			/* Track offset specified */
			if (p->arg[2] > 59) {
				p->retcode = CDA_PARMERR;
				break;
			}

			msftoblk((byte_t) p->arg[1], (byte_t) p->arg[2],
				 0, &blkno, 0);

			if (blkno >=
			    (s->trkinfo[i+1].addr - s->trkinfo[i].addr)) {
				p->retcode = CDA_PARMERR;
				break;
			}

			s->cur_trk_addr = blkno;
			s->cur_trk_min = (byte_t) p->arg[1];
			s->cur_trk_sec = (byte_t) p->arg[2];
			s->cur_trk_frame = 0;

			s->cur_tot_addr = s->trkinfo[i].addr + s->cur_trk_addr;
			blktomsf(s->cur_tot_addr, &s->cur_tot_min,
				 &s->cur_tot_sec, &s->cur_tot_frame,
				 MSF_OFFSET(s));
		}

		/* Start playback */
		di_play_pause(s);

		break;

	case CDA_PAUSE:
		if (s->mode == M_PLAY)
			di_play_pause(s);
		else
			p->retcode = CDA_INVALID;

		break;

	case CDA_STOP:
		di_stop(s, TRUE);
		break;

	case CDA_TRACK:
		if (p->arg[0] == 0) {
			/* Previous track */
			if (s->mode == M_PLAY) {
				if ((i = curtrk_pos(s)) > 0)
					s->cur_tot_addr = s->trkinfo[i].addr;
				di_prevtrk(s);
			}
			else
				p->retcode = CDA_INVALID;
		}
		else {
			/* Next track */
			if (s->mode == M_PLAY)
				di_nexttrk(s);
			else
				p->retcode = CDA_INVALID;
		}

		break;

	case CDA_INDEX:
		if (p->arg[0] == 0) {
			/* Previous index */
			if (s->mode == M_PLAY && s->prog_tot <= 0) {
				if (s->cur_idx > 1)
					s->cur_tot_addr = s->sav_iaddr;
				di_previdx(s);
			}
			else
				p->retcode = CDA_INVALID;
		}
		else {
			/* Next index */
			if (s->mode == M_PLAY && s->prog_tot <= 0)
				di_nextidx(s);
			else
				p->retcode = CDA_INVALID;
		}

		break;

	case CDA_PROGRAM:
		if (s->mode == M_NODISC)
			p->retcode = CDA_INVALID;
		else if ((int) p->arg[0] > 0) {
			/* Query */
			p->arg[0] = (word32_t) s->prog_tot;
			p->arg[1] = (word32_t) -1;

			for (i = 0; i < (int) s->prog_tot; i++) {
				p->arg[i+1] = (word32_t)
				    s->trkinfo[s->playorder[i]].trkno;
			}
		}
		else if (p->arg[0] == 0) {
			/* Clear */
			if (s->shuffle) {
				/* program and shuffle modes are mutually-
				 * exclusive.
				 */
				p->retcode = CDA_INVALID;
			}
			else {
				p->arg[1] = 0;
				s->prog_tot = 0;
			}
		}
		else if ((int) p->arg[0] < 0) {
			/* Define */
			if (s->shuffle) {
				/* program and shuffle modes are mutually-
				 * exclusive.
				 */
				p->retcode = CDA_INVALID;
				break;
			}

			s->prog_tot = -(p->arg[0]);

			for (i = 0; i < (int) s->prog_tot; i++) {
				for (j = 0; j < (int) s->tot_trks; j++) {
					if (s->trkinfo[j].trkno == p->arg[i+1])
						break;
				}

				if (j >= (int) s->tot_trks) {
					s->prog_tot = 0;
					p->retcode = CDA_PARMERR;
					break;
				}

				s->playorder[i] = j;
			}
		}
		else
			p->retcode = CDA_PARMERR;

		break;

	case CDA_SHUFFLE:
		if (!s->shuffle && s->prog_tot > 0) {
			p->retcode = CDA_INVALID;
			break;
		}

		if (s->mode != M_NODISC && s->mode != M_STOP) {
			p->retcode = CDA_INVALID;
			break;
		}

		di_shuffle(s, (bool_t) (p->arg[0] == 1));
		break;

	case CDA_REPEAT:
		di_repeat(s, (bool_t) (p->arg[0] == 1));
		break;

	case CDA_VOLUME:
		if (p->arg[0] == 0)
			/* Query */
			p->arg[1] = (word32_t) s->level;
		else if ((int) p->arg[1] >= 0 && (int) p->arg[1] <= 100)
			/* Set */
			di_level(s, (byte_t) p->arg[1], FALSE);
		else
			p->retcode = CDA_PARMERR;

		break;

	case CDA_BALANCE:
		if (p->arg[0] == 0) {
			/* Query */
			p->arg[1] = (word32_t)
			    ((int) (s->level_right - s->level_left) / 2) + 50;
		}
		else if ((int) p->arg[1] == 50) {
			/* Center setting */
			s->level_left = s->level_right = 100;
			di_level(s, (byte_t) s->level, FALSE);
		}
		else if ((int) p->arg[1] < 50 && (int) p->arg[1] >= 0) {
			/* Attenuate the right channel */
			s->level_left = 100;
			s->level_right = 100 + (((int) p->arg[1] - 50) * 2);
			di_level(s, (byte_t) s->level, FALSE);
		}
		else if ((int) p->arg[1] > 50 && (int) p->arg[1] <= 100) {
			/* Attenuate the left channel */
			s->level_left = 100 - (((int) p->arg[1] - 50) * 2);
			s->level_right = 100;
			di_level(s, (byte_t) s->level, FALSE);
		}
		else
			p->retcode = CDA_PARMERR;

		break;

	case CDA_ROUTE:
		if (p->arg[0] == 0) {
			/* Query */
			p->arg[1] = (word32_t) app_data.ch_route;
		}
		else if ((int) p->arg[1] >= 0 && (int) p->arg[1] <= 4) {
			/* Set */
			app_data.ch_route = (int) p->arg[1];
			di_route(s);
		}
		else
			p->retcode = CDA_PARMERR;

		break;

	case CDA_STATUS:
		/* Initialize */
		memset(p->arg, 0, CDA_NARGS * sizeof(word32_t));

		WR_ARG_MODE(p->arg[0], s->mode);

		if (s->caddy_lock)
			WR_ARG_LOCK(p->arg[0]);
		if (s->shuffle)
			WR_ARG_SHUF(p->arg[0]);
		if (s->repeat)
			WR_ARG_REPT(p->arg[0]);
		if (!s->shuffle && s->prog_tot > 0)
			WR_ARG_PROG(p->arg[0]);

		WR_ARG_TRK(p->arg[1], s->cur_trk);
		WR_ARG_IDX(p->arg[1], s->cur_idx);
		WR_ARG_MIN(p->arg[1], s->cur_trk_min);
		WR_ARG_SEC(p->arg[1], s->cur_trk_sec);

		if (s->repeat && s->mode == M_PLAY)
			p->arg[2] = (word32_t) s->rptcnt;
		else
			p->arg[2] = (word32_t) -1;

		break;

	case CDA_TOC:
		if (s->mode == M_NODISC) {
			p->retcode = CDA_INVALID;
			break;
		}

		offsets = (p->arg[0] == 1);

		/* Initialize */
		memset(p->arg, 0, CDA_NARGS * sizeof(word32_t));

		p->arg[0] = dbprog_discid(s);

		if (offsets) {
			for (i = 0; i < (int) s->tot_trks; i++) {
				WR_ARG_TOC(p->arg[i+1], s->trkinfo[i].trkno,
					   s->cur_trk,
					   s->trkinfo[i].min,
					   s->trkinfo[i].sec);
			}
		}
		else {
			for (i = 0; i < (int) s->tot_trks; i++) {
				j = ((s->trkinfo[i+1].min * 60 +
				      s->trkinfo[i+1].sec) - 
				     (s->trkinfo[i].min * 60 +
				      s->trkinfo[i].sec));
				min = j / 60;
				sec = j % 60;

				WR_ARG_TOC(p->arg[i+1], s->trkinfo[i].trkno,
					   s->cur_trk, min, sec);
			}
		}

		/* Lead-out track */
		WR_ARG_TOC(p->arg[i+1], s->trkinfo[i].trkno,
			   0, s->trkinfo[i].min, s->trkinfo[i].sec);

		break;

	case CDA_EXTINFO:
		if (s->mode == M_NODISC) {
			p->retcode = CDA_INVALID;
			break;
		}

		p->arg[0] = dbprog_discid(s);
		p->arg[2] = (word32_t) -1;

		if ((int) p->arg[1] == -1) {
			if (s->mode != M_PLAY) {
				p->arg[1] = p->arg[2] = (word32_t) -1;
				break;
			}

			p->arg[1] = (word32_t) s->cur_trk;
			j = (int) s->cur_trk;
		}
		else
			j = (int) p->arg[1];

		for (i = 0; i < (int) s->tot_trks; i++) {
			if ((int) s->trkinfo[i].trkno == j)
				break;
		}
		if (i < (int) s->tot_trks)
			p->arg[2] = i;
		else
			p->retcode = CDA_PARMERR;

		break;

	case CDA_DEVICE:
		sprintf((char *) p->arg,
			"CD-ROM: %s %s (%s)\nMode:   %s",
			s->vendor, s->prod, s->revnum, di_mode());
		break;

	case CDA_VERSION:
		sprintf((char *) p->arg, "%s%s PL%d\n%s",
			VERSION, VERSION_EXT, PATCHLEVEL, di_vers());
		break;

	default:
		p->retcode = CDA_FAILED;
		break;
	}

	return FALSE;
}


/*
 * prn_program
 *	Print current program sequence, if any.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_program(word32_t arg[])
{
	int	i;

	if ((int) arg[0] > 0) {
		printf("Current program:");
		for (i = 0; i < arg[0]; i++)
			printf(" %d", arg[i+1]);
		printf("\n");
	}
	else if (arg[0] == 0 && (int) arg[1] == -1)
		printf("No play sequence defined.\n");
}


/*
 * prn_vol
 *	Print current volume setting.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_vol(word32_t arg[])
{
	if (arg[0] == 0)
		printf("Current volume: %u (range 0-100)\n", arg[1]);
}


/*
 * prn_bal
 *	Print current balance setting.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_bal(word32_t arg[])
{
	if (arg[0] == 0) {
		printf("Current balance: %u (range 0-100, center:50)\n",
			arg[1]);
	}
}


/*
 * prn_route
 *	Print current channel routing setting.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_route(word32_t arg[])
{
	if (arg[0] == 0) {
		printf("Current routing: %u ", arg[1]);

		switch (arg[1]) {
		case 0:
			printf("(normal stereo)\n");
			break;
		case 1:
			printf("(reverse stereo)\n");
			break;
		case 2:
			printf("(mono-L)\n");
			break;
		case 3:
			printf("(mono-R)\n");
			break;
		case 4:
			printf("(mono-L+R)\n");
			break;
		}
	}
}


/*
 * prn_stat
 *	Print current CD status.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_stat(word32_t arg[])
{
	if (stat_cont)
		printf("\r");

	switch (RD_ARG_MODE(arg[0])) {
	case M_NODISC:
		printf("No_Disc    -- -- --:--");
		break;
	case M_STOP:
		printf("CD_Stopped -- -- --:--");
		break;
	case M_PLAY:
		printf("CD_Playing %02u %02u %02u:%02u",
			RD_ARG_TRK(arg[1]), RD_ARG_IDX(arg[1]),
			RD_ARG_MIN(arg[1]), RD_ARG_SEC(arg[1]));
		break;
	case M_PAUSE:
		printf("CD_Paused  %02u %02u %02u:%02u",
			RD_ARG_TRK(arg[1]), RD_ARG_IDX(arg[1]),
			RD_ARG_MIN(arg[1]), RD_ARG_SEC(arg[1]));
		break;
	default:
		printf("Inv_status -- -- --:--");
		break;
	}

	printf(" %slock", RD_ARG_LOCK(arg[0]) ? "+" : "-");
	printf(" %sshuf", RD_ARG_SHUF(arg[0]) ? "+" : "-");
	printf(" %sprog", RD_ARG_PROG(arg[0]) ? "+" : "-");
	printf(" %srept", RD_ARG_REPT(arg[0]) ? "+" : "-");

	if ((int) arg[2] >= 0)
		printf(" %u", arg[2]);
	else
		printf(" -");

	if (!stat_cont)
		printf("\n");
}


/*
 * prn_toc
 *	Print current CD Table Of Contents.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_toc(word32_t arg[])
{
	int		i;
	byte_t		ntrks,
			trkno,
			min,
			sec;
	bool_t		cddb,
			playing;

	/* Load CD database entry */
	cddb = dbprog_dbload(arg[0]);

	ntrks = arg[0] & 0xff;

	printf("Disc ID: %s %08x%s\n",
		(cur_db.category[0] == '\0') ?
			"(no category)" : cur_db.category,
		arg[0],
		(cur_db.extd == NULL) ? "" : " *");

	printf("%s\n\n", cddb ? cur_db.dtitle : "(unknown disc title)");

	for (i = 0; i < (int) ntrks; i++) {
		RD_ARG_TOC(arg[i+1], trkno, playing, min, sec);
		printf("%s%02u %02u:%02u %s %s\n",
			playing ? ">" : " ",
			trkno, min, sec,
			(cur_db.extt[i] == NULL) ? " " : "*",
			cddb ? cur_db.trklist[i] : "??");
	}

	RD_ARG_TOC(arg[i+1], trkno, playing, min, sec);
	printf("\nTotal Time: %02u:%02u\n", min, sec);
}


/*
 * prn_extinfo
 *	Print current Disc or Track Extended Information.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_extinfo(word32_t arg[])
{
	bool_t	cddb = FALSE;

	/* Load CD database entry */
	cddb = dbprog_dbload(arg[0]);

	if (!cddb) {
		printf("No CD database entry found for this CD\n");
		return;
	}

	printf("-------- Disc Extended Information --------\n");

	if (cur_db.extd == NULL)
		printf("(none)\n");
	else {
		printf("%s\n\n", cur_db.dtitle);
		printf("%s\n", cur_db.extd);
	}

	if ((int) arg[1] < 0)
		return;

	printf("\n------ Track %02u Extended Information ------\n", arg[1]);

	if (cur_db.extt[arg[2]] == NULL)
		printf("(none)\n");
	else {
		printf("%s\n\n", cur_db.trklist[arg[2]]);
		printf("%s\n", cur_db.extt[arg[2]]);
	}
}


/*
 * prn_device
 *	Print device information.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_device(word32_t arg[])
{
	printf("Device: %s\n", app_data.device);
	printf("%s\n", (char *) arg);
}


/*
 * prn_ver
 *	Print version number and other information.
 *
 * Args:
 *	arg - Argument array from CD audio daemon response packet.
 *
 * Return:
 *	Nothing.
 */
STATIC void
prn_ver(word32_t arg[])
{
	printf("CDA - Command Line CD Audio Player\n\n");
	printf("CD audio        v%s%s PL%d\n",
		VERSION, VERSION_EXT, PATCHLEVEL);
	printf("CD audio daemon v%s\n", (char *) arg);
	printf(COPYRIGHT);
}


/*
 * usage
 *	Display command line usage syntax
 *
 * Args:
 *	argc, argv
 *
 * Return:
 *	Nothing.
 */
STATIC void
usage(char *progname)
{
	fprintf(errfp, "Usage: %s [-dev device] [-debug] command\n",
		progname);
	fprintf(errfp, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"Valid commands are:\n",
		"\ton\n",
		"\toff\n",
		"\tdisc <load | eject>\n",
		"\tlock <on | off>\n",
		"\tplay [track# [min:sec]]\n",
		"\tpause\n",
		"\tstop\n",
		"\ttrack <prev | next>\n",
		"\tindex <prev | next>\n",
		"\tprogram [clear | track# ...]\n",
		"\tshuffle <on | off>\n",
		"\trepeat <on | off>\n",
		"\tvolume [value#]    (range 0-100)\n",
		"\tbalance [value#]   (range 0-100, center:50)\n",
		"\troute [value#]     (0:stereo 1:reverse 2:mono-L 3:mono-R 4:mono-L+R)\n",
		"\tstatus [cont [secs#]]\n",
		"\ttoc [offsets]\n",
		"\textinfo [track#]\n",
		"\tdevice\n",
		"\tversion\n",
		"\tvisual\n");
}


/*
 * parse_time
 *	Parse a string of the form "min:sec" and convert to integer
 *	minute and second values.
 *
 * Args:
 *	str - Pointer to the "min:sec" string.
 *	min - pointer to where the minute value is to be written.
 *	sec - pointer to where the second value is to be written.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
parse_time(char *str, int *min, int *sec)
{
	char	*p;

	if ((p = strchr(str, ':')) == NULL)
		return FALSE;
	
	if (!isdigit(*str) || !isdigit(*(p+1)))
		return FALSE;

	*p = '\0';
	*min = atoi(str);
	*sec = atoi(p+1);
	*p = ':';

	return TRUE;
}


/*
 * cda_parse_args
 *	Parse CDA command line arguments.
 *
 * Args:
 *	argc, argv
 *	cmd - Pointer to the command code.
 *	arg - Command argument array.
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
cda_parse_args(int argc, char **argv, word32_t *cmd, word32_t arg[])
{
	int	i,
		j,
		min,
		sec;

	/* Default values */
	*cmd = 0;
	memset(arg, 0, CDA_NARGS * sizeof(word32_t));

	/* Command line args handling */
	for (i = 1; i < argc; i++) {
		if (*cmd != 0) {
			/* Multiple commands specified */
			usage(argv[0]);
			return FALSE;
		}

		if (strcmp(argv[i], "-dev") == 0) {
			if (++i < argc) {
				if (!di_isdemo())
					app_data.device = argv[i];
			}
			else {
				usage(argv[0]);
				return FALSE;
			}
		}
		else if (strcmp(argv[i], "-debug") == 0) {
			app_data.debug = TRUE;
		}
		else if (strcmp(argv[i], "on") == 0) {
			*cmd = CDA_ON;
		}
		else if (strcmp(argv[i], "off") == 0) {
			*cmd = CDA_OFF;
		}
		else if (strcmp(argv[i], "disc") == 0) {
			/* <load | eject> */
			if (++i < argc) {
				if (strcmp(argv[i], "load") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "eject") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_DISC;
		}
		else if (strcmp(argv[i], "lock") == 0) {
			/* <on | off> */
			if (++i < argc) {
				if (strcmp(argv[i], "off") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "on") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_LOCK;
		}
		else if (strcmp(argv[i], "play") == 0) {
			/* [track# [min:sec]] */
			if ((i+1) < argc && isdigit(argv[i+1][0])) {
				/* The user specified the track number */
				if ((arg[0] = atoi(argv[++i])) == 0) {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}

				if ((i+1) < argc &&
				    parse_time(argv[i+1], &min, &sec)) {
					/* The user specified a time offset */
					arg[1] = min;
					arg[2] = sec;
					i++;
				}
				else {
					arg[1] = arg[2] = (word32_t) -1;
				}
			}
			*cmd = CDA_PLAY;
		}
		else if (strcmp(argv[i], "pause") == 0) {
			*cmd = CDA_PAUSE;
		}
		else if (strcmp(argv[i], "stop") == 0) {
			*cmd = CDA_STOP;
		}
		else if (strcmp(argv[i], "track") == 0) {
			/* <prev | next> */
			if (++i < argc) {
				if (strcmp(argv[i], "prev") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "next") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_TRACK;
		}
		else if (strcmp(argv[i], "index") == 0) {
			/* <prev | next> */
			if (++i < argc) {
				if (strcmp(argv[i], "prev") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "next") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_INDEX;
		}
		else if (strcmp(argv[i], "program") == 0) {
			/* [clear | track# ...] */
			arg[0] = 1;

			if ((i+1) < argc) {
				if (strcmp(argv[i+1], "clear") == 0) {
					i++;
					arg[0] = 0;
				}
				else {
					j = 0;
					while ((i+1) < argc &&
					       isdigit(argv[i+1][0]) &&
					       j < (CDA_NARGS-1)) {
						arg[++j] = atoi(argv[++i]);
					}
					if (j > 0)
						arg[0] = (word32_t) -j;
				}
			}
			*cmd = CDA_PROGRAM;
		}
		else if (strcmp(argv[i], "shuffle") == 0) {
			/* <on | off> */
			if (++i < argc) {
				if (strcmp(argv[i], "off") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "on") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_SHUFFLE;
		}
		else if (strcmp(argv[i], "repeat") == 0) {
			/* <on | off> */
			if (++i < argc) {
				if (strcmp(argv[i], "off") == 0)
					arg[0] = 0;
				else if (strcmp(argv[i], "on") == 0)
					arg[0] = 1;
				else {
					/* Wrong arg */
					usage(argv[0]);
					return FALSE;
				}
			}
			else {
				/* Missing arg */
				usage(argv[0]);
				return FALSE;
			}
			*cmd = CDA_REPEAT;
		}
		else if (strcmp(argv[i], "volume") == 0) {
			/* [value#] */
			if ((i+1) >= argc || !isdigit(argv[i+1][0]))
				/* Query */
				arg[0] = 0;
			else {
				/* Set */
				arg[0] = 1;
				arg[1] = (word32_t) atoi(argv[++i]);
			}
			*cmd = CDA_VOLUME;
		}
		else if (strcmp(argv[i], "balance") == 0) {
			/* [value#] */
			if ((i+1) >= argc || !isdigit(argv[i+1][0]))
				/* Query */
				arg[0] = 0;
			else {
				/* Set */
				arg[0] = 1;
				arg[1] = (word32_t) atoi(argv[++i]);
			}
			*cmd = CDA_BALANCE;
		}
		else if (strcmp(argv[i], "route") == 0) {
			/* [value#] */
			if ((i+1) >= argc || !isdigit(argv[i+1][0]))
				/* Query */
				arg[0] = 0;
			else {
				/* Set */
				arg[0] = 1;
				arg[1] = (word32_t) atoi(argv[++i]);
			}
			*cmd = CDA_ROUTE;
		}
		else if (strcmp(argv[i], "status") == 0) {
			/* [cont [secs#]] */
			if ((i+1) >= argc || strcmp(argv[i+1], "cont") != 0)
				stat_cont = FALSE;
			else {
				i++;
				stat_cont = TRUE;
				if ((i+1) < argc && isdigit(argv[i+1][0]))
					cont_delay = atoi(argv[++i]);
			}
			*cmd = CDA_STATUS;
		}
		else if (strcmp(argv[i], "toc") == 0) {
			/* [offsets] */
			if ((i+1) >= argc || strcmp(argv[i+1], "offsets") != 0)
				arg[0] = 0;
			else {
				i++;
				arg[0] = 1;
			}
			*cmd = CDA_TOC;
		}
		else if (strcmp(argv[i], "extinfo") == 0) {
			/* [track#] */
			arg[0] = 0;
			if ((i+1) >= argc || !isdigit(argv[i+1][0]))
				arg[1] = (word32_t) -1;
			else
				arg[1] = atoi(argv[++i]);

			*cmd = CDA_EXTINFO;
		}
		else if (strcmp(argv[i], "device") == 0) {
			*cmd = CDA_DEVICE;
		}
		else if (strcmp(argv[i], "version") == 0) {
			*cmd = CDA_VERSION;
		}
		else if (strcmp(argv[i], "visual") == 0) {
#ifdef NOVISUAL
			fprintf(errfp, "%s %s\n",
				"Cannot start visual mode:",
				"curses support is not compiled in!");
			return FALSE;
#else
			visual = TRUE;
			*cmd = CDA_STATUS;
			/* Make sure simulator/debug output is redirectable */
			ttyfp = stderr;
#endif
		}
		else {
			usage(argv[0]);
			return FALSE;
		}
	}

	if (*cmd == 0) {
		/* User did not specify a command */
		usage(argv[0]);
		return FALSE;
	}

	return TRUE;
}


/***********************
 *   public routines   *
 ***********************/


/*
 * cd_beep
 *	Empty stub routine.
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
	/* Null stub function */
}


/*
 * cd_quit
 *      Shut down CD audio
 *
 * Args:
 *      s - Pointer to the curstat_t structure.
 *
 * Return:
 *      Nothing.
 */
void
cd_quit(curstat_t *s)
{
	if (isdaemon) {
		/* Shut down CD interface subsystem */
		di_halt(s);

		/* Close FIFOs - daemon side */
		if (cda_sfd[0] >= 0)
			close(cda_sfd[0]);
		if (cda_rfd[0] >= 0)
			close(cda_rfd[0]);

		/* Remove FIFOs */
		if (spipe[0] != '\0')
			unlink(spipe);
		if (rpipe[0] != '\0')
			unlink(rpipe);

		/* Remove lock file */
		if (lockfile[0] != '\0')
			unlink(lockfile);
	}
#ifndef NOVISUAL
	else {
		cda_vtidy();
	}
#endif
}


/*
 * cd_timeout
 *	Stub routine.
 *
 * Args:
 *	msec - Not used.
 *	handler - Not used.
 *	arg - Not used.
 *
 * Return:
 *	Always 0.
 */
/*ARGSUSED*/
long
cd_timeout(word32_t msec, void (*handler)(), byte_t *arg)
{
	return 0;
}


/*
 * cd_untimeout
 *	Empty stub routine.
 *
 * Args:
 *	id - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
cd_untimeout(long id)
{
	/* Null stub function */
}


/*
 * cd_warning_popup
 *	Print warning message.
 *
 * Args:
 *	title - Not used.
 *	msg - The warning message text string.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
cd_warning_popup(char *title, char *msg)
{
	fprintf(errfp, "CD audio Warning: %s\n", msg);
}


/*
 * cd_fatal_popup
 *	Print fatal error message.
 *
 * Args:
 *	title - Not used..
 *	msg - The fatal error message text string.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
cd_fatal_popup(char *title, char *msg)
{
	fprintf(errfp, "CD audio Fatal Error: %s\n", msg);
	cd_quit(&status);
	exit(6);
}


/*
 * cd_devlock
 *	Create a lock to prevent another cda process from accessing
 *	the same CD-ROM device.
 *
 * Args:
 *	path - The lock file path name.
 *
 * Return:
 *	TRUE if the lock was successful.  If another cda process
 *	is currently has the lock, then this function does not
 *	return.  The program is terminated instead.
 */
bool_t
cd_devlock(char *path)
{
	int		fd;
	pid_t		pid,
			mypid;
	char		buf[12];

	if (di_isdemo())
		return TRUE;	/* No need for locks in demo mode */

	sprintf(lockfile, "%s/lock.%x", TEMP_DIR, cd_rdev);
	mypid = getpid();

	errmsg = "CD busy.";

	for (;;) {
		fd = open(lockfile, O_CREAT | O_EXCL | O_WRONLY);
		if (fd < 0) {
			if (errno == EEXIST) {
				if ((fd = open(lockfile, O_RDONLY)) < 0)
					cd_fatal_popup(NULL, errmsg);

				if (read(fd, buf, 12) > 0)
					pid = (pid_t) atoi(buf);
				else {
					close(fd);
					cd_fatal_popup(NULL, errmsg);
				}

				close(fd);

				if (pid == mypid)
					/* Our own lock */
					return TRUE;

				if (pid <= 0 ||
				    (kill(pid, 0) < 0 && errno == ESRCH)) {
					/* Pid died, steal its lockfile */
					unlink(lockfile);

					if (rpipe[0] != '\0')
						unlink(rpipe);
					if (spipe[0] != '\0')
						unlink(spipe);
				}
				else {
					/* Pid still running: clash */
					cd_fatal_popup(NULL, errmsg);
				}
			}
			else
				cd_fatal_popup(NULL, errmsg);
		}
		else {
			sprintf(buf, "%d\n", mypid);
			write(fd, buf, strlen(buf));

			close(fd);
			chmod(lockfile, 0644);

			return TRUE;
		}
	}
}


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
 * cd_pause_blink
 *	Empty stub routine.
 *
 * Args:
 *	s - not used.
 *	enable - not used.
 *
 * Return:
 *	Nothing.
 */
void
cd_pause_blink(curstat_t *s, bool_t enable)
{
	/* Null stub function */
}


/*
 * dpy_track
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_track(curstat_t *s)
{
	/* Null stub function */
}


/*
 * dpy_index
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_index(curstat_t *s)
{
	/* Null stub function */
}


/*
 * dpy_time
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *	blank - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_time(curstat_t *s, bool_t blank)
{
	/* Null stub function */
}


/*
 * dpy_rptcnt
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_rptcnt(curstat_t *s)
{
	/* Null stub function */
}


/*
 * dpy_playmode
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_playmode(curstat_t *s, bool_t blank)
{
	/* Null stub function */
}


/*
 * dpy_all
 *	Empty stub routine.
 *
 * Args:
 *	s - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dpy_all(curstat_t *s)
{
	/* Null stub function */
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
	s->program = FALSE;

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
 *	Empty stub routine.
 *
 * Args:
 *	state - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
set_lock_btn(bool_t state)
{
	/* Null stub function */
}


/*
 * set_shuffle_btn
 *	Empty stub routine.
 *
 * Args:
 *	state - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
set_shuffle_btn(bool_t state)
{
	/* Null stub function */
}


/*
 * set_vol_slider
 *	Empty stub routine.
 *
 * Args:
 *	val - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
set_vol_slider(int val)
{
	/* Null stub function */
}


/*
 * set_bal_slider
 *	Empty stub routine.
 *
 * Args:
 *	val - Not used.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
set_bal_slider(int val)
{
	/* Null stub function */
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
		/* inverse-squared taper */
		return (MAX_VOL - (SQR(MAX_VOL - v) / MAX_VOL));
	case 2:
		/* squared taper */
		return (SQR(v) / MAX_VOL);
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
		/* inverse-squared taper */
		return (MAX_VOL - isqrt(SQR(MAX_VOL) - (MAX_VOL * v)));
	case 2:
		/* squared taper */
		return (isqrt(v) * 10);
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
 *	v - The logical volume value.
 *
 * Return:
 *	The scaled volume value.
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
 *	v - The 8-bit volume value.
 *
 * Return:
 *	The logical volume value.
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

	if (cur_db.discid == 0)
		/* Already cleared */
		return;

	/* Clear database entry structure */

	cur_db.category[0] = '\0';

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
	}

	if (cur_db.playorder != NULL) {
		MEM_FREE(cur_db.playorder);
		cur_db.playorder = NULL;
	}

	cur_db.discid = 0;
}


/*
 * dbprog_dbget
 *	Load CD database entry for a CD.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
void
dbprog_dbget(curstat_t *s)
{
	dbprog_dbload(dbprog_discid(s));
}


/*
 * curstat_addr
 *	Return the address of the curstat_t structure.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
curstat_t *
curstat_addr(void)
{
	return (&status);
}


/*
 * cda_sendcmd
 *	Send command down the pipe and handle response.
 *
 * Args:
 *	cmd - The command code
 *	arg - Command arguments
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
bool_t
cda_sendcmd(word32_t cmd, word32_t arg[])
{
	cdapkt_t	p,
			r;

	/* Fill in command packet */
	memset(&p, 0, sizeof(cdapkt_t));
	p.pktid = getpid();
	p.cmd = cmd;
	p.retcode = 0;
	memcpy(p.arg, arg, CDA_NARGS * sizeof(word32_t));

	/* Send command packet */
	if (!cda_sendpkt("CD audio", cda_sfd[1], &p)) {
		errmsg = "Cannot send packet to CD audio daemon.";
		return FALSE;
	}

	/* Get response packet */
	if (!cda_getpkt("CD audio", cda_rfd[1], &r)) {
		errmsg = "Cannot get packet from CD audio daemon.";
		return FALSE;
	}

	/* Sanity check */
	if (p.pktid != r.pktid) {
		errmsg = "CD audio pipe packet sequence error.";
		return FALSE;
	}

	/* Return args */
	memcpy(arg, r.arg, CDA_NARGS * sizeof(word32_t));

	/* Check return code */
	switch (r.retcode) {
	case CDA_OK:
		return TRUE;
	case CDA_INVALID:
		errmsg = "This command is not valid in the current state.";
		return FALSE;
	case CDA_PARMERR:
		errmsg = "Command argument error.";
		return FALSE;
	case CDA_FAILED:
		errmsg = "The CD audio daemon does not support this command.";
		return FALSE;
	default:
		errmsg = "The CD audio daemon returned an invalid status.";
		return FALSE;
	}
	/*NOTREACHED*/
}


/*
 * dbprog_dbload
 *	Read in the CD database file entry pertaining to the
 *	currently loaded disc, if available.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	Nothing.
 */
bool_t
dbprog_dbload(word32_t discid)
{
	int		i,
			pos,
			lineno;
	FILE		*fp = NULL;
	char		*cp,
			*path,
			dbfile[FILE_PATH_SZ],
			buf[STR_BUF_SZ + 16],
			tmpbuf[STR_BUF_SZ + 16];
	static bool_t	first_time = TRUE;


	cur_db.discid = discid;

	if (first_time) {
		first_time = FALSE;

		/* Allocate memory for the database
		 * directories string pointers array
		 */
		dbdirs = (char **)(void *) MEM_ALLOC(
			app_data.max_dbdirs * sizeof(char *)
		);
		if (dbdirs == NULL)
			cd_fatal_popup(NULL, app_data.str_nomemory);

		if ((cp = getenv("XMCD_DBPATH")) != NULL) {
			app_data.dbdir = (char *) MEM_ALLOC(strlen(cp) + 1);
			if (app_data.dbdir == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

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
						NULL,
						app_data.str_nomemory
					);
				}

				if (path[0] == '/') {
					/* Absolute path name specified */
					strcpy(dbdirs[i], path);
				}
				else if (path[0] == '~') {
					/* Perform tilde expansion a la
					 * [ck]sh
					 */
					if (path[1] == '/') {
						sprintf(dbdirs[i], "%s%s",
							homedir(get_ouid()),
							&path[1]);
					}
					else if (path[1] == '\0') {
						strcpy(dbdirs[i],
						       homedir(get_ouid()));
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
						    sprintf(dbdirs[i],
							    "%s/%s",
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
			}

			if (cp != NULL && *cp == DBPATH_SEPCHAR)
				*cp = '\0';

			if (path[0] == '/')
				dbdirs[i] = (char *)
					MEM_ALLOC(strlen(path) + 1);
			else
				dbdirs[i] = (char *) MEM_ALLOC(
					strlen(path) +
					strlen(app_data.libdir) + 7
				);

			if (dbdirs[i] == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			if (path[0] == '/')
				strcpy(dbdirs[i], path);
			else
				sprintf(dbdirs[i], "%s/cddb/%s",
					app_data.libdir, path);
		}

		for (i++; i < app_data.max_dbdirs; i++)
			dbdirs[i] = NULL;

	}

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

	if (fp == NULL)
		/* File does not exist or not readable */
		return FALSE;

	/* Record the category */
	strcpy(cur_db.category, basename(dbdirs[i]));

	/* Read first line of database file */
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return FALSE;
	}

	/* Database file signature check */
	if (strncmp(buf, "# xmcd ", 7) != 0) {
		/* Not a supported database file */
		fclose(fp);
		return FALSE;
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
			return FALSE;
		}

		/* Disk title */
		if (!isdaemon && sscanf(buf, "DTITLE=%[^\n]\n", tmpbuf) > 0) {
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

			if (cur_db.dtitle == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			dbprog_strcat(cur_db.dtitle, tmpbuf);
			continue;
		}

		/* Track title */
		if (!isdaemon &&
		    sscanf(buf, "TTITLE%u=%[^\n]\n", &pos, tmpbuf) >= 2) {
			if (pos >= (int) (discid & 0xff))
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

			if (cur_db.trklist[pos] == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			dbprog_strcat(cur_db.trklist[pos], tmpbuf);
			continue;
		}

		/* Disk extended info */
		if (!isdaemon && sscanf(buf, "EXTD=%[^\n]\n", tmpbuf) > 0) {
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

			if (cur_db.extd == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			dbprog_strcat(cur_db.extd, tmpbuf);
			continue;
		}

		/* Track extended info */
		if (!isdaemon &&
		    sscanf(buf, "EXTT%u=%[^\n]\n", &pos, tmpbuf) >= 2) {
			if (pos >= (int) (discid & 0xff))
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

			if (cur_db.extt[pos] == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			dbprog_strcat(cur_db.extt[pos], tmpbuf);
			continue;
		}

		/* Play order */
		if (isdaemon &&
		    sscanf(buf, "PLAYORDER=%[^\n]\n", tmpbuf) > 0) {
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

			if (cur_db.playorder == NULL)
				cd_fatal_popup(NULL, app_data.str_nomemory);

			dbprog_strcat(cur_db.playorder, tmpbuf);

			/* Parse sequence string */
			dbprog_parse(&status);

			continue;
		}
	}

	fclose(fp);
	return TRUE;
}


/*
 * cda_daemon
 *	CD audio daemon main loop function.
 *
 * Args:
 *	s - Pointer to the curstat_t structure.
 *
 * Return:
 *	The CD audio daemon exit status.
 */
int
cda_daemon(curstat_t *s)
{
	cdapkt_t	p;
	bool_t		done = FALSE;
	FILE		*fp;
	struct stat	stbuf;

	/* Make temporary directory, if needed */
	sprintf(emsg, app_data.str_tmpdirerr, TEMP_DIR);
	if (stat(TEMP_DIR, &stbuf) < 0) {
		if (errno == ENOENT) {
			int	omask;

			/* The permissions should be writable by all */
			omask = umask(0);
			if (mkdir(TEMP_DIR, 0777) < 0) {
				(void) umask(omask);
				fprintf(errfp, "%s\n", emsg);
				return 1;
			}
			(void) umask(omask);
		}
		else {
			fprintf(errfp, "%s\n", emsg);
			return 1;
		}
	}
	else if (!S_ISDIR(stbuf.st_mode)) {
		fprintf(errfp, "%s\n", emsg);
		return 1;
	}

	/* Create FIFOs */
	if (MKFIFO(spipe, 0600) < 0) {
		perror("CD audio: Cannot create send pipe");
		cd_quit(s);
		return 1;
	}
	if (MKFIFO(rpipe, 0600) < 0) {
		perror("CD audio: Cannot create recv pipe");
		cd_quit(s);
		return 1;
	}

	/* Become a daemon process */
	switch (fork()) {
	case -1:
		perror("Cannot fork CD audio daemon");
		cd_quit(s);
		return 1;
	case 0:
		/* Child process */
		isdaemon = TRUE;

		if (ttyfp != stderr) {
			errfp = ttyfp;
			fclose(stdin);
			fclose(stdout);
			fclose(stderr);
		}

		break;
	default:
		/* Parent process */
		signal(SIGCLD, SIG_IGN);
		return 0;
	}

	/* Initialize and start drive interfaces */
	cda_init(s);
	cda_start(s);

	/* Handle some signals */
	signal(SIGHUP, onsig);
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#if defined(SIGTSTP) && defined(SIGCONT)
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCONT, SIG_IGN);
#endif

	/* Main command handling loop */
	while (!done) {
		/* Get command packet */
		if (!cda_getpkt("CD audio daemon", cda_sfd[0], &p)) {
			cd_quit(s);
			return 1;
		}

		/* Interpret and carry out command */
		done = cda_docmd(s, &p);

		/* Send response packet */
		if (!cda_sendpkt("CD audio daemon", cda_rfd[0], &p)) {
			cd_quit(s);
			return 1;
		}
	}

	/* Stop the drive */
	cd_quit(s);

	exit(0);
}


/*
 * main
 *	The main function
 */
void
main(int argc, char **argv)
{
	int		ret;
	word32_t	cmd;
	word32_t	arg[CDA_NARGS];
	struct stat	stbuf;
	char		*ttypath,
			*cp,
			str[FILE_PATH_SZ];

	/* Initialize */
	spipe[0] = rpipe[0] = lockfile[0] = '\0';
	if ((ttypath = ttyname(2)) == NULL)
		ttypath = "/dev/tty";
	if ((ttyfp = fopen(ttypath, "w")) != NULL)
		setbuf(ttyfp, NULL);
	else
		ttyfp = stderr;

	/* Parse command line args */
	if (!cda_parse_args(argc, argv, &cmd, arg))
		exit(1);

	/* Initialize libutil */
	util_init();

	/* Set library directory path */
	if ((cp = getenv("XMCD_LIBDIR")) == NULL)
		cd_fatal_popup(NULL, "XMCD_LIBDIR environment not defined.");

	app_data.libdir = (char *) MEM_ALLOC(strlen(cp) + 1);
	if (app_data.libdir == NULL)
		cd_fatal_popup(NULL, app_data.str_nomemory);
	strcpy(app_data.libdir, cp);

	/* Get system common configuration parameters */
	sprintf(str, "%s/config/common.cfg", app_data.libdir);
	common_parminit(str, TRUE);

	/* Get user common configuration parameters */
	sprintf(str, "%s/.xmcdcfg/common.cfg", homedir(get_ouid()));
	common_parminit(str, FALSE);

	/* Sanity check */
	if (app_data.max_dbdirs <= 0 || app_data.max_dbdirs > 100)
		cd_fatal_popup(NULL, app_data.str_dbdirserr);

	/* Check validity of device */
	if (di_isdemo())
		cd_rdev = 0;
	else {
		if (stat(app_data.device, &stbuf) < 0) {
			sprintf(emsg, "Cannot stat %s.", app_data.device);
			cd_fatal_popup(NULL, emsg);
		}
		cd_rdev = stbuf.st_rdev;
	}

	/* FIFO paths */
	sprintf(spipe, "%s/send.%x", TEMP_DIR, cd_rdev);
	sprintf(rpipe, "%s/recv.%x", TEMP_DIR, cd_rdev);

#ifndef NOVISUAL
	if (visual) {
		/* Handle some signals */
		signal(SIGINT, onsig);
		signal(SIGQUIT, onsig);
		signal(SIGTERM, onsig);

		/* Start visual mode */
		cda_visual();
	}
#endif

	if (cmd == CDA_ON) {
		/* Start CDA daemon */
		if ((ret = cda_daemon(&status)))
			exit(ret);
	}

	/* Open FIFOs - command side */
	if ((cda_sfd[1] = open(spipe, O_WRONLY)) < 0) {
		perror("CD audio: cannot open send pipe");
		cd_fatal_popup(
			NULL,
			"Run \"cda on\" first to initialize CD audio daemon."
		);
	}
	if ((cda_rfd[1] = open(rpipe, O_RDONLY)) < 0) {
		perror("CD audio: cannot open recv pipe");
		cd_fatal_popup(
			NULL,
			"Run \"cda on\" first to initialize CD audio daemon."
		);
	}

	/* Handle some signals */
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);

	for (;;) {
		/* Send command to cda daemon */
		if (!cda_sendcmd(cmd, arg)) {
			printf("%s\n", errmsg);
			exit(2);
		}

		/* Display status */
		switch (cmd) {
		case CDA_ON:
			fprintf(errfp,
				"CD audio daemon pid=%d dev=%s started.\n",
				arg[0], app_data.device);
			break;
		case CDA_OFF:
			fprintf(errfp,
				"CD audio daemon pid=%d dev=%s exited.\n",
				arg[0], app_data.device);
			break;
		case CDA_PROGRAM:
			prn_program(arg);
			break;
		case CDA_VOLUME:
			prn_vol(arg);
			break;
		case CDA_BALANCE:
			prn_bal(arg);
			break;
		case CDA_ROUTE:
			prn_route(arg);
			break;
		case CDA_STATUS:
			prn_stat(arg);
			break;
		case CDA_TOC:
			prn_toc(arg);
			break;
		case CDA_EXTINFO:
			prn_extinfo(arg);
			break;
		case CDA_DEVICE:
			prn_device(arg);
			break;
		case CDA_VERSION:
			prn_ver(arg);
			break;
		default:
			break;
		}

		fflush(stdout);

		if (!stat_cont)
			break;

		if (cont_delay > 0)
			sleep(cont_delay);
	}

	/* Close FIFOs - command side */
	if (cda_sfd[1] >= 0)
		close(cda_sfd[1]);
	if (cda_rfd[1] >= 0)
		close(cda_rfd[1]);

	exit(0);
}


