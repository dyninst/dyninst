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

/*
 * UIpublic.C : exported services of the User Interface Manager thread 
 *              of Paradyn
 */
 
/* $Id: UIpublic.C,v 1.75 2002/04/17 16:07:23 willb Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "UI.thread.CLNT.h"
#include "UI.thread.SRVR.h"
#include "UI.thread.h"
#include "thread/h/thread.h"
#include "UIglobals.h"
#include "../pdMain/paradyn.h"

#include "shgPhases.h"
#include "shgTcl.h"

#include "callGraphs.h"
#include "callGraphTcl.h"

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

    if (Tcl_Eval (interp,const_cast<char*>(tcommand.string_of())) == TCL_ERROR)
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
UIM::updateStatus(status_line* status, const char *msg)
{
  status->message(msg);
}

void
UIM::updateStatusLine(const char* sl_name, const char *msg)
{
    status_line *status = status_line::find(sl_name);
    assert(status);
    
  status->message(msg);
}

void
UIM::createStatusLine(const char* sl_name) {
    status_line *status = new status_line(sl_name);
}

void
UIM::createProcessStatusLine(const char* sl_name) {
    status_line *status = new status_line(sl_name, status_line::PROCESS);
}

void
UIM::destroyStatusLine(const char* sl_name) {
    status_line *status = status_line::find(sl_name);
    assert(status);

    delete status;
}

void UIM::enablePauseOrRun() {
   extern void enablePAUSEorRUN(); // paradyn.tcl.C
   enablePAUSEorRUN();
}

// ****************************************************************
// Metrics and Resources 
// ****************************************************************

unsigned metric_name_hash(const unsigned &metid) {return metid;}
dictionary_hash<unsigned, string> UI_all_metric_names(metric_name_hash, 16);
   // met-id (not index!) to name
bool UI_all_metrics_set_yet = false;

void 
UIM::chooseMetricsandResources(chooseMandRCBFunc cb,
			       vector<metric_focus_pair> * // pairList -- unused
   )
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
//  assert(TCL_OK == Tcl_GetBoolean(interp, Tcl_GetStringResult(interp), &result));
//  if (result)
//     return; // the window is already up for this thread!

  UIMReplyRec *reply = new UIMReplyRec;
  reply->tid = requestingThread;
  reply->cb = (void *) cb;
  Tcl_SetHashValue (entryPtr, reply);

  if (!UI_all_metrics_set_yet) {
      vector<met_name_id> *all_mets = dataMgr->getAvailableMetInfo(true);
      
      for (unsigned metlcv=0; metlcv < all_mets->size(); metlcv++) {
	 unsigned id  = (*all_mets)[metlcv].id;
	 string &name = (*all_mets)[metlcv].name;

	 UI_all_metric_names[id] = name;

	 string idString(id);
	 bool aflag;
	 aflag=(Tcl_SetVar2(interp, "metricNamesById", 
			    const_cast<char*>(idString.string_of()),
			    const_cast<char*>(name.string_of()), 
			    TCL_GLOBAL_ONLY) != NULL);
         assert(aflag);
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
  assert( numAvailMets > 0 );
  for (unsigned metlcv=0; metlcv < numAvailMets; metlcv++) {
     string metricIdStr = string(curr_avail_mets[metlcv].id);
     
     bool aflag;
     aflag = (Tcl_SetVar(interp, "temp", 
			 const_cast<char*>(metricIdStr.string_of()),
			 TCL_APPEND_VALUE | TCL_LIST_ELEMENT) != NULL);
     assert(aflag);
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
    cerr << Tcl_GetStringResult(interp) << endl;
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

extern callGraphs *theCallGraphPrograms;
//Adds a new program to the call graph display
void UIM::callGraphProgramAddedCB(unsigned programId, resourceHandle newId, 
				  const char *executableName, 
				  const char *shortName, const char *longName){
  theCallGraphPrograms->addNewProgram(programId, newId, executableName, shortName, longName);
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
   return shgRootNode::ref_why; // placate compiler
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

//Add node to the call graph
void UIM::CGaddNode(int pid, resourceHandle parent, resourceHandle newNode,
		   const char *shortName, const char *fullName, 
		    bool recursiveFlag, bool shadowFlag){
  theCallGraphPrograms->addNode(pid, parent, newNode, shortName, fullName,
				recursiveFlag, shadowFlag);
}

//This is called when the daemon notifies the DM that it has already sent
//all of the nodes for the static call graph. We should now display it.
void UIM::CGDoneAddingNodesForNow(int pid) {
  // "rethinks" the graphical display...expensive, so don't call this
  // often!
  theCallGraphPrograms->rethinkLayout(pid);
  initiateCallGraphRedraw(interp, true);
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

// ****************************************************************
// MISC 
// ****************************************************************

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
    msg += string(Tcl_GetStringResult(interp));
    uiMgr->showError(83, P_strdup(msg.string_of()));
  }  
  delete args;
}

// initialize list of external visis in the tcl interpreter:
//  this routine sets global tcl variables vnames, vnums, vcount
//
int compare_visi_names (const void *viptr1, const void *viptr2) {
  const VM_visiInfo *p1 = (const VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (const VM_visiInfo *)viptr2;
  return strcmp (p1->name.string_of(), p2->name.string_of());
}

void 
UIM::registerValidVisis (vector<VM_visiInfo> *via) {
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
    Tcl_DStringAppendElement(&namelist, (*via)[i].name.string_of());
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
UIM::allDataSaved(bool succeeded)
{
  if (succeeded) 
    ui_status->message("Requested Data Saved");
  else
    ui_status->message("Data Save Request Failed");
}

void 
UIM::resourcesSaved(bool succeeded)
{
  if (succeeded)
    ui_status->message("Resource Hierarchies Saved");
  else
    ui_status->message("Resource Hierarchy Save Request Failed");
}

void 
UIM::shgSaved (bool succeeded)
{
  if (succeeded)
    ui_status->message("Search History Graph Saved");
  else
    ui_status->message("Search History Graph Save Request Failed");
}
