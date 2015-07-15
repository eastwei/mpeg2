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
#ifndef __SCSIPT_H__
#define __SCSIPT_H__

#ifdef DI_SCSIPT

#ifndef LINT
static char *_scsipt_h_ident_ = "@(#)scsipt.h	5.9 95/01/27";
#endif


#define MAX_VENDORS		7	/* Max number of vendors */

/*
 * Vendor-unique modules
 *
 * Undefine any of these (except VENDOR_SCSI2) to remove
 * vendor-unique support for a particular vendor.  Removing
 * unused vendor-unique modules can reduce the executable
 * binary size and run-time memory usage.
 */
#define VENDOR_SCSI2		0	/* SCSI-2 mode */
#define VENDOR_CHINON		1	/* Chinon vendor-unique mode */
#define VENDOR_HITACHI		2	/* Hitachi vendor-unique mode */
#define VENDOR_NEC		3	/* NEC vendor-unique mode */
#define VENDOR_PIONEER		4	/* Pioneer vendor-unique mode */
#define VENDOR_SONY		5	/* Sony vendor-unique mode */
#define VENDOR_TOSHIBA		6	/* Toshiba vendor-unique mode */


/* Data direction code */
#define READ_OP			0	/* SCSI data-in */
#define WRITE_OP		1	/* SCSI data-out */


/* Read Sub-channel audio status */
#define AUDIO_NOTVALID		0x00	/* audio status not valid */
#define AUDIO_PLAYING		0x11	/* audio play in progress */
#define AUDIO_PAUSED		0x12	/* audio play paused */
#define AUDIO_COMPLETED		0x13	/* audio play successfully completed */
#define AUDIO_FAILED		0x14	/* audio played stopped due to error */
#define AUDIO_NOSTATUS		0x15	/* no audio status */


/* SCSI command opcodes */

/* 6-byte commands */
#define OP_S_TEST		0x00	/* test unit ready */
#define OP_S_REZERO		0x01	/* rezero */
#define OP_S_RSENSE		0x03	/* request sense */
#define OP_S_SEEK		0x0b	/* seek */
#define OP_S_INQUIR		0x12	/* inquiry */
#define OP_S_MSELECT		0x15	/* mode select */
#define OP_S_MSENSE		0x1a	/* mode sense */
#define OP_S_START		0x1b	/* start/stop unit */
#define OP_S_PREVENT		0x1e	/* prevent/allow medium removal */

/* 10-byte commands */
#define OP_M_RDCAP		0x25	/* read capacity */
#define OP_M_RDSUBQ		0x42	/* read subchannel */
#define OP_M_RDTOC		0x43	/* read TOC */
#define OP_M_RDHDR		0x44	/* read header */
#define OP_M_PLAY		0x45	/* play audio */
#define OP_M_PLAYMSF		0x47	/* play audio MSF */
#define OP_M_PLAYTI		0x48	/* play audio track/index */
#define OP_M_PLAYTR		0x49	/* play audio track relative */
#define OP_M_PAUSE		0x4b	/* pause/resume */

/* 12-byte commands */
#define OP_L_PLAY		0xa5	/* play audio */
#define OP_L_PLAYTR		0xa9	/* play audio track relative */


/* Data buffer lengths */
#define SZ_RDSUBQ		48	/* max read sub-channel Q data size */
#define SZ_RDTOC		804	/* max read TOC data size */
#define SZ_TOCHDR		4	/* TOC header size */
#define SZ_TOCENT		8	/* TOC per-track entry size */
#define SZ_MSENSE		60	/* max mode sense/mode sel data size */
#define SZ_RSENSE		18	/* max request sense data size */


/* Mode sense/mode select page codes */
#define PG_ERRECOV		0x01	/* error recovery parameters page */
#define PG_DISCONN		0x02	/* disconn/conn parameters page */
#define PG_CDROMCTL		0x0d	/* cd-rom control parameters page */
#define PG_AUDIOCTL		0x0e	/* audio control parameters page */
#define PG_ALL			0x3f	/* 0x01, 0x02, 0x0d and 0x0e */


