
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

//** debug prints need to come back out!!
void 
performanceConsultant::activateSearch(unsigned phaseID)
{
  // check if search already exists for this phase; if so unpause or 
  // start, if new
  if (PCsearch::AllPCSearches.defines(phaseID)) {
    cout << "ACTIVATE SUCCEEDS FOR PHASEID: " << phaseID << endl; 
    PCsearch *specifiedSearch = PCsearch::AllPCSearches[phaseID];
    if (specifiedSearch->paused())
      specifiedSearch->resume();
    else if (specifiedSearch->newbie())
      specifiedSearch->startSearching();
  } else {
    cout << "ACTIVATE FAILS FOR PHASEID: " << phaseID << endl;
  }
}

void 
performanceConsultant::pauseSearch(unsigned phaseID)
{
  // find appropriate search object; if running, pause; else do nothing
  if (PCsearch::AllPCSearches.defines(phaseID)) {
    PCsearch::AllPCSearches[phaseID]->pause();
  }
}

void
performanceConsultant::newSearch(phaseType pt)
{

  // check if search already exists for this phase; if so do nothing
  if (pt == GlobalPhase) {
    if (PCsearch::AllPCSearches.defines(0)) 
      return;
  } else {
    if (performanceConsultant::currentPhase
	== (performanceConsultant::DMcurrentPhaseToken + 1))
      return;
  }
  // reaching this point means we need to setup a new search
  // initialize known/base resources and top level focus once only
  if (!PChyposDefined)
    initResources();

  // create new search 
  bool sflag = PCsearch::addSearch(pt);
  assert (sflag);
}

//
// endSearch isn't implemented in the UI yet, so this is just a 
// placeholder for the future.
//
void
performanceConsultant::endSearch(unsigned phaseID)
{
  cout << "end search requested for phaseID = " << phaseID << endl;
}

// Get loads of information about an SHG node:
// "theInfo" gets filled in.  Returns true iff successful.

bool
performanceConsultant::getNodeInfo(unsigned phaseID, int nodeID, 
				   shg_node_info *theInfo)
{
  // find appropriate search object
  if (PCsearch::AllPCSearches.defines(phaseID)) {
    if (PCsearch::AllPCSearches[phaseID]->getNodeInfo(nodeID, theInfo))
      return true;
  }
    return false;
}


