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
 * PCshg.C
 * 
 * The searchHistoryNode and searchHistoryGraph class methods.
 * 
 * $Log: PCshg.C,v $
 * Revision 1.38  1996/04/14 03:21:13  karavan
 * bug fix:  added size member to shg class for use in UI batching.
 *
 * Revision 1.37  1996/04/13 04:42:30  karavan
 * better implementation of batching for new edge requests to UI shg display
 *
 * changed type returned from datamgr->magnify and datamgr->magnify2
 *
 * Revision 1.36  1996/04/09 19:25:57  karavan
 * added batch mode for adding a group of new nodes and edges to SHG display.
 *
 * Revision 1.35  1996/04/07 21:29:45  karavan
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
 * Revision 1.34  1996/03/18 07:13:09  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.33  1996/02/22 18:28:38  karavan
 * changed debug print calls from dataMgr->getFocusName to
 * dataMgr->getFocusNameFromHandle
 *
 * changed GUI node styles from #defines to enum
 *
 * added searchHistoryGraph::updateDisplayedStatus()
 *
 * Revision 1.32  1996/02/15 23:26:23  tamches
 * WHYEDGESTYLE to (unsigned)axis in refinement parameter when calling
 * uiMgr->DAGaddEdge.
 *
 * Revision 1.31  1996/02/09 05:31:40  karavan
 * changes to support multiple per-phase searches
 *
 * added true full name for search nodes.
 *
 * Revision 1.30  1996/02/08 19:52:50  karavan
 * changed performance consultant's use of tunable constants:  added 3 new
 * user-level TC's, PC_CPUThreshold, PC_IOThreshold, PC_SyncThreshold, which
 * are used for all hypotheses for the respective categories.  Also added
 * PC_useIndividualThresholds, which switches thresholds back to use hypothesis-
 * specific, rather than categorical, thresholds.
 *
 * Moved all TC initialization to PCconstants.C.
 *
 * Switched over to callbacks for TC value updates.
 *
 * Revision 1.29  1996/02/02 02:06:49  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCintern.h"
#include "PCshg.h"
#include "PCexperiment.h"
#include "PCsearch.h"

//
// default explanation functions
//
void defaultExplanation(searchHistoryNode *explainee)
{
//    ostrstream status;

//    if (explainee && explainee->why) {
//      status << "hypothesis: "<< explainee->why->name << " true for";
//    } else {
//      status << "***** NO HYPOTHESIS *******\n";
//    }
//    status << explainee->where << " at time ***" << "\n";
//    uiMgr->updateStatusDisplay (mamaGraph->guiToken, status.str());
//    delete (status.str());
  cout << "defaultExplanation" << endl;
}

//
// ****** searchHistoryNode *******
//

// searchHistoryNodes never die.

searchHistoryNode::searchHistoryNode(searchHistoryNode *parent,
				     hypothesis *why, 
				     focus whereowhere, 
				     refineType axis,
				     bool persist,
				     searchHistoryGraph *mama,
				     const char *shortName,
				     unsigned newID):
why(why), where(whereowhere), 
persistent(persist), exp(NULL), active(false), truthValue(tunknown), 
axis(axis), nodeID(newID), expanded(false),  
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
			     unsigned newID)
{
  searchHistoryNode *newkid = new searchHistoryNode (this, why, whereowhere, axis, 
				 persist, mamaGraph, shortName, newID);
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
			mamaGraph->srch, &errFlag);
  return  (! ((exp == NULL) || errFlag));
}

bool 
searchHistoryNode::startExperiment()
{
  assert (exp);
  if (active) return false;
  // check here for true path to root; parent status may have changed
  // while this node was waiting on the Ready Queue
  if (numTrueParents < 1) return false;
  if (exp->start()) {
    changeActive(true);
    PCsearch::incrNumActiveExperiments();
    return true;
  } else {
    return false;
  }
}