/* Read sub-channel format codes */
#define SUB_ALL			0x00	/* sub-Q channel data */
#define SUB_CURPOS		0x01	/* current CD-ROM position */
#define SUB_CATNO		0x02	/* media catalog num (UPC/bar code) */
#define SUB_ISRC		0x03	/* track ISRC code */


/* Inquiry data misc definitions */
#define DEV_ROM			0x05	/* ROM peripheral device type */
#define DEV_CONNECTED		0x00	/* peripheral qualifier */


/* The inquiry data structure */
typedef struct inqry_data {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	type:5;		/* peripheral device type */
	unsigned int	pqual:3;	/* peripheral qualifier */
	unsigned int	qualif:7;	/* device type qualifier */
	unsigned int	rmb:1;		/* removable media */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	pqual:3;	/* peripheral qualifier */
	unsigned int	type:5;		/* peripheral device type */
	unsigned int	rmb:1;		/* removable media */
	unsigned int	qualif:7;	/* device type qualifier */
#endif	/* _BYTE_ORDER_ */

	unsigned int	ver:8;		/* SCSI version */
	unsigned int	res1:8;		/* reserved */

	byte_t		len;		/* length of additional data */
	byte_t		res2[3];	/* reserved */
	byte_t		vendor[8];	/* vendor ID */
	byte_t		prod[16];	/* product ID */
	byte_t		revnum[4];	/* revision number */
} inquiry_data_t;


/* The Mode Sense/Mode Select data structures */

/* Block descriptor data structure */
typedef struct blk_desc {
	unsigned int	dens_code:8;	/* density code */
	unsigned int	num_blks:24;	/* number of blocks */

	unsigned int	res:8;		/* reserved */
	unsigned int	blk_len:24;	/* block length */
} blk_desc_t;

/* Audio-parameters page data structure */
typedef struct audio_pg {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	pg_code:6;	/* page code */
	unsigned int	res:2;		/* reserved */
	unsigned int	pg_len:8;	/* page length */
	unsigned int	res1:1;		/* reserved */
	unsigned int	sotc:1;		/* SOTC */
	unsigned int	immed:1;	/* immediate */
	unsigned int	res2:5;		/* reserved */
	unsigned int	res3:8;		/* reserved */

	unsigned int	res4:16;	/* reserved */
	unsigned int	audio_bps:16;	/* logical blocks per second */

	unsigned int	p0_ch_ctrl:4;	/* port 0 channel control */
	unsigned int	res5:4;		/* reserved */
	unsigned int	p0_vol:8;	/* port 0 volume */
	unsigned int	p1_ch_ctrl:4;	/* port 1 channel control */
	unsigned int	res6:4;		/* reserved */
	unsigned int	p1_vol:8;	/* port 1 volume */

	unsigned int	p2_ch_ctrl:4;	/* port 2 channel control */
	unsigned int	res7:4;		/* reserved */
	unsigned int	p2_vol:8;	/* port 2 volume */
	unsigned int	p3_ch_ctrl:4;	/* port 3 channel control */
	unsigned int	res8:4;		/* reserved */
	unsigned int	p3_vol:8;	/* port 3 volume */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	res:2;		/* reserved */
	unsigned int	pg_code:6;	/* page code */
	unsigned int	pg_len:8;	/* page length */
	unsigned int	res2:5;		/* reserved */
	unsigned int	immed:1;	/* immediate */
	unsigned int	sotc:1;		/* SOTC */
	unsigned int	res1:1;		/* reserved */
	unsigned int	res3:8;		/* reserved */

	unsigned int	res4:16;	/* reserved */
	unsigned int	audio_bps:16;	/* logical blocks per second */

	unsigned int	res5:4;		/* reserved */
	unsigned int	p0_ch_ctrl:4;	/* port 0 channel control */
	unsigned int	p0_vol:8;	/* port 0 volume */
	unsigned int	res6:4;		/* reserved */
	unsigned int	p1_ch_ctrl:4;	/* port 1 channel control */
	unsigned int	p1_vol:8;	/* port 1 volume */

	unsigned int	res7:4;		/* reserved */
	unsigned int	p2_ch_ctrl:4;	/* port 2 channel control */
	unsigned int	p2_vol:8;	/* port 2 volume */
	unsigned int	res8:4;		/* reserved */
	unsigned int	p3_ch_ctrl:4;	/* port 3 channel control */
	unsigned int	p3_vol:8;	/* port 3 volume */
#endif	/* _BYTE_ORDER_ */
} audio_pg_t;

