/*
 *   wm2xmcd - Workman-to-Xmcd CD database file converter
 *
 *   Usage: wm2xmcd [-d outdir] [-c] [-vn] dbfile ...
 *
 *   Copyright (C) 1995  Ti Kan
 *   E-mail: ti@amb.org
 *
 *   Note: Workman is a CD audio player utility for the XView/Open Look
 *	   environment, written by Steve Grimm.  All inquiries about
 *	   Workman should be directed to koreth@hyperion.com
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
static char *_wm2xmcd_c_ident_ = "@(#)wm2xmcd.c	5.2 94/12/28";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "common.d/patchlevel.h"


/* In case the OS does not define these... */
#ifndef S_ISDIR
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif


#define CDDBDIR		"cddb"
#define BUF_LEN		2048
#define STR_BUF_SZ	64
#define MAXTRACK	100
#define FRAME_PER_SEC	75
#define MIN(a,b)	((a) > (b) ? (b) : (a))

typedef struct {
	char		*name;
	unsigned long	addr;
	unsigned char	min;
	unsigned char	sec;
	unsigned char	frame;
	char		reserved;
} trk_t;

extern int	optind;
extern char	*optarg;

int		verbose = 0;
int		categorize = 0;
char		buf[BUF_LEN],
		cdname[128],
		artist[128],
		sav_artist[128],
		sav_title[128],
		*cddbdir = CDDBDIR;
trk_t		trk[MAXTRACK];
struct stat	stbuf;



/*
 * usage
 *      Print usage help information
 *
 * Args:
 *      progname - The program name string
 *
 * Return:
 *      Nothing.
 */
void
usage(char *progname)
{
	fprintf(stderr, "Usage: %s [-d outdir] [-c] [-vn] dbfile ...\n",
		progname);
}


/*
 * prg_sum
 *	Convert an integer to its text string representation, and
 *	compute its checksum.  Used by get_discid to derive the
 *	disc ID.
 *
 * Args:
 *	n - The integer value.
 *
 * Return:
 *	The integer checksum.
 */
int
prg_sum(int n)
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
 * get_discid
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
unsigned long
get_discid(int ntrks)
{
	int	i,
		t = 0,
		n = 0;

	/* For backward compatibility this algorithm must not change */
	for (i = 0; i < ntrks; i++) {
		n += prg_sum((trk[i].min * 60) + trk[i].sec);

		t += ((trk[i+1].min * 60) + trk[i+1].sec) -
		     ((trk[i].min * 60) + trk[i].sec);
	}

	return ((n % 0xff) << 24 | t << 8 | ntrks);
}


/*
 * skip_whitespace
 *	Given a character string, return a pointer to the position
 *	where the first whitespace (space or tab) occurs.  No end-of-string
 *	checking is performed.
 *
 * Args:
 *	str - The input string
 *
 * Return:
 *	Pointer to the next whitespace in str.  NULL is returned if the
 *	input string is NULL.
 */
char *
skip_whitespace(char *str)
{
	if (str == NULL)
		return NULL;

	while (*str == ' ' || *str == '\t')
		str++;

	return (str);
}


/*
 * next_slash_slash
 *	Given a character string, return a pointer to the position
 *	where the first occurrance of two slash characters appear.
 *	If the end-of-string is reached and no "//" is found, NULL
 *	is returned.
 *
 * Args:
 *	str - The input string
 *
 * Return:
 *	Pointer to the first "//" in the input string, or NULL
 *	if no "//" found.  NULL is also returned if the input string
 *	is NULL.
 */
char *
next_slash_slash(char *str)
{
	if (str == NULL)
		return NULL;

	while (*str != '\0') {
		if (*str == '/' && *(str+1) == '/')
			return (str);
		str++;	
	}

	return NULL;
}


/*
 * mystrcat
 *	String concatenation routine with special handling for the "//"
 *	pattern.
 *
 * Args:
 *	s1 - Input string 1 and destination string buffer
 *	s2 - Input string 2
 *
 * Return:
 *	Pointer to the destination string buffer
 */
