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
#ifndef __OS_CDSIM_H__
#define __OS_CDSIM_H__

#if defined(DI_SCSIPT) && defined(DEMO_ONLY)

#ifndef LINT
static char *_os_cdsim_h_ident_ = "@(#)os_cdsim.h	5.4 94/12/28";
#endif


#define CDSIM_MAGIC	0x584d4344	/* Magic number */
#define MAX_DATALEN	1000		/* Max I/O data length */

/* Return status codes */
#define CDSIM_COMPOK	0		/* Command completed OK */
#define CDSIM_COMPERR	1		/* Command completed with error */
#define CDSIM_NOTSUPP	2		/* Command not supported */
#define CDSIM_PARMERR	3		/* Command parameter error */
#define CDSIM_PKTERR	4		/* Command packet error */


/* CD simulator IPC packet structure (1024 bytes in size) */
typedef struct simpkt {
	word32_t	magic;		/* Magic number */

	byte_t		pktid;		/* Packet id */
	byte_t		retcode;	/* Return code */
	byte_t		dir;		/* Data direction: READ_OP/WRITE_OP */
	byte_t		cdbsz;		/* CDB size */

	word32_t	len;		/* Data length in bytes */

	byte_t		cdb[12];	/* CDB data */

	byte_t		data[MAX_DATALEN];
					/* I/O Data */
} simpkt_t;


#define CDSIM_NODISC	0x00		/* CD simulator no disc */
#define CDSIM_STOPPED	0x01		/* CD simulator stopped */
#define CDSIM_PAUSED	0x02		/* CD simulator paused */
#define CDSIM_PLAYING	0x03		/* CD simulator playing audio */

#define CDSIM_NTRKS	10		/* Number of tracks on simulated CD */
#define CDSIM_NIDXS	4		/* Number of index/track on sim CD */
#define CDSIM_TRKLEN	4500		/* Length of each simulated track */
#define CDSIM_IDXLEN	1125		/* Length of each simulated track */

/* CD simulator internal status structures */

typedef struct trkstat {
	word32_t	addr;		/* Starting absolute addr of track */
	byte_t		nidxs;		/* Number of indices */
	byte_t		rsvd[3];	/* Reserved */
	word32_t	iaddr[CDSIM_NIDXS];
					/* Index absolute addresses */
} trkstat_t;

typedef struct simstat {
	byte_t		status;		/* Current mode status flag */
	byte_t		ntrks;		/* Number of tracks */
	byte_t		trkno;		/* Current track */
	byte_t		idxno;		/* Current index */

	word32_t	absaddr;	/* Current absolute address */
	word32_t	reladdr;	/* Current relative address */
	trkstat_t	trk[MAXTRACK];	/* Per-track information */
	word32_t	startaddr;	/* Start play address */
	word32_t	endaddr;	/* End play address */

	bool_t		caddylock;	/* Caddy locked */
} simstat_t;


#define CDSIM_PKTSZ	sizeof(simpkt_t)
#define CDSIM_INQSZ	sizeof(inquiry_data_t)


/* Public function prototypes */
extern bool_t	cdsim_sendpkt(char *, int, simpkt_t *);
extern bool_t	cdsim_getpkt(char *, int, simpkt_t *);
extern void	cdsim_main(void);

#endif	/* DI_SCSIPT DEMO_ONLY */

#endif	/* __OS_CDSIM_H__ */