/* Mode Sense/Mode Select data structure */
typedef struct mode_sense_data {
	/* mode header */
	unsigned int	data_len:8;	/* data length */
	unsigned int	medium:8;	/* medium */
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	speed:4;	/* speed */
	unsigned int	buffered:3;	/* buffered */
	unsigned int	wprot:1;	/* write protected */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	wprot:1;	/* write protected */
	unsigned int	buffered:3;	/* buffered */
	unsigned int	speed:4;	/* speed */
#endif	/* _BYTE_ORDER_ */
	unsigned int	bdescr_len:8;	/* block descriptor length */

	byte_t		data[24];	/* block desc/page desc data */
} mode_sense_data_t;


/* Request Sense data structure */
typedef struct req_sense_data {
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	errcode:7;	/* error code */
	unsigned int	valid:1;	/* valid bit */
	unsigned int	segno:8;	/* segment number */
	unsigned int	key:4;		/* sense key */
	unsigned int	res:1;		/* reserved */
	unsigned int	ili:1;		/* ILI */
	unsigned int	eom:1;		/* end-of-medium */
	unsigned int	fm:1;		/* filemark */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	valid:1;	/* valid bit */
	unsigned int	errcode:7;	/* error code */
	unsigned int	segno:8;	/* segment number */
	unsigned int	fm:1;		/* filemark */
	unsigned int	eom:1;		/* end-of-medium */
	unsigned int	ili:1;		/* ILI */
	unsigned int	res:1;		/* reserved */
	unsigned int	key:4;		/* sense key */
#endif	/* _BYTE_ORDER_ */
	unsigned int	info0:8;	/* information */

	unsigned int	info1:8;	/* information */
	unsigned int	info2:8;	/* information */
	unsigned int	info3:8;	/* information */
	unsigned int 	addl_len:8;	/* additional sense length */

	unsigned int	cmd_spec0:8;	/* command specific information */
	unsigned int	cmd_spec1:8;	/* command specific information */
	unsigned int	cmd_spec2:8;	/* command specific information */
	unsigned int	cmd_spec3:8;	/* command specific information */

	unsigned int	code:8;		/* additional sense code */
	unsigned int	qual:8;		/* additional sense code qualifier */
	unsigned int	fruc:8;		/* field replaceable unit code */
	unsigned int	key_spec0:8;	/* sense-key specific */

	unsigned int	key_spec1:8;	/* sense-key specific */
	unsigned int	key_spec2:8;	/* sense-key specific */
	unsigned int	pad1:16;	/* pad for alignment */
} req_sense_data_t;


/* Read subchannel Q data header */
typedef struct subq_hdr {
	byte_t		reserved;	/* reserved */
	byte_t		audio_status;	/* audio status */
	word16_t	subch_len;	/* subchannel data length */
} subq_hdr_t;


/* Subchannel Q data - format 01 (CD-ROM Current Position) */
typedef struct subq_01 {
	unsigned int	fmt_code:8;	/* format code */
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	preemph:1;	/* preemphasis */
	unsigned int	copyallow:1;	/* digital copy allow */
	unsigned int	trktype:1;	/* 0=audio 1=data */
	unsigned int	audioch:1;	/* 0=2ch 1=4ch */
	unsigned int	adr:4;		/* ADR */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	adr:4;		/* ADR */
	unsigned int	audioch:1;	/* 0=2ch 1=4ch */
	unsigned int	trktype:1;	/* 0=audio 1=data */
	unsigned int	copyallow:1;	/* digital copy allow */
	unsigned int	preemph:1;	/* preemphasis */
#endif	/* _BYTE_ORDER_ */
	unsigned int	trkno:8;	/* track number */
	unsigned int	idxno:8;	/* index number */

	lmsf_t		abs_addr;	/* absolute address */
	lmsf_t		rel_addr;	/* track-relative address */
} subq_01_t;


