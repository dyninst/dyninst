/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

// shgTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the search history graph.

/* $Id: shgTcl.C,v 1.18 2003/06/20 02:12:19 pcroth Exp $ */

#include "common/h/headers.h"
#include "tkTools.h"

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "DMinclude.h" // for resourceHandle
#else
#include "paradyn/src/DMthread/DMinclude.h" // for resourceHandle
#endif

#ifdef PARADYN
#include "performanceConsultant.thread.CLNT.h"
extern performanceConsultantUser *perfConsult;
#endif

#include "shgPhases.h"
#include "shgTcl.h"

// Here is the main shg global variable.  Why is it a pointer?  Because
// we don't want to construct it until the shg window is created.
// Why?  Because the constructor assumes the shg window and certain
// subwindows exist.
shgPhases *theShgPhases = NULL;

extern bool haveSeenFirstGoodShgWid; // main.C
extern bool tryFirstGoodShgWid(Tcl_Interp *, Tk_Window); // main.C

void shgWhenIdleDrawRoutine(ClientData cd) {
   assert(haveSeenFirstGoodShgWid);

   const bool doubleBuffer = (bool)cd;

#ifdef PARADYN
#ifdef XSYNCH
   const bool isXsynchOn = true;
#else
   const bool isXsynchOn = false;
#endif
#else
   extern bool xsynchronize;
   const bool isXsynchOn = xsynchronize; // main.C
#endif

   theShgPhases->draw(doubleBuffer, isXsynchOn);
}
tkInstallIdle shgDrawWhenIdle(&shgWhenIdleDrawRoutine);

void initiateShgRedraw(Tcl_Interp *, bool doubleBuffer) {
   shgDrawWhenIdle.install((ClientData)doubleBuffer);
}

int shgResizeCallbackCommand(ClientData, Tcl_Interp *interp,
                            int, TCLCONST char **) {
   if (!tryFirstGoodShgWid(interp, Tk_MainWindow(interp)))
      return TCL_ERROR;

   if (theShgPhases->resize())
      initiateShgRedraw(interp, true); // true-->use double-buffering

   return TCL_OK;
}

int shgExposeCallbackCommand(ClientData, Tcl_Interp *interp,
			     int argc, TCLCONST char **argv) {
   if (!tryFirstGoodShgWid(interp, Tk_MainWindow(interp)))
      return TCL_ERROR;

   assert(argc == 2);
   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (count==0)
      initiateShgRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int shgSingleClickCallbackCommand(ClientData, Tcl_Interp *,
                            int argc, TCLCONST char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   theShgPhases->processSingleClick(x, y);

   return TCL_OK;
}

int shgMiddleClickCallbackCommand(ClientData, Tcl_Interp *,
                            int argc, TCLCONST char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   theShgPhases->processMiddleClick(x, y);

   return TCL_OK;
}

int shgDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
				  int argc, TCLCONST char **argv) {
   assert(haveSeenFirstGoodShgWid);
   assert(argc==3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theShgPhases->processDoubleClick(x, y))
      initiateShgRedraw(interp, true); // true--> use double buffer

   return TCL_OK;
}

//int shgShiftDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
//				       int argc, TCLCONST char **argv) {
//   assert(haveSeenFirstGoodShgWid);
//   assert(argc == 3);
//
//   const int x = atoi(argv[1]);
//   const int y = atoi(argv[2]);
//
//   if (theShg != NULL) {
//      bool needToRedrawAll=theShg->processShiftDoubleClick(x, y);
//      if (needToRedrawAll)
//         initiateShgRedraw(interp, true); // true --> double buffer
//   }
//
//   return TCL_OK;
//}
//
//int shgCtrlDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
//				      int argc, TCLCONST char **argv) {
//   assert(haveSeenFirstGoodShgWid);
//
//   assert(argc==3);
//   const int x = atoi(argv[1]);
//   const int y = atoi(argv[2]);
//
//   if (theShg != NULL) {
//      bool needToRedrawAll=theShg->processCtrlDoubleClick(x, y);
//      if (needToRedrawAll)
//         initiateShgRedraw(interp, true);
//   }
//
//   return TCL_OK;
//}

