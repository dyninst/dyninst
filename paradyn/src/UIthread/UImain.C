/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* $Log: UImain.C,v $
/* Revision 1.86  1997/05/02 04:43:47  karavan
/* added new functionality to support "SAVE" feature.
/*
/* added support to use standard tcl autoload feature for development use.
/*
 * Revision 1.85  1997/04/14 19:59:51  zhichen
 * added memoryAddedCB.
 *
 * Revision 1.84  1997/01/15 00:11:43  tamches
 * added calls to abstraction::start and end batch mode
 *
 * Revision 1.83  1996/10/16 16:12:32  tamches
 * changes to accomodate new abstractions::resizeEverything fixes a
 * sorting bug
 *
 * Revision 1.82  1996/08/16 21:06:46  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.81  1996/08/05 07:30:10  tamches
 * update for tcl 7.5
 *
 * Revision 1.80  1996/04/30 18:56:29  newhall
 * changes to support the asynchrounous enable data calls to the DM
 * this code contains a kludge to make the UI wait for the DM's async response
 *
 * Revision 1.79  1996/04/07  21:17:07  karavan
 * changed new phase notification handling; instead of being notified by the
 * data manager, the UI is notified by the performance consultant.  This prevents
 * a race condition.
 *
 * Revision 1.78  1996/03/08 03:00:34  tamches
 * fixed hide-node bug whereby a tc change before PC window was open would
 * give an assertion failure
 *
 * Revision 1.77  1996/03/08 00:20:40  tamches
 * added 7 tunable constants for hiding desired shg nodes
 *
 * Revision 1.76  1996/02/21 18:15:50  tamches
 * cleanup of applicStateChanged related to commit of paradyn.tcl.C
 *
 * Revision 1.75  1996/02/15 23:06:27  tamches
 * added support for phase 0, the initial current phase
 *
 */

/* UImain.C
 *    This is the main routine for the User Interface Manager thread, 
 *    called at thread creation.
 */

#include "tcl.h"
#include "tk.h"

#include "util/h/headers.h"
#include <sys/param.h>

#include "UIglobals.h" 
#include "paradyn/src/DMthread/DMinclude.h"
#include "dataManager.thread.h"
#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"

#include "paradyn/src/TCthread/tunableConst.h"

#include "abstractions.h"
#include "whereAxisTcl.h"
#include "shgPhases.h"
#include "shgTcl.h"
#include "tkTools.h"

// Paradyn logo:
#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm" // defines logo_bits[], logo_width, logo_height
#include "paradyn/xbm/dont.xbm" // defines error_bits[], error_width, error_height

bool haveSeenFirstGoodWhereAxisWid = false;
bool tryFirstGoodWhereAxisWid(Tcl_Interp *interp, Tk_Window topLevelTkWindow) {
   if (haveSeenFirstGoodWhereAxisWid)
      return true;

   Tk_Window theTkWindow = Tk_NameToWindow(interp, ".whereAxis.nontop.main.all",
                                           topLevelTkWindow);
   assert(theTkWindow);

   if (Tk_WindowId(theTkWindow) == 0)
      return false; // sigh...still invalid (that's why this routine is needed)

   haveSeenFirstGoodWhereAxisWid = true;

   extern abstractions *theAbstractions; // whereAxisTcl.C
   theAbstractions = new abstractions(".whereAxis.top.mbar.abs.m",
				      ".whereAxis.top.mbar.nav.m",
				      ".whereAxis.nontop.main.bottsb",
				      ".whereAxis.nontop.main.leftsb",
				      ".whereAxis.nontop.find.entry",
				      interp, theTkWindow);
   assert(theAbstractions);

   return true;   
}


/*
 * Global variables used by tcl/tk UImain program:
 */

extern Tcl_Interp *interp;	/* Interpreter for this application. */

static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
extern int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */

List<metricInstInfo *> uim_enabled;
perfStreamHandle          uim_ps_handle;
UIM                       *uim_server;
int uim_maxError;
int uim_ResourceSelectionStatus;
int UIM_BatchMode = 0;
Tcl_HashTable UIMMsgReplyTbl;
Tcl_HashTable UIMwhereDagTbl;
int UIMMsgTokenID;
appState PDapplicState = appPaused;     // used to update run/pause buttons  

