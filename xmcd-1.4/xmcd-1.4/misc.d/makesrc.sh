#!/bin/sh
#
# @(#)makesrc.sh	5.3 94/12/28
#
# Make software source code release
#
#    xmcd  - Motif(tm) CD Audio Player
#    cda   - Command-line CD Audio Player
#    libdi - CD Audio Player Device Interface Library
#
#    Copyright (C) 1995  Ti Kan
#    E-mail: ti@amb.org
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

SRCLIST=misc.d/SRCLIST
VERS=`grep "VERSION=" $SRCLIST | sed 's/^.*VERSION=//'`
TMPDIR=/tmp/_makesrc.d
ZFILE=xmcdsrc.tar.gz
UUEFILE=xmcdsrc.uue
COMPRESS=gzip
UNCOMPRESS=gunzip
ENCODE=uuencode
DECODE=uudecode

# Use Sysv echo if possible
if [ -x /usr/5bin/echo ]				# SunOS SysV echo
then
	ECHO=/usr/5bin/echo
elif [ -z "`(echo -e a) 2>/dev/null | fgrep e`" ]	# GNU bash, etc.
then
	ECHO="echo -e"
else							# generic SysV
	ECHO=echo
fi

CURDIR=`pwd`
if [ `basename "$CURDIR"` = misc.d ]
then
	cd ..
elif [ ! -f install.sh ]
then
	$ECHO "You must run the makesrc.sh script while in the xmcd"
	$ECHO "source code distribution top-level directory or in the"
	$ECHO "misc.d subdirectory."
	exit 1
fi

if [ ! -r $SRCLIST ]
then
	$ECHO "Error: Cannot open $SRCLIST"
	exit 2
fi

$ECHO "Creating xmcd/cda version $VERS source code release...\n"

trap "rm -rf $TMPDIR $ZFILE $UUEFILE" 1 2 3 5 15

# Make temp directory and copy release files to it
rm -rf $TMPDIR
mkdir -p $TMPDIR/xmcd-$VERS
for i in `awk '!/^#/ { print $1 }' $SRCLIST`
do
	$ECHO "\t$i"
	DEST=`dirname $TMPDIR/xmcd-$VERS/$i`
	mkdir -p $DEST >/dev/null 2>&1
	cp $i $DEST
done

$ECHO "\nFixing permissions..."
(cd $TMPDIR; find xmcd-$VERS -type f -print | xargs chmod 444)

$ECHO "\nCreating \"$COMPRESS\"ed tar archive..."
# Create tar archive and compress it
(cd $TMPDIR; tar cf - xmcd-$VERS) | $COMPRESS >$ZFILE

$ECHO "\n\"$ENCODE\"ing..."
$ECHO '

Instructions to unpack xmcd v_VERS_ source code release
----------------------------------------------------

The following is a "_COMPRESS_"ed and "_ENCODE_"ed tar archive
containing the xmcd, cda and libdi source code files.  To extract,
make a suitable source code directory for xmcd and save this
message in a file "_UUEFILE_" in that directory.  Then, change
to the directory and do the following:

	_DECODE_ _UUEFILE_
	_UNCOMPRESS_ < _ZFILE_ | tar xvf -

Read the README and INSTALL files for further instructions.

' | sed	-e "s/_VERS_/$VERS/g" \
	-e "s/_UUEFILE_/$UUEFILE/g" \
	-e "s/_ZFILE_/$ZFILE/g" \
	-e "s/_COMPRESS_/$COMPRESS/g" \
	-e "s/_UNCOMPRESS_/$UNCOMPRESS/g" \
	-e "s/_ENCODE_/$ENCODE/g" \
	-e "s/_DECODE_/$DECODE/g" >$UUEFILE

# Uuencode
$ENCODE $ZFILE $ZFILE >>$UUEFILE
$ECHO "\n\n\n" >>$UUEFILE

$ECHO ""

ls -l $ZFILE $UUEFILE

rm -rf $TMPDIR

exit 0

