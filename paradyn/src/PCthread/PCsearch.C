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
 * PCsearch.C
 * 
 * class PCsearch
 *
 * $Log: PCsearch.C,v $
 * Revision 1.9  1996/04/16 18:36:16  karavan
 * BUG FIX.
 *
 * Revision 1.8  1996/04/07 21:29:41  karavan
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
 * Revision 1.7  1996/03/18 07:13:00  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.5  1996/02/22 18:27:00  karavan
 * changed GUI node styles from #defines to enum
 *
 * added status message for PC pause/resume
 *
 * Revision 1.4  1996/02/22 14:42:58  naim
 * Fixing number of searches that can be done simultaneously (change made by
 * Karen and commited by Oscar) - naim
 *
 * Revision 1.3  1996/02/09  05:30:57  karavan
 * changes to support multiple per phase searching.
 *
 * Revision 1.2  1996/02/08 19:52:48  karavan
 * changed performance consultant's use of tunable constants:  added 3 new
 * user-level TC's, PC_CPUThreshold, PC_IOThreshold, PC_SyncThreshold, which
 * are used for all hypotheses for the respective categories.  Also added
 * PC_useIndividualThresholds, which switches thresholds back to use hypothesis-
 * specific, rather than categorical, thresholds.
 *
 * Moved all TC initialization to PCconstants.C.
 *
 * Switched over to callbacks for TC value updates.
 *
 * Revision 1.1  1996/02/02 02:06:48  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCsearch.h"
#include "PCfilter.h"
#include "PCshg.h"
#include "PCmetric.h"

#include <iostream.h>
#include <fstream.h>

extern void initPChypos();
extern void initPCmetrics();
extern sampleValue DivideEval (focus, sampleValue *data, int dataSize);

unsigned int PCunhash (const unsigned &val) {return (val >> 2);} 

unsigned PCsearch::PCactiveCurrentPhase = 0;  // init to undefined
dictionary_hash<unsigned, PCsearch*> PCsearch::AllPCSearches(PCunhash);
PriorityQueue<SearchQKey, searchHistoryNode*> PCsearch::GlobalSearchQueue;
PriorityQueue<SearchQKey, searchHistoryNode*> PCsearch::CurrentSearchQueue;
int PCsearch::numActiveExperiments = 0;
bool PCsearch::GlobalSearchPaused = false;
bool PCsearch::CurrentSearchPaused = false;
costModule *PCsearch::costTracker = NULL;
bool PChyposDefined = false;

//
// remove from search queues and start up as many experiments as we can 
// without exceeding our cost limit.  
//
void 
PCsearch::expandSearch (sampleValue estimatedCost)
{
  static PriorityQueue<SearchQKey, searchHistoryNode*> *q = &GlobalSearchQueue;
  bool costLimitReached = false;
  searchHistoryNode *curr;
#ifdef PCDEBUG
  cout << "total observed cost: " << estimatedCost << endl;
  cout << "cost limit: " << performanceConsultant::predictedCostLimit << endl;
#endif

  // alternate between two queues for fairness
  while (!costLimitReached) {
    // switch queues for fairness, if possible; never use q if empty or that 
    // search is paused
    if (q == &CurrentSearchQueue) {
      if (q->empty() || CurrentSearchPaused) {
	// its this queue's turn but its empty or paused so try the other
	if (GlobalSearchQueue.empty() || GlobalSearchPaused) break;
	q = &GlobalSearchQueue;
      }
    } else {
      if (q->empty() || GlobalSearchPaused) {
	if (CurrentSearchQueue.empty() || CurrentSearchPaused) break;
	q = &CurrentSearchQueue;
      }
    }
    curr = q->peek_first_data();
    float newCost = curr->getEstimatedCost();
    if ((estimatedCost + newCost) > 
	performanceConsultant::predictedCostLimit) {
      costLimitReached = true;
    } else {
      if (curr->startExperiment()) {
	estimatedCost += newCost;
      } else {
#ifdef PCDEBUG
	cout << "unable to start experiment for node: " 
	  << curr->getNodeId() << endl;
#endif
      }
      q->delete_first();
    } 
  }
}

PCsearch::PCsearch(unsigned phaseID, phaseType phase_type)
: searchStatus(schNeverRun),  phaseToken(phaseID), phType(phase_type)
{
  if (phaseID == GlobalPhaseID)
    database = performanceConsultant::globalPCMetricServer;
  else 
    database = new PCmetricInstServer(phaseID);  
  shg = new searchHistoryGraph (this, phaseID);
}

PCsearch::~PCsearch()
{
  delete database;
  delete shg;
}

