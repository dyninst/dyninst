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
 * The searchHistoryNode and searchHistoryGraph class methods.
 * $Id: PCshg.C,v 1.57 1999/05/19 07:50:26 karavan Exp $
 */

#include "PCintern.h"
#include "PCshg.h"
#include "PCexperiment.h"
#include "PCsearch.h"

//
// ****** searchHistoryNode *******
//

// note: searchHistoryNodes never die.

ostream& operator <<(ostream &os, searchHistoryNode& shn)
{
  os << "*** search history node ***" << endl;
  os << shn.nodeID << "  #nodeID" << endl;
  os << shn.name << endl;
  if (shn.exp) {
    os << "1  # experiment defined?" << endl;
    os << *(shn.exp) <<  endl;
  } else {
    os << "0  # experiment defined?" << endl;
  }
  int numKids = shn.children.size();
  for (int i = 0; i < numKids; i++) {
    os << *(shn.children[i]) ;
  }
  return os;
}

searchHistoryNode::searchHistoryNode(searchHistoryNode *parent,
				     hypothesis *why, 
				     focus whereowhere, 
				     refineType axis,
				     bool persist,
				     searchHistoryGraph *mama,
				     const char *shortName,
				     unsigned newID,
				     bool amFlag):
why(why), where(whereowhere), 
persistent(persist), altMetricFlag(amFlag), 
exp(NULL), active(false), truthValue(tunknown), 
axis(axis), nodeID(newID), exStat(expandedNone),  
mamaGraph (mama), sname(shortName)
{
  // full name of node (both why and where)
  name = why->getName();
  name += "::";
  name += dataMgr->getFocusNameFromHandle(where);

  // short label for node display
  if (sname == NULL)
    sname = why->getName();

  if (parent != NULL)
    parents += parent;
  virtualNode = why->isVirtual();

  // at this point, we know the parent is either the topLevelHypothesis or 
  // it is some other true node
  numTrueParents = 1;
  numTrueChildren = 0;
}

searchHistoryNode*
searchHistoryNode::addChild (hypothesis *why, 
			     focus whereowhere, 
			     refineType axis,
			     bool persist,
			     const char *shortName,
			     unsigned newID,
			     bool amFlag)
{
  searchHistoryNode *newkid = new searchHistoryNode (this, why, whereowhere, axis, 
				 persist, mamaGraph, shortName, newID, amFlag);
  children += newkid;
  return newkid;
}

bool 
searchHistoryNode::setupExperiment()
{
  assert (exp == NULL);
  bool errFlag;
  if (virtualNode) {
    // virtual nodes have no experiments
    return false;
  }
  exp = new experiment (why, where, persistent, this, 
			mamaGraph->srch, altMetricFlag, &errFlag);
  if (!exp || errFlag)
    return false;
  exp->findOutCost();
  return true;
}

void
searchHistoryNode::startExperiment()
{
  assert (exp);
  if (!active && (numTrueParents >= 1)) 
    // check here for true path to root; parent status may have changed
    // while this node was waiting on the Ready Queue
    exp->start();
}

void 
searchHistoryNode::enableReply (bool successful)
{
  if (successful) {
    changeActive(true);
#ifdef PCDEBUG
    cout << "experiment started for node: " << nodeID << endl;
#endif
  } else {
#ifdef PCDEBUG
    cout << "unable to start experiment for node: " << nodeID << endl;
#endif
  }
  float mycost = getEstimatedCost();
  mamaGraph->clearPendingSearch(mycost);
}

void 
searchHistoryNode::stopExperiment()
{
  assert (exp);
  exp->halt();
  changeActive(false);
}

void 
searchHistoryNode::addNodeToDisplay() 
{
  uiMgr->DAGaddNode (mamaGraph->guiToken, nodeID, 
		     searchHistoryGraph::InactiveUnknownNodeStyle, 
		     sname.string_of(), name.string_of(), 0);
}

void 
searchHistoryNode::addEdgeToDisplay(unsigned parentID, const char *label)
{
    uiMgr->DAGaddEdge (mamaGraph->guiToken, parentID, nodeID, (unsigned)axis, label);
}

