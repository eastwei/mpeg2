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
#ifndef LINT
static char *_os_cdsim_c_ident_ = "@(#)os_cdsim.c	5.3 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "libdi.d/libdi.h"
#include "libdi.d/scsipt.h"


#ifdef DEMO_ONLY

#ifndef CDSIM_VERS
#define CDSIM_VERS		"1.01"	/* CD-ROM simulator version */
#endif

extern appdata_t		app_data;
extern int			cdsim_sfd[],
				cdsim_rfd[];
extern FILE			*errfp;

STATIC simstat_t		cdsim_stat;
STATIC time_t			cdsim_start_time = 0,
				cdsim_pause_time = 0,
				cdsim_pause_elapsed = 0,
				cdsim_prev_pause = 0,
				cdsim_elapsed = 0,
				cdsim_now;

STATIC inquiry_data_t		cdsim_inqdata;
STATIC byte_t			cdsim_tocdata1[SZ_RDTOC],
				cdsim_tocdata2[SZ_RDTOC];



/*
 * cdsim_sendpkt
 *	Write a CD simulator packet down the pipe
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
bool_t
cdsim_sendpkt(char *name, int fd, simpkt_t *s)
{
	byte_t	*p = (byte_t *) s;
	int	i,
		ret;

	if (fd < 0)
		return FALSE;

	/* Brand packet with magic number */
	s->magic = CDSIM_MAGIC;

	/* Send a packet */
	i = CDSIM_PKTSZ;
	while ((ret = write(fd, p, i)) < i) {
		if (ret < 0 && errno != EBADF) {
			fprintf(errfp, "%s: packet write error (errno=%d)\n",
				name, errno);
			return FALSE;
		}

		if (ret == 0) {
			/* avoid hogging CPU */
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
 * cdsim_getpkt
 *	Read a CD simulator packet from the pipe
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
bool_t
cdsim_getpkt(char *name, int fd, simpkt_t *r)
{
	byte_t	*p = (byte_t *) r;
	int	i,
		ret;

	if (fd < 0)
		return FALSE;

	/* Get a packet */
	i = CDSIM_PKTSZ;
	while ((ret = read(fd, p, i)) < i) {
		if (ret < 0 && errno != EBADF) {
			fprintf(errfp, "%s: packet read error (errno=%d)\n",
				name, errno);
			return FALSE;
		}

		if (ret == 0) {
			/* avoid hogging CPU */
			sleep(1);
		}
		else {
			i -= ret;
			p += ret;
		}
	}

	/* Check packet for magic number */
	if (r->magic != CDSIM_MAGIC) {
		fprintf(errfp, "%s: bad packet magic number.\n", name);
		return FALSE;
	}

	return TRUE;
}


/*
 * cdsim_sig
 *	CD simulator process signal handler
 *
 * Args:
 *	sig - The signal number
 *
 * Return:
 *	Nothing.
 */
/*ARGSUSED*/
STATIC void
cdsim_sig(int sig)
{
	fprintf(errfp, "CD-ROM simulator exiting.\n");
	exit(0);
}


/*
 * cdsim_s_test
 *	Test Unit Ready command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC word32_t
cdsim_s_test(simpkt_t *r, simpkt_t *s)
{
	if (cdsim_stat.status == CDSIM_NODISC)
		return CDSIM_COMPERR;
	else
		return CDSIM_COMPOK;
}


/*
 * cdsim_s_inquir
 *	Inquiry command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
STATIC byte_t
cdsim_s_inquir(simpkt_t *r, simpkt_t *s)
{
	s->len = (r->len > CDSIM_INQSZ) ? CDSIM_INQSZ : r->len;

	/* Copy inquiry data into packet */
	memcpy(s->data, (byte_t *) &cdsim_inqdata, s->len);

	return CDSIM_COMPOK;
}


/*
 * cdsim_s_mselect
 *	Mode Select command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_s_mselect(simpkt_t *r, simpkt_t *s)
{
	return CDSIM_COMPOK;
}


/*
 * cdsim_s_msense
 *	Mode Sense command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_s_msense(simpkt_t *r, simpkt_t *s)
{
	return CDSIM_COMPOK;
}


/*
 * cdsim_s_start
 *	Start/Stop Unit command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_s_start(simpkt_t *r, simpkt_t *s)
{
	cdsim_start_time = 0;
	cdsim_elapsed = 0;
	cdsim_pause_time = 0;
	cdsim_pause_elapsed = 0;
	cdsim_prev_pause = 0;

	if (r->cdb[4] & 0x01) {
		/* Start unit */
		if (r->cdb[4] & 0x02) {
			/* Load disc */
			cdsim_stat.status = CDSIM_STOPPED;
			return CDSIM_COMPOK;
		}
		else if (cdsim_stat.status == CDSIM_NODISC)
			return CDSIM_COMPERR;
	}
	else {
		/* Stop unit */
		if (cdsim_stat.status == CDSIM_NODISC)
			return CDSIM_COMPERR;
		else if (r->cdb[4] & 0x02) {
			/* Eject disc */
			if (cdsim_stat.caddylock)
				return CDSIM_COMPERR;
			else {
				cdsim_stat.status = CDSIM_NODISC;
				return CDSIM_COMPOK;
			}
		}
		else {
			/* Stop disc */
			cdsim_stat.status = CDSIM_STOPPED;
			return CDSIM_COMPOK;
		}
	}
	return CDSIM_COMPOK;
}


