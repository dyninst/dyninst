/*
 * 
 * $Log: PCmetric.h,v $
 * Revision 1.15  1995/02/16 08:19:15  markc
 * Changed Boolean to bool
 *
 * Revision 1.14  1995/01/26  17:58:41  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.13  1994/11/09  18:39:44  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.12  1994/10/25  22:08:06  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.11  1994/09/22  01:03:47  markc
 * Added const char* to PCmetric constructor
 *
 * Revision 1.10  1994/08/05  16:04:14  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.9  1994/08/03  19:09:52  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.8  1994/07/25  04:47:08  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.7  1994/07/02  01:43:38  markc
 * Remove aggregation operator from enableDataCollection call.
 *
 * Revision 1.6  1994/06/14  15:30:26  markc
 * Moved aggregationOperator defines to util directory.
 *
 * Revision 1.5  1994/05/18  00:48:55  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.4  1994/05/02  20:38:12  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.3  1994/04/12  15:32:48  hollings
 * generalized hysteresis into a normalization constant to cover pause,
 * contention, and ignored bottlenekcks too.
 *
 * Revision 1.2  1994/02/24  04:36:49  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 * Changed header files to reflect changes in igen.
 *
 * Revision 1.1  1994/02/02  00:38:17  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.8  1993/09/03  19:02:42  hollings
 * added findMetric, removed 4th arg to value.
 *
 * Revision 1.7  1993/08/30  18:36:29  hollings
 * added getCollectionCost to collectMode
 * added samplesSinceEnable to datum.
 *
 * Revision 1.6  1993/08/05  19:00:28  hollings
 * added getUnits and new includes.
 *
 * Revision 1.5  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.4  1992/12/14  19:58:27  hollings
 * added true enable/disable.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:12:20  hollings
 * first cut at automated refinement.
 *
 * Revision 1.1  1992/08/03  20:45:54  hollings
 * Initial revision
 *
 *
 */

#ifndef METRIC_H
#define METRIC_H

#include "util/h/hist.h"
#include "util/h/aggregation.h"
#include "dataManager.thread.CLNT.h"
#include "PCwhere.h"
#include "PCwhen.h"

extern dataManagerUser *dataMgr;

typedef enum { 
    getCollectionCost, 
    enableCollection, 
    disableCollection
} collectMode;


// How do we compine multipe metrics of this type
// enum aggregation { Sum, Min, Max, Avg };
// moved to util/h/aggregateSample.h

// do we normalize by the windowWidth
enum calcType { CalcSum, CalcNormalize };

enum warningLevel { wNever, wOnce, wAlways };

extern warningLevel noDataWarning;
extern timeStamp PCcurrentTime;

class datum {
    public:
	void newSample(timeStamp start, timeStamp end, sampleValue value);

	datum();
	focus   	*f;
	resourceList	*resList;		// maps to inst level data.

	// data for previous enables.
	sampleValue   	sample;			// total of previous enables.
	timeStamp	totalUsed;		// total time enable previous.

	timeStamp	firstSampleTime;	// time the first sample arrived.
	timeStamp	lastSampleTime;		// this time enabled.
	timeStamp	lastUsed;		// this time enabled.
	int		samplesSinceEnable;	// this time enabled.

	timeStamp	time;
	bool		used;
	bool		enabled;
	int		refCount;
	metricInstance	*mi;			// when enabled.

	Histogram	*hist;
	stringHandle	metName;
};

class performanceConsultant;
//
// The name of some performance data.
//
class PCmetric {
	friend void printMetric(char *, focus*, timeStamp, timeStamp);
	friend void PCmetricFunc(performanceStream *ps, metric *met);
	friend void printStats();
	friend void globalCost();
	friend ostream& operator <<(ostream &os, PCmetric& );
    public:
	// Constructor - default is Sum to aggregate.

	stringHandle getName() { return(name); }
	const char *getUnits() { 
	    T_dyninstRPC::metricInfo *info;

	    info = dataMgr->getMetricInfo(met);
	    return(info->units.string_of()); 
	}
	datum *findDatum(focus *f) {
	  return(samples.find(f->getName()));
	}
	void addDatum(datum *d) { 
	    samples.add(d, (void *) d->f->getName()); 
	}

	bool changeCollection(collectMode);		// currentFocus
	bool changeCollection(focus*, collectMode);	// explicit focus.
	bool changeCollection(focusList, collectMode);// explicit focus.
	PCmetric(const char *, int, calcType);
	PCmetric(const char *);

	// Get back values.
	sampleValue value();			// for current global state.
	sampleValue value(focus*);		// for a single focus object.
	sampleValue value(focusList);		// apply default aggregation.
	sampleValue value(timeStamp,timeStamp);	// for current global state.

	// is this enabled?
	bool enabled(focus*);		// for a single focus object.

	// for a single focus object.
	sampleValue value(focus*,timeStamp,timeStamp);

	// apply default aggregation.
	sampleValue value(focusList,timeStamp,timeStamp);	


	// explicit calls to aggregation.
	sampleValue avg(focusList, timeStamp, timeStamp);
	sampleValue min(focusList, timeStamp, timeStamp);
	sampleValue max(focusList, timeStamp, timeStamp);
	sampleValue sum(focusList, timeStamp, timeStamp);

	//
	sampleValue avg(focusList fl) { 
	     return avg(fl, currentInterval->start, PCcurrentTime); 
	}
	sampleValue min(focusList fl) { 
	    return avg(fl, currentInterval->start, PCcurrentTime); 
	}
	sampleValue max(focusList fl) { 
	    return avg(fl, currentInterval->start, PCcurrentTime); 
	}
	sampleValue sum(focusList fl) { 
	    return avg(fl, currentInterval->start, PCcurrentTime); 
	}
    private:
	stringHandle name;
	metric	*met;		// handle to inst.
	List<datum*> samples;
	int aggregationOperator;
	calcType calc;
};

ostream& operator <<(ostream &os, PCmetric& );

extern PCmetric *findMetric(char *name);

//
// list of PCmetric.
//
typedef HTable<PCmetric*> PCmetricList;

typedef HTable<datum*> datumList;

extern datumList miToDatumMap;
extern void aggregateHist();

// list each PCmetric 

extern PCmetric CPUtime;
extern PCmetric SyncTime;
extern PCmetric SyncOps;
extern PCmetric lockHoldingTime;
extern PCmetric IOwait;
extern PCmetric IOops;
extern PCmetric IOBytes;
extern PCmetric seekWait;
extern PCmetric elapsedTime;
extern PCmetric activeProcesses;

// EDCU based PCmetrics.
extern PCmetric systemTime;
extern PCmetric runningProcesses;
extern PCmetric runableProcesses;
extern PCmetric systemCallRate;
extern PCmetric contextSwitchRate;
extern PCmetric interuptRate;
extern PCmetric trapRate;
extern PCmetric procedureCalls;
extern PCmetric packetsIn;
extern PCmetric packetsOut;
extern PCmetric pageFaults;
extern PCmetric pageReclaims;
extern PCmetric dirtyPageReclaims;
extern PCmetric zeroPageCreates;
extern PCmetric codePageIns;
extern PCmetric dataPageIns;
extern PCmetric pageOuts;
extern PCmetric swapIn;
extern PCmetric swapOut;
extern PCmetric vmDeficit;
extern PCmetric vmFree;
extern PCmetric vmDirty;
extern PCmetric activeVM;
extern PCmetric compensationFactor;
extern PCmetric observedCost;

// 
extern PCmetricList allMetrics;

#endif