char *
mystrcat(char *s1, char *s2)
{
	char	*p,
		*q;

	if (s1 == NULL || s2 == NULL)
		return NULL;

	/* This code is takes a simplistic approach to handling '//'
	 * tokens.  It does not conform strictly to the complex
	 * semantics of '//' as defined by the workmandb docs, but
	 * yields acceptable output for the most part.
	 */
	while ((p = next_slash_slash(s2)) != NULL) {
		if (p != s2) {
			*p = '\0';

			strcat(s1, s2);
			if (s1[strlen(s1)-1] != '-')
				strcat(s1, " ");

			*p = '/';
		}
		s2 = p + 2;
	}
	strcat(s1, s2);

	return (s1);
}


/*
 * xlate_trktitle
 *	Workman-to-Xmcd track title format translation routine.
 *
 * Args:
 *	trkno - The track number
 *	str - The input string
 *	aux_artist - flag to indicate artist name handling
 *	aux_title - flag to indicate track title handling
 *	lev - recursion level
 *
 * Return:
 *	Nothing.
 */
void
xlate_trktitle(int trkno, char *str, int aux_artist, int aux_title, int lev)
{
	char	tmpbuf[512],
		*p;
	int	i;

	if (trkno < 0 || str == NULL)
		/* Paranoia */
		return;

	p = str + strlen(str) - 1;
	if (*p == '\n')
		/* Eat newline */
		*p = '\0';

	if (verbose > 2) {
		if (trkno == 0 && lev == 0)
			fprintf(stderr,
				"-----------------------------------\n");
		fprintf(stderr, "DBG ");
		for (i = 0; i < (lev * 2); i++)
			putchar(' ');
		fprintf(stderr, "trk=%d str=%s\n", trkno, str);
	}

	if (str[0] == '@') {
		if (isalpha(str[1])) {
			if ((p = next_slash_slash(&str[1])) != NULL) {
				*p = '\0';
				strcpy(sav_artist, &str[1]);
				*p = '/';
				p += 2;
			}
			else {
				strcpy(sav_artist, &str[1]);
				p = "";
			}
		}
		else if ((p = next_slash_slash(&str[1])) != NULL)
			p += 2;
		else
			p = "";

		xlate_trktitle(trkno, p, ++aux_artist, aux_title, ++lev);
		return;
	}

	if (str[0] == '+') {
		if (isalpha(str[1])) {
			p = next_slash_slash(&str[1]) + 2;

			if ((p = next_slash_slash(&str[1])) != NULL) {
				*p = '\0';
				strcpy(sav_title, &str[1]);
				*p = '/';
				p += 2;
			}
			else {
				strcpy(sav_title, &str[1]);
				p = "";
			}
		}
		else if ((p = next_slash_slash(&str[1])) != NULL)
			p += 2;
		else
			p = "";

		xlate_trktitle(trkno, p, aux_artist, ++aux_title, ++lev);
		return;
	}

	tmpbuf[0] = '\0';
	if (aux_artist && strcmp(sav_artist, artist) != 0)
		sprintf(tmpbuf, "%s%s / ", tmpbuf, sav_artist);

	if (aux_title) {
		if (*str == '\0')
			strcat(tmpbuf, sav_title);
		else
			sprintf(tmpbuf, "%s%s: ", tmpbuf, sav_title);
	}

	if (*str != '\0')
		mystrcat(tmpbuf, str);

	trk[trkno].name = (char *) malloc(strlen(tmpbuf) + 1);
	if (trk[trkno].name == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(4);
	}
	strcpy(trk[trkno].name, tmpbuf);
}


/*
 * put_dbentry
 *	Write a database field to the xmcd CD database file.
 *
 * Args:
 *	fp - Output file stream
 *	idstr - Field identifier string
 *	entry - The data string
 *
 * Return:
 *	Nothing.
 */
void
put_dbentry(FILE *fp, char *idstr, char *entry)
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
 * get_categ
 *	Prompt the user to enter a category name for a given CD,
 *	and make the appropriate directory if necessary.
 *
 * Args:
 *	dtitle - Disc title string
 *
 * Return:
 *	Pointer to the category string
 */
