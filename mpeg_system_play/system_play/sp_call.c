/*************************************************************************
*  File Name:     sp_call.c
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Usage:         system_play X Windows Callback routines
*
*  Description:   This file contains the Callback routines used by the
*                 system_play application to manage events under the
*                 X/Motif environment.  Functions contained within this
*                 file perform various operations, such as: handle user
*                 menu choices, handle pushbutton events, handle audio
*                 slider bar movements, popup file dialogs, popup
*                 various warnings/error/information messages.
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

#include "system.h"
#include "sp_wins.h"

/*
 * Standard includes for builtins.
 */
#include <string.h>
#include <ctype.h>

/*************************************************************************
*  Function:      DestroyWidget()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget, will be destroyed
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*
*  Effects:       Destroys passed Widget w.
*
*  Description:   This is a simple utility function that is used to
*                 facilitate destruction of Widgets.  It may be expanded
*                 to track previous Widget existence and Widget
*                 destruction.
*   
**************************************************************************/
void
DestroyWidget(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
     XtDestroyWidget(w);
}

/*************************************************************************
*  Function:      DestroyPassedWidget()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 Widget client - client data, Widget to be destroyed
*                 XtPointer call - callback event data (unused)
*
*  Effects:       Destroys passed Widget client.
*
*  Description:   This is a simple utility function that is used to
*                 facilitate destruction of Widgets. The Widget that
*                 is destroyed is passed to the function and may not
*                 be the parent Widget.  It may be expanded
*                 to track previous Widget existence and Widget
*                 destruction.
*   
**************************************************************************/
void
DestroyPassedWidget(w, client, call)
Widget w;
Widget client;
XtPointer call;
{
     XtDestroyWidget(client);
}

/*************************************************************************
*  Function:      PopupInfoDialog()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget, become parent of dialog
*                 char *text - text string for information message
*                 XmPushButtonCallbackStruct *call - callback event data
*
*  Effects:       Creates popup Information Dialog window containing
*                 message text passed by the client function.
*
*  Description:   This callback function creates a generic Info Message
*                 Dialog Box that is used to display messages passed by
*                 a client function.  The parent Widget used is the 
*                 calling Widget.
*   
**************************************************************************/
void
PopupInfoDialog(w, text, call)
Widget w;
char *text;
XmPushButtonCallbackStruct *call;
{
     Widget dialog;
     XmString xm_string;

     Arg         args[512];
     Cardinal    argcnt;
     Boolean     argok;

     argok = False;

     /* set label for dialog */
     xm_string = XmStringCreateSimple(text);

     /* make child of w */
     argcnt = 0;
     XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNlabelFontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD","FontList", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNtitle, "INFORMATION"); argcnt++;
     XtSetArg(args[argcnt], XmNmessageString, xm_string); argcnt++;

     dialog =(Widget) XmCreateInformationDialog(w, "info", args, argcnt);

     XmStringFree(xm_string);
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

     XtAddCallback(dialog, XmNokCallback, DestroyWidget, (XtPointer)0);
     XtManageChild(dialog);
}

/*************************************************************************
*  Function:      PopupWarningDialog()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget, become parent of dialog
*                 char *text - text string for information message
*                 XmPushButtonCallbackStruct *call - callback event data
*
*  Effects:       Creates popup Warning Dialog window containing
*                 message text passed by the client function.
*
*  Description:   This callback function creates a generic Warning Message
*                 Dialog Box that is used to display messages passed by
*                 a client function.  The parent Widget used is the 
*                 calling Widget.
*   
**************************************************************************/
void
PopupWarningDialog(w, text, call)
Widget w;
char *text;
XmPushButtonCallbackStruct *call;
{
     Widget dialog;
     XmString xm_string;

     Arg         args[512];
     Cardinal    argcnt;
     Boolean     argok;

     argok = False;

     /* set label for dialog */
     xm_string = XmStringCreateSimple(text);

     /* make child of w */
     argcnt = 0;
     XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNlabelFontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD","FontList", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNtitle, "WARNING"); argcnt++;
     XtSetArg(args[argcnt], XmNmessageString, xm_string); argcnt++;

     dialog =(Widget) XmCreateWarningDialog(w, "warning", args, argcnt);

     XmStringFree(xm_string);
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

     XtAddCallback(dialog, XmNokCallback, DestroyWidget, (XtPointer)0);
     XtManageChild(dialog);
}

/*************************************************************************
*  Function:      PopupErrorDialog()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget, become parent of dialog
*                 char *text - text string for information message
*                 XtPointer call - callback event data
*
*  Effects:       Creates popup Error Dialog window containing
*                 message text passed by the client function.
*
*  Description:   This callback function creates a generic Error Message
*                 Dialog Box that is used to display messages passed by
*                 a client function.  The parent Widget used is the 
*                 calling Widget.
*   
**************************************************************************/
void
PopupErrorDialog(w, text, call)
Widget w;
char *text;
XtPointer call;
{
     Widget dialog;
     XmString xm_string;

     Arg         args[512];
     Cardinal    argcnt;
     Boolean     argok;

     argok = False;

     /* set label for dialog */
     xm_string = XmStringCreateSimple(text);

     /* make child of w */
     argcnt = 0;
     XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNlabelFontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD","FontList", 0, &argok)); if (argok) argcnt++;
     XtSetArg(args[argcnt], XmNtitle, "ERROR"); argcnt++;
     XtSetArg(args[argcnt], XmNmessageString, xm_string); argcnt++;

     dialog =(Widget) XmCreateErrorDialog(w, "error", args, argcnt);

     XmStringFree(xm_string);
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

     XtAddCallback(dialog, XmNokCallback, DestroyWidget, (XtPointer)0);
     XtManageChild(dialog);
}