void 
searchHistoryNode::stopExperiment()
{
  assert (exp);
  exp->halt();
  changeActive(false);
  PCsearch::decrNumActiveExperiments();
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

void 
searchHistoryGraph::addUIrequest(unsigned srcID, unsigned dstID, 
				 int styleID, const char *label)
{
  if (numUIrequests == 0) {
    uiRequestBuffSize = 10;
    uiRequestBuff = new vector<uiSHGrequest> (uiRequestBuffSize);
    numUIrequests = 0;
  }
  ((*uiRequestBuff)[numUIrequests]).srcNodeID = srcID;
  (*uiRequestBuff)[numUIrequests].dstNodeID = dstID;
  (*uiRequestBuff)[numUIrequests].styleID = styleID;
  (*uiRequestBuff)[numUIrequests].label = label;
  numUIrequests++;
  if (numUIrequests == uiRequestBuffSize) {
    uiRequestBuffSize *= 2;
    uiRequestBuff->resize (uiRequestBuffSize);
  }
}

void
searchHistoryGraph::flushUIbuffer()
{
  uiMgr->DAGaddBatchOfEdges(guiToken, uiRequestBuff, numUIrequests);
  numUIrequests = 0;
}

void
searchHistoryNode::expand ()
{
  assert (children.size() == 0);
#ifdef PCDEBUG
  // debug print
  if (performanceConsultant::printSearchChanges) {
    cout << "EXPAND: why=" << why->getName() << endl
      << "        foc=<" << dataMgr->getFocusNameFromHandle(where) 
	<< ">" << endl
	  << "       time=" << exp->getEndTime() << endl;
  }
#endif
  expanded = true;
  searchHistoryNode *curr;
  bool newNodeFlag;
  
  // first expand along where axis
  if (why->prunesDefined()) {
  // prunes limit the resource trees along which we will expand this node
    vector<resourceHandle> *parentFocus = dataMgr->getResourceHandles(where);
    resourceHandle currHandle;
    vector<rlNameId> *kids;
    for (unsigned m = 0; m < parentFocus->size(); m++) {
      currHandle = (*parentFocus)[m];
      if (!why->isPruned(currHandle)) {
	kids = dataMgr->magnify(currHandle, where);
	if (kids != NULL) {
	  for (unsigned j = 0; j < kids->size(); j++) {
	    curr = mamaGraph->addNode (this, why, (*kids)[j].id, 
				       refineWhereAxis,
				       false,  
				       (*kids)[j].res_name,
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
	  delete kids;
	}
      }
    }
    delete parentFocus;
  } else {
    // no prunes defined for this hypothesis so we can expand fully
    vector<rlNameId> *kids = dataMgr->magnify2(where);
    if (kids != NULL) {
      for (unsigned k = 0; k < kids->size(); k++) {
	curr = mamaGraph->addNode (this, why, (*kids)[k].id, 
				   refineWhereAxis,
				   false,  
				   (*kids)[k].res_name,
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
				  (*kids)[k].res_name);
	}
      }
      delete kids;
    }
  }
  // second expand along why axis
  vector<hypothesis*> *hypokids = why->expand();
  if (hypokids != NULL) { 
    for (unsigned i = 0; i < hypokids->size(); i++) {
      curr = mamaGraph->addNode (this, (*hypokids)[i], where,
				 refineWhyAxis, 
				 false,  
				 (*hypokids)[i]->getName(),
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
  mamaGraph->flushUIbuffer();

#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges) {
    PCsearch::printQueue(getPhase());
  }
#endif
}

void
searchHistoryNode::makeTrue()
{
  truthValue = ttrue;
  changeDisplay();
  // update Search Display Status Area (eventually this will be printed 
  // some better way, but this will have to do...)
  string status = why->getName();
  status += " tested true for ";
  const char *fname = dataMgr->getFocusNameFromHandle(where);
  status += fname;
  status += "\n";
  mamaGraph->updateDisplayedStatus (status);
}

void
searchHistoryNode::makeFalse()
{
  truthValue = tfalse;
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
	//** get key
	PCsearch::addToQueue(10, this, getPhase());
      }
    }
  } else {
    // we're percolating a change to false; if no true parents are 
    // left, we stop the experiment.
    // this case shouldn't ever happen, IMHO - klk
    numTrueParents--;
    if (numTrueParents == 0) {
      if (exp) {
	stopExperiment();
      }
      for (unsigned k = 0; k < children.size(); k++)
	children[k]->percolateDown(newValue);
    }
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
      makeFalse();
    }
  }
}

