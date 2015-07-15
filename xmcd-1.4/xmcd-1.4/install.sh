#!/bin/sh
#
# @(#)install.sh	5.7 95/02/06
#
# Script to install the software binary and support files on
# the target system.
#
#    xmcd  - Motif(tm) CD Audio Player
#    cda   - Command-line CD Audio Player
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

PATH=/bin:/usr/bin:/sbin:/usr/sbin:/etc:/usr/local/bin
export PATH

CATEGORIES="rock jazz blues newage classical reggae folk country soundtrack misc data"
XMCD_VER=1.4
DEMODB=4102560a
DIRPERM=755
CDIRPERM=777
SCRPERM=755
FILEPERM=444
BINPERM=711
XBINPERM=4711
OWNER=bin
GROUP=bin
BINOWNER=bin
BINGROUP=bin
XBINOWNER=root
XBINGROUP=bin
ERRFILE=/tmp/xmcd.err

#
# Utility functions
#

doexit()
{
	if [ $1 -eq 0 ]
	then
		$ECHO "\nInstallation of xmcd is now complete."
	else
		$ECHO "\nErrors have occurred in the installation."
		if [ $ERRFILE != /dev/null ]
		then
			$ECHO "See $ERRFILE for an error log."
		fi
	fi
	exit $1
}

logerr()
{
	if [ "$1" = "-p" ]
	then
		$ECHO "Error: $2"
	fi
	$ECHO "$2" >>$ERRFILE
	ERROR=1
}

getstr()
{
	$ECHO "$* \c"
	read ANS
	if [ -n "$ANS" ]
	then
		return 0
	else
		return 1
	fi
}

getyn()
{
	if [ -z "$YNDEF" ]
	then
		YNDEF=y
	fi

	while :
	do
		$ECHO "$*? [${YNDEF}] \c"
		read ANS
		if [ -n "$ANS" ]
		then
			case $ANS in
			[yY])
				RET=0
				break
				;;
			[nN])
				RET=1
				break
				;;
			*)
				$ECHO "Please answer y or n"
				;;
			esac
		else
			if [ $YNDEF = y ]
			then
				RET=0
			else
				RET=1
			fi
			break
		fi
	done

	YNDEF=
	return $RET
}

dolink()
{
	# Try symlink first
	ln -s $1 $2 2>/dev/null
	if [ $? != 0 ]
	then
		# Use hard link
		ln $1 $2 2>/dev/null
	fi
	STATUS=$?
	if [ $STATUS != 0 ]
	then
		$ECHO "Error: Cannot link $1 -> $2"
	fi
}

makedir()
{
	$ECHO "\t$1"
	if [ ! -d $1 ]
	then
		mkdir -p $1
	fi
	if [ $2 != _default_ ]
	then
		chmod $2 $1 2>/dev/null
	fi
	if [ $3 != _default_ ]
	then
		chown $3 $1 2>/dev/null
	fi
	if [ $4 != _default_ ]
	then
		chgrp $4 $1 2>/dev/null
	fi
	return 0
}

instfile()
{
	TDIR=`dirname $2`

	if [ -n "$TDIR" -a -d "$TDIR" -a -w "$TDIR" ]
	then
		if [ ! -f $1 ]
		then
			$ECHO "\t$2 NOT installed"
			logerr -n "Cannot install $2: file missing."
			return 1
		fi

		$ECHO "\t$2"
		if [ -f $2 ]
		then
			rm -f $2
		fi

		cp $1 $2
		if [ $? -ne 0 ]
		then
			logerr -n "Cannot install $2: file copy error."
			return 1
		fi

		if [ -f $2 ]
		then
			if [ $3 != _default_ ]
			then
				chmod $3 $2 2>/dev/null
			fi
			if [ $4 != _default_ ]
			then
				chown $4 $2 2>/dev/null
			fi
			if [ $5 != _default_ ]
			then
				chgrp $5 $2 2>/dev/null
			fi
		fi
		return 0
	else
		$ECHO "\t$2 NOT installed"
		logerr -n "Cannot install $2: target directory not writable."
		return 1
	fi
}


#
# Main execution starts here
#

# Use Sysv echo if possible
if [ -x /usr/5bin/echo ]
then
	ECHO=/usr/5bin/echo				# SunOS SysV echo
elif [ -z "`(echo -e a) 2>/dev/null | fgrep e`" ]
then
	ECHO="echo -e"					# GNU bash, etc.