/*************************************************************************
*  Function:      InitFileSelectionCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, used as pointer to 
*                                    Boolean variable (fileok) that 
*                                    checks valid user input
*                 XtPointer call - callback event data
*
*  Effects:       Creates full path string for creation of input file
*                 descriptor.
*
*  Description:   This callback function handles user input for the
*                 FileSelectionDialog created to allow the user to select
*                 the system-level input stream file on application
*                 start-up.
*   
**************************************************************************/
void
InitFileSelectionCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
     char *fname;

     Boolean *fileok = (Boolean*)client;
     XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct*)call;

     switch (cbs->reason)
       {
          case XmCR_NO_MATCH:
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &fname))
              {
                printf("\nCould not convert Compound String!!!");
                exit(1);
              }
            printf("\nNo match found for: %s", fname);
            XtFree(fname);
            break;
          case XmCR_OK:
            /* change global "filename" */
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &fname))
              {
                printf("\nCould not convert Compound String!!!");
                exit(1);
              }
            strcpy(filename,fname);
            printf("\nFile selected is: %s\n", filename);
            XtDestroyWidget(w);
            *fileok = True;
            XtFree(fname);
            break;
          case XmCR_CANCEL:
            exit(1);
            break;
          case XmCR_APPLY:
            break;
       }
}

/*************************************************************************
*  Function:      InitFileOpen()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        XtAppContext app - Application context must be passed
*                                    for handling event loop
*                 Globals: AppShell - parent shell for FileSelection
*                                     Dialog Box 
*
*  Effects:       Creates popup FileSelectionDialog and begins event
*                 loop to handle user input until valid selection is made.
*
*  Description:   This function creates the FileSelectionDialog which
*                 allows the user to select the system-level input stream
*                 file on application start-up. It also loops through
*                 event queue until a valid filename is entered or the
*                 user Cancels the operation.
*   
**************************************************************************/
void
InitFileOpen(app)
XtAppContext app;
{
    Widget fileSelectionDialog;
    XEvent event;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;
    Boolean     fileok;

    argok = False;
    fileok = False;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNtitle, "System Stream File Selection"); argcnt++;
    XtSetArg(args[argcnt], XmNx, 311); argcnt++;
    XtSetArg(args[argcnt], XmNy, 126); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 429); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 503); argcnt++;
    XtSetArg(args[argcnt], XmNforeground, CONVERT(AppShell,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelFontList, CONVERT(AppShell,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtextFontList, CONVERT(AppShell,"-*-courier-medium-r-*-*-*-120-75-75-*-*-iso8859-1=COUR_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlistVisibleItemCount, 19); argcnt++;
    XtSetArg(args[argcnt], XmNmustMatch, True); argcnt++;
    fileSelectionDialog = (Widget)XmCreateFileSelectionDialog(AppShell, "fileSelectionDialog", args, argcnt);

    XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionDialog, XmDIALOG_HELP_BUTTON));          

    XtAddCallback(fileSelectionDialog, XmNnoMatchCallback, InitFileSelectionCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNokCallback, InitFileSelectionCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNapplyCallback, InitFileSelectionCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNcancelCallback, InitFileSelectionCallback, (XtPointer)&fileok);

    XtManageChild(fileSelectionDialog);

    while (fileok == False)
      {
        XtAppNextEvent(app, &event);
        XtDispatchEvent(&event);
      }    
}

/*************************************************************************
*  Function:      InitAudioFileSaveCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, used as pointer to 
*                                    Boolean variable (fileok) that 
*                                    checks valid user input
*                 XtPointer call - callback event data
*
*  Effects:       Creates full path string for creation of output file
*                 descriptor.
*
*  Description:   This callback function handles user input for the
*                 FileSelectionDialog created to allow the user to select
*                 the audio stream output file on application
*                 start-up.
*   
**************************************************************************/
void
InitAudioFileSaveCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
     char *fname;

     Boolean *fileok = (Boolean*)client;
     XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct*)call;

     switch (cbs->reason)
       {
          case XmCR_NO_MATCH:
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &fname))
              {
                printf("\nCould not convert Compound String!!!");
                exit(1);
              }
            strcpy(audiosavename,fname);
            printf("\nNo match found, file created is: %s\n", audiosavename);
            XtDestroyWidget(w);
            *fileok = True;
            XtFree(fname);
            break;
          case XmCR_OK:
            /* change global "filename" */
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &fname))
              {
                printf("\nCould not convert Compound String!!!");
                exit(1);
              }
            strcpy(audiosavename,fname);
            printf("\nInput file selected is: %s\n", audiosavename);
            XtDestroyWidget(w);
            *fileok = True;
            XtFree(fname);
            break;
          case XmCR_CANCEL:
            exit(1);
            break;
          case XmCR_APPLY:
            break;
       }
}

