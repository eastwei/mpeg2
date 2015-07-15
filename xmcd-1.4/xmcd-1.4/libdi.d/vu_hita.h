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
 *   The name "Hitachi" is a trademark of Hitachi Corporation, and is
 *   used here for identification purposes only.  This software and its
 *   author are not affiliated in any way with Hitachi.
 *
 */
#ifndef __VU_HITA_H__
#define __VU_HITA_H__

#ifdef VENDOR_HITACHI

#ifndef LINT
static char *_vu_hita_h_ident_ = "@(#)vu_hita.h	5.2 94/12/28";
#endif


/* Hitachi vendor-unique commands */
#define OP_VH_AUDPLAY		0xe0	/* Hitachi play audio */
#define OP_VH_PAUSE		0xe1	/* Hitachi pause */
#define OP_VH_PLAYTRK		0xe2	/* Hitachi play audio track */
#define OP_VH_RDINFO		0xe3	/* Hitachi read disk info */
#define OP_VH_EJECT		0xe4	/* Hitachi eject */
#define OP_VH_RDSTAT		0xe5	/* Hitachi read audio status */
#define OP_VH_PWRSAVE		0xe6	/* Hitachi power save mode */
#define OP_VH_RDXINFO		0xe8	/* Hitachi read extended disk info */


/* Return data lengths */
#define SZ_VH_PAUSE		3	/* Hitachi pause data size */
#define SZ_VH_RDINFO		303	/* Hitachi disk info data size */
#define SZ_VH_TOCHDR		3	/* Hitachi disc info header size */
#define SZ_VH_TOCENT		3	/* Hitachi disc info per-track
					 * entry size
					 */
#define SZ_VH_RDSTAT		11	/* Hitachi audio status data size */
#define SZ_VH_RDXINFO		404	/* Hitachi ext disk info data size */
#define SZ_VH_XTOCHDR		4	/* Hitachi ext disc info header size */
#define SZ_VH_XTOCENT		4	/* Hitachi ext disc info per-track
					 * entry size
					 */


/* Hitachi MSF location */
typedef struct hmsf {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	min:7;			/* minute */
	unsigned int	data:1;			/* 0=audio 1=data */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	data:1;			/* 0=audio 1=data */
	unsigned int	min:7;			/* minute */
#endif	/* _BYTE_ORDER_ */
	unsigned int	sec:8;			/* second */
	unsigned int	frame:8;		/* frame */
	unsigned int	res:8;			/* reserved */
} hmsf_t;


/* Hitachi extended MSF location */
typedef struct hxmsf {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	adr:4;			/* ADR code */
        unsigned int	preemph:1;		/* preemphasis */
	unsigned int	copyallow:1;		/* digital copy allow */
	unsigned int	trktype:1;		/* 0=audio 1=data */
	unsigned int	audioch:1;		/* 0=2ch 1=4ch */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	audioch:1;		/* 0=2ch 1=4ch */
	unsigned int	trktype:1;		/* 0=audio 1=data */
	unsigned int	copyallow:1;		/* digital copy allow */
        unsigned int	preemph:1;		/* preemphasis */
	unsigned int	adr:4;			/* ADR code */
#endif	/* _BYTE_ORDER_ */
	unsigned int	min:8;			/* minute */
	unsigned int	sec:8;			/* second */
	unsigned int	frame:8;		/* frame */
} hxmsf_t;


/* Hitachi Disk Info data */
typedef struct hdiscinfo {
	unsigned int	pad0:8;			/* pad byte for alignment */
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	audio:1;		/* disc has audio tracks */
	unsigned int	data:1;			/* disc has data tracks */
	unsigned int	res:6;			/* reserved */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	res:6;			/* reserved */
	unsigned int	data:1;			/* disc has data tracks */
	unsigned int	audio:1;		/* disc has audio tracks */
#endif	/* _BYTE_ORDER_ */
	unsigned int	first_trk:8;		/* first track number */
	unsigned int	last_trk:8;		/* last track number */

	byte_t		msfdata[300];		/* Track MSF address data */
} hdiscinfo_t;


