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
 * $Id: PCshg.h,v 1.41 2002/12/20 07:50:03 jaw Exp $
 * classes searchHistoryNode, GraphNode, searchHistoryGraph
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
  friend ostream& operator <<(ostream &os, searchHistoryNode& shn);
public:
  searchHistoryNode(searchHistoryNode *parent, hypothesis *why, 
		    focus where, refineType axis, 
		    bool persist, searchHistoryGraph *mama, 
		    const char *shortName, unsigned newID, bool amFlag,
		    bool alreadyNarrowed);
  searchHistoryNode(searchHistoryNode *parent,
		    hypothesis *why, 
		    focus whereowhere, 
		    refineType axis,
		    bool persist,
		    searchHistoryGraph *mama,
		    const char *shortName,
		    unsigned newID,
		    bool amFlag,
		    int csp,
		    pdvector<bool> as,
		    bool alreadyNarrowed);
  searchHistoryNode *addChild (hypothesis *why, 
			       focus whereowhere, 
			       refineType axis,
			       bool persist,
			       const char *shortName,
			       unsigned newID,
			       bool amFlag);

  searchHistoryNode *addDynamicChild(resourceHandle child);

  float getEstimatedCost();
  bool print (int parent, FILE *fp);
  bool getActive(){return active;};
  testResult getStatus();
  bool hypoMatches (hypothesis *candidate) {return (why == candidate);}
  void addNodeToDisplay();
  void addEdgeToDisplay(unsigned parentID, const char *label);
  void changeDisplay ();
  void changeTruth (testResult newTruth);
  void expand();
  bool expandWhere();
  bool expandWhereOldPC();
  bool expandWhereCallGraphSearch();
  bool expandWhereNarrow();
  bool expandWhereWide();
  void expandWhereDownNewPath();
  bool expandWhy();
  void setExpanded () {exStat = expandedAll;}
  expandPolicy getExpandPolicy() {return why->getExpandPolicy();}
  expandStatus getExpandStatus() { return exStat; }
  bool setupExperiment();
  void startExperiment(); 
  void stopExperiment();
  unsigned getNodeId() {return nodeID;}
  void getInfo (shg_node_info *theInfo);
  unsigned getPhase();
  const char *getShortName() {return sname.c_str();}
  const char *getHypoName() {return why->getName();}
  focus getWhere() {return where;}
  void estimatedCostNotification(); 
  void enableReply (bool);
  void addActiveSearch();
  void retestAllChildren();
  void retest();
  int getGuiToken();
  bool isFalse(){ return (truthValue == tfalse);}
  void setOriginalParent(searchHistoryNode* orig) { originalParent = orig; }
private:
  void percolateUp(testResult newTruth);
  void percolateDown(testResult newTruth);
  void makeTrue();
  void makeFalse();
  void makeUnknown();
  void notifyParentAboutFalseChild();
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
  
  //originalParent designates the true parent of every node. Many nodes
  //only have one parent, so in this case originalParent is the same as 
  //the lone entry in the "parents" pdvector. In the case of shadow nodes,
  //which have multiple parents, originalParent refers to the original parent 
  //of the shadow node.
  //This variable is necessary for the new PC, where SHG nodes notify 
  //their original parent when they become false, so that the parent node
  //may continue its search down a different path of the resource hierarchy
  //when it no longer has any expanded children.
  searchHistoryNode* originalParent;

  pdvector<searchHistoryNode*> parents;
  pdvector<searchHistoryNode*> children;
  searchHistoryGraph *mamaGraph;
  
  string sname;

  //Index into searchPaths
  unsigned currentSearchPath;
  pdvector<bool> alreadySearched; //one entry for each part of focus
  bool narrowedSearch;
  
  int numExpandedChildren;
};

ostream& operator <<(ostream &os, searchHistoryNode& shn);

class searchHistoryGraph {
  friend class searchHistoryNode;
  friend ostream& operator <<(ostream &os, searchHistoryGraph& shg);
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
  void finalizeSearch(relTimeStamp searchEndTime);
  unsigned getPhase() {return (unsigned)guiToken;}
  void addUIrequest(unsigned srcID, unsigned dstID, int styleID, const char *label);
  void flushUIbuffer();
  void clearPendingSearch(float pcost);
  void addActiveSearch ();
  
  //Notify the PC that a new dynamic child has been added to the
  //call graph, which means that the PC may want to include the child
  // as part of a search.
  void notifyDynamicChild(resourceHandle parent, 
			  resourceHandle child);
  
 private:
  pdvector<searchHistoryNode*> Nodes;
  static unsigned uhash (const unsigned& val) {return (unsigned) (val % 19);} 
  dictionary_hash<unsigned, searchHistoryNode*> NodeIndex;
  dictionary_hash<focus, pdvector<searchHistoryNode*>*> NodesByFocus;
  searchHistoryNode *root;
  PCsearch *srch;
  int guiToken;      // use in UI calls to select correct search display  
  unsigned nextID;   // used to create unique ids for the nodes
  pdvector<uiSHGrequest> *uiRequestBuff;
};
  
ostream& operator <<(ostream &os, searchHistoryGraph& shg);

#endif
