
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
 * $Log: PCshg.C,v $
 * Revision 1.25  1995/07/17 04:29:01  tamches
 * Changed whereAxis to pcWhereAxis, avoiding a naming conflict with the
 * new UI where axis.
 *
 * Revision 1.24  1995/06/02  20:50:15  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.23  1995/02/27  19:17:37  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.22  1995/02/16  08:19:20  markc
 * Changed Boolean to bool
 *
 * Revision 1.21  1994/12/21  00:46:34  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.20  1994/10/25  22:08:11  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.19  1994/09/22  01:06:05  markc
 * Added correct number of args to printf
 * Changed malloc on line 179 "shortName = (char*) malloc(32) since more than
 * 16 chars are needed.  This is a bug.
 *
 * Revision 1.18  1994/09/06  09:26:27  karavan
 * added back color-coded edges: added int edgeStyle to SearchHistoryNode
 * class and added estyle argument to constructor and findAndAddSHG
 *
 * Revision 1.17  1994/09/05  20:01:07  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.16  1994/08/05  16:04:16  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.15  1994/08/03  19:09:54  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.14  1994/07/28  22:34:04  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.13  1994/07/25  04:47:10  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.12  1994/07/22  19:25:45  hollings
 * removed supress SHG option for now.
 *
 * Revision 1.11  1994/07/14  23:47:56  hollings
 * added beenTrue.
 *
 * Revision 1.10  1994/06/29  02:56:24  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.9  1994/06/27  18:55:11  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.8  1994/06/22  22:58:23  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.7  1994/05/30  19:35:02  hollings
 * Removed workaround for node add update bug.
 *
 * Revision 1.6  1994/05/19  00:00:30  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.5  1994/05/18  00:48:56  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.4  1994/05/12  23:34:08  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.3  1994/05/09  20:58:04  hollings
 * Added clearSHG.
 *
 * Revision 1.2  1994/04/21  04:58:53  karavan
 * updated shgInit() to include display initialization; added changeStatus
 * and changeActive member functions to searchHistoryNode.
 *
 * Revision 1.1  1994/02/02  00:38:19  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.10  1993/09/03  19:04:12  hollings
 * removed printf.
 *
 * Revision 1.9  1993/08/11  18:51:54  hollings
 * added cost model
 *
 * Revision 1.8  1993/08/05  18:57:50  hollings
 * added searchHistoryNode print and new includes.
 *
 * Revision 1.7  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.6  1993/03/23  17:18:07  hollings
 * changed getName to a function.
 *
 * Revision 1.5  1993/02/03  00:06:49  hollings
 * removed execesive friends of focus and focusCell.
 *
 * Revision 1.4  1993/01/28  19:32:14  hollings
 * make SearchHistoryGraph a pointer to a SHG node.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:29:48  hollings
 * fixed rcs log entry.
 *
 * Revision 1.1  1992/08/24  15:06:33  hollings
 * Initial revision
 *
 * Search History Graph
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "PCshg.h"
#include "PCglobals.h"
#include "../pdMain/paradyn.h"
#include "../TCthread/tunableConst.h"

// ugly global to relate back cost of collecting data for a given test.
float globalPredicatedCost;

ostream& operator <<(ostream &os, hint& h)
{
    os << "HINT ";
    os << ((char *) h.message);
    if (h.why) os << *h.why;
    if (h.where) os << *h.where;
    if (h.when) os << *h.when;
    os << endl;

    return os; // added AT 12/9/94
}

stringPool searchHistoryNode::shgNames;

searchHistoryNode::searchHistoryNode(hypothesis *h, focus *f, timeInterval *t,
				     int estyle)
{
    static int nextId;
    char tempName[255];

    why = h;
    where = f;
    when = t;
    edgeStyle = estyle;
    status = false;
    active = false;
    beenActive = false;
    beenTrue = false;
    beenTested = false;
    nodeId = ++nextId;
    children = new(searchHistoryNodeList);
    sprintf(tempName, "%s %s %s", h->name, (char*)f->getName(), t->getName());
    name= shgNames.findAndAdd(tempName);
    shortName = (char *) malloc(32);
    sprintf(shortName, " %d ", nodeId);
    style = UNTESTEDNODESTYLE;
    suppressed = false;
    if (f->getSuppressed()) suppressed = true;
};

ostream& operator <<(ostream &os, searchHistoryNode& shg)
{
    searchHistoryNodeList curr;

    if (shg.nodeId == currentSHGNode->nodeId) os << "**";
    os << shg.nodeId;
    if (shg.why) os << *shg.why;
    if (shg.where) {
	os << *shg.where, " ";
    }
    if (shg.when) os << *shg.when;
    if (shg.status)
	os << " status = TRUE, ";
    else
	os << " status = FALSE, ";
    if (shg.active)
        os << "active = TRUE";
    else
	os << "active = FALSE";

    if (shg.suppressed)
	os << " suppressed = TRUE";
    else
	os << " suppressed = FALSE";
	
    os << endl;

    return os; // added AT 12/8/94
}

