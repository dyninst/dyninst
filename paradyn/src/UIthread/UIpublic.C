/*
 * UIpublic.C : exported services of the User Interface Manager thread 
 *              of Paradyn
 *
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

/* $Log: UIpublic.C,v $
/* Revision 1.38  1996/01/30 23:04:22  tamches
/* removed include to obsolete file shgDisplay.h
/*
 * Revision 1.37  1996/01/23 06:56:42  tamches
 * uim_VisiSelections no longer a ptr
 * int2style reworked for 7 styles
 * new "label" arg for DAGaddEdge (for shadow nodes)
 *
 * Revision 1.36  1996/01/11 23:40:41  tamches
 * int2style now handles 6 styles instead of 4
 *
 * Revision 1.35  1996/01/09 00:46:52  tamches
 * added phaseID argument in call to "new shg"; removed it from call to
 * theShgPhases->add
 *
 * Revision 1.34  1995/11/28 15:50:02  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.33  1995/11/21  15:17:50  naim
 * Using string instead of char[300] in showError routine - naim
 *
 * Revision 1.32  1995/11/09  02:11:30  tamches
 * removed some obsolete references (some which had been up till now commented out),
 * such as initSHGStyles, UIMUser::chooseMenuItemREPLY, etc.
 *
 * Revision 1.31  1995/11/08 23:43:21  tamches
 * removed code for obsolete ui igen calls chooseMenuItemREPLY,
 * msgChoice, chooseMenuItem, showMsg, uimMsgReplyCmd, showMsgWait
 *
 * Revision 1.30  1995/11/08 06:25:03  tamches
 * removed some warnings by including tclclean.h and tkclean.h
 *
 * Revision 1.29  1995/11/08 05:10:26  tamches
 * removed reference to obsolete file dag.h
 *
 * Revision 1.28  1995/11/06 18:02:56  tamches
 * changed nodeIdType to unsigned (nodeIdType is no longer used)
 * Added an shg hack s.t. the root node appears as "Whole Program" instead of "1"
 *
 * Revision 1.27  1995/10/17 20:48:24  tamches
 * New search history graph:
 * Commented out StrNodeIdType (no longer needed w/ new shg).
 * Commented out references to class shgDisplay (obsoleted class).
 * Added tryFirstGoodShgWid
 * DAGaddNode, DAGaddEdge, DAGconfigNode adapted for use with new shg.
 *
 * Revision 1.26  1995/10/05 04:28:03  karavan
 * added ActiveDags to dag class.
 * removed globals formerly used for search display to move to multiple-display
 * model.
 * removed ui::DAGaddEStyle and ui::DAGaddNStyle from interface (obsolete).
 * added dagID creation to dag constructor.
 * Added shgDisplay class.
 * removed UIstatDisp class.
 * removed obsolete commented code.
 *
 * Revision 1.25  1995/09/26  20:27:05  naim
 * Minor warning fixes and some other minor error messages fixes
 *
 * Revision 1.24  1995/07/24  21:31:03  tamches
 * removed some obsolete code related to the old where axis
 *
 * Revision 1.23  1995/07/17  05:06:20  tamches
 * Changes for the new version of the where axis
 *
 * Revision 1.22  1995/06/02  20:50:37  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.21  1995/01/26  17:59:00  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.20  1994/11/08  07:50:47  karavan
 * Purified code; narrowed side margins for dag nodes.
 *
 * Revision 1.19  1994/11/02  23:44:12  karavan
 * added error service call for startup script error.
 *
 * Revision 1.18  1994/11/02  04:40:44  karavan
 * added new interface function UIM::readStartupFile which implements
 * the new -s commandline option to read in a tcl script after initialization
 * but before any other UI functions.
 *
 * Revision 1.17  1994/11/01  05:43:39  karavan
 * changed window pathname in call to dag::createDisplay() to match
 * update to createDisplay(); minor performance and warning fixes
 *
 * Revision 1.16  1994/10/25  17:57:34  karavan
 * added Resource Display Objects, which support display of multiple resource
 * abstractions.
 *
 * Revision 1.15  1994/10/09  01:24:49  karavan
 * A large number of changes related to the new UIM/visiThread metric&resource
 * selection interface and also to direct selection of resources on the
 * Where axis.
 *
 * Revision 1.14  1994/09/25  01:54:10  newhall
 * updated to support changes in VM, and UI interface
 *
 * Revision 1.13  1994/09/22  01:16:53  markc
 * Added const to char* arg in UIM::showError()
 *
 * Revision 1.12  1994/09/13  05:07:29  karavan
 * improved error handling
 *
 * Revision 1.11  1994/08/10  17:21:35  newhall
 * added parameters to chooseMetricsandResources
 *
 * Revision 1.10  1994/08/01  20:24:40  karavan
 * new version of dag; new dag support commands
 *
 * Revision 1.9  1994/07/08  04:03:18  karavan
 * changed showMsg to async function
 *
 * Revision 1.8  1994/07/07  05:58:05  karavan
 * added UI error service function
 *
 * Revision 1.7  1994/06/12  22:38:27  karavan
 * implemented status display service.
 *
 * Revision 1.6  1994/05/12  23:34:15  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.5  1994/05/07  23:26:30  karavan
 * added short explanation feature to SHG.
 *
 * Revision 1.4  1994/05/05  19:52:46  karavan
 * changed chooseMetricsandResources
 *
 * Revision 1.3  1994/04/21  05:18:17  karavan
 * Implemented DAG functions.
 *
 * Revision 1.2  1994/04/06  17:41:19  karavan
 * added working versions of getMetricsandResources, showError, showMessage
 * */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "tcl.h"
