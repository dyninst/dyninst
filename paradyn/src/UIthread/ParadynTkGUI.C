/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: ParadynTkGUI.C,v 1.7 2004/07/14 18:24:12 eli Exp $
#include "pdutil/h/TclTools.h"
#include "ParadynTkGUI.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "UIglobals.h"
#include "pdLogo.h"
#include "tkTools.h"
#include "shgPhases.h"
#include "shgTcl.h"
#include "callGraphs.h"
#include "callGraphTcl.h"
#include "abstractions.h"
#include "whereAxisTcl.h"
#include "common/h/Ident.h"


#include "paradyn/xbm/logo.xbm" // defines Paradyn bitmap info
#include "paradyn/xbm/dont.xbm" // defines error bitmap info


int initialize_tcl_sources(Tcl_Interp *);

extern abstractions* theAbstractions;
extern callGraphs* theCallGraphPrograms;
extern shgPhases* theShgPhases;



status_line *version=NULL;
status_line *ui_status=NULL;
status_line *app_status=NULL;


unsigned
metric_name_hash(const unsigned int& metid)
{
    return metid;
}



//----------------------------------------------------------------------------
// ParadynTkGUI implementation

ParadynTkGUI::ParadynTkGUI( thread_t _mainTid, pdstring _progName )
  : ParadynTclUI( _mainTid, _progName ),
    xtid( THR_TID_UNSPEC ),
    haveSeenFirstWhereAxisWindow( false ),
    haveSeenFirstCallGraphWindow( false ),
    haveSeenFirstShgWindow( false ),
    batchModeCounter( 0 ),
    PDapplicState( appPaused ),
    all_metrics_set_yet( false ),
    all_metric_names(metric_name_hash, 16),
    latest_detected_new_phase_id( 1 ),
    latest_detected_new_phase_name( "phase_0" )
{
}



ParadynTkGUI::~ParadynTkGUI( void )
{
    unInstallShgCommands( interp );
    unInstallWhereAxisCommands( interp );
    unInstallCallGraphCommands( interp );
}


