/*
 *   appenv - Common header file for xmcd, cda and libdi.
 *
 *	xmcd  - Motif(tm) CD Audio Player
 *	cda   - Command-line CD Audio Player
 *	libdi - CD Audio Player Device Interface Library
 *
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
#ifndef __APPENV_H__
#define __APPENV_H__

#ifndef LINT
static char *_appenv_h_ident_ = "@(#)appenv.h	5.11 95/01/29";
#endif


/* Whether STATIC should really be... */
#ifdef DEBUG
#define STATIC
#else
#define STATIC		static
#endif


#ifdef _XMINC_

/* Include needed X11 and Motif headers */
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/Protocols.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#endif


/* Common header files to be included for all modules */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <memory.h>
#include <dirent.h>
#include <pwd.h>
#ifdef BSDCOMPAT
#include <strings.h>
#else
#include <string.h>
#endif


/* Define these just in case the OS does not support the POSIX definitions */
#ifndef S_ISBLK
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif


/* Data type definitions: for portability */
#ifdef __alpha
#define SIXTY_FOUR_BIT
#endif

typedef unsigned char	byte_t;			/* 8-bit unsigned */
typedef char		sbyte_t;		/* 8-bit signed */
typedef unsigned short	word16_t;		/* 16-bit unsigned */
typedef short		sword16_t;		/* 16-bit signed */
#ifdef SIXTY_FOUR_BIT
typedef unsigned int	word32_t;		/* 32-bit unsigned */
typedef int		sword32_t;		/* 32-bit signed */
#else
typedef unsigned long	word32_t;		/* 32-bit unsigned */
typedef long		sword32_t;		/* 32-bit signed */
#endif
typedef char		bool_t;			/* Boolean */


/* Endianess */
#define _L_ENDIAN_	1234
#define _B_ENDIAN_	4321

#if defined(i286) || defined(i386) || defined(i486) || \
    defined(__FreeBSD__) || defined(__alpha) || defined(vax) || \
    (defined(mips) && defined(MIPSEL))
#define _BYTE_ORDER_	_L_ENDIAN_
#else
#define _BYTE_ORDER_	_B_ENDIAN_
#endif


/* Platform-specific stuff */
#ifdef macII

#ifndef WEXITSTATUS
#define WEXITSTATUS(x)	(((x.w_status) >> 8) & 0377)
#endif
#ifndef WTERMSIG
#define WTERMSIG(x)	((x.w_status) & 0177)
#endif

typedef union wait	waitret_t;

#else	/* macII */

typedef int		waitret_t;

#endif	/* macII */


#if defined(macII)
#define MKFIFO(path, mode)	mknod((path), S_IFIFO | (mode), 0)
#else
#define MKFIFO(path, mode)	mkfifo((path), (mode))
#endif


#ifdef BSDCOMPAT
#ifndef strchr
#define strchr		index
#endif
#ifndef strrchr
#define strrchr		rindex
#endif
#ifndef SIGCLD
#define SIGCLD		SIGCHLD
#endif
#endif	/* BSDCOMPAT */


/* Boolean flags */
#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif


/* Constant definitions */
#define TEMP_DIR	"/tmp/.cdaudio"		/* Temporary directory */
#define STR_BUF_SZ	64			/* Generic string buf size */
#define ERR_BUF_SZ	256			/* Generic errmsg buf size */
#define FILE_PATH_SZ	72			/* Max file path length */
#define MAXTRACK	100			/* Max number of tracks */
#define LEAD_OUT_TRACK	0xaa			/* Lead-out track number */
#define FRAME_PER_SEC	75			/* Frames per second */
#define MAX_VOL		100			/* Max logical audio volume */


/* Macros */
#define MSF_OFFSET(s)	((s)->trkinfo[0].frame + \
	(((s)->trkinfo[0].min * 60 + (s)->trkinfo[0].sec) * FRAME_PER_SEC))
						/* Starting MSF offset */


/* Defines for the type field in trkinfo_t */
#define TYP_AUDIO	1			/* Audio track */
#define TYP_DATA	2			/* Data track */


