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
 * PCsearch.h
 *
 * State information required throughout a search.
 *
 * $Log: PCsearch.h,v $
 * Revision 1.5  1996/04/07 21:29:43  karavan
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
 * Revision 1.4  1996/03/18 07:13:01  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.3  1996/02/22 18:27:04  karavan
 * changed GUI node styles from #defines to enum
 *
 * added status message for PC pause/resume
 *
 * Revision 1.2  1996/02/09 05:30:59  karavan
 * changes to support multiple per phase searching.
 *
 * Revision 1.1  1996/02/02 02:07:33  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_search_h
#define pc_search_h

#include "PCmetricInst.h"
#include "PCshg.h"

typedef enum schState {schNeverRun, schPaused, schRunning, schEnded};
typedef unsigned SearchQKey;

class costModule;

class PCsearch {
  friend class performanceConsultant;
public:
  PCsearch(unsigned phase, phaseType phase_type);
  ~PCsearch();
  void pause(); 
  void resume();
  void terminate(timeStamp searchEndTime);
  void printResults();
  unsigned getPhase() { return phaseToken; }
  bool paused() {return (searchStatus == schPaused);}
  bool newbie() {return (searchStatus == schNeverRun);}
  PCmetricInstServer *getDatabase() {return database;}
  void startSearching();
  bool getNodeInfo(int nodeID, shg_node_info *theInfo);
  static void updateCurrentPhase (unsigned phaseID, timeStamp endTime);
  static PCsearch *findSearch (phaseType pt);
  static bool addSearch (unsigned phaseID);
  static void expandSearch (sampleValue observedRecentCost);
  static void addToQueue(int key, searchHistoryNode *node, unsigned pid) {
    if (pid == GlobalPhaseID)
      PCsearch::GlobalSearchQueue.add(key, node);
    else
      PCsearch::CurrentSearchQueue.add(key, node);
  }
  static void printQueue(unsigned pid) {
    if (pid == GlobalPhaseID) {
      cout << "     ++ Global Search Queue ++" << endl;
      cout << GlobalSearchQueue << endl;
    } else {
      cout << "     ++ Current Search Queue ++" << endl;
      cout << CurrentSearchQueue << endl;
    }
  }
  static int getNumActiveExperiments() {return numActiveExperiments;}
  static void incrNumActiveExperiments() {numActiveExperiments++;}
  static void decrNumActiveExperiments() {numActiveExperiments--;}
  static void initCostTracker();
private:
  schState searchStatus;  // schNeverRun/schPaused/schRunning/schEnded
  unsigned phaseToken;          // identifier for phase of this search
  phaseType phType;     // global or current; need for DM interface
  PCmetricInstServer *database;
  searchHistoryGraph *shg;
  static dictionary_hash<unsigned, PCsearch*>AllPCSearches;
  static unsigned PCactiveCurrentPhase;
  static int numActiveExperiments;
  static costModule *costTracker;
  static PriorityQueue<SearchQKey, searchHistoryNode*> GlobalSearchQueue;
  static PriorityQueue<SearchQKey, searchHistoryNode*> CurrentSearchQueue;
  static bool CurrentSearchPaused;
  static bool GlobalSearchPaused;
};

class costModule : public dataSubscriber 
{
 public:
  void newData (PCmetDataID, sampleValue newVal, timeStamp, timeStamp, 
	   sampleValue)
    {
      if (newVal < performanceConsultant::predictedCostLimit)
	// check search queue and expand search if possible
	PCsearch::expandSearch(newVal);
    }
  void updateEstimatedCost(float) {;}
  PCmetInstHandle costFilter;
};


#endif

