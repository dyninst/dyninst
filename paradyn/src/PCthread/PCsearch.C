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
 * Revision 1.3  1996/02/09 05:30:57  karavan
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

#include <iostream.h>
#include <fstream.h>

extern void initPChypos();
extern void initPCmetrics();

unsigned int PCunhash (const unsigned &val) {return (val >> 2);} 

unsigned PCsearch::PCactiveCurrentPhase = 0;  // init to undefined
dictionary_hash<unsigned, PCsearch*> PCsearch::AllPCSearches(PCunhash);

bool PChyposDefined = false;
int PCnumActiveExperiments = 0;

PCsearch::PCsearch(u_int phaseID, phaseType phase_type)
: searchStatus(schNeverRun),  phaseToken(phaseID), phType(phase_type), 
  runQUpdateNeeded(false)
{
  database = new PCmetricInstServer(phase_type);  
  shg = new searchHistoryGraph (this, phaseID);
}

PCsearch::~PCsearch()
{
  delete database;
  delete shg;
}

bool
PCsearch::addSearch(phaseType pType)
{
  unsigned phaseID;
  // we can't use the dm token really, cause there's no dm token for 
  // global search, which throws our part of the universe into total 
  // confusion.  So, internally and in communication with the UI, we always
  // use dm's number plus one, and 0 for global phase.  
  if (pType == GlobalPhase) {
    phaseID = 0;
  } else {
    phaseID = performanceConsultant::DMcurrentPhaseToken + 1;
    performanceConsultant::currentPhase = phaseID;
  }

  PCsearch *newkid = new PCsearch(phaseID , pType);
  AllPCSearches[phaseID] = newkid;

  // * initialize PC displays *
  string msg;
  if (pType == GlobalPhase) {
    msg =  "Initializing Search for Global Phase.\n";
  } else {
    msg = "Initializing Search for Current Phase.\n";
  }
  uiMgr->updateStatusDisplay(phaseID, msg.string_of());

  // display root node with style 1 
  uiMgr->DAGaddNode (phaseID, 0, INACTIVEUNKNOWNNODESTYLE, 
		     "TopLevelHypothesis", 
		     "TopLevelHypothesis", 1);

  // if this is first search, initialize all PC metrics and hypotheses
  if (!PChyposDefined) {
    initPCmetrics();
    initPChypos();
    PChyposDefined = true;
  }
  return true;
}
  
//** this is for testing only!!
//
void
PCsearch::startSearching()
{
  shg->initPersistentNodes();
  searchStatus = schRunning;
}

void 
PCsearch::updateCurrentPhase (timeStamp phaseEndTime) 
{
  unsigned phaseID = performanceConsultant::currentPhase;
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

void
PCsearch::newData(metricInstanceHandle m_handle, sampleValue value, 
		  int bucketNum) 
{
  // pass along this new data value
  database->newRawData(m_handle, value, bucketNum);

  // if this piece of data has caused certain status changes, we may start 
  // new experiments now.
  if (runQUpdateNeeded) {
    searchHistoryNode *curr;
    bool result;

    //** currentSearchCost is WRONG.
//    float currentSearchCost = (100 * dataMgr->getCurrentSmoothObsCost());
//    cout << "TOTAL CURRENT SEARCH COST = " << currentSearchCost << endl;
    // tunable constant predictedCostLimit
    float searchCostThreshold = 100 * performanceConsultant::predictedCostLimit; 
    // we're using number of experiments only until observed cost is 
    // straightened out
    float currentSearchCost = PCnumActiveExperiments;
    while ((!SearchQueue.empty()) && 
	   (currentSearchCost < searchCostThreshold)) {
      curr = SearchQueue.peek_first_data();
      result = curr->startExperiment();
      if (result) {
	currentSearchCost++;
	//currentSearchCost += curr->getEstimatedCost();
      } else {
	//**
	cout << "unable to start experiment for node: " 
	  << curr->getNodeId() << endl;
      }
      SearchQueue.delete_first();
    }
    runQUpdateNeeded = false;
  }
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