/* Defines for the mode field in curstat_t */
#define M_NODISC	0			/* No disc loaded */
#define M_PLAY		1			/* Play mode */
#define M_PAUSE		2			/* Pause mode */
#define M_STOP		3			/* Stop mode */
#define M_A		4			/* First half of A->B mode */
#define M_AB		5			/* Running A->B mode */
#define M_SAMPLE	6			/* Sample mode */


/* Defines for the time_dpy field in curstat_t */
#define T_ELAPSED	0			/* Per-track elapsed time */
#define T_REMAIN_TRACK	1			/* Per-track remaining time */
#define T_REMAIN_DISC	2			/* Whole-disc remaining time */


/* CD per-track information */
typedef struct {
	sword32_t	trkno;			/* Track number */
	word32_t	addr;			/* Absolute offset block */
	byte_t		min;			/* Absolute offset minutes */
	byte_t		sec;			/* Absolute offset seconds */
	byte_t		frame;			/* Absolute offset frame */
	byte_t		type;			/* track type */
} trkinfo_t;


/* Structure containing current status information */
typedef struct {
	byte_t		mode;			/* Player mode */
	byte_t		time_dpy;		/* Time display mode */
	byte_t		reserved[2];		/* reserved */
	sword32_t	first_trk;		/* First track */
	sword32_t	last_trk;		/* Last track */
	byte_t		tot_trks;		/* Total number of tracks */
	byte_t		tot_min;		/* Total minutes */
	byte_t		tot_sec;		/* Total seconds */
	byte_t		tot_frame;		/* Total frame */
	word32_t	tot_addr;		/* Total block length */
	sword32_t	cur_trk;		/* Current track */
	sword32_t	cur_idx;		/* Current index */
	byte_t		cur_tot_min;		/* Current absolute minutes */
	byte_t		cur_tot_sec;		/* Current absolute seconds */
	byte_t		cur_tot_frame;		/* Current absolute frame */
	byte_t		cur_trk_min;		/* Current relative minutes */
	byte_t		cur_trk_sec;		/* Current relative seconds */
	byte_t		cur_trk_frame;		/* Current relative frame */
	byte_t		reserved2[2];		/* reserved */
	word32_t	cur_tot_addr;		/* Current absolute block */
	word32_t	cur_trk_addr;		/* Current relative block */
	word32_t	sav_iaddr;		/* Saved index abs block */
	trkinfo_t	trkinfo[MAXTRACK];	/* Per-track information */
	sword32_t	playorder[MAXTRACK];	/* Prog/Shuf sequence */
	word32_t	rptcnt;			/* Repeat iteration count */
	bool_t		repeat;			/* Repeat mode */
	bool_t		shuffle;		/* Shuffle mode */
	bool_t		program;		/* Program mode */
	bool_t		cddb;			/* CD Database entry */
	bool_t		caddy_lock;		/* Caddy lock */
	byte_t		prog_tot;		/* Prog/Shuf total tracks */
	byte_t		prog_cnt;		/* Prog/Shuf track counter */
	byte_t		level;			/* Current volume level */
	byte_t		level_left;		/* Left channel vol percent */
	byte_t		level_right;		/* Right channel vol percent */
	char		vendor[9];		/* CD-ROM drive vendor */
	char		prod[17];		/* CD-ROM drive model */
	char		revnum[5];		/* CD-ROM firmware revision */
} curstat_t;


/* Default maximum number of CDDB directories */
#define MAX_DBDIRS		20

/* Default Send CDDB site and command */
#define CDDB_MAILSITE		"xmcd-cddb@amb.org"
#define CDDB_MAILCMD		"mailx -s '%s' %s < %s"