/*************************************************************************
*  Function:      InitAudioFileSave()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        XtAppContext app - Application context must be passed
*                                    for handling event loop
*                 Globals: AppShell - parent shell for FileSelection
*                                     Dialog Box 
*
*  Effects:       Creates popup FileSelectionDialog and begins event
*                 loop to handle user input until valid selection is made.
*
*  Description:   This function creates the FileSelectionDialog which
*                 allows the user to select the audio stream output
*                 file on application start-up. It also loops through
*                 event queue until a valid filename is entered or the
*                 user Cancels the operation.
*   
**************************************************************************/
void
InitAudioFileSave(app)
XtAppContext app;
{
    Widget fileSelectionDialog;
    XEvent event;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;
    Boolean     fileok;

    argok = False;
    fileok = False;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNtitle, "Audio Stream Saved File Selection"); argcnt++;
    XtSetArg(args[argcnt], XmNx, 311); argcnt++;
    XtSetArg(args[argcnt], XmNy, 126); argcnt++;
    XtSetArg(args[argcnt], XmNwidth, 429); argcnt++;
    XtSetArg(args[argcnt], XmNheight, 503); argcnt++;
    XtSetArg(args[argcnt], XmNforeground, CONVERT(AppShell,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelFontList, CONVERT(AppShell,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtextFontList, CONVERT(AppShell,"-*-courier-medium-r-*-*-*-120-75-75-*-*-iso8859-1=COUR_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlistVisibleItemCount, 19); argcnt++;
    XtSetArg(args[argcnt], XmNmustMatch, True); argcnt++;
    fileSelectionDialog = (Widget)XmCreateFileSelectionDialog(AppShell, "fileSelectionDialog", args, argcnt);

    XtUnmanageChild((Widget)XmFileSelectionBoxGetChild(fileSelectionDialog, XmDIALOG_HELP_BUTTON));          

    XtAddCallback(fileSelectionDialog, XmNnoMatchCallback, InitAudioFileSaveCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNokCallback, InitAudioFileSaveCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNapplyCallback, InitAudioFileSaveCallback, (XtPointer)&fileok);
    XtAddCallback(fileSelectionDialog, XmNcancelCallback, InitAudioFileSaveCallback, (XtPointer)&fileok);

    XtManageChild(fileSelectionDialog);

    while (fileok == False)
      {
        XtAppNextEvent(app, &event);
        XtDispatchEvent(&event);
      }    

}

/*************************************************************************
*  Function:      PopupSaveAudioQuestionCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, used as pointer to 
*                                    Boolean variable (answered) that 
*                                    checks valid user input
*                 XtPointer call - callback event data
*                 Globals: saveAudioFile - Boolean variable tracks
*                                          if audio is to be saved to
*                                          a file
*
*  Effects:       sets global variable saveAudioFile and destroys itself
*
*  Description:   This callback function sets the global variable
*                 saveAudioFile based on the user selection YES or NO.
*   
**************************************************************************/
void
PopupSaveAudioQuestionCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    Boolean *answered = (Boolean*)client;
    XmAnyCallbackStruct *cbs = (XmAnyCallbackStruct*)call;

     switch (cbs->reason)
       {
          case XmCR_OK:
            *answered = True;
            saveAudioFile = True;
            XtDestroyWidget(w);
            break;
          case XmCR_CANCEL:
            *answered = True;
            saveAudioFile = False;
            XtDestroyWidget(w);
            break;
       }
}

/*************************************************************************
*  Function:      PopupSaveAudioQuestion()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        XtAppContext app - Application context must be passed
*                                    for handling event loop
*                 Globals: AppShell - parent shell for FileSelection
*                                     Dialog Box 
*
*  Effects:       Creates popup Question Dialog and begins event
*                 loop to handle user input until "Yes" or "No" selected.
*
*  Description:   This function creates the QuestionDialog which
*                 asks the user if he/she wishes to save audio output to
*                 a file on applicatin start-up. It also loops through
*                 event queue until an answer is chosen (Yes or No).
*   
**************************************************************************/
void
PopupSaveAudioQuestion(app)
XtAppContext app;
{
     Widget dialog;
     XEvent event;
     XmString xm_string;
     XmString xm_oklabel;
     XmString xm_cancellabel;

     Arg         args[512];
     Cardinal    argcnt;
     Boolean     argok;
     Boolean     answered;

     argok = False;
     answered = False;

     /* set label for dialog */
     xm_string = XmStringCreateSimple("Do you want to save audio stream to a file?");
     xm_oklabel = XmStringCreateSimple("YES");
     xm_cancellabel = XmStringCreateSimple("NO");

     /* make child of w */
    argcnt = 0;
    XtSetArg(args[argcnt], XmNtitle, ""); argcnt++;
    XtSetArg(args[argcnt], XmNx, 311); argcnt++;
    XtSetArg(args[argcnt], XmNy, 126); argcnt++;
    XtSetArg(args[argcnt], XmNforeground, CONVERT(AppShell,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNlabelFontList, CONVERT(AppShell,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD", "FontList", 0, &argok)); if (argok) argcnt++;
    XtSetArg(args[argcnt], XmNtextFontList, CONVERT(AppShell,"-*-courier-medium-r-*-*-*-120-75-75-*-*-iso8859-1=COUR_12_NORM", "FontList", 0, &argok)); if (argok) argcnt++;

     XtSetArg(args[argcnt], XmNmessageString, xm_string); argcnt++;

     XtSetArg(args[argcnt], XmNokLabelString, xm_oklabel); argcnt++;
     XtSetArg(args[argcnt], XmNcancelLabelString, xm_cancellabel); argcnt++;

     dialog =(Widget) XmCreateQuestionDialog(AppShell, "question", args, argcnt);

     XmStringFree(xm_string);
     XmStringFree(xm_oklabel);
     XmStringFree(xm_cancellabel); 

     XtUnmanageChild((Widget)XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

     XtAddCallback(dialog, XmNokCallback, PopupSaveAudioQuestionCallback, (XtPointer)&answered);

     XtAddCallback(dialog, XmNcancelCallback, PopupSaveAudioQuestionCallback, (XtPointer)&answered);

     XtManageChild(dialog);

     while (answered == False)
      {
        XtAppNextEvent(app, &event);
        XtDispatchEvent(&event);
      }    
}

/*************************************************************************
*  Function:      FileExitCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, integer exit value
*                 XtPointer call - callback event data (unused)
*                 Globals: parent_pid - parent process that spawns all
*                                       audio/video processes
*                          pid[] - process id array for audio/video procs
*                          M - # video processes
*                          N - # audio processes
*  Effects:       Exits the application.
*
*  Description:   This callback function expects an integer to be passed
*                 in client data. It first cleans up the application by
*                 killing any live audio/video processes.  Then, it calls
*                 the exit() system call with the integer value as the 
*                 argument to the function.
*   
**************************************************************************/
void
FileExitCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;
    int exitValue = (int)client;


    if(kill(parent_pid,SIGINT) < 0) {       /* demux terminates cleanly*/
      perror("could not send sigint");
    }

   /* causes immediate termination of audio/video processes*/
   for(i=0;i<(N+M+1);i++){ 
     kill(pid[i],SIGINT);    /* also kills mixer*/  
   }

    exit(exitValue);
}

/*************************************************************************
*  Function:      WindowStreamCtrlCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*
*  Effects:       Forces Single Stream Control Window to be focus window.
*
*  Description:   This is the callback function for the Windows-Stream
*                 Control menu option. It forces the Audio/Video 
*                 Single Stream Control Window to become the focus window.
*                 If it was closed,it becomes Managed.
*   
**************************************************************************/
void
WindowStreamCtrlCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    /* not implemented in current release */
}

/*************************************************************************
*  Function:      WindowVideoCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*
*  Effects:       none.
*
*  Description:   This is the callback function meant to be used
*                 for the Windows-Video Stream-Video #X menu option. 
*                 It forces the selected Video window to become the
*                 focus window. (NOT IMPLEMENTED IN THIS VERSION)
*   
**************************************************************************/
void
WindowVideoCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* not implemented in current release */
}

/*************************************************************************
*  Function:      HelpIndexCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*
*  Effects:       Forces Single Stream Control Window to be focus window.
*
*  Description:   This callback function for the Help-Index menu option.
*                 It creates a popup Selection Dialog containing a list of
*                 Help topics.
*   
**************************************************************************/
void
HelpIndexCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{

}

/*************************************************************************
*  Function:      StartAllCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: master_state - master control "state"
*                          pid[] - process id array for audio/video procs
*                          M - # video processes
*                          N - # audio processes
*
*  Effects:       Begins playback of all streams.
*
*  Description:   This callback function is for the Start All PushButton
*                 located in the Master Playback Control Window.
*                 On application start-up, playback does not begin until
*                 the user presses this button. It allows time for the
*                 user to set the output windows on the screen as
*                 preferred.  Note that playback of all streams may not
*                 appear to be simultaneous due to their relative offset
*                 times in the encoded system-level stream.
*   
**************************************************************************/
void
StartAllCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;    
    if(master_state == STOP){
     master_state = PLAY;
     for(i=N;i<(N+M);i++){
	if ( kill(pid[i],SIGUSR2) < 0) {
	  perror( "couldn't send usr2 signal");
	}

     } /* end for i */
    }
}

/*************************************************************************
*  Function:      StopAllCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: master_state - master control "state"
*                          pid[] - process id array for audio/video procs
*                          M - # video processes
*                          N - # audio processes
*
*  Effects:       Terminates playback of all streams.
*
*  Description:   This callback function is for the Stop All PushButton
*                 located in the Master Playback Control Window. It
*                 allows the user to quickly terminate playback of all
*                 streams virtually simultaneously.
*   
**************************************************************************/
void
StopAllCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
  int i;
  if(master_state == PLAY){
    master_state = STOP;
    for( i=N; i<(N+M);i++){
      if ( kill(pid[i],SIGUSR2) < 0) {
       perror( "couldn't send stop signal");
      }
    } /* end for i */
  }
}

/*************************************************************************
*  Function:      PauseAllCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - control state of single stream
*                          pid[] - process id array for audio/video procs
*                          M - # video processes
*                          N - # audio processes
*
*  Effects:       Pauses playback of all streams (audio is Muted).
*
*  Description:   This callback function is for the Pause All PushButton
*                 located in the Master Playback Control Window. It
*                 allows the user to quickly pause/mute playback of all
*                 streams virtually simultaneously.
*   
**************************************************************************/
void
PauseAllCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;
   
    /*assumes in master state play*/
    for(i=0;i<(N+M);i++){
      if(adi->pstatus[i] == PLAY){
        adi->pstatus[i] = PAUSE;   /* PAUSE = MUTE for audio*/
        if(i >= N){
          if ( kill(pid[i],SIGUSR1) < 0) {
	    perror( "couldn't send usr1 signal");
          }
        }
      }
    } 
}

