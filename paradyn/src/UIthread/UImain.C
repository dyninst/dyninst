/* $Log: UImain.C,v $
/* Revision 1.15  1994/05/26 20:57:16  karavan
/* added tcl variable for location of bitmap files.
/*
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
 * main.c --
 *
 *	This file contains the main program for "wish", a windowing
 *	shell based on Tk and Tcl.  It also provides a template that
 *	can be used as the basis for main programs for other Tk
 *	applications.
 *
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



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/param.h>

extern "C" {
  #include "tcl.h"
  #include "tk.h"
}
#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "UIglobals.h" 

/*
 * Global variables used by tcl/tk UImain program:
 */

static Tk_Window mainWindow;	/* The main window for the application.  If
				 * NULL then the application no longer
				 * exists. */
 Tcl_Interp *interp;	/* Interpreter for this application. */

  /* this is the tcl script which paints the main toolbar on the screen. 
     Toolbar will be painted, then commands accepted interactively.
   */
char *tcl_RcFileName = "../src/UIthread/pdStartup.tcl";

static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */
static char errorExitCmd[] = "exit 1";

resource                  *uim_rootRes;
int                       uim_eid;
List<metricInstance*>     uim_enabled;
performanceStream         *uim_defaultStream;
UIM                       *uim_server;
Tcl_HashTable UIMMsgReplyTbl;
Tcl_HashTable UIMwhereDagTbl;
int UIMMsgTokenID;
int UIMwhereDagID;

/*
 * Command-line options:
 */

static int synchronize = 0;
static char *fileName = NULL;
static char *name = NULL;
static char *display = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-file", TK_ARGV_STRING, (char *) NULL, (char *) &fileName,
	"File from which to read commands"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
	"Initial geometry for window"},
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &display,
	"Display to use"},
    {"-name", TK_ARGV_STRING, (char *) NULL, (char *) &name,
	"Name to use for application"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
	"Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
	(char *) NULL}
};

/*
 * Declarations for various library procedures and variables 
 */

extern "C" {
  void		exit _ANSI_ARGS_((int status));
  char *	strrchr _ANSI_ARGS_((CONST char *string, int c));
  int Tk_DagCmd _ANSI_ARGS_((ClientData clientData,
        Tcl_Interp *interp, int argc, char **argv));
}


extern void initParadynCmd(Tcl_Interp *interp);
extern int UimpdCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[]);

/*
 * Forward declarations for procedures defined later in this file:
 */

void             Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
void             StdinProc _ANSI_ARGS_((ClientData clientData,
                            int mask));
void reaper();
void controlFunc (performanceStream *ps, resource *parent, 
                  resource *newResource, char *name);
void foldFunc (performanceStream *ps, timeStamp width);


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

/* This callback function invoked by dataManager whenever a new 
   resource has been defined.  Maintains where axis display.
*/
void controlFunc (performanceStream *ps , 
                  resource *parent, 
                  resource *newResource, 
                  char *name)
{
  int nodeID;
  char tcommand[200];
  char *parname, *label;
    
  dataMgr->enableResourceCreationNotification(ps, newResource);
  nodeID = (int) Tk_GetUid(name);
  label = strrchr(name, '/'); label++; 
  if (parent == uim_rootRes) {
    sprintf 
      (tcommand, "$WHEREname.d01 addNode %d -root yes -style %d -label \"%s\"",
       nodeID, 1, label);
    if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
      printf ("WHEREaddNodeError: %s\n", interp->result);
    }
  }
  else {
    sprintf (tcommand, "$WHEREname.d01 addNode %d -root no -style 1 -label \"%s\"",
	     nodeID, label);
    if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
      printf ("WHEREaddNodeError: %s\n", interp->result);
    }
    parname = dataMgr->getResourceName(parent);
    sprintf (tcommand, "$WHEREname.d01 addEdge %d %d -style %d",
	     (int) Tk_GetUid(parname), nodeID, 1);
    if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
      printf ("WHEREaddEdgeError: %s\n", interp->result);
    }
  
  }
}

