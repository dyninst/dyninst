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
 * PCsearch.C
 * 
 * class PCsearch
 *
 * $Log: PCsearch.C,v $
 * Revision 1.23  1997/02/06 20:47:52  karavan
 * changed MaxActiveExperiments constant to guard against deadlock.
 *
 * Revision 1.22  1996/08/16 21:03:41  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.21  1996/07/26 18:02:37  karavan
 * added display of status if search throttled back.
 *
 * Revision 1.20  1996/07/24 20:10:37  karavan
 * Fixed error in numActiveExperiments calculation; numActiveCurrentExperiments
 * now zero'd at phase boundary.
 *
 * Revision 1.19  1996/07/23 20:28:05  karavan
 * second part of two-part commit.
 *
 * implements new search strategy which retests false nodes under certain
 * circumstances.
 *
 * change in handling of high-cost nodes blocking the ready queue.
 *
 * code cleanup.
 *
 * Revision 1.18  1996/07/22 18:55:44  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.17  1996/05/16 06:58:37  karavan
 * increased max num experiments and also min time to conclusion.
 *
 * Revision 1.16  1996/05/15 04:35:17  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.15  1996/05/11 01:58:03  karavan
 * fixed bug in PendingCost calculation.
 *
 * Revision 1.14  1996/05/08 07:35:26  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 */

#include "PCintern.h"
#include "PCsearch.h"
#include "PCfilter.h"
#include "PCshg.h"
#include "PCmetric.h"

#include <iostream.h>
#include <fstream.h>

// for debugging purposes
#ifdef MYPCDEBUG
extern double TESTgetTime();
#endif

extern void initPChypos();
extern void initPCmetrics();
extern sampleValue DivideEval (focus, sampleValue *data, int dataSize);

unsigned int PCunhash (const unsigned &val) {return (val >> 3);} 

unsigned PCsearch::PCactiveCurrentPhase = 0;  // init to undefined
dictionary_hash<unsigned, PCsearch*> PCsearch::AllPCSearches(PCunhash);
PriorityQueue<SearchQKey, searchHistoryNode*> PCsearch::GlobalSearchQueue;
PriorityQueue<SearchQKey, searchHistoryNode*> PCsearch::CurrentSearchQueue;
PriorityQueue<SearchQKey, searchHistoryNode*> *PCsearch::q = &PCsearch::GlobalSearchQueue;
int PCsearch::numActiveGlobalExperiments = 0;
int PCsearch::numActiveCurrentExperiments = 0;
bool PCsearch::GlobalSearchPaused = false;
bool PCsearch::CurrentSearchPaused = false;
costModule *PCsearch::costTracker = NULL;
float PCsearch::PendingCurrentCost = 0.0;
float PCsearch::PendingGlobalCost = 0.0;
int PCsearch::PendingGlobalSearches = 0;
int PCsearch::PendingCurrentSearches = 0;
bool PCsearch::SearchThrottledBack = false;
searchHistoryNode *PCsearch::SearchThrottleNode = NULL;

