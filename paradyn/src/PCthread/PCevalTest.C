
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
 * $Log: PCevalTest.C,v $
 * Revision 1.40  1995/11/09 02:07:55  tamches
 * removed #include of uistatdisp.h, an obsolete file
 *
 * Revision 1.39  1995/10/05 04:38:43  karavan
 * changed igen calls to accommodate new PC|UI interface.
 *
 * Revision 1.38  1995/07/17  04:28:58  tamches
 * Changed whereAxis to pcWhereAxis, avoiding a naming conflict with the
 * new UI where axis.
 *
 * Revision 1.37  1995/06/02  20:50:06  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.36  1995/02/27  19:17:26  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.35  1995/02/16  08:19:05  markc
 * Changed Boolean to bool
 *
 * Revision 1.34  1995/01/26  17:58:36  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.33  1994/12/21  00:46:29  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.32  1994/11/11  10:46:26  markc
 * Used status line to print status
 *
 * Revision 1.31  1994/11/09  18:39:40  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.30  1994/11/04  13:02:46  markc
 * Moved calculation of the compensationFactor to the inner loop so each hypothesis
 * will get get a normalization factor for the interval over which it is collected.
 *
 * Revision 1.29  1994/10/25  22:08:01  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.28  1994/09/22  00:59:35  markc
 * Added const to char* args for void testValue::addHint(focus *f, const char* message)
 *
 * Revision 1.27  1994/09/05  20:00:50  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.26  1994/08/03  19:09:47  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.25  1994/07/26  20:03:31  hollings
 * added resetActiveFlag
 *
 * Revision 1.24  1994/07/25  04:47:04  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.23  1994/07/22  19:25:44  hollings
 * removed supress SHG option for now.
 *
 * Revision 1.22  1994/07/14  23:49:49  hollings
 * added batch mode for SHG, fixed the sort function to use beenTrue.
 *
 * Revision 1.21  1994/06/29  02:56:19  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.20  1994/06/22  22:58:18  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.19  1994/06/14  15:28:52  markc
 * Added conflict flag to moreSpecific procedure since a desired magnification
 * may conflict with the current focus (if the desired magnification has the
 * same base as the focus, but one is not an ancestor of the other).
 *
 * Allowed compensationFactor to be computed from pause_time since pause_time
 * is now aggregated and averaged.
 *
 * Revision 1.18  1994/06/12  22:40:47  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.17  1994/05/31  21:42:58  markc
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream.h>

#include "../TCthread/tunableConst.h"
#include "../pdMain/paradyn.h"
#include "../DMthread/DMresource.h"
#include "PCwhy.h"
#include "PCwhere.h"
#include "PCwhen.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "PCglobals.h"
#include "PCauto.h"
#include "performanceConsultant.thread.SRVR.h"

// bool printTestResults = false;
// bool printNodes = false;

int PCsearchPaused=1;
extern bool textMode;
List<datum *> *enabledGroup;
extern timeStamp PCendTransTime;
extern timeStamp PCstartTransTime;
extern int samplesSinceLastChange;
extern timeStamp PClastTestChangeTime;
extern timeStamp PCshortestEnableTime;

bool testResult::operator == (testResult *arg)
{
    if ((t == arg->t) && (at == arg->at) && (f = arg->f))
	return(true);
    else 
	return(false);
}

ostream& operator <<(ostream &os, testResult & tr)
{
    os << tr.t;
    os << tr.f;
    os << tr.at;
    cout << " == ";
    if (tr.state.status == true) {
	cout << "TRUE";
    } else {
	cout << "FALSE";
    }

    return os; // added AT 12/8/94
}

