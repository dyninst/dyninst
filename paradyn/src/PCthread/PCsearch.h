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
 * Revision 1.9  1996/05/11 01:58:04  karavan
 * fixed bug in PendingCost calculation.
 *
 * Revision 1.8  1996/05/08 07:35:27  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.7  1996/04/30 06:26:44  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
 * Revision 1.6  1996/04/16 18:36:17  karavan
 * BUG FIX.
 *
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
  void updateDisplayedStatus (string *msg) {
    shg->updateDisplayedStatus (msg);
  }
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
  static int getNumActiveExperiments() {return PCsearch::numActiveExperiments;}
  static void incrNumActiveExperiments() {PCsearch::numActiveExperiments += 1;}
  static void decrNumActiveExperiments() {PCsearch::numActiveExperiments -= 1;}
  static void decrNumPendingSearches() {PCsearch::PendingSearches -= 1;}
  static void initCostTracker();
  static void clearPendingCost(float val) { PCsearch::PendingCost -= val; }
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
  static float PendingCost;
  static int PendingSearches;
  static PriorityQueue<SearchQKey, searchHistoryNode*> GlobalSearchQueue;
  static PriorityQueue<SearchQKey, searchHistoryNode*> CurrentSearchQueue;
  static bool CurrentSearchPaused;
  static bool GlobalSearchPaused;
  static PriorityQueue<SearchQKey, searchHistoryNode*> *q;
};

class costModule : public dataSubscriber 
{
 public:
  void newData (PCmetDataID, sampleValue newVal, timeStamp, timeStamp, 
	   sampleValue)
    {
#ifdef PCDEBUG
      cout << "cost module returns: " << newVal << endl;
#endif
      if (newVal < performanceConsultant::predictedCostLimit)
	// check search queue and expand search if possible
	PCsearch::expandSearch(newVal);
    }
  void updateEstimatedCost(float) {;}
  void enableReply(unsigned, unsigned, unsigned, bool) {;}
  PCmetInstHandle costFilter;
};


#endif

