// shgTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the search history graph.

/* $Log: shgTcl.C,v $
/* Revision 1.7  1996/02/02 18:54:13  tamches
/* added shgDrawKeyCallback, shgDrawTipsCallback,
/* shgMiddleClickCallbackCommand is new.
/* shgAltReleaseCommand shrunk accordingly.
/* added shgRefineGlobalPhase (temporarily)
/* fixed code in shgSearchCommand
/*
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
//#include "../pdMain/paradyn.h"
#include "performanceConsultant.thread.CLNT.h"
extern performanceConsultantUser *perfConsult;
#endif

#include "shgPhases.h"
#include "shgTcl.h"

// Here is the main shg global variable:
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

   if (theShgPhases->existsCurrent())
      theShgPhases->getCurrent().draw(doubleBuffer, isXsynchOn);
}
tkInstallIdle shgDrawWhenIdle(&shgWhenIdleDrawRoutine);

void initiateShgRedraw(Tcl_Interp *, bool doubleBuffer) {
   shgDrawWhenIdle.install((ClientData)doubleBuffer);
}

int shgResizeCallbackCommand(ClientData, Tcl_Interp *interp, int, char **) {
   if (!tryFirstGoodShgWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   if (theShgPhases->existsCurrent()) {
      theShgPhases->getCurrent().resize(true); // true --> we are curr shg
      initiateShgRedraw(interp, true); // true-->use double-buffering
   }

   return TCL_OK;
}

int shgExposeCallbackCommand(ClientData, Tcl_Interp *interp,
			     int argc, char **argv) {
   if (!tryFirstGoodShgWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 2);

   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (theShgPhases->existsCurrent() && count==0)
      initiateShgRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int shgSingleClickCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theShgPhases->existsCurrent())
      theShgPhases->getCurrent().processSingleClick(x, y);

   return TCL_OK;
}

int shgMiddleClickCallbackCommand(ClientData, Tcl_Interp *, int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theShgPhases->existsCurrent())
      theShgPhases->getCurrent().processMiddleClick(x, y);

   return TCL_OK;
}

int shgDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
				  int argc, char **argv) {
   assert(haveSeenFirstGoodShgWid);
   assert(argc==3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theShgPhases->existsCurrent()) {
      bool needToRedrawAll=theShgPhases->getCurrent().processDoubleClick(x, y);
      if (needToRedrawAll)
         initiateShgRedraw(interp, true); // true--> use double buffer
   }

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

   if (!theShgPhases->existsCurrent())
      return TCL_OK;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
			   theShgPhases->getVertSBName(),
			   theShgPhases->getCurrent().getVertSBOffset(),  // <= 0
			   theShgPhases->getCurrent().getTotalVertPixUsed(),
			   theShgPhases->getCurrent().getVisibleVertPix(),
			   newFirst);

   if (anyChanges)
      anyChanges = theShgPhases->getCurrent().adjustVertSBOffset(newFirst);
   
   if (anyChanges)
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

   if (!theShgPhases->existsCurrent())
      return TCL_OK;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
			   theShgPhases->getHorizSBName(),
			   theShgPhases->getCurrent().getHorizSBOffset(), // <= 0
			   theShgPhases->getCurrent().getTotalHorizPixUsed(),
			   theShgPhases->getCurrent().getVisibleHorizPix(),
			   newFirst);
   if (anyChanges)
      anyChanges = theShgPhases->getCurrent().adjustHorizSBOffset(newFirst);

   if (anyChanges)
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

bool currInstalledShgAltMoveHandler = false;
bool ignoreNextShgAltMove = false;

int shgAltPressCommand(ClientData, Tcl_Interp *interp, int argc, char **argv) {
   static int shgAltAnchorX;
   static int shgAltAnchorY;

   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;

   if (!theShgPhases->existsCurrent())
      return TCL_OK;

   assert(argc==3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);

   if (currInstalledShgAltMoveHandler) {
      if (ignoreNextShgAltMove) {
         ignoreNextShgAltMove = false;
         return TCL_OK;
      }

      int deltax = x - shgAltAnchorX;
      int deltay = y - shgAltAnchorY;
//      cout << "Scroll (" << deltax << "," << deltay << ")" << endl;

      // add some extra speedup juice as an incentive to use alt-mousemove scrolling
      deltax *= 4;
      deltay *= 4;

      theShgPhases->getCurrent().adjustHorizSBOffsetFromDeltaPix(deltax);
      theShgPhases->getCurrent().adjustVertSBOffsetFromDeltaPix(deltay);

      initiateShgRedraw(interp, true);

      Tk_Window theTkWindow = theShgPhases->getTkWindow();

      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   shgAltAnchorX, shgAltAnchorY);

      ignoreNextShgAltMove = true;

      return TCL_OK;
   }
   else {
//      cout << "I detect mouse-motion w/alt pressed at (" << x << ", " << y << ")" << "; installing handler" << endl;

      shgAltAnchorX = x;
      shgAltAnchorY = y;

      currInstalledShgAltMoveHandler = true;
   }

   return TCL_OK;
}

int shgAltReleaseCommand(ClientData, Tcl_Interp *, int, char **) {
   // Un-install the mouse-move event handler that may have been
   // installed by the above routine.

   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;
   if (!theShgPhases->existsCurrent())
      return TCL_OK;

   if (currInstalledShgAltMoveHandler)
      currInstalledShgAltMoveHandler = false;

   return TCL_OK;
}

int shgChangePhaseCommand(ClientData, Tcl_Interp *interp, int argc, char **argv) {
   if (!haveSeenFirstGoodShgWid)
      return TCL_OK;
   if (!theShgPhases->existsCurrent())
      return TCL_OK;

   assert(argc == 2);

   const int phaseId = atoi(argv[1]);
   if (!theShgPhases->changeByPhaseId(phaseId))
      // nothing changed
      return TCL_OK;

   initiateShgRedraw(interp, true);

   return TCL_OK;
}

int shgDefineGlobalPhaseCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // a temporary routine...I think

   theShgPhases->defineNewSearch(0, // the global phase has phase id 0
				 "Global Phase");

#ifdef PARADYN
   perfConsult->newSearch(GlobalPhase);
#endif   

   strcpy(interp->result, "true");
   return TCL_OK;
}

int shgSearchCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // we basically want to call "paradyn search true <curr-phase-name> -1",
   // as in "paradyn search true global -1"
   
   // But, currently, the phase name we are given doesn't correspond.
   // For example, we store "Global Search" instead of "global", which is
   // what the paradyn command expects.  So, we go directly to the igen calls...

   if (!theShgPhases->existsCurrent()) {
      cerr << "shgSearchCommand: no search has been defined; ignoring..." << endl;
      strcpy(interp->result, "false");
      return TCL_OK;
   }

#ifdef PARADYN
   // the shg test program does not "really" do a search
   int curr_phase_id = theShgPhases->getCurrentId();

   cout << "shgSearchCommand: activating search for phase id " << curr_phase_id << endl;
   theShgPhases->activateSearch(curr_phase_id);

   strcpy(interp->result, "true");
#else
   strcpy(interp->result, "true");
#endif

   return TCL_OK;
}

int shgPauseCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // we basically want to call "paradyn search pause <curr-phase-name>",
   // as in "paradyn search pause global"
   
   // But, currently, the phase name we are given doesn't correspond.
   // For example, we store "Global Search" instead of "global", which is
   // what the paradyn command expects.  So, we go directly to the igen calls...

   if (!theShgPhases->existsCurrent()) {
      cerr << "shgPauseCommand: no search has been defined; ignoring..." << endl;
      sprintf(interp->result, "false");
      return TCL_OK;
   }

#ifdef PARADYN
   // the shg test program does not "really" do a search
   int curr_phase_id = theShgPhases->getCurrentId();
   cout << "shgPauseCommand: about to pause search for phase id " << curr_phase_id << endl;
   setResultBool(interp, theShgPhases->pauseSearch(curr_phase_id));
#else
   strcpy(interp->result, "true");
#endif

   return TCL_OK;
}

int shgResumeCommand(ClientData, Tcl_Interp *interp, int, char **) {
   // we basically want to call "paradyn search true <curr-phase-name> -1",
   // as in "paradyn search true global -1"

   // But, currently, the phase name we are given doesn't correspond.
   // For example, we store "Global Search" instead of "global", which is
   // what the paradyn command expects.  So, we go directly to the igen calls...

   if (!theShgPhases->existsCurrent()) {
      cerr << "shgResumeCommand: no search has been defined; ignoring..." << endl;
      sprintf(interp->result, "false");
      return TCL_OK;
   }

#ifdef PARADYN
   // the shg test program does not "really" do a search
   int curr_phase_id = theShgPhases->getCurrentId();
   setResultBool(interp, theShgPhases->resumeSearch(curr_phase_id));
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
   Tcl_CreateCommand(interp, "shgDefineGlobalPhaseCommand", shgDefineGlobalPhaseCommand,
		     NULL, shgDeleteDummyProc);
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
   Tcl_DeleteCommand(interp, "shgDefineGlobalPhaseCommand");
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