/*************************************************************************
*  Function:      ResumeAllCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - control state of single stream
*                          pid[] - process id array for audio/video procs
*                          M - # video processes
*                          N - # audio processes
*
*  Effects:       Resumes playback of all streams (audio is Unmuted).
*
*  Description:   This callback function is for the Resume All PushButton
*                 located in the Master Playback Control Window. It
*                 allows the user to quickly resume/unmute playback of all
*                 streams virtually simultaneously.
*   
**************************************************************************/
void
ResumeAllCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;

    /* assumes in master state play*/
    for(i=0;i<(N+M);i++){
      if(adi->pstatus[i] == PAUSE){
        adi->pstatus[i] = PLAY;   /* PAUSE = MUTE for audio*/
        if(i >= N){
          if ( kill(pid[i],SIGUSR1) < 0) {
       	    perror( "couldn't send usr1 signal");
          }
        }
      }
    }    

}

/*************************************************************************
*  Function:      MasterAudioLevelCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data, used to get
*                                  changed value from scale Widget
*                 Globals: adi->master_volume - master volume level
*
*  Effects:       Changes overall audio volume level to user selection.
*
*  Description:   This callback function is for the Master Audio Level
*                 Scale located in the Master Playback Control Window.
*                 It allows the user to change volume levels
*                 by moving the scale to +/- integer values. The integer
*                 value is stored in the master_volume global variable,
*                 which acts as a multiplier for audio output.
*   
**************************************************************************/
void
MasterAudioLevelCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct*)call;

    adi->master_volume = (int)cbs->value;
}