else
	ECHO=echo					# generic SysV
fi

# Remove old error log file
ERROR=0
rm -f $ERRFILE
if [ -f $ERRFILE ]
then
	$ECHO "Cannot remove old $ERRFILE: error logging not enabled."
	ERRFILE=/dev/null
fi

# Determine whether to remove distribution files after installation
NOREMOVE=0
if [ $# -eq 1 ]
then
	# This could be combined with the above if statement with -a, but
	# FreeBSD's /bin/sh seems to dislike that.
	if [ "$1" = "-n" ]
	then
		NOREMOVE=1
	fi
elif [ -f COPYING -a -f INSTALL -a -f PORTING ]
then
	NOREMOVE=1
fi

# Check privilege
(id | fgrep 'uid=0(root)') >/dev/null 2>&1
if [ $? != 0 ]
then
	$ECHO "Warning: You should be the super user to install xmcd."

	YNDEF=n
	if getyn "Proceed anyway"
	then
		$ECHO "\nWithout super-user privilege, some files may not"
		$ECHO "be properly installed, or they may be installed with\n"
		$ECHO "incorrect permissions.\n"

		XBINPERM=711
		XBINOWNER=_default_
		BINOWNER=_default_
		OWNER=_default_
		GROUP=_default_
	else
		logerr -p "Not super user: installation aborted by user"
		doexit 1
	fi
fi

# Implement platform-specific features and deal with OS quirks
SCO=0
if (uname -X | fgrep "Release = 3.2") >/dev/null 2>&1
then
	SCO=1
fi

OS_SYS=`uname -s 2>/dev/null`
case "$OS_SYS" in
ULTRIX)
	SHELL=/bin/sh5
	;;
A/UX)
	SHELL=/bin/ksh
	;;
*)
	SHELL=/bin/sh
	;;
esac


$ECHO "Installing \"xmcd\" Motif CD Player version $XMCD_VER by Ti Kan"


# Determine BINDIR

if [ -z "$BINDIR" ]
then
	if [ -d /usr/bin/X11 ]
	then
		BINDIR=/usr/bin/X11
	elif [ -d /usr/X/bin ]
	then
		BINDIR=/usr/X/bin
	elif [ -d /usr/X11/bin ]
	then
		BINDIR=/usr/X11/bin
	elif [ -d /usr/X386/bin ]
	then
		BINDIR=/usr/X386/bin
	elif [ -d /usr/X11R5/bin ]
	then
		BINDIR=/usr/X11R5/bin
	elif [ -d /usr/X11R6/bin ]
	then
		BINDIR=/usr/X11R6/bin
	elif [ -d /usr/openwin/bin ]
	then
		BINDIR=/usr/openwin/bin
	elif [ -d /usr/local/bin/X11 ]
	then
		BINDIR=/usr/local/bin/X11
	elif [ -d /usr/local/bin ]
	then
		BINDIR=/usr/local/bin
	elif [ -d /usr/lbin ]
	then
		BINDIR=/usr/lbin
	else
		BINDIR=/usr/bin/X11
	fi
else
	BINDIR=`echo $BINDIR | sed 's/\/\//\//g'`
fi

while :
do
	if getstr "\nEnter X binary directory\n[${BINDIR}]:"
	then
		if [ -d "$ANS" ]
		then
			BINDIR=$ANS
			break
		else
			$ECHO "Error: $ANS does not exist."
		fi
	else
		break
	fi
done


# Determine LIBDIR

if [ -z "$LIBDIR" ]
then
	if [ -d /usr/lib/X11 ]
	then
		LIBDIR=/usr/lib/X11
	elif [ -d /usr/X/lib ]
	then
		LIBDIR=/usr/X/lib
	elif [ -d /usr/X11/lib/X11 ]
	then
		LIBDIR=/usr/X11/lib
	elif [ -d /usr/X386/lib/X11 ]
	then
		LIBDIR=/usr/X386/lib
	elif [ -d /usr/X11R5/lib/X11 ]
	then
		LIBDIR=/usr/X11R5/lib
	elif [ -d /usr/X11R6/lib/X11 ]
	then
		LIBDIR=/usr/X11R6/lib
	elif [ -d /usr/openwin/lib ]
	then
		LIBDIR=/usr/openwin/lib/X11
	elif [ -d /usr/local/lib/X11 ]
	then
		LIBDIR=/usr/local/lib/X11
	else
		LIBDIR=/usr/lib/X11
	fi