int shgNewVertScrollPositionCommand(ClientData, Tcl_Interp *interp,
				    int argc, TCLCONST char **argv) {
   assert(haveSeenFirstGoodShgWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (theShgPhases->newVertScrollPosition(argc, argv))
      initiateShgRedraw(interp, true);

   return TCL_OK;
}

int shgNewHorizScrollPositionCommand(ClientData, Tcl_Interp *interp,
				     int argc, TCLCONST char **argv) {
   assert(haveSeenFirstGoodShgWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (theShgPhases->newHorizScrollPosition(argc, argv))
      initiateShgRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

//int shgClearSelectionsCommand(ClientData, Tcl_Interp *interp,
//			      int argc, TCLCONST char **) {
//   assert(haveSeenFirstGoodShgWid);
//
//   assert(argc == 1);
//   if (theShg != NULL) {
//      theShg->clearSelections(); // doesn't redraw
//      initiateShgRedraw(interp, true); // true --> double buffer
//   }
// 
//   return TCL_OK;
//}

//int shgNavigateToCommand(ClientData, Tcl_Interp *interp,
//                        int argc, TCLCONST char **argv) {
//   assert(haveSeenFirstGoodShgWid);
//
//   assert(argc == 2);
//   const int level = atoi(argv[1]);
//
//   if (theShg != NULL) {
//      theShg->navigateTo(level);
//      initiateShgRedraw(interp, true); // true --> redraw now
//   }
//
//   return TCL_OK;
//}

//int whereAxisFindCommand(ClientData, Tcl_Interp *interp,
//			 int argc, TCLCONST char **argv) {
//   assert(haveSeenFirstGoodWhereAxisWid);
//
//   assert(argc == 2);
//   const char *str = argv[1];
//
//   if (theAbstractions->existsCurrent()) {
//      const int result = theAbstractions->getCurrent().find(str);
//         // 0 --> not found
//         // 1 --> found, and nothing had to be expanded (i.e. just a pure scroll)
//         // 2 --> found, and stuff had to be expanded (i.e. must redraw everything)
//   
//      if (result==1 || result==2)
//         initiateWhereAxisRedraw(interp, true);
//   }
//
//   return TCL_OK;
//}


int shgAltPressCommand(ClientData, Tcl_Interp *interp,
                        int argc, TCLCONST char **argv) {
   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   assert(argc==3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);

   if (theShgPhases->altPress(x, y))
      initiateShgRedraw(interp, true);

   return TCL_OK;
}

int shgAltReleaseCommand(ClientData, Tcl_Interp *, int, TCLCONST char **) {
   // Un-install the mouse-move event handler that may have been
   // installed by the above routine.

   if (haveSeenFirstGoodShgWid)
      theShgPhases->altRelease();

   return TCL_OK;
}

int shgChangePhaseCommand(ClientData, Tcl_Interp *interp,
                        int argc, TCLCONST char **argv) {
   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   assert(argc == 2);
   const int phaseId = atoi(argv[1]);

   if (theShgPhases->changeByPhaseId(phaseId))
      initiateShgRedraw(interp, true);

   return TCL_OK;
}

int shgSearchCommand(ClientData, Tcl_Interp *interp, int, TCLCONST char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully started.

#ifdef PARADYN
	assert( theShgPhases != NULL );

   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->activateCurrSearch());
#else
   Tcl_SetObjResult(interp, Tcl_NewStringObj("true", -1));
#endif

   return TCL_OK;
}

int shgPauseCommand(ClientData, Tcl_Interp *interp, int, TCLCONST char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully paused.

#ifdef PARADYN
   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->pauseCurrSearch());
#else
   Tcl_SetObjResult(interp, Tcl_NewStringObj("true", -1));
#endif

   return TCL_OK;
}

int shgResumeCommand(ClientData, Tcl_Interp *interp, int, TCLCONST char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully resumed.

#ifdef PARADYN
   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->resumeCurrSearch());
#else
   Tcl_SetObjResult(interp, Tcl_NewStringObj("true", -1));
#endif

   return TCL_OK;
}

int shgDestroyCommand(ClientData,
                      Tcl_Interp*,
                      int, TCLCONST char** )
{
    // release resources that should be released by the
    // time we destroy the GUI
    delete theShgPhases;
    theShgPhases = NULL;

    return TCL_OK;
}


/* ******************************************************************** */