/*
 * cdsim_s_prevent
 *	Prevent/Allow Medium Removal command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_s_prevent(simpkt_t *r, simpkt_t *s)
{
	if (r->cdb[4] & 0x01)
		cdsim_stat.caddylock = TRUE;
	else
		cdsim_stat.caddylock = FALSE;

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_rdsubq
 *	Read Subchannel command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
STATIC byte_t
cdsim_m_rdsubq(simpkt_t *r, simpkt_t *s)
{
	subq_hdr_t	*h = (subq_hdr_t *)(void *) s->data;
	subq_01_t	*s1 = (subq_01_t *)(void *)
			      (s->data + sizeof(subq_hdr_t));

	/* Subchannel formats */
	switch (r->cdb[3]) {
	case SUB_CURPOS:
		h->subch_len = 15;

		s1->fmt_code = SUB_CURPOS;
		s1->preemph = 0;
		s1->copyallow = 0;
		s1->trktype = 0;
		s1->audioch = 0;
		s1->adr = 0;

		s1->trkno = cdsim_stat.trkno;
		s1->idxno = cdsim_stat.idxno;

		if (r->cdb[1] & 0x02) {
			blktomsf(
				cdsim_stat.absaddr,
				&s1->abs_addr.msf.min,
				&s1->abs_addr.msf.sec,
				&s1->abs_addr.msf.frame,
				FRAME_PER_SEC << 1
			);

			blktomsf(
				cdsim_stat.reladdr,
				&s1->rel_addr.msf.min,
				&s1->rel_addr.msf.sec,
				&s1->rel_addr.msf.frame,
				0
			);
		}
		else {
			s1->abs_addr.logical = bswap32(cdsim_stat.absaddr);
			s1->rel_addr.logical = bswap32(cdsim_stat.reladdr);
		}

		s->len = sizeof(subq_hdr_t) + sizeof(subq_01_t);

		break;

	default:
		/* The other formats are not implemented */
		return CDSIM_COMPERR;
	}

	switch (cdsim_stat.status) {
	case CDSIM_PLAYING:
		h->audio_status = AUDIO_PLAYING;
		break;

	case CDSIM_PAUSED:
		h->audio_status = AUDIO_PAUSED;
		break;

	default:
		h->audio_status = AUDIO_COMPLETED;
		break;
	}

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_rdtoc
 *	Read TOC command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