bool searchHistoryNode::print(int parent, FILE *fp)
{
    bool omited;
    searchHistoryNodeList curr;

    if (!beenTested) {
	return(true);
	// fprintf(fp, "draw %d as Box color yellow label %d;\n", nodeId, nodeId);
    } else if (!ableToEnable) {
	// omit nodes we can't enable.
	return(true);
    } else if (!active) {
	fprintf(fp, "draw %d as Box color blue pointsize 30 label %d;\n", nodeId, nodeId);
    } else if (status) {
	fprintf(fp, "draw %d as Circle color red pointsize 400 label %d;\n", nodeId, nodeId);
    } else {
	fprintf(fp, "draw %d as Box color green label %d;\n", nodeId, nodeId);
    }

    if (parent != -1) fprintf(fp, "path %d %d;\n", parent, nodeId);

    omited = false;
    for (curr = *children; *curr; curr++) {
	omited = omited | (*curr)->print(nodeId, fp);
    }
    if (omited) {
	fprintf(fp,"draw %d.st as Box color orange label %d\\n;\n",nodeId, nodeId);
	fprintf(fp,"draw %d.end as Box color orange label %d\\n;\n",nodeId,nodeId);
	fprintf(fp, "path %d %d.st;\n", parent, nodeId);
	fprintf(fp, "path %d %d.end;\n", parent, nodeId);
    }
    return(false);
}

inline void searchHistoryNode::changeColor()
{
  int newStyle;

  if (!beenTested) {
      newStyle = UNTESTEDNODESTYLE;
  } else {
      if (active) {
	  if (status) {
	      newStyle = ACTIVETRUENODESTYLE;
	  } else {
	      newStyle = ACTIVEFALSENODESTYLE;
	  }
      } else {
	  newStyle = INACTIVENODESTYLE;
      }
  }
  if (newStyle != style) {
      uiMgr->DAGconfigNode(SHGid, nodeId, newStyle);
      style = newStyle;
  }
}

void searchHistoryNode::changeActive(bool newact)
{
    searchHistoryNodeList parent;

    active = newact;
#ifdef SHG_ADD_ON_EVAL
    tunableBooleanConstant supressSHG = tunableConstantRegistry::findBoolTunableConstant("suppressSHG");
    if (!supressSHG.getValue()) {
      if (newact && !beenActive) {
	beenActive = true;
        // make sure we have the node in the SHG
	for (parent = this->parents; *parent; parent++) {
	    uiMgr->DAGaddNode (SHGid, this->nodeId, UNTESTEDNODESTYLE,
			       this->shortName, (char *) this->name, 0);
	    uiMgr->DAGaddEdge(SHGid, (*parent)->nodeId, this->nodeId, 
				this->edgeStyle);
        }
      }
    }
#endif
    changeColor();
}    

void searchHistoryNode::changeStatus(bool newstat)
{
  status = newstat;
  if (status) {
     beenTrue = true;
  }
  changeColor();
}
    
void searchHistoryNode::changeTested(bool newstat)
{
  beenTested = newstat;
  changeColor();
}
    
void searchHistoryNode::resetStatus()
{
    searchHistoryNodeList curr;

    changeStatus(false);
//    
    if (children) {
	for (curr = *children; *curr; curr++) {
	    (*curr)->resetStatus();
	}
    }
}

//
// Find the current node in the shg graph, and activate all nodes along all 
//   paths from the root to the current node.  This will work when we have 
//   multiple nodes.
//
bool searchHistoryNode::resetActive()
{
    bool ret;
    searchHistoryNodeList curr;

    ret = false;
    if (this == currentSHGNode) {
	ret = true;
    } else if (children) {
	for (curr = *children; *curr; curr++) {
	    if ((*curr)->resetActive()) {
		ret = true;
	    }
	}
    }
    changeActive(ret);
    return(active);
}

//
// Get the cost of evaluatin the associated node.
//
// This realy needs to check which why axis parts need to be tested.
//	-- e.g. implicit testing of children of root hypothesis.
//
float searchHistoryNode::cost()
{
    focus *prevFocus;

    // now call enable func.
    prevFocus = currentFocus;
    currentFocus = where;
    globalPredicatedCost = 0.0;

    why->cost();

    currentFocus = prevFocus;
    return(globalPredicatedCost);
}

searchHistoryNode *findAndAddSHG(searchHistoryNode *parent,
				 hypothesis *why,
                                 focus *where,
                                 timeInterval *when,
				 int estyle)
{
    searchHistoryNode *ret;
    searchHistoryNode *temp;

    temp = new searchHistoryNode(why,where,when,estyle);
    ret = allSHGNodes.find(temp->name);
    if (ret) {
	delete temp;
	return(ret);
    } else {
	allSHGNodes.add(temp, temp->name);
	return(temp);
    }
}

void shgInit()
{
    extern int PCpathDepth;

    // init default interval.
    whenAxis.start = 0;
    whenAxis.end = HUGE_VAL;

    currentInterval = &whenAxis;

    SearchHistoryGraph = new searchHistoryNode(&whyAxis,pcWhereAxis,&whenAxis,
					       WHEREEDGESTYLE);
    allSHGNodes.add(SearchHistoryGraph, SearchHistoryGraph->name);

    currentSHGNode = SearchHistoryGraph;
    currentSHGNode->changeActive(true);

    // begin visual display of shg
    SHGid = uiMgr->initSHG();     
    // display root node with style 1 
    uiMgr->DAGaddNode (SHGid, currentSHGNode->nodeId, UNTESTEDNODESTYLE, 
		       currentSHGNode->shortName, 
		       (char *) currentSHGNode->name, 1);

    PCpathDepth = 0;
}

searchHistoryNode *SearchHistoryGraph;
searchHistoryNodeList allSHGNodes;
searchHistoryNode *currentSHGNode;

