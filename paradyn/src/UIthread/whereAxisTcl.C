/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// whereAxisTcl.C
// Ariel Tamches

// Implementations of new commands and tk bindings related to the where axis.

/* $Id: whereAxisTcl.C,v 1.17 2001/10/31 19:35:02 willb Exp $ */

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "DMinclude.h" // for resourceHandle
#else
#include "paradyn/src/DMthread/DMinclude.h" // for resourceHandle
#endif

#include "abstractions.h"
#include "whereAxisTcl.h"
#include "tkTools.h"
#include "VM.thread.h"
#include "../pdMain/paradyn.h"
#include "../DMthread/DMmetric.h"

// Here is the main where axis global variable:
abstractions *theAbstractions;

extern bool haveSeenFirstGoodWhereAxisWid; // test.C
extern bool tryFirstGoodWhereAxisWid(Tcl_Interp *, Tk_Window); // test.C

void whereAxisWhenIdleDrawRoutine(ClientData cd) {
   assert(haveSeenFirstGoodWhereAxisWid);

   const bool doubleBuffer = (bool)cd;

#ifdef PARADYN
   const bool isXsynchOn = false;
#else
   extern bool xsynchronize;
   const bool isXsynchOn = xsynchronize;
#endif

   theAbstractions->drawCurrent(doubleBuffer, isXsynchOn);
}
tkInstallIdle whereAxisDrawWhenIdle(&whereAxisWhenIdleDrawRoutine);

void initiateWhereAxisRedraw(Tcl_Interp *, bool doubleBuffer) {
   whereAxisDrawWhenIdle.install((ClientData)doubleBuffer);
}

int whereAxisResizeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int, char **) {
   if (!tryFirstGoodWhereAxisWid(interp, Tk_MainWindow(interp)))
      return TCL_ERROR;

   if (theAbstractions->existsCurrent()) {
      theAbstractions->resizeCurrent();
      initiateWhereAxisRedraw(interp, true); // true-->use double-buffering
   }

   return TCL_OK;
}

int whereAxisExposeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, Tk_MainWindow(interp)))
      return TCL_ERROR;

   assert(argc == 2);

   const int count = atoi(argv[1]); // Xevent count field (we should only redraw if 0)

   if (theAbstractions->existsCurrent() && count==0)
      initiateWhereAxisRedraw(interp, true); // true --> double buffer

   return TCL_OK;
}

int whereAxisVisibilityCallbackCommand(ClientData, Tcl_Interp *interp,
				       int argc, char **argv) {
   if (!tryFirstGoodWhereAxisWid(interp, Tk_MainWindow(interp)))
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
      theAbstractions->processSingleClick(x, y);

   return TCL_OK;
}

int whereAxisCtrlClickCallbackCommand(ClientData, Tcl_Interp *,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   numlist select_focus;
   bool result=false;
   if (theAbstractions->existsCurrent())
      result=theAbstractions->processCtrlClick(x, y,select_focus);

   if (result == true)
   {
      vector<VM_visiInfo> *avail_Visis = vmMgr->VMAvailableVisis();
      int  table_id=-1;
      for (unsigned i=0;i < avail_Visis->size(); i++)
      {
      	if ((*avail_Visis)[i].name == "Table")
		table_id=i;
      }
      if (table_id == -1)
      {
         uiMgr->showError(119,"");
      	 return TCL_OK;
      }
      
      vector<metric_focus_pair> *matrix=new vector<metric_focus_pair>;
      *matrix += metric_focus_pair(UNUSED_METRIC_HANDLE,select_focus);

      //paradyn visi create #table GlobalPhase
      int forceProcessStart = 1;
      vmMgr->VMCreateVisi(1,forceProcessStart,table_id,GlobalPhase,matrix);
   }
   
   return TCL_OK;
}

int whereAxisDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
					int argc, char **argv) {
   assert(haveSeenFirstGoodWhereAxisWid);
   assert(argc==3);

   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if (theAbstractions->existsCurrent()) {
      bool needToRedrawAll=theAbstractions->processDoubleClick(x, y);
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
      bool needToRedrawAll=theAbstractions->processShiftDoubleClick(x, y);
 
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
      bool needToRedrawAll=theAbstractions->processCtrlDoubleClick(x, y);
 
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
			   theAbstractions->getVertSBOffset(),  // <= 0
			   theAbstractions->getTotalVertPixUsed(),
			   theAbstractions->getVisibleVertPix(),
			   newFirst);

   if (anyChanges)
      anyChanges = theAbstractions->adjustVertSBOffset(newFirst);
   
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
			   theAbstractions->getHorizSBOffset(), // <= 0
			   theAbstractions->getTotalHorizPixUsed(),
			   theAbstractions->getVisibleHorizPix(),
			   newFirst);
   if (anyChanges)
      anyChanges = theAbstractions->adjustHorizSBOffset(newFirst);

   if (anyChanges)
      initiateWhereAxisRedraw(interp, true);   

   return TCL_OK;
}

int whereAxisClearSelectionsCommand(ClientData, Tcl_Interp *interp,
				    int argc, char **) {
   assert(haveSeenFirstGoodWhereAxisWid);

   assert(argc == 1);
   if (theAbstractions->existsCurrent()) {
      theAbstractions->clearSelections(); // doesn't redraw
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
      theAbstractions->navigateTo(level);

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
      theAbstractions->resizeCurrent();

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
      const int result = theAbstractions->find(str);
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

      theAbstractions->adjustHorizSBOffsetFromDeltaPix(deltax);
      theAbstractions->adjustVertSBOffsetFromDeltaPix(deltay);

      initiateWhereAxisRedraw(interp, true);

      Tk_Window theTkWindow = theAbstractions->getTkWindow();

#if !defined(i386_unknown_nt4_0)
      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   altAnchorX, altAnchorY);
#else // !defined(i386_unknown_nt4_0)
		// TODO - implement warping behavior
#endif // !defined(i386_unknown_nt4_0)

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


int
whereAxisDestroyHandler(ClientData, Tcl_Interp*, int, char **)
{

   if (!haveSeenFirstGoodWhereAxisWid)
      return TCL_OK;

	// cleanup data owned by the whereAxis window
   delete theAbstractions;
   theAbstractions = NULL;

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
   Tcl_CreateCommand(interp, "whereAxisCtrlClickHook",
		     whereAxisCtrlClickCallbackCommand,
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

   Tcl_CreateCommand(interp, "whereAxisFindHook", whereAxisFindCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisAltPressHook", whereAxisAltPressCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisAltReleaseHook", whereAxisAltReleaseCommand,
		     NULL, deleteDummyProc);
   Tcl_CreateCommand(interp, "whereAxisDestroyHook", whereAxisDestroyHandler,
		     NULL, deleteDummyProc);
}

void unInstallWhereAxisCommands(Tcl_Interp *interp) {
   Tcl_DeleteCommand(interp, "whereAxisAltReleaseHook");
   Tcl_DeleteCommand(interp, "whereAxisAltPressHook");
   Tcl_DeleteCommand(interp, "whereAxisFindHook");
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
   Tcl_DeleteCommand(interp, "whereAxisDestroyHook");
}