/*************************************************************************
*  Function:      CancelVStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentVideoStream - currently selected video
*                                               stream to which button
*                                               presses apply
*                          N - # audio processes
*                              (video processes are listed after
*                               all audio processes in adi->pstatus[]
*
*  Effects:       Terminates playback of currently selected video stream.
*
*  Description:   This callback function is for the Cancel PushButton
*                 located in the Video section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 terminate playback of the currently selected video
*                 stream.
*   
**************************************************************************/
void
CancelVStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentVideoStream +N] != CANCEL){
      adi->pstatus[currentVideoStream + N] = CANCEL;
      if ( kill(pid[currentVideoStream+ N],SIGINT) < 0) {     
        perror( "couldn't send SIGINT");
      }
    }
}

/*************************************************************************
*  Function:      PauseVStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentVideoStream - currently selected video
*                                               stream to which button
*                                               presses apply
*                          N - # audio processes
*                              (video processes are listed after
*                               all audio processes in adi->pstatus[]
*
*  Effects:       Pauses playback of currently selected video stream.
*
*  Description:   This callback function is for the Pause PushButton
*                 located in the Video section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 pause playback of the currently selected video
*                 stream.
*   
**************************************************************************/
void
PauseVStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentVideoStream +N] == PLAY){
      adi->pstatus[currentVideoStream + N] = PAUSE;
  
      if ( kill(pid[currentVideoStream+ N],SIGUSR1) < 0) {     
        perror( "couldn't send SIGUSR1");
      }
    }
}

/*************************************************************************
*  Function:      ResumeVStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentVideoStream - currently selected video
*                                               stream to which button
*                                               presses apply
*                          N - # audio processes
*                              (video processes are listed after
*                               all audio processes in adi->pstatus[]
*
*  Effects:       Resumes playback of currently selected video stream.
*
*  Description:   This callback function is for the Resume PushButton
*                 located in the Video section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 resume playback of the currently selected video
*                 stream.
*   
**************************************************************************/
void
ResumeVStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentVideoStream +N] == PAUSE){
      adi->pstatus[currentVideoStream + N] = PLAY;
      if ( kill(pid[currentVideoStream+ N],SIGUSR1) < 0) {     
        perror( "couldn't send SIGUSR1");
      }
    }
}

/*************************************************************************
*  Function:      CancelAStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentAudioStream - currently selected audio
*                                               stream to which button
*                                               presses apply
*
*  Effects:       Cancels playback of currently selected audio stream.
*
*  Description:   This callback function is for the Cancel PushButton
*                 located in the Audio section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 terminate playback of the currently selected audio
*                 stream.
*   
**************************************************************************/
void
CancelAStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentAudioStream] != CANCEL){
      adi->pstatus[currentAudioStream] = CANCEL;
      if ( kill(pid[currentAudioStream],SIGINT) < 0) { 
        perror( "couldn't send SIGINT");
      }
    }
}

