// whereAxisTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the where axis.

/* $Log: whereAxisTcl.C,v $
/* Revision 1.3  1995/07/24 21:37:37  tamches
/* better existsCurrent() error checking.
/* Implemented alt-freescroll feature
/*
 * Revision 1.2  1995/07/18  03:41:27  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:12  tamches
 * First version of the new where axis
 *
 */

#include "tclclean.h"
#include "tkclean.h"

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "DMinclude.h" // for resourceHandle
#else
#include "paradyn/src/DMthread/DMinclude.h" // for resourceHandle
#endif

#include "abstractions.h"
#include "whereAxisTcl.h"

// Here is the main where axis global variable:
abstractions<resourceHandle> *theAbstractions;

extern bool haveSeenFirstGoodWhereAxisWid; // test.C
extern bool tryFirstGoodWhereAxisWid(Tcl_Interp *, Tk_Window); // test.C

#ifndef PARADYN
extern Tk_Window topLevelTkWindow; // test.C
#else
extern Tk_Window mainWindow;
#define topLevelTkWindow mainWindow
#endif


bool currentlyInstalledDrawWhenIdle = false;
void whereAxisWhenIdleDrawRoutine(ClientData cd) {
   assert(haveSeenFirstGoodWhereAxisWid);

   currentlyInstalledDrawWhenIdle=false;
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

void initiateWhereAxisRedraw(Tcl_Interp *interp, bool doubleBuffer) {
   if (!currentlyInstalledDrawWhenIdle) {
      Tk_DoWhenIdle(whereAxisWhenIdleDrawRoutine, (ClientData)doubleBuffer);
      currentlyInstalledDrawWhenIdle = true;
   }
}

int whereAxisResizeCallbackCommand(ClientData cd, Tcl_Interp *interp,
				   int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 3);
   const int newWidth  = atoi(argv[1]);
   const int newHeight = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      theAbstractions->getCurrent().resize(newWidth, newHeight);
      initiateWhereAxisRedraw(interp, true); // true-->use double-buffering
   }

   return TCL_OK;
}

int whereAxisExposeCallbackCommand(ClientData cd, Tcl_Interp *interp,
				   int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, topLevelTkWindow))
      return TCL_ERROR;

   assert(argc == 2);

   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (theAbstractions->existsCurrent() && count==0)
      initiateWhereAxisRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int whereAxisSingleClickCallbackCommand(ClientData cd, Tcl_Interp *interp,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent())
      theAbstractions->getCurrent().processSingleClick(x, y, true);
         // true --> redraw now

   return TCL_OK;
}

int whereAxisDoubleClickCallbackCommand(ClientData cd, Tcl_Interp *interp,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processDoubleClick(x, y, true);
         // true --> redraw now

      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true); // true--> use double buffer
   }

   return TCL_OK;
}

