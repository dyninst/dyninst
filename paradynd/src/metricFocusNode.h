/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * metric.h 
 *
 * $Log: metricFocusNode.h,v $
 * Revision 1.9  1994/07/05 03:26:11  hollings
 * observed cost model
 *
 * Revision 1.8  1994/07/02  01:46:42  markc
 * Use aggregation operator defines from util/h/aggregation.h
 * Changed average aggregations to summations.
 *
 * Revision 1.7  1994/06/29  02:52:38  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.6  1994/06/27  18:56:58  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.5  1994/06/02  23:27:58  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.4  1994/04/11  23:25:23  hollings
 * Added pause_time metric.
 *
 * Revision 1.3  1994/03/26  19:31:37  hollings
 * Changed sample time to be consistant.
 *
 * Revision 1.2  1994/03/24  16:42:01  hollings
 * Moved sample aggregation to lib/util (so paradyn could use it).
 *
 * Revision 1.1  1994/01/27  20:31:29  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.9  1994/01/20  17:48:13  hollings
 * dataReqNode overcome by events.
 *
 * Revision 1.8  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.7  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.6  1993/08/20  22:01:51  hollings
 * added getMetricValue and returnCounterInstance.
 *
 * Revision 1.5  1993/08/11  01:52:55  hollings
 * new build before use metrics.
 *
 * Revision 1.4  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.3  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.2  1993/04/27  14:39:21  hollings
 * signal forwarding and args tramp.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

#ifndef METRIC_H
#define METRIC_H

#include "comm.h"
#include "util/h/aggregateSample.h"

/*
 * internal representation of an inst. request.
 *
 */
typedef enum { intCounter, timer } dataObjectType;

class AstNode;
class metricDefinitionNode;

class dataReqNode {
    public:
	dataReqNode(dataObjectType, process*, int, Boolean iReport, timerType);
	~dataReqNode();
	float getMetricValue();
	float cost();
	void insertInstrumentation(metricDefinitionNode *mi);
	void disable();

	// return a pointer in the inferrior address space of this data object.
	caddr_t getInferriorPtr();
	intCounterHandle *returnCounterInstance() { 
	    return((intCounterHandle *) instance); 
	}
	int getSampleId()	{ return(id.id); }
	dataObjectType getType() { return(type); }

    private:
	timerHandle *createTimerInstance();
	intCounterHandle *createCounterInstance();

	dataObjectType	type;
	process		*proc;
	int		initialValue;
	Boolean		report;
	timerType	tType;
	sampleId	id;	// unique id for this sample

	void		*instance;
};

class instReqNode {
    public:
	instReqNode(process*, instPoint*, AstNode*, callWhen, callOrder order);
	~instReqNode();

	Boolean insertInstrumentation();
	void disable();
	float cost();

    private:
	process		*proc;
	instPoint	*point;
	AstNode		*ast;
	callWhen	when;
	callOrder	order;
	instInstance	*instance;
};

class metricDefinitionNode {
    public:
        // styles are enumerated in util/h/aggregation.h
	metricDefinitionNode(process *p, int agg_style = aggSum);
	metricDefinitionNode(metric m, List<metricDefinitionNode*> parts); 
	~metricDefinitionNode();
	void disable();
	void updateValue(time64, sampleValue);
	void forwardSimpleValue(timeStamp, timeStamp, sampleValue);

	Boolean match(resourceList l, metric m) {
	    return(resList == l && met == m);
	}
	Boolean insertInstrumentation();
	float cost();
	// used by controller.
	float getMetricValue();
	dataReqNode *addIntCounter(int inititalValue, Boolean report) {
	    dataReqNode *tp;

	    tp = new dataReqNode(intCounter, proc, inititalValue,
		report,processTime);
	    data.add(tp);
	    return(tp);
	};
	dataReqNode *addTimer(timerType type) {
	    dataReqNode *tp;
	    tp = new dataReqNode(timer,proc,0,True,type);
	    data.add(tp);
	    return(tp);
	};
	void addInst(instPoint *point,AstNode *ast, callWhen when, callOrder o){
 	    instReqNode *temp;
            temp = new instReqNode(proc, point, ast, when, o);
	    requests.add(temp);
        };

	int id;				// unique id for this one 
	metric met;			// what type of metric
	resourceList resList;		// what resource list is this for.
	float originalCost;

	// is this a final value or a component of a larger metric.
	Boolean inform;

	process			*proc;
    private:
	void updateAggregateComponent(metricDefinitionNode *,
				      timeStamp time, 
				      sampleValue value);

	Boolean			aggregate;
	Boolean			inserted;

	/* for aggregate metrics */
	List<metricDefinitionNode*>   components;	

	/* for non-aggregate metrics */
	List<dataReqNode*>	data;
	List<instReqNode*> 	requests;

	// which metricDefinitionNode depend on this value.
	List<metricDefinitionNode*>   aggregators;	

	List<sampleInfo*>	valueList;	// actual data for comp.
	sampleInfo sample;
};

typedef AstNode *(*createPredicateFunc)(metricDefinitionNode *mn, 
    char *resource, AstNode *trigger);

typedef void (*createMetricFunc)(metricDefinitionNode*, AstNode *trigger);

typedef struct {
    createPredicateFunc	creator;
} predicateDefinition;

typedef enum { invalidPredicate, 
	       nullPredicate, 
	       simplePredicate, 
	       replaceBase } predicateType;

typedef struct {
    char *namePrefix;			/* leading part of resource path */
    predicateType	type;		
    createPredicateFunc creator;	/* create a metric */
} resourcePredicate;

typedef struct {
    createMetricFunc baseFunc;		/* base definition */
    resourcePredicate *predicates;	/* how to handle where refinements */
} metricDefinition;

struct _metricRec {
    metricInfo info;
    metricDefinition definition;
};

extern List<metricDefinitionNode*> allMIs;

//
// Return the current wall time -- 
//
//    If firstRecordRelative is TRUE, time starts at the arrival of record 0.
//    otherwise it starts at 1/1/1970 0:00 GMT.
//
timeStamp getCurrentTime(Boolean firstRecordRelative);

#endif
