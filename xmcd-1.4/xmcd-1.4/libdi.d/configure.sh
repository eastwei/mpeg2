#!/bin/sh
#
# @(#)configure.sh	5.13 95/01/22
#
# Script to set up the device-dependent configuration files.
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

PATH=/bin:/usr/bin:/sbin:/usr/sbin:/etc:/usr/local/bin:/usr/ucb
export PATH

ERRFILE=/tmp/xmcd.err

# Change the following directory to fit your local configuration
LIBDIR=/usr/lib/X11

VER=1.4


# Utility functions

doexit()
{
	if [ $1 -eq 0 ]
	then
		$ECHO "\nXmcd set-up is now complete.\n"
		$ECHO "Please read the README file supplied with the xmcd"
		$ECHO "distribution for hardware configuration information"
		$ECHO "about specific CD-ROM drives.\n"
	else
		$ECHO "\nErrors have occurred configuring xmcd."
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
		$ECHO "Error: $2" >&2
	fi
	$ECHO "$2" >>$ERRFILE
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
		logerr -p "Cannot link $1 -> $2"
	fi
}


ask_scsi_config()
{
	$ECHO "\n  Since you have an unlisted CD-ROM drive, I will assume"
	$ECHO "  that it is SCSI-2 compliant.  If this is not true then"
	$ECHO "  xmcd will probably not work."

	YNDEF=n
	if getyn "\n  Do you want to continue"
	then
		METHOD=0
		VENDOR=0
		VOLBASE=0
		VOLTAPER=0
		PLAYNOTUR=0
	else
		return 1
	fi

	$ECHO "\n  You will now be asked several technical questions about"
	$ECHO "  your CD-ROM drive.  If you don't know the answer, try"
	$ECHO "  accepting the default values, and if problems occur when"
	$ECHO "  using xmcd, reconfigure the settings by running this"
	$ECHO "  script again, or editing the $CONFIG"
	$ECHO "  file."
	$ECHO "\n  If you get an unlisted drive working with xmcd in this"
	$ECHO "  manner, the author of xmcd would like to hear from you"
	$ECHO "  and incorporate the settings into the next xmcd release."
	$ECHO "  Please send e-mail to \"ti@amb.org\"."

	while :
	do
		$ECHO "\n  Does your CD-ROM drive on $XMCD_DEV support the following:\n"

		YNDEF=y
		if getyn "  - The Play_Audio_MSF SCSI command"
		then
			PLAYMSF=True
		else
			PLAYMSF=False
		fi

		YNDEF=n
		if getyn "  - The Play_Audio(12) SCSI command"
		then
			PLAY12=True
		else
			PLAY12=False
		fi

		YNDEF=y
		if getyn "  - The Play_Audio(10) SCSI command"
		then
			PLAY10=True
		else
			PLAY10=False
		fi

		YNDEF=y
		if getyn "  - The Play_Audio_Track/Index command"
		then
			PLAYTI=True
		else
			PLAYTI=False
		fi

		YNDEF=n
		if getyn "  - Caddy load via the Start_Stop_Unit SCSI command"
		then
			LOAD=True
		else
			LOAD=False
		fi

		YNDEF=y
		if getyn "  - Caddy eject via the Start_Stop_Unit SCSI command"
		then
			EJECT=True
		else
			EJECT=False
		fi

		YNDEF=y
		if getyn "  - Disable block descriptor in the Mode_Sense SCSI command"
		then
			MODEDBD=True
		else
			MODEDBD=False
		fi

		YNDEF=n
		if getyn "  - Audio volume control via the Mode_Select SCSI command"
		then
			YNDEF=n
			if getyn "  - Independent SCSI Mode_Select volume control for each channel"
			then
				VOLSUPP=True
				BALSUPP=True
			else
				VOLSUPP=True
				BALSUPP=False
			fi

			YNDEF=n
			if getyn "  - Audio channel routing via SCSI Mode_Select"
			then
				CHRSUPP=True
			else
				CHRSUPP=False
			fi
		else
			VOLCTL=0
			VOLSUPP=False
			BALSUPP=False
			CHRSUPP=False
		fi

		YNDEF=y
		if getyn "  - The Pause/Resume SCSI command"
		then
			PAUSE=True
		else
			PAUSE=False
		fi

		YNDEF=y
		if getyn "  - The Prevent/Allow_Medium_Removal SCSI command"
		then
			CADDYLOCK=True
		else
			CADDYLOCK=False
		fi

		YNDEF=n
		if getyn "  - Data Format 1 of the Read_Subchannel SCSI command"
		then
			CURPOSFMT=True
		else
			CURPOSFMT=False
		fi

		$ECHO "\n  This is the configuration for ${XMCD_DEV}:\n"
		$ECHO "  logicalDriverNumber:   $DRVNO"
		$ECHO "  playAudio12Support:    $PLAY12"
		$ECHO "  playAudioMSFSupport:   $PLAYMSF"
		$ECHO "  playAudio10Support:    $PLAY10"
		$ECHO "  playAudioTISupport:    $PLAYTI"
		$ECHO "  loadSupport:           $LOAD"
		$ECHO "  ejectSupport:          $EJECT"
		$ECHO "  modeSenseSetDBD:       $MODEDBD"
		$ECHO "  volumeControlSupport:  $VOLSUPP"
		$ECHO "  balanceControlSupport: $BALSUPP"
		$ECHO "  pauseResumeSupport:    $PAUSE"
		$ECHO "  caddyLockSupport:      $CADDYLOCK"
		$ECHO "  curposFormat:          $CURPOSFMT"

		YNDEF=y
		if getyn "\n  Is this acceptable"
		then
			break
		fi

		$ECHO "  Try again..."
	done

	return 0
}