//
// check the state of the passed hypothesis to see if it is true at the passed
//   focus.
//
bool checkIfTrue(hypothesis *why, 
		    focus *where, 
		    timeInterval *when, 
		    hintList **hints)
{
    bool ret;
    testResult *res;
    hypothesisList curr;

    if (!why->testObject) {
	ret = checkPreconditions(why, where, when);
	if (ret) {
	    // see if any of its children are true.
	    if (!why->children) return(true);
	    for (curr = *why->children; *curr; curr++) {
		ret = checkIfTrue(*curr, where, when, hints);
		if (ret) return(true);
	    }
	    *hints = NULL;
	}
	return(false);
    } else {
	res = currentTestResults->find(why->testObject, where, when);
	if (!res) {
	    // we should have results for the test.
	    *hints = NULL;
	    return(false);
	}
	if (res->state.status) {
	    *hints = res->state.hints;
	    // test meet, now check pre Conditions.
	    return(checkPreconditions(why, where, when));
	} else {
	    *hints = NULL;
	    return(false);
	}
    }
}

//
// Check that all of the preCondtions of hypothesis h are meet for the passed
//  focus f.
//
bool checkPreconditions(hypothesis *why, focus *where, timeInterval *when)
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
	    if (!val->state.status) break;
	}
    }
    if (!curr) {
	// all preCondtions are meet.
	return(true);
    } else {
	return(false);
    }
}

void testValue::addHint(focus *f, const char* message)
{
    int i;
    int limit;
    resource *res;

    limit = f->data ? f->data->getCount() : 0;
    for (i=0; i < limit; i++) {
	resourceHandle *r_handle;
	if(f->data->getNth(i,r_handle)){
	    res = resource::handle_to_resource(*r_handle);
	    addHint(res, message);
	}
    }
}

void testValue::addHint(resource *res, const char* message)
{
    focus *f;
    bool conflicts;
    char *newM;

    if (message)
      newM = strdup(message);
    else
      newM = 0;
    // now find the focus that has this focus cell.
    while (res) {
	f = currentFocus->moreSpecific(res, conflicts);
	if (f) {
	    if (!hints) hints = new(hintList);
	    hints->add(f, newM);
	}
	resourceHandle parent = res->getParent();
	if(parent)
	    res = resource::handle_to_resource(parent);
        else
	    res = NULL;
    }
}


bool buildTestResultForHypothesis(testResultList *currResults, 
				  testResultList *prevResults,
			          hypothesis *why,
				  focus *where,
				  timeInterval *when,
				  bool checkChildren = true);

bool buildTestResultForHypothesis(testResultList *currResults, 
				  testResultList *prevResults,
			          hypothesis *why,
				  focus *where,
				  timeInterval *when,
				  bool checkChildren)

