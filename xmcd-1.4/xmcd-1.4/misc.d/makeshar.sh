#!/bin/sh
#
# @(#)makeshar.sh	5.2 94/12/28
#
# Make software source code release in shar format
#
# Note: This script was designed to work with the shar program
#       as provided in the "cshar" package from Rich Salz.  Other
#       shar implementations may support different command line
#       parameters which is incompatible with cshar.  This script
#       must be modified in order to be used with those.
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
BNAME=xmcdshar
MAXSZ=60000
TMP1=/tmp/dlist1.$$
TMP2=/tmp/dlist2.$$
TMP3=/tmp/prep.$$
TMP4=/tmp/flist1.$$
TMP5=/tmp/flist2.$$

trap "rm -f $TMP1 $TMP2 $TMP3 $TMP4 $TMP5" 0 1 2 3 5 15


#
# Register a list of files to be shar'ed into a single archive
#
prep_shar()
{
	SHARNUM=`expr $SHARNUM + 1`
	SHARTOT=$SHARNUM

	rm -f $TMP1 $TMP2

	for f in $*
	do
		dname=$f
		while :
		do
			dname=`dirname $dname`
			if [ $dname != . ]
			then
				$ECHO "$dname" >>$TMP1
			else
				break
			fi
		done
	done

	if [ -r $TMP1 ]
	then
		sort $TMP1 | uniq >$TMP2
		for f in `cat $TMP2`
		do
			$ECHO "$f \c" >>$TMP3
		done
	fi

	$ECHO "$*" >>$TMP3

	rm -f $TMP1 $TMP2

	$ECHO "  == Part $SHARNUM processing done ==\n"
}


#
# Create all shar files
#
do_shar()
{
	if [ ! -r $TMP3 ]
	then
		$ECHO "Error: cannot open $TMP3"
		exit 1
	fi

	$AWK '
	{
		printf("\t%s.%02d\n", bname, NR);
		cmd = sprintf("shar -n%d -e%s -o %s.%02d -t %s %s\n",
		    NR, tot, bname, NR,
		    "\"Read the README and INSTALL files for instructions.\"",
		    $0)
		system(cmd)
	}
	' tot=$SHARTOT bname=$BNAME $TMP3
}


#
# Main execution starts here
#
SHARNUM=0

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

# Use nawk instead of awk is available
if (echo "" | nawk '{ print }') >/dev/null 2>&1
then
	AWK=nawk
else
	AWK=awk
fi

# Figure out whether to use the -g option to ls
if [ `ls -ld / | wc -w` -eq 8 ]
then
	LSCMD="ls -lg"
else
	LSCMD="ls -l"
fi

CURDIR=`pwd`
if [ `basename "$CURDIR"` = misc.d ]
then
	cd ..
elif [ ! -f install.sh ]
then
	$ECHO "You must run the makeshar.sh script while in the xmcd"
	$ECHO "source code distribution top-level directory or in the"
	$ECHO "misc.d subdirectory."
	exit 1
fi

if [ ! -r $SRCLIST ]
then
	$ECHO "Error: Cannot open $SRCLIST"
	exit 2
fi

$ECHO "Xmcd/cda v$VERS source code shar format release utility"
$ECHO "\nCreating file lists.  This will take a few minutes..."

rm -f $TMP3
$AWK '!/^#/ { print $1 }' $SRCLIST >$TMP4

# Extract names of all files bigger than MAXSZ first
for i in `cat $TMP4`
do
	ssize=`$LSCMD $i | $AWK '
		BEGIN { n = 0 }
		{ n += $5 }
		END { printf("%d\n", n) }
	'`

	if [ $ssize -gt $MAXSZ ]
	then
		BIGFILES="$BIGFILES $i"
	else
		echo "$i" >>$TMP5
	fi
done

$ECHO ""


# Process all other files
while :
do
	cp $TMP5 $TMP4
	>$TMP5

	if [ ! -s $TMP4 ]
	then
		break
	fi

	ssize=0
	THISSHAR=""

	for i in `cat $TMP4` _xoxo_
	do
		if [ $i = _xoxo_ ]
		then
			prep_shar $THISSHAR
			break
		fi

		ssize=`$LSCMD $THISSHAR $i | $AWK '
			BEGIN { n = 0 }
			{ n += $5 }
			END { printf("%d\n", n) }
		'`

		if [ $ssize -le $MAXSZ ]
		then
			THISSHAR="$THISSHAR $i"
			$ECHO "  Adding $i"
		else
			echo "$i" >>$TMP5
		fi
	done
done

# Handle big files
for i in $BIGFILES
do
	$ECHO "  Adding $i"
	prep_shar $i
done

# Create the shar files
$ECHO "\nCreating shar files...\n"
do_shar

exit 0