#include "tk.h"

#include "UI.thread.CLNT.h"
#include "UI.thread.SRVR.h"
#include "UI.thread.h"
#include "thread/h/thread.h"
#include "UIglobals.h"
#include "../pdMain/paradyn.h"

#include "shgPhases.h"
#include "shgTcl.h"

 /* globals for metric resource selection */
vector<metric_focus_pair> uim_VisiSelections;
char **uim_AvailMets;
int uim_AvailMetsSize;
metricHandle *uim_AvailMetHandles;

void
UIMUser::chosenMetricsandResources
          (chooseMandRCBFunc cb,
	   vector<metric_focus_pair> *pairList)
{
  (cb) (pairList);
}
	
// 
// Startup File
void 
UIM::readStartupFile(const char *script)
{
  string tcommand;
  if (script != NULL) {   // script specified on paradyn command line
    tcommand = string("source \"") + string(script);
    tcommand += string("\"");
    if (Tcl_Eval (interp, P_strdup(tcommand.string_of())) == TCL_ERROR) {
      uiMgr->showError(24, "Error reading tcl startup script");
    }
  }
}

// ****************************************************************
// Error Message Display Service 
// ****************************************************************

void
UIM::showError(int errCode, const char *errString)
{
  string tcommand;

    // custom error info string to be printed
    tcommand = string("showError ") + string(errCode);
    tcommand += string(" {");
    tcommand += string(errString);
    tcommand += string("}");
    if (Tcl_VarEval (interp,tcommand.string_of(),(char *) NULL) == TCL_ERROR) {
      printf ("newShowError: Tcl call to showError fails, %s\n", 
	      interp->result);
      thr_exit(0);
    }
}

// ****************************************************************
// Metrics and Resources 
// ****************************************************************

void 
UIM::chooseMetricsandResources(chooseMandRCBFunc cb,
			       vector<metric_focus_pair> *pairList)
{
      // store record with unique id and callback function
  UIMMsgTokenID++;
  int newptr;
  Tcl_HashEntry *entryPtr = Tcl_CreateHashEntry (&UIMMsgReplyTbl,
						 (char *)UIMMsgTokenID, 
						 &newptr);
  if (newptr) {
    UIMReplyRec *reply = new UIMReplyRec;
      /* grab thread id of requesting thread */
    reply->tid = getRequestingThread();
    reply->cb = (showMsgCBFunc) cb;
    Tcl_SetHashValue (entryPtr, reply);
  } 
  else {
    uiMgr->showError(21, "Tcl command failure - Bad pointer");
    thr_exit(0);
  }

     // initialize metric menu 
  vector<met_name_id> *amets = dataMgr->getAvailableMetInfo(false);
  uim_AvailMetsSize = amets->size();
  uim_AvailMets = new (char *) [uim_AvailMetsSize];
  for (int j = 0; j < uim_AvailMetsSize; j++) 
    uim_AvailMets[j] = strdup(((*amets)[j]).name.string_of());
  uim_AvailMetHandles = new metricHandle [uim_AvailMetsSize];
  for (int i = 0; i < uim_AvailMetsSize; i++) 
    uim_AvailMetHandles[i] = (*amets)[i].id;
  delete amets;
  char *ml = Tcl_Merge (uim_AvailMetsSize, uim_AvailMets);
  ml = Tcl_SetVar (interp, "metList", ml, 0);
  char ctr[16];
  sprintf (ctr, "%d", uim_AvailMetsSize);
  Tcl_SetVar (interp, "metCount", ctr, 0);

  // set global tcl variable to list of currently defined where axes

  string tcommand("getMetsAndRes ");
  tcommand += string(UIMMsgTokenID);
  tcommand += string(" ");
  tcommand += string(0);
  // the last parameter (an Rdo token) is obsolete...0 is just a filler

  int retVal = Tcl_VarEval (interp, P_strdup(tcommand.string_of()), 0);
  if (retVal == TCL_ERROR)  {
    uiMgr->showError (22, "Tcl command failure");
    printf ("%s\n", interp->result);
    thr_exit(0);  
  } 
}

// ****************************************************************
//  DAG/SHG Display Service Routines
// ****************************************************************

