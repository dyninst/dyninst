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
/* Revision 1.57  1996/05/07 18:05:56  newhall
/* added threadExiting routine
/*
 * Revision 1.56  1996/05/01  20:54:20  tamches
 * added DAGinactivateEntireSearch
 *
 * Revision 1.55  1996/05/01 14:07:54  naim
 * Multiples changes in UI to make call to requestNodeInfoCallback async.
 * (UI<->PC) - naim
 *
 * Revision 1.54  1996/04/19  18:28:17  naim
 * Adding a procedure that will be called when we want to add a new process,
 * as it is done using the "paradyn process" command - naim
 *
 * Revision 1.53  1996/04/18  20:46:35  tamches
 * new DAGaddBatchOfEdges to correspond with PCthread/PCshg.C changes
 *
 * Revision 1.52  1996/04/16 18:37:27  karavan
 * fine-tunification of UI-PC batching code, plus addification of some
 * Ari-like verbification commentification.
 *
 * Revision 1.51  1996/04/13 04:39:39  karavan
 * better implementation of batching for edge requests
 *
 * Revision 1.50  1996/04/09 19:25:07  karavan
 * added batch mode to cut down on shg redraw time.
 *
 * Revision 1.49  1996/04/07 21:17:12  karavan
 * changed new phase notification handling; instead of being notified by the
 * data manager, the UI is notified by the performance consultant.  This prevents
 * a race condition.
 *
 * Revision 1.48  1996/04/01 22:42:14  tamches
 * added UI_all_metric_names, UI_all_metrics_set_yet
 * removed uim_AvailMets etc.
 * new params in call to getMetsAndRes
 *
 * Revision 1.47  1996/03/08 00:15:53  tamches
 * where appropriate, some more showError() calls pass empty string as 2d arg
 *
 * Revision 1.46  1996/02/23 22:10:01  naim
 * Adding igen call to display status line (fixes a deadlock situation) - naim
 *
 * Revision 1.45  1996/02/21  22:28:10  tamches
 * correct handling of blank 2d arg to showError
 *
 * Revision 1.44  1996/02/15 23:08:02  tamches
 * added correct support for why vs. where axis refinement in the shg
 *
 * Revision 1.43  1996/02/11 18:22:48  tamches
 * internal cleanup; more tk window names parameterized
 *
 * Revision 1.42  1996/02/07 21:46:57  tamches
 * defineNewSearch returns bool
 *
 * Revision 1.41  1996/02/07 19:06:13  tamches
 * initSHG gone
 * root node now TopLevelHypothesis instead of Whole Program
 * deferred phase adding features
 *
 * Revision 1.40  1996/02/02 18:42:17  tamches
 * Global search initialized when the shg window is
 * UIM::initShg should now be unused
 * new cleaner shgPhases routines corresponding to the PC-->UI igen calls
 *
 * Revision 1.39  1996/02/02 01:01:28  karavan
 * Changes to support the new PC/UI interface
 *
 * Revision 1.38  1996/01/30 23:04:22  tamches
 * removed include to obsolete file shgDisplay.h
 *
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
vector<metric_focus_pair> uim_VisiSelections; // keep this one

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
  if (script != NULL) {   // script specified on paradyn command line
    string tcommand = string("source \"") + string(script) + "\"";

    if (Tcl_Eval (interp, tcommand.string_of()) == TCL_ERROR)
      uiMgr->showError(24, "");
  }
}

// ****************************************************************
// Error Message Display Service 
// ****************************************************************

void
UIM::showError(int errCode, const char *errString)
{
    // errString -- custom error info string to be printed
    // Note that UIthread code can make the call to the tcl
    // routine "showError" directly; no need to call us.

    string tcommand = string("showError ") + string(errCode) + string(" ");
    if (errString == NULL || errString[0] == '\0')
       tcommand += string("\"\"");
    else
       tcommand += string("{") + string(errString) + string("}");
    myTclEval(interp, tcommand);
}

// ****************************************************************
// Status Line Display Service
// ****************************************************************

void
UIM::updateStatus(status_line *status, const char *msg)
{
  status->message(msg);
}

// ****************************************************************
// Metrics and Resources 
// ****************************************************************

unsigned metric_name_hash(const unsigned &metid) {return metid;}
dictionary_lite<unsigned, string> UI_all_metric_names(metric_name_hash);
   // met-id (not index!) to name
bool UI_all_metrics_set_yet = false;

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
  if (newptr == 0) {
    uiMgr->showError(21, "");
    thr_exit(0);
  }

  unsigned requestingThread = getRequestingThread();
     // in theory, we can check here whether this (VISI-) thread already
     // has an outstanding metric request.  But for now, we let code in mets.tcl do this...
//  string commandStr = string("winfo exists .metmenunew") + string(requestingThread);
//  myTclEval(interp, commandStr);
//  int result;
//  assert(TCL_OK == Tcl_GetBoolean(interp, interp->result, &result));
//  if (result)
//     return; // the window is already up for this thread!

  UIMReplyRec *reply = new UIMReplyRec;
  reply->tid = requestingThread;
  reply->cb = (showMsgCBFunc) cb;
  Tcl_SetHashValue (entryPtr, reply);

  if (!UI_all_metrics_set_yet) {
      vector<met_name_id> *all_mets = dataMgr->getAvailableMetInfo(true);
      
      for (unsigned metlcv=0; metlcv < all_mets->size(); metlcv++) {
	 unsigned id  = (*all_mets)[metlcv].id;
	 string &name = (*all_mets)[metlcv].name;

	 UI_all_metric_names[id] = name;

	 string idString(id);
	 assert(Tcl_SetVar2(interp, "metricNamesById", idString.string_of(),
			    name.string_of(), TCL_GLOBAL_ONLY));
      }
      
      delete all_mets;
      UI_all_metrics_set_yet = true;
  }

  // Set metIndexes2Id via "temp"
  (void)Tcl_UnsetVar(interp, "temp", 0);
     // ignore result; temp may not have existed
  vector<met_name_id> *curr_avail_mets_ptr = dataMgr->getAvailableMetInfo(false);
  vector<met_name_id> &curr_avail_mets = *curr_avail_mets_ptr;
  unsigned numAvailMets = curr_avail_mets.size();
  for (unsigned metlcv=0; metlcv < numAvailMets; metlcv++) {
     string metricIdStr = string(curr_avail_mets[metlcv].id);
     
     assert(Tcl_SetVar(interp, "temp", metricIdStr.string_of(),
		       TCL_APPEND_VALUE | TCL_LIST_ELEMENT));
  }
  delete curr_avail_mets_ptr;
  

  string tcommand("getMetsAndRes ");
  tcommand += string(UIMMsgTokenID);
  tcommand += string(" ") + string(requestingThread);
  tcommand += string(" ") + string(numAvailMets);
  tcommand += string(" $temp");

  int retVal = Tcl_VarEval (interp, tcommand.string_of(), 0);
  if (retVal == TCL_ERROR)  {
    uiMgr->showError (22, "");
    cerr << interp->result << endl;
    thr_exit(0);  
  } 
}

//
// called by an exiting thread to notify the UI that it is exiting
// this is necessary so that the UI does not try to send a metrics
// menuing response to a dead tid
//
void UIM::threadExiting(){

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

extern shgPhases *theShgPhases;

// The following two variables tell the shg which phase to try to
// activate when _first_ opening the shg window.  We initialize it
// to the well-known values for the "current phase" which is
// created on startup.
int latest_detected_new_phase_id = 1;
const char *latest_detected_new_phase_name = "phase_0";

void UIM::newPhaseNotification (unsigned ph, const char *name, bool with_new_pc) {
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
	 assert(theShgPhases->changeByPhaseId(ph));

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

void
UIM::updateStatusDisplay (int dagid, string *info)
{
   theShgPhases->addToStatusDisplay(dagid, *info);
   delete info;
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
   extern int latest_detected_new_phase_id;
   extern const char *latest_detected_new_phase_name;
   if (latest_detected_new_phase_id >= 0) {
      theShgPhases->defineNewSearch(latest_detected_new_phase_id,
				    latest_detected_new_phase_name);
   }

   initiateShgRedraw(interp, true);

   return true;
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

shgRootNode::refinement int2refinement(int styleid) {
   if (styleid == 0)
      return shgRootNode::ref_where;
   else if (styleid == 1)
      return shgRootNode::ref_why;

   cerr << "unrecognized refinement id " << styleid << endl;
   exit(5);
}

/*  flags: 1 = root
 *         0 = non-root
 */

