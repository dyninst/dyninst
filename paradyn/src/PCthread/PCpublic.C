/*
 * Copyright (c) 1996 Barton P. Miller
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
 * $Id: PCpublic.C,v 1.45 2002/05/13 19:53:09 mjbrim Exp $
 * PC thread interface functions
 */

#include <stdio.h>
#include <stdlib.h>

#include "PCintern.h"
#include "PCsearch.h"
extern void initResources();

void 
performanceConsultant::activateSearch(unsigned phaseID)
{
  // if no search exists for this phase, create one 
  if (!PCsearch::AllPCSearches.defines(phaseID)) {
    // this request may be for a phase we haven't heard about yet. 
    // if so, update our notion of the current phase
    if (phaseID > (performanceConsultant::DMcurrentPhaseToken + 1)) {
      performanceConsultant::DMcurrentPhaseToken = phaseID-1;
    }
    // initialize known/base resources and top level focus once only
    if (!performanceConsultant::PChyposDefined) {
      initResources();
      // this is used to normalize data throughout the PC
      metricHandle *tmpmh = dataMgr->findMetric("pause_time");
      assert (tmpmh);
      performanceConsultant::normalMetric = *tmpmh;
      delete tmpmh;
      // number of metrics is used to setup filteredDataServer data structures
      vector<string> *mets = dataMgr->getAvailableMetrics(true);
      performanceConsultant::numMetrics = mets->size();
      delete mets;
    }

    // create new search 
    bool aflag;
    aflag = PCsearch::addSearch(phaseID);
    assert (aflag);
  }
  PCsearch *specifiedSearch = PCsearch::AllPCSearches[phaseID];
  if (specifiedSearch->paused())
    specifiedSearch->resume();
  else if (specifiedSearch->newbie())
    specifiedSearch->startSearching();
}

void 
performanceConsultant::pauseSearch(unsigned phaseID)
{
  // find appropriate search object; if running, pause; else do nothing
  if (PCsearch::AllPCSearches.defines(phaseID)) {
    PCsearch::AllPCSearches[phaseID]->pause();
  }
}

//
// endSearch isn't implemented in the UI yet, so this is just a 
// placeholder for the future.
//
void
performanceConsultant::endSearch(unsigned)
{
#ifdef PCDEBUG
  cout << "end search requested" << endl;
#endif
}

void performanceConsultant::notifyDynamicChild(resourceHandle parent,
					       resourceHandle child){
  if(performanceConsultant::useCallGraphSearch == true){
    unsigned size = PCsearch::AllPCSearches.size();
    unsigned int i;
    for(i = 0; i < size; i++){
      if(PCsearch::AllPCSearches.defines(i)){
	PCsearch::AllPCSearches[i]->notifyDynamicChild(parent, child);
      }
    }
  }
}

// Get loads of information about an SHG node:
// "theInfo" gets filled in.  Returns true iff successful.

void
performanceConsultant::requestNodeInfo(unsigned phaseID, int nodeID) 
{
  shg_node_info *theInfo = new shg_node_info;
  bool ok=false;
  if (theInfo) {
    // find appropriate search object
    if (PCsearch::AllPCSearches.defines(phaseID)) {
      if (PCsearch::AllPCSearches[phaseID]->getNodeInfo(nodeID,theInfo))
        ok=true;
    }
  }
  uiMgr->requestNodeInfoCallback(phaseID,nodeID,theInfo,ok);
}

void
performanceConsultant::saveSHG(const char *filename, int flag)
{
  bool success = false;
  string dir = string (filename) + string("/shg.txt");
  ofstream saveFile (dir.c_str(), ios::out);
  if (!saveFile) {
    success = false;
  } else {
    if ((flag == 1) || (flag == 3)) { // global phase selected
      if (PCsearch::AllPCSearches.defines(0)) {
        saveFile << *(PCsearch::AllPCSearches[0]);
        success = true;
      }
    }
    if ((flag == 2) || (flag == 3))  {   // current phase selected
      if (PCsearch::PCactiveCurrentPhase > 0) {  // save all current phases
        for (unsigned counter = PCsearch::PCactiveCurrentPhase; counter > 0; counter--) {
          saveFile << *(PCsearch::AllPCSearches[counter]);
        }
        success = true;
      }
    }
    saveFile.close();
  }
  uiMgr->shgSaved(success);
}