else
	LIBDIR=`echo $LIBDIR | sed 's/\/\//\//g'`
fi

while :
do
	if getstr "\nEnter X library directory\n[${LIBDIR}]:"
	then
		if [ -d "$ANS" ]
		then
			LIBDIR=$ANS
			break
		else
			$ECHO "Error: $ANS does not exist."
		fi
	else
		break
	fi
done


# Determine MANFILE

if [ -z "$MANFILE" ]
then
	MANFILE=/usr/man/man1/xmcd.1

	if [ -d /usr/man/man.LOCAL ]
	then
		MANFILE=/usr/man/man.LOCAL/xmcd.LOCAL
	elif [ -d /usr/local/man/man1 ]
	then
		MANFILE=/usr/local/man/man1/xmcd.1
	fi
else
	MANFILE=`echo $MANFILE | sed 's/\/\//\//g'`
fi

if getstr "\nEnter xmcd on-line manual file path\n[${MANFILE}]:"
then
	MANFILE=$ANS
fi

MANDIR=`dirname $MANFILE`
if [ ! -d $MANDIR ]
then
	YNDEF=y
	if getyn "Directory $MANDIR does not exist.  Create it"
	then
		makedir $MANDIR $DIRPERM $OWNER $GROUP
	else
		$ECHO "The xmcd on-line manual will not be installed."
		MANFILE=
		MANDIR=
	fi
fi

# Determine CMANFILE

CMANFILE="`dirname $MANFILE`/`basename $MANFILE | sed 's/xmcd/cda/'`"
if getstr "\nEnter cda on-line manual file path\n[${CMANFILE}]:"
then
	CMANFILE=$ANS
fi

CMANDIR=`dirname $CMANFILE`
if [ ! -d $CMANDIR ]
then
	YNDEF=y
	if getyn "Directory $CMANDIR does not exist.  Create it"
	then
		makedir $CMANDIR $DIRPERM $OWNER $GROUP
	else
		$ECHO "The cda on-line manual will not be installed."
		CMANFILE=
		CMANDIR=
	fi
fi

# Determine WMANFILE

WMANFILE="`dirname $MANFILE`/`basename $MANFILE | sed 's/xmcd/wm2xmcd/'`"
if getstr "\nEnter wm2xmcd on-line manual file path\n[${WMANFILE}]:"
then
	WMANFILE=$ANS
fi

WMANDIR=`dirname $WMANFILE`
if [ ! -d $WMANDIR ]
then
	YNDEF=y
	if getyn "Directory $WMANDIR does not exist.  Create it"
	then
		makedir $WMANDIR $DIRPERM $OWNER $GROUP
	else
		$ECHO "The wm2xmcd on-line manual will not be installed."
		WMANFILE=
		WMANDIR=
	fi
fi


# Make all necessary directories

$ECHO "\nMaking directories..."

makedir $LIBDIR/xmcd $DIRPERM $OWNER $GROUP
makedir $LIBDIR/xmcd/config $DIRPERM $OWNER $GROUP
makedir $LIBDIR/xmcd/config/.tbl $DIRPERM $OWNER $GROUP
makedir $LIBDIR/xmcd/help $DIRPERM $OWNER $GROUP
makedir $LIBDIR/xmcd/cddb $DIRPERM $OWNER $GROUP

for i in $CATEGORIES
do
	makedir $LIBDIR/xmcd/cddb/$i $CDIRPERM $OWNER $GROUP
done


# Install files
$ECHO "\nInstalling xmcd files..."

# Binaries
instfile xmcd.d/xmcd $BINDIR/xmcd $XBINPERM $XBINOWNER $GROUP
instfile cda.d/cda $BINDIR/cda $XBINPERM $XBINOWNER $GROUP
instfile wm2xmcd.d/wm2xmcd $BINDIR/wm2xmcd $BINPERM $BINOWNER $GROUP

# X resource defaults file
if instfile xmcd.d/XMcd.ad $LIBDIR/app-defaults/XMcd $FILEPERM $OWNER $GROUP &&
   [ $LIBDIR != "/usr/lib/X11" -a -d /usr/lib/X11/app-defaults ]