/* Default message strings */
#define MAIN_TITLE		"CD Audio Player"
#define STR_DBMODE		"cddb"
#define STR_PROGMODE		"prog"
#define STR_ELAPSE		"elapse"
#define STR_REMAIN_TRK		"r-trac"
#define STR_REMAIN_DISC		"r-disc"
#define STR_PLAY		"play"
#define STR_PAUSE		"pause"
#define STR_READY		"ready"
#define STR_SAMPLE		"sample"
#define STR_USAGE		"Usage:"
#define STR_BADOPTS		"The following options are unrecognized:"
#define STR_NODISC		"no disc"
#define STR_BUSY		"cd busy"
#define STR_UNKNDISC		"unknown disc title"
#define STR_UNKNTRK		"unknown track title"
#define STR_DATA		"data"
#define STR_WARNING		"Warning"
#define STR_FATAL		"Fatal Error"
#define STR_CONFIRM		"Confirm"
#define STR_INFO		"Information"
#define STR_ABOUT		"About"
#define STR_QUIT		"Really Quit?"
#define STR_NOMEMORY		"Out of memory!"
#define STR_TMPDIRERR		"Cannot create or access directory %s!"
#define STR_LIBDIRERR		"Neither the XMcd*libdir resource nor the XMCD_LIBDIR environment is defined!"
#define STR_NOMETHOD		"Unsupported deviceInterfaceMethod parameter!"
#define STR_NOVU		"Unsupported driveVendorCode parameter!"
#define STR_NOHELP		"The help file on this topic is not installed!"
#define STR_NOLINK		"There is no likely CDDB entry to link to."
#define STR_NODB		"No CD database directory."
#define STR_NOCFG		"Cannot open configuration file \"%s\"."
#define STR_NOTROM		"Device is not a CD-ROM!"
#define STR_NOTSCSI2		"Device is not SCSI-II compliant."
#define STR_SEND		"Mail current CDDB entry to \"%s\"?"
#define STR_MAILERR		"Mail command failed."
#define STR_MODERR		"Binary permissions error."
#define STR_STATERR		"Cannot stat device \"%s\"."
#define STR_NODERR		"\"%s\" is not the appropriate device type!"
#define STR_SEQFMTERR		"Program sequence string format error."
#define STR_DBDIRSERR		"maxDbdirs parameter error."
#define STR_RECOVERR		"Recovering from audio playback error..."
#define STR_MAXERR		"Too many errors."
#define STR_SAVERR_FORK		"File not saved:\nCannot fork. (errno %d)"
#define STR_SAVERR_SUID		"File not saved:\nCannot setuid to %d."
#define STR_SAVERR_OPEN		"File not saved:\nCannot open file for writing."
#define STR_SAVERR_CLOSE	"File not saved:\nCannot save file."
#define STR_SAVERR_KILLED	"File not saved:\nChild killed. (signal %d)"
#define STR_LNKERR_FORK		"File not linked:\nCannot fork. (errno %d)"
#define STR_LNKERR_SUID		"File not linked:\nCannot setuid to %d."
#define STR_LNKERR_LINK		"File not linked:\nLink failed."
#define STR_LNKERR_KILLED	"File not linked:\nChild killed. (signal %d)"