/* Read TOC command data header */
typedef struct toc_hdr {
	word16_t	data_len;	/* TOC data length */
	byte_t		first_trk;	/* first track number */
	byte_t		last_trk;	/* last track number */
} toc_hdr_t;


/* Read TOC command track descriptor */
typedef struct toc_trk_descr {
	unsigned int	res1:8;		/* reserved */
#if _BYTE_ORDER_ == _L_ENDIAN_
	unsigned int	preemph:1;	/* preemphasis */
	unsigned int	copyallow:1;	/* digital copy allow */
	unsigned int	trktype:1;	/* 0=audio 1=data */
	unsigned int	audioch:1;	/* 0=2ch 1=4ch */
	unsigned int	adr:4;		/* ADR */
#else	/* _BYTE_ORDER_ == _B_ENDIAN_ */
	unsigned int	adr:4;		/* ADR */
	unsigned int	audioch:1;	/* 0=2ch 1=4ch */
	unsigned int	trktype:1;	/* 0=audio 1=data */
	unsigned int	copyallow:1;	/* digital copy allow */
	unsigned int	preemph:1;	/* preemphasis */
#endif	/* _BYTE_ORDER_ */
	unsigned int	trkno:8;	/* track number */
	unsigned int	res2:8;		/* reserved */

	lmsf_t		abs_addr;	/* absolute address */
} toc_trk_descr_t;


/* Vendor-unique module initialization jump table */
typedef struct {
	void		(*init)(void);	/* VU init function */
} vuinit_tbl_t;


/* Vendor-unique module entry jump table */
typedef struct {
	/* Vendor name string */
	char		*vendor;

	/* Play audio function */
	bool_t		(*playaudio)(byte_t, word32_t, word32_t,
				     msf_t *, msf_t *, byte_t, byte_t);

	/* Pause/resume function */
	bool_t		(*pause_resume)(bool_t);

	/* Start/stop function */
	bool_t		(*start_stop)(bool_t, bool_t);

	/* Playback status function */
	bool_t		(*get_playstatus)(curstat_t *, byte_t *);

	/* Playback volume function */
	int		(*volume)(int, curstat_t *, bool_t);

	/* Channel routing function */
	bool_t		(*route)(curstat_t *);

	/* Playback mute function */
	bool_t		(*mute)(bool_t);

	/* Read TOC function */
	bool_t		(*get_toc)(curstat_t *);

	/* Eject function */
	bool_t		(*eject)(void);

	/* Module start function */
	void		(*start)(void);

	/* Module halt function */
	void		(*halt)(void);
} vu_tbl_t;



/***** Additional include files *****/

/* OS interface library headers */
#include "libdi.d/os_aix.h"		/* IBM AIX support header */
#include "libdi.d/os_aux.h"		/* Apple A/UX support header */
#include "libdi.d/os_dec.h"		/* DEC OSF/1 & Ultrix support header */
#include "libdi.d/os_dgux.h"		/* Data General DG/UX support header */
#include "libdi.d/os_frbsd.h"		/* FreeBSD support header */
#include "libdi.d/os_hpux.h"		/* HP-UX support header */
#include "libdi.d/os_irix.h"		/* SGI IRIX support header */
#include "libdi.d/os_linux.h"		/* Linux support header */
#include "libdi.d/os_odt.h"		/* SCO ODT support header */
#include "libdi.d/os_sun.h"		/* SunOS support header */
#include "libdi.d/os_svr4.h"		/* SVR4 support header */

/* If compiling on a non-supported OS, force demo-only mode */
#if !defined(OS_MODULE) && !defined(DEMO_ONLY)
#define DEMO_ONLY
#endif