char *
get_categ(char *dtitle)
{
	char		path[128];
	static char	categ[128];

	while (1) {
		fprintf(stdout, "\nDisc title: %s\nEnter category name: ",
			dtitle);

		if (fgets(categ, sizeof(categ), stdin) == NULL)
			exit(0);

		categ[strlen(categ)-1] = '\0';

		sprintf(path, "%s/%s", cddbdir, categ);

		if (stat(path, &stbuf) < 0 && errno == ENOENT) {
			if (mkdir(path, 0777) < 0)
				fprintf(stderr,
					"ERROR: Cannot make directory %s\n",
					path);
			else
				break;
		}

		if (!S_ISDIR(stbuf.st_mode))
			fprintf(stderr, "%s is not a directory.\n", path);
		else
			break;
	}

	return (categ);
}


/*
 * newdb
 *	High level function to create a new xmcd CD database file for
 *	the disc being processed.
 *
 * Args:
 *	cdname - Disc title string
 *	artist - Disc artist string
 *	ntrks - The number of tracks on this disc
 *
 * Return:
 *	Nothing.
 */
void
newdb(char *cdname, char *artist, int ntrks)
{
	FILE	*fo;
	char	*categ = NULL,
		discid[9],
		outfile[128],
		tmpstr[128];
	int	i;

	sprintf(discid, "%08x", get_discid(ntrks));

	if (categorize) {
		sprintf(tmpstr, "%s / %s", artist, cdname);
		categ = get_categ(tmpstr);
		sprintf(outfile, "%s/%s/%s", cddbdir, categ, discid);
	}
	else
		sprintf(outfile, "%s/%s", cddbdir, discid);

	errno = 0;
	if (stat(outfile, &stbuf) < 0 && errno != ENOENT) {
		fprintf(stderr, "ERROR: Cannot stat %s (errno=%d)\n",
			outfile, errno);
		return;
	}
	else if (errno != ENOENT) {
		switch (stbuf.st_mode & S_IFMT) {
		case S_IFREG:
			fprintf(stdout, "%s already exists.  Remove? [yn] ",
				outfile);
			if (fgets(tmpstr, 3, stdin) == NULL)
				exit(5);
			if (tmpstr[0] == 'y' || tmpstr[0] == 'Y') {
				if (unlink(outfile) < 0) {
					fprintf(stderr,
					"ERROR: Cannot unlink %s (errno=%d)\n",
						outfile, errno);
					return;
				}
			}
			else {
				fprintf(stderr, "ERROR: %s not saved.\n",
					outfile);
				return;
			}
			break;
		default:
			fprintf(stderr, "ERROR: %s is not a regular file.",
				outfile);
			return;
		}
	}

	if (verbose > 1)
		fprintf(stderr, "discid=%s, categ=%s\n",
			discid, categorize ? categ : "(none)");

	if ((fo = fopen(outfile, "w")) == NULL) {
		fprintf(stderr, "ERROR: Cannot open %s for writing.\n",
			outfile);
		return;
	}

	fprintf(fo, "# xmcd %s CD database file\n", VERSION);
	fprintf(fo, "# Copyright (C) 1995 Ti Kan\n");
	fprintf(fo, "#\n# Track frame offsets:\n");
	for (i = 0; i < ntrks; i++)
		fprintf(fo, "#\t%u\n", trk[i].addr);
	fprintf(fo, "#\n# Disc length: %u seconds\n#\n",
		trk[ntrks].addr / FRAME_PER_SEC);

	fprintf(fo, "DISCID=%s\n", discid);

	sprintf(tmpstr, "%s / %s", artist, cdname);
	put_dbentry(fo, "DTITLE", tmpstr);

	for (i = 0; i < ntrks; i++) {
		sprintf(tmpstr, "TTITLE%u", i);
		put_dbentry(fo, tmpstr, trk[i].name);
	}

	put_dbentry(fo, "EXTD", NULL);

	for (i = 0; i < ntrks; i++) {
		sprintf(tmpstr, "EXTT%u", i);
		put_dbentry(fo, tmpstr, NULL);
	}

	put_dbentry(fo, "PLAYORDER", NULL);

	fclose(fo);
}


/*
 * do_conv
 *	High level function to convert a Workman database file into
 *	Xmcd database files.  Interprets Workman file keywords and
 *	perform appropriate translations.
 *
 * Args:
 *	fp - The Workman database input file stream
 *
 * Return:
 *	Nothing.
 */