ask_nonscsi_config()
{
	$ECHO "\n  Non-SCSI CD-ROM drives are currently supported only"
	$ECHO "  via the SunOS/Linux ioctl interface.  If you are not"
	$ECHO "  running Linux then this will not work.  You must have the"
	$ECHO "  appropriate CD-ROM driver configured in your Linux kernel."

	YNDEF=n
	getyn "\n  Do you want to continue"
	if [ $? -ne 0 ]
	then
		return 1
	fi

	# Hardwire the parameters
	METHOD=1
	VENDOR=0
	VOLBASE=0
	VOLTAPER=0
	PLAYMSF=True
	PLAY12=False
	PLAY10=False
	PLAYTI=True
	LOAD=True
	EJECT=True
	MODEDBD=False
	VOLSUPP=True
	BALSUPP=True
	CHRSUPP=False
	VOLCTL=3
	PAUSE=True
	CADDYLOCK=False
	CURPOSFMT=False
	PLAYNOTUR=False

	return 0
}


config_drive()
{
	$ECHO "\n  CD-ROM drive ($XMCD_DEV) configuration"
	$ECHO "  Please select the CD-ROM drive brand:\n"

	eval `
	(
		$ECHO "ENTRIES=\""
		cd $LIBDIR/xmcd/config/.tbl
		for i in *
		do
			if [ -f $i ]
			then
				if fgrep tblver=2 $LIBDIR/xmcd/config/.tbl/$i \
			   		>/dev/null 2>&1
				then
					$ECHO "$i \c"
				else
					logerr -p \
					"$LIBDIR/xmcd/config/.tbl/$i version mismatch"
				fi
			fi
		done
		$ECHO "\""
	)`

	j=1
	if [ -n "$ENTRIES" ]
	then
		for i in $ENTRIES
		do
			brand=`fgrep tblalias= \
				$LIBDIR/xmcd/config/.tbl/$i 2>/dev/null | \
				sed 's/^.*tblalias=//'`
			if [ -z "$brand" ]
			then
				brand=$i
			fi
			$ECHO "  $j.\t$brand"
			j=`expr $j + 1`
		done
	fi
	$ECHO "  $j.\tother (SCSI)"
	j=`expr $j + 1`
	$ECHO "  $j.\tother (non-SCSI)"
	$ECHO "  q.\tquit"

	while :
	do
		$ECHO "\n  Enter choice: \c"
		read ANS

		if [ "$ANS" = q ]
		then
			return 1
		fi

		if [ -z "$ANS" -o "$ANS" -lt 1 -o "$ANS" -gt $j ]
		then
			$ECHO "  Please answer 1 to $j."
		elif [ "$ANS" = "`expr $j - 1`" ]
		then
			ask_scsi_config
			return $?
		elif [ "$ANS" = "$j" ]
		then
			ask_nonscsi_config
			return $?
		else
			k=1
			for i in $ENTRIES
			do
				if [ $k = $ANS ]
				then
					model_sel $i $LIBDIR/xmcd/config/.tbl/$i
					return $?
				fi
				k=`expr $k + 1`
			done

			# Should not get here.
			return 1
		fi
	done

	# Should not get here.
	return 1
}