status_line *ui_status=NULL;
status_line *app_status=NULL;

/*
 * Declarations for various library procedures and variables 
 */

extern int UimpdCmd(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int argc, 
		    char *argv[]);
extern int ParadynCmd(ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[]);
extern void resourceAddedCB (perfStreamHandle handle, 
		      resourceHandle parent, 
		      resourceHandle newResource, 
		      const char *name,
		      const char *abstraction);
extern void memoryAddedCB (perfStreamHandle handle,
                      const char *vname,
                      int start,
                      unsigned mem_size,
                      unsigned blk_size,
                      resourceHandle parent,
                      vector<resourceHandle> *newResources ) ;

/*
 * Forward declarations for procedures defined later in this file:
 */

// This callback invoked by dataManager before and after a large 
// batch of draw requests.
// I'm not sure what the perfStreamHandle argument is for --ari
 
void resourceBatchChanged(perfStreamHandle, batchMode mode)
{
    if (mode == batchStart) {
      ui_status->message("receiving where axis items [batch mode]");

      UIM_BatchMode++;

      extern abstractions *theAbstractions;
      assert(theAbstractions);

      theAbstractions->startBatchMode();
    } else {
      UIM_BatchMode--;

      if (UIM_BatchMode == 0) {
         // Batch mode is done with.  We need to update the where axis'
         // spatial graphications...
         ui_status->message("Rethinking after batch mode");

         extern abstractions *theAbstractions;
         assert(theAbstractions);

	 theAbstractions->endBatchMode();
	    // does: resizeEverything(true); (resorts, rethinks layout.  expensive)

         initiateWhereAxisRedraw(interp, true); // true--> double buffer

         ui_status->message("ready");

         // Shouldn't we also Tcl_Eval(interp, "update"), to process any
         // pending idle events?
      }
    }
    assert(UIM_BatchMode >= 0);
}

void applicStateChanged (perfStreamHandle, appState newstate) {
  if (! app_status) {
    app_status = new status_line("Application status");
  }
 
  if (newstate == appRunning && PDapplicState == appPaused) {
    myTclEval(interp, "changeApplicState 1");
    app_status->state(status_line::NORMAL);
    app_status->message("RUNNING");
  } else if (newstate == appPaused && PDapplicState == appRunning) {
    myTclEval(interp, "changeApplicState 0");
    app_status->state(status_line::URGENT);
    app_status->message("PAUSED");
  } else if (newstate == appExited) {
    app_status->state(status_line::URGENT);
    app_status->message("EXITED");
  }
  else if (newstate == PDapplicState) {
    // no state change
  }
  else {
    // state changed, but UI buttons didn't need updating?
  }

  PDapplicState = newstate;
}

// Currently this routine never executes because of a kludge
// that receives the response message from the DM before a call to this
// routine is made.  This routine must still be registered with the DM on
// createPerformanceStream, otherwise the DM will not send the response
// message.  If the UI is changed so that the call to getPredictedDataCost
// is handled in a truely asynchronous manner, then this routine should
// contain the code to handle the upcall from the DM
void UIenableDataResponse(vector<metricInstInfo> *,  u_int){
    cout << "UIenableDataResponse: THIS SHOULD NEVER EXECUTE" << endl;
}

void panic(const char *msg) {
   cerr << msg << endl;
   exit(5);
}

void processPendingTkEventsNoBlock() {
   // We use Tk_DoOneEvent (w/o blocking) to soak up and process
   // pending tk events, if any.  Returns as soon as there are no
   // tk events to process.
   // NOTE: This includes (as it should) tk idle events.
   // NOTE: This is basically the same as Tcl_Eval(interp, "update"), but
   //       who wants to incur the expense of tcl parsing?

   while (Tk_DoOneEvent(TK_DONT_WAIT) > 0)
      ;
}

#define UIMBUFFSIZE 256

void Prompt(Tcl_Interp *, int partial);
void StdinProc(ClientData, int mask);