extern shgPhases *theShgPhases;

void
UIM::updateStatusDisplay (int shgToken, const char *info)
{
   if (theShgPhases->existsCurrent())
      theShgPhases->getByID(shgToken).addToStatusDisplay(info);

//   cout << "STATUS (" << shgToken << "): " << info << endl; // ari temp hack
}

bool haveSeenFirstGoodShgWid = false;

bool tryFirstGoodShgWid(Tcl_Interp *interp, Tk_Window topLevelTkWindow) {
   // called in shgTcl.C
   // like whereAxis's and barChart's techniques...
   // Tk_WindowId() returns 0 until the tk window has been mapped for the first
   // time, which takes a surprisingly long time.  Therefore, this hack is needed.

   if (haveSeenFirstGoodShgWid)
      return true;

   Tk_Window theTkWindow = Tk_NameToWindow(interp, ".shg.nontop.main.all",
                                           topLevelTkWindow);
   assert(theTkWindow);

   if (Tk_WindowId(theTkWindow) == 0)
      return false; // this happens in practice...that's why this routine is needed

   haveSeenFirstGoodShgWid = true;

   /* *********************************************************** */

   theShgPhases = new shgPhases(".shg.titlearea.left.menu.mbar.phase.m",
                                ".shg.nontop.main.bottsb",
                                ".shg.nontop.main.leftsb",
				".shg.nontop.labelarea.current",
                                interp, theTkWindow);
   assert(theShgPhases);

   initiateShgRedraw(interp, true);

   return true;
}


//
// called from Paradyn Search Command when search requested; returns SHG token
// if successful, -1 for error
//

int 
UIM::initSHG(const char *phaseName, int phaseID) 
{
   assert(theShgPhases);

   shg *theNewShg = new shg(phaseID, interp,
			    theShgPhases->getTkWindow(), // _not_ the main window!
			    theShgPhases->getHorizSBName(),
			    theShgPhases->getVertSBName(),
			    theShgPhases->getCurrItemLabelName());
   assert(theNewShg);

   //theShgPhases->add(theNewShg, phaseID, phaseName);
   theShgPhases->add(theNewShg, phaseName);

   assert(theShgPhases->existsCurrent());
   theShgPhases->getCurrent().resize(theShgPhases->getCurrentId()==phaseID);

   initiateShgRedraw(interp, true); // true --> double buffer

   return phaseID;
}


void int2style(int styleid, bool &active, shgRootNode::evaluationState &theEvalState) {
   if (styleid == 0) {
      active = false;
      theEvalState = shgRootNode::es_never;
   }
   else if (styleid == 1) {
      active = false;
      theEvalState = shgRootNode::es_unknown;
   }
   else if (styleid == 2) {
      active = false;
      theEvalState = shgRootNode::es_false;
   }
   else if (styleid == 3) {
      active = true;
      theEvalState = shgRootNode::es_true;
   }
   else if (styleid == 4) {
      active = true;
      theEvalState = shgRootNode::es_unknown;
   }
   else if (styleid == 5) {
      active = true;
      theEvalState = shgRootNode::es_false;
   }
   else if (styleid == 6) {
      active = false;
      theEvalState = shgRootNode::es_true;
   }
   else {
      cerr << "unrecognized style id " << styleid << endl;
      exit(5);
   }
}

/*  flags: 1 = root
 *         0 = non-root
 */

int 
UIM::DAGaddNode(int dagID, unsigned nodeID, int styleID, 
		char *label, char *shgname, int flags)
{
   shg &theShg = theShgPhases->getByID(dagID);

   const bool isRootNode = (flags == 1);
   bool active;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState);

   // A temporary hack for the mysterious "1" that appears for the root node:
   theShg.addNode(nodeID, active, theEvalState,
		  isRootNode ? "Whole Program" : label,
		  shgname, isRootNode);

   // note: we _intentionally_ don't redraw...do you see why?
   // (no edge connections to this node --> it shouldn't appear yet) 
   // exception: the root node
   if (isRootNode)
      initiateShgRedraw(interp, true);

   return 1;
}

int 
UIM::DAGaddEdge (int dagID, unsigned srcID, 
		 unsigned dstID, int styleID,
		 const char *label // only used for shadow node; else NULL
		 )
{
   shg &theShg = theShgPhases->getByID(dagID);
   bool active;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState);

   theShg.addEdge(srcID, // parent
		  dstID, // child
		  theEvalState,
		  label);

   initiateShgRedraw(interp, true); // true --> double buffer

   return 1;
}

  
int 
UIM::DAGconfigNode (int dagID, unsigned nodeID, int styleID)
{
   shg &theShg = theShgPhases->getByID(dagID);
   bool active;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState);
  
   if (theShg.configNode(nodeID, active, theEvalState))
      initiateShgRedraw(interp, true); // interp --> double buffer

   return 1;
}