void 
PCsearch::initCostTracker () 
{
  bool err;
  PCmetric *pcm;
  assert (PCmetric::AllPCmetrics.defines("normSmoothCost"));

  pcm = PCmetric::AllPCmetrics ["normSmoothCost"];
  PCsearch::costTracker = new costModule;
  PCmetricInstServer *miserve = new PCmetricInstServer (GlobalPhase);
  costTracker->costFilter = 
    miserve->addSubscription(costTracker, pcm, topLevelFocus, &err);
  costTracker->costFilter->activate();
}

bool
PCsearch::addSearch(unsigned phaseID)
{
  string *msg;
  phaseType pType;
  // we can't use the dm token really, cause there's no dm token for 
  // global search, which throws our part of the universe into total 
  // confusion.  So, internally and in communication with the UI, we always
  // use dm's number plus one, and 0 for global phase.  
  if (phaseID > 0) {
    // non-global search
    performanceConsultant::currentPhase = phaseID;
    msg = new string ("Initializing Search for Current Phase ");
    *msg += (phaseID-1);
    *msg += string(".\n");
    pType = CurrentPhase;
  } else {
    msg =  new string ("Initializing Search for Global Phase.\n");
    pType = GlobalPhase;
  }

  // if this is first search, initialize all PC metrics and hypotheses
  if (!PChyposDefined) {
    initPCmetrics();
    initPChypos();
    PChyposDefined = true;
    initCostTracker();
  }

  PCsearch *newkid = new PCsearch(phaseID , pType);
  AllPCSearches[phaseID] = newkid;

  // * initialize PC displays *
  newkid->updateDisplayedStatus(msg);

  // display root node with style 1 
  uiMgr->DAGaddNode (phaseID, 0, searchHistoryGraph::InactiveUnknownNodeStyle, 
		     "TopLevelHypothesis", 
		     "TopLevelHypothesis", 1);
  return true;
}
  
//
// start instrumentation requests for the first time 
//
void
PCsearch::startSearching()
{
  shg->initPersistentNodes();
  searchStatus = schRunning;
}

//
// to pause a search we disable all instrumentation requests
//
void 
PCsearch::pause() 
{     
  if (searchStatus == schRunning) {
    searchStatus = schPaused;
    if (phType == GlobalPhase) 
      // no nodes will be started off the global search queue until false
      GlobalSearchPaused = true;
    else 
      // no nodes will be started off the current search queue until false
      CurrentSearchPaused = true;
    shg->updateDisplayedStatus ("Search Paused by User.\n");
    database->unsubscribeAllRawData();
  }
}

void 
PCsearch::resume() 
{
  if (searchStatus == schPaused) {
    searchStatus = schRunning;
    if (phType == GlobalPhase)
      GlobalSearchPaused = false;
    else
      CurrentSearchPaused = false;
    database->resubscribeAllRawData();
    shg->updateDisplayedStatus ("Search Resumed.\n");
  }
}

//
// this permanently ends a search, it can never be restarted
//
void 
PCsearch::terminate(timeStamp searchEndTime) {
  searchStatus = schEnded;
  // need to flush search nodes for this defunct search from the queue
  if (phType == CurrentPhase) {
    while (!PCsearch::CurrentSearchQueue.empty()) {
      CurrentSearchQueue.delete_first();
    }
  } else {
    while (!PCsearch::GlobalSearchQueue.empty()) {
      GlobalSearchQueue.delete_first();
    }
  }
  shg->finalizeSearch(searchEndTime);
}

void 
PCsearch::updateCurrentPhase (unsigned newPhaseID, timeStamp phaseEndTime) 
{
  unsigned phaseID = performanceConsultant::currentPhase;
  // the UI may be notified of the new phase, and the user select a 
  // search for it, before the PC gets this callback from the DM.  So
  // we must check here if we are already searching this "new" phase.
  if (phaseID == newPhaseID) return;
  // here if this is the first we're hearing of this phase.  If a search
  // is in progress for the old current phase, we need to end it.
  if (phaseID && AllPCSearches.defines(phaseID) ) {
    // this one's done for good
    AllPCSearches[phaseID]->terminate(phaseEndTime); 
  }
  // 0 indicates no current phase search in progress
  performanceConsultant::currentPhase = 0;
}

bool 
PCsearch::getNodeInfo(int nodeID, shg_node_info *theInfo)
{
  searchHistoryNode *const currNode = shg->getNode(nodeID);
  if (! currNode)
    return false;

  currNode->getInfo (theInfo);
  return true;
}

PCsearch *
PCsearch::findSearch (phaseType pt)
{
  if (pt == GlobalPhase) 
    return PCsearch::AllPCSearches[0];
  else {
    if (performanceConsultant::currentPhase && 
	PCsearch::AllPCSearches.defines(performanceConsultant::currentPhase))
      return PCsearch::AllPCSearches[performanceConsultant::currentPhase];
    else
      return (PCsearch *)NULL;
  }
}
