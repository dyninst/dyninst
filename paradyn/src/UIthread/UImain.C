/* $Log: UImain.C,v $
/* Revision 1.40  1994/11/08 07:50:43  karavan
/* Purified code; narrowed side margins for dag nodes.
/*
 * Revision 1.39  1994/11/07  07:26:58  karavan
 * changed requested main window size.
 *
 * Revision 1.38  1994/11/03  22:18:42  karavan
 * eliminated redundancy in status display
 *
 * Revision 1.37  1994/11/03  20:25:05  krisna
 * added status_lines for application name and application status
 *
 * Revision 1.36  1994/11/03  06:16:14  karavan
 * status display and where axis added to main window and the look cleaned
 * up a little bit.  Added option to ResourceDisplayObj class to specify
 * a parent window for an RDO with the constructor.
 *
 * Revision 1.35  1994/11/03  02:44:58  krisna
 * status lines are now added into paradyn.
 *
 * Revision 1.34  1994/11/02  04:42:55  karavan
 * cleanup for new handling of commandline arguments
 *
 * Revision 1.33  1994/11/01  22:39:27  karavan
 * changed debugging printf to call to PARADYN_DEBUG
 *
 * Revision 1.32  1994/11/01  05:42:32  karavan
 * some minor performance and warning fixes
 *
 * Revision 1.31  1994/10/25  17:57:32  karavan
 * added Resource Display Objects, which support display of multiple resource
 * abstractions.
 *
 * Revision 1.30  1994/10/09  01:24:47  karavan
 * A large number of changes related to the new UIM/visiThread metric&resource
 * selection interface and also to direct selection of resources on the
 * Where axis.
 *
 * Revision 1.29  1994/09/30  19:18:28  rbi
 * Abstraction interface change.
 *
 * Revision 1.28  1994/09/05  20:04:49  jcargill
 * Fixed read-before-write of thread stack data (spotted by purify)
 *
 * Revision 1.27  1994/08/30  16:23:17  karavan
 * added "silent" node trimming to the base where axis.
 *
 * Revision 1.26  1994/08/05  16:04:25  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.25  1994/08/01  20:24:39  karavan
 * new version of dag; new dag support commands
 *
 * Revision 1.24  1994/07/28  22:32:16  krisna
 * proper starting sequence for UImain thread
 *
 * Revision 1.23  1994/07/25  14:58:14  hollings
 * added suppress resource option.
 *
 * Revision 1.22  1994/07/07  17:40:33  karavan
 * added error and batch mode features.
 *
 * Revision 1.21  1994/06/29  21:46:25  hollings
 * Removed dead variable.
 *
 * Revision 1.20  1994/06/29  02:56:42  hollings
 * AFS path changes
 *
 * Revision 1.19  1994/06/27  21:25:17  rbi
 * New abstraction parameter for performance streams
 *
 * Revision 1.18  1994/06/17  22:08:07  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.17  1994/06/12  22:37:11  karavan
 * implemented status change for run/pause buttons.
 * bug fix:  node labels may now contain tcl special characters, eg [].
 *
 * Revision 1.16  1994/05/31  19:11:47  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.15  1994/05/26  20:57:16  karavan
 * added tcl variable for location of bitmap files.
 *
 * Revision 1.14  1994/05/23  01:59:31  karavan
 * added callbacks for resource notification and state change notification.
 *
 * Revision 1.13  1994/05/12  23:34:13  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.12  1994/05/10  03:57:49  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.11  1994/05/07  23:27:21  karavan
 * eliminated [location-dependent] tcl init file.
 *
 * Revision 1.10  1994/05/05  23:35:00  karavan
 * changed tcl calls to procedures.  removed conflicting def'n of read.
 * changed name of tcl initialization script.
 *
 * Revision 1.9  1994/05/02  20:38:30  hollings
 * added search pause command and shg commands.
 *
 * Revision 1.8  1994/04/21  23:24:50  hollings
 * added process command.
 *
 * Revision 1.7  1994/04/21  19:42:51  karavan
 * the *working* version, this time!
 *
 * Revision 1.6  1994/04/21  19:17:59  karavan
 * Added initialization of tcl dag command.
 *
 * Revision 1.5  1994/04/13  01:33:04  markc
 * Changed pointer to .tcl file.
 *
 * Revision 1.4  1994/04/06  22:39:57  markc
 * Added code to provide local paradynd with a machine name via
 * addExecutable.
 *
 * Revision 1.3  1994/04/06  17:39:56  karavan
 * added call to tcl initialization script
 *
 * Revision 1.2  1994/04/05  23:49:21  rbi
 * Fixed a bunch of tcl related stuff.
 *
 * Revision 1.1  1994/04/05  04:42:35  karavan
 * initial version of UI thread code and tcl paradyn command
 * */

