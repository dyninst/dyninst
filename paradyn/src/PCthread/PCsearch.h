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
 * $Id: PCsearch.h,v 1.22 2003/05/21 18:21:17 pcroth Exp $
 * PCsearch.h: State information required throughout a search.
 */

#ifndef pc_search_h
#define pc_search_h

#include "PCmetricInst.h"
#include "PCshg.h"

typedef enum {schNeverRun, schPaused, schRunning, schEnded} schState;
typedef unsigned SearchQKey;

class costModule;

class PCsearch {
  friend class performanceConsultant;
  friend ostream& operator <<(ostream &os, PCsearch& srch);
public:
  PCsearch(unsigned phase, phaseType phase_type);
  ~PCsearch();
  void pause(); 
  void resume();
  void terminate(relTimeStamp searchEndTime);
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
  void decrNumActiveExperiments() {
    if (isGlobal())
      PCsearch::numActiveGlobalExperiments -= 1;
    else
      PCsearch::numActiveCurrentExperiments -= 1;
  }

  void notifyDynamicChild(resourceHandle parent, resourceHandle child){
    shg->notifyDynamicChild(parent, child);
  }

  static void updateCurrentPhase (unsigned phaseID, relTimeStamp endTime);
  static PCsearch *findSearch (phaseType pt);
  static bool addSearch (unsigned phaseID);
  static void expandSearch (float observedRecentCost);
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
  static int getNumActiveExperiments() {
    return PCsearch::numActiveGlobalExperiments + 
      PCsearch::numActiveCurrentExperiments;
  }
  static int getNumPendingSearches() {
    return PendingCurrentSearches + PendingGlobalSearches;
  }
  static float getPendingCost() {
    return (PendingCurrentCost + PendingGlobalCost); }
  static void addGlobalActiveExperiment() {
    PCsearch::numActiveGlobalExperiments += 1;}
  static void addCurrentActiveExperiment() {
    PCsearch::numActiveCurrentExperiments += 1;}
  static void decrNumPendingGlobalSearches() 
    {PCsearch::PendingGlobalSearches -= 1;}
  static void decrNumPendingCurrentSearches() 
    {PCsearch::PendingCurrentSearches -= 1;}
  static void initCostTracker();
  static void clearPendingCurrentCost(float val) 
    { PCsearch::PendingCurrentCost -= val; }
  static void clearPendingGlobalCost(float val) 
    { PCsearch::PendingGlobalCost -= val; }
private:
  schState searchStatus;        // schNeverRun/schPaused/schRunning/schEnded
  unsigned phaseToken;          // identifier for phase of this search
  phaseType phType;             // global or current; need for DM interface
  PCmetricInstServer *database;
  searchHistoryGraph *shg;
  bool isGlobal() {return (phType == GlobalPhase);}
  static relTimeStamp phaseChangeTime;     // last phase start time
  static dictionary_hash<unsigned, PCsearch*>AllPCSearches;
  static unsigned PCactiveCurrentPhase;
  static costModule *costTracker;
  static float PendingGlobalCost;
  static float PendingCurrentCost;
  static int PendingCurrentSearches;
  static int PendingGlobalSearches;
  static int numActiveGlobalExperiments;
  static int numActiveCurrentExperiments;
  static PriorityQueue<SearchQKey, searchHistoryNode*> GlobalSearchQueue;
  static PriorityQueue<SearchQKey, searchHistoryNode*> CurrentSearchQueue;
  static bool CurrentSearchPaused;
  static bool GlobalSearchPaused;
  static searchHistoryNode *SearchThrottleNode;
  static bool SearchThrottledBack;
  static PriorityQueue<SearchQKey, searchHistoryNode*> *q;
};

ostream& operator <<(ostream &os, PCsearch& srch);
        
class costModule : public dataSubscriber 
{
 public:
  void newData (PCmetDataID, pdRate newVal, relTimeStamp, relTimeStamp)
    {
	newVal = (newVal - pdRate(1))/newVal;
	if (newVal < pdRate(performanceConsultant::predictedCostLimit))
	    // check search queue and expand search if possible
	      PCsearch::expandSearch(static_cast<float>(newVal.getValue()));
    }
  void updateEstimatedCost(float) {;}
  void enableReply(unsigned, unsigned, unsigned, bool, string = "") {;}
  PCmetInstHandle costFilter;
};


#endif