void tclPromptCallback(bool newValue) {
   // Such a change affects whether or not a prompt is displayed; and,
   // more importantly, whether or not we should bind to stdin.
   if (newValue) {
      //cout << "binding to stdin:" << endl;
      msg_bind(fileno(stdin),
	       1 // "special" flag --> libthread leaves it to us to manually
	         // dequeue messages
	       );
   }
   else {
      //cout << "unbinding from stdin:" << endl;
      msg_unbind(fileno(stdin));

      //cout << "deleting file handler:" << endl;
      Tk_DeleteFileHandler(fileno(stdin));
   }
}

extern shgPhases *theShgPhases;
void tcShgHideGeneric(shg::changeType ct, bool hide) {
   if (theShgPhases == NULL)
      // the shg window hasn't been opened yet...
      // do nothing.  When "theShgPhases" is first created, it'll
      // grab fresh values from the tunables
      return;

   assert(theShgPhases);
   bool anyChanges = theShgPhases->changeHiddenNodes(ct, hide);
   if (anyChanges)
      initiateShgRedraw(interp, true); // true --> double buffer

   // ...and here is where we update the tk labels; hidden
   // node types are now drawn with a shaded background color.
}

void tcShgHideTrueCallback(bool hide) {
   tcShgHideGeneric(shg::ct_true, hide);
}
void tcShgHideFalseCallback(bool hide) {
   tcShgHideGeneric(shg::ct_false, hide);
}
void tcShgHideUnknownCallback(bool hide) {
   tcShgHideGeneric(shg::ct_unknown, hide);
}
void tcShgHideNeverCallback(bool hide) {
   tcShgHideGeneric(shg::ct_never, hide);
}
void tcShgHideActiveCallback(bool hide) {
   tcShgHideGeneric(shg::ct_active, hide);
}
void tcShgHideInactiveCallback(bool hide) {
   tcShgHideGeneric(shg::ct_inactive, hide);
}
void tcShgHideShadowCallback(bool hide) {
   tcShgHideGeneric(shg::ct_shadow, hide);
}