/* UImain.C
 *    This is the main routine for the User Interface Manager thread, 
 *    called at thread creation.
 */

/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/param.h>

#include "UIglobals.h" 
#include "../DMthread/DMresource.h"
#include "../DMthread/DMabstractions.h"
#include "dataManager.h"
#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "dag.h"

#include "Status.h"

/*
 * Global variables used by tcl/tk UImain program:
 */

static Tk_Window mainWindow;	/* The main window for the application.  If
				 * NULL then the application no longer
				 * exists. */
 Tcl_Interp *interp;	/* Interpreter for this application. */

static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */

resource                  *uim_rootRes;
int                       uim_eid;
List<metricInstance*>     uim_enabled;
performanceStream         *uim_defaultStream;
UIM                       *uim_server;
int uim_maxError;
int uim_ResourceSelectionStatus;
List<resourceList *> uim_CurrentResourceSelections;
int UIM_BatchMode = 0;
Tcl_HashTable UIMMsgReplyTbl;
Tcl_HashTable UIMwhereDagTbl;
int UIMMsgTokenID;
appState PDapplicState = appPaused;     // used to update run/pause buttons  


/*
 * Command-line options:
 */

static int synchronize = 0;
static char *fileName = NULL;
static char *name = NULL;
static char *display = NULL;
static char *geometry = NULL;

/*
 * Declarations for various library procedures and variables 
 */

extern "C" {
  void		exit _ANSI_ARGS_((int status));
  char *	strrchr _ANSI_ARGS_((CONST char *string, int c));
  int Tk_DagCmd _ANSI_ARGS_((ClientData clientData,
        Tcl_Interp *interp, int argc, char **argv));
}

extern int UimpdCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[]);
extern int ParadynCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[]);
extern int getDagToken ();
extern void resourceAddedCB (performanceStream *ps , 
		      resource *parent, 
		      resource *newResource, 
		      char *name);
extern int initMainWhereDisplay ();

/*
 * Forward declarations for procedures defined later in this file:
 */

void             Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
void             StdinProc _ANSI_ARGS_((ClientData clientData,
                            int mask));

/*** I don't think we need this anymore -klk
void reaper();

void reaper()
{
    int ret;
    int status;

    printf("**** In reaper\n");
    ret = wait(&status);
    if (WIFSTOPPED(status)) {
        printf("child stopped\n");
    } else if (WIFEXITED(status)) {
        printf("child gone\n");
    } else {
        printf("%x\n", status);
    }
}
*/

// This callback invoked by dataManager before and after a large 
// batch of draw requests.  If UIM_BatchMode is set, the UI thread 
// will not examine idle event queue.
 
void resourceBatchChanged(performanceStream *ps, batchMode mode)
{
    if (mode == batchStart) {
      UIM_BatchMode++;
    } else {
      UIM_BatchMode--;
    }
}

