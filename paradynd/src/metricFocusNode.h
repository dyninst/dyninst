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
 * Revision 1.40  1996/11/14 14:28:01  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
 * Revision 1.39  1996/10/31 09:25:59  tamches
 * the shm-sampling commit; completely redesigned dataReqNode; designed
 * a handful of derived classes of dataReqNode, which replaces a lot of
 * existing code.
 *
 * Revision 1.38  1996/10/03 22:12:03  mjrg
 * Removed multiple stop/continues when inserting instrumentation
 * Fixed bug on process termination
 * Removed machine dependent code from metric.C and process.C
 *
 * Revision 1.37  1996/08/20 19:03:17  lzheng
 * Implementation of moving multiple instructions sequence and
 * Splitting the instrumentation into two phases
 *
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
 */

#ifndef METRIC_H
#define METRIC_H

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/aggregateSample.h"
#include "ast.h"

class dataReqNode {
 private:
  // Since we don't define these, make sure they're not used:
  dataReqNode &operator=(const dataReqNode &src);
  dataReqNode (const dataReqNode &src);

 protected:
  dataReqNode() {}

 public:
  virtual ~dataReqNode() {};

  virtual unsigned getInferiorPtr() const = 0;
  virtual int getSampleId() const = 0;

//  float getMetricValue();
//  virtual float cost() const = 0;

  virtual bool insertInstrumentation(process *, metricDefinitionNode *) = 0;
     // Allocates stuff from inferior heap, instrumenting DYNINSTreportCounter
     // as appropriate.  
     // Returns true iff successful.

  virtual void disable(process *,
		       const vector< vector<unsigned> > &pointsToCheck) = 0;
     // the opposite of insertInstrumentation.  Deinstruments, deallocates
     // from inferior heap, etc.

  virtual dataReqNode *dup(process *childProc, int iCounterId) const = 0;
     // duplicate 'this' (allocate w/ new) and return.  Call after a fork().
};

//#ifndef SHM_SAMPLING
class sampledIntCounterReqNode : public dataReqNode {
 private:
   // The following fields are always properly initialized in ctor:
   int sampleId;
   int initialValue; // needed when dup()'ing

   // The following fields are NULL until insertInstrumentation() called:   
   intCounter *counterPtr;		/* NOT in our address space !!!! */
   instInstance *sampler;		/* function to sample value */

   // Since we don't use these, disallow:
   sampledIntCounterReqNode &operator=(const sampledIntCounterReqNode &);
   sampledIntCounterReqNode(const sampledIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
                    process *childProc, int iCounterId);

   void writeToInferiorHeap(process *theProc, const intCounter &src) const;

 public:
   sampledIntCounterReqNode(int iValue, int iCounterId);
  ~sampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.  A bit of a complication
      // since disable() needs an arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
      // allocates from inferior heap; initializes it; instruments
      // DYNINSTsampleValues

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(counterPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)counterPtr;
   }

   int getSampleId() const {return sampleId;}
};
//#endif

#ifdef SHM_SAMPLING
class sampledShmIntCounterReqNode : public dataReqNode {
 private:
   // The following fields are always properly initialized in ctor:
   int sampleId;
   int initialValue; // needed when dup()'ing

   // The following fields are NULL until insertInstrumentation() called:
   unsigned allocatedIndex; // probably redundant w/ next field; can be removed
   intCounter *inferiorCounterPtr;	/* NOT in our address space !!!! */

   // Since we don't use these, disallow:
   sampledShmIntCounterReqNode &operator=(const sampledShmIntCounterReqNode &);
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
                               process *childProc, int iCounterId);

 public:
   sampledShmIntCounterReqNode(int iValue, int iCounterId);
  ~sampledShmIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
      // allocates from inferior heap; initializes it, etc.

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(inferiorCounterPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)inferiorCounterPtr;
   }

   int getSampleId() const {return sampleId;}
};
#endif

class nonSampledIntCounterReqNode : public dataReqNode {
 private:
   // The following fields are always properly initialized in ctor:
   int sampleId;
   int initialValue; // needed when dup()'ing

   // The following is NULL until insertInstrumentation() called:   
   intCounter *counterPtr;		/* NOT in our address space !!!! */

   // Since we don't use these, disallow:
   nonSampledIntCounterReqNode &operator=(const nonSampledIntCounterReqNode &);
   nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &);

   // private fork-ctor called by dup():
   nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &src,
                               process *childProc, int iCounterId);

   void writeToInferiorHeap(process *theProc, const intCounter &src) const;

 public:
   nonSampledIntCounterReqNode(int iValue, int iCounterId);
  ~nonSampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
      // allocates from inferior heap; initializes it

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(counterPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)counterPtr;
   }

   int getSampleId() const {return sampleId;}
};

