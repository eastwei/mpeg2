/*************************************************************************
*  File Name:     sp_wins.h
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Usage:         Header file for system_play X Windows management.  
*                 Contains defines, includes, and external function
*                 declarations.
*
*  Description:   This file contains the header information needed
*                 by the X-related functions in system_play.
*
**************************************************************************/

/**************************************************************************
* Copyright (c) 1994 The Multimedia Communications Lab, Boston University.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL BOSTON UNIVERSITY BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BOSTON
* UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* BOSTON UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
* ON AN "AS IS" BASIS, AND BOSTON UNIVERSITY HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*
**************************************************************************/

#include <Xm/Xm.h>

/*
 * Application name and class definition.
 */
#define APP_NAME "sysplay"
#define APP_CLASS "Sysplay"
#define VIDEO_LIST_STRING "Video #"
#define AUDIO_LIST_STRING "Audio #"

#define MAX_VIDEO_STREAMS 16
#define MAX_AUDIO_STREAMS 32

/* Globals */
extern int         currentAudioStream;
extern int         currentVideoStream;
extern Boolean     saveAudioFile;

/*
 * Global Widget variable declarations.
 */
extern Widget AppShell; /* The Main Application Shell */
extern Widget Shell000;
extern Widget MasterPlaybackControlWindow;
extern Widget Shell001;
extern Widget StreamPlaybackForm;

/*  Function prototypes for the widget callback routines 
 *  located in sp_call.c 
 */

extern void DestroyWidget();
extern void DestroyPassedWidget();
extern void PopupInfoDialog();
extern void PopupWarningDialog();
extern void PopupErrorDialog();
extern void InitFileSelectionCallback();
extern void InitFileOpen();
extern void InitAudioFileSaveCallback();
extern void InitAudioFileSave();
extern void PopupSaveAudioQuestionCallback();
extern void PopupSaveAudioQuestion();
extern void FileExitCallback();
extern void WindowStreamCtrlCallback();
extern void WindowVideoCallback();
extern void HelpIndexCallback();
extern void StartAllCallback();
extern void StopAllCallback();
extern void PauseAllCallback();
extern void ResumeAllCallback();
extern void MasterAudioLevelCallback();
extern void CancelVStreamCallback();
extern void PauseVStreamCallback();
extern void ResumeVStreamCallback();
extern void CancelAStreamCallback();
extern void MuteAStreamCallback();
extern void UnmuteAStreamCallback();
extern void StreamAudioLevelCallback();
extern void VideoStreamSelectionCallback();
extern void AudioStreamSelectionCallback();
extern void ShowVideoList();
extern void ShowAudioList();
extern void ChangeVideoStreamCtrl();
extern void ChangeAudioStreamCtrl();


/* Function prototypes for the window creation routines
 * contained in sp_creat.c
 */

extern Widget CreatemasterPlaybackControlWindow();
extern Widget CreatestreamPlaybackForm();

/* Function prototypes for creation of help text and
 * widgets.  Contained in sp_help.c
 */

extern void PopupHelpDialog();
extern void HelpSelectionCallback();
extern void ShowHelpIndex();

/* Function prototypes for BX utilities contained in
 * bxutils.c
 */

extern XtPointer CONVERT();
extern void MENU_POST();
extern void RegisterBxConverters();