//
// experiment reports a change in truth value
//
void 
searchHistoryNode::changeTruth (testResult newTruth)
{
  if (truthValue == newTruth) {
    // oops!  this isn't really a change!
    return;
  }
  if (newTruth == tfalse)  {
    // status change to false
    // if this node contains an experiment (ie, its non-virtual) then 
    // we want to halt the experiment for now
    if (!persistent) {
      stopExperiment();
      //** add to aging queue
    }
    if (truthValue == tunknown) {
      this->makeFalse();
    } else {
      // change to false was from true
      this->makeFalse();
      for (unsigned i = 0; i < parents.size(); i++) {
	parents[i] -> percolateUp(tfalse);
      }
      for (unsigned k = 0; k < children.size(); k++) {
	children[k]->percolateDown(tfalse);
      }
    }
  } else if (newTruth == ttrue) {
    // status change to true
    this->makeTrue();
    for (unsigned i = 0; i < parents.size(); i++) {
      parents[i] -> percolateUp(ttrue);
    }
    if (expanded) {
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

float 
searchHistoryNode::getEstimatedCost()
{
  return exp->getEstimatedCost();
}

unsigned 
searchHistoryNode::getPhase() {
  return mamaGraph->getPhase();
}

void 
searchHistoryNode::getInfo (shg_node_info *theInfo)
{
  if (exp != NULL) {
    theInfo->currentConclusion = exp->getCurrentConclusion();
    theInfo->timeTrueFalse = exp->getTimeTrueFalse();
    theInfo->currentValue = exp->getCurrentValue();
    theInfo->startTime = exp->getStartTime();
    theInfo->endTime = exp->getEndTime();
    theInfo->estimatedCost = exp->getEstimatedCost();
    theInfo->persistent = persistent;
  } else {
    //** this will change with split into virtual nodes
    theInfo->currentConclusion = tunknown;
    theInfo->timeTrueFalse = 0;
    theInfo->currentValue = 0.0;
    theInfo->startTime = 0;
    theInfo->endTime = 0;
    theInfo->estimatedCost = 0.0;
    theInfo->persistent = true;
  }
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
 uiRequestBuff(NULL),
 numUIrequests(0),
 uiRequestBuffSize(0)
{
  vector<searchHistoryNode*> Nodes;
  root = new searchHistoryNode ((searchHistoryNode *)NULL,
				topLevelHypothesis,
				topLevelFocus, refineWhyAxis,
				true, this, "TopLevelHypothesis", nextID);
  root->setExpanded();
  Nodes += root;
  NodeIndex[nextID] = root;
  nextID++;
}

void 
searchHistoryGraph::updateDisplayedStatus (string &newmsg)
{
  uiMgr->updateStatusDisplay(guiToken, newmsg.string_of());
}
  
void
searchHistoryNode::inActivateAll()
{
  this->changeActive(false);
  for (unsigned i = 0; i < children.size(); i++)
    children[i]->inActivateAll();
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
  string status = string("\nSearch for Phase ") 
    + string(performanceConsultant::DMcurrentPhaseToken)
      + string (" ended due to end of phase at time ")
	+ string (searchEndTime) + string (".\n");
  updateDisplayedStatus(status);
  // change display of all nodes to indicate "inactive"; no 
  // change to truth value displayed
  root->inActivateAll();
}

searchHistoryNode* 
searchHistoryGraph::addNode (searchHistoryNode *parent,
			     hypothesis *why,
			     focus whereowhere,
			     refineType axis,
			     bool persist,
			     const char *shortName,
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
			     shortName, nextID++);
  *foclist += newkid;
  newkid->setupExperiment();
  //
  //** this will be replaced with more rational priority calculation
  if (axis == refineWhyAxis)
    PCsearch::addToQueue(5, newkid, getPhase());
  else
    PCsearch::addToQueue(10, newkid, getPhase());
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
		       &nodeAdded);
    nodeptr->addNodeToDisplay();
    nodeptr->addEdgeToDisplay(root->getNodeId(), (char *)NULL);

    // note: at this point no experiment has been started for this search.
    // addNode puts these on the ready queue, but the ready queue is only 
    // checked when new data arrives from the data manager, so its a chicken
    // and egg deal.  We go ahead and start the experiments here; when these
    // nodes are removed from the queue, there will be an extra call to start
    // them which will have no effect.  We reset numActiveExperiments to 
    // 0, when they come off the ready queue numActiveExperiments will be 
    // bumped back up.

    nodeptr->startExperiment();
  }
  delete topmost;
}