{
    testResult *r;
    testResult *ret;
    focus *prevFocus;
    hypothesis *curr;
    hypothesisList kids;
    bool status = false;

    // ??? what should this value be??
    if (why->labeled) return(true);
    why->labeled = true;
    if (why->testObject) {
	r = new(testResult);
	r->t = why->testObject;
	r->at = when;
	r->f = where;
	r->state.status = false;
	r->metFociUsed = enabledGroup;
	ret = currResults->find(r);
	if (ret) {
	    delete(r);
	    status = ret->ableToEnable;
	} else {
	    ret = prevResults->find(r);
	    if (ret) {
		bool found;

		currResults->addUnique(ret);
		found = prevResults->remove(ret);
		assert(found);
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
	status = false;
	for (kids = *(why->children); curr = *kids; kids++) {
	    status = buildTestResultForHypothesis(currResults, prevResults,
		    curr, where, when) || status;
	}
    } else {
	// we have enabled nothing (sucessfully).
	status = true;
    }
    if (why->preCondition) 
	status = buildTestResultForHypothesis(currResults, prevResults,
	    why->preCondition, where, when, false) && status;
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

    dataMgr->pauseApplication();

    // make sure we know who to compensate for data.
    enabledGroup = new(List<datum*>);
    compensationFactor.changeCollection(pcWhereAxis, enableCollection);

    delete(enabledGroup);
    enabledGroup = NULL;

    if (currentTestResults) {
	prevTestResults = *currentTestResults;
    }
    currentTestResults = new(testResultList);
    for (currNode = allSHGNodes; curr=*currNode; currNode++) {
	if (curr->getActive()) {
	    whyAxis.unLabel();
	    // Setting beenTested here assumes that a test that is configured 
	    //   get run. We really should not assume this.
	    curr->changeTested(true);
	    curr->metricFoci = new (List<datum*>);
	    enabledGroup = curr->metricFoci;
	    curr->ableToEnable = 
	       buildTestResultForHypothesis(currentTestResults,&prevTestResults,
		curr->why, curr->where, curr->when);
	    enabledGroup = NULL;
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
    dataMgr->continueApplication();
}

//
// Compute the lowest common demoninator of the valid time interval for the
//   passed datum items.
//
void getTimes(timeStamp *start, timeStamp *end, List<datum*> *items)
{
    datum *d;
    List<datum*> curr;

    curr = *items;

    *start = (*curr)->firstSampleTime;
    *end = (*curr)->lastSampleTime;

    for (; d = *curr; curr++) {
        if ((d->firstSampleTime) && (d->firstSampleTime > *start)) {
            *start = d->firstSampleTime;
        }

        // earliest last sample
        if ((d->enabled) && (d->lastSampleTime < *end)) {
            *end = d->lastSampleTime;
        }
    }
}

//
// try the currently enabled tests on all of the data.
//
bool evalTests()
{
    bool ret=false;
    float factor;
    testResult *r;
    float normalize;
    float hysteresis;
    testResultList curr;
    bool previousStatus;
    float fctr;

    // get a fresh value of "pcEvalPrint", "printTestResults", and "hysteresisRange"
    tunableBooleanConstant pcEvalPrint = tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    tunableBooleanConstant printTestResults = tunableConstantRegistry::findBoolTunableConstant("printTestResults");
    tunableFloatConstant hysteresisRange = tunableConstantRegistry::findFloatTunableConstant("hysteresisRange");

#ifdef notdef
    // this is incorrect here.  Each hypothesis has a different time interval
    // and should get a different compensationFactor.
    factor = (1.0-(fctr=compensationFactor.value(pcWhereAxis)));
    // TODO mdc
    cout << "\n\npaused= " << fctr << "   running=  " << 1.0 - fctr << endl;
    // if (factor < 0.0) factor = 0.01;
    // assert ((factor <= 1.0) && (factor > 0.0));
#endif

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
	    if (printTestResults.getValue()) {
		cout << "evaluate ";
		cout << *(r->t);
		cout << *(r->f);
		cout << " at time " << PCcurrentTime << endl;
	    }
	    if (r->state.status) {
		hysteresis = 1.0 - hysteresisRange.getValue();
	    } else {
		hysteresis = 1.0 + hysteresisRange.getValue();
	    }

	    // define the correct time interval.
	    assert(r->metFociUsed);
	    getTimes(&PCstartTransTime, &PCendTransTime, r->metFociUsed);
	    if (pcEvalPrint.getValue()) {
		cout << PCstartTransTime << " to " << PCendTransTime << " ";
	    }

	    // NEW CODE
	    factor = (1.0-(fctr=compensationFactor.value(pcWhereAxis)));
	    if (pcEvalPrint.getValue()) 
	      cout << "\n\npaused= " << fctr << "   running=  " << 1.0 - fctr << endl;
	    if (factor < 0.0) factor = 0.00001;
	    if (factor > 1.0) factor = 0.99999;
	    // NEW CODE

	    // allow for "compensated" time.
	    normalize = hysteresis * factor;

	    (r->t->evaluate(&(r->state), normalize));
	}
	// always return for now
	// this is done to pick up changes in shg that are due to
	// refinements, but that don't require additional tests.
	ret = true;
	if (r->state.status && !previousStatus) {
	    if (printTestResults.getValue()) {
		cout << "TEST RETURNED TRUE: \n";
		cout << *(r->t);
		cout << *(r->f);
		cout << " ";
		cout << *(r->at);
		cout << endl;
		cout << "     at time " << PCcurrentTime << endl;
		cout << "     false from " << r->time << " to ";
		cout << PCcurrentTime << endl;
	    }
	    r->time = PCcurrentTime;
	    ret = true;
	} else if (r->state.status != previousStatus) {
	    if (printTestResults.getValue()) {
		cout << "TEST BECAME FALSE: ";
		cout << *(r->t);
		cout << *(r->f);
		cout << *(r->at);
		cout << endl;
		cout << "     at time " << PCcurrentTime << endl;
		cout << "     true from " << r->time << " to ";
		cout << PCcurrentTime << endl;
	    }
	    r->time = PCcurrentTime;
	    ret = true;
	}
    }
    return(ret);
}

void hintList::add(focus *f, char* message)
{
    hint *h;
    bool newItem;

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
    bool newItem;

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
    bool newItem;

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
bool doScan()
{
    bool change;
    bool ret;
    hintList currHint;
    bool shgStateChanged;
    searchHistoryNode *shgNode;
    searchHistoryNodeList curr;

    // get a fresh value of tunable constant "printNodes"
    tunableBooleanConstant printNodes = tunableConstantRegistry::findBoolTunableConstant("printNodes");

    if (PCcurrentTime >= whenAxis.end) return(false);

    shgStateChanged = false;
    change = evalTests();
    if (change) {
	// update status of hypotheses.
	for (curr = allSHGNodes; shgNode = *curr; curr++) {
	    if (!shgNode->getActive()) continue;
	    assert(shgNode->why);
	    ret = checkIfTrue(shgNode->why, shgNode->where, 
		shgNode->when, &shgNode->hints);
	    if (shgNode->getStatus() != ret) {
		shgNode->changeStatus(ret);

		if (printNodes.getValue()) {
		    cout << "SHG Node " << shgNode->nodeId;
		    if (ret) {
			cout << " TRUE at time " << PCcurrentTime << endl;
			if (shgNode->hints) {
			    for (currHint = *shgNode->hints; 
				 *currHint; 
				 currHint++){
				cout << *(*currHint);
			    }
			}
		    } else {
			cout << "FALSE at time " << PCcurrentTime << endl;
		    }
		}
		if (!ret) {
		    delete(shgNode->hints);
		    shgNode->hints = NULL;
		}
		shgStateChanged = true;
	    }
	}
    }
    return(shgStateChanged);
}

int PCpathMax;
int PCpathDepth;
extern int UIM_BatchMode;
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


//
// mark current node and decendents as status = false.
//
void resetActiveFlag(searchHistoryNode *curr)
{
    searchHistoryNodeList currNode;

    if (curr->children) {
	for (currNode = *curr->children; curr = *currNode; currNode++) {
	    if (!curr->getActive()) curr->changeStatus(false);
	    resetActiveFlag(curr);
	}
    }
}

bool verifyPreviousRefinements()
{
    int i;
    searchHistoryNode *curr;
    searchHistoryNodeList currNode;

    // verify that the refinement path we took is still true;
    for (i=0; i < PCpathDepth; i++) {
	if (!PCrefinementPath[i]->getStatus()) {
	    // pauseApplication();

	  uiMgr->updateStatusDisplay 
	    (SHGid, "The bottleneck we were refining changed\nThe bottleneck was: ");
	  
	    // disable all nodes.
	    UIM_BatchMode++;
	    for (currNode = allSHGNodes; curr = *currNode; currNode++) {
		curr->changeTested(false);
		curr->changeActive(false);
	    }
	    UIM_BatchMode--;

	    // Find the last point down the PCrefinementPath that has all
	    // previous nodes true.
	    SearchHistoryGraph->changeActive(true);
	    currentSHGNode = SearchHistoryGraph;
	    for (i=0; i < PCpathDepth; i++) {
		if (PCrefinementPath[i]->getStatus()) {
		    currentSHGNode = PCrefinementPath[i];
		    currentSHGNode->changeTested(true);
		    currentSHGNode->changeActive(true);
		} else {
		    // the current one is not true so stop marking.
		    PCpathDepth = i;
		    break;
		}
	    }

	    // reset status of disabled nodes decended from the new current
	    // refinement.   These are things we might have rejected before,
	    // but due to backing up the search we want to consider them 
	    // again.
	    if (currentSHGNode->children) {
		for (currNode = *currentSHGNode->children; curr = *currNode; 
		     currNode++) {
		    resetActiveFlag(curr);
		    if (!curr->getActive()) curr->changeStatus(false);
		}
	    }
	  uiMgr->updateStatusDisplay 
	    (SHGid, "The search has been reset to the bottleneck\n");
	  defaultExplanation(currentSHGNode);
	  return(false);
	}
    }
    return(true);
}


void PCevaluateWorld()
{
    bool changed;
    bool autoTestRefinements();
    void autoTimeLimitExpired();

    // get fresh value of tunable constants "minObservationTime", "sufficientTime",
    // 
    tunableFloatConstant minObservationTime = tunableConstantRegistry::findFloatTunableConstant("minObservationTime");
    tunableFloatConstant sufficientTime = tunableConstantRegistry::findFloatTunableConstant("sufficientTime");

    //
    // see that we are actively searching before trying to eval tests!
    //
    if (PCsearchPaused) {
//        printf("Returning: PCsearchPaused=TRUE\n");
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
	      char buffer[500];
	      sprintf(buffer, "autorefinement timelimit reached at %f\nsamplesSinceLastChange = %d\nshortest enable time = %f\n",
		      PCcurrentTime, 
		      samplesSinceLastChange, 
		      PCshortestEnableTime);

	      uiMgr->updateStatusDisplay (SHGid, buffer);	     
	      autoTimeLimitExpired();
	    }
	} else if (changed) {
	    if (verifyPreviousRefinements()) {
		dataMgr->pauseApplication();
		PCsearchPaused = true;
		uiMgr->updateStatusDisplay (SHGid, "application paused\n");
	    } else {
		// previous refinement now false.
		dataMgr->pauseApplication();
	    }
	}
    }
}


void performanceConsultant::search(bool stopOnChange, int limit, int phaseID)
{
    extern void autoSelectRefinements();

    PClastTestChangeTime = PCcurrentTime;
    PCstartTransTime = PCcurrentTime;
    samplesSinceLastChange = 0;
    char buffer[100];
    sprintf(buffer, "Setting PC start search time to %f\n", PCstartTransTime);
    uiMgr->updateStatusDisplay(SHGid, buffer);
    if (!dataMgr->applicationDefined()) {
      uiMgr->updateStatusDisplay(SHGid,
				 "must specify application to run first\n");
      return;
    }
    if (currentTestResults) 
	delete(currentTestResults); 
    currentTestResults = NULL;

    if (!currentSHGNode->getStatus()) {
	SearchHistoryGraph->resetStatus();
	SearchHistoryGraph->resetActive();
//** this doesn't seem necessary	configureTests();
    }

    // refine one step now and then let it go.
    sprintf(buffer, "setting limit to %d\n", limit);
    uiMgr->updateStatusDisplay (SHGid, buffer);
    PCsearchPaused = false;
    PCautoRefinementLimit = limit;
    autoSelectRefinements();
    dataMgr->continueApplication();
}

void performanceConsultant::pauseSearch(int phaseID)
{
     PCsearchPaused = true;
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
    if (!n->getActive()) {
	printf("Node %d is not active \n", node);
	return(-1);
    }
    if (!n->getStatus()) {
	printf("Node %d is not true \n", node);
	return(-1);
    }
    for (i=0; i < PCpathDepth; i++) {
        if (!PCrefinementPath[i]->getStatus()) {
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

