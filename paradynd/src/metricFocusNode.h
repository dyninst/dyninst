/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * metric.h 
 *
 * $Log: metricFocusNode.h,v $
 * Revision 1.25  1996/02/08 23:03:41  newhall
 * fixed Ave. aggregation for CM5 daemons, Max and Min don't work, but are
 * approximated by ave rather than sum
 *
 * Revision 1.24  1996/01/31  19:49:59  newhall
 * changes to do average aggregation correctly
 *
 * Revision 1.23  1996/01/29  22:09:32  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.22  1995/12/15 14:40:57  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.21  1995/11/29  18:45:23  krisna
 * added inlines for compiler. added templates
 *
 * Revision 1.20  1995/08/24 15:04:20  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.19  1995/05/18  10:38:57  markc
 * Removed class metric
 *
 * Revision 1.18  1995/02/16  08:53:44  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.17  1995/02/16  08:33:51  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.16  1995/01/30  17:32:13  jcargill
 * changes for gcc-2.6.3; intCounter was both a typedef and an enum constant
 *
 * Revision 1.15  1994/11/10  18:58:08  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.14  1994/11/09  18:40:17  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.13  1994/11/02  11:11:24  markc
 * Added classes and removed compiler warnings.
 *
 * Revision 1.12  1994/09/22  02:14:13  markc
 * Changed structs to classes
 *
 * Revision 1.11  1994/09/05  20:33:43  jcargill
 * Bug fix:  enabling certain metrics could cause no instrumentation to be
 * inserted, but still return a mid; this hosed the PC
 *
 * Revision 1.10  1994/07/21  01:34:20  hollings
 * Fixed to skip over null point and ast nodes for addInst calls.
 *
 * Revision 1.9  1994/07/05  03:26:11  hollings
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

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/aggregateSample.h"
#include "process.h"
#include "dyninstP.h"
#include <strstream.h>
#include "inst.h"
#include "paradynd/src/process.h"
#include "dyninstRPC.xdr.h"

/*
 * internal representation of an inst. request.
 *
 */
typedef enum { INTCOUNTER, TIMER } dataObjectType;

class AstNode;
class metricDefinitionNode;
class metric;

class dataReqNode {
public:
  dataReqNode(dataObjectType, process*, int, bool iReport, timerType);
  ~dataReqNode();
  float getMetricValue();
  float cost();
  void insertGlobal();    // allow a global "variable" to be inserted
  void insertInstrumentation(metricDefinitionNode *mi);
  void disable();

  // return a pointer in the inferior address space of this data object.
  unsigned getInferiorPtr();
  intCounterHandle *returnCounterInstance() { return((intCounterHandle *) instance); }
  int getSampleId()	{ return(id.id); }
  dataObjectType getType() { return(type); }

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
  instReqNode(process*, instPoint*, AstNode*, callWhen, callOrder order);
  ~instReqNode();

  bool insertInstrumentation();
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
friend int startCollecting(string&, vector<u_int>&, int id);

public:
  // styles are enumerated in util/h/aggregation.h
  metricDefinitionNode(process *p, string& metric_name, vector< vector<string> >& foc,
		       string& cat_name, int agg_style = aggSum);
  metricDefinitionNode(string& metric_name, vector< vector<string> >& foc,
		       string& cat_name, vector<metricDefinitionNode*>& parts); 
  ~metricDefinitionNode();
  void disable();
  void updateValue(time64, sampleValue);
  void updateCM5AggValue(time64, sampleValue,int,bool);
  void forwardSimpleValue(timeStamp, timeStamp, sampleValue,unsigned,bool);

  string getMetName() const { return met_; }
  string getFullName() const { return flat_name_; }
  process *proc() const { return proc_; }

  bool nonNull() { return (requests.size() || data.size()); }
  bool insertInstrumentation();
  float cost();
  float originalCost() const { return originalCost_; }

  // used by controller.
  float getMetricValue();

  inline dataReqNode *addIntCounter(int inititalValue, bool report);
  inline dataReqNode *addTimer(timerType type);
  inline void addInst(instPoint *point,AstNode *ast, callWhen when, callOrder o);
  void set_inform(bool new_val) { inform_ = new_val; }

  // propagate this metric instance to process p
  void propagateMetricInstance(process *p);  

private:

  void updateAggregateComponent(metricDefinitionNode *,
				timeStamp time, 
				sampleValue value);

  bool			aggregate_;
  bool			inserted_;
  string met_;			// what type of metric
  vector< vector<string> > focus_;
  string flat_name_;

  /* for aggregate metrics */
  vector<metricDefinitionNode*>   components;	

  /* for non-aggregate metrics */
  vector<dataReqNode*>	data;
  vector<instReqNode*> 	requests;

  // which metricDefinitionNode depend on this value.
  vector<metricDefinitionNode*>   aggregators;	

  List<sampleInfo*>	valueList;	// actual data for comp.
  sampleInfo sample;
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

inline void metricDefinitionNode::addInst(instPoint *point, AstNode *ast, callWhen when,
				   callOrder o) {
  instReqNode *temp;
  if (!point || !ast) return;
  temp = new instReqNode(proc_, point, ast, when, o);
  assert(temp);
  requests += temp;
};

extern dictionary_hash<unsigned, metricDefinitionNode*> allMIs;

//
// Return the current wall time -- 
//
//    If firstRecordRelative is true, time starts at the arrival of record 0.
//    otherwise it starts at 1/1/1970 0:00 GMT.
//
timeStamp getCurrentTime(bool firstRecordRelative);

extern double currentSmoothObsValue;
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

#endif