//** this is currently being studied!! (klk)
const float costFudge = 0.1;
const int MaxPendingSearches = 30;
const int MaxActiveExperiments = 100;
//
// remove from search queues and start up as many experiments as we can 
// without exceeding our cost limit.  
//
void 
PCsearch::expandSearch (sampleValue estimatedCost)
{
  bool costLimitReached = false;
  searchHistoryNode *curr;
  float candidateCost = 0.0;

#ifdef PCDEBUG
  cout << "START OF EXPAND" << endl;
  cout << "Global Qsize: " << GlobalSearchQueue.size() << 
    " Current Qsize: " << CurrentSearchQueue.size() << endl;
#endif

  // alternate between two queues for fairness
  while (!costLimitReached 
	 && (PCsearch::getNumPendingSearches() < MaxPendingSearches)
	 && (PCsearch::getNumActiveExperiments() < MaxActiveExperiments)) {
    // switch queues for fairness, if possible; never use q if empty or that 
    // search is paused
    if (q == &CurrentSearchQueue) {
	if ( !(GlobalSearchQueue.empty() || GlobalSearchPaused))
	  q = &GlobalSearchQueue;
	else
	  if (q->empty() || CurrentSearchPaused) 
	    // no queue is ready
	    break;
    } else {
      if ( !(CurrentSearchQueue.empty() || CurrentSearchPaused)) 
	q = &CurrentSearchQueue;
      else
	if (q->empty() || GlobalSearchPaused) 
	  // no queue is ready 
	  break;
    }
    curr = q->peek_first_data();
    candidateCost = curr->getEstimatedCost();
#ifdef PCDEBUG
    cout << " considering node with cost: " << candidateCost << endl;
#endif
    sampleValue predMax = (1-costFudge)*performanceConsultant::predictedCostLimit;
    if (candidateCost > predMax) {
      // **for now just get it out of the way
      int dispToken = curr->getGuiToken();
      q->delete_first();
      string *ds = new string 
	("WARNING:  Predicted Node Search Code exceeds limit.  Skipped Node:\n\t");
      *ds += curr->getShortName();
      *ds += "||";
      *ds += curr->getHypoName();
      *ds += "\n";
      uiMgr->updateStatusDisplay(dispToken, ds);
    } else if ((estimatedCost + candidateCost + PCsearch::getPendingCost()) 
	       > predMax) {
      costLimitReached = true;
      // print status to display but just once per blockage
      if ( !PCsearch::SearchThrottledBack && 
	  (curr != PCsearch::SearchThrottleNode)) {
	int dispToken = curr->getGuiToken();
	string *ds = new string ("Search Slowed:  Cost Limit Reached.\n");
	uiMgr->updateStatusDisplay(dispToken, ds);
	PCsearch::SearchThrottleNode = curr;
	PCsearch::SearchThrottledBack = true;
      }
    } else {
      curr->startExperiment();
      if (PCsearch::SearchThrottledBack) {
	int dispToken = curr->getGuiToken();
	string *ds = new string ("Search Resumed.\n");
	uiMgr->updateStatusDisplay(dispToken, ds);
	PCsearch::SearchThrottledBack = false;
	PCsearch::SearchThrottleNode = NULL;
      }
      if (q == &GlobalSearchQueue) {
	PCsearch::PendingGlobalSearches += 1;
	PCsearch::PendingGlobalCost += candidateCost;
      } else {
	PCsearch::PendingCurrentSearches += 1;
	PCsearch::PendingCurrentCost += candidateCost;
      }
      q->delete_first();
    }
  }   
  if ((PCsearch::getNumActiveExperiments() >=  MaxActiveExperiments) 
      && (!PCsearch::SearchThrottledBack)) {
    int dispToken = curr->getGuiToken();
    string *ds = new string ("Search Slowed:  max active nodes reached.\n");
    uiMgr->updateStatusDisplay(dispToken, ds);
    PCsearch::SearchThrottledBack = true;
  } else if ((PCsearch::getNumPendingSearches() >=  MaxPendingSearches)
	     && (!PCsearch::SearchThrottledBack)) {
    int dispToken = curr->getGuiToken();
    string *ds = new string ("Search Slowed:  max pending nodes.\n");
    uiMgr->updateStatusDisplay(dispToken, ds);
    PCsearch::SearchThrottledBack = true;
  }
#ifdef PCDEBUG
  cout << "END OF EXPAND" << endl;
  cout << "cost limit: " << performanceConsultant::predictedCostLimit << endl;
  cout << "total observed cost: " << estimatedCost << 
    "/" << (1-costFudge)*performanceConsultant::predictedCostLimit << endl;
  cout << "numActiveExperiments: " << PCsearch::getNumActiveExperiments() << endl;
  cout << "pendingEnables: " << PCsearch::getNumPendingSearches() << endl;
  cout << "pendingCost = " << PCsearch::getPendingCost() << endl;
#endif
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
  bool err = true;
  PCmetric *pcm;
  assert (PCmetric::AllPCmetrics.defines("normSmoothCost"));

  pcm = PCmetric::AllPCmetrics ["normSmoothCost"];
  PCsearch::costTracker = new costModule;
  PCmetricInstServer *miserve = new PCmetricInstServer (GlobalPhase);
  costTracker->costFilter = 
    miserve->createPcmi(pcm, topLevelFocus, &err);
  assert(costTracker->costFilter);
  costTracker->costFilter->addSubscription(costTracker);
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
  if (!performanceConsultant::PChyposDefined) {
    initPCmetrics();
    initPChypos();
    performanceConsultant::PChyposDefined = true;
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
    PCsearch::PendingCurrentCost = 0;
    PCsearch::PendingCurrentSearches = 0;
    PCsearch::numActiveCurrentExperiments = 0;
  } else {
    while (!PCsearch::GlobalSearchQueue.empty()) {
      GlobalSearchQueue.delete_first();
    }
    PCsearch::PendingGlobalCost = 0;
    PCsearch::PendingGlobalSearches = 0;
    PCsearch::numActiveGlobalExperiments = 0;
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
  PCsearch::numActiveCurrentExperiments = 0;
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
