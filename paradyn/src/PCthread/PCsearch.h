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
extern void ExpandSearch(sampleValue);

class costModule : public dataSubscriber 
{
 public:
  void newData (PCmetDataID, sampleValue newVal, timeStamp, timeStamp endsAt, 
	   sampleValue)
    {
      //**
      cout << "cost modules receives: " << newVal << " ends at: " 
	<< endsAt << endl;  
      if (newVal < performanceConsultant::predictedCostLimit)
	// check search queue and expand search if possible
	ExpandSearch(newVal);
    }
  void updateEstimatedCost(float) {;}
  PCmetInstHandle costFilter;
};


class PCsearch {

  friend class performanceConsultant;

public:
  PCsearch(unsigned phase, phaseType phase_type);
  ~PCsearch();

  static void updateCurrentPhase (timeStamp endTime);
  static PCsearch *findSearch (phaseType pt);
  static bool addSearch (phaseType pt);
  void pause(); 
  void resume();
  void terminate(timeStamp searchEndTime) {
    searchStatus = schEnded;
    shg->finalizeSearch(searchEndTime);
  }
  void printResults();
  unsigned getPhase() { return phaseToken; }
  bool paused() {return (searchStatus == schPaused);}
  bool newbie() {return (searchStatus == schNeverRun);}
  PCmetricInstServer *getDatabase() {return database;}
  void startSearching();
  bool getNodeInfo(int nodeID, shg_node_info *theInfo);
  //** the horror!!! *data* temporarily in the *public* section!!! 
  static PriorityQueue<SearchQKey, searchHistoryNode*> SearchQueue;
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
};

#endif