void 
searchHistoryNode::changeDisplay()
{
  if (active) {
    switch (truthValue) {
    case ttrue:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::ActiveTrueNodeStyle);
      break;
    case tfalse:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::ActiveFalseNodeStyle);
      break;
    case tunknown:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::ActiveUnknownNodeStyle);
      break;
    };
  } else {
    switch (truthValue) {
    case ttrue:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::InactiveTrueNodeStyle);
      break;
    case tfalse:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::InactiveFalseNodeStyle);
      break;
    case tunknown:
      uiMgr->DAGconfigNode (mamaGraph->guiToken, nodeID, 
			    searchHistoryGraph::InactiveUnknownNodeStyle);
      break;
    };
  }
}

ostream& operator <<(ostream &os, searchHistoryGraph& shg)
{
  os << '\n' << "# " << shg.guiToken << 
    " Performance Consultant Search History Graph"  << endl;
  os << *(shg.root) <<  endl;
  return os;
}

void 
searchHistoryGraph::addUIrequest(unsigned srcID, unsigned dstID, 
				 int styleID, const char *label)
{
  if (uiRequestBuff == NULL)
    uiRequestBuff = new vector<uiSHGrequest>;
  uiSHGrequest newGuy;
  newGuy.srcNodeID = srcID;
  newGuy.dstNodeID = dstID;
  newGuy.styleID = styleID;
  newGuy.label = label;
  (*uiRequestBuff) += newGuy;
}

void
searchHistoryGraph::flushUIbuffer()
{
  if (uiRequestBuff) {
    unsigned bufSize = uiRequestBuff->size();
    if (!bufSize) return; // avoid sending empty buffer
    uiMgr->DAGaddBatchOfEdges(guiToken, uiRequestBuff, bufSize);
    uiRequestBuff = 0;
  }
}

void 
searchHistoryGraph::clearPendingSearch(float pcost) {
  if (guiToken == 0) {
    PCsearch::decrNumPendingGlobalSearches();
    PCsearch::clearPendingGlobalCost(pcost);
  }
  else {
    PCsearch::decrNumPendingCurrentSearches();
    PCsearch::clearPendingCurrentCost(pcost);
  }
}

void 
searchHistoryNode::addActiveSearch() 
{
  mamaGraph->addActiveSearch();
}

void 
searchHistoryGraph::addActiveSearch ()
{
  if (guiToken == 0)
    PCsearch::addGlobalActiveExperiment();
  else
    PCsearch::addCurrentActiveExperiment();
}

bool
searchHistoryNode::expandWhere()
{
  searchHistoryNode *curr;
  bool newNodeFlag;
  bool altFlag;
  bool expansionPossible = false;

  // expand along where axis
  vector<resourceHandle> *parentFocus = dataMgr->getResourceHandles(where);
  vector<resourceHandle> *currFocus;
  resourceHandle currHandle;
  vector<rlNameId> *kids;
  for (unsigned m = 0; m < parentFocus->size(); m++) {
    currHandle = (*parentFocus)[m];
    altFlag = (currHandle == Processes);
    if (altFlag && (where == topLevelFocus))
      // it is never useful to refine along process from the top level
      continue;
    if (!why->isPruned(currHandle)) {
      // prunes limit the resource trees along which we will expand this node
      kids = dataMgr->magnify(currHandle, where);
      if ( (kids != NULL) && (kids->size() >= 1)) {
	// note we don't bother refining if there's only a single child, 
	// because the child would trivially test true.
	for (unsigned j = 0; j < kids->size(); j++) {
	  // eliminate all resources explicitly pruned for this hypothesis
	  currFocus = dataMgr->getResourceHandles((*kids)[j].id);
	  bool suppressFound = false;;
	  for (unsigned n = 0; n < currFocus->size(); n++) {
	    if (why->isSuppressed((*currFocus)[n])) {
	      suppressFound = true;
	      break;
	    }
	  }
	  if (!suppressFound) {
	    expansionPossible = true;
	    curr = mamaGraph->addNode (this, why, (*kids)[j].id, 
				       refineWhereAxis,
				       false,  
				       (*kids)[j].res_name,
				       (altFlag || altMetricFlag),
				       &newNodeFlag);
	    if (newNodeFlag) {
	      // a new node was added
	      curr->addNodeToDisplay(); 
	      mamaGraph->addUIrequest(nodeID,   // parent ID
				      curr->getNodeId(),  // child ID
				      (unsigned)refineWhereAxis, // edge style
				      (char *)NULL);
	    } else {
	      // shadow node
	      mamaGraph->addUIrequest(nodeID,
				      curr->getNodeId(),
				      (unsigned)refineWhereAxis,
				      (*kids)[j].res_name);
	    }
	  }
	}
	delete kids;
      }
    }
  }
  delete parentFocus;
  return expansionPossible;
}

