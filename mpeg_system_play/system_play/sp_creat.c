/*************************************************************************
*  File Name:     sp_creat.c
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Usage:         system_play X Windows Widget Creation routines
*
*  Description:   This file contains the creation routines used by the
*                 main() module of the system_play application. They
*                 are used to create control windows and all child
*                 widgets.
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

/*
 * MOTIF include files needed for widget creation.
 */
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#include <Xm/ArrowB.h>
#include <Xm/Separator.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/SelectioB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "system.h"
#include "sp_wins.h"

/*************************************************************************
*  Function:      BxRegisterConverters()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        none.
*
*  Effects:       none.
*
*  Description:   This function registers all the converters for all
*                 Widgets.
*   
**************************************************************************/
static void
BxRegisterConverters()
{
    XtInitializeWidgetClass(xmTextFieldWidgetClass);
    XtInitializeWidgetClass(xmArrowButtonWidgetClass);
    XtInitializeWidgetClass(xmSeparatorWidgetClass);
    XtInitializeWidgetClass(xmCascadeButtonWidgetClass);
    XtInitializeWidgetClass(xmPushButtonWidgetClass);
    XtInitializeWidgetClass(xmLabelWidgetClass);
    XtInitializeWidgetClass(xmScaleWidgetClass);
    XtInitializeWidgetClass(xmMainWindowWidgetClass);
    XtInitializeWidgetClass(xmFormWidgetClass);
    XtInitializeWidgetClass(xmSelectionBoxWidgetClass);
    XtInitializeWidgetClass(xmRowColumnWidgetClass);
    XtInitializeWidgetClass(topLevelShellWidgetClass);
    XtInitializeWidgetClass(xmDialogShellWidgetClass);
    XtInitializeWidgetClass(xmMenuShellWidgetClass);
    XtInitializeWidgetClass(xmTextWidgetClass);
    XtInitializeWidgetClass(xmScrolledWindowWidgetClass);
}

