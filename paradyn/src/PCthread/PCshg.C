/*
 * $Log: PCshg.C,v $
 * Revision 1.2  1994/04/21 04:58:53  karavan
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
static char Copyright[] = 
    "@(#) Copyright (c) 1992 Jeff Hollingsworth. All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCshg.C,v 1.2 1994/04/21 04:58:53 karavan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "PCshg.h"
#include "PCglobals.h"
#include "paradyn.h"

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
    beenTested = FALSE;
    nodeId = ++nextId;
    children = new(searchHistoryNodeList);
    sprintf(tempName, "%s %s %s", h->name, f->getName(), t->getName());
    name= shgNames.findAndAdd(tempName);
    shortName = (char *) malloc(16);
    sprintf(shortName, " %d ", nodeId);
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

void 
searchHistoryNode::changeActive(Boolean newact)
{
  printf ("SHN::changeActive\n");
  active = newact;
  if (newact == TRUE) {
    if (status == TRUE) 
      uiMgr->DAGconfigNode(SHGid, nodeId, ACTIVETRUENODESTYLE);
    else
      uiMgr->DAGconfigNode(SHGid, nodeId, ACTIVEFALSENODESTYLE);
  }
  else
    uiMgr->DAGconfigNode (SHGid, nodeId, INACTIVENODESTYLE);
}    

void 
searchHistoryNode::changeStatus(Boolean newstat)
{
  printf ("SHN::changeStatus\n");
  status = newstat;
  if (newstat == TRUE) {
    uiMgr->DAGconfigNode(SHGid, nodeId, ACTIVETRUENODESTYLE);
  }
  else {
    uiMgr->DAGconfigNode(SHGid, nodeId, ACTIVEFALSENODESTYLE);
  }
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
    // init default interval.
    whenAxis.start = 0;
    whenAxis.end = FLT_MAX;

    currentInterval = &whenAxis;

    SearchHistoryGraph = new searchHistoryNode(&whyAxis,whereAxis,&whenAxis);
    allSHGNodes.add(SearchHistoryGraph, SearchHistoryGraph->name);

    currentSHGNode = SearchHistoryGraph;
    currentSHGNode->active = TRUE;

    // begin visual display of shg
    SHGid = uiMgr->initSHG();     
    printf ("PC: SHG initialized w/ SHGid = %d\n", SHGid);
    // display root node with style 1 
    uiMgr->DAGaddNode (SHGid, currentSHGNode->nodeId, UNTESTEDNODESTYLE, 
		       currentSHGNode->shortName, currentSHGNode->name, 1);

}

searchHistoryNode *SearchHistoryGraph;
searchHistoryNodeList allSHGNodes;
searchHistoryNode *currentSHGNode;