/*************************************************************************
*  Function:      MuteAStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentAudioStream - currently selected audio
*                                               stream to which button
*                                               presses apply
*
*  Effects:       Mutes playback of currently selected audio stream.
*
*  Description:   This callback function is for the Mute PushButton
*                 located in the Audio section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 mute playback of the currently selected audio
*                 stream.
*   
**************************************************************************/
void
MuteAStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentAudioStream] == PLAY){
      adi->pstatus[currentAudioStream] = MUTE;  
    }	
}

/*************************************************************************
*  Function:      UnmuteAStreamCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          pid[] - process id array for audio/video procs
*                          currentAudioStream - currently selected audio
*                                               stream to which button
*                                               presses apply
*
*  Effects:       Unmutes playback of currently selected audio stream.
*
*  Description:   This callback function is for the Unmute PushButton
*                 located in the Audio section of the Audio/Video Single 
*                 Stream Control Window. It allows the user to quickly 
*                 unmute playback of the currently selected audio
*                 stream.
*   
**************************************************************************/
void
UnmuteAStreamCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    if(adi->pstatus[currentAudioStream] == MUTE){
      adi->pstatus[currentAudioStream] = PLAY;
    }
}

/*************************************************************************
*  Function:      StreamAudioLevelCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data, used to get
*                                  changed value from scale Widget
*                 Globals: adi->audio_weight[] - weighting multiplier
*                                                for volume levels of
*                                                individual streams.
*                          currentAudioStream - currently selected audio
*                                               stream to which scale
*                                               movements apply
*
*  Effects:       Changes currently selected audio stream volume level
*                 to user selection.
*
*  Description:   This callback function is for the Single Stream Audio
*                 Level Scale located in the Audio/Video Single Stream
*                 Playback Control Window. It allows the user to change 
*                 volume levels by moving the scale to +/- integer 
*                 values. The integer value is stored as an audio 
*                 weighting factor (multiplier) for each stream separately.
*   
**************************************************************************/
void
StreamAudioLevelCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct*)call;

    adi->audio_weight[currentAudioStream] = (int)cbs->value;
}

/*************************************************************************
*  Function:      VideoStreamSelectionCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, cast to Widget that
*                                    represents Text Box Widget so that
*                                    value chosen can be reset in Text
*                                    Widget on display
*                 XtPointer call - callback event data
*                 Globals: currentVideoStream - currently selected video
*                                               stream to which to which
*                                               button presses apply
*
*  Effects:       Resets the currentVideoStream and displays video
*                 stream number in Text Widget.
*                 
*  Description:   This callback function handles user input for the
*                 SelectionDialog created to allow the user to change
*                 the current video stream selected. The selected stream
*                 becomes the focus stream for all pushbutton actions.
*   
**************************************************************************/
void
VideoStreamSelectionCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    char *itemstr;
    char *txtptr;
    
    Widget *txtwidget = (Widget*)client;

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
            txtptr = itemstr+(sizeof(VIDEO_LIST_STRING)-1);
            currentVideoStream = atoi(txtptr);
            XmTextSetString(*txtwidget, txtptr);
            XtFree(itemstr);
            break;
          case XmCR_CANCEL:
            break;
	}

    XtUnmanageChild(w);
}

/*************************************************************************
*  Function:      AudioStreamSelectionCallback()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data, cast to Widget that
*                                    represents Text Box Widget so that
*                                    value chosen can be reset in Text
*                                    Widget on display
*                 XtPointer call - callback event data
*                 Globals: currentAudioStream - currently selected audio
*                                               stream to which to which
*                                               button presses apply
*
*  Effects:       Resets the currentAudioStream and displays audio
*                 stream number in Text Widget.
*                 
*  Description:   This callback function handles user input for the
*                 SelectionDialog created to allow the user to change
*                 the current audio stream selected. The selected stream
*                 becomes the focus stream for all pushbutton and 
*                 audio level scale actions.
*   
**************************************************************************/
void
AudioStreamSelectionCallback(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    char *itemstr;
    char *txtptr;

    Widget *txtwidget = (Widget*)client;

    XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct*)call;

    switch (cbs->reason)
       {
          case XmCR_NO_MATCH:
            PopupErrorDialog(w, "No match found for audio selection!", (XtPointer)0);
            break;
	  case XmCR_OK:
            if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &itemstr))
              {
                PopupErrorDialog(w, "Could not convert Compound String!", (XtPointer)0);
                return;
              }
            txtptr = itemstr+(sizeof(AUDIO_LIST_STRING)-1);
            currentAudioStream = atoi(txtptr);
            XmTextSetString(*txtwidget, txtptr);
            XtFree(itemstr);
            break;
          case XmCR_CANCEL:
            break;
	}

    XtUnmanageChild(w);
}

