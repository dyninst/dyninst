// whereAxisTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the where axis.

/* $Log: whereAxisTcl.C,v $
/* Revision 1.8  1996/04/01 22:34:35  tamches
/* added whereAxisVisibilityCallbackCommand
/*
 * Revision 1.7  1996/01/09 23:56:19  tamches
 * added whereAxisDrawTipsCallback
 *
 * Revision 1.6  1995/10/17 22:22:44  tamches
 * abstractions is no longer a templated type.
 * Other minor changes corresponding to new where axis commits.
 *
 * Revision 1.5  1995/09/20 01:30:36  tamches
 * File size reduced by using some utilities in the new tkTools.C file
 *
 * Revision 1.4  1995/08/04  19:19:25  tamches
 * Commented out some cout statements that are for debugging only.
 *
 * Revision 1.3  1995/07/24  21:37:37  tamches
 * better existsCurrent() error checking.
 * Implemented alt-freescroll feature
 *
 * Revision 1.2  1995/07/18  03:41:27  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:12  tamches
 * First version of the new where axis
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

#include "abstractions.h"
#include "whereAxisTcl.h"

// Here is the main where axis global variable:
abstractions *theAbstractions;

extern bool haveSeenFirstGoodWhereAxisWid; // test.C
extern bool tryFirstGoodWhereAxisWid(Tcl_Interp *, Tk_Window); // test.C

#ifndef PARADYN
extern Tk_Window topLevelTkWindow; // test.C
#else
extern Tk_Window mainWindow;
#define topLevelTkWindow mainWindow
#endif

void whereAxisWhenIdleDrawRoutine(ClientData cd) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const bool doubleBuffer = (bool)cd;

#ifdef PARADYN
   const bool isXsynchOn = false;
#else
   extern bool xsynchronize;
   const bool isXsynchOn = xsynchronize;
#endif

   if (theAbstractions->existsCurrent())
      theAbstractions->getCurrent().draw(doubleBuffer, isXsynchOn);
}
tkInstallIdle whereAxisDrawWhenIdle(&whereAxisWhenIdleDrawRoutine);

void initiateWhereAxisRedraw(Tcl_Interp *, bool doubleBuffer) {
   whereAxisDrawWhenIdle.install((ClientData)doubleBuffer);
}

int whereAxisResizeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int, char **) {
   if (!tryFirstGoodWhereAxisWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   if (theAbstractions->existsCurrent()) {
      theAbstractions->getCurrent().resize(true); // true --> we are curr abstraction
      initiateWhereAxisRedraw(interp, true); // true-->use double-buffering
   }

   return TCL_OK;
}

int whereAxisExposeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 2);

   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (theAbstractions->existsCurrent() && count==0)
      initiateWhereAxisRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int whereAxisVisibilityCallbackCommand(ClientData, Tcl_Interp *interp,
				       int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 2);

   char *newVisibility = argv[1];

   if (0==strcmp(newVisibility, "VisibilityUnobscured"))
      theAbstractions->makeVisibilityUnobscured();
   else if (0==strcmp(newVisibility, "VisibilityPartiallyObscured"))
      theAbstractions->makeVisibilityPartiallyObscured();
   else if (0==strcmp(newVisibility, "VisibilityFullyObscured"))
      theAbstractions->makeVisibilityFullyObscured();
   else {
      cerr << "unrecognized visibility " << newVisibility << endl;
      return TCL_ERROR;
   }

   return TCL_OK;
}

int whereAxisSingleClickCallbackCommand(ClientData, Tcl_Interp *,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent())
      theAbstractions->getCurrent().processSingleClick(x, y);

   return TCL_OK;
}

int whereAxisDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);
   assert(argc==3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processDoubleClick(x, y);
      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true); // true--> use double buffer
   }

   return TCL_OK;
}

int whereAxisShiftDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
					     int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);
   assert(argc == 3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processShiftDoubleClick(x, y);
 
      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisCtrlDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
					    int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc==3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processCtrlDoubleClick(x, y);
 
      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisNewVertScrollPositionCommand(ClientData, Tcl_Interp *interp,
					  int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (!theAbstractions->existsCurrent())
      return TCL_OK;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
			   theAbstractions->getVertSBName(),
			   theAbstractions->getCurrent().getVertSBOffset(),  // <= 0
			   theAbstractions->getCurrent().getTotalVertPixUsed(),
			   theAbstractions->getCurrent().getVisibleVertPix(),
			   newFirst);

   if (anyChanges)
      anyChanges = theAbstractions->getCurrent().adjustVertSBOffset(newFirst);
   
   if (anyChanges)
      initiateWhereAxisRedraw(interp, true);

   return TCL_OK;
}

int whereAxisNewHorizScrollPositionCommand(ClientData, Tcl_Interp *interp,
					   int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (!theAbstractions->existsCurrent())
      return TCL_OK;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
			   theAbstractions->getHorizSBName(),
			   theAbstractions->getCurrent().getHorizSBOffset(), // <= 0
			   theAbstractions->getCurrent().getTotalHorizPixUsed(),
			   theAbstractions->getCurrent().getVisibleHorizPix(),
			   newFirst);
   if (anyChanges)
      anyChanges = theAbstractions->getCurrent().adjustHorizSBOffset(newFirst);

   if (anyChanges)
      initiateWhereAxisRedraw(interp, true);   

   return TCL_OK;
}

int whereAxisClearSelectionsCommand(ClientData, Tcl_Interp *interp,
				    int argc, char **) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 1);
   if (theAbstractions->existsCurrent()) {
      theAbstractions->getCurrent().clearSelections(); // doesn't redraw
      initiateWhereAxisRedraw(interp, true);
   }
 
   return TCL_OK;
}

int whereAxisNavigateToCommand(ClientData, Tcl_Interp *interp,
			       int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 2);
   const int level = atoi(argv[1]);

   if (theAbstractions->existsCurrent()) {   
      theAbstractions->getCurrent().navigateTo(level);

      initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisChangeAbstractionCommand(ClientData, Tcl_Interp *interp,
				      int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc==2);
   const int absId = atoi(argv[1]); // base-1, not 0

   if (theAbstractions->existsCurrent()) {
      (void)theAbstractions->change(absId - 1);
   
      // We must assume that the toplevel window has been resized since
      // the newly-displayed-whereAxis was set aside.  In short, we need to
      // simulate a resize right now.
      theAbstractions->getCurrent().resize(true);

      initiateWhereAxisRedraw(interp, true);   
   }

   return TCL_OK;
}

int whereAxisFindCommand(ClientData, Tcl_Interp *interp,
			 int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 2);
   const char *str = argv[1];

   if (theAbstractions->existsCurrent()) {
      const int result = theAbstractions->getCurrent().find(str);
         // 0 --> not found
         // 1 --> found, and nothing had to be expanded (i.e. just a pure scroll)
         // 2 --> found, and stuff had to be expanded (i.e. must redraw everything)
   
      if (result==1 || result==2)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int altAnchorX, altAnchorY;
bool currentlyInstalledAltMoveHandler = false;
bool ignoreNextAltMove = false;

int whereAxisAltPressCommand(ClientData, Tcl_Interp *interp,
			     int argc, char **argv) {
   if (!haveSeenFirstGoodWhereAxisWid)
      return TCL_OK;
   if (!theAbstractions->existsCurrent())
      return TCL_OK;

   assert(argc==3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);


   if (currentlyInstalledAltMoveHandler) {
      if (ignoreNextAltMove) {
         ignoreNextAltMove = false;
         return TCL_OK;
      }

      int deltax = x - altAnchorX;
      int deltay = y - altAnchorY;
//      cout << "Scroll (" << deltax << "," << deltay << ")" << endl;

      // add some extra speedup juice as an incentive to use alt-mousemove scrolling
      deltax *= 4;
      deltay *= 4;

      theAbstractions->getCurrent().adjustHorizSBOffsetFromDeltaPix(deltax);
      theAbstractions->getCurrent().adjustVertSBOffsetFromDeltaPix(deltay);

      initiateWhereAxisRedraw(interp, true);

      Tk_Window theTkWindow = theAbstractions->getTkWindow();

      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   altAnchorX, altAnchorY);

      ignoreNextAltMove = true;

      return TCL_OK;
   }
   else {
//      cout << "I detect mouse-motion w/alt pressed at (" << x << ", " << y << ")" << "; installing handler" << endl;

      altAnchorX = x;
      altAnchorY = y;

      currentlyInstalledAltMoveHandler = true;
   }

   return TCL_OK;
}

int whereAxisAltReleaseCommand(ClientData, Tcl_Interp *,
			       int argc, char **) {
//   cout << "welcome to whereAxisAltReleaseCommand" << endl;

   if (!haveSeenFirstGoodWhereAxisWid)
      return TCL_OK;
   if (!theAbstractions->existsCurrent())
      return TCL_OK;

   assert(argc==1);
   
   // Now un-install the mouse-move event handler that may have been
   // installed by the above routine.

   if (currentlyInstalledAltMoveHandler) {
//      cout << "releasing alt-move event handler now." << endl;
      currentlyInstalledAltMoveHandler = false;
   }
   else {
      // cout << "no need to release alt-move event handler (not installed?)" << endl;
   }

   return TCL_OK;
}

/* ******************************************************************** */

