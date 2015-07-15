/*
 *   xmcd - Motif(tm) CD Audio Player
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
#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#ifndef LINT
static char *_resource_h_ident_ = "@(#)resource.h	5.6 95/02/16";
#endif

#define XmcdNdevnum			"devnum"
#define XmcdCDevnum			"Devnum"
#define XmcdNdevice			"device"
#define XmcdCDevice			"Device"
#define XmcdNlibdir			"libdir"
#define XmcdCLibdir			"Libdir"
#define XmcdNdbdir			"dbdir"
#define XmcdCDbdir			"Dbdir"
#define XmcdNmaxDbdirs			"maxDbdirs"
#define XmcdCMaxDbdirs			"MaxDbdirs"
#define XmcdNdbFileMode			"dbFileMode"
#define XmcdCDbFileMode			"DbFileMode"
#define XmcdNcddbMailSite		"cddbMailSite"
#define XmcdCCddbMailSite		"CddbMailSite"
#define XmcdNcddbMailCmd		"cddbMailCmd"
#define XmcdCCddbMailCmd		"CddbMailCmd"

#define XmcdNcaddyLock			"caddyLock"
#define XmcdCCaddyLock			"CaddyLock"
#define XmcdNdeviceInterfaceMethod	"deviceInterfaceMethod"
#define XmcdCDeviceInterfaceMethod	"DeviceInterfaceMethod"
#define XmcdNstatusPollInterval		"statusPollInterval"
#define XmcdCStatusPollInterval		"StatusPollInterval"
#define XmcdNinsertPollInterval		"insertPollInterval"
#define XmcdCInsertPollInterval		"InsertPollInterval"
#define XmcdNpreviousThreshold		"previousThreshold"
#define XmcdCPreviousThreshold		"PreviousThreshold"
#define XmcdNsearchSkipBlocks		"searchSkipBlocks"
#define XmcdCSearchSkipBlocks		"SearchSkipBlocks"
#define XmcdNsearchPauseInterval	"searchPauseInterval"
#define XmcdCSearchPauseInterval	"SearchPauseInterval"
#define XmcdNsearchSpeedUpCount		"searchSpeedUpCount"
#define XmcdCSearchSpeedUpCount		"SearchSpeedUpCount"
#define XmcdNsearchVolumePercent	"searchVolumePercent"
#define XmcdCSearchVolumePercent	"SearchVolumePercent"
#define XmcdNsearchMinVolume		"searchMinVolume"
#define XmcdCSearchMinVolume		"SearchMinVolume"
#define XmcdNsampleBlocks		"sampleBlocks"
#define XmcdCSampleBlocks		"SampleBlocks"
#define XmcdNminimumPlayBlocks		"minimumPlayBlocks"
#define XmcdCMinimumPlayBlocks		"MinimumPlayBlocks"
#define XmcdNdisplayBlinkOnInterval	"displayBlinkOnInterval"
#define XmcdCDisplayBlinkOnInterval	"DisplayBlinkOnInterval"
#define XmcdNdisplayBlinkOffInterval	"displayBlinkOffInterval"
#define XmcdCDisplayBlinkOffInterval	"DisplayBlinkOffInterval"
#define XmcdNdriveVendorCode		"driveVendorCode"
#define XmcdCDriveVendorCode		"DriveVendorCode"
#define XmcdNplayAudio10Support		"playAudio10Support"
#define XmcdCPlayAudio10Support		"PlayAudio10Support"
#define XmcdNplayAudio12Support		"playAudio12Support"
#define XmcdCPlayAudio12Support		"PlayAudio12Support"
#define XmcdNplayAudioMSFSupport	"playAudioMSFSupport"
#define XmcdCPlayAudioMSFSupport	"PlayAudioMSFSupport"
#define XmcdNplayAudioTISupport		"playAudioTISupport"
#define XmcdCPlayAudioTISupport		"PlayAudioTISupport"
#define XmcdNloadSupport		"loadSupport"
#define XmcdCLoadSupport		"LoadSupport"
#define XmcdNejectSupport		"ejectSupport"
#define XmcdCEjectSupport		"EjectSupport"
#define XmcdNmodeSenseSetDBD		"modeSenseSetDBD"
#define XmcdCModeSenseSetDBD		"ModeSenseSetDBD"
#define XmcdNvolumeControlSupport	"volumeControlSupport"
#define XmcdCVolumeControlSupport	"VolumeControlSupport"
#define XmcdNbalanceControlSupport	"balanceControlSupport"
#define XmcdCBalanceControlSupport	"BalanceControlSupport"
#define XmcdNchannelRouteSupport	"channelRouteSupport"
#define XmcdCChannelRouteSupport	"ChannelRouteSupport"
#define XmcdNscsiAudioVolumeBase	"scsiAudioVolumeBase"
#define XmcdCScsiAudioVolumeBase	"ScsiAudioVolumeBase"
#define XmcdNvolumeControlTaper		"volumeControlTaper"
#define XmcdCVolumeControlTaper		"VolumeControlTaper"
#define XmcdNchannelRoute		"channelRoute"
#define XmcdCChannelRoute		"ChannelRoute"
#define XmcdNpauseResumeSupport		"pauseResumeSupport"
#define XmcdCPauseResumeSupport		"PauseResumeSupport"
#define XmcdNcaddyLockSupport		"caddyLockSupport"
#define XmcdCCaddyLockSupport		"CaddyLockSupport"
#define XmcdNnoTURWhenPlaying		"noTURWhenPlaying"
#define XmcdCNoTURWhenPlaying		"NoTURWhenPlaying"
#define XmcdNcurposFormat		"curposFormat"
#define XmcdCCurposFormat		"CurposFormat"
#define XmcdNspinDownOnLoad		"spinDownOnLoad"
#define XmcdCSpinDownOnLoad		"SpinDownOnLoad"
#define XmcdNplayOnLoad			"playOnLoad"
#define XmcdCPlayOnLoad			"PlayOnLoad"
#define XmcdNejectOnDone		"ejectOnDone"
#define XmcdCEjectOnDone		"EjectOnDone"
#define XmcdNejectOnExit		"ejectOnExit"
#define XmcdCEjectOnExit		"EjectOnExit"
#define XmcdNstopOnExit			"stopOnExit"
#define XmcdCStopOnExit			"StopOnExit"
#define XmcdNexitOnEject		"exitOnEject"
#define XmcdCExitOnEject		"ExitOnEject"
#define XmcdNcloseOnEject		"closeOnEject"
#define XmcdCCloseOnEject		"CloseOnEject"
#define XmcdNsolaris2VolumeManager	"solaris2VolumeManager"
#define XmcdCSolaris2VolumeManager	"Solaris2VolumeManager"
#define XmcdNshowScsiErrMsg		"showScsiErrMsg"
#define XmcdCShowScsiErrMsg		"ShowScsiErrMsg"
#define XmcdNmainShowFocus		"mainShowFocus"
#define XmcdCMainShowFocus		"MainShowFocus"
#define XmcdNdebugMode			"debugMode"
#define XmcdCDebugMode			"DebugMode"

#define XmcdNmainWindowTitle		"mainWindowTitle"
#define XmcdCMainWindowTitle		"MainWindowTitle"
#define XmcdNdbModeMsg			"dbModeMsg"
#define XmcdCDbModeMsg			"DbModeMsg"
#define XmcdNprogModeMsg		"progModeMsg"
#define XmcdCProgModeMsg		"ProgModeMsg"
#define XmcdNelapseMsg			"elapseMsg"
#define XmcdCElapseMsg			"ElapseMsg"
#define XmcdNremainTrackMsg		"remainTrackMsg"
#define XmcdCRemainTrackMsg		"RemainTrackMsg"
#define XmcdNremainDiscMsg		"remainDiscMsg"
#define XmcdCRemainDiscMsg		"RemainDiscMsg"
#define XmcdNplayMsg			"playMsg"
#define XmcdCPlayMsg			"PlayMsg"
#define XmcdNpauseMsg			"pauseMsg"
#define XmcdCPauseMsg			"PauseMsg"
#define XmcdNreadyMsg			"readyMsg"
#define XmcdCReadyMsg			"ReadyMsg"
#define XmcdNsampleMsg			"sampleMsg"
#define XmcdCSampleMsg			"SampleMsg"
#define XmcdNusageMsg			"usageMsg"
#define XmcdCUsageMsg			"UsageMsg"
#define XmcdNbadOptsMsg			"badOptsMsg"
#define XmcdCBadOptsMsg			"BadOptsMsg"
#define XmcdNnoDiscMsg			"noDiscMsg"
#define XmcdCNoDiscMsg			"NoDiscMsg"
#define XmcdNdevBusyMsg			"devBusyMsg"
#define XmcdCDevBusyMsg			"DevBusyMsg"
#define XmcdNunknownDiscMsg		"unknownDiscMsg"
#define XmcdCUnknownDiscMsg		"UnknownDiscMsg"
#define XmcdNunknownTrackMsg		"unknownTrackMsg"
#define XmcdCUnknownTrackMsg		"UnknownTrackMsg"
#define XmcdNdataMsg			"dataMsg"
#define XmcdCDataMsg			"DataMsg"
#define XmcdNwarningMsg			"warningMsg"
#define XmcdCWarningMsg			"WarningMsg"
#define XmcdNfatalMsg			"fatalMsg"
#define XmcdCFatalMsg			"FatalMsg"
#define XmcdNconfirmMsg			"confirmMsg"
#define XmcdCConfirmMsg			"ConfirmMsg"
#define XmcdNinfoMsg			"infoMsg"
#define XmcdCInfoMsg			"InfoMsg"
#define XmcdNaboutMsg			"aboutMsg"
#define XmcdCAboutMsg			"AboutMsg"
#define XmcdNquitMsg			"quitMsg"
#define XmcdCQuitMsg			"QuitMsg"
#define XmcdNnoMemMsg			"noMemMsg"
#define XmcdCNoMemMsg			"NoMemMsg"
#define XmcdNtmpdirErrMsg		"tmpdirErrMsg"
#define XmcdCTmpdirErrMsg		"TmpdirErrMsg"
#define XmcdNlibdirErrMsg		"libdirErrMsg"
#define XmcdCLibdirErrMsg		"LibdirErrMsg"
#define XmcdNnoMethodErrMsg		"noMethodErrMsg"
#define XmcdCNoMethodErrMsg		"NoMethodErrMsg"
#define XmcdNnoVuErrMsg			"noVuErrMsg"
#define XmcdCNoVuErrMsg			"NoVuErrMsg"
#define XmcdNnoHelpMsg			"noHelpMsg"
#define XmcdCNoHelpMsg			"NoHelpMsg"
#define XmcdNnoLinkMsg			"noLinkMsg"
#define XmcdCNoLinkMsg			"NoLinkMsg"
#define XmcdNnoDbMsg			"noDbMsg"
#define XmcdCNoDbMsg			"NoDbMsg"
#define XmcdNnoCfgMsg			"noCfgMsg"
#define XmcdCNoCfgMsg			"NoCfgMsg"
#define XmcdNnotRomMsg			"notRomMsg"
#define XmcdCNotRomMsg			"NotRomMsg"
#define XmcdNnotScsi2Msg		"notScsi2Msg"
#define XmcdCNotScsi2Msg		"NotScsi2Msg"
#define XmcdNsendConfirmMsg		"sendConfirmMsg"
#define XmcdCSendConfirmMsg		"SendConfirmMsg"
#define XmcdNmailErrMsg			"mailErrMsg"
#define XmcdCMailErrMsg			"MailErrMsg"
#define XmcdNmodeErrMsg			"modeErrMsg"
#define XmcdCModeErrMsg			"ModeErrMsg"
#define XmcdNstatErrMsg			"statErrMsg"
#define XmcdCStatErrMsg			"StatErrMsg"
#define XmcdNnodeErrMsg			"nodeErrMsg"
#define XmcdCNodeErrMsg			"NodeErrMsg"
#define XmcdNseqFmtErrMsg		"seqFmtErrMsg"
#define XmcdCSeqFmtErrMsg		"SeqFmtErrMsg"
#define XmcdNdbdirsErrMsg		"dbdirsErrMsg"
#define XmcdCDbdirsErrMsg		"DbdirsErrMsg"
#define XmcdNrecovErrMsg		"recovErrMsg"
#define XmcdCRecovErrMsg		"RecovErrMsg"
#define XmcdNmaxErrMsg			"maxErrMsg"
#define XmcdCMaxErrMsg			"MaxErrMsg"
#define XmcdNsavErrForkMsg		"savErrForkMsg"
#define XmcdCSavErrForkMsg		"SavErrForkMsg"
#define XmcdNsavErrSuidMsg		"savErrSuidMsg"
#define XmcdCSavErrSuidMsg		"SavErrSuidMsg"
#define XmcdNsavErrOpenMsg		"savErrOpenMsg"
#define XmcdCSavErrOpenMsg		"SavErrOpenMsg"
#define XmcdNsavErrCloseMsg		"savErrCloseMsg"
#define XmcdCSavErrCloseMsg		"SavErrCloseMsg"
#define XmcdNsavErrKilledMsg		"savErrKilledMsg"
#define XmcdCSavErrKilledMsg		"SavErrKilledMsg"
#define XmcdNlnkErrForkMsg		"lnkErrForkMsg"
#define XmcdCLnkErrForkMsg		"LnkErrForkMsg"
#define XmcdNlnkErrSuidMsg		"lnkErrSuidMsg"
#define XmcdCLnkErrSuidMsg		"LnkErrSuidMsg"
#define XmcdNlnkErrLinkMsg		"lnkErrLinkMsg"
#define XmcdCLnkErrLinkMsg		"LnkErrLinkMsg"
#define XmcdNlnkErrKilledMsg		"lnkErrKilledMsg"
#define XmcdCLnkErrKilledMsg		"LnkErrKilledMsg"

#define XmcdNbuttonLabelKey		"buttonLabelKey"
#define XmcdCButtonLabelKey		"ButtonLabelKey"
#define XmcdNlockKey			"lockKey"
#define XmcdCLockKey			"LockKey"
#define XmcdNrepeatKey			"repeatKey"
#define XmcdCRepeatKey			"RepeatKey"
#define XmcdNshuffleKey			"shuffleKey"
#define XmcdCShuffleKey			"ShuffleKey"
#define XmcdNejectKey			"ejectKey"
#define XmcdCEjectKey			"EjectKey"
#define XmcdNpowerOffKey		"powerOffKey"
#define XmcdCPowerOffKey		"PowerOffKey"
#define XmcdNdbprogKey			"dbprogKey"
#define XmcdCDbprogKey			"DbprogKey"
#define XmcdNhelpKey			"helpKey"
#define XmcdCHelpKey			"HelpKey"
#define XmcdNoptionsKey			"optionsKey"
#define XmcdCOptionsKey			"OptionsKey"
#define XmcdNtimeKey			"timeKey"
#define XmcdCTimeKey			"TimeKey"
#define XmcdNabKey			"abKey"
#define XmcdCAbKey			"AbKey"
#define XmcdNsampleKey			"sampleKey"
#define XmcdCSampleKey			"SampleKey"
#define XmcdNkeypadKey			"keypadKey"
#define XmcdCKeypadKey			"KeypadKey"
#define XmcdNplayPauseKey		"playPauseKey"
#define XmcdCPlayPauseKey		"PlayPauseKey"
#define XmcdNstopKey			"stopKey"
#define XmcdCStopKey			"StopKey"
#define XmcdNprevTrackKey		"prevTrackKey"
#define XmcdCPrevTrackKey		"PrevTrackKey"
#define XmcdNnextTrackKey		"nextTrackKey"
#define XmcdCNextTrackKey		"NextTrackKey"
#define XmcdNprevIndexKey		"prevIndexKey"
#define XmcdCPrevIndexKey		"PrevIndexKey"
#define XmcdNnextIndexKey		"nextIndexKey"
#define XmcdCNextIndexKey		"NextIndexKey"
#define XmcdNrewKey			"rewKey"
#define XmcdCRewKey			"RewKey"
#define XmcdNffKey			"ffKey"
#define XmcdCFfKey			"FfKey"
#define XmcdNkeypadNumKey0		"keypadNumKey0"
#define XmcdCKeypadNumKey0		"KeypadNumKey0"
#define XmcdNkeypadNumKey1		"keypadNumKey1"
#define XmcdCKeypadNumKey1		"KeypadNumKey1"
#define XmcdNkeypadNumKey2		"keypadNumKey2"
#define XmcdCKeypadNumKey2		"KeypadNumKey2"
#define XmcdNkeypadNumKey3		"keypadNumKey3"
#define XmcdCKeypadNumKey3		"KeypadNumKey3"
#define XmcdNkeypadNumKey4		"keypadNumKey4"
#define XmcdCKeypadNumKey4		"KeypadNumKey4"
#define XmcdNkeypadNumKey5		"keypadNumKey5"
#define XmcdCKeypadNumKey5		"KeypadNumKey5"
#define XmcdNkeypadNumKey6		"keypadNumKey6"
#define XmcdCKeypadNumKey6		"KeypadNumKey6"
#define XmcdNkeypadNumKey7		"keypadNumKey7"
#define XmcdCKeypadNumKey7		"KeypadNumKey7"
#define XmcdNkeypadNumKey8		"keypadNumKey8"
#define XmcdCKeypadNumKey8		"KeypadNumKey8"
#define XmcdNkeypadNumKey9		"keypadNumKey9"
#define XmcdCKeypadNumKey9		"KeypadNumKey9"
#define XmcdNkeypadClearKey		"keypadClearKey"
#define XmcdCKeypadClearKey		"KeypadClearKey"
#define XmcdNkeypadEnterKey		"keypadEnterKey"
#define XmcdCKeypadEnterKey		"KeypadEnterKey"
#define XmcdNkeypadCancelKey		"keypadCancelKey"
#define XmcdCKeypadCancelKey		"KeypadCancelKey"


STATIC XtResource	resources[] = {
	{
		XmcdNdevnum, XmcdCDevnum,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, devnum), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNdevice, XmcdCDevice,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, device), XmRImmediate,
		(XtPointer) NULL,
	},
	{
		XmcdNlibdir, XmcdCLibdir,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, libdir), XmRImmediate,
		(XtPointer) NULL,
	},
	{
		XmcdNdbdir, XmcdCDbdir,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, dbdir), XmRImmediate,
		(XtPointer) NULL,
	},
	{
		XmcdNmaxDbdirs, XmcdCMaxDbdirs,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, max_dbdirs), XmRImmediate,
		(XtPointer) MAX_DBDIRS,
	},
	{
		XmcdNdbFileMode, XmcdCDbFileMode,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, dbfile_mode), XmRImmediate,
		(XtPointer) NULL,
	},
	{
		XmcdNcddbMailSite, XmcdCCddbMailSite,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, cddb_mailsite), XmRImmediate,
		(XtPointer) CDDB_MAILSITE,
	},
	{
		XmcdNcddbMailCmd, XmcdCCddbMailCmd,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, cddb_mailcmd), XmRImmediate,
		(XtPointer) CDDB_MAILCMD,
	},
	{
		XmcdNcaddyLock, XmcdCCaddyLock,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, caddy_lock), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNspinDownOnLoad, XmcdCSpinDownOnLoad,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, load_spindown), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNplayOnLoad, XmcdCPlayOnLoad,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, load_play), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNejectOnDone, XmcdCEjectOnDone,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, done_eject), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNejectOnExit, XmcdCEjectOnExit,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, exit_eject), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNstopOnExit, XmcdCStopOnExit,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, exit_stop), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNexitOnEject, XmcdCExitOnEject,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, eject_exit), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNcloseOnEject, XmcdCCloseOnEject,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, eject_close), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNsolaris2VolumeManager, XmcdCSolaris2VolumeManager,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, sol2_volmgt), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNshowScsiErrMsg, XmcdCShowScsiErrMsg,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, scsierr_msg), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNmainShowFocus, XmcdCMainShowFocus,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, main_showfocus), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNdebugMode, XmcdCDebugMode,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, debug), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNdeviceInterfaceMethod, XmcdCDeviceInterfaceMethod,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, di_method), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNstatusPollInterval, XmcdCStatusPollInterval,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, stat_interval), XmRImmediate,
		(XtPointer) 260,
	},
	{
		XmcdNinsertPollInterval, XmcdCInsertPollInterval,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, ins_interval), XmRImmediate,
		(XtPointer) 2000,
	},
	{
		XmcdNpreviousThreshold, XmcdCPreviousThreshold,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, prev_threshold), XmRImmediate,
		(XtPointer) 100,
	},
	{
		XmcdNsearchSkipBlocks, XmcdCSearchSkipBlocks,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, skip_blks), XmRImmediate,
		(XtPointer) 90,
	},
	{
		XmcdNsearchPauseInterval, XmcdCSearchPauseInterval,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, skip_pause), XmRImmediate,
		(XtPointer) 45,
	},
	{
		XmcdNsearchSpeedUpCount, XmcdCSearchSpeedUpCount,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, skip_spdup), XmRImmediate,
		(XtPointer) 15,
	},
	{
		XmcdNsearchVolumePercent, XmcdCSearchVolumePercent,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, skip_vol), XmRImmediate,
		(XtPointer) 35,
	},
	{
		XmcdNsearchMinVolume, XmcdCSearchMinVolume,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, skip_minvol), XmRImmediate,
		(XtPointer) 2,
	},
	{
		XmcdNsampleBlocks, XmcdCSampleBlocks,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, sample_blks), XmRImmediate,
		(XtPointer) 750,
	},
	{
		XmcdNminimumPlayBlocks, XmcdCMinimumPlayBlocks,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, min_playblks), XmRImmediate,
		(XtPointer) 25,
	},
	{
		XmcdNdisplayBlinkOnInterval, XmcdCDisplayBlinkOnInterval,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, blinkon_interval), XmRImmediate,
		(XtPointer) 850,
	},
	{
		XmcdNdisplayBlinkOffInterval, XmcdCDisplayBlinkOffInterval,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, blinkoff_interval), XmRImmediate,
		(XtPointer) 150,
	},
	{
		XmcdNdriveVendorCode, XmcdCDriveVendorCode,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, vendor_code), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNplayAudio10Support, XmcdCPlayAudio10Support,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, play10_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNplayAudio12Support, XmcdCPlayAudio12Support,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, play12_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNplayAudioMSFSupport, XmcdCPlayAudioMSFSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, playmsf_supp), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNplayAudioTISupport, XmcdCPlayAudioTISupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, playti_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNloadSupport, XmcdCLoadSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, load_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNejectSupport, XmcdCEjectSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, eject_supp), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNmodeSenseSetDBD, XmcdCModeSenseSetDBD,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, msen_dbd), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNvolumeControlSupport, XmcdCVolumeControlSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, mselvol_supp), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNbalanceControlSupport, XmcdCBalanceControlSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, balance_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNchannelRouteSupport, XmcdCChannelRouteSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, chroute_supp), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNscsiAudioVolumeBase, XmcdCScsiAudioVolumeBase,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, base_scsivol), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNvolumeControlTaper, XmcdCVolumeControlTaper,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, vol_taper), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNchannelRoute, XmcdCChannelRoute,
		XmRInt, sizeof(int),
		XtOffsetOf(appdata_t, ch_route), XmRImmediate,
		(XtPointer) 0,
	},
	{
		XmcdNpauseResumeSupport, XmcdCPauseResumeSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, pause_supp), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNcaddyLockSupport, XmcdCCaddyLockSupport,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, caddylock_supp), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNnoTURWhenPlaying, XmcdCNoTURWhenPlaying,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, play_notur), XmRImmediate,
		(XtPointer) False,
	},
	{
		XmcdNcurposFormat, XmcdCCurposFormat,
		XmRBoolean, sizeof(Boolean),
		XtOffsetOf(appdata_t, curpos_fmt), XmRImmediate,
		(XtPointer) True,
	},
	{
		XmcdNmainWindowTitle, XmcdCMainWindowTitle,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, main_title), XmRImmediate,
		(XtPointer) MAIN_TITLE,
	},
	{
		XmcdNdbModeMsg, XmcdCDbModeMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_dbmode), XmRImmediate,
		(XtPointer) STR_DBMODE,
	},
	{
		XmcdNprogModeMsg, XmcdCProgModeMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_progmode), XmRImmediate,
		(XtPointer) STR_PROGMODE,
	},
	{
		XmcdNelapseMsg, XmcdCElapseMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_elapse), XmRImmediate,
		(XtPointer) STR_ELAPSE,
	},
	{
		XmcdNremainTrackMsg, XmcdCRemainTrackMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_remaintrk), XmRImmediate,
		(XtPointer) STR_REMAIN_TRK,
	},
	{
		XmcdNremainDiscMsg, XmcdCRemainDiscMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_remaindisc), XmRImmediate,
		(XtPointer) STR_REMAIN_DISC,
	},
	{
		XmcdNplayMsg, XmcdCPlayMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_play), XmRImmediate,
		(XtPointer) STR_PLAY,
	},
	{
		XmcdNpauseMsg, XmcdCPauseMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_pause), XmRImmediate,
		(XtPointer) STR_PAUSE,
	},
	{
		XmcdNreadyMsg, XmcdCReadyMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_ready), XmRImmediate,
		(XtPointer) STR_READY,
	},
	{
		XmcdNsampleMsg, XmcdCSampleMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_sample), XmRImmediate,
		(XtPointer) STR_SAMPLE,
	},
	{
		XmcdNusageMsg, XmcdCUsageMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_usage), XmRImmediate,
		(XtPointer) STR_USAGE,
	},
	{
		XmcdNbadOptsMsg, XmcdCBadOptsMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_badopts), XmRImmediate,
		(XtPointer) STR_BADOPTS,
	},
	{
		XmcdNnoDiscMsg, XmcdCNoDiscMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nodisc), XmRImmediate,
		(XtPointer) STR_NODISC,
	},
	{
		XmcdNdevBusyMsg, XmcdCDevBusyMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_busy), XmRImmediate,
		(XtPointer) STR_BUSY,
	},
	{
		XmcdNunknownDiscMsg, XmcdCUnknownDiscMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_unkndisc), XmRImmediate,
		(XtPointer) STR_UNKNDISC,
	},
	{
		XmcdNunknownTrackMsg, XmcdCUnknownTrackMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_unkntrk), XmRImmediate,
		(XtPointer) STR_UNKNTRK,
	},
	{
		XmcdNdataMsg, XmcdCDataMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_data), XmRImmediate,
		(XtPointer) STR_DATA,
	},
	{
		XmcdNwarningMsg, XmcdCWarningMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_warning), XmRImmediate,
		(XtPointer) STR_WARNING,
	},
	{
		XmcdNfatalMsg, XmcdCFatalMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_fatal), XmRImmediate,
		(XtPointer) STR_FATAL,
	},
	{
		XmcdNconfirmMsg, XmcdCConfirmMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_confirm), XmRImmediate,
		(XtPointer) STR_CONFIRM,
	},
	{
		XmcdNinfoMsg, XmcdCInfoMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_info), XmRImmediate,
		(XtPointer) STR_INFO,
	},
	{
		XmcdNaboutMsg, XmcdCAboutMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_about), XmRImmediate,
		(XtPointer) STR_ABOUT,
	},
	{
		XmcdNquitMsg, XmcdCQuitMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_quit), XmRImmediate,
		(XtPointer) STR_QUIT,
	},
	{
		XmcdNnoMemMsg, XmcdCNoMemMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nomemory), XmRImmediate,
		(XtPointer) STR_NOMEMORY,
	},
	{
		XmcdNnoMethodErrMsg, XmcdCNoMethodErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nomethod), XmRImmediate,
		(XtPointer) STR_NOMETHOD,
	},
	{
		XmcdNnoVuErrMsg, XmcdCNoVuErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_novu), XmRImmediate,
		(XtPointer) STR_NOVU,
	},
	{
		XmcdNtmpdirErrMsg, XmcdCTmpdirErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_tmpdirerr), XmRImmediate,
		(XtPointer) STR_TMPDIRERR,
	},
	{
		XmcdNlibdirErrMsg, XmcdCLibdirErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_libdirerr), XmRImmediate,
		(XtPointer) STR_LIBDIRERR,
	},
	{
		XmcdNnoHelpMsg, XmcdCNoHelpMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nohelp), XmRImmediate,
		(XtPointer) STR_NOHELP,
	},
	{
		XmcdNnoLinkMsg, XmcdCNoLinkMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nolink), XmRImmediate,
		(XtPointer) STR_NOLINK,
	},
	{
		XmcdNnoDbMsg, XmcdCNoDbMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nodb), XmRImmediate,
		(XtPointer) STR_NODB,
	},
	{
		XmcdNnoCfgMsg, XmcdCNoCfgMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_nocfg), XmRImmediate,
		(XtPointer) STR_NOCFG,
	},
	{
		XmcdNnotRomMsg, XmcdCNotRomMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_notrom), XmRImmediate,
		(XtPointer) STR_NOTROM,
	},
	{
		XmcdNnotScsi2Msg, XmcdCNotScsi2Msg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_notscsi2), XmRImmediate,
		(XtPointer) STR_NOTSCSI2,
	},
	{
		XmcdNsendConfirmMsg, XmcdCSendConfirmMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_send), XmRImmediate,
		(XtPointer) STR_SEND,
	},
	{
		XmcdNmailErrMsg, XmcdCMailErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_mailerr), XmRImmediate,
		(XtPointer) STR_MAILERR,
	},
	{
		XmcdNmodeErrMsg, XmcdCModeErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_moderr), XmRImmediate,
		(XtPointer) STR_MODERR,
	},
	{
		XmcdNstatErrMsg, XmcdCStatErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_staterr), XmRImmediate,
		(XtPointer) STR_STATERR,
	},
	{
		XmcdNnodeErrMsg, XmcdCNodeErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_noderr), XmRImmediate,
		(XtPointer) STR_NODERR,
	},
	{
		XmcdNseqFmtErrMsg, XmcdCSeqFmtErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_seqfmterr), XmRImmediate,
		(XtPointer) STR_SEQFMTERR,
	},
	{
		XmcdNdbdirsErrMsg, XmcdCDbdirsErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_dbdirserr), XmRImmediate,
		(XtPointer) STR_DBDIRSERR,
	},
	{
		XmcdNrecovErrMsg, XmcdCRecovErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_recoverr), XmRImmediate,
		(XtPointer) STR_RECOVERR,
	},
	{
		XmcdNmaxErrMsg, XmcdCMaxErrMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_maxerr), XmRImmediate,
		(XtPointer) STR_MAXERR,
	},
	{
		XmcdNsavErrForkMsg, XmcdCSavErrForkMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_saverr_fork), XmRImmediate,
		(XtPointer) STR_SAVERR_FORK,
	},
	{
		XmcdNsavErrSuidMsg, XmcdCSavErrSuidMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_saverr_suid), XmRImmediate,
		(XtPointer) STR_SAVERR_SUID,
	},
	{
		XmcdNsavErrOpenMsg, XmcdCSavErrOpenMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_saverr_open), XmRImmediate,
		(XtPointer) STR_SAVERR_OPEN,
	},
	{
		XmcdNsavErrCloseMsg, XmcdCSavErrCloseMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_saverr_close), XmRImmediate,
		(XtPointer) STR_SAVERR_CLOSE,
	},
	{
		XmcdNsavErrKilledMsg, XmcdCSavErrKilledMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_saverr_killed), XmRImmediate,
		(XtPointer) STR_SAVERR_KILLED,
	},
	{
		XmcdNlnkErrForkMsg, XmcdCLnkErrForkMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_lnkerr_fork), XmRImmediate,
		(XtPointer) STR_LNKERR_FORK,
	},
	{
		XmcdNlnkErrSuidMsg, XmcdCLnkErrSuidMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_lnkerr_suid), XmRImmediate,
		(XtPointer) STR_LNKERR_SUID,
	},
	{
		XmcdNlnkErrLinkMsg, XmcdCLnkErrLinkMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_lnkerr_link), XmRImmediate,
		(XtPointer) STR_LNKERR_LINK,
	},
	{
		XmcdNlnkErrKilledMsg, XmcdCLnkErrKilledMsg,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, str_lnkerr_killed), XmRImmediate,
		(XtPointer) STR_LNKERR_KILLED,
	},
	{
		XmcdNbuttonLabelKey, XmcdCButtonLabelKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, btnlbl_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNlockKey, XmcdCLockKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, lock_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNrepeatKey, XmcdCRepeatKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, repeat_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNshuffleKey, XmcdCShuffleKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, shuffle_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNejectKey, XmcdCEjectKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, eject_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNpowerOffKey, XmcdCPowerOffKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, poweroff_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNdbprogKey, XmcdCDbprogKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, dbprog_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNhelpKey, XmcdCHelpKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, help_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNoptionsKey, XmcdCOptionsKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, options_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNtimeKey, XmcdCTimeKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, time_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNabKey, XmcdCAbKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, ab_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNsampleKey, XmcdCSampleKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, sample_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadKey, XmcdCKeypadKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNplayPauseKey, XmcdCPlayPauseKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, playpause_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNstopKey, XmcdCStopKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, stop_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNprevTrackKey, XmcdCPrevTrackKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, prevtrk_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNnextTrackKey, XmcdCNextTrackKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, nexttrk_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNprevIndexKey, XmcdCPrevIndexKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, previdx_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNnextIndexKey, XmcdCNextIndexKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, nextidx_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNrewKey, XmcdCRewKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, rew_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNffKey, XmcdCFfKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, ff_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey0, XmcdCKeypadNumKey0,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad0_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey1, XmcdCKeypadNumKey1,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad1_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey2, XmcdCKeypadNumKey2,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad2_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey3, XmcdCKeypadNumKey3,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad3_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey4, XmcdCKeypadNumKey4,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad4_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey5, XmcdCKeypadNumKey5,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad5_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey6, XmcdCKeypadNumKey6,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad6_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey7, XmcdCKeypadNumKey7,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad7_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey8, XmcdCKeypadNumKey8,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad8_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadNumKey9, XmcdCKeypadNumKey9,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypad9_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadClearKey, XmcdCKeypadClearKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypadclear_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadEnterKey, XmcdCKeypadEnterKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypadenter_key), XmRImmediate,
		(XtPointer) "",
	},
	{
		XmcdNkeypadCancelKey, XmcdCKeypadCancelKey,
		XmRString, sizeof(String),
		XtOffsetOf(appdata_t, keypadcancel_key), XmRImmediate,
		(XtPointer) "",
	},
};


STATIC XrmOptionDescRec	options[] = {
	{ "-dev",	"*device",	XrmoptionSepArg,	NULL },
	{ "-debug",	"*debugMode",	XrmoptionNoArg,		"True" },
#if defined(SVR4) && defined(sun)
	/* Solaris 2 volume manager auto-startup support */
	{ "-c",		"*device",	XrmoptionSepArg,	NULL },
	{ "-X",		"*exitOnEject",	XrmoptionNoArg,		"True" },
	{ "-o",		"",		XrmoptionNoArg,		"False" },