/*************************************************************************
*  Function:      ShowVideoList()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          M - # of video streams
*                          N - # audio streams
*                              (video processes are listed after
*                               all audio processes in adi->pstatus[]
*
*  Effects:       Creates and Manages Video SelectionDialog to select
*                 current focus stream for video stream control.
*                 
*  Description:   This callback function is called when the user presses
*                 the Arrow PushButton in the Video Section of the Audio/
*                 Video Single Stream Control Window.  It creates and
*                 manages a static SelectionDialog that allows the user
*                 to select from "active" video streams and change the
*                 current focus stream for video control. Any stream
*                 that has not been CANCELed is considered is valid
*                 (i.e. PAUSED streams are ok)
*   
**************************************************************************/
void
ShowVideoList(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;
    int numitems;
    char namestr[20];
    XmString *itemname;

    Widget *txtwidget = (Widget*)client;

    static Widget videoStreamDialog = NULL;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;

    argok = FALSE;

    itemname = (XmString*)XtMalloc(M * sizeof(XmString));

    if (videoStreamDialog == NULL)
      {
	argcnt = 0;
	XtSetArg(args[argcnt], XmNtitle, "Active Video Streams"); argcnt++;
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
             CONVERT(w,"Choose Video Stream:", "XmString", 0, &argok)); if (argok) argcnt++;
	XtSetArg(args[argcnt], XmNlistVisibleItemCount, 9); argcnt++;
	XtSetArg(args[argcnt], XmNmustMatch, True); argcnt++;

	videoStreamDialog =(Widget)XmCreateSelectionDialog(w, "videoStreamDialog", args, argcnt);

        XtUnmanageChild((Widget)XmSelectionBoxGetChild(videoStreamDialog, XmDIALOG_APPLY_BUTTON));
        XtUnmanageChild((Widget)XmSelectionBoxGetChild(videoStreamDialog, XmDIALOG_HELP_BUTTON));

	XtAddCallback(videoStreamDialog, XmNnoMatchCallback, VideoStreamSelectionCallback, (XtPointer)0);
	XtAddCallback(videoStreamDialog, XmNokCallback, VideoStreamSelectionCallback, (XtPointer)txtwidget);
	XtAddCallback(videoStreamDialog, XmNcancelCallback, VideoStreamSelectionCallback, (XtPointer)0);
      }

    /* create video selection list */
    numitems = 0;
    for (i=0; i<M; i++)
      {
        if (adi->pstatus[i+N] != CANCEL)
          {
            sprintf(namestr, "%s%d", VIDEO_LIST_STRING, i);
            itemname[numitems] = XmStringCreateSimple(namestr);
            numitems++;
          }
      }
    XtVaSetValues(videoStreamDialog,
                  XmNlistItems,     itemname,
                  XmNlistItemCount, numitems,
                  NULL);

    for (i=0; i<numitems; i++)
        XtFree(itemname[i]);

    XtManageChild(videoStreamDialog);
}

/*************************************************************************
*  Function:      ShowAudioList()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          N - # audio streams
*
*  Effects:       Creates and Manages Audio SelectionDialog to select
*                 current focus stream for audio stream control.
*                 
*  Description:   This callback function is called when the user presses
*                 the Arrow PushButton in the Audio Section of the Audio/
*                 Video Single Stream Control Window.  It creates and
*                 manages a static SelectionDialog that allows the user
*                 to select from "active" audio streams and change the
*                 current focus stream for audio control. Any stream
*                 that has not been CANCELed is considered is valid
*                 (i.e. MUTED streams are ok)
*   
**************************************************************************/
void
ShowAudioList(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
    int i;
    int numitems;
    char namestr[20];
    XmString *itemname;

    Widget *txtwidget = (Widget*)client;

    static Widget audioStreamDialog = NULL;

    Arg         args[512];
    Cardinal    argcnt;
    Boolean     argok;

    argok = FALSE;

    itemname = (XmString*)XtMalloc(N * sizeof(XmString));

    if (audioStreamDialog == NULL)
      {
        argcnt = 0;
        XtSetArg(args[argcnt], XmNtitle, "Active Audio Streams"); argcnt++;
        XtSetArg(args[argcnt], XmNx, 684); argcnt++;
        XtSetArg(args[argcnt], XmNy, 200); argcnt++;
        XtSetArg(args[argcnt], XmNwidth, 422); argcnt++;
        XtSetArg(args[argcnt], XmNheight, 337); argcnt++;
        XtSetArg(args[argcnt], XmNforeground,
             CONVERT(w,"Black", "Pixel", 0, &argok)); if (argok) argcnt++;
        XtSetArg(args[argcnt], XmNlabelFontList,
             CONVERT(w,"-*-helvetica-bold-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12_BOLD","FontList", 0, &argok)); if (argok) argcnt++;
        XtSetArg(args[argcnt], XmNtextFontList,
             CONVERT(w,"-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso8859-1=HELV_12", "FontList", 0, &argok)); if (argok) argcnt++;
        XtSetArg(args[argcnt], XmNlistLabelString,
             CONVERT(w,"Choose Audio Stream:", "XmString", 0, &argok)); if (argok) argcnt++;
        XtSetArg(args[argcnt], XmNlistVisibleItemCount, 8); argcnt++;
        XtSetArg(args[argcnt], XmNmustMatch, True); argcnt++;

        audioStreamDialog = (Widget)XmCreateSelectionDialog(w, "audioStreamDialog", args, argcnt);

	XtUnmanageChild((Widget)XmSelectionBoxGetChild(audioStreamDialog, XmDIALOG_APPLY_BUTTON));
	XtUnmanageChild((Widget)XmSelectionBoxGetChild(audioStreamDialog, XmDIALOG_HELP_BUTTON));

        XtAddCallback(audioStreamDialog, XmNnoMatchCallback, AudioStreamSelectionCallback, (XtPointer)0);
        XtAddCallback(audioStreamDialog, XmNokCallback, AudioStreamSelectionCallback, (XtPointer)txtwidget);
        XtAddCallback(audioStreamDialog, XmNcancelCallback, AudioStreamSelectionCallback, (XtPointer)0);

      }

    /* create audio selection list */
    numitems = 0;
    for (i=0; i<N; i++)
      {
        if (adi->pstatus[i] != CANCEL)
          {
            sprintf(namestr, "%s%d",AUDIO_LIST_STRING,i);
            itemname[numitems] = XmStringCreateSimple(namestr);
            numitems++;
          }
      }
    XtVaSetValues(audioStreamDialog,
                  XmNlistItems,     itemname,
                  XmNlistItemCount, numitems,
                  NULL);        

    for (i=0; i<numitems; i++)
        XtFree(itemname[i]);

    XtManageChild(audioStreamDialog);
}