void
do_conv(FILE *fp)
{
	char	*p;
	int	i,
		ntrks,
		trkno = -1,
		num;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		/* Comment */
		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		/* tracks */
		if (strncmp(buf, "tracks", 6) == 0) {
			trkno = 0;

			p = skip_whitespace(buf + 6);
			if (sscanf(p, "%d ", &ntrks) < 0) {
				/* File format error */
				trkno = -1;
				continue;
			}

			p = strchr(p, ' ');

			for (i = 0; i < ntrks; i++) {
				if (sscanf(p, " %d", &num) > 0) {
					trk[i].addr = num;
					trk[i].min =
						(num / FRAME_PER_SEC) / 60;
					trk[i].sec =
						(num / FRAME_PER_SEC) % 60;
					trk[i].frame = num % FRAME_PER_SEC;

					p = strchr(p+1, ' ');
				}
				else {
					trkno = -1;
					break;
				}
			}

			if (trkno < 0)
				continue;

			num = 0;
			if (sscanf(p, " %d", &num) > 0) {
				trk[i].addr = num * FRAME_PER_SEC;
				trk[i].min = num / 60;
				trk[i].sec = num % 60;
				trk[i].frame = 0;
			}

			if (num == 0)
				trkno = -1;

			cdname[0] = artist[0] = '\0';
			sav_artist[0] = sav_title[0] = '\0';

			continue;
		}

		/* cdname */
		if (trkno >= 0 && strncmp(buf, "cdname", 6) == 0) {
			strcpy(cdname, skip_whitespace(buf + 6));
			cdname[strlen(cdname)-1] = '\0';
			continue;
		}

		/* artist */
		if (trkno >= 0 && strncmp(buf, "artist", 6) == 0) {
			strcpy(artist, skip_whitespace(buf + 6));
			artist[strlen(artist)-1] = '\0';
			continue;
		}

		/* track */
		if (trkno >= 0 && strncmp(buf, "track", 5) == 0) {
			xlate_trktitle(trkno, skip_whitespace(buf + 5),
				       0, 0, 0);

			if (trkno >= ntrks - 1 &&
			    artist[0] != '\0' &&
			    cdname[0] != '\0') {
				if (verbose > 1) {
					fprintf(stderr, "%s\n",
					"-----------------------------------");
				}

				newdb(cdname, artist, ntrks);

				if (verbose)
					fprintf(stderr, "%s / %s\n",
						artist, cdname);

				for (i = 0; i < ntrks; i++) {
					if (verbose > 1) {
						fprintf(stderr,
							"%02u %02u:%02u %s\n", 
							i,
							trk[i].min,
							trk[i].sec,
							trk[i].name);
					}

					if (trk[i].name != NULL) {
						free(trk[i].name);
						trk[i].name = NULL;
					}
				}
			}

			trkno++;
			continue;
		}

		/* The following keywords are ignored.  They are
		 * irrelevant in the scope of converting to an xmcd
		 * cddb file.
		 *
		 * whendone
		 * playnew
		 * sections
		 * playmode
		 * autoplay
		 * cdvolume
		 * playlist
		 * continue
		 * dontplay
		 * volume
		 * mark
		 */
	}
}


/*
 * main
 *      The main function
 */
void
main(int argc, char **argv)
{
	FILE	*fp;
	int	i,
		c;

	while ((c = getopt(argc, argv, "cd:v:")) != EOF) {
		switch (c) {
		case 'd':
			cddbdir = optarg;
			break;
		case 'c':
			categorize = 1;
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		case '?':
			usage(argv[0]);
			exit(1);
		default:
			fprintf(stderr, "%s: invalid option -%c\n",
				argv[0], c);
			usage(argv[0]);
			exit(1);
		}
	}

	if (optind == argc) {
		usage(argv[0]);
		exit(1);
	}

	/* Make output directory */
	if (mkdir(cddbdir, 0777) < 0 && errno != EEXIST) {
		fprintf(stderr, "ERROR: Cannot make output directory %s\n",
			cddbdir);
		exit(2);
	}

	/* Initialize */
	for (i = 0; i < MAXTRACK; i++)
		trk[i].name = NULL;

	/* Process each input file */
	for (i = optind; i < argc; i++) {
		if ((fp = fopen(argv[i], "r")) == NULL) {
			fprintf(stderr, "%s: Cannot open %s\n",
				argv[0], argv[i]);
			exit(3);
		}

		do_conv(fp);
		fclose(fp);
	}

	exit(0);
}

