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
		    const char *shortName);
  float getEstimatedCost(); 
  bool print (int parent, FILE *fp);
  bool getActive();
  testResult getStatus();
  bool hypoMatches (hypothesis *candidate) {return (why == candidate);}
  void addToDisplay(unsigned parentID, const char *label, bool edgeOnlyFlag);
  void changeDisplay ();
  void changeTruth (testResult newTruth);
  void expand();
  bool alreadyExpanded() {return expanded;}
  bool setupExperiment();
  bool startExperiment(); 
  void stopExperiment();
  unsigned getNodeId() {return nodeID;}
  void getInfo (shg_node_info *theInfo);
  void setExpanded () {expanded = true;}
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
  experiment *exp;
  bool active;
  testResult truthValue;
  string name;
  refineType axis;
  unsigned nodeID; // used for display and for unique priority key
  static unsigned nextID;
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
  void initPersistentNodes();
  searchHistoryNode *addNode (searchHistoryNode *parent,
			      hypothesis *why,
			      focus whereowhere,
			      refineType axis,
			      bool persist,
			      const char *shortName);
  searchHistoryNode *const getNode (unsigned nodeId);
  void setSearchUpdateNeeded ();
 private:
  vector<searchHistoryNode*> Nodes;
  static unsigned uhash (unsigned& val) {return (unsigned) (val % 20);} 
  dictionary_hash<unsigned, searchHistoryNode*> NodeIndex;
  dictionary_hash<focus, vector<searchHistoryNode*>*> NodesByFocus;
  searchHistoryNode *root;
  PCsearch *srch;
  int guiToken;
};
  
#endif