bool
searchHistoryNode::expandWhy()
{
  searchHistoryNode *curr;
  bool newNodeFlag;
  bool expansionPossible = false;

  // expand along why axis
  vector<hypothesis*> *hypokids = why->expand();
  if (hypokids != NULL) { 
    expansionPossible = true;
    for (unsigned i = 0; i < hypokids->size(); i++) {
      curr = mamaGraph->addNode (this, (*hypokids)[i], where,
				 refineWhyAxis, 
				 false,  
				 (*hypokids)[i]->getName(),
				 altMetricFlag,
				 &newNodeFlag);
      if (newNodeFlag) {
	// a new node was added
	curr->addNodeToDisplay();
	mamaGraph->addUIrequest(nodeID,   // parent ID
				curr->getNodeId(),  // child ID
				(unsigned)refineWhereAxis, // edge style
				(char *)NULL);
      } else {
	// shadow node
	mamaGraph->addUIrequest(nodeID,
				curr->getNodeId(),
				(unsigned)refineWhereAxis,
				(*hypokids)[i]->getName());
      }
    }
    delete hypokids;
  }
  return expansionPossible;
}

void
searchHistoryNode::expand ()
{
  assert (children.size() == 0);
  assert (exStat != expandedAll);

#ifdef PCDEBUG
  // debug print
  if (performanceConsultant::printSearchChanges) {
    cout << "EXPAND: why=" << why->getName() << endl
      << "        foc=<" << dataMgr->getFocusNameFromHandle(where) 
	<< ">" << endl
	  << "       time=" << exp->getEndTime() << endl;
  }
#endif
  exStat = expandedAll;
  switch (getExpandPolicy()) {
  case whyAndWhere:
    expandWhy();
    expandWhere();
    break;
  case whereAndWhy:
    expandWhere();
    expandWhy();
    break;
  case whyBeforeWhere:
    if ( expandWhy() == false)
      expandWhere();
    break;
  case whereBeforeWhy:
    if (expandWhere() == false)
      expandWhy();
    break;
  case whyOnly:
    expandWhy();
    break;
  case whereOnly:
    expandWhere();
    break;
  }  // end switch
  
  mamaGraph->flushUIbuffer();
}

void
searchHistoryNode::makeTrue()
{
  truthValue = ttrue;
  changeDisplay();
  // update Search Display Status Area (eventually this will be printed 
  // some better way, but this will have to do...)
  string *status = new string (why->getName());
  *status += string(" tested true for ");
  *status += string(dataMgr->getFocusNameFromHandle(where));
  *status += string("\n");
  mamaGraph->updateDisplayedStatus (status);
}

void
searchHistoryNode::makeFalse()
{
  truthValue = tfalse;
  // if this node contains an experiment (ie, its non-virtual) then 
  // we want to halt the experiment for now
  if (!persistent) stopExperiment();
  // change truth value and/or active status on the GUI
  changeDisplay();
}

void
searchHistoryNode::makeUnknown()
{
  truthValue = tunknown;
  changeDisplay();
}

void
searchHistoryNode::changeActive (bool live)
{
  active = live;
  changeDisplay();
}


void 
searchHistoryNode::percolateDown(testResult newValue)
{
  if (newValue == ttrue) {
    // one of one or more parents just changed to true
    numTrueParents++;
    if (numTrueParents == 1) {
      if ((exp) && (!active)) {
	//** this will be replaced with more rational priority calculation
	if (axis == refineWhyAxis)
	  PCsearch::addToQueue(5, this, getPhase());
	else
	  PCsearch::addToQueue(10, this, getPhase());
      }
    }
  } else {
    // we're percolating a change to false; if no true parents are 
    // left, this node is false by definition, we stop the experiment.
    // Eventually the child node would become false on its own, this
    // is just a shortcut.
    numTrueParents--;
    if (numTrueParents == 0)
      changeTruth(tfalse); 
  }
}
      