void *UImain(void*) {
    tag_t mtag;
    int retVal;
    unsigned msgSize = 0;

    char UIMbuff[UIMBUFFSIZE];
    controlCallback controlFuncs;
    dataCallback dataFunc;

    tunableBooleanConstantDeclarator tcWaShowTips("showWhereAxisTips",
						  "If true, the where axis window will be drawn with helpful reminders on shortcuts for expanding, unexpanding, selecting, and scrolling.  A setting of false saves screen real estate.",
						  true, // initial value
						  whereAxisDrawTipsCallback,
						  userConstant);
						  
    tunableBooleanConstantDeclarator tcShgShowKey("showShgKey",
						  "If true, the search history graph will be drawn with a key for decoding the meaning of the several background colors, text colors, italics, etc.  A setting of false saves screen real estate.",
						  true, // initial value
						  shgDrawKeyCallback,
						  userConstant);
						  

    tunableBooleanConstantDeclarator tcShgShowTips("showShgTips",
						  "If true, the search history graph will be drawn with reminders on shortcuts for expanding, unexpanding, selecting, and scrolling.  A setting of false saves screen real estate.",
						  true, // initial value
						  shgDrawTipsCallback,
						  userConstant);
						  

    tunableBooleanConstantDeclarator tcHideTcl("tclPrompt",
					       "Allow access to a command-line prompt accepting arbitrary tcl commands in the Paradyn process",
					       false, // initial value
					       tclPromptCallback,
					       developerConstant);

    tunableBooleanConstantDeclarator tcHideTrue("hideShgTrueNodes",
						"To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all true nodes (background colored blue)",
						false, // initial value
						tcShgHideTrueCallback,
						userConstant);

    tunableBooleanConstantDeclarator tcHideFalse("hideShgFalseNodes",
						 "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all false nodes (background colored pink)",
						 false, // initial value
						 tcShgHideFalseCallback,
						 userConstant);

    tunableBooleanConstantDeclarator tcHideUnknown("hideShgUnknownNodes",
						   "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all nodes with an unknown value (background colored green)",
						   false, // initial value
						   tcShgHideUnknownCallback,
						   userConstant);

    tunableBooleanConstantDeclarator tcHideNever("hideShgNeverSeenNodes",
						 "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all never-before-seen nodes (background colored gray)",
						 false, // initial value
						 tcShgHideNeverCallback,
						 userConstant);

    tunableBooleanConstantDeclarator tcHideActive("hideShgActiveNodes",
						  "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all active nodes (foreground text white)",
						  false, // initial value
						  tcShgHideActiveCallback,
						  userConstant);

    tunableBooleanConstantDeclarator tcHideInactive("hideShgInactiveNodes",
						    "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all inactive nodes (foreground text black)",
						    false, // initial value
						    tcShgHideInactiveCallback,
						    userConstant);

    tunableBooleanConstantDeclarator tcHideShadow("hideShgShadowNodes",
						  "To save space in the Performance Consultant Search History Graph, a true setting of this tunable constant will hide all true nodes",
						  false, // initial value
						  tcShgHideShadowCallback,
						  userConstant);

    // Add internal UIM command to the tcl interpreter.
    Tcl_CreateCommand(interp, "uimpd", 
		      UimpdCmd, (ClientData) Tk_MainWindow(interp),
		      (Tcl_CmdDeleteProc *) NULL);

    // add Paradyn tcl command to active interpreter
    Tcl_CreateCommand(interp, "paradyn", ParadynCmd, (ClientData) NULL,
		      (Tcl_CmdDeleteProc *) NULL);

/*
 * load all converted Tcl sources into the interpreter.
 * the function `initialize_tcl_sources(Tcl_Interp *)' is automatically
 * generated by `tcl2c'.
 *
 */
    extern int initialize_tcl_sources(Tcl_Interp *);
    if (initialize_tcl_sources(interp) != TCL_OK) {
        fprintf(stderr, "initialize_tcl_sources: ERROR in Tcl sources, exiting\n");
        exit(-1);
    }

    // Initialize the 2 bitmaps we use
    pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width, logo_height);
    pdLogo::install_fixed_logo("dont", error_bits, error_width, error_height);
    
    // now install the tcl cmd "createPdLogo" (must be done before anyone
    // tries to create a logo)
    tcl_cmd_installer createPdLogo(interp, "makeLogo", pdLogo::makeLogoCommand,
				   (ClientData)Tk_MainWindow(interp));

    /* display the paradyn main menu tool bar */
    myTclEval(interp, "drawToolBar");
    
    // initialize number of errors read in from error database 
    Tcl_VarEval (interp, "getNumPdErrors", (char *)NULL);
    uim_maxError = atoi(interp->result);

    // Initialize "command", an important global variable that accumulates
    // a typed-in line of data.  It gets updated in StdinProc(), below.
    Tcl_DStringInit(&command);
    if (tty)
      Prompt(interp, 0);

    // Initialize UIM thread as UIM server 
    thr_name ("UIM");
    uim_server = new UIM(MAINtid);

    // register fd for X events with threadlib as special
    Display *UIMdisplay = Tk_Display (Tk_MainWindow(interp));
    int xfd = XConnectionNumber (UIMdisplay);
    retVal = msg_bind (xfd,
		       1 // "special" flag --> libthread leaves it to us to manually
                         // dequeue these messages
		       );

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
    controlFuncs.rFunc = resourceAddedCB;
    controlFuncs.mFunc = NULL;
    controlFuncs.fFunc = NULL;
    controlFuncs.sFunc = applicStateChanged;
    controlFuncs.bFunc = resourceBatchChanged;
    controlFuncs.pFunc = NULL;
    controlFuncs.eFunc = UIenableDataResponse;
    controlFuncs.xFunc = memoryAddedCB;
    dataFunc.sample = NULL;

    uim_ps_handle = dataMgr->createPerformanceStream
      (Sample, dataFunc, controlFuncs);
    
    // New Where Axis: --ari
    installWhereAxisCommands(interp);
    myTclEval(interp, "whereAxisInitialize");

    // New Search History Graph: --ari
    installShgCommands(interp);

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
    status_line::status_init(interp);

    ui_status = new status_line("UIM status");
    assert(ui_status);
    ui_status->message("ready");