#ifdef PARADYN
void shgDrawKeyCallback(bool newValue) {
   extern Tcl_Interp *interp;
   if (newValue)
      myTclEval(interp, "redrawKeyAndTipsAreas on nc");
   else
      myTclEval(interp, "redrawKeyAndTipsAreas off nc");
}
void shgDrawTipsCallback(bool newValue) {
   extern Tcl_Interp *interp;
   if (newValue)
      myTclEval(interp, "redrawKeyAndTipsAreas nc on");
   else
      myTclEval(interp, "redrawKeyAndTipsAreas nc off");
}
#endif

/* ******************************************************************** */

void shgDeleteDummyProc(ClientData) {}
void installShgCommands(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "shgConfigureHook", shgResizeCallbackCommand,
		     NULL, // clientData
		     shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgExposeHook", shgExposeCallbackCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgSingleClickHook", shgSingleClickCallbackCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgMiddleClickHook", shgMiddleClickCallbackCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgDoubleClickHook", shgDoubleClickCallbackCommand,
		     NULL, shgDeleteDummyProc);
//   Tcl_CreateCommand(interp, "shgShiftDoubleClickHook",
//		     shgShiftDoubleClickCallbackCommand,
//		     NULL, shgDeleteDummyProc);
//   Tcl_CreateCommand(interp, "shgCtrlDoubleClickHook",
//		     shgCtrlDoubleClickCallbackCommand,
//		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgNewVertScrollPosition",
		     shgNewVertScrollPositionCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgNewHorizScrollPosition",
		     shgNewHorizScrollPositionCommand,
		     NULL, shgDeleteDummyProc);
//   Tcl_CreateCommand(interp, "shgClearSelections",
//		     shgClearSelectionsCommand,
//		     NULL, shgDeleteDummyProc);
//   Tcl_CreateCommand(interp, "shgNavigateTo", shgNavigateToCommand,
//		     NULL, shgDeleteDummyProc);
//   Tcl_CreateCommand(interp, "shgFindHook", shgFindCommand,
//		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgAltPressHook", shgAltPressCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgAltReleaseHook", shgAltReleaseCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgChangePhase", shgChangePhaseCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgSearchCommand", shgSearchCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgPauseCommand", shgPauseCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgResumeCommand", shgResumeCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgDestroyHook", shgDestroyCommand,
		     NULL, shgDeleteDummyProc);
}

void unInstallShgCommands(Tcl_Interp *interp) {
   Tcl_DeleteCommand(interp, "shgChangePhase");
   Tcl_DeleteCommand(interp, "shgAltReleaseHook");
   Tcl_DeleteCommand(interp, "shgAltPressHook");
   Tcl_DeleteCommand(interp, "shgFindHook");
   Tcl_DeleteCommand(interp, "shgChangeAbstraction");
   Tcl_DeleteCommand(interp, "shgNavigateTo");
   Tcl_DeleteCommand(interp, "shgClearSelections");
   Tcl_DeleteCommand(interp, "shgNewHorizScrollPosition");
   Tcl_DeleteCommand(interp, "shgNewVertScrollPosition");
//   Tcl_DeleteCommand(interp, "shgCtrlDoubleClickHook");
//   Tcl_DeleteCommand(interp, "shgShiftDoubleClickHook");
   Tcl_DeleteCommand(interp, "shgDoubleClickHook");
   Tcl_DeleteCommand(interp, "shgSingleClickHook");
   Tcl_DeleteCommand(interp, "shgMiddleClickHook");
   Tcl_DeleteCommand(interp, "shgExposeHook");
   Tcl_DeleteCommand(interp, "shgConfigureHook");
   Tcl_DeleteCommand(interp, "shgSearchCommand");
   Tcl_DeleteCommand(interp, "shgPauseCommand");
   Tcl_DeleteCommand(interp, "shgResumeCommand");
   Tcl_DeleteCommand(interp, "shgDestroyHook");
}

void shgDevelModeChange(Tcl_Interp *interp, bool inDevelMode) {
   // the developer-mode tunable constant has changed.
   // If the PC shg window is up, then we need to change the height of
   // the status line window, ".shg.nontop.labelarea.current"

   unsigned height = inDevelMode ? 4 : 1;
   string commandStr = string("shgChangeCurrLabelHeight ") + string(height);
   myTclEval(interp, commandStr);
}