/*************************************************************************
*  Function:      CreatemasterPlaybackControlWindow()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget parent - calling Widget, used as parent for
*                                 shell Widgets
*
*  Effects:       Create masterPlaybackControlWindow hierarchy of widgets.
*
*  Description:   This function creates the Master Control Window and all
*                 of its child Widgets. 
*   
**************************************************************************/
Widget
CreatemasterPlaybackControlWindow( parent)
Widget parent;
{
    int         i;
    char        videolabel[30];
    char        namestr[25];

    Arg    	args[512];
    Cardinal   	argcnt;
    Boolean   	argok;
    Widget 	retval;
    Widget	masterPlaybackControlWindow;
    Widget	menuBar;
    Widget	fileMenuItem;
    Widget	menuShell;
    Widget	filePulldownMenu;
    Widget	fileExitPushButton;
    Widget	windowsMenuItem;
    Widget	menuShell2;
    Widget	windowsPulldownMenu;
    Widget	windowStreamCtrlPushButton;
    Widget	helpMenuItem;
    Widget	menuShell4;
    Widget	helpPulldownMenu;
    Widget	helpindexPushButton;
    Widget	masterPlaybackForm;
    Widget	masterStartAllPushButton;
    Widget	masterStopAllPushButton;
    Widget	masterPauseAllPushButton;
    Widget	masterResumeAllPushButton;
    Widget	masterAudioLevelScale;
    Widget	masterAudioScaleLabel;

    argok = False;

    BxRegisterConverters();

    argcnt = 0;
    XtSetArg(args[argcnt], XmNx, 8); argcnt++;
    XtSetArg(args[argcnt], XmNy, 24); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 445); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 150); argcnt++;
    masterPlaybackControlWindow = XtCreateWidget("masterPlaybackControlWindow",
		xmMainWindowWidgetClass,
		parent,
		args,
		argcnt);
    retval = masterPlaybackControlWindow;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrowColumnType, XmMENU_BAR); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 445); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 34); argcnt++;
    menuBar = XtCreateWidget("menuBar",
		xmRowColumnWidgetClass,
		masterPlaybackControlWindow,
		args,
		argcnt);
    XtManageChild(menuBar);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"File", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNmnemonic, 'F'); argcnt++;
    XtSetArg(args[argcnt], XmNx, 5); argcnt++;
    XtSetArg(args[argcnt], XmNy, 5); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 41); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 24); argcnt++;
    fileMenuItem = XtCreateWidget("fileMenuItem",
		xmCascadeButtonWidgetClass,
		menuBar,
		args,
		argcnt);

    XtManageChild(fileMenuItem);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNwidth, 1); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 1); argcnt++;
    menuShell = XtCreatePopupShell("menuShell",
		xmMenuShellWidgetClass,
		XtParent(fileMenuItem),
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrowColumnType, XmMENU_PULLDOWN); argcnt++;
    XtSetArg(args[argcnt], XmNx, 0); argcnt++;
    XtSetArg(args[argcnt], XmNy, 0); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 52); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 80); argcnt++;
    filePulldownMenu = XtCreateWidget("filePulldownMenu",
		xmRowColumnWidgetClass,
		menuShell,
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Exit", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    fileExitPushButton = XtCreateWidget("fileExitPushButton",
		xmPushButtonWidgetClass,
		filePulldownMenu,
		args,
		argcnt);

    XtAddCallback(fileExitPushButton, XmNactivateCallback, FileExitCallback, (XtPointer)0);
    XtManageChild(fileExitPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNsubMenuId, filePulldownMenu); argcnt++;
    XtSetValues(fileMenuItem, args, argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Windows", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNmnemonic, 'W'); argcnt++;
    XtSetArg(args[argcnt], XmNx, 119); argcnt++;
    XtSetArg(args[argcnt], XmNy, 5); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 79); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 24); argcnt++;
    windowsMenuItem = XtCreateWidget("windowsMenuItem",
		xmCascadeButtonWidgetClass,
		menuBar,
		args,
		argcnt);

    XtManageChild(windowsMenuItem);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNwidth, 1); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 1); argcnt++;
    menuShell2 = XtCreatePopupShell("menuShell2",
		xmMenuShellWidgetClass,
		XtParent(windowsMenuItem),
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrowColumnType, XmMENU_PULLDOWN); argcnt++;
    XtSetArg(args[argcnt], XmNx, 0); argcnt++;
    XtSetArg(args[argcnt], XmNy, 0); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 136); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 52); argcnt++;
    windowsPulldownMenu = XtCreateWidget("windowsPulldownMenu",
		xmRowColumnWidgetClass,
		menuShell2,
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Stream Control", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNsensitive, False); argcnt++;
    windowStreamCtrlPushButton = XtCreateWidget("windowStreamCtrlPushButton",
		xmPushButtonWidgetClass,
		windowsPulldownMenu,
		args,
		argcnt);

    XtAddCallback(windowStreamCtrlPushButton, XmNactivateCallback, WindowStreamCtrlCallback, (XtPointer)0);
    XtManageChild(windowStreamCtrlPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNsubMenuId, windowsPulldownMenu); argcnt++;
    XtSetValues(windowsMenuItem, args, argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Help", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNmnemonic, 'H'); argcnt++;
    XtSetArg(args[argcnt], XmNx, 198); argcnt++;
    XtSetArg(args[argcnt], XmNy, 5); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 47); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 24); argcnt++;
    helpMenuItem = XtCreateWidget("helpMenuItem",
		xmCascadeButtonWidgetClass,
		menuBar,
		args,
		argcnt);

    /* establish as special "help" type widget on menubar */
    XtVaSetValues(menuBar, XmNmenuHelpWidget, helpMenuItem, NULL);

    XtManageChild(helpMenuItem);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNwidth, 1); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 1); argcnt++;
    menuShell4 = XtCreatePopupShell("menuShell4",
		xmMenuShellWidgetClass,
		XtParent(helpMenuItem),
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrowColumnType, XmMENU_PULLDOWN); argcnt++;
    XtSetArg(args[argcnt], XmNx, 0); argcnt++;
    XtSetArg(args[argcnt], XmNy, 0); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 49); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 28); argcnt++;
    helpPulldownMenu = XtCreateWidget("helpPulldownMenu",
		xmRowColumnWidgetClass,
		menuShell4,
		args,
		argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Index", "XmString", 0, &argok)); if (argok) argcnt++;
    helpindexPushButton = XtCreateWidget("helpindexPushButton",
		xmPushButtonWidgetClass,
		helpPulldownMenu,
		args,
		argcnt);

    XtAddCallback(helpindexPushButton, XmNactivateCallback, ShowHelpIndex ,(XtPointer)0);
    XtManageChild(helpindexPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNsubMenuId, helpPulldownMenu); argcnt++;
    XtSetValues(helpMenuItem, args, argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNresizePolicy, XmRESIZE_GROW); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 445); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 116); argcnt++;
    masterPlaybackForm = XtCreateWidget("masterPlaybackForm",
		xmFormWidgetClass,
		masterPlaybackControlWindow,
		args,
		argcnt);
    XtManageChild(masterPlaybackForm);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Master Audio Level Control:", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 50); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 60); argcnt++;
    XtSetArg(args[argcnt], XmNx, 60); argcnt++;
    XtSetArg(args[argcnt], XmNy, 50); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 325); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    masterAudioScaleLabel = XtCreateWidget("masterAudioScaleLabel",
		xmLabelWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);
    XtManageChild(masterAudioScaleLabel);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNorientation, XmHORIZONTAL); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-100-75-75-*-*-iso8859-1=HELV_10_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshowValue, True); argcnt++;
    XtSetArg(args[argcnt], XmNscaleMultiple, 1); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 75); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 60); argcnt++;
    XtSetArg(args[argcnt], XmNx, 60); argcnt++;
    XtSetArg(args[argcnt], XmNy, 75); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 325); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 35); argcnt++;
    XtSetArg(args[argcnt], XmNmaximum, 10); argcnt++;
    XtSetArg(args[argcnt], XmNminimum,-10); argcnt++;
    XtSetArg(args[argcnt], XmNscaleMultiple, 1); argcnt++;
    XtSetArg(args[argcnt], XmNvalue, 0); argcnt++;
    masterAudioLevelScale = XtCreateWidget("masterAudioLevelScale",
		xmScaleWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);

    XtAddCallback(masterAudioLevelScale, XmNdragCallback, MasterAudioLevelCallback, (XtPointer)0);
    XtManageChild(masterAudioLevelScale);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 15); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Resume All", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 340); argcnt++;
    XtSetArg(args[argcnt], XmNrightOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 340); argcnt++;
    XtSetArg(args[argcnt], XmNy, 15); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    masterResumeAllPushButton = XtCreateWidget("masterResumeAllPushButton",
		xmPushButtonWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);

    XtAddCallback(masterResumeAllPushButton, XmNactivateCallback, ResumeAllCallback, (XtPointer)0);
    XtManageChild(masterResumeAllPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 15); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Pause All", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 235); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    masterPauseAllPushButton = XtCreateWidget("masterPauseAllPushButton",
		xmPushButtonWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);

    XtAddCallback(masterPauseAllPushButton, XmNactivateCallback, PauseAllCallback, (XtPointer)0);
    XtManageChild(masterPauseAllPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 15); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Stop All", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNrightPosition, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 130); argcnt++;
    XtSetArg(args[argcnt], XmNrightOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 130); argcnt++;
    XtSetArg(args[argcnt], XmNy, 15); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    masterStopAllPushButton = XtCreateWidget("masterStopAllPushButton",
		xmPushButtonWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);

    XtAddCallback(masterStopAllPushButton, XmNactivateCallback, StopAllCallback, (XtPointer)0);
    XtManageChild(masterStopAllPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 15); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Start All", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightPosition, 0); argcnt++;
    XtSetArg(args[argcnt], XmNbottomOffset, 284); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 25); argcnt++;
    XtSetArg(args[argcnt], XmNrightOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    masterStartAllPushButton = XtCreateWidget("masterStartAllPushButton",
		xmPushButtonWidgetClass,
		masterPlaybackForm,
		args,
		argcnt);

    XtAddCallback(masterStartAllPushButton, XmNactivateCallback, StartAllCallback, (XtPointer)0);
    XtManageChild(masterStartAllPushButton);

    return( retval );
}