#ifdef PARADYN
void whereAxisDrawTipsCallback(bool newValue) {
   extern Tcl_Interp *interp;
   if (newValue)
      myTclEval(interp, "whereAxisDrawTips");
   else
      myTclEval(interp, "whereAxisEraseTips");
}
#endif

/* ******************************************************************** */

void deleteDummyProc(ClientData) {}
void installWhereAxisCommands(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "whereAxisConfigureHook", whereAxisResizeCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisExposeHook", whereAxisExposeCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisVisibilityHook",
		     whereAxisVisibilityCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisSingleClickHook",
		     whereAxisSingleClickCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisDoubleClickHook",
		     whereAxisDoubleClickCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisShiftDoubleClickHook",
		     whereAxisShiftDoubleClickCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisCtrlDoubleClickHook",
		     whereAxisCtrlDoubleClickCallbackCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisNewVertScrollPosition",
		     whereAxisNewVertScrollPositionCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisNewHorizScrollPosition",
		     whereAxisNewHorizScrollPositionCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisClearSelections",
		     whereAxisClearSelectionsCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisNavigateTo", whereAxisNavigateToCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisChangeAbstraction",
		     whereAxisChangeAbstractionCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisFindHook", whereAxisFindCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisAltPressHook", whereAxisAltPressCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisAltReleaseHook", whereAxisAltReleaseCommand,
		     NULL, deleteDummyProc);
}

void unInstallWhereAxisCommands(Tcl_Interp *interp) {
   Tcl_DeleteCommand(interp, "whereAxisAltReleaseHook");
   Tcl_DeleteCommand(interp, "whereAxisAltPressHook");
   Tcl_DeleteCommand(interp, "whereAxisFindHook");
   Tcl_DeleteCommand(interp, "whereAxisChangeAbstraction");
   Tcl_DeleteCommand(interp, "whereAxisNavigateTo");
   Tcl_DeleteCommand(interp, "whereAxisClearSelections");
   Tcl_DeleteCommand(interp, "whereAxisNewHorizScrollPosition");
   Tcl_DeleteCommand(interp, "whereAxisNewVertScrollPosition");
   Tcl_DeleteCommand(interp, "whereAxisCtrlDoubleClickHook");
   Tcl_DeleteCommand(interp, "whereAxisShiftDoubleClickHook");
   Tcl_DeleteCommand(interp, "whereAxisDoubleClickHook");
   Tcl_DeleteCommand(interp, "whereAxisSingleClickHook");
   Tcl_DeleteCommand(interp, "whereAxisExposeHook");
   Tcl_DeleteCommand(interp, "whereAxisConfigureHook");
}
