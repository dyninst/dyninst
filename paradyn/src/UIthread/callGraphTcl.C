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

// $Id: callGraphTcl.C,v 1.8 2004/03/23 01:12:29 eli Exp $

//CallGraphTcl.C: this file contains all of the tcl routines necessary
//to control the callGraph. These are all modified versions of the functions
//that appear in whereAxisTcl.C and shgTcl.C

#include <iostream>
#include "../pdMain/paradyn.h"
#include "callGraphTcl.h"
#include "callGraphs.h"
#include "tkTools.h"
#include "ParadynTkGUI.h"
#include "UIglobals.h"


callGraphs *theCallGraphPrograms;

void callGraphWhenIdleDrawRoutine(ClientData cd) {

    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );

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

   theCallGraphPrograms->draw(doubleBuffer, isXsynchOn);
}

tkInstallIdle callGraphDrawWhenIdle(&callGraphWhenIdleDrawRoutine);

int callGraphResizeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int, TCLCONST char **) {
  
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( !ui->TryFirstCallGraphWindow() )
    {
        return TCL_ERROR;
    }
  
  if(theCallGraphPrograms->existsCurrent()){
    theCallGraphPrograms->resize();
    initiateCallGraphRedraw(interp, true);
  }
  
   return TCL_OK;
}



int callGraphExposeCallbackCommand(ClientData, Tcl_Interp *interp,
				   int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( !ui->TryFirstCallGraphWindow() )
    {
        return TCL_ERROR;
    }

   assert(argc == 2);
   const int count = atoi(argv[1]);
   if(theCallGraphPrograms->existsCurrent() && count==0){
     initiateCallGraphRedraw(interp, true);
   }
   return TCL_OK;
}

//NEED TO IMPLEMENT????
int callGraphVisibilityCallbackCommand(ClientData, Tcl_Interp* /* interp */,
				       int argc, TCLCONST char **) {
   cerr << "In callGraphVisibilityCallbackCommand\n";

    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( !ui->TryFirstCallGraphWindow() )
    {
        return TCL_ERROR;
    }

   cerr << "Out of callGraphVisibilityCallbackCommand\n";
   assert(argc == 2);

   return TCL_OK;
}

int callGraphSingleClickCallbackCommand(ClientData, Tcl_Interp *,
					int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );
   assert(argc == 3);
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);
   
   if (theCallGraphPrograms->existsCurrent())
      theCallGraphPrograms->processSingleClick(x, y);

   return TCL_OK;
}

int callGraphDoubleClickCallbackCommand(ClientData, Tcl_Interp *interp,
					int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );
   assert(argc==3);
   
   const int x = atoi(argv[1]);
   const int y = atoi(argv[2]);

   if(theCallGraphPrograms->existsCurrent()){
     if(theCallGraphPrograms->processDoubleClick(x,y))
       initiateCallGraphRedraw(interp, true);
   }
   return TCL_OK;
}

int callGraphNewVertScrollPositionCommand(ClientData, Tcl_Interp *interp,
					  int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );
   if(theCallGraphPrograms->newVertScrollPosition(argc, argv))
     initiateCallGraphRedraw(interp, true);

   return TCL_OK;

}

int callGraphNewHorizScrollPositionCommand(ClientData, Tcl_Interp *interp,
					   int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );
   if(theCallGraphPrograms->newHorizScrollPosition(argc,argv))
     initiateCallGraphRedraw(interp, true);
   return TCL_OK;
}

int callGraphFindCommand(ClientData, Tcl_Interp *interp,
			 int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );

   assert(argc == 2);
   const char *str = argv[1];

   if (theCallGraphPrograms->existsCurrent()) {
      const int result = theCallGraphPrograms->find(str);
         // 0 --> not found
         // 1 --> found, and nothing had to be expanded (i.e. just a pure scroll)
         // 2 --> found, and stuff had to be expanded (i.e. must redraw everything)
   
      if (result==1 || result==2)
         initiateCallGraphRedraw(interp, true);
   }

   return TCL_OK;
}


int callGraphChangeProgramCommand(ClientData, Tcl_Interp *interp,
				      int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    assert( ui->HaveSeenFirstCallGraphWindow() );
   assert(argc==2);
   
   const int programId = atoi(argv[1]);
   if(theCallGraphPrograms->changeByProgramId(programId))
     initiateCallGraphRedraw(interp, true);

   return TCL_OK;
}