void
applicStateChanged (performanceStream*, appState state) 
{
	static status_line app_status("Application status");

  if ((state == appRunning) && (PDapplicState == appPaused)) { 
    if (Tcl_VarEval (interp, "changeApplicState 1", 0) == TCL_ERROR) {
      printf ("changeApplicStateERROR: %s\n", interp->result);
    }
	app_status.state(status_line::NORMAL);
	app_status.message("RUNNING");
  } else if ((state == appPaused) && (PDapplicState == appRunning)) {
    if (Tcl_VarEval (interp, "changeApplicState 0", 0) == TCL_ERROR) {
      printf ("changeApplicStateERROR: %s\n", interp->result);
    }
	app_status.state(status_line::URGENT);
	app_status.message("PAUSED");
  }
    PDapplicState = state;
}


/*
 *----------------------------------------------------------------------
 *
 * UImain --
 *
 *	Main program for UI thread
 *
 * Side effects:
 *	This procedure initializes the wish world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

void *
UImain(void* vargs)
{
    CLargStruct* clargs = (CLargStruct *) vargs;

    char *args;
    char buf[20];
    int code;
    int uiargc;
    char **uiargv;
    int xfd;
    Display *UIMdisplay;
    tag_t mtag;
    int retVal;
    unsigned msgSize = 0;
    char UIMbuff[UIMBUFFSIZE];
    char *temp;
    controlCallback controlFuncs;
    dataCallback dataFunc;
    char *tclscript = NULL;

    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    // Parse commandline arguments 
    uiargc = clargs->clargc;
    uiargv = clargs->clargv;

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display != NULL) {
	Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);
    }

    // Tk main window initialization
    char *name, *cls;
    name = new char[8];
    cls = new char[3];
    strcpy (name, "paradyn");
    strcpy (cls, "Tk");
    mainWindow = Tk_CreateMainWindow(interp, display, name, cls);
    if (mainWindow == NULL) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }
    if (synchronize) {
	XSynchronize(Tk_Display(mainWindow), True);
    }
    Tk_GeometryRequest(mainWindow, 725, 475);

    Tk_SetClass(mainWindow, "Paradyn");
    
    // Copy commandline arguments into the Tcl variables "argc" and "argv"  
    args = Tcl_Merge(uiargc-1, uiargv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    
    sprintf(buf, "%d", uiargc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : uiargv[0],
	    TCL_GLOBAL_ONLY);
    // set tcl geometry variable
    if (geometry != NULL) {
	Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

     // initialize tcl and tk
    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);
    if (Tcl_Init(interp) == TCL_ERROR) {
      fprintf(stderr, "%s\n", interp->result);
    }
    if (Tk_Init(interp) == TCL_ERROR) {
      fprintf (stderr, "%s\n", interp->result);
    }

     // Add internal UIM command to the tcl interpreter.
    Tcl_CreateCommand(interp, "uimpd", 
		      UimpdCmd, (ClientData) mainWindow,
		      (Tcl_CmdDeleteProc *) NULL);

    // add Paradyn tcl command to active interpreter
    Tcl_CreateCommand(interp, "paradyn", ParadynCmd, (ClientData) NULL,
		      (Tcl_CmdDeleteProc *) NULL);

    // Set the geometry of the main window, if requested.
    if (geometry != NULL) {
	code = Tcl_VarEval(interp, "wm geometry . ", geometry, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	}
      }

    /* tell interpreter where the tcl files are */
    if (!(temp = (char *) getenv("PARADYNTCL"))) {
      temp = new char[80];
      strcpy (temp, "/p/paradyn/core/paradyn/tcl");
    }

    if (Tcl_VarEval (interp, "set auto_path [linsert $auto_path 0 ",
		 temp, "]", 0) == TCL_ERROR)
      printf ("can't set auto_path: %s\n", interp->result);
    Tcl_SetVar (interp, "PdBitmapDir", temp, 0);

   /* display the paradyn main menu tool bar */
    if (Tcl_VarEval (interp, "drawToolBar", 0) == TCL_ERROR)
      printf ("NOTOOLBAR:: %s\n", interp->result);
     // initialize number of errors read in from error database 
    uim_maxError = atoi(Tcl_GetVar (interp, "numPdErrors", 0));

   // first take care of any events caused by initialization.

    while (Tk_DoOneEvent (TK_DONT_WAIT) > 0)
      ;

   // bind stdin to this thread & setup command-line input w/prompt

    retVal = msg_bind (0, 1);

    if (tty) {
      Prompt(interp, 0);
    }
    fflush(stdout);
    Tcl_DStringInit(&command);

   // Initialize UIM thread as UIM server 
    thr_name ("UIM");
    uim_server = new UIM(MAINtid);

   // register fd for X events with threadlib as special */

    UIMdisplay = Tk_Display (mainWindow);
    xfd = XConnectionNumber (UIMdisplay);
    retVal = msg_bind (xfd, 1);

    // initialize hash table for async call replies
    Tcl_InitHashTable (&UIMMsgReplyTbl, TCL_ONE_WORD_KEYS);
    UIMMsgTokenID = 0;

   // wait for all other main module threads to complete initialization
   //  before continuing.

    retVal = msg_send (MAINtid, MSG_TAG_UIM_READY, (char *) NULL, 0);
    mtag = MSG_TAG_ALL_CHILDREN_READY;
    retVal = msg_recv (&mtag, UIMbuff, &msgSize);

    PARADYN_DEBUG(("UIM thread past barrier\n"));

   // subscribe to DM new resource notification service

    uim_rootRes = dataMgr->getRootResource();
    if (!uim_rootRes) abort();
    controlFuncs.rFunc = resourceAddedCB;
    controlFuncs.mFunc = NULL;
    controlFuncs.fFunc = NULL;
    controlFuncs.sFunc = applicStateChanged;
    controlFuncs.bFunc = resourceBatchChanged;
    dataFunc.sample = NULL;
    uim_defaultStream = dataMgr->createPerformanceStream
      (context, Sample, dataFunc, controlFuncs);
    dataMgr->enableResourceCreationNotification(uim_defaultStream, 
						uim_rootRes);

    uim_ResourceSelectionStatus = 0;    // no selection in progress
    Tcl_LinkVar (interp, "resourceSelectionStatus", 
		 (char *) &uim_ResourceSelectionStatus, TCL_LINK_INT);    
    retVal = 0;
    initMainWhereDisplay();

    //
    // initialize status lines library
    // it is assumed that by this point, all the appropriate
    // containing frames have been created and packed into place
    //
    // after this point onwards, any thread may make use of
    // status lines.
    // if any thread happens to create status_lines before here,
    // it should be prepared for a core dump.
    //
    // --krishna
    //
    if (status_line::status_init(interp) != TCL_OK) {
        fprintf(stderr, "status_line::status_init -> `%s'\n",
            interp->result);
    }

    status_line ui_status("UIM status");
    ui_status.message("WELCOME to Paradyn.  Interfaces ready");

