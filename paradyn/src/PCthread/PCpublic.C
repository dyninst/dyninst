/*
 * 
 * $Log: PCpublic.C,v $
 * Revision 1.6  1994/05/02 20:38:13  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.5  1994/04/27  22:55:03  hollings
 * Merged refine auto and search.
 *
 * Revision 1.4  1994/04/21  04:56:04  karavan
 * added calls to draw shg; changed node shortName from # to a short word
 *
 * Revision 1.3  1994/02/24  04:36:50  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 * Changed header files to reflect changes in igen.
 *
 * Revision 1.2  1994/02/09  22:35:48  hollings
 * fixed pointers refs that pur caught.
 *
 * Revision 1.1  1994/02/02  00:38:17  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.10  1993/09/03  19:01:59  hollings
 * removed 4th arg to printMetric.
 *
 * Revision 1.9  1993/09/03  18:54:25  hollings
 * added functions to print predicted cost.
 *
 * Revision 1.8  1993/08/05  18:53:14  hollings
 * new include format and way to chage current refinement.
 *
 * Revision 1.7  1993/05/07  20:21:26  hollings
 * upgrade to use dyninst interface.
 *
 * Revision 1.6  1993/02/03  00:06:49  hollings
 * removed execesive friends of focus and focusCell.
 *
 * Revision 1.5  1993/01/28  19:31:08  hollings
 * made SearchHistoryGraph a pointer to a SHG node.
 *
 * Revision 1.4  1992/12/14  19:58:27  hollings
 * added true enable/disable.
 *
 * Revision 1.3  1992/10/23  20:14:51  hollings
 * working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:06:33  hollings
 * first cut at automated refinement.
 *
 * Revision 1.1  1992/08/03  20:42:59  hollings
 * Initial revision
 *
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1992 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCpublic.C,v 1.6 1994/05/02 20:38:13 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "PCmetric.h"
#define extern
#include "PCglobals.h"
#undef extern
#include "PCwhy.h"
#include "PCwhen.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "performanceConsultant.SRVR.h"
#include "paradyn.h"
void performanceConsultant::printTestStatus()
{
    testResultList curr;

    if (currentTestResults) {
	printf("\ntest results\n");
	for (curr = *currentTestResults; *curr; curr++) {
	    (*curr)->print();
	    printf("\n");
	}
    } else {
	printf("No test results available\n");
    }
}


void performanceConsultant::createInterval(int parent, 
					   timeStamp start, 
					   timeStamp end, 
					   char *name)
{
    timeInterval *pi, *i;

    pi = allTimeIntervals.find((void *) parent);
    if (!pi) {
	printf("time interval i%d does not exist\n", parent);
	return;
    }
    i = new timeInterval(pi, start, end, name);
}


searchHistoryNodeList BuildWhyRefinements(searchHistoryNode *of)
{
    Boolean val;
    hypothesis *h;
    hypothesisList hl;
    searchHistoryNode *newNode;
    searchHistoryNodeList ret;
    char *nptr;

    if (!of) return(ret);
    if (!of->why->children) return(ret);

    for (hl = *of->why->children; h = *hl; hl++) {
	newNode = findAndAddSHG(of, h, of->where, of->when);
	ret.add(newNode);
	val = of->children->addUnique(newNode);

	  // extract display label string from node name
	nptr = strchr (newNode->name, ' ');
	*newNode->shortName = '\0';
	strncat (newNode->shortName, newNode->name, (nptr-newNode->name));
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
	uiMgr->DAGaddEdge (SHGid, of->nodeId, newNode->nodeId, WHYEDGESTYLE);
    }
    return(ret);
}

SHNptr_Array performanceConsultant::getWhyRefinements(searchHistoryNode *node)
{
    int i;
    SHNptr_Array ret;
    searchHistoryNodeList temp;

    temp = BuildWhyRefinements(node);
    ret.count = temp.count();
    ret.data = (SHNptr *) malloc(sizeof(SHNptr)*ret.count);
    for (i=0; *temp; temp++, i++) {
	ret.data[i] = *temp;
    }
    return(ret);
}

searchHistoryNodeList BuildWhenRefinements(searchHistoryNode *of)
{
    Boolean val;
    timeInterval *i;
    timeIntervalList il;
    searchHistoryNodeList ret;
    searchHistoryNode *newNode;

    if (!of) return(ret);
    if (!of->when->subIntervals) return(ret);

    for (il=*of->when->subIntervals; i = *il; il++) {
	newNode = findAndAddSHG(of, of->why, of->where, i);
	ret.add(newNode);
	val = of->children->addUnique(newNode);

          // note:  this will draw the nodeID# as the shg node label!!
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
	uiMgr->DAGaddEdge (SHGid, of->nodeId, newNode->nodeId, WHENEDGESTYLE);
	
    }
    return(ret);
}


SHNptr_Array performanceConsultant::getWhenRefinements(searchHistoryNode *node)
{
    int i;
    SHNptr_Array ret;
    searchHistoryNodeList temp;

    temp = BuildWhenRefinements(node);
    ret.count = temp.count();
    ret.data = (SHNptr *) malloc(sizeof(SHNptr)*ret.count);
    for (i=0; *temp; temp++, i++) {
	ret.data[i] = *temp;
    }
    return(ret);
}

searchHistoryNodeList BuildWhereRefinements(searchHistoryNode *of)
{
    focus *f;
    Boolean val;
    focusList newFoci;
    searchHistoryNodeList ret;
    searchHistoryNode *newNode;
    char *childf;
    char *tempa, *tempb, *c;
    char sep[] = ",";

    if (!of) return(ret);

    for (newFoci = of->where->enumerateRefinements(); f=*newFoci; newFoci++){
	newNode = findAndAddSHG(of, of->why, f, of->when);

	    // get label name for shg display
            // by comparing full names of parent and newNode and 
            // setting shortName to the end of the different path
            // Best feature of the following code: it works. :-P

	childf = new char[strlen (newNode->name) + 1];
	strcpy (childf, newNode->name);
	c = strchr(childf, ' ');
	c++; c++;
	tempa = strtok(c, sep);
	if (strstr(of->name, tempa) != NULL) 
	  while ((tempa = strtok(NULL, sep)) != NULL) {
	    if (strstr(of->name, tempa) == NULL) {
	    break;
	  }
	}
	tempb = strrchr (tempa, '/');
	tempb++;
	if (tempa = strchr (tempb, '>'))
	  *tempa = '\0';
	*newNode->shortName = '\0';
	strcat (newNode->shortName, tempb);
	delete (childf);
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
	uiMgr->DAGaddEdge (SHGid, of->nodeId, newNode->nodeId, WHEREEDGESTYLE);
	
	val = of->children->addUnique(newNode);
	ret.add(newNode);
    }
    // delete(newFoci);

    return(ret);
}

SHNptr_Array performanceConsultant::getWhereRefinements(searchHistoryNode *node)
{
    int i;
    SHNptr_Array ret;
    searchHistoryNodeList temp;

    temp = BuildWhereRefinements(node);
    ret.count = temp.count();
    ret.data = (SHNptr *) malloc(sizeof(SHNptr)*ret.count);
    for (i=0; *temp; temp++, i++) {
	ret.data[i] = *temp;
    }
    return(ret);
}

searchHistoryNodeList BuildAllRefinements(searchHistoryNode *of)
{
    searchHistoryNodeList ret;

    // ??? ADD Dag Code Here!!
    ret += BuildWhyRefinements(of);
    ret += BuildWhereRefinements(of);
    ret += BuildWhenRefinements(of);
    // ??? maybe here to update view of world.
    return(ret);
}

SHNptr_Array performanceConsultant::getAllRefinements(searchHistoryNode *node)
{
    int i;
    SHNptr_Array ret;
    searchHistoryNodeList temp;

    temp = BuildAllRefinements(node);
    ret.count = temp.count();
    ret.data = (SHNptr *) malloc(sizeof(SHNptr)*ret.count);
    for (i=0; *temp; temp++, i++) {
	ret.data[i] = *temp;
    }
    return(ret);
}

searchHistoryNode *performanceConsultant::getCurrentRefinement()
{
    return(currentSHGNode);
}

int performanceConsultant::getCurrentNodeId()
{
    return(currentSHGNode->nodeId);
}

void performanceConsultant::printSHGList()
{
    searchHistoryNodeList curr;

    printf("\nNodes in Search History Graph\n");
    for (curr=allSHGNodes; *curr; curr++) {
	(*curr)->print(-1);
	printf("\n");
    }
}

void performanceConsultant::printSHGNode(searchHistoryNode *node)
{
    node->print(-1);
}

void performanceConsultant::doRefine(int_Array ids)
{
    int i;
    int id;
    searchHistoryNode *n;
    searchHistoryNodeList curr;

    for (i=0; i < ids.count; i++) {
	// find the named id.
	id = ids.data[i];
	for (curr = allSHGNodes; n = *curr; curr++) {
	     if (n->nodeId == id) {
		 n->changeActive(TRUE);
		 break;
	     }
	}
	if (!n) {
	    printf("Node %d does not exist\n", id);
	}
    }
}

void performanceConsultant::resetRefinement()
{
    searchHistoryNode *curr;
    searchHistoryNodeList ptr;

    setCurrentRefinement(SearchHistoryGraph);
    for (ptr = allSHGNodes; curr = *ptr; ptr++) {
	curr->active = FALSE;
	curr->status = FALSE;
	curr->beenTested = FALSE;
	   // update SHG display
	uiMgr->DAGconfigNode (SHGid, curr->nodeId, UNTESTEDNODESTYLE);
    }
    currentSHGNode->changeActive(TRUE);
}

void printStats()
{
    datum *d;
    int count;
    PCmetric *met;
    List<datum*> dl;
    PCmetricList curr;

    count = 0;
    for (curr = allMetrics; *curr; curr++) {
	met = *curr;
	for (dl  = met->samples; d = *dl; dl++) {
	    if (d->used) count++;
	}
    }
    printf("%d metric/focus pairs used\n", count);
}


//
// print the predicted cost of all currently enabled metrics.
//
void globalCost()
{
    datum *d;
    int count;
    PCmetric *met;
    List<datum*> dl;
    PCmetricList curr;
    extern float globalPredicatedCost;

    count = 0;
    globalPredicatedCost = 0.0;
    for (curr = allMetrics; *curr; curr++) {
	met = *curr;
	for (dl  = met->samples; d = *dl; dl++) {
	    if (d->enabled) {
		met->changeCollection(d->f, getCollectionCost);
	    }
	}
    }
    printf("total predicted cost = %f\n", globalPredicatedCost);
}

