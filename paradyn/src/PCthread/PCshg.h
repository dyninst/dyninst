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
 * PCshg.h
 * 
 * classes searchHistoryNode, GraphNode, searchHistoryGraph
 *
 * $Log: PCshg.h,v $
 * Revision 1.33  1997/03/29 02:05:23  sec
 * Debugging stuff
 *
 * Revision 1.32  1996/12/08 17:36:22  karavan
 * part 1 of 2 part commit to add new searching functionality
 *
 * Revision 1.31  1996/08/16 21:03:46  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.30  1996/08/16 07:07:53  karavan
 * minor code cleanup
 *
 * Revision 1.29  1996/07/23 20:28:08  karavan
 * second part of two-part commit.
 *
 * implements new search strategy which retests false nodes under certain
 * circumstances.
 *
 * change in handling of high-cost nodes blocking the ready queue.
 *
 * code cleanup.
 *
 * Revision 1.28  1996/07/22 18:55:47  karavan
 * part one of two-part commit for new PC functionality of restarting searches.
 *
 * Revision 1.27  1996/05/15 04:35:25  karavan
 * bug fixes: changed pendingCost pendingSearches and numexperiments to
 * break down by phase type, so starting a new current phase updates these
 * totals correctly; fixed error in estimated cost propagation.
 *
 * Revision 1.26  1996/05/08 07:35:34  karavan
 * Changed enable data calls to be fully asynchronous within the performance consultant.
 *
 * some changes to cost handling, with additional limit on number of outstanding enable requests.
 *
 * Revision 1.25  1996/05/06 04:35:29  karavan
 * Bug fix for asynchronous predicted cost changes.
 *
 * added new function find() to template classes dictionary_hash and
 * dictionary_lite.
 *
 * changed filteredDataServer::DataFilters to dictionary_lite
 *
 * changed normalized hypotheses to use activeProcesses:cf rather than
 * activeProcesses:tlf
 *
 * code cleanup
 *
 * Revision 1.24  1996/05/02 19:46:54  karavan
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
 * Revision 1.23  1996/04/30 06:27:11  karavan
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

#ifndef pc_shg_h
#define pc_shg_h

#include "PCintern.h"

/*
NOTES:
The Search History Graph is a hierarchical directed acyclic graph.
Each node (SHGnode) represents a single experiment which is testing a
     unique hypothesis-focus pair.  We know the graph is acyclic
     because our search through the search space consists of
     incrementally refining one of the two axes, and refinement is a
     partial ordering.

*/

#include "PCwhy.h"
class PCsearch;
class experiment;

typedef enum {refineWhereAxis, refineWhyAxis} refineType;
typedef enum {expandedNone, expandedWhy, expandedWhere, expandedAll} expandStatus;

// forward declarations
class searchHistoryNode;
class searchHistoryGraph;

class searchHistoryNode {
  
public:
  searchHistoryNode(searchHistoryNode *parent, hypothesis *why, 
		    focus where, refineType axis, 
		    bool persist, searchHistoryGraph *mama, 
		    const char *shortName, unsigned newID, bool amFlag);
  searchHistoryNode *addChild (hypothesis *why, 
			       focus whereowhere, 
			       refineType axis,
			       bool persist,
			       const char *shortName,
			       unsigned newID,
			       bool amFlag);
  float getEstimatedCost(); 
  bool print (int parent, FILE *fp);
  bool getActive();
  testResult getStatus();
  bool hypoMatches (hypothesis *candidate) {return (why == candidate);}
  void addNodeToDisplay();
  void addEdgeToDisplay(unsigned parentID, const char *label);
  void changeDisplay ();
  void changeTruth (testResult newTruth);
  void expand();
  bool expandWhere();
  bool expandWhy();
  void setExpanded () {exStat = expandedAll;}
  expandPolicy getExpandPolicy() {return why->getExpandPolicy();}
  bool setupExperiment();
  void startExperiment(); 
  void stopExperiment();
  unsigned getNodeId() {return nodeID;}
  void getInfo (shg_node_info *theInfo);
  unsigned getPhase();
  const char *getShortName() {return sname.string_of();}
  const char *getHypoName() {return why->getName();}
  // const char *getFocus() {return exp->getFocus();}
  void estimatedCostNotification(); 
  void enableReply (bool);
  void addActiveSearch();
  void retestAllChildren();
  void retest();
  int getGuiToken();
private:
  void percolateUp(testResult newTruth);
  void percolateDown(testResult newTruth);
  void makeTrue();
  void makeFalse();
  void makeUnknown();
  void changeActive(bool live);
  hypothesis *why;
  focus where;
  bool persistent;
  // this set causes alternate PCmetric for hypothesis to be used when 
  // activating experiment
  bool altMetricFlag;
  experiment *exp;
  bool active;
  testResult truthValue;
  string name;
  refineType axis;
  unsigned nodeID; // used for display and for unique priority key
  expandStatus exStat;   // has this node ever been expanded in the past??
  expandPolicy exType;
  int numTrueParents;
  int numTrueChildren;
  bool virtualNode;
  vector<searchHistoryNode*> parents;
  vector<searchHistoryNode*> children;
  searchHistoryGraph *mamaGraph;
  string sname;
};

class searchHistoryGraph {
  friend class searchHistoryNode;
 public:
  searchHistoryGraph(PCsearch *searchPhase, unsigned displayToken);

  // display styles currently in use by GUI
  enum nodeDisplayStyle {
    NeverActiveUnknownNodeStyle, 
    InactiveUnknownNodeStyle,
    InactiveFalseNodeStyle,
    ActiveTrueNodeStyle,
    ActiveUnknownNodeStyle,
    ActiveFalseNodeStyle,
    InactiveTrueNodeStyle };

  void initPersistentNodes();
  searchHistoryNode *addNode (searchHistoryNode *parent,
			      hypothesis *why,
			      focus whereowhere,
			      refineType axis,
			      bool persist,
			      const char *shortName,
			      bool amFlag,
			      bool *newFlag);
  searchHistoryNode *const getNode (unsigned nodeId);
  void updateDisplayedStatus (string *newmsg);
  void updateDisplayedStatus (char *newmsg);
  void finalizeSearch(timeStamp searchEndTime);
  unsigned getPhase() {return (unsigned)guiToken;}
  void addUIrequest(unsigned srcID, unsigned dstID, int styleID, const char *label);
  void flushUIbuffer();
  void clearPendingSearch(float pcost);
  void addActiveSearch ();
 private:
  vector<searchHistoryNode*> Nodes;
  static unsigned uhash (unsigned& val) {return (unsigned) (val % 19);} 
  dictionary_hash<unsigned, searchHistoryNode*> NodeIndex;
  dictionary_hash<focus, vector<searchHistoryNode*>*> NodesByFocus;
  searchHistoryNode *root;
  PCsearch *srch;
  int guiToken;      // use in UI calls to select correct search display  
  unsigned nextID;   // used to create unique ids for the nodes
  vector<uiSHGrequest> *uiRequestBuff;
};
  
#endif
