/*
 * $Log: PCevalTest.C,v $
 * Revision 1.17  1994/05/31 21:42:58  markc
 * Allow compensationFactor to be computed, but keep it within 0 and 1, which
 * is a short term fix.  Enable the hotSyncObject test in PCrules.C.
 *
 * Revision 1.16  1994/05/31  19:11:41  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.15  1994/05/31  18:43:03  markc
 * Commented out computation of compensation factor due to pause time.  This
 * value is not within 0 and 1, but it should be.  This is a short term fix
 * until pause time is reported properly.
 *
 * Revision 1.14  1994/05/19  00:00:27  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.13  1994/05/18  00:48:52  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.12  1994/05/17  01:13:22  hollings
 * Removed PCcurrentTime = 0.0 from search.
 *
 * Revision 1.11  1994/05/02  20:38:09  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.10  1994/04/27  22:55:01  hollings
 * Merged refine auto and search.
 *
 * Revision 1.9  1994/04/21  04:56:46  karavan
 * added calls to changeStatus and changeActive - klk
 *
 * Revision 1.8  1994/04/20  15:30:17  hollings
 * Added error numbers.
 *
 * Revision 1.7  1994/04/13  01:37:06  markc
 * Added ifdef print statements to see hypothesis checks.
 *
 * Revision 1.6  1994/04/12  15:32:46  hollings
 * generalized hysteresis into a normalization constant to cover pause,
 * contention, and ignored bottlenekcks too.
 *
 * Revision 1.5  1994/04/11  23:19:28  hollings
 * Changed default hyst. to 15%.
 *
 * Revision 1.4  1994/03/01  21:25:09  hollings
 * added tunable constants.
 *
 * Revision 1.3  1994/02/24  04:36:47  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 * Changed header files to reflect changes in igen.
 *
 * Revision 1.2  1994/02/08  21:06:03  hollings
 * Found a few pointer problems.
 *
 * Revision 1.1  1994/02/02  00:38:12  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.11  1993/12/15  21:05:21  hollings
 * better explanations for reset refinement.
 *
 * Revision 1.10  1993/08/30  18:27:02  hollings
 * made redraw only happen when using X.
 *
 * Revision 1.9  1993/08/11  18:53:45  hollings
 * added check to remove.
 *
 * Revision 1.8  1993/08/05  18:54:33  hollings
 * include syntax and general upgrades.
 *
 * Revision 1.7  1993/05/07  20:20:32  hollings
 * upgrade to use dyninst interface.
 *
 * Revision 1.6  1993/02/03  00:06:49  hollings
 * removed execesive friends of focus and focusCell.
 *
 * Revision 1.5  1993/01/28  19:31:08  hollings
 * made SearchHistoryGraph a pointer to a SHG node.
 *
 * Revision 1.4  1992/12/14  19:56:57  hollings
 * added true enable/disable.
 *
 * Revision 1.3  1992/10/23  20:14:51  hollings
 * working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:29:48  hollings
 * fixed rcs log entry.
 *
 * Revision 1.1  1992/08/24  15:06:33  hollings
 * Initial revision
 *
 * Evaluation Engine.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1992 Jeff Hollingsowrth\
  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/Attic/PCevalTest.C,v 1.17 1994/05/31 21:42:58 markc Exp $";
#endif


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../DMthread/DMresource.h"
#include "PCwhy.h"
#include "PCwhere.h"
#include "PCwhen.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "PCglobals.h"
#include "PCauto.h"
#include "performanceConsultant.SRVR.h"

tunableConstant hysteresisRange(0.15, 0.0, 1.0, NULL, "hysteresisRange",
  "Fraction above and bellow threshold that a test should use.");

//
// Fix this soon... This should be based on some real information.
//
tunableConstant minObservationTime(1.0, 0.0, 60.0, NULL, "minObservationTime",
 "min. time (in seconds) to wait after chaning inst to start try hypotheses.");

tunableConstant sufficientTime(6.0, 0.0, 1000.0, NULL, "sufficientTime",
  "How long to wait (in seconds) before we can concule a hypothesis is false.");

int PCsearchPaused;
extern Boolean textMode;
extern int samplesSinceLastChange;
extern timeStamp PClastTestChangeTime;
extern timeStamp PCshortestEnableTime;
Boolean printTestResults = FALSE;
Boolean printNodes = FALSE;

int testResult::operator ==(testResult *arg)
{
    if ((t == arg->t) && (at == arg->at) && (f = arg->f))
	return((int) TRUE);
    else 
	return((int) FALSE);
}

void testResult::print()
{
    t->print();
    f->print();
    at->print(-1);
    printf(" == ");
    if (this->state.status == TRUE) {
	printf("TRUE");
    } else {
	printf("FALSE");
    }
}

//
// check the state of the passed hypothesis to see if it is true at the passed
//   focus.
//
Boolean checkIfTrue(hypothesis *why, 
		    focus *where, 
		    timeInterval *when, 
		    hintList **hints)
{
    Boolean ret;
    testResult *res;
    hypothesisList curr;

    if (!why->testObject) {
	ret = checkPreconditions(why, where, when);
	if (ret == TRUE) {
	    // see if any of its children are true.
	    if (!why->children) return(TRUE);
	    for (curr = *why->children; *curr; curr++) {
		ret = checkIfTrue(*curr, where, when, hints);
		if (ret == TRUE) return(TRUE);
	    }
	    *hints = NULL;
	}
	return(FALSE);
    } else {
	res = currentTestResults->find(why->testObject, where, when);
	if (!res) {
	    // we should have results for the test.
	    *hints = NULL;
	    return(FALSE);
	}
	if (res->state.status == TRUE) {
	    *hints = res->state.hints;
	    // test meet, now check pre Conditions.
	    return(checkPreconditions(why, where, when));
	} else {
	    *hints = NULL;
	    return(FALSE);
	}
    }
}

//
// Check that all of the preCondtions of hypothesis h are meet for the passed
//  focus f.
//
Boolean checkPreconditions(hypothesis *why, focus *where, timeInterval *when)
{
    testResult *val;
    hypothesis *curr;

    for (curr = why->preCondition; curr; curr=curr->preCondition) {
	// see if preConditions are meet.
	if (curr->testObject) {
	    // a hypothesis without a test is considered true.
	    val = currentTestResults->find(curr->testObject, where, when);
	    if (!val) {
		 // Error preCondition 
		 abort();
	    }
	    if (val->state.status != TRUE) break;
	}
    }
    if (!curr) {
	// all preCondtions are meet.
	return(TRUE);
    } else {
	return(FALSE);
    }
}

void testValue::addHint(focus *f, char* message)
{
    int i;
    int limit;
    resource *res;

    limit = f->data ? f->data->getCount() : 0;
    for (i=0; i < limit; i++) {
	res = f->data->getNth(i);
	addHint(res, message);
    }
}

void testValue::addHint(resource *res, char* message)
{
    focus *f;

    // now find the focus that has this focus cell.
    while (res) {
	f = currentFocus->moreSpecific(res);
	if (f) {
	    if (!hints) hints = new(hintList);
	    hints->add(f, message);
	}
	res = res->getParent();
    }
}


Boolean buildTestResultForHypothesis(testResultList *currResults, 
				  testResultList *prevResults,
			          hypothesis *why,
				  focus *where,
				  timeInterval *when,
				  Boolean checkChildren = TRUE);

Boolean buildTestResultForHypothesis(testResultList *currResults, 
				  testResultList *prevResults,
			          hypothesis *why,
				  focus *where,
				  timeInterval *when,
				  Boolean checkChildren)

{
    testResult *r;
    testResult *ret;
    focus *prevFocus;
    hypothesis *curr;
    hypothesisList kids;
    Boolean status = FALSE;

    // ??? what should this value be??
    if (why->labeled) return(TRUE);
    why->labeled = TRUE;
    if (why->testObject) {
	r = new(testResult);
	r->t = why->testObject;
	r->at = when;
	r->f = where;
	r->state.status = FALSE;

	ret = currResults->find(r);
	if (ret) {
	    delete(r);
	    status = ret->ableToEnable;
	} else {
	    ret = prevResults->find(r);
	    if (ret) {
		Boolean found;

		currResults->addUnique(ret);
		found = prevResults->remove(ret);
		assert(found == True);
		status = ret->ableToEnable;
		delete(r);
	    } else {
		// now call enable func.
		prevFocus = currentFocus;
		currentFocus = r->f;
		r->ableToEnable = r->t->changeCollection(enableCollection);

		currentFocus = prevFocus;
		currResults->addUnique(r);
		status = r->ableToEnable;
	    }
	}
    } else if (checkChildren && **(why->children)) {
	assert(why->children); 
	status = FALSE;
	for (kids = *(why->children); curr = *kids; kids++) {
	    status = buildTestResultForHypothesis(currResults, prevResults,
		    curr, where, when) || status;
	}
    } else {
	// we have enabled nothing (sucessfully).
	status = TRUE;
    }
    if (why->preCondition) 
	status = buildTestResultForHypothesis(currResults, prevResults,
	    why->preCondition, where, when, FALSE) && status;
    return(status);
}

//
// build a list of tests to consider, we look through the Search History
//   graph for nodes that are marked as active.
//
void configureTests()
{
    testResult *ct;
    focus *prevFocus;
    searchHistoryNode *curr;
    searchHistoryNodeList currNode;
    testResultList prevTestResults;

    dataMgr->pauseApplication(context);

    // make sure we know who to compensate for data.
    compensationFactor.changeCollection(whereAxis, enableCollection);

    if (currentTestResults) {
	prevTestResults = *currentTestResults;
    }
    currentTestResults = new(testResultList);
    for (currNode = allSHGNodes; curr=*currNode; currNode++) {
	if (curr->getActive()) {
	    whyAxis.unLabel();
	    // Setting beenTested here assumes that a test that is configured 
	    //   get run. We really should not assume this.
	    curr->changeTested(TRUE);
	    curr->ableToEnable = 
	       buildTestResultForHypothesis(currentTestResults,&prevTestResults,
		curr->why, curr->where, curr->when);
	}
    }

    prevFocus = currentFocus;
    for (; ct = *prevTestResults; prevTestResults++) {

	// now call disable func.
	currentFocus = ct->f;
	(void) ct->t->changeCollection(disableCollection);
    }
    currentFocus = prevFocus;

    // delete(prevTestResults);
    dataMgr->continueApplication(context);
}

//
// try the currently enabled tests on all of the data.
//
Boolean evalTests()
{
    Boolean ret;
    float factor;
    testResult *r;
    float normalize;
    float hysteresis;
    testResultList curr;
    Boolean previousStatus;

    factor = (1.0-compensationFactor.value(whereAxis));
    if (factor < 0.0) factor = 0.01;
    assert ((factor <= 1.0) && (factor > 0.0));

    for (curr = *currentTestResults; r=*curr; curr++) {
	// try the test
	// FIX passed arg.
	currentFocus = r->f;
	currentInterval = r->at;

	previousStatus = r->state.status;
	//
	// old hints like old fish are more trouble than value.
	//
	if (r->state.hints) {
	    delete(r->state.hints);
	    r->state.hints = NULL;
	}

	if (r->ableToEnable) {
	    if (printTestResults) {
		printf("evaluate ");
		r->t->print();
		r->f->print();
		printf(" at time %f\n", PCcurrentTime);
	    }
	    if (r->state.status == TRUE) {
		hysteresis = 1.0 - hysteresisRange.getValue();
	    } else {
		hysteresis = 1.0 + hysteresisRange.getValue();
	    }

	    // allow for "compensated" time.
	    normalize = hysteresis * factor;
	    (r->t->evaluate(&(r->state), normalize));
	}
	// always return for now
	// this is done to pick up changes in shg that are due to
	// refinements, but that don't require additional tests.
	ret = TRUE;
	if ((r->state.status == TRUE) && (previousStatus == FALSE)) {
	    if (printTestResults) {
		printf("TEST RETURNED TRUE: ");
		r->t->print();
		r->f->print();
		r->at->print(-1);
		printf(" at time %f\n", PCcurrentTime);
		printf("false from %f to %f\n", r->time, PCcurrentTime);
	    }
	    r->time = PCcurrentTime;
	    ret = TRUE;
	} else if (r->state.status != previousStatus) {
	    if (printTestResults) {
		printf("TEST BECAME FALSE: ");
		r->t->print();
		r->f->print();
		r->at->print(-1);
		printf(" at time %f\n", PCcurrentTime);
		printf("true from %f to %f\n", r->time, PCcurrentTime);
	    }
	    r->time = PCcurrentTime;
	    ret = TRUE;
	}
    }
    return(ret);
}

void hintList::add(focus *f, char* message)
{
    hint *h;
    Boolean newItem;

    h = new(hint);
    h->why = NULL;
    h->where = f;
    h->when = NULL;
    h->message = message;

    newItem = addUnique(h, f);
    if (!newItem) {
	delete(h);
    }
}

void hintList::add(hypothesis *w, char* message)
{
    hint *h;
    Boolean newItem;

    h = new(hint);
    h->why = w;
    h->where = NULL;
    h->when = NULL;
    h->message = message;

    newItem = addUnique(h, w);
    if (!newItem) {
	delete(h);
    }
}

void hintList::add(timeInterval *ti, char* message)
{
    hint *h;
    Boolean newItem;

    h = new(hint);
    h->why = NULL;
    h->where = NULL;
    h->when = ti;
    h->message = message;

    newItem = addUnique(h, ti);
    if (!newItem) {
	delete(h);
    }
}

//
// call search and then check the status of each active shg node.
//
Boolean doScan()
{
    Boolean change;
    Boolean ret;
    hintList currHint;
    Boolean shgStateChanged;
    searchHistoryNode *shgNode;
    searchHistoryNodeList curr;

    if (PCcurrentTime >= whenAxis.end) return(False);

    shgStateChanged = FALSE;
    change = evalTests();
    if (change) {
	// update status of hypotheses.
	for (curr = allSHGNodes; shgNode = *curr; curr++) {
	    // printf("checking shg node ");
	    // (*curr)->print(-1);
	    // printf("\n");

	    if (shgNode->getActive() == FALSE) continue;
	    assert(shgNode->why);
	    ret = checkIfTrue(shgNode->why, shgNode->where, 
		shgNode->when, &shgNode->hints);
	    if (shgNode->getStatus() != ret) {
		shgNode->changeStatus(ret);

		if (printNodes) {
		    printf("SHG Node %d ", shgNode->nodeId);
		    if (ret == TRUE) {
			printf(" TRUE at time %f\n", PCcurrentTime);
			if (shgNode->hints) {
			    for (currHint = *shgNode->hints; 
				 *currHint; 
				 currHint++){
				(*currHint)->print();
			    }
			}
		    } else {
			printf(" FALSE at time %f\n", PCcurrentTime);
		    }
		}
		if (ret == FALSE) {
		    delete(shgNode->hints);
		    shgNode->hints = NULL;
		}
		shgStateChanged = TRUE;
	    }
	}
    }
    return(shgStateChanged);
}

int PCpathMax;
int PCpathDepth;
extern int PCsearchPaused;
extern int PCautoRefinementLimit;
searchHistoryNode **PCrefinementPath;
extern void defaultExplanation(searchHistoryNode *explainee);

void setCurrentRefinement(searchHistoryNode *curr)
{
    int i;

    if (curr == SearchHistoryGraph) {
	// reseting search.
	PCpathDepth = 0;
    }

    // check for cycles in refinement path.
    for (i=0; i < PCpathDepth; i++) {
       assert(PCrefinementPath[i] != curr);
    }

    currentSHGNode = curr;

    // push old currentSHGNode on refinement path.
    if (!PCrefinementPath) {
	PCpathMax = 10;
	PCrefinementPath = (searchHistoryNode **)
	    calloc(PCpathMax, sizeof(searchHistoryNode*));
    } else if (PCpathMax == PCpathDepth) {
	PCpathMax += 10;
	PCrefinementPath = (searchHistoryNode **)
	    realloc(PCrefinementPath, PCpathMax * sizeof(searchHistoryNode*));
    }
    PCrefinementPath[PCpathDepth++] = curr;
}

Boolean verifyPreviousRefinements()
{
    int i;
    searchHistoryNode *curr;
    searchHistoryNodeList currNode;

    // verify that the refinement path we took is still true;
    for (i=0; i < PCpathDepth; i++) {
	if (PCrefinementPath[i]->getStatus() != TRUE) {
	    // pauseApplication(context);

	    printf("The bottleneck we were refining changed\n");
	    printf("The bottleneck was:");
	    defaultExplanation(PCrefinementPath[i]);

	    // disable all nodes.
	    for (currNode = allSHGNodes; curr = *currNode; currNode++) {
		curr->changeTested(FALSE);
		curr->changeActive(FALSE);
	    }

	    // Find the last point down the PCrefinementPath that has all
	    // previous nodes true.
	    SearchHistoryGraph->changeActive(TRUE);
	    currentSHGNode = SearchHistoryGraph;
	    for (i=0; i < PCpathDepth; i++) {
		if (PCrefinementPath[i]->getStatus() == TRUE) {
		    currentSHGNode = PCrefinementPath[i];
		    currentSHGNode->changeTested(TRUE);
		    currentSHGNode->changeActive(TRUE);
		} else {
		    // the current one is not true so stop marking.
		    PCpathDepth = i;
		    break;
		}
	    }

	    // reset status of disabled nodes.
	    for (currNode = allSHGNodes; curr = *currNode; currNode++) {
		if (curr->getActive() == FALSE) curr->changeStatus(FALSE);
	    }

	    printf("The search has been reset to the bottleneck\n    ");
	    defaultExplanation(currentSHGNode);

	    return(FALSE);
	}
    }
    return(TRUE);
}


void PCevaluateWorld()
{
    Boolean changed;

    //
    // see that we are actively searching before trying to eval tests!
    //
    if (PCsearchPaused) {
	return;
    }

    //
    // wait minObservationTime between calls to eval.
    //
    if (PCcurrentTime >= PClastTestChangeTime + minObservationTime.getValue()) {
	/* try to evaluate a test */
	changed = doScan();
	if (PCautoRefinementLimit != 0) {
	    if (changed) {
		autoTestRefinements();
	    } else if ((PCshortestEnableTime > sufficientTime.getValue()) &&
		       (samplesSinceLastChange > sufficientTime.getValue())) {
		// we have waited sufficient observation time move on.
		printf("autorefinement timelimit reached at %f\n", 
		    PCcurrentTime);
		printf("samplesSinceLastChange = %d\n", samplesSinceLastChange);
		printf("shortest enable time = %f\n", PCshortestEnableTime);
		autoTimeLimitExpired();
	    }
	} else if (changed) {
	    if (verifyPreviousRefinements() == TRUE) {
		dataMgr->pauseApplication(context);
		PCsearchPaused = TRUE;
		printf("application paused\n");
	    } else {
		// previous refinement now false.
		dataMgr->pauseApplication(context);
	    }
	}
    }
}


