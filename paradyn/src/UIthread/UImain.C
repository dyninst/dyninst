/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: UImain.C,v 1.107 2003/05/27 03:30:26 schendel Exp $

/* UImain.C
 *    This is the main routine for the User Interface Manager thread, 
 *    called at thread creation.
 */

#include "common/h/headers.h"

#if !defined(i386_unknown_nt4_0)
#include <sys/param.h>
#endif // !defined(i386_unknown_nt4_0)

#include "UIglobals.h" 
#include "paradyn/src/DMthread/DMinclude.h"
#include "dataManager.thread.h"
#include "pdthread/h/thread.h"
#include "../pdMain/paradyn.h"

#include "paradyn/src/TCthread/tunableConst.h"

#include "abstractions.h"
#include "whereAxisTcl.h"
#include "callGraphTcl.h"
#include "callGraphs.h"
#include "shgPhases.h"
#include "shgTcl.h"
#include "tkTools.h"
#include "pdutil/h/TclTools.h"

// Paradyn logo:
#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm" // defines logo_bits[], logo_width, logo_height
#include "paradyn/xbm/dont.xbm" // defines error_bits[], error_width, error_height

#include "common/h/Ident.h"
extern "C" const char V_paradyn[];
const Ident V_id(V_paradyn,"Paradyn");
extern "C" const char V_libpdutil[];
const Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libpdthread[];
const Ident V_Tid(V_libpdthread,"Paradyn");


#if defined(i386_unknown_nt4_0)
extern bool waitForKeypressOnExit;
#endif // defined(i386_unknown_nt4_0)


//----------------------------------------------------------------------------
// prototypes of functions used in this file
//----------------------------------------------------------------------------
static    void    ShowPrompt( bool continuation );
static    void    StdinInputHandler( Tcl_Interp* interp );

//----------------------------------------------------------------------------
// variables used in this file
//----------------------------------------------------------------------------
static    Tcl_Obj* stdinCmdObj = NULL;
static    bool stdinIsTTY = true;
thread_t stdin_tid = THR_TID_UNSPEC;


//----------------------------------------------------------------------------
// variables "global" to the UI thread
//----------------------------------------------------------------------------T
Tcl_Interp* interp = NULL;

extern abstractions *theAbstractions; // whereAxisTcl.C
extern bool inDeveloperMode;


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

   theAbstractions = new abstractions("",
                      ".whereAxis.top.mbar.nav.m",
                      ".whereAxis.nontop.main.bottsb",
                      ".whereAxis.nontop.main.leftsb",
                      ".whereAxis.nontop.find.entry",
                      interp, theTkWindow);
   assert(theAbstractions);
   
   return true;   
}


extern callGraphs *theCallGraphPrograms;
bool haveSeenFirstGoodCallGraphWid = false;

//Call Graph creation routine
bool tryFirstGoodCallGraphWid(Tcl_Interp *interp, Tk_Window topLevelTkWindow) {
   if (haveSeenFirstGoodCallGraphWid)
      return true;

   Tk_Window theTkWindow = Tk_NameToWindow(interp,".callGraph.nontop.main.all",
                                           topLevelTkWindow);
   assert(theTkWindow);
      
   if (Tk_WindowId(theTkWindow) == 0)
      return false; // sigh...still invalid (that's why this routine is needed)
   
   theCallGraphPrograms = 
     new callGraphs(".callGraph.titlearea.left.menu.mbar.program.m",
            ".callGraph.nontop.main.bottsb",
            ".callGraph.nontop.main.leftsb",
            ".callGraph.nontop.labelarea.current",
            ".callGraph.nontop.currprogramarea.label2",
            interp, theTkWindow);
   
   assert(theCallGraphPrograms);
   initiateCallGraphRedraw(interp, true);
   haveSeenFirstGoodCallGraphWid = true;

   return true;   
}



/*
 * Global variables used by tcl/tk UImain program:
 */

extern Tcl_Interp *interp;    /* Interpreter for this application. */