//#ifndef SHM_SAMPLING
class sampledTimerReqNode : public dataReqNode {
 private:
   // The following fields are always initialized in the ctor:   
   int sampleId;
   timerType theTimerType;

   // The following fields are NULL until insertInstrumentation():
   tTimer *timerPtr;			/* NOT in our address space !!!! */
   instInstance *sampler;		/* function to sample value */

   // since we don't use these, disallow:
   sampledTimerReqNode(const sampledTimerReqNode &);
   sampledTimerReqNode &operator=(const sampledTimerReqNode &);

   // fork ctor:
   sampledTimerReqNode(const sampledTimerReqNode &src, process *childProc,
		       int iCounterId);

   void writeToInferiorHeap(process *theProc, const tTimer &dataSrc) const;

 public:
   sampledTimerReqNode(timerType iType, int iCounterId);
  ~sampledTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(timerPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)timerPtr;
   }

   int getSampleId() const {return sampleId;}
};
//#endif

#ifdef SHM_SAMPLING
class sampledShmWallTimerReqNode : public dataReqNode {
 private:
   // The following fields are always initialized in the ctor:   
   int sampleId;

   // The following fields are NULL until insertInstrumentatoin():
   unsigned allocatedIndex;  // probably redundant w/ next field; can be removed
   tTimer *inferiorTimerPtr; // NOT in our address space !!!!

   // since we don't use these, disallow:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &);
   sampledShmWallTimerReqNode &operator=(const sampledShmWallTimerReqNode &);

   // fork ctor:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src, process *childProc,
			      int iCounterId);

 public:
   sampledShmWallTimerReqNode(int iCounterId);
  ~sampledShmWallTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(inferiorTimerPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)inferiorTimerPtr;
   }

   int getSampleId() const {return sampleId;}
};

class sampledShmProcTimerReqNode : public dataReqNode {
 private:
   // The following fields are always initialized in the ctor:   
   int sampleId;

   // The following fields are NULL until insertInstrumentatoin():
   unsigned allocatedIndex;  // probably redundant w/ next field; can be removed
   tTimer *inferiorTimerPtr; // NOT in our address space !!!!

   // since we don't use these, disallow:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &);
   sampledShmProcTimerReqNode &operator=(const sampledShmProcTimerReqNode &);

   // fork ctor:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src, process *childProc,
			      int iCounterId);

 public:
   sampledShmProcTimerReqNode(int iCounterId);
  ~sampledShmProcTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId) const;

   bool insertInstrumentation(process *, metricDefinitionNode *);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr() const {
      assert(inferiorTimerPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)inferiorTimerPtr;
   }

   int getSampleId() const {return sampleId;}
};
#endif

/* ************************************************************************ */

class instReqNode {
public:
  instReqNode(instPoint*, AstNode *, callWhen, callOrder order,
              bool iManuallyTrigger);
 ~instReqNode();

  instReqNode() {
     // needed by Vector class
     ast = NULL; point=NULL; instance = NULL; manuallyTrigger = false;
  }

  instReqNode(const instReqNode &src) {
     point = src.point;
     when = src.when;
     order = src.order;
     instance = src.instance;
     ast = assignAst(src.ast);
     manuallyTrigger = src.manuallyTrigger;
  }
  instReqNode &operator=(const instReqNode &src) {
     if (this == &src)
        return *this;

     point = src.point;
     ast = assignAst(src.ast);
     when = src.when;
     order = src.order;
     instance = src.instance;
     manuallyTrigger = src.manuallyTrigger;

     return *this;
  }

  bool insertInstrumentation(process *theProc,
			     returnInstance *&retInstance);

  void disable(const vector<unsigned> &pointsToCheck);
  float cost(process *theProc) const;

  static instReqNode forkProcess(const instReqNode &parent, process *child);

  instInstance *getInstance() const { return instance; }

  bool anythingToManuallyTrigger() const {return manuallyTrigger;}
  bool triggerNow(process *theProc);

private:
  instPoint	*point;
  AstNode	*ast;
  callWhen	when;
  callOrder	order;
  instInstance	*instance;
  bool manuallyTrigger;
     // if true, then 'ast' is manually executed (inferiorRPC) when
     // inserting instrumentation.
};