void performanceConsultant::search(Boolean stopOnChange, int limit)
{
    extern void autoSelectRefinements();

    PClastTestChangeTime = PCcurrentTime;
    if (!dataMgr->applicationDefined(context)) {
	printf("must specify application to run first\n");
	return;
    }
    if (currentTestResults) 
	delete(currentTestResults); 
    currentTestResults = NULL;

    if (currentSHGNode->getStatus() != TRUE) {
	SearchHistoryGraph->resetStatus();
	SearchHistoryGraph->resetActive();
	configureTests();
    }

    // refine one step now and then let it go.
    printf("setting limit to %d\n", limit);
    PCsearchPaused = FALSE;
    PCautoRefinementLimit = limit;
    autoSelectRefinements();
    dataMgr->continueApplication(context);
}

void performanceConsultant::pauseSearch()
{
     PCsearchPaused = TRUE;
     PCautoRefinementLimit = 0; 
}

//
// returns 0 if able to set it.
//
int performanceConsultant::setCurrentSHGnode(int node)
{
    int i;
    searchHistoryNode *n;
    searchHistoryNodeList curr;

    for (curr = allSHGNodes; n = *curr; curr++) {
	 if (n->nodeId == node) {
	     break;
	 }
    }
    if (!n) {
	printf("paradyn Error #8\n");
	return(-1);
    }
    if (n->getActive() != TRUE) {
	printf("Node %d is not active \n", node);
	return(-1);
    }
    if (n->getStatus() != TRUE) {
	printf("Node %d is not true \n", node);
	return(-1);
    }
    for (i=0; i < PCpathDepth; i++) {
        if (PCrefinementPath[i]->getStatus() != TRUE) {
            printf("search history graph ancestor not true\n");
            printf("paradyn Error #9\n");
            return(-1);
        }
    }

    setCurrentRefinement(n);
    return(0);
}

//
// now the module globals.
//
testResultList *currentTestResults;

stringPool testResultList::resultPool;