void
searchHistoryNode::percolateUp(testResult newValue)
{
  if (newValue == ttrue) {    
    numTrueChildren++;
    if ((virtualNode) && (numTrueChildren == 1)) {
      // virtual Node represents or, one's enough
      makeTrue();
      for (unsigned i = 0; i < parents.size(); i++) {
	parents[i] -> percolateUp(newValue);
      }
    }
  } else {
    numTrueChildren--;
    if ((virtualNode) && (numTrueChildren == 0)) {
      changeTruth(tfalse);
    }
  }
}

//
// experiment reports a change in truth value
//
void 
searchHistoryNode::changeTruth (testResult newTruth)
{
  testResult oldValue = truthValue;
  if (newTruth == tfalse)  {
    // status change to false
    this->makeFalse();
    if (oldValue == ttrue) {
      // change to false was from true
      for (unsigned i = 0; i < parents.size(); i++) {
	parents[i] -> percolateUp(tfalse);
      }
      for (unsigned k = 0; k < children.size(); k++) {
	children[k]->percolateDown(tfalse);
      }
    }
  } else {
    if (oldValue == newTruth) {
      // oops!  this isn't really a change!
      return;
    }
    if (newTruth == ttrue) {
      // status change to true
      this->makeTrue();
      for (unsigned i = 0; i < parents.size(); i++) {
	parents[i] -> percolateUp(ttrue);
      }
      if (exStat != expandedNone) {
	for (unsigned k = 0; k < children.size(); k++) {
	  children[k]->percolateDown(ttrue);
	}
      } else {
	expand();
      }
    } else {
      // change to unknown
      makeUnknown();
      changeDisplay ();
    }
  }
}

//
// move a node which has been tested back onto the ready q
//
void 
searchHistoryNode::retest()
{
  if ((exp) && (!active)) {
    //** this will be replaced with more rational priority calculation
    if (axis == refineWhyAxis)
      PCsearch::addToQueue(5, this, getPhase());
    else
      PCsearch::addToQueue(10, this, getPhase());
  }
}

void
searchHistoryNode::retestAllChildren()
{
  if (exStat != expandedNone) {
    for (unsigned k = 0; k < children.size(); k++) {
      children[k]->retest();
    }
  }
}
  
float 
searchHistoryNode::getEstimatedCost()
{
  return exp->getEstimatedCost();
}

unsigned 
searchHistoryNode::getPhase() {
  return (mamaGraph->getPhase());
}

void 
searchHistoryNode::getInfo (shg_node_info *theInfo)
{
  if (exp != NULL) {
    theInfo->active = active;
    theInfo->currentConclusion = exp->getCurrentConclusion();
    theInfo->timeTrueFalse = exp->getTimeTrueFalse();
    theInfo->currentValue = exp->getCurrentValue();
    theInfo->adjustedValue = exp->getAdjustedValue();
    theInfo->lastThreshold = exp->getLastThreshold();
    theInfo->hysConstant = exp->getHysConstant();
    theInfo->startTime = exp->getStartTime();
    theInfo->endTime = exp->getEndTime();
    theInfo->estimatedCost = exp->getEstimatedCost();
    theInfo->persistent = persistent;
  } else {
    //** this will change with split into virtual nodes
    theInfo->active = false;
    theInfo->currentConclusion = tunknown;
    theInfo->timeTrueFalse = 0;
    theInfo->currentValue = 0.0;
    theInfo->adjustedValue = 0.0;
    theInfo->lastThreshold = 0.0;
    theInfo->hysConstant = 0.0;
    theInfo->startTime = 0;
    theInfo->endTime = 0;
    theInfo->estimatedCost = 0.0;
    theInfo->persistent = true;
  }
}

void 
searchHistoryNode::estimatedCostNotification() 
{
#ifdef PCDEBUG
  cout << "Cost Received for Node " << name << endl;
  cout << " in phase " << mamaGraph->guiToken << " is " << 
    exp->getEstimatedCost() << endl;
#endif
  if (!active && numTrueParents >= 1) {
    // check numTrueParents here because parent may have become false
    // while cost request was pending
    //** this will be replaced with more rational priority calculation
    if (axis == refineWhyAxis)
      PCsearch::addToQueue(5, this, getPhase());
    else
      PCsearch::addToQueue(10, this, getPhase());
  }
}