/*******************************
 *    Main Loop for UIthread.
 ********************************/

   while (Tk_GetNumMainWindows() > 0) {
      // Before we block, let's process any pending tk DoWhenIdle events.
      processPendingTkEventsNoBlock();

      msgSize = UIMBUFFSIZE;
      mtag = MSG_TAG_ANY;
      int pollsender = msg_poll (&mtag, 1); // 1-->make this a blocking poll
                                            // i.e., not really a poll at all...
      // Why don't we do a blocking msg_recv() in all cases?  Because it soaks
      // up the pending message, throwing off the X file descriptor (tk wants to
      // dequeue itself).  Plus igen feels that way too.
      // NOTE: It would be nice if the above poll could take in a TIMEOUT argument,
      //       so we can handle tk "after <n millisec> do <script>" events properly.
      //       But libthread doesn't yet support time-bounded blocking!!!

      // check for X events or commands on stdin
      if (mtag == MSG_TAG_FILE) {
         // Note: why don't we do a msg_recv(), to consume the pending
         //       event?  Because both of the MSG_TAG_FILEs we have set
         //       up have the special flag set (in the call to msg_bind()),
         //       which indicated that we, instead of libthread, will take
         //       responsibility for that.  In other words, a msg_recv()
         //       now would not dequeue anything, so there's no point in doing it...

	 if (pollsender == xfd)
            processPendingTkEventsNoBlock();
         else if (pollsender == fileno(stdin))
            // process all pending stdin events
            StdinProc(NULL, 0);
         else
            cerr << "hmmm...unknown sender of a MSG_TAG_FILE message...ignoring" << endl;

	 // The above processing may have created some pending tk DoWhenIdle
	 // requests.  If so, process them now.
         processPendingTkEventsNoBlock();
      }
      else  {
         // check for upcalls
         if (dataMgr->isValidTag((T_dataManager::message_tags)mtag)) {
            if (dataMgr->waitLoop(true, (T_dataManager::message_tags)mtag) ==
		T_dataManager::error) {
               // TODO
               assert(0);
	    }
         }
         else if (uim_server->isValidTag((T_UI::message_tags)mtag)) {
            // check for incoming client requests
            if (uim_server->waitLoop(true, (T_UI::message_tags)mtag) ==
	       T_UI::error) {
	      // TODO
	      assert(0);
            }
	 }
         else
            panic("ui main loop: neither dataMgr nor uim_server report isValidTag() of true");
	 // The above processing may have created some pending tk DoWhenIdle
	 // requests.  If so, process them now.
         processPendingTkEventsNoBlock();
      }
   } 

   unInstallShgCommands(interp);
   unInstallWhereAxisCommands(interp);

   /*
    * Exiting this thread will signal the main/parent to exit.  No other
    * notification is needed.  This call will be reached when there are 
    * no windows remaining for the application -- either grievous error 
    * or user has selected "EXIT".
    */
   thr_exit(0);
   return ((void*)0);
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
 * StdinProc--
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

void
StdinProc(ClientData, int)
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
    code = Tcl_RecordAndEval(interp, cmd, TCL_EVAL_GLOBAL);
    Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
    Tcl_DStringFree(&command);
    if (*interp->result != 0) {
        if ((code != TCL_OK) || tty)
            puts(interp->result);
    }

    /*
     * Output a prompt.
     */

    prompt:
    if (tty)
        Prompt(interp, gotPartial);

    Tcl_ResetResult(interp);
}

void Prompt(Tcl_Interp *,         /* Interpreter to use for prompting. */
	    int)                  /* Non-zero means there already
				   * exists a partial command, so use
				   * the secondary prompt. */
{
   tunableBooleanConstant showTclPrompt = tunableConstantRegistry::findBoolTunableConstant("tclPrompt");
   if (showTclPrompt.getValue()) {
      fputs("pd> ", stdout);
      fflush(stdout);
   }
}
