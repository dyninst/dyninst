// shgTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the search history graph.

/* $Log: shgTcl.C,v $
/* Revision 1.8  1996/02/07 19:14:28  tamches
/* made use of new routines in shgPhases which operate on the current
/* search -- no more getCurrent() usage here.
/* Some global vars moved to shgPhases
/*
 * Revision 1.7  1996/02/02 18:54:13  tamches
 * added shgDrawKeyCallback, shgDrawTipsCallback,
 * shgMiddleClickCallbackCommand is new.
 * shgAltReleaseCommand shrunk accordingly.
 * added shgRefineGlobalPhase (temporarily)
 * fixed code in shgSearchCommand
 *
 * Revision 1.6  1996/02/02 02:03:12  karavan
 * oops!  corrected call to performanceconsultant::newSearch
 *
 * Revision 1.5  1996/02/02 01:01:34  karavan
 * Changes to support the new PC/UI interface
 *
 * Revision 1.4  1996/01/23 07:10:16  tamches
 * fixed a UI bug noticed by Marcelo that could lead to an assertion
 * failure when scrolling large amounts.
 *
 * Revision 1.3  1996/01/09 01:07:43  tamches
 * added shgDevelModeChangeCallback
 *
 * Revision 1.2  1995/11/06 19:28:15  tamches
 * removed some warnings
 *
 * Revision 1.1  1995/10/17 22:09:07  tamches
 * initial version, for the new search history graph.
 *
 */

#include "tcl.h"
#include "tk.h"
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
shgPhases *theShgPhases;

extern bool haveSeenFirstGoodShgWid; // main.C
extern bool tryFirstGoodShgWid(Tcl_Interp *, Tk_Window); // main.C

#ifndef PARADYN
extern Tk_Window topLevelTkWindow; // main.C
#else
extern Tk_Window mainWindow;
#define topLevelTkWindow mainWindow
#endif

void shgWhenIdleDrawRoutine(ClientData cd) {
   assert(haveSeenFirstGoodShgWid);

   const bool doubleBuffer = (bool)cd;

#ifdef PARADYN
   const bool isXsynchOn = false;
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

int shgResizeCallbackCommand(ClientData, Tcl_Interp *interp, int, char **) {
   if (!tryFirstGoodShgWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   if (theShgPhases->resize())
      initiateShgRedraw(interp, true); // true-->use double-buffering

   return TCL_OK;
}

int shgExposeCallbackCommand(ClientData, Tcl_Interp *interp,
			     int argc, char **argv) {
   if (!tryFirstGoodShgWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 2);
   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (count==0)
      initiateShgRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int shgSingleClickCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   theShgPhases->processSingleClick(x, y);

   return TCL_OK;
}

int shgMiddleClickCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   theShgPhases->processMiddleClick(x, y);

   return TCL_OK;
}

int shgDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
				  int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);
   assert(argc==3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theShgPhases->processDoubleClick(x, y))
      initiateShgRedraw(interp, true); // true--> use double buffer

   return TCL_OK;
}

//int shgShiftDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
//				       int argc, char **argv) {
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
//				      int argc, char **argv) {
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
				    int argc, char **argv) {
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
				     int argc, char **argv) {
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
//			      int argc, char **) {
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

//int shgNavigateToCommand(ClientData, Tcl_Interp *interp, int argc, char **argv) {
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
//			 int argc, char **argv) {
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


int shgAltPressCommand(ClientData, Tcl_Interp *interp, int argc, char **argv) {
   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   assert(argc==3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);

   if (theShgPhases->altPress(x, y))
      initiateShgRedraw(interp, true);

   return TCL_OK;
}

int shgAltReleaseCommand(ClientData, Tcl_Interp *, int, char **) {
   // Un-install the mouse-move event handler that may have been
   // installed by the above routine.

   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   theShgPhases->altRelease();

   return TCL_OK;
}

int shgChangePhaseCommand(ClientData, Tcl_Interp *interp, int argc, char **argv) {
   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   assert(argc == 2);
   const int phaseId = atoi(argv[1]);

   if (theShgPhases->changeByPhaseId(phaseId))
      initiateShgRedraw(interp, true);

   return TCL_OK;
}

//int shgDefineGlobalPhaseCommand(ClientData, Tcl_Interp *interp, int, char **) {
//   // a temporary routine...I think
//
//   theShgPhases->defineNewSearch(0, // the global phase has phase id 0
//				 "Global Phase");
//
//#ifdef PARADYN
//   perfConsult->newSearch(GlobalPhase);
//#endif   
//
//   strcpy(interp->result, "true");
//   return TCL_OK;
//}

int shgSearchCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully started.

#ifdef PARADYN
   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->activateCurrSearch());
#else
   strcpy(interp->result, "true");
#endif

   return TCL_OK;
}

int shgPauseCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully paused.

#ifdef PARADYN
   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->pauseCurrSearch());
#else
   strcpy(interp->result, "true");
#endif

   return TCL_OK;
}

int shgResumeCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // sets tcl result string to true/false indicating whether the search
   // was successfully resumed.

#ifdef PARADYN
   // the shg test program does not "really" do a search
   setResultBool(interp, theShgPhases->resumeCurrSearch());
#else
   strcpy(interp->result, "true");
#endif

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
//   Tcl_CreateCommand(interp, "shgDefineGlobalPhaseCommand", shgDefineGlobalPhaseCommand,
//		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgSearchCommand", shgSearchCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgPauseCommand", shgPauseCommand,
		     NULL, shgDeleteDummyProc);
   Tcl_CreateCommand(interp, "shgResumeCommand", shgResumeCommand,
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
//   Tcl_DeleteCommand(interp, "shgDefineGlobalPhaseCommand");
   Tcl_DeleteCommand(interp, "shgSearchCommand");
   Tcl_DeleteCommand(interp, "shgPauseCommand");
   Tcl_DeleteCommand(interp, "shgResumeCommand");
}

void shgDevelModeChange(Tcl_Interp *interp, bool inDevelMode) {
   // the developer-mode tunable constant has changed.
   // If the PC shg window is up, then we need to change the height of
   // the status line window, ".shg.nontop.labelarea.current"

   unsigned height = inDevelMode ? 4 : 1;
   string commandStr = string("shgChangeCurrLabelHeight ") + string(height);
   myTclEval(interp, commandStr);
}