/*******************************
 *    Main Loop for UIM thread.  
 ********************************/

    while (tk_NumMainWindows > 0) {

      msgSize = UIMBUFFSIZE;
      mtag = MSG_TAG_ANY;

      retVal = msg_poll (&mtag, 1);

// check for X events or commands on stdin
//   These generate a notification message only which can be ignored.

      if (mtag == MSG_TAG_FILE) {
	retVal = msg_recv (&mtag, UIMbuff, &msgSize);
	if (retVal == xfd) {
	  if (UIM_BatchMode) {
	    while (Tk_DoOneEvent (TK_X_EVENTS | TK_FILE_EVENTS 
				  | TK_TIMER_EVENTS | TK_DONT_WAIT) > 0)
	      ;
	  }
	  else {
	    while (Tk_DoOneEvent (TK_DONT_WAIT) > 0)
	      ;
	  }
	} else 
	  StdinProc((ClientData) NULL, 0);
      } else  {

// check for upcalls

	if (dataMgr->isValidUpCall(mtag)) {
	  dataMgr->awaitResponce(-1);
	  if (!UIM_BatchMode) {
	    Tcl_VarEval (interp, "update", 0);
	    while (Tk_DoOneEvent (TK_DONT_WAIT) > 0)
	      ;
	  }
	} else {

// check for incoming client requests
	  uim_server->mainLoop();
	  if (!UIM_BatchMode) {
	    Tcl_VarEval (interp, "update", 0);
	  }
	}
      }
    } 

    /*
     * Exiting this thread will signal the main/parent to exit.  No other
     * notification is needed.  This call will be reached when there are 
     * no windows remaining for the application -- either grievous error 
     * or user has selected "EXIT".
     */
    delete name;
    delete cls;
    thr_exit(0);
  }
    