int 
UIM::DAGaddNode(int dagID, unsigned nodeID, int styleID, 
		const char *label, const char *shgname, int flags)
{
   const bool isRootNode = (flags == 1);
   bool active;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState);

   if (theShgPhases->addNode(dagID, nodeID, active, theEvalState,
			     label, shgname, isRootNode))
      // shouldn't we should only be redrawing for a root node?
      if (isRootNode)
         initiateShgRedraw(interp, true);

   return 1;
}

void
UIM::DAGaddBatchOfEdges (int dagID, vector<uiSHGrequest> *requests,
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

int 
UIM::DAGaddEdge (int dagID, unsigned srcID, 
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

   return 1;
}

  
int 
UIM::DAGconfigNode (int dagID, unsigned nodeID, int styleID)
{
   bool active;
   shgRootNode::evaluationState theEvalState;
   int2style(styleID, active, theEvalState);
  
   if (theShgPhases->configNode(dagID, nodeID, active, theEvalState))
      initiateShgRedraw(interp, true); // true --> double buffer

   return 1;
}

void UIM::DAGinactivateEntireSearch(int dagID) {
   if (theShgPhases->inactivateEntireSearch(dagID))
      initiateShgRedraw(interp, true); // true --> double buffer
}

void UIM::requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                  shg_node_info *theInfo, bool ok)
{
  if (ok) {
    theShgPhases->nodeInformation(phaseID,nodeID,*theInfo);
    delete theInfo;
  }
}

//
// This procedure is used when paradyn create a process after 
// reading a configuration file (using option -f).
//
void UIM::ProcessCmd(string *args)
{
  string command;
  command = string("paradyn process ") + (*args);
  if (Tcl_VarEval(interp,command.string_of(),0)==TCL_ERROR) {
    string msg = string("Tcl interpreter failed in routine UIM::ProcessCmd: ");
    msg += string((const char *) interp->result);
    uiMgr->showError(83, P_strdup(msg.string_of()));
  }  
  delete args;
}