model_sel()
{
	$ECHO "\n  CD-ROM drive ($XMCD_DEV) configuration"
	$ECHO "  Please select the $1 CD-ROM drive model:\n"

	$AWK -F: '
	BEGIN {
		n = 1
		printf("\t%-20s%s\n\n", "Model", "Mode")
	}
	!/^#/ {
		if ($2 == 0) {
			if ($3 == 1)
				mode = "SunOS/Linux ioctl"
			else
				mode = "other"
		}
		else if ($2 == 1)
			mode = "SCSI-1"
		else if ($2 >= 2)
			mode = "SCSI-2"

		printf("  %d.\t%-20s%s\n", n, $1, mode)
		n++
	}
	END {
		printf("  %d.\t%-20s%s\n", n, "other", "SCSI")
		printf("  %d.\t%-20s%s\n", n+1, "other", "non-SCSI")
		printf("  q.\tquit\n")
	}
	' $2

	while :
	do
		$ECHO "\n  Enter choice: \c"
		read ANS

		j=`grep -v "^#" $2 | wc -l | sed 's/^[ 	]*//'`
		j=`expr $j + 2`

		if [ "$ANS" = q ]
		then
			return 1
		fi

		if [ -z "$ANS" -o "$ANS" -lt 1 -o "$ANS" -gt $j ]
		then
			$ECHO "  Please answer 1 to $j."
		elif [ "$ANS" = "`expr $j - 1`" ]
		then
			ask_scsi_config
		elif [ "$ANS" = "$j" ]
		then
			ask_nonscsi_config
		else
			read_config $2 $ANS
		fi
		return $?
	done

	# Should not get here.
	return 1
}


read_config()
{
	eval `$AWK -F: '
	BEGIN {
		n = 1
	}
	!/^#/ {
		if (n == sel) {
			if ($2 > 0) {
				print "METHOD=0"
				printf("VENDOR=%d\n", $3)
			}
			else {
				printf("METHOD=%d\n", $3)
				print "VENDOR=0"
			}

			if ($4 == 0)
				print "PLAYMSF=False"
			else
				print "PLAYMSF=True"
			if ($5 == 0)
				print "PLAY12=False"
			else
				print "PLAY12=True"
			if ($6 == 0)
				print "PLAY10=False"
			else
				print "PLAY10=True"
			if ($7 == 0)
				print "PLAYTI=False"
			else
				print "PLAYTI=True"
			if ($8 == 0)
				print "LOAD=False"
			else
				print "LOAD=True"
			if ($9 == 0)
				print "EJECT=False"
			else
				print "EJECT=True"
			if ($10 == 0)
				print "MODEDBD=False"
			else
				print "MODEDBD=True"

			printf("VOLCTL=%d\n", $11)
			printf("VOLBASE=%d\n", $12)
			printf("VOLTAPER=%d\n", $13)

			if ($14 == 0)
				print "PAUSE=False"
			else
				print "PAUSE=True"
			if ($15 == 0)
				print "CADDYLOCK=False"
			else
				print "CADDYLOCK=True"
			if ($16 == 0)
				print "CURPOSFMT=False"
			else
				print "CURPOSFMT=True"
			if ($17 == 0)
				print "PLAYNOTUR=False"
			else
				print "PLAYNOTUR=True"
		}
		n++
	}
	' sel=$2 $1`

	return $?
}


# Main starts here

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

# If awk doesn't work well on your system, try changing the
# following to nawk or gawk.
AWK=awk

# Error log file handling
if [ -f $ERRFILE -a ! -w $ERRFILE ]
then
	ERRFILE=/dev/null