void
applicStateChanged (performanceStream*, appState state) 
{
  char cmd[6];
  sprintf (cmd, "%d", state);
  if (Tcl_VarEval (interp, "changeApplicState ", cmd, 0) == TCL_ERROR) {
    printf ("changeApplicStateERROR: %s\n", interp->result);
  }
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
UImain(CLargStruct *clargs)
{
    char *args;
    char buf[20];
    int code;
    int uiargc;
    char **uiargv;
    int xfd;
    Display *UIMdisplay;
    tag_t mtag;
    int retVal;
    unsigned msgSize;
    char UIMbuff[UIMBUFFSIZE];
    controlCallback controlFuncs;
    dataCallback dataFunc;
    char *temp;
    char *newres;
    printf ("starting mainUI\n");

    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /*
     * Parse command-line arguments.
     */

    uiargc = clargs->clargc;
    uiargv = clargs->clargv;

    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &uiargc, uiargv, 
		     argTable, 0)
	    != TCL_OK) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display != NULL) {
	Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);
    }

    /*
     * Initialize the Tk application.
     */

    mainWindow = Tk_CreateMainWindow(interp, display, "paradyn", "Tk");
    if (mainWindow == NULL) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }
    if (synchronize) {
	XSynchronize(Tk_Display(mainWindow), True);
    }
    Tk_GeometryRequest(mainWindow, 200, 200);

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".  Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    args = Tcl_Merge(uiargc-1, uiargv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);

    sprintf(buf, "%d", uiargc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : uiargv[0],
	    TCL_GLOBAL_ONLY);
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

     // Add the dag command to the tcl interpreter.

    Tcl_CreateCommand(interp, "dag", Tk_DagCmd, (ClientData) mainWindow,
	    (Tcl_CmdDeleteProc *) NULL);

     // Add internal UIM command to the tcl interpreter.

    Tcl_CreateCommand(interp, "uimpd", 
		      UimpdCmd, (ClientData) mainWindow,
		      (Tcl_CmdDeleteProc *) NULL);

    /*
     * Set the geometry of the main window, if requested.
     */

    if (geometry != NULL) {
	code = Tcl_VarEval(interp, "wm geometry . ", geometry, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	}
      }

    /* tell interpreter where the tcl files are */
    if (!(temp = (char *) getenv("PARADYNTCL"))) {
      temp = new char[80];
      strcpy (temp, "/usr/home/paradyn/core/paradyn/tcl");
    }

    if (Tcl_VarEval (interp, "set auto_path [linsert $auto_path 0 ",
		 temp, "]", 0) == TCL_ERROR)
      printf ("can't set auto_path: %s\n", interp->result);
    Tcl_SetVar (interp, "PdBitmapDir", temp, 0);

   /* display the paradyn main menu tool bar */
    if (Tcl_VarEval (interp, "drawToolBar", 0) == TCL_ERROR)
      printf ("NOTOOLBAR:: %s\n", interp->result);

   // first take care of any events caused by initialization.

    while (Tk_DoOneEvent (TK_DONT_WAIT) > 0)
      ;

  // add Paradyn tcl command to active interpreter

    initParadynCmd (interp);

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

    fprintf (stderr, "UIM thread past barrier\n");

   // subscribe to DM new resource notification service

    uim_rootRes = dataMgr->getRootResource();
    newres = dataMgr->getResourceName (uim_rootRes);
    printf ("root resource: %s\n", newres);
    controlFuncs.rFunc = controlFunc;
    controlFuncs.mFunc = NULL;
    controlFuncs.fFunc = NULL;
    controlFuncs.sFunc = applicStateChanged;
    dataFunc.sample = NULL;
    uim_defaultStream = dataMgr->createPerformanceStream(context, Sample, 
        dataFunc, controlFuncs);
    dataMgr->enableResourceCreationNotification(uim_defaultStream, uim_rootRes);
   /* display the where axis */
    {
      if (Tcl_VarEval (interp, "initWHERE .where", 0) == TCL_ERROR)
	printf ("NOWHERE:: %s\n", interp->result);
    }
    
/********************************
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
	  while (Tk_DoOneEvent (TK_DONT_WAIT) > 0)
	    ;
	} else 
	  StdinProc((ClientData) NULL, 0);
      } else 

// check for upcalls

	if (dataMgr->isValidUpCall(mtag)) {
	  dataMgr->awaitResponce(-1);
	  Tcl_VarEval (interp, "update", 0);
	} 
	else { 
         
// check for incoming client requests
	  uim_server->mainLoop();
	  Tcl_VarEval (interp, "update", 0);
	}
    }

    /*
     * Exiting this thread will signal the main/parent to exit.  No other
     * notification is needed.  This call will be reached when there are 
     * no windows remaining for the application -- either grievous error 
     * or user has selected "EXIT".
     */
    thr_exit(0);
  }
    


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