bool
ParadynTkGUI::Init( void )
{
    // initialize our base class
    if( !ParadynTclUI::Init() )
    {
        return false;
    }

    // do our own initialization
    if( Tk_Init( interp ) == TCL_ERROR )
    {
        Panic( "Tk_Init() failed (perhaps TK_LIBRARY not set?)" );
    }

#if READY
    Tk_Window mainWin = Tk_MainWindow( interp );
    if( mainWin == NULL )
    {
        Panic( "Tk_MainWindow gave NULL after Tk_Init" );
    }
    Tk_SetClass( mainWin, "Paradyn" );
#endif // READY

    tunableBooleanConstantDeclarator* tcWaShowTips = 
        new tunableBooleanConstantDeclarator("showWhereAxisTips",
        "If true, the WhereAxis window will be drawn with helpful reminders"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
        true, // default value
        ShowWhereAxisTipsCallback,
        userConstant);

	// initialize tunable constants
    tunableBooleanConstantDeclarator* tcHideRetiredRes = 
        new tunableBooleanConstantDeclarator("whereAxisHideRetiredRes",
        "If true, the WhereAxis window will not display resource"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
        false, // default value
        HideWhereAxisRetiredResCallback,
        userConstant);
                          
    tunableBooleanConstantDeclarator* tcShgShowKey = 
        new tunableBooleanConstantDeclarator("showShgKey",
    "If true, the search history graph will be drawn with a key for"
        " decoding the meaning of the several background colors, text colors,"
        " italics, etc.  A setting of false saves screen real estate.",
    true, // default value
    ShowShgKeyCallback,
    userConstant);

    tunableBooleanConstantDeclarator* tcShgShowTips = 
        new tunableBooleanConstantDeclarator("showShgTips",
    "If true, the search history graph will be drawn with reminders"
        " on shortcuts for expanding, unexpanding, selecting, and scrolling."
        "  A setting of false saves screen real estate.",
    true, // default value
    ShowShgTipsCallback,
    userConstant);

    tunableBooleanConstantDeclarator* tcShowTrue = 
        new tunableBooleanConstantDeclarator("showShgTrueNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all true nodes"
        " (background colored blue).",
        true, // default value
        ShowShgTrueCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideFalse = 
        new tunableBooleanConstantDeclarator("showShgFalseNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all false nodes"
        " (background colored pink).",
        true, // default value
        ShowShgFalseCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideUnknown = 
        new tunableBooleanConstantDeclarator("showShgUnknownNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all nodes with"
        " an unknown value (background colored green).",
        true, // default value
        ShowShgUnknownCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideNever =
        new tunableBooleanConstantDeclarator("showShgNeverSeenNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all"
        " never-before-seen nodes (background colored gray).",
        true, // default value
        ShowShgNeverCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideActive =
        new tunableBooleanConstantDeclarator("showShgActiveNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all active nodes"
        " (foreground text white).",
        true, // default value
        ShowShgActiveCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideInactive = 
        new tunableBooleanConstantDeclarator("showShgInactiveNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all inactive nodes"
        " (foreground text black).",
        true, // default value
        ShowShgInactiveCallback,
        userConstant);

    tunableBooleanConstantDeclarator* tcHideShadow =
        new tunableBooleanConstantDeclarator("showShgShadowNodes",
        "To save space in the Performance Consultant Search History Graph,"
        " a false setting of this tunable constant will hide all true nodes.",
        true, // default value
        ShowShgShadowCallback,
        userConstant);


    // load the TCL sources into the interpreter
    if(initialize_tcl_sources( interp ) != TCL_OK)
    {
        // we already indicated the error to the user
        return false;
    }

    // Initialize the bitmaps we use
    pdLogo::install_fixed_logo("paradynLogo", logo_bits,
                                logo_width, logo_height);
    pdLogo::install_fixed_logo("dont", error_bits,
                                error_width, error_height);
    
    // now install the tcl cmd "createPdLogo" (must be done before anyone
    // tries to create a logo)
    tcl_cmd_installer createPdLogo( interp, "makeLogo", pdLogo::makeLogoCommand,
                   (ClientData)Tk_MainWindow( interp ));

    /* display the paradyn main menu tool bar */
    myTclEval( interp, "buildMainWindow");
    
    // New Where Axis: --ari
    installWhereAxisCommands( interp );
    myTclEval( interp, "whereAxisInitialize");

    // New Search History Graph: --ari
    installShgCommands( interp );

    // New Call Graph --trey
    installCallGraphCommands( interp );

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
    status_line::status_init( interp );

    ui_status = new status_line("UIM status");
    assert(ui_status);
    ui_status->message("ready");

    // set our version number
    char buf[32];
    sprintf( buf, "setTitleVersion %s", (V_id.OK() ? V_id.release() : "v4") );
    myTclEval( interp, buf);

    // register fd for X events with threadlib as special
#if !defined(i386_unknown_nt4_0)
    Display *UIMdisplay = Tk_Display( Tk_MainWindow( interp ) );
    int xfd = XConnectionNumber( UIMdisplay );
    msg_bind_socket( xfd,
                       1, // libthread leaves it to us to manually dequeue
                       NULL,
                       NULL,
                       &xtid );
#else // !defined(i386_unknown_nt4_0)
    msg_bind_wmsg( &xtid );
#endif // !defined(i386_unknown_nt4_0)

    return true;
}


bool
ParadynTkGUI::IsDoneHandlingEvents( void ) const
{
   return (Tk_GetNumMainWindows() == 0);
}


void
ParadynTkGUI::DoPendingTkEvents( void )
{
    // We use Tk_DoOneEvent (w/o blocking) to soak up and process
    // pending tk events, if any.  Returns as soon as there are no
    // tk events to process.
    // NOTE: This includes (as it should) tk idle events.
    // NOTE: This is basically the same as Tcl_Eval(interp, "update"), but
    //       who wants to incur the expense of tcl parsing?
    while(Tk_DoOneEvent(TK_DONT_WAIT) > 0)
    {
        // nothing else to do
    }
}


void
ParadynTkGUI::DoPendingWork( void )
{
    // do our own pending work
    DoPendingTkEvents();
}


bool
ParadynTkGUI::HandleEvent( thread_t mtid, tag_t mtag )
{
    bool ret = false;

    if( mtag == MSG_TAG_SOCKET )
    {
        if( mtid == xtid )
        {
            // it is a Tk or X event -
            // let Tk dispatch the event
            DoPendingTkEvents();
            ret = true;

            // let the thread library know we handled the event
            clear_ready_sock( thr_socket( xtid ) );
        }
    }
#if defined(i386_unknown_nt4_0)
    else if( mtag == MSG_TAG_WMSG )
    {
        // it is a Windows event
        // again, let Tk handle it
        DoPendingTkEvents();
        ret = true;

        // consume the message
        msg_recv( &mtid, &mtag, NULL, 0 );
    }
#endif // defined(i386_unknown_nt4_0)

    if( !ret )
    {
        // let our base class try to handle the event
        ret = ParadynTclUI::HandleEvent( mtid, mtag );
    }

    return ret;
}


void
ParadynTkGUI::setDeveloperMode( bool newVal )
{
    ParadynUI::setDeveloperMode( newVal );

	// the SHG needs to hear of changes in developer mode to resize
	// its status line appropriately
	shgDevelModeChange( interp, newVal );
}



//
// tunable constant callbacks
//
// These callbacks could execute in any thread (e.g., if the tunable
// constant is set programmatically).  We have to ensure that Tcl interpreter
// access occurs from the UI thread only, so we delegate handling that
// requires Tcl access to the UI thread.
//

void
ParadynTkGUI::ShowWhereAxisTipsCallback( bool newVal )
{
	// ask the UI thread to show or hide the where axis tips
	uiMgr->showWhereAxisTips( newVal );
}

void
ParadynTkGUI::HideWhereAxisRetiredResCallback(bool)
{
    // TODO implement this?   
}

void
ParadynTkGUI::ShowShgKeyCallback( bool newVal )
{
	// ask the UI thread to show or hide the PC key
	uiMgr->showSHGKey( newVal );
}

void
ParadynTkGUI::ShowShgTipsCallback( bool newVal )
{
	// ask the UI thread to show or hide the PC tips
	uiMgr->showSHGTips( newVal );
}

void
ParadynTkGUI::ShowShgTrueCallback( bool show )
{
	uiMgr->showSHGTrueNodes( show );
}

void
ParadynTkGUI::ShowShgFalseCallback( bool show )
{
	uiMgr->showSHGFalseNodes( show );
}

void
ParadynTkGUI::ShowShgUnknownCallback( bool show )
{
	uiMgr->showSHGUnknownNodes( show );
}

void
ParadynTkGUI::ShowShgNeverCallback( bool show )
{
	uiMgr->showSHGNeverExpandedNodes( show );
}

void
ParadynTkGUI::ShowShgActiveCallback( bool show )
{
	uiMgr->showSHGActiveNodes( show );
}

void
ParadynTkGUI::ShowShgInactiveCallback( bool show )
{
	uiMgr->showSHGInactiveNodes( show );
}


void
ParadynTkGUI::ShowShgShadowCallback( bool show )
{
	uiMgr->showSHGShadowNodes( show );
}



bool
ParadynTkGUI::TryFirstWhereAxisWindow( void )
{
   if (haveSeenFirstWhereAxisWindow)
      return true;

   Tk_Window topLevelTkWindow = Tk_MainWindow( interp );
   Tk_Window theTkWindow = Tk_NameToWindow(interp, ".whereAxis.nontop.main.all",
                                           topLevelTkWindow);
   assert(theTkWindow);

   if (Tk_WindowId(theTkWindow) == 0)
      return false; // sigh...still invalid (that's why this routine is needed)

   haveSeenFirstWhereAxisWindow = true;

   theAbstractions = new abstractions("",
                      ".whereAxis.top.mbar.nav.m",
                      ".whereAxis.nontop.main.bottsb",
                      ".whereAxis.nontop.main.leftsb",
                      ".whereAxis.nontop.find.entry",
                      interp, theTkWindow);
   assert(theAbstractions);
   
   return true;   
}



bool
ParadynTkGUI::TryFirstCallGraphWindow( void )
{
   if (haveSeenFirstCallGraphWindow)
      return true;

   Tk_Window topLevelTkWindow = Tk_MainWindow( interp );
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
   haveSeenFirstCallGraphWindow = true;

   return true;   
}



void
ParadynTkGUI::ResourceBatchChanged( perfStreamHandle, batchMode mode )
{
    if (mode == batchStart) {
      ui_status->message("receiving where axis items [batch mode]");

      batchModeCounter++;

      assert(theAbstractions);

      theAbstractions->startBatchMode();
    } else {
      batchModeCounter--;

      if (batchModeCounter == 0) {
         // Batch mode is done with.  We need to update the where axis'
         // spatial graphications...
         ui_status->message("Rethinking after batch mode");

         assert(theAbstractions);

     theAbstractions->endBatchMode();
        // does: resizeEverything(true); (resorts, rethinks layout.  expensive)

         initiateWhereAxisRedraw( interp, true); // true--> double buffer

         ui_status->message("ready");

         // Shouldn't we also Tcl_Eval(interp, "update"), to process any
     // pending idle events?
      }
    }
    assert(batchModeCounter >= 0);
}

void
ParadynTkGUI::ApplicStateChanged(perfStreamHandle, appState newstate)
{
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


void
ParadynTkGUI::ResourceAdded(perfStreamHandle, resourceHandle parent, 
                     resourceHandle newResource, const char *name,
                     const char *abs)
{
#if UIM_DEBUG
  printf ("resourceAddedCB %s\n", name);
#endif

  const bool inBatchMode = IsInBatchMode();
  if (!inBatchMode)
     ui_status->message("receiving where axis item");

  extern abstractions *theAbstractions;
  assert(theAbstractions);

  abstractions &theAbs = *theAbstractions;
  pdstring theAbstractionName = abs;
  whereAxis &theWhereAxis = theAbs[theAbstractionName];
     // may create a where axis!

  // Strip away all path components up to our name. Look for '/', but
  // be careful not to strip away "operator/".
  const char *nameLastPart = strstr(name, "operator/");
  if (nameLastPart == 0) {
      nameLastPart = strrchr(name, '/');
  }
  else {
#ifdef FIXME_AFTER_4
      cerr << "WARNING: Change the whereAxis separator from '/' to something\n"
	   << "else. Otherwise, operator/ needs to be special-cased\n";
#endif
      // Scan back from the current point looking for '/'. Can't use
      // strrchr, since it always scans from the end of the string.
      while (nameLastPart != name && *nameLastPart != '/') {
	  nameLastPart--;
      }
  }

#ifdef i386_unknown_nt4_0
  // under Windows, we may find our Code resources
  // are paths containing backslashes as separators
  // instead of forward slashes.  The strrchr on
  // the forward slash gets us to the path name, now
  // we need to extract the last component of the path.
  const char* backslashNameLastPart = strrchr(nameLastPart, '\\');
  if( backslashNameLastPart != NULL )
  {
    nameLastPart = backslashNameLastPart;
  }
#endif // i386_unknown_nt4_0

  assert(nameLastPart);
  nameLastPart++;

  theWhereAxis.addItem(nameLastPart, parent, newResource,
		       false, // don't rethink graphics
		       false // don't resort (if not in batch mode, code below
		             // will do that, so don't worry)
		       );

  if (!inBatchMode) {
     ui_status->message("Rethinking after a non-batch receive");

     theWhereAxis.recursiveDoneAddingChildren(true); // true --> resort
     theWhereAxis.resize(true);
        // super-expensive operation...rethinks EVERY node's
        // listbox & children dimensions.  Actually, this only needs
        // to be done for the just-added node and all its ancestors.
     initiateWhereAxisRedraw( interp, true);

     ui_status->message("ready");
  }
}


void
ParadynTkGUI::ResourceRetired(perfStreamHandle /* h */,
                       resourceHandle uniqueID, 
                       const char* /* name */)
{
   extern abstractions *theAbstractions;
   assert(theAbstractions);
   
   abstractions &theAbs = *theAbstractions;
   pdstring theAbstractionName = NULL;
   whereAxis &theWhereAxis = theAbs[theAbstractionName];
   // may create a where axis!

   theWhereAxis.retireItem(uniqueID);
   initiateWhereAxisRedraw( interp, true);
}


void 
ParadynTkGUI::chooseMetricsandResources(chooseMandRCBFunc cb,
			       pdvector<metric_focus_pair> * /* pairList */ )
{
      // store record with unique id and callback function
  UIMMsgTokenID++;
  int newptr;
  Tcl_HashEntry *entryPtr = Tcl_CreateHashEntry (&UIMMsgReplyTbl,
						 (char *)UIMMsgTokenID, 
						 &newptr);
  if (newptr == 0) {
    showError(21, "");
    thr_exit(0);
  }

  unsigned requestingThread = getRequestingThread();
     // in theory, we can check here whether this (VISI-) thread already
     // has an outstanding metric request.  But for now, we let code in mets.tcl do this...
//  pdstring commandStr = pdstring("winfo exists .metmenunew") + pdstring(requestingThread);
//  myTclEval(interp, commandStr);
//  int result;
//  assert(TCL_OK == Tcl_GetBoolean(interp, Tcl_GetStringResult(interp), &result));
//  if (result)
//     return; // the window is already up for this thread!

  UIMReplyRec *reply = new UIMReplyRec;
  reply->tid = requestingThread;
  reply->cb = (void *) cb;
  Tcl_SetHashValue (entryPtr, reply);

  if (!all_metrics_set_yet) {
      pdvector<met_name_id> *all_mets = dataMgr->getAvailableMetInfo(true);
      
      for (unsigned metlcv=0; metlcv < all_mets->size(); metlcv++) {
	 unsigned id  = (*all_mets)[metlcv].id;
	 pdstring &name = (*all_mets)[metlcv].name;

	 all_metric_names[id] = name;

	 pdstring idString(id);
	 bool aflag;
	 aflag=(Tcl_SetVar2(interp, "metricNamesById", 
			    const_cast<char*>(idString.c_str()),
			    const_cast<char*>(name.c_str()), 
			    TCL_GLOBAL_ONLY) != NULL);
         assert(aflag);
      }
      
      delete all_mets;
      all_metrics_set_yet = true;
  }

  // Set metIndexes2Id via "temp"
  (void)Tcl_UnsetVar(interp, "temp", 0);
     // ignore result; temp may not have existed
  pdvector<met_name_id> *curr_avail_mets_ptr = dataMgr->getAvailableMetInfo(false);
  pdvector<met_name_id> &curr_avail_mets = *curr_avail_mets_ptr;
  unsigned numAvailMets = curr_avail_mets.size();
  assert( numAvailMets > 0 );
  for (unsigned metlcv=0; metlcv < numAvailMets; metlcv++) {
     pdstring metricIdStr = pdstring(curr_avail_mets[metlcv].id);
     
     bool aflag;
     aflag = (Tcl_SetVar(interp, "temp", 
			 const_cast<char*>(metricIdStr.c_str()),
			 TCL_APPEND_VALUE | TCL_LIST_ELEMENT) != NULL);
     assert(aflag);
  }
  delete curr_avail_mets_ptr;
  

  pdstring tcommand("getMetsAndRes ");
  tcommand += pdstring(UIMMsgTokenID);
  tcommand += pdstring(" ") + pdstring(requestingThread);
  tcommand += pdstring(" ") + pdstring(numAvailMets);
  tcommand += pdstring(" $temp");

  int retVal = Tcl_VarEval (interp, tcommand.c_str(), 0);
  if (retVal == TCL_ERROR)  {
    uiMgr->showError (22, "");
    cerr << Tcl_GetStringResult(interp) << endl;
    thr_exit(0);  
  } 
}

void
ParadynTkGUI::newPhaseNotification(unsigned ph,
                                    const char *name,
                                    bool with_new_pc)
{
//   cout << "UI welcome to new_phase_detected" << endl;
//   cout << "id=" << ph << endl;
//   cout << "name=" << name << endl;

   // For the benefit of the shg, in the event that the shg window
   // has not yet been opened, with the result that "theShgPhases"
   // hasn't yet been constructed:
   extern shgPhases *theShgPhases;
   if (theShgPhases == NULL) {
      latest_detected_new_phase_id = ph;
      //** memory leak
      latest_detected_new_phase_name = name;
      //cout << "ui_newPhaseDetected: deferring phase id " << ph << " (" << name << ") since shg window not yet opened" << endl;
      if (with_new_pc)
         cout << "can't begin searching the new phase since Perf Consultant window not yet opened" << endl;
   }
   else {
      //cout << "ui_newPhaseDetected: adding the phase now" << endl;
      bool redraw = theShgPhases->defineNewSearch(ph, name);

      if (with_new_pc) {
         // the user has requested that we begin searching immediately on this
         // new phase, as if we had clicked on the "Search" button.  So let's do
         // the equivalent.  But first, we must switch to the new "screen".
	 bool aflag;
	 aflag=theShgPhases->changeByPhaseId(ph);
	 assert(aflag);

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


// ****************************************************************
// Metrics and Resources 
// ****************************************************************




//
// called by an exiting thread to notify the UI that it is exiting
// this is necessary so that the UI does not try to send a metrics
// menuing response to a dead tid
//
void
ParadynTkGUI::threadExiting( void )
{

    thread_t tid = getRequestingThread();

    Tcl_HashSearch *searchPtr = new Tcl_HashSearch;
    Tcl_HashEntry *entry = Tcl_FirstHashEntry(&UIMMsgReplyTbl,searchPtr);

    // check to see if there is an outstanding metrics menuing request
    // for this thread, and if so, remove its entry from the table
    while(entry){
        UIMReplyRec *msgRec = (UIMReplyRec *)Tcl_GetHashValue(entry);
        if(msgRec->tid == tid){
	    Tcl_DeleteHashEntry(entry);
	    if(searchPtr) delete searchPtr;
	    return;
	}
	entry = Tcl_NextHashEntry(searchPtr);
    }
    if(searchPtr) delete searchPtr;
}

// ****************************************************************
//  DAG/SHG Display Service Routines
// ****************************************************************


//Adds a new program to the call graph display
void
ParadynTkGUI::callGraphProgramAddedCB(unsigned programId,
                                        resourceHandle newId, 
				                        const char *executableName, 
				                        const char *shortName,
                                        const char *longName)
{
  theCallGraphPrograms->addNewProgram(programId, newId, executableName, shortName, longName);
}

void
ParadynTkGUI::updateStatusDisplay (int dagid, pdstring *info)
{
   theShgPhases->addToStatusDisplay(dagid, *info);
   delete info;
}


bool
ParadynTkGUI::TryFirstShgWindow( void )
{
   // called in shgTcl.C
   // like whereAxis's and barChart's techniques...
   // Tk_WindowId() returns 0 until the tk window has been mapped for the first
   // time, which takes a surprisingly long time.  Therefore, this hack is needed.

   if (haveSeenFirstShgWindow)
      return true;

   Tk_Window topLevelTkWindow = Tk_MainWindow( interp );
   Tk_Window theTkWindow = Tk_NameToWindow(interp, ".shg.nontop.main.all",
                                           topLevelTkWindow);
   assert(theTkWindow);

   if (Tk_WindowId(theTkWindow) == 0)
      return false; // this happens in practice...that's why this routine is needed

   haveSeenFirstShgWindow = true;

   /* *********************************************************** */

   // Why don't we construct "theShgPhases" earlier (perhaps at startup)?
   // Why do we wait until the shg window has been opened?
   // Because the constructor requires window names as arguments.
   theShgPhases = new shgPhases(".shg.titlearea.left.menu.mbar.phase.m",
                                ".shg.nontop.main.bottsb",
                                ".shg.nontop.main.leftsb",
				".shg.nontop.labelarea.current",
				".shg.nontop.textarea.text",
				".shg.nontop.buttonarea.left.search",
				".shg.nontop.buttonarea.middle.pause",
				".shg.nontop.currphasearea.label2",
                                interp, theTkWindow);
   assert(theShgPhases);

   // Now is as good a time as any to define the global phase.
   const int GlobalPhaseId = 0; // a hardcoded constant
   (void)theShgPhases->defineNewSearch(GlobalPhaseId,
				       "Global Phase");

   // Also add the "current phase", if applicable.
   // We check "latest_detected_new_phase_id", set by ui_newPhaseDetected (UImain.C)
   if (latest_detected_new_phase_id >= 0) {
      theShgPhases->defineNewSearch(latest_detected_new_phase_id,
				    latest_detected_new_phase_name);
   }

   initiateShgRedraw(interp, true);

   return true;
}

void int2style(int styleid,
                bool &active,
                shgRootNode::evaluationState &theEvalState,
                bool& deferred )
{
   if (styleid == 0) {
      active = false;
      theEvalState = shgRootNode::es_never;
        deferred = false;
   }
   else if (styleid == 1) {
      active = false;
      theEvalState = shgRootNode::es_unknown;
        deferred = false;
   }
   else if (styleid == 2) {
      active = false;
      theEvalState = shgRootNode::es_false;
        deferred = false;
   }
   else if (styleid == 3) {
      active = true;
      theEvalState = shgRootNode::es_true;
        deferred = false;
   }
   else if (styleid == 4) {
      active = true;
      theEvalState = shgRootNode::es_unknown;
        deferred = false;
   }
   else if (styleid == 5) {
      active = true;
      theEvalState = shgRootNode::es_false;
        deferred = false;
   }
   else if (styleid == 6) {
      active = false;
      theEvalState = shgRootNode::es_true;
        deferred = false;
   }
   else if (styleid == 7) {
        active = false;
        theEvalState = shgRootNode::es_unknown;
        deferred = true;
   }
   else {
      cerr << "unrecognized style id " << styleid << endl;
      exit(5);
   }
}

shgRootNode::refinement int2refinement(int styleid) {
   if (styleid == 0)
      return shgRootNode::ref_where;
   else if (styleid == 1)
      return shgRootNode::ref_why;

   cerr << "unrecognized refinement id " << styleid << endl;
   exit(5);
   return shgRootNode::ref_why; // placate compiler
}



/*  flags: 1 = root
 *         0 = non-root
 */

void
ParadynTkGUI::DAGaddNode(int dagID, unsigned nodeID, int styleID, 
		const char *label, const char *shgname, int flags)
{
   const bool isRootNode = (flags == 1);
   bool active;
   bool deferred;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState, deferred);

   if (theShgPhases->addNode(dagID, nodeID, active, theEvalState, deferred,
			     label, shgname, isRootNode))
      // shouldn't we should only be redrawing for a root node?
      if (isRootNode)
         initiateShgRedraw(interp, true);
}

//Add node to the call graph
void
ParadynTkGUI::CGaddNode(int pid, resourceHandle parent, resourceHandle newNode,
		   const char *shortName, const char *fullName, 
		    bool recursiveFlag, bool shadowFlag){
  theCallGraphPrograms->addNode(pid, parent, newNode, shortName, fullName,
				recursiveFlag, shadowFlag);
}

//This is called when the daemon notifies the DM that it has already sent
//all of the nodes for the static call graph. We should now display it.
void
ParadynTkGUI::CGDoneAddingNodesForNow(int pid)
{
  // "rethinks" the graphical display...expensive, so don't call this
  // often!
  theCallGraphPrograms->rethinkLayout(pid);
  initiateCallGraphRedraw(interp, true);
}

void
ParadynTkGUI::DAGaddBatchOfEdges (int dagID, pdvector<uiSHGrequest> *requests,
			 unsigned numRequests)
{
  // "requests" was allocated (using new) by the producer (PCshg.C code); we
  // delete it here.
  bool redraw = false;
  assert(requests->size() == numRequests); // a sanity check just for fun

  for (unsigned i = 0; i < numRequests; i++) {
    const uiSHGrequest &curr = (*requests)[i];
    if (theShgPhases->addEdge(dagID,
			      curr.srcNodeID, // parent
			      curr.dstNodeID, // child
			      int2refinement(curr.styleID),
			      curr.label,
			      i==numRequests-1 // rethink only once, at the end
			      ))
       redraw = true;
  }

  delete requests;

  if (redraw)
     initiateShgRedraw(interp, true); // true --> double buffer
}

void 
ParadynTkGUI::DAGaddEdge (int dagID, unsigned srcID, 
		 unsigned dstID,
		 int styleID, // why vs. where refinement
		 const char *label // only used for shadow node; else NULL
		 )
{
   if (theShgPhases->addEdge(dagID, 
			     srcID, // parent
			     dstID, // child
			     int2refinement(styleID),
			     label, true)) // true --> rethink
     initiateShgRedraw(interp, true); // true --> double buffer
}

void
ParadynTkGUI::DAGconfigNode (int dagID, unsigned nodeID, int styleID)
{
   bool active;
   bool deferred;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState, deferred);
  
   if (theShgPhases->configNode(dagID, nodeID, active, theEvalState, deferred))
      initiateShgRedraw(interp, true); // true --> double buffer
}


void
ParadynTkGUI::DAGinactivateEntireSearch(int dagID) {
   if (theShgPhases->inactivateEntireSearch(dagID))
      initiateShgRedraw(interp, true); // true --> double buffer
}

void
ParadynTkGUI::requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                  shg_node_info *theInfo, bool ok)
{
  if (ok) {
    theShgPhases->nodeInformation(phaseID,nodeID,*theInfo);
    delete theInfo;
  }
}

// ****************************************************************
// MISC 
// ****************************************************************

//
// This procedure is used when paradyn create a process after 
// reading a configuration file (using option -f).
//
void
ParadynTkGUI::ProcessCmd(pdstring *args)
{
  pdstring command;
  command = pdstring("paradyn process ") + (*args);
  if (Tcl_VarEval(interp,command.c_str(),0)==TCL_ERROR) {
    pdstring msg = pdstring("Tcl interpreter failed in routine ProcessCmd: ");
    msg += pdstring(Tcl_GetStringResult(interp));
    uiMgr->showError(83, P_strdup(msg.c_str()));
  }  
  delete args;
}

// initialize list of external visis in the tcl interpreter:
//  this routine sets global tcl variables vnames, vnums, vcount
//
int compare_visi_names (const void *viptr1, const void *viptr2) {
  const VM_visiInfo *p1 = (const VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (const VM_visiInfo *)viptr2;
  return strcmp (p1->name.c_str(), p2->name.c_str());
}

void 
ParadynTkGUI::registerValidVisis (pdvector<VM_visiInfo> *via) {
  int i;
  int count;
  Tcl_DString namelist;
  Tcl_DString numlist;
  char num[8];

  count = via->size();  
  via->sort (compare_visi_names);
  Tcl_DStringInit(&namelist);
  Tcl_DStringInit(&numlist);
  
  for (i = 0; i < count; i++) {
    Tcl_DStringAppendElement(&namelist, (*via)[i].name.c_str());
    sprintf (num, "%d", ((*via)[i]).visiTypeId);
    Tcl_DStringAppendElement(&numlist, num);
  }
  Tcl_SetVar (interp, "vnames", Tcl_DStringValue(&namelist), 0);
  Tcl_SetVar (interp, "vnums", Tcl_DStringValue(&numlist), 0);
  Tcl_DStringFree (&namelist);
  Tcl_DStringFree (&numlist);
  sprintf (num, "%d", count);
  Tcl_SetVar (interp, "vcount", num, 0);
  delete via;
}





void 
ParadynTkGUI::allDataSaved(bool succeeded)
{
  if (succeeded) 
    ui_status->message("Requested Data Saved");
  else
    ui_status->message("Data Save Request Failed");
}

void 
ParadynTkGUI::resourcesSaved(bool succeeded)
{
  if (succeeded)
    ui_status->message("Resource Hierarchies Saved");
  else
    ui_status->message("Resource Hierarchy Save Request Failed");
}

void 
ParadynTkGUI::shgSaved (bool succeeded)
{
  if (succeeded)
    ui_status->message("Search History Graph Saved");
  else
    ui_status->message("Search History Graph Save Request Failed");
}

void
ParadynTkGUI::CloseTkConnection( void )
{
#if !defined(i386_unknown_nt4_0)
	Display* disp = Tk_Display( Tk_MainWindow(interp) );
	int fd = XConnectionNumber( disp );
	close( fd );
#else
	// we don't need to worry about closing the Tk connection
	// under Windows, because the process created by CreateProcess()
	// doesn't interfere with our use of Tk
#endif // defined(i386_unknown_nt4_0)
}


//
// Tunable constant callbacks
//
void
ParadynTkGUI::showWhereAxisTips(bool newValue)
{
	// TODO do we do this here, or do we pass it along yet again to another object
	if(newValue)
	{
		myTclEval(interp, "whereAxisDrawTips");
	}
	else
	{
		myTclEval(interp, "whereAxisEraseTips");
	}
}


void
ParadynTkGUI::showSHGKey(bool newValue)
{
	if(newValue)
	{
		myTclEval(interp, "redrawKeyAndTipsAreas on nc");
	}
	else
	{
		myTclEval(interp, "redrawKeyAndTipsAreas off nc");
	}
}


void
ParadynTkGUI::showSHGTips(bool newValue)
{
	if(newValue)
	{
		myTclEval(interp, "redrawKeyAndTipsAreas nc on");
	}
	else
	{
		myTclEval(interp, "redrawKeyAndTipsAreas nc off");
	}
}


// utility function for showing/hiding SHG nodes, used by
// the following few SHG-related igen calls
void
ParadynTkGUI::tcShgShowGeneric(shg::changeType ct, bool show)
{
	extern shgPhases *theShgPhases;
   if (theShgPhases == NULL)
      // the shg window hasn't been opened yet...
      // do nothing.  When "theShgPhases" is first created, it'll
      // grab fresh values from the tunables
      return;

   assert(theShgPhases);
   bool anyChanges = theShgPhases->changeHiddenNodes(ct, show);
   if (anyChanges)
      initiateShgRedraw(interp, true); // true --> double buffer

   // ...and here is where we update the tk labels; hidden
   // node types are now drawn with a shaded background color.
}


void
ParadynTkGUI::showSHGTrueNodes(bool show)
{
	tcShgShowGeneric(shg::ct_true, show);
}



void
ParadynTkGUI::showSHGFalseNodes(bool show)
{
	tcShgShowGeneric(shg::ct_false, show);
}


void
ParadynTkGUI::showSHGUnknownNodes(bool show)
{
	tcShgShowGeneric(shg::ct_unknown, show);
}


void
ParadynTkGUI::showSHGNeverExpandedNodes(bool show)
{
	tcShgShowGeneric(shg::ct_never, show);
}



void
ParadynTkGUI::showSHGActiveNodes(bool show)
{
	tcShgShowGeneric(shg::ct_active, show);
}



void
ParadynTkGUI::showSHGInactiveNodes(bool show)
{
	tcShgShowGeneric(shg::ct_inactive, show);
}



void
ParadynTkGUI::showSHGShadowNodes(bool show)
{
	tcShgShowGeneric(shg::ct_shadow, show);
}


void
ParadynTkGUI::DMready( void )
{
    ParadynUI::DMready();

    // The TermWin has an annoying habit of being mapped 
    // on top of the main Paradyn window.  Give it a chance
    // to be mapped and then raise the main window to the
    // foreground in case it has been covered.
#if defined(i386_unknown_nt4_0)
    Sleep( 1000 );
#else
    usleep( 1000 );
#endif // defined(i386_unknown_nt4_0)
    myTclEval( interp, "raise ." );
}