then
	# Test LIBDIR
	rm -f /usr/lib/X11/app-defaults/._junk_

	>$LIBDIR/app-defaults/._junk_

	if [ ! -f /usr/lib/X11/app-defaults/._junk_ ]
	then
		rm -f /usr/lib/X11/app-defaults/XMcd
		dolink $LIBDIR/app-defaults/XMcd /usr/lib/X11/app-defaults/XMcd
	fi

	rm -f $LIBDIR/app-defaults/._junk_
fi

# Help files
for i in xmcd.d/hlpfiles/*Btn xmcd.d/hlpfiles/*Ind xmcd.d/hlpfiles/*Txt \
	 xmcd.d/hlpfiles/*Scale xmcd.d/hlpfiles/*List xmcd.d/hlpfiles/*Box
do
	j=`echo $i | sed 's/xmcd\.d\/hlpfiles\///'`
	instfile $i $LIBDIR/xmcd/help/$j $FILEPERM $OWNER $GROUP
done

# Demo cddb file
instfile misc.d/demo.db $LIBDIR/xmcd/cddb/misc/$DEMODB $FILEPERM $OWNER $GROUP

# Configuration files
instfile libdi.d/common.cfg $LIBDIR/xmcd/config/common.cfg \
	$FILEPERM $OWNER $GROUP
instfile libdi.d/device.cfg $LIBDIR/xmcd/config/device.cfg \
	$FILEPERM $OWNER $GROUP

ENTRIES=`(cd libdi.d/cfgtbl; echo * | \
	sed -e 's/Imakefile//' -e 's/Makefile//' -e 's/SCCS//' -e 's/RCS//')`
for i in $ENTRIES
do
	if (fgrep "tblver=" libdi.d/cfgtbl/$i) >/dev/null 2>&1
	then
		instfile libdi.d/cfgtbl/$i $LIBDIR/xmcd/config/.tbl/$i \
			$FILEPERM $OWNER $GROUP
	fi
done

# Configuration script
SHELL_S=`echo $SHELL | sed 's/\//\\\\\//g'`
LIBDIR_S=`echo $LIBDIR | sed 's/\//\\\\\//g'`
sed -e "s/^#!\/bin\/sh.*/#!$SHELL_S/" \
    -e "s/^LIBDIR=.*/LIBDIR=$LIBDIR_S/" <libdi.d/configure.sh >/tmp/xmcdcfg.$$

instfile /tmp/xmcdcfg.$$ $LIBDIR/xmcd/config/configure.sh \
	$SCRPERM $OWNER $GROUP
rm -f /tmp/xmcdcfg.$$

# Convenience link to configure.sh
if [ "$SCO" = 1 ]
then
	if [ -w /usr/lib/mkdev ]
	then
		$ECHO "\t/usr/lib/mkdev/xmcd"
		rm -f /usr/lib/mkdev/xmcd
		dolink $LIBDIR/xmcd/config/configure.sh /usr/lib/mkdev/xmcd
	fi
fi

# Motif XKeysymDB file
if [ ! -f $LIBDIR/XKeysymDB ]
then
	instfile xmcd.d/XKeysymDB $LIBDIR/XKeysymDB $FILEPERM $OWNER $GROUP
fi

# Manual page files
if [ -n "$MANFILE" ]
then
	instfile xmcd.d/xmcd.man $MANFILE $FILEPERM $OWNER $GROUP
fi
if [ -n "$CMANFILE" ]
then
	instfile cda.d/cda.man $CMANFILE $FILEPERM $OWNER $GROUP
fi
if [ -n "$WMANFILE" ]
then
	instfile wm2xmcd.d/wm2xmcd.man $WMANFILE $FILEPERM $OWNER $GROUP
fi

# Run device-dependent config script
if [ -r $LIBDIR/xmcd/config/configure.sh ]
then
	$SHELL $LIBDIR/xmcd/config/configure.sh
	if [ $? != 0 ]
	then
		logerr -n "$LIBDIR/xmcd/config/configure.sh failed."
	fi
else
	logerr -p "Cannot execute $LIBDIR/xmcd/config/configure.sh"
fi

if [ $NOREMOVE = 0 ]
then
	rm -rf common.d libdi.d misc.d xmcd.d cda.d wm2xmcd.d install.sh \
		xmcd.tar
	mv README README.xmcd 2>/dev/null
	mv FAQ FAQ.xmcd 2>/dev/null
fi

doexit $ERROR