/* The two procedures below are taken from the tcl/tk distribution and
 * the following copyright notice applies.
 */
/* 
 * Copyright (c) 1990-1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*
 *----------------------------------------------------------------------
 *
 * StdinProc --
 *
 *      This procedure is invoked by the event dispatcher whenever
 *      standard input becomes readable.  It grabs the next line of
 *      input characters, adds them to a command being assembled, and
 *      executes the command if it's complete.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Could be almost arbitrary, depending on the command that's
 *      typed.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
void
StdinProc(ClientData clientData, int mask)
{
#define BUFFER_SIZE 4000
    char input[BUFFER_SIZE+1];
    static int gotPartial = 0;
    char *cmd;
    int code, count;

    count = read(fileno(stdin), input, BUFFER_SIZE);
    if (count <= 0) {
        if (!gotPartial) {
            if (tty) {
                Tcl_Eval(interp, "exit");
                exit(1);
            } else {
                Tk_DeleteFileHandler(0);
            }
            return;
        } else {
            count = 0;
        }
    }
    cmd = Tcl_DStringAppend(&command, input, count);
    if (count != 0) {
        if ((input[count-1] != '\n') && (input[count-1] != ';')) {
            gotPartial = 1;
            goto prompt;
        }
        if (!Tcl_CommandComplete(cmd)) {
            gotPartial = 1;
            goto prompt;
        }
    }
    gotPartial = 0;

    /*
     * Disable the stdin file handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tk_CreateFileHandler(0, 0, StdinProc, (ClientData) 0);
    code = Tcl_RecordAndEval(interp, cmd, 0);
    Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
    Tcl_DStringFree(&command);
    if (*interp->result != 0) {
        if ((code != TCL_OK) || (tty)) {
            printf("%s\n", interp->result);
        }
    }

    /*
     * Output a prompt.
     */

    prompt:
    if (tty) {
        Prompt(interp, gotPartial);
    }
}
/*
 *----------------------------------------------------------------------
 *
 * Prompt --
 *
 *      Issue a prompt on standard output, or invoke a script
 *      to issue the prompt.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      A prompt gets output, and a Tcl script may be evaluated
 *      in interp.
 *
 *----------------------------------------------------------------------
 */

void
Prompt(Tcl_Interp *interp,   /* Interpreter to use for prompting. */
       int partial)          /* Non-zero means there already
                              * exists a partial command, so use
                              * the secondary prompt. */
{
    char *promptCmd;
    int code;

    promptCmd = Tcl_GetVar(interp,
        partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    if (promptCmd == NULL) {
        defaultPrompt:
        if (!partial) {
            fputs("pd> ", stdout);
        }
    } else {
        code = Tcl_Eval(interp, promptCmd);
        if (code != TCL_OK) {
            Tcl_AddErrorInfo(interp,
                    "\n    (script that generates prompt)");
            fprintf(stderr, "%s\n", interp->result);
            goto defaultPrompt;
        }
    }
    fflush(stdout);
  }

