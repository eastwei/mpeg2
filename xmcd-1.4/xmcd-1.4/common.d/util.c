/*
 *   util.c - Common utility routines for xmcd, cda and libdi.
 *
 *   xmcd  - Motif(tm) CD Audio Player
 *   cda   - Command-line CD Audio Player
 *   libdi - CD Audio Player Device Interface Library
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
#ifndef LINT
static char *_util_c_ident_ = "@(#)util.c	5.3 94/12/28";
#endif

#include "common.d/appenv.h"
#include "common.d/util.h"

extern char		*progname;
extern appdata_t	app_data;
extern FILE		*errfp;

STATIC uid_t		ouid = 30001;	/* Default to something safe */
STATIC gid_t		ogid = 30001;	/* Default to something safe */


/***********************
 *   public routines   *
 ***********************/


/*
 * util_init
 *	Initialize the libutil module.  This should be called before
 *	the calling program does a setuid.
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	Nothing
 */
void
util_init(void)
{
	ouid = getuid();
	ogid = getgid();
}


/*
 * get_ouid
 *	Get original user ID
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	Original uid value.
 */
uid_t
get_ouid(void)
{
	return (ouid);
}


/*
 * get_ogid
 *	Get original group ID
 *
 * Args:
 *	Nothing
 *
 * Return:
 *	Original gid value.
 */
gid_t
get_ogid(void)
{
	return (ogid);
}


/*
 * ltobcd
 *	32-bit integer to BCD conversion routine
 *
 * Args:
 *	n - 32-bit integer
 *
 * Return:
 *	BCD representation of n
 */
sword32_t
ltobcd(sword32_t n)
{
	return ((n % 10) | ((n / 10) << 4));
}


/*
 * bcdtol
 *	BCD to 32-bit integer conversion routine
 *
 * Args:
 *	n - BCD value
 *
 * Return:
 *	integer representation of n
 */
sword32_t
bcdtol(sword32_t n)
{
	return ((n & 0x0f) + ((n >> 4) * 10));
}


/*
 * stob
 *	String to boolean conversion routine
 *
 * Args:
 *	s - text string "True", "true", "False" or "false"
 *
 * Return:
 *	Boolean value representing the string
 */
bool_t
stob(char *s)
{
	if (strcmp(s, "True") == 0 || strcmp(s, "true") == 0 ||
	    strcmp(s, "TRUE") == 0)
		return TRUE;

	return FALSE;
}


/*
 * basename
 *	Return the basename of a file path
 *
 * Args:
 *	path - The file path string
 *
 * Return:
 *	The basename string
 */
char *
basename(char *path)
{
	char	*p;

	if ((p = strrchr(path, '/')) == NULL)
		return (path);
	
	return (p + 1);
}


/*
 * dirname
 *	Return the dirname of a file path
 *
 * Args:
 *	path - The file path string
 *
 * Return:
 *	The dirname string
 */
char *
dirname(char *path)
{
	char		*p;
	static char	buf[FILE_PATH_SZ];

	if ((int) strlen(path) >= FILE_PATH_SZ)
		/* Error: path name too long */
		return NULL;

	strcpy(buf, path);

	if ((p = strrchr(buf, '/')) == NULL)
		return (buf);
	
	*p = '\0';
	return (buf);
}


/*
 * homedir
 *	Return the home directory path of a user given the uid
 *
 * Args:
 *	uid - The uid of the user
 *
 * Return:
 *	The home directory path name string
 */
char *
homedir(uid_t uid)
{
	struct passwd	*pw;
	char		*cp;

	/* Get home directory from the password file if possible */
	if ((pw = getpwuid(uid)) != NULL)
		return (pw->pw_dir);

	/* Try the HOME environment variable */
	if (uid == ouid && (cp = getenv("HOME")) != NULL)
		return (cp);

	/* If we still can't get the home directory, just set it to the
	 * current directory (shrug).
	 */
	return (".");
}


/*
 * uhomedir
 *	Return the home directory path of a user given the name
 *
 * Args:
 *	name - The name of the user
 *
 * Return:
 *	The home directory path name string
 */
char *
uhomedir(char *name)
{
	struct passwd	*pw;
	char		*cp;

	/* Get home directory from the password file if possible */
	if ((pw = getpwnam(name)) != NULL)
		return (pw->pw_dir);

	/* If we still can't get the home directory, just set it to the
	 * current directory (shrug).
	 */
	return (".");
}


/*
 * isqrt
 *	Fast integer-based square root routine
 *
 * Args:
 *	n - The integer value whose square-root is to be taken
 *
 * Return:
 *	Resultant square-root integer value
 */
int
isqrt(int n)
{
	int	a, b, c, as, bs;

	a = 1;
	b = 1;
	while (a <= n) {
		a = a << 2;
		b = b << 1;
	}
	as = 0;
	bs = 0;
	while (b > 1 && n > 0) {
		a = a >> 2;
		b = b >> 1;
		c = n - (as | a);
		if (c >= 0) {
			n = c;
			as |= (a << 1);
			bs |= b;
		}
		as >>= 1;
	}

	return (bs);
}