/*************************************************************************
*  Function:      ChangeVideoStreamCtrl()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          N - # audio streams
*                              (video processes are listed after
*                               all audio processes in adi->pstatus[]
*                          currentVideoStream - currently selected video
*                                               stream to which button
*                                               presses apply
*
*  Effects:       If a valid stream value is entered, changes
*                 current focus stream for video stream control.
*                 
*  Description:   This callback function handles user input for the Text
*                 Widget in the Video Section of the Audio/Video Single
*                 Stream Control Window. It allows the user to change 
*                 the current video stream selected. The entered value
*                 is checked for validity and the current focus stream
*                 for video control is changed.  Any stream
*                 that has not been CANCELed is considered is valid
*                 (i.e. PAUSED streams are ok)
*
**************************************************************************/
void
ChangeVideoStreamCtrl(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
     char *txtvalue;
     int value;

     /* get text value entered */
     txtvalue =(char*) XmTextFieldGetString((Widget)w);
     value = atoi(txtvalue);

     if ((value < 0) || (value > (M-1)))
       {
         PopupErrorDialog(w,"Invalid video stream value entered!",(XtPointer)0);
       }
     else
       {
         if (value == 0)
           {
             if(!strcmp(txtvalue,"0"))
	       {
                 if (adi->pstatus[value+N] == CANCEL)
                   PopupErrorDialog(w,"Selected video stream already CANCELED!",(XtPointer)0);
                 else
                   currentVideoStream = value;
               }
             else
	       {
                 PopupErrorDialog(w,"Invalid video stream value entered!",(XtPointer)0);
               }
           }
         else
           {
             if (adi->pstatus[value+N] == CANCEL)
               PopupErrorDialog(w,"Selected video stream already CANCELED!",(XtPointer)0);
             else
               currentVideoStream = value;
           }          
       }

     XtFree(txtvalue);
}

/*************************************************************************
*  Function:      ChangeAudioStreamCtrl()
*  Creation Date: 08/12/94
*  Author:        Elisa J. Rubin
*                 email:  erubin@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Inputs:        Widget w - calling Widget
*                 XtPointer client - client data (unused)
*                 XtPointer call - callback event data (unused)
*                 Globals: adi->pstatus[] - single stream control "state"
*                          currentAudioStream - currently selected audio
*                                               stream to which button
*                                               presses and audio level
*                                               scale movements apply
*
*  Effects:       If a valid stream value is entered, changes
*                 current focus stream for audio stream control.
*                 
*  Description:   This callback function handles user input for the Text
*                 Widget in the Audio Section of the Audio/Video Single
*                 Stream Control Window. It allows the user to change 
*                 the current audio stream selected. The entered value
*                 is checked for validity and the current focus stream
*                 for audio control is changed.  Any stream
*                 that has not been CANCELed is considered is valid
*                 (i.e. PAUSED streams are ok)
*
**************************************************************************/
void
ChangeAudioStreamCtrl(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
     char *txtvalue;
     int value;

     /* get text value entered */
     txtvalue =(char*) XmTextFieldGetString((Widget)w);
     value = atoi(txtvalue);

     if ((value < 0) || (value > (N-1)))
       {
         PopupErrorDialog(w,"Invalid audio stream value entered!",(XtPointer)0);
       }
     else
       {
         if (value == 0)
           {
             if(!strcmp(txtvalue,"0"))
	       {
                 if (adi->pstatus[value] == CANCEL)
                   PopupErrorDialog(w,"Selected audio stream already CANCELED!",(XtPointer)0);
                 else
                   currentAudioStream = value;
               }
             else
	       {
                 PopupErrorDialog(w,"Invalid audio stream value entered!",(XtPointer)0);
               }
           }
         else
           {
             if (adi->pstatus[value] == CANCEL)
               PopupErrorDialog(w,"Selected audio stream already CANCELED!",(XtPointer)0);
             else
               currentAudioStream = value;
           }          
       }

     XtFree(txtvalue);
}





