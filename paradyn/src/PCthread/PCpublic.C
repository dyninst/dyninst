
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
 * 
 * $Log: PCpublic.C,v $
 * Revision 1.21  1995/06/02 20:50:12  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.20  1995/02/16  08:19:16  markc
 * Changed Boolean to bool
 *
 * Revision 1.19  1995/01/26  17:58:43  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.18  1994/11/11  10:46:34  markc
 * Used status line to print status
 *
 * Revision 1.17  1994/10/25  22:08:08  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.16  1994/09/06  09:26:25  karavan
 * added back color-coded edges: added int edgeStyle to SearchHistoryNode
 * class and added estyle argument to constructor and findAndAddSHG
 *
 * Revision 1.15  1994/08/05  16:04:15  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.14  1994/07/28  22:34:03  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.13  1994/06/29  02:56:22  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.12  1994/06/27  18:55:09  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.11  1994/06/22  22:58:21  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.10  1994/05/19  00:00:29  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.9  1994/05/12  23:34:07  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.8  1994/05/09  20:58:02  hollings
 * Added clearSHG.
 *
 * Revision 1.7  1994/05/06  06:39:36  karavan
 * SHG now initialized only upon request
 *
 * Revision 1.6  1994/05/02  20:38:13  hollings
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "PCmetric.h"
#include "PCglobals.h"
#include "PCwhy.h"
#include "PCwhen.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "performanceConsultant.thread.SRVR.h"
#include "../pdMain/paradyn.h"

extern void shgInit();

void performanceConsultant::printTestStatus()
{
    testResultList curr;

    if (currentTestResults) {
	cout << endl << "test results" << endl;
	for (curr = *currentTestResults; *curr; curr++) {
	    cout << *curr << endl;
	}
    } else {
	cout << "No test results available" << endl;
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
    bool newArc;
    hypothesis *h;
    hypothesisList hl;
    searchHistoryNode *newNode;
    searchHistoryNodeList ret;
    char *nptr;

    if (!of) return(ret);
    if (!of->why->children) return(ret);

    for (hl = *of->why->children; h = *hl; hl++) {
	newNode = findAndAddSHG(of, h, of->where, of->when, WHYEDGESTYLE);
	ret.add(newNode);

	newNode->parents.addUnique(of);
	newArc = of->children->addUnique(newNode);

	  // extract display label string from node name
	nptr = strchr ((char *) newNode->name, ' ');
	*newNode->shortName = '\0';
	(void) strncat (newNode->shortName, (char *) newNode->name, 
	    (nptr-(char *)newNode->name));
#ifndef SHG_ADD_ON_EVAL
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
	if (newArc) {
	    uiMgr->DAGaddEdge(SHGid, of->nodeId, 
			      newNode->nodeId, WHYEDGESTYLE);
	}
#endif
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
    bool newArc;
    timeInterval *i;
    timeIntervalList il;
    searchHistoryNodeList ret;
    searchHistoryNode *newNode;

    if (!of) return(ret);
    if (!of->when->subIntervals) return(ret);

    for (il=*of->when->subIntervals; i = *il; il++) {
	newNode = findAndAddSHG(of, of->why, of->where, i, WHENEDGESTYLE);
	ret.add(newNode);
	newArc = of->children->addUnique(newNode);
	newNode->parents.addUnique(of);

#ifndef SHG_ADD_ON_EVAL
        // note:  this will draw the nodeID# as the shg node label!!
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
	if (newArc) {
	    uiMgr->DAGaddEdge(SHGid, of->nodeId, 
			      newNode->nodeId, WHENEDGESTYLE);
	}
#endif
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

//
// get label name for shg display
// by comparing full names of parent and newNode and 
// setting shortName to the end of the different path
// Best feature of the following code: it works. :-P
//
char *makeShortName(char *parent, char *child)
{
    char *ret;
    char *childf;
    char sep[] = ",";
    char *tempa, *tempb, *c;

    childf = new char[strlen (child) + 1];
    strcpy (childf, child);
    c = strchr(childf, ' ');
    c++; c++;
    tempa = strtok(c, sep);
    if (strstr(parent, tempa) != NULL) 
      while ((tempa = strtok(NULL, sep)) != NULL) {
	if (strstr(parent, tempa) == NULL) {
	break;
      }
    }
    tempb = strrchr (tempa, '/');
    tempb++;
    if (tempa = strchr (tempb, '>'))
      *tempa = '\0';

    ret = strdup(tempb);
    delete (childf);
    return(ret);
}

searchHistoryNodeList BuildWhereRefinements(searchHistoryNode *of)
{
    focus *f;
    bool newArc;
    focusList newFoci;
    searchHistoryNodeList ret;
    searchHistoryNode *newNode;

    if (!of) return(ret);

    // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
    (void) (newFoci = of->where->enumerateRefinements());
    for (; *newFoci; newFoci++){
	f = *newFoci;
	newNode = findAndAddSHG(of, of->why, f, of->when, WHEREEDGESTYLE);
	newNode->shortName = 
	    makeShortName((char *) of->name, (char *) newNode->name);

#ifndef SHG_ADD_ON_EVAL
	uiMgr->DAGaddNode (SHGid, newNode->nodeId, UNTESTEDNODESTYLE, 
			   newNode->shortName, newNode->name, 0);
#endif
	
	newNode->parents.addUnique(of);
	newArc = of->children->addUnique(newNode);
#ifndef SHG_ADD_ON_EVAL
	if (newArc) {
	    uiMgr->DAGaddEdge(SHGid, of->nodeId, newNode->nodeId, 
		WHEREEDGESTYLE);
	}
#endif
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

    cout << "\nNodes in Search History Graph\n";
    for (curr=allSHGNodes; *curr; curr++) {
	cout << *(*curr);
	cout << endl;
    }
}

void performanceConsultant::printSHGNode(searchHistoryNode *node)
{
    cout << *node;
}

void performanceConsultant::startSHG()
{
  static int init;

  if (!init) {
      shgInit();
  }
}


void performanceConsultant::clearSHG()
{
    searchHistoryNode *n;
    searchHistoryNodeList curr;

    for (curr = allSHGNodes; n = *curr; curr++) {
	assert(allSHGNodes.remove(n->name));
	// delete(n);
    }

    // init a new root node.
    shgInit();
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
		 n->changeActive(true);
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
	curr->changeActive(false);
	curr->changeStatus(false);
	curr->changeTested(false);
    }
    currentSHGNode->changeActive(true);
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
    char buffer[100];
    sprintf(buffer, "%d metric/focus pairs used\n", count);
    PCstatusDisplay->updateStatusDisplay(PC_STATUSDISPLAY, buffer);
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