#endif
};


STATIC String		fallbacks[] = {
	"*mainForm.width: 360",
	"*mainForm.height: 133",
	"*keypadForm.width: 150",
	"*keypadForm.height: 230",
	"*optionsForm.width: 220",
	"*optionsForm.height: 370",
	"*dbprogForm.width: 420",
	"*dbprogForm.height: 440",
	"*extDiscInfoForm.width: 390",
	"*extDiscInfoForm.height: 340",
	"*extTrackInfoForm.width: 390",
	"*extTrackInfoForm.height: 340",
	"*helpForm.width: 390",
	"*helpForm.height: 340",
	"*dirSelectForm.width: 270",
	"*dirSelectForm.height: 230",
	"*linkSelectForm.width: 350",
	"*linkSelectForm.height: 430",
	"*mainForm.checkBox*labelType: PIXMAP",
	"*mainForm.XmPushButton*labelType: PIXMAP",
	"*mainForm*checkBoxFrame*fontList: -*-helvetica-medium-r-*--10-100-*",
	"*mainForm*XmPushButton*fontList: -*-helvetica-medium-r-*--10-100-*",
	"*trackIndicator.fontList: -*-helvetica-medium-o-*--24-240-*",
	"*indexIndicator.fontList: -*-helvetica-bold-o-*--14-140-*",
	"*timeIndicator.fontList: -*-helvetica-medium-o-*--24-240-*",
	"*repeatCountIndicator.fontList: -*-helvetica-bold-o-*--12-120-*",
	"*dbModeIndicator.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*progModeIndicator.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*timeModeIndicator.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*playModeIndicator.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*discTitleIndicator.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*trackTitleIndicator.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*aboutPopup*fontList: -*-times-bold-i-*--24-240-*=chset1, -*-times-bold-i-*--12-120-*=chset2, -*-fixed-medium-r-*--10-100-*=chset3, fixed",
	"*discIdIndicator.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*dirSelectList.fontList: -*-helvetica-medium-r-*--12-120-*",
	"*linkSelectList*fontList: -*-helvetica-medium-r-*--12-120-*=chset1, -*-helvetica-bold-r-*--12-120-*=chset2, fixed",
	"*extDiscInfoLabel.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*extTrackInfoLabel.fontList: -*-helvetica-bold-r-*--12-120-*",
	"*keypadForm*keypadLabel.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*keypadForm*keypadIndicator.fontList: -*-helvetica-bold-o-*--14-140-*",
	"*keypadForm*trackWarpLabel.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*keypadForm*keypadCancelButton.fontList: -*-helvetica-medium-r-*--12-120-*",
	"*keypadForm*fontList: -*-helvetica-medium-r-*--10-100-*",
	"*optionsForm*XmFrame*fontList: -*-helvetica-medium-r-*--10-100-*",
	"*optionsForm*XmLabel*fontList: -*-helvetica-medium-r-*--10-100-*",
	"*optionsForm*balanceCenterButton.fontList: -*-helvetica-medium-r-*--10-100-*",
	"*optionsForm*fontList: -*-helvetica-medium-r-*--12-120-*",
	"*helpForm*XmPushButton*fontList: -*-helvetica-medium-r-*--12-120-*",
	"*XmList.fontList: -*-helvetica-medium-r-*--12-120-*=chset1, -*-helvetica-bold-r-*--12-120-*=chset2, fixed",
	"*XmText.fontList: -*-helvetica-medium-r-*--12-120-*",
	"*XmScale.fontList: 6x10",
	"*fontList: -*-helvetica-medium-r-*--12-120-*",
	"*checkBox*button_0.labelString: disp",
	"*checkBox*button_1.labelString: lock",
	"*checkBox*button_2.labelString: rept",
	"*checkBox*button_3.labelString: shuf",
	"*ejectButton.labelString: eject",
	"*powerOffButton.labelString: quit",
	"*dbprogButton.labelString: cddb\nprog",
	"*helpButton.labelString: help",
	"*optionsButton.labelString: opt",
	"*timeButton.labelString: time",
	"*abButton.labelString: a->b",
	"*sampleButton.labelString: samp",
	"*keypadButton.labelString: kpad",
	"*playPauseButton.labelString: play / pause",
	"*stopButton.labelString: stop",
	"*prevTrackButton.labelString: < track",
	"*nextTrackButton.labelString: track >",
	"*prevIndexButton.labelString: < index",
	"*nextIndexButton.labelString: index >",
	"*rewButton.labelString: << rew",
	"*ffButton.labelString: ff >>",
	"*keypadForm.dialogTitle: Keypad",
	"*keypadLabel.labelString: Direct track access",
	"*keypadNumButton0.labelString: 0",
	"*keypadNumButton1.labelString: 1",
	"*keypadNumButton2.labelString: 2",
	"*keypadNumButton3.labelString: 3",
	"*keypadNumButton4.labelString: 4",
	"*keypadNumButton5.labelString: 5",
	"*keypadNumButton6.labelString: 6",
	"*keypadNumButton7.labelString: 7",
	"*keypadNumButton8.labelString: 8",
	"*keypadNumButton9.labelString: 9",
	"*keypadEnterButton.labelString: Enter",
	"*keypadClearButton.labelString: Clear",
	"*trackWarpLabel.labelString: Track warp",
	"*keypadCancelButton.labelString: Cancel",
	"*optionsForm.dialogTitle: Options",
	"*onLoadLabel.labelString: On Load",
	"*onLoadCheckBox*button_0.labelString: auto lock",
	"*onLoadRadioBox*button_0.labelString: none",
	"*onLoadRadioBox*button_1.labelString: spin down",
	"*onLoadRadioBox*button_2.labelString: auto play",
	"*onExitLabel.labelString: On Exit",
	"*onExitRadioBox*button_0.labelString: none",
	"*onExitRadioBox*button_1.labelString: auto stop",
	"*onExitRadioBox*button_2.labelString: auto eject",
	"*onDoneLabel.labelString: On Done",
	"*onDoneCheckBox*button_0.labelString: auto eject",
	"*onEjectLabel.labelString: On Eject",
	"*onEjectCheckBox*button_0.labelString: auto exit",
	"*channelRouteLabel.labelString: Channel routing",
	"*channelRouteRadioBox*button_0.labelString: normal",
	"*channelRouteRadioBox*button_1.labelString: reverse",
	"*channelRouteRadioBox*button_2.labelString: mono L",
	"*channelRouteRadioBox*button_3.labelString: mono R",
	"*channelRouteRadioBox*button_4.labelString: mono L+R",
	"*volTaperLabel.labelString: Volume ctrl taper",
	"*volTaperRadioBox*button_0.labelString: linear",
	"*volTaperRadioBox*button_1.labelString: square",
	"*volTaperRadioBox*button_2.labelString: inverse sqr",
	"*balanceLabel.labelString: Balance",
	"*balanceLeftLabel.labelString: Left",
	"*balanceRightLabel.labelString: Right",
	"*balanceCenterButton.labelString: Center",
	"*resetButton.labelString: Reset",
	"*okButton.labelString: OK",
	"*dbprogForm.dialogTitle: CD Database / Track Program Editor",
	"*aboutButton.labelString: About...",
	"*discTitleLabel.labelString: Disc artist / Title",
	"*discLabel.labelString: Disc",
	"*extDiscInfoButton.labelString: Ext Info...",
	"*trackListLabel.labelString: Track / Time / Title",
	"*timeSelectLabel.labelString: Time",
	"*timeSelectBox*button_0.labelString: total",
	"*timeSelectBox*button_1.labelString: track",
	"*discIdLabel.labelString: Disc ID",
	"*discIdIndicator.labelString: --",
	"*trackTitleLabel.labelString: Track title edit",
	"*trackLabel.labelString: Track",
	"*extTrackInfoButton.labelString: Ext Info...",
	"*programLabel.labelString: Program",
	"*addProgramButton.labelString: Add",
	"*clearProgramButton.labelString: Clear",
	"*playProgramButton.labelString: Play",
	"*programSequenceLabel.labelString: Program sequence",
	"*sendButton.labelString: Send",
	"*saveDatabaseButton.labelString: Save",
	"*linkDatabaseButton.labelString: Link",
	"*loadDatabaseButton.labelString: Load",
	"*dbprogCancelButton.labelString: Cancel",
	"*dirSelectForm.dialogTitle: CD Database Directory Selection",
	"*dirSelectLabel.labelString: Choose one",
	"*dirSelectOkButton.labelString: OK",
	"*dirSelectCancelButton.labelString: Cancel",
	"*linkSelectForm.dialogTitle: CDDB Link Entry Selection",
	"*linkSelectLabel.labelString: Choose one\nAvg diff time, Disc artist / title",
	"*linkSelectOkButton.labelString: OK",
	"*linkSelectCancelButton.labelString: Cancel",
	"*extDiscInfoForm.dialogTitle: Disc Information",
	"*extDiscInfoOkButton.labelString: OK",
	"*extDiscInfoClearButton.labelString: Clear",
	"*extDiscInfoCancelButton.labelString: Cancel",
	"*extTrackInfoForm.dialogTitle: Track Information",
	"*extTrackInfoOkButton.labelString: OK",
	"*extTrackInfoClearButton.labelString: Clear",
	"*extTrackInfoCancelButton.labelString: Cancel",
	"*helpForm.dialogTitle: Xmcd Help",
	"*helpOkButton.labelString: OK",
	"*trackIndicator.foreground: white",
	"*trackIndicator.background: black",
	"*indexIndicator.foreground: white",
	"*indexIndicator.background: black",
	"*timeIndicator.foreground: white",
	"*timeIndicator.background: black",
	"*discTitleIndicator.foreground: black",
	"*discTitleIndicator.background: white",
	"*trackTitleIndicator.foreground: black",
	"*trackTitleIndicator.background: white",
	"*repeatCountIndicator.foreground: white",
	"*repeatCountIndicator.background: black",
	"*dbModeIndicator.foreground: white",
	"*dbModeIndicator.background: black",
	"*progModeIndicator.foreground: white",
	"*progModeIndicator.background: black",
	"*timeModeIndicator.foreground: white",
	"*timeModeIndicator.background: black",
	"*playModeIndicator.foreground: white",
	"*playModeIndicator.background: black",
	"*keypadIndicator.foreground: white",
	"*keypadIndicator.background: black",
	"*foreground: black",
	"*background: white",
	"*mainForm*borderWidth: 0",
	"*keypadForm*borderWidth: 0",
	"*optionsForm*borderWidth: 0",
	"*dbprogForm*borderWidth: 0",
	"*dirselForm*borderWidth: 0",
	"*linkselForm*borderWidth: 0",
	"*helpForm*borderWidth: 0",
	"*borderWidth: 0",
	"*mainForm*highlightThickness: 0",
	"*highlightThickness: 1",
	"*labelType: STRING",
	NULL,
};

#endif	/* __RESOURCE_H__ */

