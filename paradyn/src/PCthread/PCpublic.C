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
 * PCpublic.C
 *
 * PC thread interface functions
 *
 * $Log: PCpublic.C,v $
 * Revision 1.41  1999/05/19 07:50:25  karavan
 *
 * Added new shg save feature.
 *
 * Revision 1.40  1996/11/26 16:13:53  naim
 * Fixing asserts - naim
 *
 * Revision 1.39  1996/08/16 21:03:38  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.38  1996/07/26 07:25:02  karavan
 * added global performanceConsultant::numMetrics which is used to size
 * some filteredDataServer data structures.
 *
 * Revision 1.37  1996/05/08 07:35:21  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.36  1996/05/06 04:35:20  karavan
 * Bug fix for asynchronous predicted cost changes.
 *
 * added new function find() to template classes dictionary_hash and
 * dictionary_lite.
 *
 * changed filteredDataServer::DataFilters to dictionary_lite
 *
 * changed normalized hypotheses to use activeProcesses:cf rather than
 * activeProcesses:tlf
 *
 * code cleanup
 *
 * Revision 1.35  1996/05/02 19:46:46  karavan
 * changed predicted data cost to be fully asynchronous within the pc.
 *
 * added predicted cost server which caches predicted cost values, minimizing
 * the number of calls to the data manager.
 *
 * added new batch version of ui->DAGconfigNode
 *
 * added hysteresis factor to cost threshold
 *
 * eliminated calls to dm->enable wherever possible
 *
 * general cleanup
 *
 * Revision 1.34  1996/05/01 14:07:01  naim
 * Multiples changes in PC to make call to requestNodeInfoCallback async.
 * (UI<->PC). I also added some debugging information - naim
 *
 * Revision 1.33  1996/04/21  21:44:23  newhall
 * removed performanceConsultant::getPredictedDataCostCallbackPC
 *
 * Revision 1.32  1996/04/18  22:01:57  naim
 * Changes to make getPredictedDataCost asynchronous - naim
 *
 * Revision 1.31  1996/04/07  21:29:40  karavan
 * split up search ready queue into two, one global one current, and moved to
 * round robin queue removal.
 *
 * eliminated startSearch(), combined functionality into activateSearch().  All
 * search requests are for a specific phase id.
 *
 * changed dataMgr->enableDataCollection2 to take phaseID argument, with needed
 * changes internal to PC to track phaseID, to avoid enable requests being handled
 * for incorrect current phase.
 *
 * added update of display when phase ends, so all nodes changed to inactive display
 * style.
 *
 * Revision 1.30  1996/02/09 05:30:55  karavan
 * changes to support multiple per phase searching.
 *
 * Revision 1.29  1996/02/02 02:06:44  karavan
 * A baby Performance Consultant is born!
 *
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
  ofstream saveFile (dir.string_of(), ios::out);
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
  delete filename;
  uiMgr->shgSaved(success);
}