/* Hitachi Audio Status data */
typedef struct haudstat {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	playing:1;		/* Audio playing */
	unsigned int	res1:7;			/* Reserved */
	unsigned int	adr:4;			/* ADR code */
        unsigned int	preemph:1;		/* preemphasis */
	unsigned int	copyallow:1;		/* digital copy allow */
	unsigned int	trktype:1;		/* 0=audio 1=data */
	unsigned int	audioch:1;		/* 0=2ch 1=4ch */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	res1:7;			/* Reserved */
	unsigned int	playing:1;		/* Audio playing */
	unsigned int	audioch:1;		/* 0=2ch 1=4ch */
	unsigned int	trktype:1;		/* 0=audio 1=data */
	unsigned int	copyallow:1;		/* digital copy allow */
        unsigned int	preemph:1;		/* preemphasis */
	unsigned int	adr:4;			/* ADR code */
#endif	/* _BYTE_ORDER_ */
	unsigned int	trkno:8;		/* track number */
	unsigned int	res2:8;			/* reserved */

	hmsf_t		rel_addr;		/* relative address */
	hmsf_t		abs_addr;		/* absolute address */
} haudstat_t;


/* Hitachi Extended Disk Info data */
typedef struct hxdiscinfo {
	byte_t		res[2];			/* reserved */
	byte_t		first_trk;		/* first track number */
	byte_t		last_trk;		/* last track number */

	byte_t		xmsfdata[400];		/* track MSF address data */
} hxdiscinfo_t;


/* Argument for the Hitachi play audio and Hitachi play audio by track
 * commands.
 */
typedef struct haudio_arg {
	union {
#if _BYTE_ORDER_ == _L_ENDIAN_
		struct {
			byte_t	res;		/* reserved */
			byte_t	frame;		/* frame */
			byte_t	sec;		/* seconds */
			byte_t	min;		/* minutes */
		} startmsf;			/* OP_VH_AUDPLAY start */
		struct {
			byte_t	frame;		/* frame */
			byte_t	sec;		/* seconds */
			byte_t	min;		/* minutes */
			byte_t	res;		/* reserved */
		} endmsf;			/* OP_VH_AUDPLAY end */
		struct {
			byte_t	res[3];		/* reserved */
			byte_t	track;		/* track number */
		} starttrk;			/* OP_VH_PLAYTRK start */
		struct {
			byte_t	res1[2];	/* reserved */
			byte_t	track;		/* track number */
			byte_t	res2;		/* reserved */
		} endtrk;			/* OP_VH_PLAYTRK end */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
		struct {
			byte_t	min;		/* minutes */
			byte_t	sec;		/* seconds */
			byte_t	frame;		/* frame */
			byte_t	res;		/* reserved */
		} startmsf;			/* OP_VH_AUDPLAY start */
		struct {
			byte_t	res;		/* reserved */
			byte_t	min;		/* minutes */
			byte_t	sec;		/* seconds */
			byte_t	frame;		/* frame */
		} endmsf;			/* OP_VH_AUDPLAY end */
		struct {
			byte_t	track;		/* track number */
			byte_t	res[3];		/* reserved */
		} starttrk;			/* OP_VH_PLAYTRK start */
		struct {
			byte_t	res2;		/* reserved */
			byte_t	track;		/* track number */
			byte_t	res1[2];	/* reserved */
		} endtrk;			/* OP_VH_PLAYTRK end */
#endif	/* _BYTE_ORDER_ */
	} _addr;
} haudio_arg_t;

#define addr_smin	_addr.startmsf.min
#define addr_ssec	_addr.startmsf.sec
#define addr_sframe	_addr.startmsf.frame
#define addr_emin	_addr.endmsf.min
#define addr_esec	_addr.endmsf.sec
#define addr_eframe	_addr.endmsf.frame
#define addr_strk	_addr.starttrk.track
#define addr_etrk	_addr.endtrk.track


/* Public function prototypes */
extern bool_t	hita_playaudio(byte_t, word32_t, word32_t, msf_t *, msf_t *,
			byte_t, byte_t);
extern bool_t	hita_pause_resume(bool_t);
extern bool_t	hita_start_stop(bool_t, bool_t);
extern bool_t	hita_get_playstatus(curstat_t *, byte_t *);
extern bool_t	hita_get_toc(curstat_t *);
extern bool_t	hita_mute(bool_t);
extern bool_t	hita_eject(void);
extern void	hita_init(void);

#else

#define hita_init	NULL

#endif	/* VENDOR_HITACHI */

#endif	/* __VU_HITA_H__ */