fi

$ECHO "\nXmcd version $VER Device Configuration Program"

# Sanity check

if [ ! -w $LIBDIR/xmcd/config ]
then
	logerr -p "No write permission in $LIBDIR/xmcd/config"
	doexit 1
fi

if [ ! -r $LIBDIR/xmcd/config/device.cfg ]
then
	logerr -p "Cannot find $LIBDIR/xmcd/config/device.cfg"
	doexit 2
fi

fgrep cfgver=1 $LIBDIR/xmcd/config/device.cfg >/dev/null 2>&1
if [ $? != 0 ]
then
	logerr -p "$LIBDIR/xmcd/config/device.cfg version mismatch"
	doexit 2
fi

if [ ! -d $LIBDIR/xmcd/config/.tbl ]
then
	logerr -p "The directory $LIBDIR/xmcd/config/.tbl is missing"
	doexit 2
fi

# Configure platform-dependent and device-dependent parameters

OS_SYS=`uname -s 2>/dev/null`
OS_VER=`uname -r 2>/dev/null`
DEVBASE=/dev/rcdrom
DEVSUFFIX=
FIRST=0
BLKDEV=0
LINKLIBS=False
VOLMGT=False
STOPONLOAD=True
EJECTONEXIT=False
STOPONEXIT=True
EXITONEJECT=False
CLOSEONEJECT=False
MAILCMD="mailx -s '%S' %A <%F >/dev/null 2>&1"

# Determine what platform we are running on

if [ "$OS_SYS" = A/UX ]
then
	# Apple A/UX
	DEVBASE=/dev/scsi/
	DEVSUFFIX=
	FIRST=3
	MAILCMD="mush -s '%S' %A <%F >/dev/null 2>&1"
elif [ "$OS_SYS" = dgux ]
then
	# Data General DG/UX
	DEVBASE="/dev/scsi/scsi(ncsc@7(FFFB0000,7),"
	DEVSUFFIX=",0)"
	FIRST=2
elif [ "$OS_SYS" = FreeBSD ]
then
	# FreeBSD
	DEVBASE=/dev/rcd
	DEVSUFFIX=c
	FIRST=0
elif [ "$OS_SYS" = OSF1 ]
then
	case "`uname -m`" in
	alpha)	# DEC OSF/1
		DEVBASE=/dev/rrz
		DEVSUFFIX=c
		FIRST=4
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ "$OS_SYS" = ULTRIX ]
then
	case "`uname -m`" in
	RISC)	# DEC Ultrix
		DEVBASE=/dev/rrz
		DEVSUFFIX=c
		FIRST=4
		MAILCMD="Mail -s '%S' %A <%F >/dev/null 2>&1"
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ "$OS_SYS" = AIX ]
then
	# IBM AIX
	DEVBASE=/dev/rcd
	DEVSUFFIX=
	FIRST=0
elif [ "$OS_SYS" = Linux ]
then
	# Linux
	DEVBASE=/dev/sr
	DEVSUFFIX=
	FIRST=0
	BLKDEV=1
	MAILCMD="elm -s '%S' %A <%F >/dev/null 2>&1"
elif [ "$OS_SYS" = IRIX ]
then
	# SGI IRIX
	DEVBASE=`hinv | grep CDROM | line | \
		sed 's/^.*controller \([0-9]*\).*$/\/dev\/scsi\/sc\1d/'`
	DEVSUFFIX=l0
	FIRST=`hinv | grep CDROM | line | sed 's/^.*unit \([0-9]*\).*$/\1/'`
