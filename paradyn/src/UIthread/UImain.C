/* $Log: UImain.C,v $
/* Revision 1.78  1996/03/08 03:00:34  tamches
/* fixed hide-node bug whereby a tc change before PC window was open would
/* give an assertion failure
/*
 * Revision 1.77  1996/03/08 00:20:40  tamches
 * added 7 tunable constants for hiding desired shg nodes
 *
 * Revision 1.76  1996/02/21 18:15:50  tamches
 * cleanup of applicStateChanged related to commit of paradyn.tcl.C
 *
 * Revision 1.75  1996/02/15 23:06:27  tamches
 * added support for phase 0, the initial current phase
 *
 * Revision 1.74  1996/02/08 01:00:03  tamches
 * implementing starting a phase w/ pc
 *
 * Revision 1.73  1996/02/07 21:46:38  tamches
 * defineNewSearch returns bool flag
 *
 * Revision 1.72  1996/02/07 19:04:37  tamches
 * added deferred-phase-adding features
 *
 * Revision 1.71  1996/02/05 18:51:47  newhall
 * Change to DM interface: StartPhase and newPhaseCallback
 *
 * Revision 1.70  1996/02/02  18:39:18  tamches
 * added prelim version of ui_newPhaseDetected
 * added shgShowKey, shgShowTips tunables
 *
 * Revision 1.69  1996/01/09 23:55:18  tamches
 * moved whereAxisDrawTipsCallback to whereAxisTcl.C
 * added tclPromptCallback to better implement the "tclPrompt"
 * tunable constant.
 * On startup, we no longer msg_bind to stdin
 *
 * Revision 1.68  1995/12/20 02:27:25  tamches
 * removed some warnings
 *
 * Revision 1.67  1995/11/29 00:18:58  tamches
 * Paradyn logo is now hard-coded; PARADYNTCL/PdBitmapDir are now
 * obsolete.
 *
 * Revision 1.66  1995/11/20 03:22:38  tamches
 * added showWhereAxisTips tunable constant, and related code
 * added tclPrompt tunable constant, and related code
 * removed set auto_path
 *
 * Revision 1.65  1995/11/09 17:11:35  tamches
 * deleted some obsolete stuff which had been commented out (e.g. uim_rootRes).
 * added UIMBUFFSIZE (moved from UIglobals.h)
 *
 * Revision 1.64  1995/11/08 06:24:15  tamches
 * removed some warnings
 *
 * Revision 1.63  1995/11/08 05:10:03  tamches
 * removed reference to obsolete file dag.h
 *
 * Revision 1.62  1995/11/06 02:40:19  tamches
 * added an include to tkTools.h
 * removed several warnings
 * UImain() no longer takes in any args
 *
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

extern Tk_Window mainWindow;	/* The main window for the application.  If
				 * NULL then the application no longer
				 * exists. */
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
      // cout << "+" << endl; cout.flush();
    } else {
      UIM_BatchMode--;
      // cout << "-" << endl; cout.flush();
      if (UIM_BatchMode == 0) {
         // Batch mode is done with.  We need to update the where axis'
         // spatial graphications...
         ui_status->message("Rethinking after batch mode");

         extern abstractions *theAbstractions;
         assert(theAbstractions);

         theAbstractions->resizeEverything();
            // super-expensive

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

// The following two variables tell the shg which phase to try to
// activate when _first_ opening the shg window.  We initialize it
// to the well-known values for the "current phase" which is
// created on startup.
int latest_detected_new_phase_id = 1;
const char *latest_detected_new_phase_name = "phase_0";

void ui_newPhaseDetected(perfStreamHandle,
			 const char *name, phaseHandle ph,
			 timeStamp begin, timeStamp end,
			 float bucketwidth,
			 bool with_new_pc,
			 bool with_visis) {
//   cout << "welcome to new_phase_detected" << endl;
//   cout << "id=" << ph + 1 << endl;
//   cout << "name=" << name << endl;
//   cout << "begin=" << begin << "; end=" << end << endl;

   // For the benefit of the shg, in the event that the shg window
   // has not yet been opened, with the result that "theShgPhases"
   // hasn't yet been constructed:
   extern shgPhases *theShgPhases;
   if (theShgPhases == NULL) {
      latest_detected_new_phase_id = ph + 1;
      latest_detected_new_phase_name = name;
      //cout << "ui_newPhaseDetected: deferring phase id " << ph+1 << " (" << name << ") since shg window not yet opened" << endl;
      if (with_new_pc)
         cout << "can't begin searching the new phase since Perf Consultant window not yet opened" << endl;
   }
   else {
      //cout << "ui_newPhaseDetected: adding the phase now" << endl;
      perfConsult->newSearch(CurrentPhase);
      bool redraw = theShgPhases->defineNewSearch(ph+1, name);

      if (with_new_pc) {
         // the user has requested that we begin searching immediately on this
         // new phase, as if we had clicked on the "Search" button.  So let's do
         // the equivalent.  But first, we must switch to the new "screen".
	 assert(theShgPhases->changeByPhaseId(ph+1));

	 myTclEval(interp, "shgClickOnSearch");
	    // calls shgSearchCommand (shgTcl.C), which calls activateCurrSearch()
            // in shgPhases.C

         //cout << "ui_newPhaseDetected: started the new search!" << endl;

	 redraw = true;
      }
      
      if (redraw)
         initiateShgRedraw(interp, true);
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
		      UimpdCmd, (ClientData) mainWindow,
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
        fprintf(stderr, "initialize_tcl_sources: ERROR in Tcl sources, exitting\n");
        exit(-1);
    }
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/applic.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/errorList.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/focusUtils.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/generic.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/mainMenu.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/mets.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/shg.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/startVisi.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/status.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/tclTunable.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/uimProcs.tcl"));
//assert(TCL_OK==Tcl_EvalFile(interp, "/p/paradyn/development/tamches/core/paradyn/tcl/whereAxis.tcl"));

   // Initialize the 2 bitmaps we use
   pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width, logo_height);
   pdLogo::install_fixed_logo("dont", error_bits, error_width, error_height);

   // now install the tcl cmd "createPdLogo" (must be done before anyone
   // tries to create a logo)
   tcl_cmd_installer createPdLogo(interp, "makeLogo", pdLogo::makeLogoCommand,
				  (ClientData)mainWindow);

   /* display the paradyn main menu tool bar */
   myTclEval(interp, "drawToolBar");

     // initialize number of errors read in from error database 
    uim_maxError = atoi(Tcl_GetVar (interp, "numPdErrors", 0));

    // Initialize "command", an important global variable that accumulates
    // a typed-in line of data.  It gets updated in StdinProc(), below.
    Tcl_DStringInit(&command);
    if (tty)
      Prompt(interp, 0);

    // Initialize UIM thread as UIM server 
    thr_name ("UIM");
    uim_server = new UIM(MAINtid);

    // register fd for X events with threadlib as special
    Display *UIMdisplay = Tk_Display (mainWindow);
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
    controlFuncs.pFunc = ui_newPhaseDetected;
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

   while (tk_NumMainWindows > 0) {
      processPendingTkEventsNoBlock();

      msgSize = UIMBUFFSIZE;
      mtag = MSG_TAG_ANY;
      int pollsender = msg_poll (&mtag, 1); // 1-->make this a blocking poll
                                            // i.e., not really a poll at all...
      // Why don't we do a blocking msg_recv() in all cases?  Because it soaks
      // up the pending message, throwing off the X file descriptor (tk wants to
      // dequeue itself).  Plus igen feels that way too.

//      processPendingTkEventsNoBlock();

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
