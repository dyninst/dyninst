/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * metric.h 
 *
 * $Log: metricFocusNode.h,v $
 * Revision 1.36  1996/08/16 21:19:24  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.35  1996/08/12 16:27:05  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.34  1996/07/25 23:24:07  mjrg
 * Added sharing of metric components
 *
 * Revision 1.33  1996/05/10 22:36:38  naim
 * Bug fix and some improvements passing a reference instead of copying a
 * structure - naim
 *
 * Revision 1.32  1996/05/08  23:54:57  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.31  1996/04/29 03:40:03  tamches
 * added getFocus() member func
 *
 * Revision 1.30  1996/04/03 14:27:48  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.29  1996/03/25  20:22:58  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 */

#ifndef METRIC_H
#define METRIC_H

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/aggregateSample.h"
#include "process.h"
#include "dyninstP.h"
#include <strstream.h>
#include "inst.h"
#include "ast.h"
#include "paradynd/src/process.h"
#include "dyninstRPC.xdr.h"

/*
 * internal representation of an inst. request.
 *
 */
typedef enum { INTCOUNTER, TIMER } dataObjectType;

class metricDefinitionNode;
class metric;

class dataReqNode {
public:
  dataReqNode(dataObjectType, process*, int, bool iReport, timerType);
  ~dataReqNode();
  float getMetricValue();
  float cost();
  void insertGlobal();    // allow a global "variable" to be inserted
  bool insertInstrumentation(metricDefinitionNode *mi);
  void disable(const vector<unsigVecType> &pointsToCheck);

  // return a pointer in the inferior address space of this data object.
  unsigned getInferiorPtr();
  intCounterHandle *returnCounterInstance() { return((intCounterHandle *) instance); }
  int getSampleId()	{ return(id.id); }
  dataObjectType getType() { return(type); }

  static dataReqNode *forkProcess(metricDefinitionNode *childMi, dataReqNode *parent);

private:
  timerHandle *createTimerInstance();
  intCounterHandle *createCounterInstance();

  dataObjectType	type;
  process		*proc;
  int		initialValue;
  bool		report;
  timerType	tType;
  sampleId	id;	// unique id for this sample

  void		*instance;
};

class instReqNode {
public:
  instReqNode(process*, instPoint*, const AstNode &, callWhen, callOrder order);
 ~instReqNode();

  instReqNode() {
     // needed by Vector class
     proc = NULL; point=NULL; instance = NULL;
  }

  instReqNode(const instReqNode &src) : ast(src.ast) {
     proc = src.proc;
     point = src.point;
     when = src.when;
     order = src.order;
     instance = src.instance;
  }
  instReqNode &operator=(const instReqNode &src) {
     if (this == &src)
        return *this;

     proc = src.proc;
     point = src.point;
     ast = src.ast;
     when = src.when;
     order = src.order;
     instance = src.instance;

     return *this;
  }

  bool insertInstrumentation();
  void disable(const vector<unsigned> &pointsToCheck);
  float cost();

  static instReqNode instReqNode::forkProcess(const instReqNode &parent, process *child);

  instInstance *getInstance() { return instance; }

private:
  process	*proc;
  instPoint	*point;
  AstNode	ast;
  callWhen	when;
  callOrder	order;
  instInstance	*instance;
};


/*
   metricDefinitionNode describe metric instances. There are two types of
   nodes: aggregates and non-aggregates (Maybe this class should be divided in
   two).
   Aggregate nodes represent a metric instance with one or more components.
   Each component is represented by a non-aggregate metricDefinitionNode, and
   is associated with a process.
   All metric instance have an aggregate metricDefinitionNode, even if it has 
   only one component (this simplifies doing metric propagation when new 
   processes start).
   Components can be shared by two or more aggregate metricDefinitionNodes, 
   so for example if there are two metric/focus pairs enabled, cpu time for
   the whole program and cpu time for process p, there will be one non-aggregate
   instance shared between two aggregate metricDefinitionNodes.
*/
class metricDefinitionNode {
friend int startCollecting(string&, vector<u_int>&, int id);

public:
  // styles are enumerated in util/h/aggregation.h
  metricDefinitionNode(process *p, string& metric_name, vector< vector<string> >& foc,
		       string& cat_name, int agg_style = aggSum);
  metricDefinitionNode(string& metric_name, vector< vector<string> >& foc,
		       string& cat_name, vector<metricDefinitionNode*>& parts,
		       int agg_op);
  ~metricDefinitionNode();
  void disable();
  void updateValue(time64, sampleValue);
  void forwardSimpleValue(timeStamp, timeStamp, sampleValue,unsigned,bool);

