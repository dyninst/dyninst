/*
 * $Log: PCevalTest.h,v $
 * Revision 1.6  1994/09/05 20:00:53  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.5  1994/08/05  16:04:11  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.4  1994/08/03  19:09:48  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.3  1994/03/01  21:25:10  hollings
 * added tunable constants.
 *
 * Revision 1.2  1994/02/03  23:27:01  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:38:13  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.5  1993/08/11  19:08:27  hollings
 * added remove.
 *
 * Revision 1.4  1993/08/05  18:59:42  hollings
 * added setCurrentRefinement proto.
 *
 * Revision 1.3  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.2  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.1  1992/08/24  15:12:20  hollings
 * Initial revision
 *
 * 
 * Interface to evaluation engine.
 *
 */
#ifndef EVAL_H
#define EVAL_H

#include "util/h/tunableConst.h"

//
// What tests return.
//
class testValue {
    public:
	testValue() { status = FALSE; }
	void addHint(focus*, char*);
	void addHint(resource*, char*);
	void addHint(hypothesis*, char*);
	void addHint(timeInterval*, char*);

	Boolean status;
	hintList *hints;
};

class hypothesisValue {
    public:
	hypothesisValue() { status = FALSE; }
	Boolean status;
};

//
// the result of a single test on a single focus.
//
class testResult {
    public:
	testResult() 	{ state.hints = NULL; time = 0.0; ableToEnable = FALSE;}
	void print();
	operator ==(testResult *);
	testValue state;
	test *t;		// which test
	focus *f;		// at this focus
	timeInterval *at;	// for this interval.
	Histogram *hist;
	timeStamp time;		// time it last changed
	Boolean ableToEnable;	// could we turn it on?
	List <datum*> *metFociUsed;	// what does this test need to run.
};

class testResultList: public List<testResult*> {
    public:
	Boolean addUnique(testResult *res) { 
	    char str[80];
	    stringHandle id;
	    testResult *ret;

	    sprintf(str, "%d %d %d", res->t, res->f, res->at);
	    id = resultPool.findAndAdd(str);
	    ret = List<testResult*>::find(id);
	    if (ret) {
		return(0);
	    } else {
		List<testResult*>::addUnique(res, id);
		return(1);
	    }
	}
	testResult *find(testResult *res) { 
	    return(find(res->t, res->f, res->at));
	}
	testResult *find(test *t, focus *f, timeInterval *when) { 
	    char str[80];
	    stringHandle id;

	    sprintf(str, "%d %d %d", t, f, when);
	    id = resultPool.findAndAdd(str);
	    return (List<testResult*>::find(id)); 
	}
        Boolean remove(testResult *res) {
            char str[80];
            stringHandle id;
  
            sprintf(str, "%d %d %d", res->t, res->f, res->at);
            id = resultPool.findAndAdd(str);
            return List<testResult*>::remove(id);
	}

    private:
	static stringPool resultPool;
};

extern Boolean doScan();
extern void configureTests();
extern tunableFloatConstant hysteresisRange;

extern testResultList *currentTestResults;

extern void setCurrentRefinement(searchHistoryNode*);

#endif
