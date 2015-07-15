/*************************************************************************
*  File Name:     sp_help.c
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Usage:         system_play X Windows Help functions
*
*  Description:   This file contains the Help functions used by the
*                 system_play application to create and  manage Help
*                 Windows. 
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
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>


#include "system.h"
#include "sp_wins.h"

/* Global variables used in help lists and text boxes */

char *help_item_txt[] = {"EXITing the Application",
                         "MASTER PLAYBACK CONTROL WINDOW Overview",
                         "    Menu Bar",
                         "    Master PushButton Actions",
                         "    Master Audio Level Control",
                         "AUDIO/VIDEO SINGLE STREAM CONTROL WINDOW Overview",
                         "    Stream PushButton Actions",
                         "    Stream Audio Level Control",
                         "    Changing Audio/Video Active Stream"};

int help_num_items = XtNumber(help_item_txt);

String exit_app[] = {
     "To EXIT system_play select FILE-EXIT from the menubar",
     "in the Master Playback Control Window.  You may use",
     "[MOUSEBUTTON1] to click on FILE and drag to EXIT, or",
     "[META-F] and select EXIT.\n\n",
     "Note:  You may stop playback without exiting the",
     "application.  However, since only 'paused' streams",
     "may be 'resumed', you have no choice other than to exit",
     "the application and begin again.",
     NULL
   };

String master_ctrl[] = {
     "The MASTER PLAYBACK CONTROL WINDOW provides you with the",
     "ability to control all system streams as a group, change",
     "the overall audio level output, manage the other control",
     "windows, get help, and exit the application.",
     NULL
   };

String menu_bar[] = {
     "The menu options within the Master Control Window are sparse since",
     "most of the control can be accomplished by clicking the PushButtons",
     "provided.  The following Menu options are available:\n",
     "    FILE\n",
     "       Exit - exit the application\n",
     "    WINDOWS\n",
     "       Single Stream Ctrl - forces the Single Stream\n",
     "                            Control Window to be the\n",
     "                            focus window (re-opens if\n",
     "                            it was closed)\n",
     "    HELP\n",
     "       Index - provides interactive index on help topics\n",
     NULL
   };

String master_push_buttons[] = {
     "PushButton options within the Master Control Window control all",
     "active streams simultaneously.  They include:\n",
     "    START ALL - begin playback of all streams\n",
     "    STOP ALL - terminate playback of all streams\n",
     "    PAUSE ALL - pause video streams and\n",
     "                mute audio streams\n",
     "    RESUME ALL - resume playback of all streams\n",
     "                 (unmute audio streams)\n\n",
     "Note that all streams may not appear to begin simultaneously",
     "due to their actual start time offsets within the multiplexed",
     "system stream.\n\n",
     "To select a PushButton, single click [MOUSEBUTTON1].",
     NULL
   };

String master_audio_level[] = {
     "The Master Audio Level Control is used to change the audio output",
     "level as a whole, the mixed audio streams.  This scale ranges in value",
     "between -10 and 10.  The integer values represent multipliers for",
     "audio levels, rather than a specific value (i.e. Hz).  At application",
     "start-up the audio level multipliers are initialized to 0.\n\n",
     "You can click [MOUSEBUTTON1] and drag the scale button to the desired",
     "level.  Also, you can move incrementally by clicking either to the right",
     "or left of the button to move it in the desired direction.",
     NULL
   };

String single_stream_ctrl[] = {
     "The AUDIO/VIDEO SINGLE STREAM CONTROL WINDOW provides you",
     "with the ability to control each audio and video stream",
     "individually.  You may enter stream numbers directly into",
     "the text boxes provided or choose from lists of currently",
     "active streams via popup list boxes.  A stream is considered",
     "ACTIVE until it is CANCELed.  In addition, you may control",
     "audio levels of the streams individually.\n\n",
     "The window is divided into two sections.  The top section",
     "handles video stream control, and the bottom section handles",
     "audio stream control.",
     NULL
   };