/*
 * blktomsf
 *	CD logical block to MSF conversion routine
 *
 * Args:
 *	blk - The logical block address
 *	ret_min - Minute (return)
 *	ret_sec - Second (return)
 *	ret_frame - Frame (return)
 *	offset - Additional logical block address offset
 *
 * Return:
 *	Nothing.
 */
void
blktomsf(word32_t blk, byte_t *ret_min, byte_t *ret_sec, byte_t *ret_frame,
	 word32_t offset)
{
	*ret_min = (blk + offset) / FRAME_PER_SEC / 60;
	*ret_sec = ((blk + offset) / FRAME_PER_SEC) % 60;
	*ret_frame = (blk + offset) % FRAME_PER_SEC;
}


/*
 * msftoblk
 *	CD MSF to logical block conversion routine
 *
 * Args:
 *	min - Minute
 *	sec - Second
 *	frame - Frame
 *	ret_blk - The logical block address (return)
 *	offset - Additional logical block address offset
 *
 * Return:
 *	Nothing.
 */
void
msftoblk(byte_t min, byte_t sec, byte_t frame, word32_t *ret_blk,
	 word32_t offset)
{
	*ret_blk = FRAME_PER_SEC * (min * 60 + sec) + frame - offset;
}


/*
 * bswap16
 *	16-bit little-endian to big-endian byte-swap routine.
 *	On a big-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word16_t
bswap16(word16_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	word16_t	ret;

	ret  = (x & 0x00ff) << 8;
	ret |= (word16_t) (x & 0xff00) >> 8;
	return (ret);
#else
	return (x);
#endif
}


/*
 * bswap24
 *	24-bit little-endian to big-endian byte-swap routine.
 *	On a big-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word32_t
bswap24(word32_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	word32_t	ret;

	ret  = (x & 0x0000ff) << 16;
	ret |= (x & 0x00ff00);
	ret |= (x & 0xff0000) >> 16;
	return (ret);
#else
	return (x);
#endif
}


/*
 * bswap32
 *	32-bit little-endian to big-endian byte-swap routine.
 *	On a big-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word32_t
bswap32(word32_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	word32_t	ret;

	ret  = (x & 0x000000ff) << 24;
	ret |= (x & 0x0000ff00) << 8;
	ret |= (x & 0x00ff0000) >> 8;
	ret |= (x & 0xff000000) >> 24;
	return (ret);
#else
	return (x);
#endif
}


/*
 * lswap16
 *	16-bit big-endian to little-endian byte-swap routine.
 *	On a little-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word16_t
lswap16(word16_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	return (x);
#else
	word16_t	ret;

	ret  = (x & 0x00ff) << 8;
	ret |= (word16_t) (x & 0xff00) >> 8;
	return (ret);
#endif
}


/*
 * lswap24
 *	24-bit big-endian to little-endian byte-swap routine.
 *	On a little-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word32_t
lswap24(word32_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	return (x);
#else
	word32_t	ret;

	ret  = (x & 0x0000ff) << 16;
	ret |= (x & 0x00ff00);
	ret |= (x & 0xff0000) >> 16;
	return (ret);
#endif
}


/*
 * lswap32
 *	32-bit big-endian to little-endian byte-swap routine.
 *	On a little-endian system architecture this routines has no effect.
 *
 * Args:
 *	x - The data to be swapped
 *
 * Return:
 *	The swapped data.
 */
word32_t
lswap32(word32_t x)
{
#if _BYTE_ORDER_ == _L_ENDIAN_
	return (x);
#else
	word32_t	ret;

	ret  = (x & 0x000000ff) << 24;
	ret |= (x & 0x0000ff00) << 8;
	ret |= (x & 0x00ff0000) >> 8;
	ret |= (x & 0xff000000) >> 24;
	return (ret);
#endif
}



/*
 * dbgdump
 *	Dump a data buffer to screen.
 *
 * Args:
 *	title - Message banner
 *	data - Address of data
 *	len - Number of bytes to dump
 *
 * Return:
 *	Nothing.
 */
void
dbgdump(char *title, byte_t *data, int len)
{
	int	i, j, k, n,
		lines;

	if (title == NULL || data == NULL || len <= 0)
		return;

	fprintf(errfp, "\n%s:", title);

	lines = ((len - 1) / 16) + 1;

	for (i = 0, k = 0; i < lines; i++) {
		fprintf(errfp, "\n%04x    ", k);

		for (j = 0, n = k; j < 16; j++, k++) {
			if (k < len)
				fprintf(errfp, "%02x ", *(data + k));
			else
				fprintf(errfp, "-- ");

			if (j == 7)
				fprintf(errfp, " ");
		}

		fprintf(errfp, "   ");

		for (j = 0, k = n; j < 16; j++, k++) {
			if (k < len) {
				fprintf(errfp, "%c",
				    isprint(*(data + k)) ? *(data + k) : '.'
				);
			}
			else
				fprintf(errfp, ".");
		}
	}

	fprintf(errfp, "\n");
}


