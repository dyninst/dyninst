
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
 * Do automated refinement
 *
 * $Log: PCauto.C,v $
 * Revision 1.22  1995/06/02 20:50:04  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.21  1995/02/27  19:17:24  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.20  1995/02/16  08:19:00  markc
 * Changed Boolean to bool
 *
 * Revision 1.19  1995/01/26  17:58:32  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.18  1994/12/21  00:46:27  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.17  1994/10/26  23:21:26  tamches
 * Removed min/max for tunable constant "predictedCostLimit", replacing it
 * with a simple validation-function "predictedCostLimitValidChecker", after
 * consulting w/ Jeff & Jon
 *
 * Revision 1.16  1994/10/25  22:07:58  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.15  1994/09/05  20:00:44  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.14  1994/08/05  16:04:10  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.13  1994/08/03  19:09:45  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.12  1994/07/25  04:47:02  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.11  1994/07/14  23:49:48  hollings
 * added batch mode for SHG, fixed the sort function to use beenTrue.
 *
 * Revision 1.10  1994/06/22  22:58:17  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.9  1994/06/12  22:40:46  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.8  1994/05/19  00:00:25  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.7  1994/05/18  02:49:26  hollings
 * Changed the time since last change to use the time of the first sample
 * arrivial after the change (rather than the time of the change).
 *
 * Revision 1.6  1994/05/18  00:48:50  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.5  1994/05/02  20:38:08  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.4  1994/04/27  22:54:59  hollings
 * Merged refine auto and search.
 *
 * Revision 1.3  1994/03/01  21:25:08  hollings
 * added tunable constants.
 *
 * Revision 1.2  1994/02/05  23:14:45  hollings
 * Added context to pauseApplication call.
 *
 * Revision 1.1  1994/02/02  00:38:10  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.8  1993/09/03  18:53:05  hollings
 * switched to a cost base limit on the number of refinements considered from
 * a static limit of 5.
 *
 * Revision 1.7  1993/08/05  18:52:34  hollings
 * new include and improved mode for giving up on includes.
 *
 * Revision 1.6  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.5  1993/01/28  19:31:08  hollings
 * made SearchHistoryGraph a pointer to a SHG node.
 *
 * Revision 1.4  1992/12/14  19:56:57  hollings
 * added true enable/disable.
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
 *
 */


#include <stdlib.h>
#include <stdio.h>

#include "../TCthread/tunableConst.h"
#include "dataManager.thread.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "PCglobals.h"
#include "PCauto.h"
#include "../pdMain/paradyn.h"


// 25% to start
bool predictedCostLimitValidChecker(float newVal) {
   // checker function for the tunable constant "predictedCostLimit",
   // whose declaration has moved to pdMain/main.C
   if (newVal < 0.0)
      return false; // no good
   else
      return true; // okay
}


searchHistoryNode *PCbaseSHGNode;
extern int PCautoRefinementLimit;
extern searchHistoryNodeList BuildAllRefinements(searchHistoryNode *of);

extern int UIM_BatchMode;

int CompareOptions(const void *left, const void *right)
{
    hint *h;
    hintList hl;
    searchHistoryNode *a = *(searchHistoryNode**) left;
    searchHistoryNode *b = *(searchHistoryNode**) right;

    // first check for suppressed.
    if (a->getSuppressed() != b->getSuppressed()) {
	if (a->getSuppressed()) {
	    return(1);
	} else {
	    return(-1);
	}
    }

    // second use hints.
    if (PCbaseSHGNode->hints) {
	hl = *PCbaseSHGNode->hints;
	for (;h = *hl; (void ) hl++) {
	    if (a->why == h->why) {
		return(-1);
	    }
	    if (b->why == h->why) {
		return(1);
	    }
	    if (a->where == h->where) {
		return(-1);
	    }
	    if (b->where == h->where) {
		return(1);
	    }
	}
	return(0);
    }
    // next use why before where.
    if ((a->why != PCbaseSHGNode->why) || (b->why !=  PCbaseSHGNode->why)) {
	if (a->why == PCbaseSHGNode->why)
	    return(1);
	if (b->why == PCbaseSHGNode->why)
	    return(-1);
	return(strcmp(a->why->name, b->why->name));
    } else if ((a->where != PCbaseSHGNode->where) || 
	       (b->where != PCbaseSHGNode->where)) {
	if (a->where == PCbaseSHGNode->where)
	    return(1);
	if (b->where == PCbaseSHGNode->where)
	    return(-1);


	// both on where, if one is been true and the other hasn't use it.
	if (a->getBeenTrue() != b->getBeenTrue()) {
	    if (a->getBeenTrue()) {
		return(-1);
	    } else {
		return(1);
	    }
	}

	// final tie breaker, use strcmp to ensure a unique ordering.
	return(strcmp((char *) a->where->getName(), 
		      (char *) b->where->getName()));
    } else {
	return(0);
    }
}

static int refineCount;
static int currentRefinementBase;
static int currentRefinementLimit;
static searchHistoryNode **refinementOptions;

void autoSelectRefinements()
{
    int i;
    searchHistoryNodeList refList;
    void autoChangeRefineList();

    if (!currentSHGNode->getStatus()) {
	// select this node.
	refList.add(currentSHGNode); 
    } else {
	refList = BuildAllRefinements(currentSHGNode);
    }

    refineCount = refList.count();

    if (refineCount == 0) return;

    // build an array of refinements.
    currentRefinementBase = 0;
    refinementOptions = (searchHistoryNode**) 
	calloc(sizeof(searchHistoryNode *), refineCount);
    for (i=0; *refList; i++, refList++) {
	refinementOptions[i] = *refList;
    }

    // order the options list.
    PCbaseSHGNode = currentSHGNode;
    qsort(refinementOptions, refineCount, sizeof(searchHistoryNode*), 
	CompareOptions);

    // now consider the refinements.
    autoChangeRefineList();
}