STATIC byte_t
cdsim_m_rdtoc(simpkt_t *r, simpkt_t *s)
{
	byte_t	*startoff;

	if (r->cdb[1] & 0x02)
		startoff = cdsim_tocdata2 + sizeof(toc_hdr_t);
	else
		startoff = cdsim_tocdata1 + sizeof(toc_hdr_t);

	s->len = (r->len > SZ_RDTOC) ? SZ_RDTOC : r->len;

	if (r->cdb[6] > 1) {
		int	skip;

		skip = ((int) r->cdb[6] - 1) *
			sizeof(toc_trk_descr_t);
		s->len -= skip;
		startoff += skip;
	}

	/* Copy TOC data into packet */

	/* Header info */
	memcpy(s->data, cdsim_tocdata1, sizeof(toc_hdr_t));

	/* TOC data */
	memcpy(s->data + sizeof(toc_hdr_t), startoff, s->len);

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_play
 *	Play Audio (10) command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_m_play(simpkt_t *r, simpkt_t *s)
{
	cdsim_stat.startaddr = (r->cdb[2] << 24) | (r->cdb[3] << 16) |
		(r->cdb[4] << 8) | r->cdb[5];
	cdsim_stat.endaddr = cdsim_stat.startaddr +
		((r->cdb[7] << 8) | r->cdb[8]);

	if (cdsim_stat.endaddr <= cdsim_stat.startaddr)
		return CDSIM_PARMERR;

	cdsim_start_time = cdsim_now;
	cdsim_elapsed = 0;
	cdsim_pause_time = 0;
	cdsim_pause_elapsed = 0;
	cdsim_prev_pause = 0;

	cdsim_stat.status = CDSIM_PLAYING;

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_playmsf
 *	Play Audio MSF command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_m_playmsf(simpkt_t *r, simpkt_t *s)
{
	msftoblk(r->cdb[3], r->cdb[4], r->cdb[5], &cdsim_stat.startaddr,
		 FRAME_PER_SEC << 1);
	msftoblk(r->cdb[6], r->cdb[7], r->cdb[8], &cdsim_stat.endaddr,
		 FRAME_PER_SEC << 1);

	if (cdsim_stat.endaddr <= cdsim_stat.startaddr)
		return CDSIM_PARMERR;

	cdsim_start_time = cdsim_now;
	cdsim_elapsed = 0;
	cdsim_pause_time = 0;
	cdsim_pause_elapsed = 0;
	cdsim_prev_pause = 0;

	cdsim_stat.status = CDSIM_PLAYING;

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_playti
 *	Play Audio Track/index command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_m_playti(simpkt_t *r, simpkt_t *s)
{
	int	strk = (int) r->cdb[4],
		sidx = (int) r->cdb[5],
		etrk = (int) r->cdb[7],
		eidx = (int) r->cdb[8];

	if (sidx > (int) cdsim_stat.trk[strk - 1].nidxs) {
		strk++;
		sidx = 1;
	}

	if (strk > (int) cdsim_stat.ntrks || etrk > (int) cdsim_stat.ntrks)
		return CDSIM_PARMERR;

	if (eidx > (int) cdsim_stat.trk[strk - 1].nidxs)
		eidx = (int) cdsim_stat.trk[strk - 1].nidxs;

	cdsim_stat.startaddr = cdsim_stat.trk[strk - 1].iaddr[sidx - 1];
	cdsim_stat.endaddr = cdsim_stat.trk[etrk].iaddr[eidx - 1];

	if (cdsim_stat.endaddr <= cdsim_stat.startaddr)
		return CDSIM_PARMERR;

	cdsim_start_time = cdsim_now;
	cdsim_elapsed = 0;
	cdsim_pause_time = 0;
	cdsim_pause_elapsed = 0;
	cdsim_prev_pause = 0;

	cdsim_stat.status = CDSIM_PLAYING;

	return CDSIM_COMPOK;
}


/*
 * cdsim_m_pause
 *	Pause/Resume command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_m_pause(simpkt_t *r, simpkt_t *s)
{
	if (cdsim_stat.status == CDSIM_PAUSED && r->cdb[8] & 0x01) {
		/* Resume */
		cdsim_stat.status = CDSIM_PLAYING;
		cdsim_prev_pause += cdsim_pause_elapsed;
		cdsim_pause_elapsed = 0;

		return CDSIM_COMPOK;
	}
	else {
		cdsim_stat.status = CDSIM_PAUSED;
		time(&cdsim_pause_time);
		return CDSIM_COMPOK;
	}
}


/*
 * cdsim_m_pause
 *	Play Audio (12) command simulation function
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	Command completion status code
 */
/*ARGSUSED*/
STATIC byte_t
cdsim_l_play(simpkt_t *r, simpkt_t *s)
{
	cdsim_stat.startaddr = (r->cdb[2] << 24) | (r->cdb[3] << 16) |
		(r->cdb[4] << 8) | r->cdb[5];
	cdsim_stat.endaddr = cdsim_stat.startaddr +
		((r->cdb[6] << 24) | (r->cdb[7] << 16) |
		 (r->cdb[8] << 8) | r->cdb[9]);

	if (cdsim_stat.endaddr <= cdsim_stat.startaddr)
		return CDSIM_PARMERR;

	cdsim_start_time = cdsim_now;
	cdsim_elapsed = 0;
	cdsim_pause_time = 0;
	cdsim_pause_elapsed = 0;
	cdsim_prev_pause = 0;

	cdsim_stat.status = CDSIM_PLAYING;

	return CDSIM_COMPOK;
}


/*
 * cdsim_svccmd
 *	Service a command
 *
 * Args:
 *	r - Pointer to the command packet
 *	s - Pointer to the response packet
 *
 * Return:
 *	TRUE - success
 *	FALSE - failure
 */
STATIC bool_t
cdsim_svccmd(simpkt_t *r, simpkt_t *s)
{
	memset(s, 0, CDSIM_PKTSZ);

	DBGPRN(errfp, "\ncdsim: pktid=%d cdbsz=%d len=%d dir=%d cdb=",
		r->pktid, r->cdbsz, r->len, r->dir);

	DBGDUMP("cdsim: SCSI CDB bytes", (byte_t *) r->cdb, (int) r->cdbsz);

	/* Set return packet id */
	s->pktid = r->pktid;

	/* Copy CDB */
	s->cdbsz = r->cdbsz;
	memcpy(s->cdb, r->cdb, r->cdbsz);

	/* Truncate if necessary */
	if (s->len > MAX_DATALEN)
		s->len = MAX_DATALEN;

	/* Direction flag */
	s->dir = r->dir;

	/* Interpret CDB and service the command */
	switch (r->cdb[0]) {
	case OP_S_TEST:
		/* Test unit ready */
		s->retcode = cdsim_s_test(r, s);
		break;

	case OP_S_INQUIR:
		/* Inquiry */
		s->retcode = cdsim_s_inquir(r, s);
		break;

	case OP_S_MSELECT:
		/* Mode select */
		s->retcode = cdsim_s_mselect(r, s);
		break;

	case OP_S_MSENSE:
		/* Mode sense */
		s->retcode = cdsim_s_msense(r, s);
		break;

	case OP_S_START:
		/* Start/stop unit */
		s->retcode = cdsim_s_start(r, s);
		break;

	case OP_S_PREVENT:
		/* Prevent/allow medium removal */
		s->retcode = cdsim_s_prevent(r, s);
		break;

	case OP_M_RDSUBQ:
		/* Read subchannel */
		s->retcode = cdsim_m_rdsubq(r, s);
		break;

	case OP_M_RDTOC:
		/* Read TOC */
		s->retcode = cdsim_m_rdtoc(r, s);
		break;

	case OP_M_PLAY:
		/* Play audio (10) */
		s->retcode = cdsim_m_play(r, s);
		break;

	case OP_M_PLAYMSF:
		/* Play audio MSF */
		s->retcode = cdsim_m_playmsf(r, s);
		break;

	case OP_M_PLAYTI:
		/* Play audio track/index */
		s->retcode = cdsim_m_playti(r, s);
		break;

	case OP_M_PAUSE:
		/* Pause/resume */
		s->retcode = cdsim_m_pause(r, s);
		break;

	case OP_L_PLAY:
		/* Play audio (12) */
		s->retcode = cdsim_l_play(r, s);
		break;

	default:
		/* Command not implemented */
		s->retcode = CDSIM_NOTSUPP;
		break;
	}

	return (cdsim_sendpkt("cdsim", cdsim_rfd[1], s));
}


/*
 * cdsim_init
 *	Initialize the CD-simulator subsystem
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
STATIC void
cdsim_init(void)
{
	int		i,
			j;
	toc_hdr_t	*h1 = (toc_hdr_t *)(void *) cdsim_tocdata1,
			*h2 = (toc_hdr_t *)(void *) cdsim_tocdata2;
	toc_trk_descr_t	*t1,
			*t2;

	/* Initialize internal states */
	cdsim_stat.status = CDSIM_STOPPED;
	cdsim_stat.ntrks = CDSIM_NTRKS;
	cdsim_stat.absaddr = 0;
	cdsim_stat.reladdr = 0;
	cdsim_stat.startaddr = 0;
	cdsim_stat.endaddr = 0;
	cdsim_stat.caddylock = 0;

	/* Addresses for each simulated track and index */
	for (i = 0; i < CDSIM_NTRKS; i++) {
		if (i == 0)
			cdsim_stat.trk[i].addr = 0;
		else
			cdsim_stat.trk[i].addr = (i * CDSIM_TRKLEN) - 150;

		cdsim_stat.trk[i].nidxs = CDSIM_NIDXS;

		for (j = 0; j < (int) cdsim_stat.trk[i].nidxs; j++) {
			cdsim_stat.trk[i].iaddr[j] = (j * CDSIM_IDXLEN) +
						     cdsim_stat.trk[i].addr;
		}
	}

	/* Simulated lead-out track */
	cdsim_stat.trk[i].addr = (i * CDSIM_TRKLEN) - 150;
	cdsim_stat.trk[i].nidxs = 0;
	for (j = 0; j < CDSIM_NIDXS; j++)
		cdsim_stat.trk[i].iaddr[j] = cdsim_stat.trk[i].addr;

	/* Initialize inquiry data */
	cdsim_inqdata.type = DEV_ROM;
	cdsim_inqdata.pqual = 0;
	cdsim_inqdata.qualif = 0;
	cdsim_inqdata.rmb = 1;
	cdsim_inqdata.ver = 2;
	cdsim_inqdata.len = 38;
	strncpy((char *) cdsim_inqdata.vendor, "XMCD    ", 8);
	strncpy((char *) cdsim_inqdata.prod, "CD-ROM SIMULATOR", 16);
	strncpy((char *) cdsim_inqdata.revnum, CDSIM_VERS, 4);

	/* Initialize TOC data */
	h1->data_len = h2->data_len = bswap16(
		((CDSIM_NTRKS + 1) * sizeof(toc_trk_descr_t)) + 2
	);

	h1->first_trk = h2->first_trk = 1;
	h1->last_trk = h2->last_trk = CDSIM_NTRKS;

	t1 = (toc_trk_descr_t *)(void *) (cdsim_tocdata1 + sizeof(toc_hdr_t));
	t2 = (toc_trk_descr_t *)(void *) (cdsim_tocdata2 + sizeof(toc_hdr_t));

	for (i = 0; i < CDSIM_NTRKS; i++) {
		t1->preemph = t2->preemph = 0;
		t1->copyallow = t2->copyallow = 0;
		t1->trktype = t2->trktype = 0;
		t1->audioch = t2->audioch = 0;
		t1->adr = t2->adr = 0;
		t1->trkno = t2->trkno = i + 1;
		t1->abs_addr.logical = bswap32(cdsim_stat.trk[i].addr);
		blktomsf(
			cdsim_stat.trk[i].addr,
			&t2->abs_addr.msf.min,
			&t2->abs_addr.msf.sec,
			&t2->abs_addr.msf.frame,
			FRAME_PER_SEC << 1
		);

		t1 = (toc_trk_descr_t *)
			((byte_t *)(void *) t1 + sizeof(toc_trk_descr_t));
		t2 = (toc_trk_descr_t *)
			((byte_t *)(void *) t2 + sizeof(toc_trk_descr_t));
	}

	/* Lead-out track */
	t1->preemph = t2->preemph = 0;
	t1->copyallow = t2->copyallow = 0;
	t1->trktype = t2->trktype = 0;
	t1->audioch = t2->audioch = 0;
	t1->adr = t2->adr = 0;
	t1->trkno = t2->trkno = LEAD_OUT_TRACK;
	t1->abs_addr.logical = bswap32(cdsim_stat.trk[i].addr);
	blktomsf(
		cdsim_stat.trk[i].addr,
		&t2->abs_addr.msf.min,
		&t2->abs_addr.msf.sec,
		&t2->abs_addr.msf.frame,
		FRAME_PER_SEC << 1
	);
}


