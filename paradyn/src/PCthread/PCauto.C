/*
 * Do automated refinement
 *
 * $Log: PCauto.C,v $
 * Revision 1.9  1994/06/12 22:40:46  karavan
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

#include "util/h/tunableConst.h"
#include "dataManager.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "PCglobals.h"
#include "PCauto.h"
#include "../pdMain/paradyn.h"


// 25% to start
tunableConstant predictedCostLimit(0.25, 0.0, 1.0, NULL, "predictedCostLimit",
  "Max. allowable perturbation of the application.");

searchHistoryNode *PCbaseSHGNode;
extern int PCautoRefinementLimit;
extern searchHistoryNodeList BuildAllRefinements(searchHistoryNode *of);

int CompareOptions(const void *left, const void *right)
{
    hint *h;
    hintList hl;
    searchHistoryNode *a = *(searchHistoryNode**) left;
    searchHistoryNode *b = *(searchHistoryNode**) right;

    // first use hints.
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
	// use strcmp to ensure a unique ordering.
	return(strcmp(a->where->getName(), b->where->getName()));
    } else {
	return(0);
    }
}

tunableConstant maxEval(25.0, 0.0, 250.0, NULL, "maxEval",
    "Max. number of hypotheses to consider at once.");

static int refineCount;
static int currentRefinementBase;
static int currentRefinementLimit;
static searchHistoryNode **refinementOptions;

void autoSelectRefinements()
{
    int i;
    searchHistoryNodeList refList;

    if (currentSHGNode->getStatus() != TRUE) {
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
    char msgBuf[128];
    // float newCost;
    timeStamp timeLimit;
    float totalCost = 0.0;
    searchHistoryNode *curr;
    extern Boolean PCsearchPaused;
    extern tunableConstant sufficientTime;

    if (printNodes) printf("TRYING: ");
    for (i =currentRefinementBase; 
	 totalCost < predictedCostLimit.getValue(); i++) {
	if (i >= currentRefinementBase + maxEval.getValue()) {
	    // its one too high.
	    i--;
	    break;
	}
	if (i >= refineCount) break;
	if (printNodes) printf("%d ", refinementOptions[i]->nodeId);
	curr = refinementOptions[i];
	// newCost = curr->cost(); 
	// totalCost += newCost;
	curr->changeActive(TRUE);
    }
    currentRefinementLimit = i;

    if (printNodes) printf("\n");

    // see if there was any thing to test.
    if (currentRefinementBase >= refineCount) {
	dataMgr->pauseApplication(context);
/***
	uiMgr->updateStatusDisplay(1, "all refinements considered...application paused\n");
*/
	// prevent any further auto refinement.
	PCautoRefinementLimit = 0;
	PCsearchPaused = TRUE;

	return;
    }

    configureTests();
    timeLimit = PCcurrentTime + sufficientTime.getValue();
    sprintf (msgBuf, "setting autorefinement timelimit to %f\n", timeLimit);
/***
    uiMgr->updateStatusDisplay(1, msgBuf);
*/
    // see if the new ones are true already.
    // (void) autoTestRefinements();
}

void autoTimeLimitExpired()
{
    int i;

    if (printNodes) printf("GIVING UP ON: ");
    for (i=currentRefinementBase; i <= currentRefinementLimit; i++) {
	if (i >= refineCount) break;
	if (printNodes) printf("%d ",  refinementOptions[i]->nodeId);
	refinementOptions[i]->changeActive(FALSE);
    }
    if (printNodes) printf("\n");

    currentRefinementBase = currentRefinementLimit+1;
    autoChangeRefineList();
}

Boolean autoTestRefinements()
{
    int i;
    Boolean previousOK;
    searchHistoryNode *curr;
    extern void setCurrentRefinement(searchHistoryNode*);
    extern Boolean verifyPreviousRefinements();

    previousOK = verifyPreviousRefinements();
    if (!previousOK) {
	// something changed 
	free(refinementOptions);
	refinementOptions = NULL;

	// find a a good new refinement to try back of the SHG. 
	autoSelectRefinements();
	return(TRUE);
    }

    // see if we found one.
    for (i=currentRefinementBase; i <= currentRefinementLimit; i++) {
	if (i >= refineCount) break;
	curr = refinementOptions[i];
	if (curr->getStatus() == TRUE) {

	    setCurrentRefinement(curr);

	    // disable other refinement options.
	    for (i=0; i < refineCount; i++) {
		if (refinementOptions[i] == curr) continue;
		refinementOptions[i]->changeActive(FALSE);
	    }
	    free(refinementOptions);
	    refinementOptions = NULL;

	    // we made a refinement decrement counter.
	    PCautoRefinementLimit--;

	    // advance to next refinement canidate list.
	    autoSelectRefinements();
	    return(TRUE);
	}
    }
    return(FALSE);
}