ListWithKey<metricInstInfo *, metricInstanceHandle> uim_enabled;
perfStreamHandle          uim_ps_handle;
UIM                       *uim_server;
int uim_maxError;
int uim_ResourceSelectionStatus;
int UIM_BatchMode = 0;
Tcl_HashTable UIMMsgReplyTbl;
Tcl_HashTable UIMwhereDagTbl;
int UIMMsgTokenID;
appState PDapplicState = appPaused;     // used to update run/pause buttons  

status_line *version=NULL;
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

extern void resourceRetiredCB(perfStreamHandle handle, resourceHandle uniqueID,
                              const char *name, const char *abs);

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

      assert(theAbstractions);

      theAbstractions->startBatchMode();
    } else {
      UIM_BatchMode--;

      if (UIM_BatchMode == 0) {
         // Batch mode is done with.  We need to update the where axis'
         // spatial graphications...
         ui_status->message("Rethinking after batch mode");

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
void UIenableDataResponse(pdvector<metricInstInfo> *,  u_int, u_int){
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


//
// tunable constant callbacks
//
// These callbacks could execute in any thread (e.g., if the tunable
// constant is set programmatically).  We have to ensure that Tcl interpreter
// access occurs from the UI thread only, so we delegate handling that
// requires Tcl access to the UI thread.
//

void
tclPromptCallback( bool show )
{
	uiMgr->showTclPrompt( show );
}

void
whereAxisShowTipsCallback( bool newVal )
{
	// ask the UI thread to show or hide the where axis tips
	uiMgr->showWhereAxisTips( newVal );
}

void
whereAxisHideRetiredResCallback(bool)
{
   
}

void
shgShowKeyCallback( bool newVal )
{
	// ask the UI thread to show or hide the PC key
	uiMgr->showSHGKey( newVal );
}

void
shgShowTipsCallback( bool newVal )
{
	// ask the UI thread to show or hide the PC tips
	uiMgr->showSHGTips( newVal );
}

void
shgShowTrueCallback( bool show )
{
	uiMgr->showSHGTrueNodes( show );
}

void
shgShowFalseCallback( bool show )
{
	uiMgr->showSHGFalseNodes( show );
}

void
shgShowUnknownCallback( bool show )
{
	uiMgr->showSHGUnknownNodes( show );
}

void
shgShowNeverCallback( bool show )
{
	uiMgr->showSHGNeverExpandedNodes( show );
}

void
shgShowActiveCallback( bool show )
{
	uiMgr->showSHGActiveNodes( show );
}

void
shgShowInactiveCallback( bool show )
{
	uiMgr->showSHGInactiveNodes( show );
}


void
shgShowShadowCallback( bool show )
{
	uiMgr->showSHGShadowNodes( show );
}


void
develModeCallback( bool newVal )
{
	inDeveloperMode = newVal;

	// the SHG needs to hear of changes in developer mode to resize
	// its status line appropriately
	uiMgr->setDeveloperMode( newVal );
}




void *UImain(void*) {
    thread_t mtid;
    tag_t mtag;
    int retVal;
    unsigned msgSize = 0;

    char UIMbuff[UIMBUFFSIZE];
    controlCallback controlFuncs;
    dataCallback dataFunc;

    PARADYN_DEBUG(("%s\n",V_paradyn));

	// Initialize tcl/tk
	interp = Tcl_CreateInterp();
	assert(interp);

	Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

	if( Tcl_Init(interp) == TCL_ERROR)
		tclpanic(interp, "tcl_init() failed (perhaps TCL_LIBRARY not set?)");

	if( Tk_Init(interp) == TCL_ERROR)
		tclpanic(interp, "tk_init() failed (perhaps TK_LIBRARY not set?)");


	// Here is one tunable constant that is definitely intended to be hard-coded in:
	tunableBooleanConstantDeclarator tcInDeveloperMode("developerMode",
		"Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)",
		false, 	// intial value
		develModeCallback,
		userConstant);

	tunableConstantRegistry::createFloatTunableConstant
		("EnableRequestPacketSize",
		 "Enable request packet size",
		 NULL,
		 developerConstant,
		 10.0, // initial value
		 1.0, // min
		 100.0); // max

	// initialize tunable constants
    tunableBooleanConstantDeclarator tcWaShowTips("showWhereAxisTips",
        "If true, the WhereAxis window will be drawn with helpful reminders"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
        true, // default value
        whereAxisShowTipsCallback,
        userConstant);

	// initialize tunable constants
    tunableBooleanConstantDeclarator tcHideRetiredRes(
        "whereAxisHideRetiredRes",
        "If true, the WhereAxis window will not display resource"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
        false, // default value
        whereAxisHideRetiredResCallback,
        userConstant);
                          
    tunableBooleanConstantDeclarator tcShgShowKey("showShgKey",
    "If true, the search history graph will be drawn with a key for"
        " decoding the meaning of the several background colors, text colors,"
        " italics, etc.  A setting of false saves screen real estate.",
    true, // default value
    shgShowKeyCallback,
    userConstant);

    tunableBooleanConstantDeclarator tcShgShowTips("showShgTips",
    "If true, the search history graph will be drawn with reminders"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
    true, // default value
    shgShowTipsCallback,
    userConstant);

    tunableBooleanConstantDeclarator tcHideTcl("tclPrompt",
        "Allow access to a command-line prompt accepting arbitrary tcl"
        " commands in the Paradyn process.",
        false, // default value
        tclPromptCallback,
        developerConstant);

    tunableBooleanConstantDeclarator tcShowTrue("showShgTrueNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all true nodes"
        " (background colored blue).",
        true, // default value
        shgShowTrueCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideFalse("showShgFalseNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all false nodes"
        " (background colored pink).",
        true, // default value
        shgShowFalseCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideUnknown("showShgUnknownNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all nodes with"
        " an unknown value (background colored green).",
        true, // default value
        shgShowUnknownCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideNever("showShgNeverSeenNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all"
        " never-before-seen nodes (background colored gray).",
        true, // default value
        shgShowNeverCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideActive("showShgActiveNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all active nodes"
        " (foreground text white).",
        true, // default value
        shgShowActiveCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideInactive("showShgInactiveNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all inactive nodes"
        " (foreground text black).",
        true, // default value
        shgShowInactiveCallback,
        userConstant);

    tunableBooleanConstantDeclarator tcHideShadow("showShgShadowNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all true nodes.",
        true, // default value
        shgShowShadowCallback,
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
        // we already indicated the error to the user
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
    
    char buf[32];
    if (V_id.OK())
        sprintf (buf, "setTitleVersion %s", V_id.release());
    else
        sprintf (buf, "setTitleVersion v3");    // set a reasonable? default
    myTclEval(interp, buf);

    // initialize number of errors read in from error database 
    Tcl_VarEval (interp, "getNumPdErrors", (char *)NULL);
    uim_maxError = atoi(Tcl_GetStringResult(interp));

    // Initialize UIM thread as UIM server 
    thr_name ("UIM");
    uim_server = new UIM(MAINtid);

    // register fd for X events with threadlib as special
    thread_t xtid;
#if !defined(i386_unknown_nt4_0)
    Display *UIMdisplay = Tk_Display (Tk_MainWindow(interp));
    int xfd = XConnectionNumber (UIMdisplay);
    retVal = msg_bind_socket (xfd,
               1, // "special" flag --> libthread leaves it to us to manually
                         // dequeue these messages
                NULL,
                NULL,
                &xtid
               );
#else // !defined(i386_unknown_nt4_0)
    retVal = msg_bind_wmsg( &xtid );
#endif // !defined(i386_unknown_nt4_0)

    // initialize hash table for async call replies
    Tcl_InitHashTable (&UIMMsgReplyTbl, TCL_ONE_WORD_KEYS);
    UIMMsgTokenID = 0;

    // wait for all other main module threads to complete initialization
    //  before continuing.

    retVal = msg_send (MAINtid, MSG_TAG_UIM_READY, (char *) NULL, 0);
    mtag = MSG_TAG_ALL_CHILDREN_READY;
    mtid = MAINtid;
    retVal = msg_recv (&mtid, &mtag, UIMbuff, &msgSize);
    assert( mtid == MAINtid );

    PARADYN_DEBUG(("UIM thread past barrier\n"));

    // subscribe to DM new resource notification service

    controlFuncs.rFunc = resourceAddedCB;
    controlFuncs.retireFunc = resourceRetiredCB;
    controlFuncs.mFunc = NULL;
    controlFuncs.fFunc = NULL;
    controlFuncs.sFunc = applicStateChanged;
    controlFuncs.bFunc = resourceBatchChanged;
    controlFuncs.pFunc = NULL;
    controlFuncs.eFunc = UIenableDataResponse;
    dataFunc.sample = NULL;

    uim_ps_handle = dataMgr->createPerformanceStream
      (Sample, dataFunc, controlFuncs);
    
    // New Where Axis: --ari
    installWhereAxisCommands(interp);
    myTclEval(interp, "whereAxisInitialize");

    // New Search History Graph: --ari
    installShgCommands(interp);

    // New Call Graph --trey
    installCallGraphCommands(interp);

    // set up for reading Tcl commands from the command line, if needed
#if !defined(i386_unknown_nt4_0)
    stdinIsTTY = isatty( 0 );
#else
    stdinIsTTY = _isatty( _fileno( stdin ) );
#endif // defined(i386_unknown_nt4_0)
    tunableBooleanConstant showTclPrompt = tunableConstantRegistry::findBoolTunableConstant("tclPrompt");
    if( showTclPrompt.getValue() )
    {
        InstallStdinHandler();
    }

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
      thread_t pollsender = THR_TID_UNSPEC;
      mtag = MSG_TAG_ANY;

      // returns an integer variable indicating any errors
      msg_poll (&pollsender, &mtag, 1); // 1-->make this a blocking poll
                                            // i.e., not really a poll at all...
      // Why don't we do a blocking msg_recv() in all cases?  Because it soaks
      // up the pending message, throwing off the X file descriptor (tk wants to
      // dequeue itself).  Plus igen feels that way too.
      // NOTE: It would be nice if the above poll could take in a TIMEOUT argument,
      //       so we can handle tk "after <n millisec> do <script>" events properly.
      //       But libthread doesn't yet support time-bounded blocking!!!

      // check for X events or commands on stdin
      if (mtag == MSG_TAG_SOCKET) {
         // Note: why don't we do a msg_recv(), to consume the pending
         //       event?  Because both of the MSG_TAG_FILE we have set
         //       up have the special flag set (in the call to msg_bind()),
         //       which indicated that we, instead of libthread, will take
         //       responsibility for that.  In other words, a msg_recv()
         //       now would not dequeue anything, so there's no point in doing it...

     if (pollsender == xtid)
	 {
            processPendingTkEventsNoBlock();
			clear_ready_sock( thr_socket( xtid ) );
	 }
         else
            cerr << "hmmm...unknown sender of a MSG_TAG_SOCKET message...ignoring" << endl;

     // The above processing may have created some pending tk DoWhenIdle
     // requests.  If so, process them now.
         processPendingTkEventsNoBlock();
      }
#if defined(i386_unknown_nt4_0)
        else if( mtag == MSG_TAG_WMSG )
        {
            // there are events in the Windows message queue - handle them
            processPendingTkEventsNoBlock();
        }
#endif // defined(i386_unknown_nt4_0)

#if !defined(i386_unknown_nt4_0)
        else if( mtag == MSG_TAG_FILE )
        {
            // we only have one file bound - stdin
            StdinInputHandler( interp );

            // The above processing may have created some pending tk DoWhenIdle
            // requests.  If so, process them now.
            processPendingTkEventsNoBlock();
        }
#endif // defined(i386_unknown_nt4_0)
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
   unInstallCallGraphCommands(interp);

   Tcl_DeleteInterp( interp );

#if defined(i386_unknown_nt4_0)
    // In many places throughout our code we dump error or status message
    // to stderr and then exit.  On Windows, we dump stdout and stderr to 
    // a Windows console window.  Unless we keep this window up at
    // program exit, the user doesn't get a chance to see the messages.
    // 
    // However, in case of a graceful exit, we just want the program to
    // quit without forcing the user to dismiss the console window.
    waitForKeypressOnExit = false;
#endif // defined(i386_unknown_nt4_0)

   /*
    * Exiting this thread will signal the main/parent to exit.  No other
    * notification is needed.  This call will be reached when there are 
    * no windows remaining for the application -- either grievous error 
    * or user has selected "EXIT".
    */
   thr_exit(0);
   return ((void*)0);
}
    




// InstallStdinHandler - install a channel handler for stdin to read
// commands and execute them when a complete command is recognized
//
void
InstallStdinHandler( void )
{
    if( stdinCmdObj != NULL )
    {
        Tcl_SetStringObj( stdinCmdObj, "", 0 );
    }
    else
    {
        stdinCmdObj = Tcl_NewStringObj( "", -1 );

        // we need to hold onto this string object 
        Tcl_IncrRefCount( stdinCmdObj );
    }
    if( stdinIsTTY )
    {
        ShowPrompt( false );
    }
}




// UninstallStdinHandler - remove the channel handler for stdin
// used on exit or in response to tclPrompt changing to false
//
void
UninstallStdinHandler( void )
{
    Tcl_Channel ochan = Tcl_GetStdChannel( TCL_STDOUT );


    // dump something to show that the prompt is no longer valid
    Tcl_WriteChars( ochan, "<done>", -1 );
    Tcl_Flush( ochan );

    // unbind thread library from stdin
    assert( stdin_tid != THR_TID_UNSPEC );
    msg_unbind( stdin_tid );
}





// StdinInputHandler - channel handler, called when there is input on stdin
//
static
void
StdinInputHandler( Tcl_Interp* interp )
{
    Tcl_Channel ichan = Tcl_GetStdChannel( TCL_STDIN );
    bool continuation = false;


    // read the available input, appending to any existing input
    Tcl_GetsObj( ichan, stdinCmdObj );

    // check if we have a complete command
    if( Tcl_GetCharLength( stdinCmdObj ) > 0 )
    {
        if( Tcl_CommandComplete( Tcl_GetStringFromObj( stdinCmdObj, NULL )) ) 
        {
            // issue the command
            int evalRes = Tcl_EvalObjEx( interp, stdinCmdObj, TCL_EVAL_DIRECT );

            // output the result
            Tcl_Channel ochan = Tcl_GetStdChannel( (evalRes == TCL_OK) ? 
                                                    TCL_STDOUT : TCL_STDERR );
            Tcl_WriteChars( ochan, Tcl_GetStringResult( interp ), -1 );
            Tcl_WriteChars( ochan, "\n", -1 );
            Tcl_Flush( Tcl_GetStdChannel( TCL_STDOUT ) );

            // clear our old command
            continuation = false;    // we had a complete command
            Tcl_SetObjLength( stdinCmdObj, 0 );
        }
        else
        {
            // we've already saved the input for later completion
            continuation = true;
        }
    }
    else if( Tcl_Eof( ichan ) )
    {
        // we've reached EOF on the stdin file - stop looking for input
        if( !stdinIsTTY )
        {
            UninstallStdinHandler();
        }
    }

    // show prompt if needed
    if( stdinIsTTY )
    {
        ShowPrompt( continuation );
    }
}


static
void
ShowPrompt( bool continuation )
{
    Tcl_Channel ochan = Tcl_GetStdChannel( TCL_STDOUT );

    // issue an appropriate prompt
    Tcl_WriteChars( ochan, continuation ? " +> " : "pd> ", -1 );
    Tcl_Flush( ochan );
}