/* Application resource/option data */
typedef struct {
	char		*libdir;		/* Library path */

	/*
	 * Common config parameters
	 */
	char		*device;		/* Default CD-ROM Device */
	char		*dbdir;			/* Database paths */
	int		max_dbdirs;		/* Max number of db dirs */
	char		*dbfile_mode;		/* Database file permissions */
	char		*cddb_mailsite;		/* Send CDDB mail site */
	char		*cddb_mailcmd;		/* Send CDDB mail command */
	int		stat_interval;		/* Status poll interval (ms) */
	int		ins_interval;		/* Insert poll interval (ms) */
	int		prev_threshold;		/* Previous track/index
						 * threshold (blocks)
						 */
	int		skip_blks;		/* FF/REW skip blocks */
	int		skip_pause;		/* FF/REW pause (msec) */
	int		skip_spdup;		/* FF/REW speedup count */
	int		skip_vol;		/* FF/REW percent volume */
	int		skip_minvol;		/* FF/REW minimum volume */
	int		sample_blks;		/* Sample play blocks */
	int		blinkon_interval;	/* Display blink on (ms) */
	int		blinkoff_interval;	/* Display blink off (ms) */
	bool_t		main_showfocus;		/* Highlight kbd focus? */
	bool_t		scsierr_msg;		/* Print SCSI error msg? */
	bool_t		sol2_volmgt;		/* Solaris 2.x Vol Mgr */
	bool_t		debug;			/* Verbose debug output */

	/*
	 * Device-specific config parameters
	 */

	/* Privileged */
	int		devnum;			/* Logical device number */
	int		di_method;		/* Device interface method */
	int		vendor_code;		/* Vendor command set code */
	int		base_scsivol;		/* SCSI volume value base */
	int		min_playblks;		/* Minimum play blocks */
	bool_t		play10_supp;		/* Play Audio (10) supported */
	bool_t		play12_supp;		/* Play Audio (12) supported */
	bool_t		playmsf_supp;		/* Play Audio MSF supported */
	bool_t		playti_supp;		/* Play Audio T/I supported */
	bool_t		load_supp;		/* Motorized load supported */
	bool_t		eject_supp;		/* Motorized eject supported */
	bool_t		msen_dbd;		/* Set DBD bit for msense */
	bool_t		mselvol_supp;		/* Audio vol chg supported */
	bool_t		balance_supp;		/* Indep vol chg supported */
	bool_t		chroute_supp;		/* Channel routing support */
	bool_t		pause_supp;		/* Pause/Resume supported */
	bool_t		caddylock_supp;		/* Caddy lock supported */
	bool_t		curpos_fmt;		/* Fmt 1 of RdSubch command */
	bool_t		play_notur;		/* No Tst U Rdy when playing */

	/* User-modifiable */
	int		vol_taper;		/* Volume control taper */
	int		ch_route;		/* Channel routing */
	bool_t		load_spindown;		/* Spin down disc on CD load */
	bool_t		load_play;		/* Auto play on CD load */
	bool_t		done_eject;		/* Auto eject on done */
	bool_t		exit_eject;		/* Eject disc on exit? */
	bool_t		exit_stop;		/* Stop disc on exit? */
	bool_t		eject_exit;		/* Exit upon disc eject? */
	bool_t		eject_close;		/* Close upon disc eject? */
	bool_t		caddy_lock;		/* Lock caddy on CD load? */

	/*
	 * Various application message strings
	 */
	char		*main_title;		/* Main window title */
	char		*str_dbmode;		/* cddb */
	char		*str_progmode;		/* prog */
	char		*str_elapse;		/* elapse */
	char		*str_remaintrk;		/* r-trac */
	char		*str_remaindisc;	/* r-disc */
	char		*str_play;		/* play */
	char		*str_pause;		/* pause */
	char		*str_ready;		/* ready */
	char		*str_sample;		/* sample */
	char		*str_usage;		/* Usage */
	char		*str_badopts;		/* Bad command-line options */
	char		*str_nodisc;		/* No disc */
	char		*str_busy;		/* Device busy */
	char		*str_unkndisc;		/* unknown disc title */
	char		*str_unkntrk;		/* unknown track title */
	char		*str_data;		/* Data */
	char		*str_warning;		/* Warning */
	char		*str_fatal;		/* Fatal error */
	char		*str_confirm;		/* Confirm */
	char		*str_info;		/* Information */
	char		*str_about;		/* About */
	char		*str_quit;		/* Really Quit? */
	char		*str_nomemory;		/* Out of memory */
	char		*str_tmpdirerr;		/* tempdir problem */
	char		*str_libdirerr;		/* libdir not defined */
	char		*str_nomethod;		/* Invalid di_method */
	char		*str_novu;		/* Invalid vendor code */
	char		*str_nohelp;		/* No help available on item */
	char		*str_nolink;		/* No likely CDDB link */
	char		*str_nodb;		/* No CDDB directory */
	char		*str_nocfg;		/* Can't open config file */
	char		*str_notrom;		/* Not a CD-ROM device */
	char		*str_notscsi2;		/* Not SCSI-II compliant */
	char		*str_send;		/* Send CDDB confirm msg */
	char		*str_mailerr;		/* Mail command failed */
	char		*str_moderr;		/* Binary perms error */
	char		*str_staterr;		/* Can't stat device */
	char		*str_noderr;		/* Not a character device */
	char		*str_seqfmterr;		/* Pgm sequence format err */
	char		*str_dbdirserr;		/* maxDbdirs parameter error */
	char		*str_recoverr;		/* Recovering audio play err */
	char		*str_maxerr;		/* Too many errors */
	char		*str_saverr_fork;	/* File save err: fork */
	char		*str_saverr_suid;	/* File save err: setuid */
	char		*str_saverr_open;	/* File save err: open */
	char		*str_saverr_close;	/* File save err: close */
	char		*str_saverr_killed;	/* File save err: child kill */
	char		*str_lnkerr_fork;	/* File link err: fork */
	char		*str_lnkerr_suid;	/* File link err: setuid */
	char		*str_lnkerr_link;	/* File link err: link */
	char		*str_lnkerr_killed;	/* File link err: child kill */

	/*
	 * Short-cut key definitions
	 */
	char		*btnlbl_key;		/* Button label */
	char		*lock_key;		/* Lock */
	char		*repeat_key;		/* Repeat */
	char		*shuffle_key;		/* Shuffle */
	char		*eject_key;		/* Eject */
	char		*poweroff_key;		/* Quit */
	char		*dbprog_key;		/* Database/Program popup */
	char		*help_key;		/* Help popup */
	char		*options_key;		/* Options */
	char		*time_key;		/* Time */
	char		*ab_key;		/* A->B */
	char		*sample_key;		/* Sample */
	char		*keypad_key;		/* Keypad popup */
	char		*playpause_key;		/* Play/Pause */
	char		*stop_key;		/* Stop */
	char		*prevtrk_key;		/* Prev track */
	char		*nexttrk_key;		/* Next track */
	char		*previdx_key;		/* Prev index */
	char		*nextidx_key;		/* Next index */
	char		*rew_key;		/* REW */
	char		*ff_key;		/* FF */
	char		*keypad0_key;		/* Keypad 0 */
	char		*keypad1_key;		/* Keypad 1 */
	char		*keypad2_key;		/* Keypad 2 */
	char		*keypad3_key;		/* Keypad 3 */
	char		*keypad4_key;		/* Keypad 4 */
	char		*keypad5_key;		/* Keypad 5 */
	char		*keypad6_key;		/* Keypad 6 */
	char		*keypad7_key;		/* Keypad 7 */
	char		*keypad8_key;		/* Keypad 8 */
	char		*keypad9_key;		/* Keypad 9 */
	char		*keypadclear_key;	/* Keypad Clear */
	char		*keypadenter_key;	/* Keypad Enter */
	char		*keypadcancel_key;	/* Keypad Cancel */
} appdata_t;


