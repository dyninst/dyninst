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
 * PCshg.h
 * 
 * classes searchHistoryNode, GraphNode, searchHistoryGraph
 *
 * $Log: PCshg.h,v $
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
 * Revision 1.22  1996/04/18 20:43:19  tamches
 * uiRequestBuff no longer a pointer; numUIrequests no longer needed
 *
 * Revision 1.21  1996/04/16 18:36:14  karavan
 * BUG FIX.
 *
 * Revision 1.20  1996/04/14 03:21:16  karavan
 * bug fix:  added size member to shg class for use in UI batching.
 *
 * Revision 1.19  1996/04/13 04:42:32  karavan
 * better implementation of batching for new edge requests to UI shg display
 *
 * changed type returned from datamgr->magnify and datamgr->magnify2
 *
 * Revision 1.18  1996/04/07 21:29:46  karavan
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
 * Revision 1.17  1996/03/18 07:13:11  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.16  1996/02/22 18:28:43  karavan
 * changed debug print calls from dataMgr->getFocusName to
 * dataMgr->getFocusNameFromHandle
 *
 * changed GUI node styles from #defines to enum
 *
 * added searchHistoryGraph::updateDisplayedStatus()
 *
 * Revision 1.15  1996/02/09 05:31:43  karavan
 * changes to support multiple per-phase searches
 *
 * added true full name for search nodes.
 *
 * Revision 1.14  1996/02/02 02:07:35  karavan
 * A baby Performance Consultant is born!
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
// forward declarations
class searchHistoryNode;
class searchHistoryGraph;

class searchHistoryNode {
friend void defaultExplanation(searchHistoryNode *explainee);
  
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
  bool alreadyExpanded() {return expanded;}
  bool setupExperiment();
  void startExperiment(); 
  void stopExperiment();
  unsigned getNodeId() {return nodeID;}
  void getInfo (shg_node_info *theInfo);
  void setExpanded () {expanded = true;}
  unsigned getPhase();
  const char *getShortName() {return sname.string_of();}
  const char *getHypoName() {return why->getName();}
  //const char *getFocus() {return exp->getFocus();}
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
  bool expanded;   // has this node ever been expanded in the past??
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
