
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

/* 
 * PCpublic.C
 *
 * PC thread interface functions
 *
 * $Log: PCpublic.C,v $
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
    if (!PChyposDefined) {
      initResources();
      // this is used to normalize data throughout the PC
      metricHandle *tmpmh = dataMgr->findMetric("pause_time");
      assert (tmpmh);
      performanceConsultant::normalMetric = *tmpmh;
      delete tmpmh;
    }

    // create new search 
    bool sflag = PCsearch::addSearch(phaseID);
    assert (sflag);
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
performanceConsultant::endSearch(unsigned phaseID)
{
#ifdef PCDEBUG
  cout << "end search requested for phaseID = " << phaseID << endl;
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