void autoChangeRefineList()
{
    int i;
    float newCost;
    timeStamp timeLimit;
    float totalCost = 0.0;
    searchHistoryNode *curr;
    static float batchCost = 0.0;
    extern bool PCsearchPaused;

    // obtain fresh values of tunable constants "printNodes", "predictedCostLimit",
    // and "maxEval"
    tunableBooleanConstant printNodes = tunableConstantRegistry::findBoolTunableConstant("printNodes");
    tunableFloatConstant predictedCostLimit = tunableConstantRegistry::findFloatTunableConstant("predictedCostLimit");
    tunableFloatConstant maxEval = tunableConstantRegistry::findFloatTunableConstant("maxEval");

    if (printNodes.getValue())
       cout << "TRYING: " << endl;

    UIM_BatchMode++;
    totalCost = dataMgr->getCurrentHybridCost();

    if (currentRefinementBase) {
	// some number of tests just expired.
	totalCost -= batchCost;

	if (totalCost < 0.0) {
	    // The estimated cost of the last batch was way too high!!!
	    totalCost = 0.0;
	}
    }

    batchCost = 0.0;
    // printf("startingCost = %f\n", totalCost);
    for (i =currentRefinementBase; 
	 totalCost < predictedCostLimit.getValue(); i++) {
	if (i >= currentRefinementBase + maxEval.getValue()) {
	    // its one too high.
	    i--;
	    break;
	}
	if (i >= refineCount) break;
	curr = refinementOptions[i];
	newCost = curr->cost(); 

	if (printNodes.getValue()) {
	    cout << "  " << ((char *) curr->name) << endl;
	    cout << "    (cost = " << curr->cost() << ")" << endl;
	}

	if (totalCost + newCost > predictedCostLimit.getValue()) {
	    // too expensive to add.
	    i--;
	    if (i < currentRefinementBase) i = currentRefinementBase;
	    break;
	}
	batchCost += newCost;
	totalCost += newCost;
	// printf("totalCost = %f\n", totalCost);

	curr->changeActive(true);
    }
    UIM_BatchMode--;

    currentRefinementLimit = i;
    if (printNodes.getValue()) cout <<  "\n";

    // see if there was any thing to test.
    if (currentRefinementBase >= refineCount) {
	dataMgr->pauseApplication();
	PCstatusDisplay->updateStatusDisplay(PC_STATUSDISPLAY, 
	    "all refinements considered...application paused\n");
	// prevent any further auto refinement.
	PCautoRefinementLimit = 0;
	PCsearchPaused = true;

	return;
    }

    if (i == currentRefinementBase) {
	dataMgr->pauseApplication();
	PCstatusDisplay->updateStatusDisplay(PC_STATUSDISPLAY,
	    "unable to consider further refinements within cost limits\n");
	PCstatusDisplay->updateStatusDisplay(PC_STATUSDISPLAY,
	    "predicted cost of (%f +%f) >= limit %f\n", 
	    totalCost, newCost, predictedCostLimit.getValue());
	// prevent any further auto refinement.
	PCautoRefinementLimit = 0;
	PCsearchPaused = true;

	return;
    }

    configureTests();

    // obtain "sufficientTime" tunable constant
    tunableFloatConstant sufficientTime = tunableConstantRegistry::findFloatTunableConstant("sufficientTime");
    timeLimit = PCcurrentTime + sufficientTime.getValue();
}

void autoTimeLimitExpired()
{
    int i;

    // obtain "printNodes" tunable constant
    tunableBooleanConstant printNodes = tunableConstantRegistry::findBoolTunableConstant("printNodes");

    if (printNodes.getValue())
       cout << "GIVING UP ON: " << endl;

    UIM_BatchMode++;
    for (i=currentRefinementBase; i <= currentRefinementLimit; i++) {
	if (i >= refineCount) break;
	if (printNodes.getValue()) 
	    cout << "   " << *refinementOptions[i] << endl;
	refinementOptions[i]->changeActive(false);
    }
    UIM_BatchMode--;
    if (printNodes.getValue()) cout << endl;

    currentRefinementBase = currentRefinementLimit+1;
    autoChangeRefineList();
}

bool autoTestRefinements()
{
    int i;
    bool previousOK;
    searchHistoryNode *curr;
    extern void setCurrentRefinement(searchHistoryNode*);
    extern bool verifyPreviousRefinements();

    previousOK = verifyPreviousRefinements();
    if (!previousOK) {
	// something changed 
	free(refinementOptions);
	refinementOptions = NULL;

	// find a a good new refinement to try back of the SHG. 
	autoSelectRefinements();
	return(true);
    }

    // see if we found one.
    for (i=currentRefinementBase; i <= currentRefinementLimit; i++) {
	if (i >= refineCount) break;
	curr = refinementOptions[i];
	if (curr->getStatus()) {

	    setCurrentRefinement(curr);

	    // disable other refinement options.
	    for (i=0; i < refineCount; i++) {
		if (refinementOptions[i] == curr) continue;
		refinementOptions[i]->changeActive(false);
	    }
	    free(refinementOptions);
	    refinementOptions = NULL;

	    // we made a refinement decrement counter.
	    PCautoRefinementLimit--;

	    // advance to next refinement canidate list.
	    autoSelectRefinements();
	    return(true);
	}
    }
    return(false);
}
