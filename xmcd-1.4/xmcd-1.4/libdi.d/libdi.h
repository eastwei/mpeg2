/*
 *   libdi - CD Audio Player Device Interface Library
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
#ifndef __LIBDI_H__
#define __LIBDI_H__

#ifndef LINT
static char *_libdi_h_ident_ = "@(#)libdi.h	5.8 95/01/27";
#endif

/* Max number of libdi modules */
#define MAX_METHODS	2

/*
 * Supported libdi methods
 *
 * Uncomment any of these to remove support for a method.
 * Removing unused methods can reduce the executable
 * binary size and run-time memory usage.  At least one
 * method must be defined.
 *
 * Note: If compiling for DEMO_ONLY or on a non-supported OS
 * platform, DI_SCSIPT must be defined.
 */
#define DI_SCSIPT	0	/* SCSI pass-through method */
#if defined(linux) || defined(sun)
#define DI_SLIOC	1	/* SunOS/Linux ioctl method */
#endif


/* Play audio format codes */
#define ADDR_BLK		0x01	/* block address specified */
#define ADDR_MSF		0x02	/* MSF address specified */
#define ADDR_TRKIDX		0x04	/* track/index numbers specified */
#define ADDR_OPTEND		0x80	/* End address can be ignored */


/* Slider control flags */
#define WARP_VOL        0x1             /* Set volume slider thumb */
#define WARP_BAL        0x2             /* Set balance slider thumb */


/* Misc constants */
#define MAX_SRCH_BLKS		225	/* max search play blks per sample */
#define MAX_RECOVERR		20	/* Max number of err recovery tries */
#define ERR_SKIPBLKS		10	/* Number of frame to skip on error */
#define ERR_CLRTHRESH		1500	/* If there hasn't been any errors
					 * for this many blocks of audio
					 * playback, then the previous errors
					 * count is cleared.
					 */


/* CD position MSF structure */
typedef struct msf {
	byte_t		res;		/* reserved */
	byte_t		min;		/* minutes */
	byte_t		sec;		/* seconds */
	byte_t		frame;		/* frame */
} msf_t;


/* Combined MSF and logical address union */
typedef union lmsf {
	msf_t		msf;		/* MSF address */
	word32_t	logical;	/* logical address */
} lmsf_t;


/*
 * Jump table structure for libdi interface
 */
typedef struct {
	bool_t	(*check_disc)(curstat_t *);
	void	(*status_upd)(curstat_t *);
	void	(*lock)(curstat_t *, bool_t);
	void	(*repeat)(curstat_t *, bool_t);
	void	(*shuffle)(curstat_t *, bool_t);
	void	(*load_eject)(curstat_t *);
	void	(*ab)(curstat_t *);
	void	(*sample)(curstat_t *);
	void	(*level)(curstat_t *, byte_t, bool_t);
	void	(*play_pause)(curstat_t *);
	void	(*stop)(curstat_t *, bool_t);
	void	(*prevtrk)(curstat_t *);
	void	(*nexttrk)(curstat_t *);
	void	(*previdx)(curstat_t *);
	void	(*nextidx)(curstat_t *);
	void	(*rew)(curstat_t *, bool_t);
	void	(*ff)(curstat_t *, bool_t);
	void	(*warp)(curstat_t *);
	void	(*route)(curstat_t *);
	void	(*mute_on)(curstat_t *);
	void	(*mute_off)(curstat_t *);
	void	(*start)(curstat_t *);
	void	(*icon)(curstat_t *, bool_t);
	void	(*halt)(curstat_t *);
	char *	(*mode)(void);
	char *	(*vers)(void);
} di_tbl_t;


/*
 * Jump table for libdi initialization
 */
typedef struct {
	void	(*init)(curstat_t *, di_tbl_t *);
} diinit_tbl_t;


/*
 * Public function prototypes: libdi external calling interface.
 */
extern void	di_init(curstat_t *);
extern bool_t	di_check_disc(curstat_t *);
extern void	di_status_upd(curstat_t *);
extern void	di_lock(curstat_t *, bool_t);
extern void	di_repeat(curstat_t *, bool_t);
extern void	di_shuffle(curstat_t *, bool_t);
extern void	di_load_eject(curstat_t *);
extern void	di_ab(curstat_t *);
extern void	di_sample(curstat_t *);
extern void	di_level(curstat_t *, byte_t, bool_t);
extern void	di_play_pause(curstat_t *);
extern void	di_stop(curstat_t *, bool_t);
extern void	di_prevtrk(curstat_t *);
extern void	di_nexttrk(curstat_t *);
extern void	di_previdx(curstat_t *);
extern void	di_nextidx(curstat_t *);
extern void	di_rew(curstat_t *, bool_t);
extern void	di_ff(curstat_t *, bool_t);
extern void	di_warp(curstat_t *);
extern void	di_route(curstat_t *);
extern void	di_mute_on(curstat_t *);
extern void	di_mute_off(curstat_t *);
extern void	di_start(curstat_t *);
extern void	di_icon(curstat_t *, bool_t);
extern void	di_halt(curstat_t *);
extern char	*di_mode(void);
extern char	*di_vers(void);
extern bool_t	di_isdemo(void);


#endif	/* __LIBDI_H__ */