/*
 * cdsim_main
 *	The CD simulator main function
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
void
cdsim_main(void)
{
	int		i,
			j;
	simpkt_t	spkt,
			rpkt;

	fprintf(errfp, "CD-ROM simulator version %s (pid=%d) starting...\n",
		CDSIM_VERS, getpid());

	/* Install signal handlers */
	signal(SIGINT, cdsim_sig);
	signal(SIGTERM, cdsim_sig);
	signal(SIGHUP, cdsim_sig);

	/* Initialize CD-ROM simulator */
	cdsim_init();

	/* Main simulation service loop */
	for (;;) {
		/* Get SCSI request */
		if (!cdsim_getpkt("cdsim", cdsim_sfd[0], &rpkt))
			continue;

		time(&cdsim_now);
		/* Update status */
		switch (cdsim_stat.status) {
		case CDSIM_PLAYING:
			cdsim_elapsed = cdsim_now - cdsim_start_time -
				cdsim_pause_elapsed - cdsim_prev_pause;

			cdsim_stat.absaddr = cdsim_elapsed * FRAME_PER_SEC +
				cdsim_stat.startaddr;

			cdsim_stat.trkno = 0;
			for (i = 0; i < (int) cdsim_stat.ntrks; i++) {
				if (cdsim_stat.trk[i].addr > cdsim_stat.absaddr)
					break;
				cdsim_stat.trkno++;
			}

			cdsim_stat.idxno = 0;
			for (j = 0; j < (int) cdsim_stat.trk[i-1].nidxs; j++) {
				if (cdsim_stat.trk[i-1].iaddr[j] >
				    cdsim_stat.absaddr)
					break;
				cdsim_stat.idxno++;
			}

			cdsim_stat.reladdr = cdsim_stat.absaddr -
				cdsim_stat.trk[i-1].addr;

			if (cdsim_stat.absaddr > cdsim_stat.endaddr) {
				cdsim_stat.status = CDSIM_STOPPED;
				cdsim_elapsed = 0;
				cdsim_start_time = 0;
				cdsim_pause_time = 0;
				cdsim_prev_pause = 0;
				cdsim_pause_elapsed = 0;
			}
			break;

		case CDSIM_PAUSED:
			cdsim_pause_elapsed = cdsim_now - cdsim_pause_time;
			break;
		}

		/* Process SCSI request */
		cdsim_svccmd(&rpkt, &spkt);
	}
}

#endif	/* DEMO_ONLY */