/* If demo-only, no need to compile in the vendor-unique modules */
#ifdef DEMO_ONLY
#ifdef VENDOR_CHINON
#undef VENDOR_CHINON
#endif
#ifdef VENDOR_HITACHI
#undef VENDOR_HITACHI
#endif
#ifdef VENDOR_NEC
#undef VENDOR_NEC
#endif
#ifdef VENDOR_PIONEER
#undef VENDOR_PIONEER
#endif
#ifdef VENDOR_SONY
#undef VENDOR_SONY
#endif
#ifdef VENDOR_TOSHIBA
#undef VENDOR_TOSHIBA
#endif
#endif	/* DEMO_ONLY */

/* Demo mode support header */
#include "libdi.d/os_demo.h"

/* Vendor-unique library headers */
#include "libdi.d/vu_chin.h"		/* Chinon vendor-unique header */
#include "libdi.d/vu_hita.h"		/* Hitachi vendor-unique header */
#include "libdi.d/vu_nec.h"		/* NEC vendor-unique header */
#include "libdi.d/vu_pion.h"		/* Pioneer vendor-unique header */
#include "libdi.d/vu_sony.h"		/* Sony vendor-unique header */
#include "libdi.d/vu_tosh.h"		/* Toshiba vendor-unique header */


/*
 * Public functions
 */
extern void	scsipt_init(curstat_t *, di_tbl_t *);
extern bool_t	scsipt_check_disc(curstat_t *);
extern void	scsipt_status_upd(curstat_t *);
extern void	scsipt_lock(curstat_t *, bool_t);
extern void	scsipt_repeat(curstat_t *, bool_t);
extern void	scsipt_shuffle(curstat_t *, bool_t);
extern void	scsipt_load_eject(curstat_t *);
extern void	scsipt_ab(curstat_t *);
extern void	scsipt_sample(curstat_t *);
extern void	scsipt_level(curstat_t *, byte_t, bool_t);
extern void	scsipt_play_pause(curstat_t *);
extern void	scsipt_stop(curstat_t *, bool_t);
extern void	scsipt_prevtrk(curstat_t *);
extern void	scsipt_nexttrk(curstat_t *);
extern void	scsipt_previdx(curstat_t *);
extern void	scsipt_nextidx(curstat_t *);
extern void	scsipt_rew(curstat_t *, bool_t);
extern void	scsipt_ff(curstat_t *, bool_t);
extern void	scsipt_warp(curstat_t *);
extern void	scsipt_route(curstat_t *);
extern void	scsipt_mute_on(curstat_t *);
extern void	scsipt_mute_off(curstat_t *);
extern void	scsipt_start(curstat_t *);
extern void	scsipt_icon(curstat_t *, bool_t);
extern void	scsipt_halt(curstat_t *);
extern char	*scsipt_mode(void);
extern char	*scsipt_vers(void);

/*
 * Functions for vendor-unique module use only
 */
extern bool_t	scsipt_rdsubq(byte_t *, byte_t, byte_t, int, bool_t);
extern bool_t	scsipt_modesense(byte_t *, byte_t, byte_t);
extern bool_t	scsipt_modesel(byte_t *, byte_t);
extern bool_t	scsipt_inquiry(byte_t *, int);
extern bool_t	scsipt_rdtoc(byte_t *, bool_t, int);
extern bool_t	scsipt_tst_unit_rdy(void);
extern bool_t	scsipt_playmsf(msf_t *, msf_t *);
extern bool_t	scsipt_play10(word32_t, word32_t);
extern bool_t	scsipt_play12(word32_t, word32_t);
extern bool_t	scsipt_prev_allow(bool_t);
extern bool_t	scsipt_start_stop(bool_t, bool_t);
extern bool_t	scsipt_pause_resume(bool_t);
extern bool_t	scsipt_play_trkidx(int, int, int, int);

#else

#define scsipt_init	NULL

#endif

#endif	/* __SCSIPT_H__ */