int callGraphAltPressCommand(ClientData, Tcl_Interp *interp,
			     int argc, TCLCONST char **argv) {
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( !ui->HaveSeenFirstCallGraphWindow() )
    {
        return TCL_ERROR;
    }
   
   assert(argc==3);
   int x = atoi(argv[1]);
   int y = atoi(argv[2]);
   if (theCallGraphPrograms->altPress(x, y))
      initiateCallGraphRedraw(interp, true);
   return TCL_OK;
}

int callGraphAltReleaseCommand(ClientData, Tcl_Interp *,
			       int, TCLCONST char **) {

    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( ui->HaveSeenFirstCallGraphWindow() )
    {
        theCallGraphPrograms->altRelease();
    }
   
   return TCL_OK;
}

int callGraphShowFullPathCommand(ClientData, Tcl_Interp *,
				 int, TCLCONST char **){
  theCallGraphPrograms->changeNameStyle(true);
  return TCL_OK;
}

int callGraphHideFullPathCommand(ClientData, Tcl_Interp *,
				 int, TCLCONST char **){
  theCallGraphPrograms->changeNameStyle(false);
  return TCL_OK;
}

int callGraphCreateCommand(ClientData, Tcl_Interp *,
		     int, TCLCONST char **){
  dataMgr->createCallGraph();
  return TCL_OK;
}

void initiateCallGraphRedraw(Tcl_Interp *, bool doubleBuffer) {
  callGraphDrawWhenIdle.install((ClientData)doubleBuffer);
}

int
callGraphDestroyCommand(ClientData, Tcl_Interp*,
						int, TCLCONST char** )
{
    ParadynTkGUI* ui = dynamic_cast<ParadynTkGUI*>( pdui );
    if( !ui->HaveSeenFirstCallGraphWindow() )
    {
		return TCL_OK;
	}

	// clean up data owned by the call graph window
	delete theCallGraphPrograms;
	theCallGraphPrograms = NULL;

	return TCL_OK;
}


void CGdeleteDummyProc(ClientData) {}
void installCallGraphCommands(Tcl_Interp *interp){
  
  Tcl_CreateCommand(interp, "callGraphCreateHook", callGraphCreateCommand, 
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphConfigureHook", 
		    callGraphResizeCallbackCommand,
		    NULL,  // clientData
		    CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphExposeHook", 
		    callGraphExposeCallbackCommand,
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphVisibilityHook",
		    callGraphVisibilityCallbackCommand,
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphSingleClickHook",
		    callGraphSingleClickCallbackCommand,
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphDoubleClickHook",
		    callGraphDoubleClickCallbackCommand,
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphNewVertScrollPosition",
		    callGraphNewVertScrollPositionCommand,
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphNewHorizScrollPosition",
		    callGraphNewHorizScrollPositionCommand, 
		    NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphChangeProgram",
		    callGraphChangeProgramCommand, NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphAltPressHook", 
		    callGraphAltPressCommand, NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphAltReleaseHook", 
		    callGraphAltReleaseCommand, NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp, "callGraphFindHook", 
		    callGraphFindCommand,  NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp,"callGraphShowFullPath", 
		    callGraphShowFullPathCommand, NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp,"callGraphHideFullPath", 
		    callGraphHideFullPathCommand, NULL, CGdeleteDummyProc);
  Tcl_CreateCommand(interp,"callGraphDestroyHook",
            callGraphDestroyCommand, NULL, CGdeleteDummyProc);
}


void unInstallCallGraphCommands(Tcl_Interp *interp) {
  Tcl_DeleteCommand(interp, "callGraphCreateHook");
  Tcl_DeleteCommand(interp, "callGraphAltReleaseHook");
  Tcl_DeleteCommand(interp, "callGraphAltPressHook");
  Tcl_DeleteCommand(interp, "callGraphChangeAbstraction");
  Tcl_DeleteCommand(interp, "callGraphNewHorizScrollPosition");
  Tcl_DeleteCommand(interp, "callGraphNewVertScrollPosition");
  Tcl_DeleteCommand(interp, "callGraphDoubleClickHook");
  Tcl_DeleteCommand(interp, "callGraphSingleClickHook");
  Tcl_DeleteCommand(interp, "callGraphVisibilityHook");
  Tcl_DeleteCommand(interp, "callGraphExposeHook");
  Tcl_DeleteCommand(interp, "callGraphConfigureHook");
  Tcl_DeleteCommand(interp, "callGraphFindHook");
  Tcl_DeleteCommand(interp, "callGraphShowFullPath");
  Tcl_DeleteCommand(interp, "callGraphHideFullPath");
  Tcl_DeleteCommand(interp, "callGraphDestroyCommand");
}