elif [ -x /bin/hp-pa ] && hp-pa
then
	case $OS_VER in
	A.09*)	# HP-UX 9.x
		DEVBASE=/dev/rdsk/c201d
		DEVSUFFIX=s0
		FIRST=4
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ -x /bin/ftx ] && ftx
then
	case $OS_VER in
	4.*)	
		if [ -x /bin/hppa ] && hppa
		then
			# Stratus FTX SVR4/PA-RISC
			DEVBASE=/dev/rcdrom/c4a1d
			DEVSUFFIX=
			FIRST=0
			LINKLIBS=True
		else
			# On non-supported FTX variants
			OS_VER=unknown
		fi
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ -x /bin/sun ] && sun
then
	case $OS_VER in
	4.*)	# SunOS 4.x
		case `arch -k` in
		sun4[cm])
			DEVBASE=/dev/rsr
			DEVSUFFIX=
			FIRST=0
			MAILCMD="Mail -s '%S' %A <%F >/dev/null 2>&1"
			;;
		*)
			OS_VER=unknown
			;;
		esac
		;;
	5.*)	# SunOS 5.x
		YNDEF=n
		if getyn \
		"Does your system support the Volume Manager (/usr/sbin/vold)"
		then
			DEVBASE=/vol/dev/aliases/cdrom
			DEVSUFFIX=
			FIRST=0
			VOLMGT=True
			CLOSEONEJECT=True
		else
			DEVBASE=/dev/rdsk/c0t
			DEVSUFFIX=d0s0
			FIRST=6
		fi
		LINKLIBS=True
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ -x /bin/i386 -o -x /sbin/i386 ] && i386
then
	case $OS_VER in
	3.2)	# SCO ODT
		DEVBASE=/dev/rcd
		DEVSUFFIX=
		FIRST=0
		;;
	4.0)	# UNIX SVR4.0/x86
		DEVBASE=/dev/rcdrom/cd
		DEVSUFFIX=
		FIRST=0
		LINKLIBS=True
		;;
	4.1)	# UNIX SVR4.1/x86
		DEVBASE=/dev/rcdrom/cdrom
		DEVSUFFIX=
		FIRST=1
		LINKLIBS=True
		;;
	4.2)	# UNIX SVR4.2/x86
		DEVBASE=/dev/rcdrom/cdrom
		DEVSUFFIX=
		FIRST=1
		LINKLIBS=True
		;;
	4*MP)	# UNIX SVR4.2MP/x86
		DEVBASE=/dev/rcdrom/cdrom
		DEVSUFFIX=
		FIRST=1
		LINKLIBS=True
		;;
	*)
		OS_VER=unknown
		;;
	esac
elif [ -x /bin/m88k ] && m88k
then
	case $OS_VER in
	4.0)	# UNIX SVR4.0/88k
		DEVBASE=/dev/rdsk/m187_c0d
		DEVSUFFIX=s7
		FIRST=3
		LINKLIBS=True
		;;
	*)
		OS_VER=unknown
		;;
	esac
else
	OS_VER=unknown
fi

NOT_SUPPORTED="Error: You are not running an operating system that's\n\
currently supported by xmcd."

if [ "$OS_VER" = unknown ]
then
	$ECHO "$NOT_SUPPORTED"
	YNDEF=n
	getyn "Would you like to proceed anyway?"
	if [ $? -ne 0 ]
	then
		$ECHO "\nConfiguration aborted." >&2
		logerr -n "Configuration aborted by user"
		doexit 3
	fi
fi