String stream_push_buttons[] = {
     "PushButton options within the Audio/Video Single Stream Control",
     "Window control the most recently selected active stream. (see",
     "Changing Audio/Video Active Stream for details on selecting",
     "the currently active stream).  PushButton options include:\n",
     "    CANCEL - terminate playback of a stream\n",
     "             (one pushbutton for video, one for audio)\n",
     "    PAUSE - pause selected video stream playback\n",
     "    RESUME - resume selected video stream playback\n",
     "    MUTE - mute selected audio stream output\n",
     "    UNMUTE - unmute (resume) selected audio stream output\n",
     "    (DownArrow) - popup list selection box with currently\n",
     "                  active streams (one for video, one for\n",
     "                  audio)\n\n",
     "To select a PushButton, single click [MOUSEBUTTON1].",
     NULL
   };

String stream_audio_level[] = {
     "The Stream Audio Level Control is used to change the audio output",
     "level of the single selected stream.  This scale ranges in value",
     "between -10 and 10.  The integer values represent multipliers for",
     "audio levels, rather than a specific value (i.e. Hz).  At application",
     "start-up the audio level multipliers are initialized to 0.\n\n",
     "You can click [MOUSEBUTTON1] and drag the scale button to the desired",
     "level.  Also, you can move incrementally by clicking either to the right",
     "or left of the button to move it in the desired direction.",
     NULL
   };

String active_stream[] = {
     "To change the active stream that is affected by the controls, you",
     "may enter the stream numbers directly in the text boxes, or click",
     "the DownArrow PushButtons for a list of currently active video",
     "and audio streams.  If you enter an inactive video stream number",
     "or an invalid text string, a message dialog box will appear to",
     "alert you.  Click OK to remove it.\n\n",
     "If you are unsure of the currently active streams, you may use",
     "the selection list method.  Click the DownArrow PushButton and a",
     "selection list box will popup.  You may select any item on the",
     "list by double-clicking [MOUSEBUTTON1] on the item or typing the",
     "full text in the selection area.  Note that selecting a new stream",
     "will automatically cause the value in the Control Window's text box",
     "to change accordingly.",
     NULL
   };

String *help_txt[] = {
     exit_app,
     master_ctrl,
     menu_bar,
     master_push_buttons,
     master_audio_level,
     single_stream_ctrl,
     stream_push_buttons,
     stream_audio_level,
     active_stream
   };