int 
searchHistoryNode::getGuiToken() 
{
  return mamaGraph->guiToken;
}
  
//
//  ******  searchHistoryGraph ********
//

// searchHistoryGraphs never die.

searchHistoryGraph::searchHistoryGraph(PCsearch *searchPhase, 
				       unsigned phaseToken)
:
 NodeIndex(searchHistoryGraph::uhash),
 NodesByFocus(searchHistoryGraph::uhash),
 srch(searchPhase), 
 guiToken(phaseToken),
 nextID(0),
 uiRequestBuff(NULL) 
{
  vector<searchHistoryNode*> Nodes;
  root = new searchHistoryNode ((searchHistoryNode *)NULL,
				topLevelHypothesis,
				topLevelFocus, refineWhyAxis,
				true, this, "TopLevelHypothesis", nextID, 
				false);
  root->setExpanded();
  Nodes += root;
  NodeIndex[nextID] = root;
  nextID++;
}

void 
searchHistoryGraph::updateDisplayedStatus (char *newmsg)
{
  string *msgStr = new string (newmsg); // UI will call 'delete' on this
  uiMgr->updateStatusDisplay(guiToken, msgStr);
}
  
void 
searchHistoryGraph::updateDisplayedStatus (string *newmsg)
{
  // note: UI will call 'delete' on newmsg
  uiMgr->updateStatusDisplay(guiToken, newmsg);
}

//
// Any cleanup associated with search termination.
//
void 
searchHistoryGraph::finalizeSearch(timeStamp searchEndTime)
{
  // right now search only terminates if phase ends, so just
  // update Search Display Status Area (eventually this will be printed 
  // some better way, but this will have to do...)
  string *status = new string("\nSearch for Phase "); 
  *status += string(performanceConsultant::DMcurrentPhaseToken);
  *status += string (" ended due to end of phase at time ");
  *status += string (searchEndTime);
  *status += string(".\n");
  updateDisplayedStatus(status);
  // change display of all nodes to indicate "inactive"; no 
  // change to truth value displayed
  uiMgr->DAGinactivateEntireSearch(guiToken);
}

searchHistoryNode* 
searchHistoryGraph::addNode (searchHistoryNode *parent,
			     hypothesis *why,
			     focus whereowhere,
			     refineType axis,
			     bool persist,
			     const char *shortName,
			     bool amFlag,
			     bool *newFlag)
{
  // check if node already exists
  searchHistoryNode *newkid = NULL;
  vector<searchHistoryNode*> *foclist = NULL;
  *newFlag = false;
  if (NodesByFocus.defines(whereowhere)) {
    foclist = NodesByFocus[whereowhere];
    for (unsigned i = 0; i < foclist->size(); i++) {
      if ((*foclist)[i]->hypoMatches(why)) {
	newkid = (*foclist)[i];
	return newkid;
      }
    }
  }
  *newFlag = true;
  if (foclist == NULL) {
    foclist = new vector<searchHistoryNode*>;
    NodesByFocus[whereowhere] = foclist;
  }
  newkid = parent->addChild (why, whereowhere, axis, persist, 
			     shortName, nextID++, amFlag);
  *foclist += newkid;
  newkid->setupExperiment();
  Nodes += newkid;
  NodeIndex [(newkid->getNodeId())] = newkid;
  return newkid;
}
  
searchHistoryNode *const
searchHistoryGraph::getNode (unsigned nodeId)
{
  return NodeIndex [nodeId];
}

// for each hypothesis on the why axis, create a search node with 
// that hypothesis at the top level focus.
//
void 
searchHistoryGraph::initPersistentNodes()
{
  searchHistoryNode *nodeptr;
  hypothesis *currhypo;
  vector<hypothesis*> *topmost = topLevelHypothesis->expand();
  bool nodeAdded;
  for (unsigned i = 0; i < topmost->size(); i++) {
    currhypo = (*topmost)[i];
    nodeptr = addNode (root, currhypo, topLevelFocus, 
		       refineWhyAxis, true,  
		       currhypo->getName(),
		       false,
		       &nodeAdded);
    nodeptr->addNodeToDisplay();
    nodeptr->addEdgeToDisplay(root->getNodeId(), (char *)NULL);
  }
  delete topmost;
}

