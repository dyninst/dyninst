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
 * PCsearch.h
 *
 * State information required throughout a search.
 *
 * $Log: PCsearch.h,v $
 * Revision 1.16  1999/05/19 07:50:29  karavan
 * Added new shg save feature.
 *
 * Revision 1.15  1997/03/16 23:17:08  lzheng
 * Changes made for the value of observed cost
 *
 * Revision 1.14  1996/08/16 21:03:43  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.13  1996/07/26 18:02:39  karavan
 * added display of status if search throttled back.
 *
 * Revision 1.12  1996/07/24 20:10:38  karavan
 * Fixed error in numActiveExperiments calculation; numActiveCurrentExperiments
 * now zero'd at phase boundary.
 *
 * Revision 1.11  1996/07/22 18:55:45  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.10  1996/05/15 04:35:19  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
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
  friend ostream& operator <<(ostream &os, PCsearch& srch);
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
  void decrNumActiveExperiments() {
    if (isGlobal())
      PCsearch::numActiveGlobalExperiments -= 1;
    else
      PCsearch::numActiveCurrentExperiments -= 1;
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
  schState searchStatus;  // schNeverRun/schPaused/schRunning/schEnded
  unsigned phaseToken;          // identifier for phase of this search
  phaseType phType;     // global or current; need for DM interface
  PCmetricInstServer *database;
  searchHistoryGraph *shg;
  bool isGlobal() {return (phType == GlobalPhase);}
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
  void newData (PCmetDataID, sampleValue newVal, timeStamp, timeStamp, 
	   sampleValue)
    {
	newVal = (newVal - 1)/newVal;
	if (newVal < performanceConsultant::predictedCostLimit)
	    // check search queue and expand search if possible
	      PCsearch::expandSearch(newVal);
    }
  void updateEstimatedCost(float) {;}
  void enableReply(unsigned, unsigned, unsigned, bool) {;}
  PCmetInstHandle costFilter;
};


#endif