  int getMId() const { return id_; }
  string getMetName() const { return met_; }
  string getFullName() const { return flat_name_; }
  const vector< vector<string> > &getFocus() const {return focus_;}

  process *proc() const { return proc_; }

  bool nonNull() { return (requests.size() || data.size()); }
  bool insertInstrumentation();
  float cost();
  float originalCost() const { return originalCost_; }

  // used by controller.
  float getMetricValue();

  inline dataReqNode *addIntCounter(int inititalValue, bool report);
  inline dataReqNode *addTimer(timerType type);
  inline void addInst(instPoint *point, const AstNode &, callWhen when, callOrder o);
  void set_inform(bool new_val) { inform_ = new_val; }

  // propagate this metric instance to process p
  void propagateMetricInstance(process *p);  

  metricDefinitionNode *forkProcess(process *child);
  static void handleFork(process *parent, process *child);

  // remove an instance from an aggregate metric
  void removeThisInstance();

private:

  void removeComponent(metricDefinitionNode *comp);
  void endOfDataCollection();
  void removeFromAggregate(metricDefinitionNode *comp);
  void updateAggregateComponent();

  bool			aggregate_;
  int aggOp; // the aggregate operator
  bool			inserted_;
  string met_;			// what type of metric
  vector< vector<string> > focus_;
  string flat_name_;

  /* for aggregate metrics */
  vector<metricDefinitionNode*>   components;	
  aggregateSample aggSample;    // current aggregate value

  /* for non-aggregate metrics */
  vector<dataReqNode*>	data;

  vector<instReqNode> requests;
  sampleValue cumulativeValue; // cumulative value for this metric

  // which metricDefinitionNode depend on this value.
  vector<metricDefinitionNode*>   aggregators;	
  vector<sampleInfo *> samples;  // one sample for each aggregator.
                                 // samples[i] is the sample of aggregators[i].

  int id_;				// unique id for this one 
  float originalCost_;

  // is this a final value or a component of a larger metric.
  bool inform_;
  process *proc_;

  string metric_name_;
  metricStyle style_; 

};

inline dataReqNode *metricDefinitionNode::addIntCounter(int inititalValue, bool report) {
  dataReqNode *tp;
  tp = new dataReqNode(INTCOUNTER, proc_, inititalValue, report, processTime);
  assert(tp);
  data += tp;
  return(tp);
};

inline dataReqNode *metricDefinitionNode::addTimer(timerType type) {
  dataReqNode *tp;
  tp = new dataReqNode(TIMER, proc_, 0, true, type);
  assert(tp);
  data += tp;
  return(tp);
};

inline void metricDefinitionNode::addInst(instPoint *point, const AstNode &ast,
					  callWhen when,
					  callOrder o) {
  if (!point) return;

  instReqNode temp(proc_, point, ast, when, o);
  requests += temp;
};

extern dictionary_hash<unsigned, metricDefinitionNode*> allMIs;
// allMIComponents: the metric components indexed by flat_name.
extern dictionary_hash<string, metricDefinitionNode*> allMIComponents;

//
// Return the current wall time -- 
//
//    If firstRecordRelative is true, time starts at the arrival of record 0.
//    otherwise it starts at 1/1/1970 0:00 GMT.
//
timeStamp getCurrentTime(bool firstRecordRelative);

extern double currentPredictedCost;
extern void processCost(process *proc, traceHeader *h, costUpdate *s);
extern void reportInternalMetrics();

/*
 * Routines to control data collection.
 *
 * focus		- a list of resources
 * metricName		- what metric to collect data for
 */
int startCollecting(string& metricName, vector<u_int>& focus); 

/*
 * Return the expected cost of collecting performance data for a single
 *    metric at a given focus.  The value returned is the fraction of
 *    perturbation expected (i.e. 0.10 == 10% slow down expected).
 */
float guessCost(string& metric_name, vector<u_int>& focus);


/*
 * process a sample ariving from an inferior process
 *
 */
void processSample(traceHeader*, traceSample *);


#endif