/* Public functions that the application layer must export to libdi */
extern void		cd_beep(void);
extern void		cd_quit(curstat_t *);
extern long		cd_timeout(word32_t, void (*)(), byte_t *);
extern void		cd_untimeout(long);
extern void		cd_warning_popup(char *, char *);
extern void		cd_fatal_popup(char *, char *);
extern bool_t		cd_devlock(char *);
extern int		curtrk_pos(curstat_t *);
extern int		curprog_pos(curstat_t *);
extern void		cd_pause_blink(curstat_t *, bool_t);
extern void		dpy_track(curstat_t *);
extern void		dpy_index(curstat_t *);
extern void		dpy_time(curstat_t *, bool_t);
extern void		dpy_rptcnt(curstat_t *);
extern void		dpy_playmode(curstat_t *, bool_t);
extern void		dpy_all(curstat_t *);
extern void		reset_curstat(curstat_t *, bool_t);
extern void		reset_shuffle(curstat_t *);
extern void		set_lock_btn(bool_t);
extern void		set_shuffle_btn(bool_t);
extern void		set_vol_slider(int);
extern void		set_bal_slider(int);
extern int		taper_vol(int);
extern int		untaper_vol(int);
extern int		scale_vol(int);
extern int		unscale_vol(int);
extern void		dbprog_dbclear(curstat_t *s);
extern void		dbprog_dbget(curstat_t *s);
extern curstat_t	*curstat_addr(void);


#endif	/* __APPENV_H__ */