/*************************************************************************
*  Function:      PopupHelpDialog()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget, becomes parent of dialog
*                 int index - index into global help_txt[]
*                 XtPointer call - callback event data (unused)
*                 Globals: help_txt[] - char** array of data to be
*                                       used in scrolling help window 
*
*  Effects:       Creates popup Dialog containing Scrolled Window
*                 with Help Text.
*
*  Description:   This function may be used as a callback function but
*                 is called directly from HelpSelectionCallback with the
*                 index value of help_txt.  This determines the text
*                 string to display.  Widgets are not static but rather
*                 are destroyed upon user "ok".  Thus, several
*                 Help screens may be displayed at once.
*   
**************************************************************************/
void
PopupHelpDialog(w, index, call)
Widget w;
int index;
XtPointer call;
{
    char        *p;
    char        buf[1000];
    int         i;

    Widget      helpDialogShell;
    Widget      helpForm;
    Widget      helpScrolledWindow;
    Widget      helpText;
    Widget      helpSeparator;
    Widget      okPushButton;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;

    argok = False;

    /* set up text to display */
    for (p=buf, i=0; help_txt[index][i]; i++)
      {
        p+=strlen(strcpy(p, help_txt[index][i]));
        if (!isspace(p[-1]))
          *p++ = ' ';  /* space at end of each line */
      }
    *--p = 0; /* no trailing space */


    argcnt = 0;
    XtSetArg(args[argcnt], XmNtitle, "SYSTEM_PLAY HELP"); argcnt++;
    XtSetArg(args[argcnt], XmNdeleteResponse, XmDESTROY); argcnt++;
    XtSetArg(args[argcnt], XmNx, 250); argcnt++;
    XtSetArg(args[argcnt], XmNy, 250); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 450); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 327); argcnt++;
    helpDialogShell = XtCreatePopupShell("helpDialogShell",
                xmDialogShellWidgetClass,
                w,
                args,
                argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground,
             CONVERT(w,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNresizePolicy, XmRESIZE_GROW); argcnt++;
    XtSetArg(args[argcnt], XmNx, 250); argcnt++;
    XtSetArg(args[argcnt], XmNy, 250); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 450); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 327); argcnt++;
    helpForm = XtCreateWidget("helpForm",
                xmFormWidgetClass,
                helpDialogShell,
                args,
                argcnt);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground,
             CONVERT(w,"Dim Gray", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNfontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-140-75-75-*-*-iso8859-1=HELV_14_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelString,
             CONVERT(w,"OK", "XmString", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNrecomputeSize, False); argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 263); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 190); argcnt++;
    XtSetArg(args[argcnt], XmNx, 190); argcnt++;
    XtSetArg(args[argcnt], XmNy, 263); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 60); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 30); argcnt++;
    okPushButton = XtCreateWidget("okPushButton",
                xmPushButtonWidgetClass,
                helpForm,
                args,
                argcnt);

    XtAddCallback(okPushButton, XmNactivateCallback, DestroyPassedWidget, helpDialogShell);
    XtManageChild(okPushButton);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground,
             CONVERT(w,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNrightAttachment, XmATTACH_POSITION); argcnt++;
    XtSetArg(args[argcnt], XmNrightPosition, 100); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 240); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 0); argcnt++;
    XtSetArg(args[argcnt], XmNx, 0); argcnt++;
    XtSetArg(args[argcnt], XmNy, 240); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 450); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 20); argcnt++;
    helpSeparator = XtCreateWidget("helpSeparator",
                xmSeparatorWidgetClass,
                helpForm,
                args,
                argcnt);
    XtManageChild(helpSeparator);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground,
             CONVERT(w,"Light Grey", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtopAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNleftAttachment, XmATTACH_FORM); argcnt++;
    XtSetArg(args[argcnt], XmNtopOffset, 21); argcnt++;
    XtSetArg(args[argcnt], XmNleftOffset, 37); argcnt++;
    XtSetArg(args[argcnt], XmNx, 37); argcnt++;
    XtSetArg(args[argcnt], XmNy, 21); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 381); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 199); argcnt++;
    helpScrolledWindow = XtCreateWidget("helpScrolledWindow",
                xmScrolledWindowWidgetClass,
                helpForm,
                args,
                argcnt);
    XtManageChild(helpScrolledWindow);

    argcnt = 0;
    XtSetArg(args[argcnt], XmNbackground,
             CONVERT(w,"Sky Blue", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Midnight Blue", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNeditMode, XmMULTI_LINE_EDIT); argcnt++;
    XtSetArg(args[argcnt], XmNeditable, False); argcnt++;
    XtSetArg(args[argcnt], XmNfontList,
             CONVERT(w,"-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNscrollVertical, True); argcnt++;
    XtSetArg(args[argcnt], XmNscrollHorizontal, False); argcnt++;
    XtSetArg(args[argcnt], XmNcursorPositionVisible, False); argcnt++;
    XtSetArg(args[argcnt], XmNwordWrap, True); argcnt++;
    XtSetArg(args[argcnt], XmNrows, 10); argcnt++;
    XtSetArg(args[argcnt], XmNvalue, buf); argcnt++;
    helpText = XtCreateWidget("helpText",
                xmTextWidgetClass,
                helpScrolledWindow,
                args,
                argcnt);
    XtManageChild(helpText);

    XtManageChild(helpForm);
}

/*************************************************************************
*  Function:      HelpSelectionCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data
*                 Globals: help_item_txt[] - array of char* containing
*                                            text items in list
*                          help_num_items - # items in help_item_txt[]
*
*  Effects:       Determines the position of the selected string
*                 pops-up a help screen with the corresponding text
*                 displayed.  SelectionDialog is unmanaged.
*                 
*  Description:   This callback function handles user input for the
*                 SelectionDialog created to allow the user to view
*                 online Help screens. The position of the selected
*                 item becomes the index into the char* array of help
*                 screen text values.  A help screen is popped-up.
*   
**************************************************************************/
void
HelpSelectionCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int position;
    char *itemstr;

    XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct*)call;

    switch (cbs->reason)
       {
          case XmCR_NO_MATCH:
            PopupErrorDialog(w, "No match found for video selection!", (XtPointer)0);
            break;
	  case XmCR_OK:
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &itemstr))
              {
                PopupErrorDialog(w, "Could not convert Compound String!", (XtPointer)0);
                return;
              }
            position=0;
            while((strcmp(itemstr,help_item_txt[position])) && (position<help_num_items))
              position++;
            PopupHelpDialog(XtParent(w),position,(XtPointer)0);
            break;
          case XmCR_CANCEL:
            break;
	}

    XtUnmanageChild(w);

}