int whereAxisShiftDoubleClickCallbackCommand(ClientData cd, Tcl_Interp *interp,
					     int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processShiftDoubleClick(x, y);
 
      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisCtrlDoubleClickCallbackCommand(ClientData cd, Tcl_Interp *interp,
					    int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->getCurrent().processCtrlDoubleClick(x, y);
 
      if (needToRedrawAll)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisNewVertScrollPositionCommand(ClientData cd, Tcl_Interp *interp,
					  int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (theAbstractions->existsCurrent()) {
      if (0==strcmp(argv[1], "moveto")) {
         const float newVertPosition = atof(argv[2]);
         theAbstractions->getCurrent().adjustVertSBOffset(newVertPosition);
      }
      else if (0==strcmp(argv[1], "scroll")) {
         const int num = atoi(argv[2]);
         if (0==strcmp(argv[3], "unit"))
            theAbstractions->getCurrent().adjustVertSBOffsetFromDeltaPix(num);
         else if (0==strcmp(argv[3], "units"))
            theAbstractions->getCurrent().adjustVertSBOffsetFromDeltaPix(num);
         else if (0==strcmp(argv[3], "page"))
            theAbstractions->getCurrent().adjustVertSBOffsetFromDeltaPages(num);
         else if (0==strcmp(argv[3], "pages"))
            theAbstractions->getCurrent().adjustVertSBOffsetFromDeltaPages(num);
         else {
            cerr << "unrecognized: " << argv[3] << " (expected 'unit(s)' or 'page(s)')" << endl;
            assert(false);
         }
      }
      else
         assert(false);
   
      initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int whereAxisNewHorizScrollPositionCommand(ClientData cd, Tcl_Interp *interp,
					   int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   // The arguments will be one of:
   // 1) moveto [fraction]
   // 2) scroll [num-units] unit   (num-units is always either -1 or 1)
   // 3) scroll [num-pages] page   (num-pages is always either -1 or 1)

   if (theAbstractions->existsCurrent()) {
      if (0==strcmp(argv[1], "moveto")) {
         const float newFrac = atof(argv[2]);
         theAbstractions->getCurrent().adjustHorizSBOffset(newFrac);
      }
      else if (0==strcmp(argv[1], "scroll")) {
         const int num = atoi(argv[2]);
         if (0==strcmp(argv[3], "unit") || 0==strcmp(argv[3], "units")) {
            theAbstractions->getCurrent().adjustHorizSBOffsetFromDeltaPix(num);
         }
         else if (0==strcmp(argv[3], "page") || 0==strcmp(argv[3], "pages")) {
            theAbstractions->getCurrent().adjustHorizSBOffsetFromDeltaPages(num);
         }
         else {
            cerr << "unrecognized: " << argv[3] << " (expected 'unit(s)' or 'page(s)')" << endl;
            assert(false);
         }
      }
      else
         assert(false);

      initiateWhereAxisRedraw(interp, true);   
   }

   return TCL_OK;
}

int whereAxisClearSelectionsCommand(ClientData cd, Tcl_Interp *interp,
				    int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 1);
   if (theAbstractions->existsCurrent()) {
      theAbstractions->getCurrent().clearSelections(); // doesn't redraw
      initiateWhereAxisRedraw(interp, true);
   }
 
   return TCL_OK;
}

int whereAxisNavigateToCommand(ClientData cd, Tcl_Interp *interp,
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

int whereAxisChangeAbstractionCommand(ClientData cd, Tcl_Interp *interp,
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

int whereAxisFindCommand(ClientData cd, Tcl_Interp *interp,
			 int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 2);
   const char *str = argv[1];

   if (theAbstractions->existsCurrent()) {
      const int result = theAbstractions->getCurrent().find(str);
         // 0 --> not found
         // 1 --> found, and nothing had to be expanded (i.e. just a pure scroll)
         // 2 --> found, and stuff had to be expanded (i.e. must redraw everything)
   
      if (result == 0) {
         //cout << "could not find what you were looking for" << endl;
      }
      else if (result == 1) {
         //cout << "found what you were looking for...no expansion necessary" << endl;
      }
      else if (result == 2) {
         //cout << "found what you were looking for...had to expand stuff tho" << endl;
      }
      else
         assert(false);
   
      if (result==1 || result==2)
         initiateWhereAxisRedraw(interp, true);
   }

   return TCL_OK;
}

int altAnchorX, altAnchorY;
bool currentlyInstalledAltMoveHandler = false;
bool ignoreNextAltMove = false;

int whereAxisAltPressCommand(ClientData cd, Tcl_Interp *interp,
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
      cout << "I detect mouse-motion w/alt pressed at (" << x << ", " << y << ")" << "; installing handler" << endl;

      altAnchorX = x;
      altAnchorY = y;

      currentlyInstalledAltMoveHandler = true;
   }

   return TCL_OK;
}

int whereAxisAltReleaseCommand(ClientData cd, Tcl_Interp *interp,
			       int argc, char **argv) {
//   cout << "welcome to whereAxisAltReleaseCommand" << endl;

   if (!haveSeenFirstGoodWhereAxisWid)
      return TCL_OK;
   if (!theAbstractions->existsCurrent())
      return TCL_OK;

   assert(argc==1);
   
   // Now un-install the mouse-move event handler that may have been
   // installed by the above routine.

   if (currentlyInstalledAltMoveHandler) {
      cout << "releasing alt-move event handler now." << endl;
      currentlyInstalledAltMoveHandler = false;
   }
   else {
      // cout << "no need to release alt-move event handler (not installed?)" << endl;
   }

   return TCL_OK;
}

/* ******************************************************************** */

void deleteDummyProc(ClientData cd) {}
void installWhereAxisCommands(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "configureHook", whereAxisResizeCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "exposeHook", whereAxisExposeCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "singleClickHook", whereAxisSingleClickCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "doubleClickHook", whereAxisDoubleClickCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "shiftDoubleClickHook",
		     whereAxisShiftDoubleClickCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "ctrlDoubleClickHook",
		     whereAxisCtrlDoubleClickCallbackCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "newVertScrollPosition",
		     whereAxisNewVertScrollPositionCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "newHorizScrollPosition",
		     whereAxisNewHorizScrollPositionCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisClearSelections",
		     whereAxisClearSelectionsCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "navigateTo", whereAxisNavigateToCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "changeAbstraction", whereAxisChangeAbstractionCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "findHook", whereAxisFindCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "altPressHook", whereAxisAltPressCommand,
		     NULL, // clientData
		     deleteDummyProc);
   Tcl_CreateCommand(interp, "altReleaseHook", whereAxisAltReleaseCommand,
		     NULL, // clientData
		     deleteDummyProc);
}

void unInstallWhereAxisCommands(Tcl_Interp *interp) {
   Tcl_DeleteCommand(interp, "altReleaseHook");
   Tcl_DeleteCommand(interp, "altPressHook");
   Tcl_DeleteCommand(interp, "findHook");
   Tcl_DeleteCommand(interp, "changeAbstraction");
   Tcl_DeleteCommand(interp, "navigateTo");
   Tcl_DeleteCommand(interp, "whereAxisClearSelections");
   Tcl_DeleteCommand(interp, "newHorizScrollPosition");
   Tcl_DeleteCommand(interp, "newVertScrollPosition");
   Tcl_DeleteCommand(interp, "ctrlDoubleClickHook");
   Tcl_DeleteCommand(interp, "shiftDoubleClickHook");
   Tcl_DeleteCommand(interp, "doubleClickHook");
   Tcl_DeleteCommand(interp, "singleClickHook");
   Tcl_DeleteCommand(interp, "exposeHook");
   Tcl_DeleteCommand(interp, "configureHook");
}

