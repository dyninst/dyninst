
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
 * Revision 1.11  1994/07/14 23:47:56  hollings
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

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCshg.C,v 1.11 1994/07/14 23:47:56 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "PCshg.h"
#include "PCglobals.h"
#include "../pdMain/paradyn.h"
#include "util/h/tunableConst.h"

// ugly global to relate back cost of collecting data for a given test.
float globalPredicatedCost;

void hint::print()
{
    printf("HINT %s:", message);
    if (why) why->print(0);
    if (where) where->print();
    if (when) when->print(0);
    printf("\n");
}

stringPool searchHistoryNode::shgNames;

searchHistoryNode::searchHistoryNode(hypothesis *h, focus *f, timeInterval *t)
{
    static int nextId;
    char tempName[255];

    why = h;
    where = f;
    when = t;
    status = FALSE;
    active = FALSE;
    beenActive = FALSE;
    beenTrue = FALSE;
    beenTested = FALSE;
    nodeId = ++nextId;
    children = new(searchHistoryNodeList);
    sprintf(tempName, "%s %s %s", h->name, f->getName(), t->getName());
    name= shgNames.findAndAdd(tempName);
    shortName = (char *) malloc(16);
    sprintf(shortName, " %d ", nodeId);
    style = UNTESTEDNODESTYLE;
};

void searchHistoryNode::print(int level)
{
    int i;
    searchHistoryNodeList curr;

    for (i=0; i < level; i++) printf("    ");
    if (this == currentSHGNode) printf("**");
    printf("%d: ", nodeId);
    if (why) why->print(-1);
    if (where) {
	where->print();
	printf(" ");
    }
    if (when) when->print(-1);
    if (status == TRUE) 
	printf(" status = TRUE, ");
    else
	printf(" status = FALSE, ");
    if (active == TRUE)
	printf("active = TRUE");
    else
	printf("active = FALSE");
	
     if (level >= 0) {
	 printf("\n");
         for (curr = *children; *curr; curr++) {
	     (*curr)->print(level+1);
	 }
     }
}

Boolean searchHistoryNode::print(int parent, FILE *fp)
{
    Boolean omited;
    searchHistoryNodeList curr;

    if (!beenTested) {
	return(True);
	// fprintf(fp, "draw %d as Box color yellow label %d;\n", nodeId, nodeId);
    } else if (!ableToEnable) {
	// omit nodes we can't enable.
	return(True);
    } else if (!active) {
	fprintf(fp, "draw %d as Box color blue pointsize 30 label %d;\n", nodeId, nodeId);
    } else if (status == TRUE) {
	fprintf(fp, "draw %d as Circle color red pointsize 400 label %d;\n", nodeId, nodeId);
    } else {
	fprintf(fp, "draw %d as Box color green label %d;\n", nodeId, nodeId);
    }

    if (parent != -1) fprintf(fp, "path %d %d;\n", parent, nodeId);

    omited = False;
    for (curr = *children; *curr; curr++) {
	omited = omited | (*curr)->print(nodeId, fp);
    }
    if (omited) {
	fprintf(fp,"draw %d.st as Box color orange label\\n;\n",nodeId,nodeId);
	fprintf(fp,"draw %d.end as Box color orange label\\n;\n",nodeId,nodeId);
	fprintf(fp, "path %d %d.st;\n", parent);
	fprintf(fp, "path %d %d.end;\n", parent);
    }
    return(False);
}

inline void searchHistoryNode::changeColor()
{
  int newStyle;

  if (beenTested == FALSE) {
      newStyle = UNTESTEDNODESTYLE;
  } else {
      if (active == TRUE) {
	  if (status == TRUE) {
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

tunableConstant supressSHG(0.0, 0.0, 1.0, NULL, "supressSHG",
    "Don't print the SHG");

void searchHistoryNode::changeActive(Boolean newact)
{
    searchHistoryNodeList parent;

    active = newact;
#ifdef SHG_ADD_ON_EVAL
    if (!supressSHG.getValue()) {
      if (newact && !beenActive) {
	beenActive = TRUE;
        // make sure we have the node in the SHG
	for (parent = this->parents; *parent; parent++) {
	    uiMgr->DAGaddNode (SHGid, this->nodeId, UNTESTEDNODESTYLE,
			       this->shortName, this->name, 0);
	    uiMgr->DAGaddEdge(SHGid, (*parent)->nodeId, this->nodeId, 
				WHEREEDGESTYLE);
        }
      }
    }
#endif
    changeColor();
}    

void searchHistoryNode::changeStatus(Boolean newstat)
{
  status = newstat;
  if (status == TRUE) {
     beenTrue = TRUE;
  }
  changeColor();
}
    
void searchHistoryNode::changeTested(Boolean newstat)
{
  beenTested = newstat;
  changeColor();
}
    
void searchHistoryNode::resetStatus()
{
    searchHistoryNodeList curr;

    changeStatus(FALSE);
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
Boolean searchHistoryNode::resetActive()
{
    Boolean ret;
    searchHistoryNodeList curr;

    ret = FALSE;
    if (this == currentSHGNode) {
	ret = TRUE;
    } else if (children) {
	for (curr = *children; *curr; curr++) {
	    if ((*curr)->resetActive() == TRUE) {
		ret = TRUE;
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
                                 timeInterval *when)
{
    searchHistoryNode *ret;
    searchHistoryNode *temp;

    temp = new searchHistoryNode(why,where,when);
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
    whenAxis.end = infinity();

    currentInterval = &whenAxis;

    SearchHistoryGraph = new searchHistoryNode(&whyAxis,whereAxis,&whenAxis);
    allSHGNodes.add(SearchHistoryGraph, SearchHistoryGraph->name);

    currentSHGNode = SearchHistoryGraph;
    currentSHGNode->changeActive(TRUE);

    // begin visual display of shg
    SHGid = uiMgr->initSHG();     
    // display root node with style 1 
    uiMgr->DAGaddNode (SHGid, currentSHGNode->nodeId, UNTESTEDNODESTYLE, 
		       currentSHGNode->shortName, currentSHGNode->name, 1);

    PCpathDepth = 0;
}

searchHistoryNode *SearchHistoryGraph;
searchHistoryNodeList allSHGNodes;
searchHistoryNode *currentSHGNode;