/*
   metricDefinitionNode describe metric instances. There are two types of
   nodes: aggregates and non-aggregates (Maybe this class should be divided in
   two).
   Aggregate nodes represent a metric instance with one or more components.
   Each component is represented by a non-aggregate metricDefinitionNode, and
   is associated with a different process.
   All metric instance have an aggregate metricDefinitionNode, even if it has 
   only one component (this simplifies doing metric propagation when new 
   processes start).
   Components can be shared by two or more aggregate metricDefinitionNodes, 
   so for example if there are two metric/focus pairs enabled, cpu time for
   the whole program and cpu time for process p, there will be one non-aggregate
   instance shared between two aggregate metricDefinitionNodes.
*/
class metricDefinitionNode {
friend int startCollecting(string&, vector<u_int>&, int id, 
			   vector<process *> &procsToContinue); // called by dynrpc.C

 private:
   /* unique id for a counter or timer */
   static int counterId=0;

public:
  // styles are enumerated in util/h/aggregation.h
  metricDefinitionNode(process *p, string& metric_name, 
                       vector< vector<string> >& foc,
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

  bool nonNull() const { return (instRequests.size() || dataRequests.size()); }
  bool insertInstrumentation();

  float cost() const;
  bool checkAndInstallInstrumentation();

  float originalCost() const { return originalCost_; }

  // The following routines are (from the outside world's viewpoint)
  // the heart of it all.  They append to dataRequets or instRequests, so that
  // a future call to metricDefinitionNode::insertInstrumentation() will
  // "do their thing".
  dataReqNode *addSampledIntCounter(int initialValue);
  dataReqNode *addUnSampledIntCounter(int initialValue);
  dataReqNode *addWallTimer();
  dataReqNode *addProcessTimer();
  inline void addInst(instPoint *point, AstNode *, callWhen when, 
                      callOrder o,
		      bool manuallyTrigger);

  // propagate this metric instance to process p
  void propagateMetricInstance(process *p);  

  // what is the difference between the following 2?
  metricDefinitionNode *forkProcess(process *child);
  static void handleFork(const process *parent, process *child);

  // remove an instance from an aggregate metric
  void removeThisInstance();

  bool anythingToManuallyTrigger() const;

  void manuallyTrigger();

private:
  // Since we don't define these, make sure they're not used:
  metricDefinitionNode &operator=(const metricDefinitionNode &src);
  metricDefinitionNode(const metricDefinitionNode &src);

  void removeComponent(metricDefinitionNode *comp);
  void endOfDataCollection();
  void removeFromAggregate(metricDefinitionNode *comp);

  void updateAggregateComponent();

  bool			aggregate_;
  int aggOp; // the aggregate operator
  bool			inserted_;
  bool                  installed_;
  string met_;			// what type of metric
  vector< vector<string> > focus_;
  string flat_name_;

  /* for aggregate metrics */
  vector<metricDefinitionNode*>   components;	
  aggregateSample aggSample;    // current aggregate value

  /* for non-aggregate metrics */
  vector<dataReqNode*>	dataRequests;

  vector<instReqNode> instRequests;
  vector<returnInstance *> returnInsts;

  sampleValue cumulativeValue; // cumulative value for this metric

  // which metricDefinitionNode depend on this value.
  vector<metricDefinitionNode*>   aggregators;	
  vector<sampleInfo *> samples;  // one sample for each aggregator.
                                 // samples[i] is the sample of aggregators[i].

  int id_;				// unique id for this one 
  float originalCost_;

  // is this a final value or a component of a larger metric.
//  bool inform_;

  process *proc_;

  string metric_name_;
  metricStyle style_; 

};

inline void metricDefinitionNode::addInst(instPoint *point, AstNode *ast,
					  callWhen when,
					  callOrder o,
                                          bool manuallyTrigger) {
  if (!point) return;

  instReqNode temp(point, ast, when, o, manuallyTrigger);
  instRequests += temp;
};

extern dictionary_hash<unsigned, metricDefinitionNode*> allMIs;

// allMIComponents: the metric components indexed by flat_name.
extern dictionary_hash<string, metricDefinitionNode*> allMIComponents;

extern double currentPredictedCost;

#ifndef SHM_SAMPLING
extern void processCost(process *proc, traceHeader *h, costUpdate *s);
#endif

extern void reportInternalMetrics(bool force);

/*
 * Routines to control data collection.
 *
 * focus		- a list of resources
 * metricName		- what metric to collect data for
 * id                   - metric id
 * procsToContinue      - a list of processes that had to be stopped to insert
 *                        instrumentation. The caller must continue these processes.
 */
int startCollecting(string& metricName, vector<u_int>& focus, int id,
		    vector<process *> &procsToContinue); 

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
#ifndef SHM_SAMPLING
void processSample(traceHeader*, traceSample *);
#endif

#endif