DRVNO=0
DEVNO=$FIRST
SEDLINE=
while :
do
	$ECHO "\nConfiguring SCSI CD-ROM drive $DRVNO..."

	DEFAULT_DEV="${DEVBASE}${DEVNO}${DEVSUFFIX}"

	while :
	do
		if getstr "\n  Enter CD-ROM device path: [$DEFAULT_DEV]"
		then
			XMCD_DEV=$ANS
		else
			XMCD_DEV=$DEFAULT_DEV
		fi

		if [ $VOLMGT = True ]
		then
			break
		fi
		if [ $BLKDEV = 0 -a -c $XMCD_DEV ]
		then
			break
		fi
		if [ $BLKDEV = 1 -a -b $XMCD_DEV ]
		then
			break
		fi

		$ECHO "  $XMCD_DEV is an invalid device."
	done

	if [ $DRVNO -eq 0 ]
	then
		#
		# Configure app-defaults/XMcd file
		#
		chmod 644 $LIBDIR/app-defaults/XMcd 2>/dev/null
		if [ -w $LIBDIR/app-defaults/XMcd ]
		then
			$AWK '
			/^XMcd\*libdir:/ {
			    printf("XMcd*libdir:\t\t\t%s/xmcd\n", libdir)
			}
			/^XMcd\*cddbMailCmd:/ {
			    printf("XMcd*cddbMailCmd:\t\t%s\n", mailcmd)
			}
			!/^XMcd\*(libdir|cddbMailCmd):/ {
			    print $0
			}' libdir="$LIBDIR" mailcmd="$MAILCMD" \
				$LIBDIR/app-defaults/XMcd > /tmp/xmcd.$$

			cp /tmp/xmcd.$$ $LIBDIR/app-defaults/XMcd
			rm -f /tmp/xmcd.$$

			if [ $LIBDIR != "/usr/lib/X11" -a \
			     -d /usr/lib/X11/app-defaults ]
			then
				rm -f /usr/lib/X11/app-defaults/._trash_
				>$LIBDIR/app-defaults/._trash_

				if [ ! -f /usr/lib/X11/app-defaults/._trash_ ]
				then
					rm -f /usr/lib/X11/app-defaults/XMcd
					dolink $LIBDIR/app-defaults/XMcd \
						/usr/lib/X11/app-defaults/XMcd
				fi

				rm -f $LIBDIR/app-defaults/._trash_
			fi
		else
			logerr -p "Cannot configure $LIBDIR/app-defaults/XMcd"
		fi

		#
		# Configure common.cfg file
		#
		chmod 644 $LIBDIR/xmcd/config/common.cfg 2>/dev/null
		if [ -w $LIBDIR/xmcd/config/common.cfg ]
		then
			$AWK '
			/^device:/	{
			    printf("device:\t\t\t\t%s\n", device)
			}
			/^solaris2VolumeManager:/ {
			    printf("solaris2VolumeManager:\t%s\n", volmgt)
			}
			!/^(device|solaris2VolumeManager):/ {
			    print $0
			}' device=$XMCD_DEV volmgt=$VOLMGT \
				$LIBDIR/xmcd/config/common.cfg > /tmp/xmcd.$$

			cp /tmp/xmcd.$$ $LIBDIR/xmcd/config/common.cfg
			rm -f /tmp/xmcd.$$
		else
			logerr -p \
			    "Cannot configure $LIBDIR/xmcd/config/common.cfg"
		fi
	fi

	CONFIG=$LIBDIR/xmcd/config/`basename $XMCD_DEV`

	config_drive
	if [ $? != 0 ]
	then
		$ECHO "\nConfiguration aborted." >&2
		logerr -n "Configuration aborted by user."
		doexit $?
	fi

	DRVNOTICE="\n\n  NOTE: This drive does not support these features:"

	if [ $PLAYTI = False ]
	then
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The Previous Index and Next Index buttons."
	fi

	if [ $CADDYLOCK = False ]
	then
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The caddy lock."
	fi

	if [ $LOAD = False ]
	then
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - Software-controlled caddy load."
	fi

	if [ $PAUSE = False -a $VENDOR = 0 ]
	then
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - Audio pause/resume function."
	fi

	case "$VOLCTL" in
	0)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The volume, balance and channel routing controls."
		VOLSUPP=False
		BALSUPP=False
		CHRSUPP=False
		;;
	1)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The balance and channel routing controls."
		VOLSUPP=True
		BALSUPP=False
		CHRSUPP=False
		;;
	2)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The volume and channel routing controls."
		VOLSUPP=False
		BALSUPP=True
		CHRSUPP=False
		;;
	3)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The channel routing control."
		VOLSUPP=True
		BALSUPP=True
		CHRSUPP=False
		;;
	4)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The volume and balance controls."
		VOLSUPP=False
		BALSUPP=False
		CHRSUPP=True
		;;
	5)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The balance control."
		VOLSUPP=True
		BALSUPP=False
		CHRSUPP=True
		;;
	6)
		$ECHO "$DRVNOTICE"
		DRVNOTICE="\c"
		$ECHO "  - The volume control."
		VOLSUPP=False
		BALSUPP=True
		CHRSUPP=True
		;;
	7)
		VOLSUPP=True
		BALSUPP=True
		CHRSUPP=True
		;;
	*)
		;;
	esac

	$ECHO "\n  Creating the $CONFIG file..."

	sed \
	-e "s/^logicalDriveNumber:.*/logicalDriveNumber:	$DRVNO/" \
	-e "s/^deviceInterfaceMethod:.*/deviceInterfaceMethod:	$METHOD/" \
	-e "s/^driveVendorCode:.*/driveVendorCode:	$VENDOR/" \
	-e "s/^playAudio12Support:.*/playAudio12Support:	$PLAY12/" \
	-e "s/^playAudioMSFSupport:.*/playAudioMSFSupport:	$PLAYMSF/" \
	-e "s/^playAudio10Support:.*/playAudio10Support:	$PLAY10/" \
	-e "s/^playAudioTISupport:.*/playAudioTISupport:	$PLAYTI/" \
	-e "s/^loadSupport:.*/loadSupport:		$LOAD/" \
	-e "s/^ejectSupport:.*/ejectSupport:		$EJECT/" \
	-e "s/^modeSenseSetDBD:.*/modeSenseSetDBD:	$MODEDBD/" \
	-e "s/^volumeControlSupport:.*/volumeControlSupport:	$VOLSUPP/" \
	-e "s/^balanceControlSupport:.*/balanceControlSupport:	$BALSUPP/" \
	-e "s/^channelRouteSupport:.*/channelRouteSupport:	$CHRSUPP/" \
	-e "s/^volumeControlTaper:.*/volumeControlTaper:	$VOLTAPER/" \
	-e "s/^scsiAudioVolumeBase:.*/scsiAudioVolumeBase:	$VOLBASE/" \
	-e "s/^pauseResumeSupport:.*/pauseResumeSupport:	$PAUSE/" \
	-e "s/^caddyLockSupport:.*/caddyLockSupport:	$CADDYLOCK/" \
	-e "s/^curposFormat:.*/curposFormat:		$CURPOSFMT/" \
	-e "s/^noTURWhenPlaying:.*/noTURWhenPlaying:	$PLAYNOTUR/" \
	-e "s/^spinDownOnLoad:.*/spinDownOnLoad:		$STOPONLOAD/" \
	-e "s/^ejectOnExit:.*/ejectOnExit:		$EJECTONEXIT/" \
	-e "s/^stopOnExit:.*/stopOnExit:		$STOPONEXIT/" \
	-e "s/^exitOnEject:.*/exitOnEject:		$EXITONEJECT/" \
	-e "s/^closeOnEject:.*/closeOnEject:		$CLOSEONEJECT/" \
	   < $LIBDIR/xmcd/config/device.cfg > $CONFIG
	chmod 644 $CONFIG 2>/dev/null
	chown bin $CONFIG 2>/dev/null
	chgrp bin $CONFIG 2>/dev/null

	YNDEF=n
	if getyn "\n  Do you have more SCSI CD-ROM drives on your system"
	then
		DRVNO=`expr $DRVNO + 1`
		DEVNO=`expr $DEVNO + 1`
	else
		break
	fi
done


if [ $LINKLIBS = True ]
then
MSG="\nFor security reasons, SVR4 setuid programs (such as\n\
xmcd) search only /usr/lib and /usr/ccs/lib for shared\n\
libraries, unless explicitly compiled otherwise.\n\
Most X shared libraries on SVR4 systems do not reside\n\
in the standard locations and xmcd may not run correctly.\n\
Symbolic links can be made to correct this problem.\n"

	FIRST=1
	for i in $LIBDIR /usr/X/lib /usr/X11/lib /usr/X386/lib /usr/X11R5/lib \
		 /usr/X11R6/lib /usr/lib/X11
	do
		if [ $i != /usr/lib -a -d $i ]
		then
			cd $i
			for j in libX*.so*
			do
				if [ "$j" != 'libX*.so*' -a ! -f /usr/lib/"$j" ]
				then
					if [ $FIRST -eq 1 ]
					then
						$ECHO "$MSG"
						FIRST=0
					fi

					YNDEF=y
					if getyn "Link $i/$j to /usr/lib/$j"
					then
						dolink $i/$j /usr/lib/$j
					else
						$ECHO "$i/$j not linked."
					fi
				fi
			done
		fi
	done
fi

doexit 0