/*************************************************************************
*  Function:      ShowHelpIndex()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: help_item_txt[] - array of char* containing
*                                            text items in list
*                          help_num_items - # items in help_item_txt[]
*
*  Effects:       Creates and Manages Help SelectionDialog to select
*                 from an index of Help topics.
*                 
*  Description:   This callback function is called when the user presses
*                 the Help-Index menu selection in the Master Control
*                 Window.  It creates and manages a static SelectionDialog
*                 that allows the user to select from an "index" of Help
*                 topics.  The position of the string selected will act
*                 as an index into the array of help text items.
*   
**************************************************************************/
void
ShowHelpIndex(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;

    XmStringTable *item_list;

    static Widget helpDialog = NULL;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;

    argok = FALSE;

    item_list = (XmStringTable*)XtMalloc(help_num_items * sizeof(XmString*));

    if (helpDialog == NULL)
      {
	argcnt = 0;
	XtSetArg(args[argcnt], XmNtitle, "Help Index Listing"); argcnt++;
	XtSetArg(args[argcnt], XmNx, 684); argcnt++;
	XtSetArg(args[argcnt], XmNy, 92); argcnt++;
	XtSetArg(args[argcnt], XmNwidth, 422); argcnt++;
	XtSetArg(args[argcnt], XmNheight, 337); argcnt++;
	XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
	XtSetArg(args[argcnt], XmNlabelFontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD","FontList", 0, &argok)); if (argok) argcnt++;
	XtSetArg(args[argcnt], XmNtextFontList,
             CONVERT(w,"-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12", "FontList", 0, &argok)); if (argok) argcnt++;
	XtSetArg(args[argcnt], XmNlistLabelString,
             CONVERT(w,"Choose Help Item:", "XmString", 0, &argok)); if (argok) argcnt++;
	XtSetArg(args[argcnt], XmNlistVisibleItemCount, 9); argcnt++;
	XtSetArg(args[argcnt], XmNmustMatch, True); argcnt++;

	helpDialog =(Widget)XmCreateSelectionDialog(w, "helpDialog", args, argcnt);

        XtUnmanageChild((Widget)XmSelectionBoxGetChild(helpDialog, XmDIALOG_APPLY_BUTTON));
        XtUnmanageChild((Widget)XmSelectionBoxGetChild(helpDialog, XmDIALOG_HELP_BUTTON));

	XtAddCallback(helpDialog, XmNnoMatchCallback, HelpSelectionCallback, (XtPointer)0);
	XtAddCallback(helpDialog, XmNokCallback, HelpSelectionCallback, (XtPointer)0);
	XtAddCallback(helpDialog, XmNcancelCallback, HelpSelectionCallback, (XtPointer)0);
     

        /* create help selection list */
        for (i=0; i<help_num_items; i++)
          {
            item_list[i] = (XmString*)XmStringCreateSimple(help_item_txt[i]);
          }
        XtVaSetValues(helpDialog,
                      XmNlistItems,     item_list,
                      XmNlistItemCount, help_num_items,
                      NULL);

        for (i=0; i<help_num_items; i++)
            XmStringFree((XmString)item_list[i]);
        XtFree((XmString)item_list);
    }

    XtManageChild(helpDialog);
}