/*************************************************************************
*  Function:      CreatestreamPlaybackForm()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget parent - calling Widget, used as parent for
*                                 shell Widgets
*
*  Effects:       Create streamPlaybackForm hierarchy of widgets.
*
*  Description:   This function creates the Audio/Video Single Stream
*                 Control Window and all of its child Widgets. 
*   
**************************************************************************/
Widget
CreatestreamPlaybackForm( parent)
Widget parent;
{
    Arg    	args[512];
    Cardinal   	argcnt;
    Boolean   	argok;
    Widget 	retval;
    Widget	streamPlaybackForm;
    Widget	videoCancelPushButton;
    Widget	videoPausePushButton;
    Widget	videoResumePushButton;
    Widget	streamAudioLevelScale;
    Widget	streamAudioScaleLabel;
    Widget	audioCancelPushButton;
    Widget	audioMutePushButton;
    Widget	audioUnmutePushButton;
    Widget	videoStreamTextField;
    Widget	audioStreamTextField;
    Widget	streamWinSeparator;
    Widget	audioStreamPromptLabel;
    Widget	videoStreamPromptLabel;
    Widget	videoStreamArrowButton;
    Widget	audioStreamArrowButton;

    argok = False;

    BxRegisterConverters();

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNresizePolicy, XmRESIZE_GROW); argcnt++;
    XtSetArg(args[argcnt], XmNx, 668); argcnt++;
    XtSetArg(args[argcnt], XmNy, 25); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 445); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 240); argcnt++;
    streamPlaybackForm = XtCreateWidget("streamPlaybackForm",
		xmFormWidgetClass,
		parent,
		args,
		argcnt);
    retval = streamPlaybackForm;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Enter Video Stream Number:", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 8); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 50); argcnt++;
    XtSetArg(args[argcnt], XmNx, 50); argcnt++;
    XtSetArg(args[argcnt], XmNy, 8); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 200); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    videoStreamPromptLabel = XtCreateWidget("videoStreamPromptLabel",
		xmLabelWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);
    XtManageChild(videoStreamPromptLabel);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Enter Audio Stream Number:", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 100); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 50); argcnt++;
    XtSetArg(args[argcnt], XmNx, 50); argcnt++;
    XtSetArg(args[argcnt], XmNy, 100); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 200); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    audioStreamPromptLabel = XtCreateWidget("audioStreamPromptLabel",
		xmLabelWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);
    XtManageChild(audioStreamPromptLabel);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 4); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_POSITION); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_POSITION); argcnt++;
    XtSetArg(args[argcnt], XmNleftPosition, 0); argcnt++;
    XtSetArg(args[argcnt], XmNrightPosition, 100); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 80); argcnt++;
    XtSetArg(args[argcnt], XmNbottomOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 0); argcnt++;
    XtSetArg(args[argcnt], XmNy, 80); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 445); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 10); argcnt++;
    streamWinSeparator = XtCreateWidget("streamWinSeparator",
		xmSeparatorWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);
    XtManageChild(streamWinSeparator);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNcolumns, 6); argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 97); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 252); argcnt++;
    XtSetArg(args[argcnt], XmNx, 252); argcnt++;
    XtSetArg(args[argcnt], XmNy, 97); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 33); argcnt++;
    XtSetArg(args[argcnt], XmNmaxLength, 2); argcnt++;
    XtSetArg(args[argcnt], XmNvalue, "0"); argcnt++;
    audioStreamTextField = XtCreateWidget("audioStreamTextField",
		xmTextFieldWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(audioStreamTextField, XmNactivateCallback, ChangeAudioStreamCtrl, (XtPointer)0);
    XtManageChild(audioStreamTextField);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNcolumns, 6); argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 5); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 252); argcnt++;
    XtSetArg(args[argcnt], XmNx, 252); argcnt++;
    XtSetArg(args[argcnt], XmNy, 5); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 33); argcnt++;
    XtSetArg(args[argcnt], XmNmaxLength, 2); argcnt++;
    XtSetArg(args[argcnt], XmNvalue, "0"); argcnt++;
    videoStreamTextField = XtCreateWidget("videoStreamTextField",
		xmTextFieldWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(videoStreamTextField, XmNactivateCallback, ChangeVideoStreamCtrl, (XtPointer)0);
    XtManageChild(videoStreamTextField);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNarrowDirection, XmARROW_DOWN); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 100); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 320); argcnt++;
    XtSetArg(args[argcnt], XmNrightOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 320); argcnt++;
    XtSetArg(args[argcnt], XmNy, 100); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 25); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    audioStreamArrowButton = XtCreateWidget("audioStreamArrowButton",
		xmArrowButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(audioStreamArrowButton, XmNactivateCallback, ShowAudioList, (XtPointer)audioStreamTextField);
    XtManageChild(audioStreamArrowButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNarrowDirection, XmARROW_DOWN); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 8); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 320); argcnt++;
    XtSetArg(args[argcnt], XmNrightOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 320); argcnt++;
    XtSetArg(args[argcnt], XmNy, 8); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 25); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    videoStreamArrowButton = XtCreateWidget("videoStreamArrowButton",
		xmArrowButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(videoStreamArrowButton, XmNactivateCallback, ShowVideoList, (XtPointer)videoStreamTextField);
    XtManageChild(videoStreamArrowButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 135); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Unmute", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 297); argcnt++;
    XtSetArg(args[argcnt], XmNx, 297); argcnt++;
    XtSetArg(args[argcnt], XmNy, 135); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    audioUnmutePushButton = XtCreateWidget("audioUnmutePushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(audioUnmutePushButton, XmNactivateCallback, UnmuteAStreamCallback, (XtPointer)0);
    XtManageChild(audioUnmutePushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 135); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Mute", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 182); argcnt++;
    XtSetArg(args[argcnt], XmNx, 182); argcnt++;
    XtSetArg(args[argcnt], XmNy, 135); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    audioMutePushButton = XtCreateWidget("audioMutePushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(audioMutePushButton, XmNactivateCallback, MuteAStreamCallback, (XtPointer)0);
    XtManageChild(audioMutePushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 135); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Cancel", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 67); argcnt++;
    XtSetArg(args[argcnt], XmNx, 67); argcnt++;
    XtSetArg(args[argcnt], XmNy, 135); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    audioCancelPushButton = XtCreateWidget("audioCancelPushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(audioCancelPushButton, XmNactivateCallback, CancelAStreamCallback, (XtPointer)0);
    XtManageChild(audioCancelPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Stream Audio Level Control:", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 170); argcnt++;
    XtSetArg(args[argcnt], XmNbottomOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 60); argcnt++;
    XtSetArg(args[argcnt], XmNx, 60); argcnt++;
    XtSetArg(args[argcnt], XmNy, 170); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 325); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    streamAudioScaleLabel = XtCreateWidget("streamAudioScaleLabel",
		xmLabelWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);
    XtManageChild(streamAudioScaleLabel);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNorientation, XmHORIZONTAL); argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-100-75-75-*-*-iso8859-1=HELV_10_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshowValue, True); argcnt++;
    XtSetArg(args[argcnt], XmNscaleMultiple, 1); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_NONE); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 200); argcnt++;
    XtSetArg(args[argcnt], XmNbottomOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 60); argcnt++;
    XtSetArg(args[argcnt], XmNx, 60); argcnt++;
    XtSetArg(args[argcnt], XmNy, 200); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 325); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 35); argcnt++;
    XtSetArg(args[argcnt], XmNmaximum, 10); argcnt++;
    XtSetArg(args[argcnt], XmNminimum, -10); argcnt++;
    XtSetArg(args[argcnt], XmNscaleMultiple, 1); argcnt++;
    XtSetArg(args[argcnt], XmNvalue, 0); argcnt++;
    streamAudioLevelScale = XtCreateWidget("streamAudioLevelScale",
		xmScaleWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(streamAudioLevelScale, XmNdragCallback, StreamAudioLevelCallback, (XtPointer)0);
    XtManageChild(streamAudioLevelScale);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 40); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Resume", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNbottomOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 297); argcnt++;
    XtSetArg(args[argcnt], XmNx, 297); argcnt++;
    XtSetArg(args[argcnt], XmNy, 40); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    videoResumePushButton = XtCreateWidget("videoResumePushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(videoResumePushButton, XmNactivateCallback, ResumeVStreamCallback, (XtPointer)0);
    XtManageChild(videoResumePushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 40); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Pause", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 182); argcnt++;
    XtSetArg(args[argcnt], XmNx, 182); argcnt++;
    XtSetArg(args[argcnt], XmNy, 40); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    videoPausePushButton = XtCreateWidget("videoPausePushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(videoPausePushButton, XmNactivateCallback, PauseVStreamCallback, (XtPointer)0);
    XtManageChild(videoPausePushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground, 
             CONVERT(parent,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList, 
             CONVERT(parent,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground, 
             CONVERT(parent,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNshadowThickness, 2); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 40); argcnt++;
    XtSetArg(args[argcnt], XmNlabelString, 
             CONVERT(parent,"Cancel", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftPosition, 0); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 67); argcnt++;
    XtSetArg(args[argcnt], XmNx, 67); argcnt++;
    XtSetArg(args[argcnt], XmNy, 40); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 80); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 25); argcnt++;
    videoCancelPushButton = XtCreateWidget("videoCancelPushButton",
		xmPushButtonWidgetClass,
		streamPlaybackForm,
		args,
		argcnt);

    XtAddCallback(videoCancelPushButton, XmNactivateCallback, CancelVStreamCallback, (XtPointer)0);
    XtManageChild(videoCancelPushButton);

    return( retval );
}


