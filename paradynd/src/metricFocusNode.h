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
v * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: metricFocusNode.h,v 1.75 2001/09/04 19:48:48 gurari Exp $ 

#ifndef METRIC_H
#define METRIC_H

#include "common/h/String.h"
// trace data streams
#include "pdutil/h/ByteArray.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "pdutil/h/sampleAggregator.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/inst.h" // for "enum callWhen"
#include "dyninstRPC.xdr.SRVR.h" // for flag_cons
#include "common/h/Time.h"
#include "pdutil/h/metricStyle.h"

#define OPT_VERSION 1

class instInstance; // enough since we only use instInstance* in this file
#if defined(MT_THREAD)
class pdThread; // enough since we only use pdThread* in this file
#endif

#if defined(SHM_SAMPLING)
#include "paradynd/src/superTable.h"
#endif

class dataReqNode {
 private:
  // Since we don't define these, make sure they're not used:
  dataReqNode &operator=(const dataReqNode &src);
  dataReqNode (const dataReqNode &src);

 protected:
  dataReqNode() {}

 public:
  virtual ~dataReqNode() {};

  virtual Address getInferiorPtr(process *proc=NULL) const = 0;
  virtual unsigned getAllocatedIndex() const = 0;
  virtual unsigned getAllocatedLevel() const = 0;

  virtual int getSampleId() const = 0;

#if defined(MT_THREAD)
  virtual int getThreadId() const = 0;
  virtual bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool) = 0;
#else 
  virtual bool insertInstrumentation(process *, metricDefinitionNode *, bool) = 0;
#endif
     // Allocates stuff from inferior heap, instrumenting DYNINSTreportCounter
     // as appropriate.  
     // Returns true iff successful.

  virtual void disable(process *,
		       const vector< vector<Address> > &pointsToCheck) = 0;
     // the opposite of insertInstrumentation.  Deinstruments, deallocates
     // from inferior heap, etc.

  virtual dataReqNode *dup(process *childProc, metricDefinitionNode *,
			   int iCounterId,
			   const dictionary_hash<instInstance*,instInstance*> &map
			   ) const = 0;
     // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

     // "map" provides a dictionary that maps instInstance's of the parent process to
     // those in the child process; dup() may find it useful. (for now, the shm
     // dataReqNodes have no need for it; the alarm sampled ones need it).

  virtual bool unFork(dictionary_hash<instInstance*,instInstance*> &map) = 0;
     // the fork syscall duplicates all instrumentation (except on AIX), which is a
     // problem when we determine that a certain mi shouldn't be propagated to the child
     // process.  Hence this routine.
};

/*
 * Classes derived from dataReqNode
 *
 * There are a lot of classes derived from dataReqNode, so things can get a little
 * confusing.  Most of them are used in either shm_sampling mode or not; few are
 * used in both.  Here's a bit of documentation that should sort things out:
 *
 * A) classes used in _both_ shm_sampling and non-shm-sampling
 *
 *    nonSampledIntCounterReqNode --
 *       provides an intCounter value that is never sampled.  Currently useful
 *       for predicates (constraint booleans).  The intCounter is allocated from
 *       the "conventional" heap (where tramps are allocated) using good old
 *       inferiorMalloc(), even when shm_sampling.
 *       (In the future, we may provide a separate shm_sampling version, to make
 *       allocation faster [allocation in the shm seg heap is always faster])
 *
 * B) classes used only when shm_sampling
 *
 *    sampledShmIntCounterReqNode --
 *       provides an intCounter value that is sampled with shm sampling.  The
 *       intCounter is allocated from the shm segment (i.e., not with inferiorMalloc).
 *
 *    sampledShmWallTimerReqNode --
 *       provides a wall timer value that is sampled with shm sampling.  The tTimer is
 *       allocated from the shm segment (i.e., not with inferiorMalloc)
 *    
 *    sampledShmProcTimerReqNode --
 *       provides a process timer value that is sampled with shm sampling.  The tTimer
 *       is allocated from the shm segment (i.e., not with inferiorMalloc)
 *    
 * C) classes used only when _not_ shm-sampling
 *    
 *    sampledIntCounterReqNode --
 *       provides an intCounter value that is sampled.  The intCounter is allocated
 *       in the conventional heap with inferiorMalloc().  Sampling is done the
 *       old-fasioned SIGALRM way: by instrumenting DYNINSTsampleValues to call
 *       DYNINSTreportTimer
 *
 *    sampledTimerReqNode --
 *       provides a tTimer value (can be wall or process timer) that is sampled.  The
 *       tTimer is allocated in the conventional heap with inferiorMalloc().  Sampling
 *       is done the old-fasioned SIGALRM way: by instrumenting DYNINSTsampleValues to
 *       call DYNINSTreportTimer.
 *
 */

#ifndef SHM_SAMPLING
class sampledIntCounterReqNode : public dataReqNode {
 // intCounter for use when not shm sampling.  Allocated in the conventional heap
 // with inferiorMalloc().  Sampled the old-fasioned way: by instrumenting
 // DYNINSTsampleValues() to call DYNINSTreportCounter.
 private:
   // The following fields are always properly initialized in ctor:
   int theSampleId;
   rawTime64 initialValue; // needed when dup()'ing

   // The following fields are NULL until insertInstrumentation() called:   
   intCounter *counterPtr;		/* NOT in our address space !!!! */
   instInstance *sampler;		/* function to sample value */

   // Since we don't use these, disallow:
   sampledIntCounterReqNode &operator=(const sampledIntCounterReqNode &);
   sampledIntCounterReqNode(const sampledIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
			    process *childProc, metricDefinitionNode *, int iCounterId,
                            const dictionary_hash<instInstance*,instInstance*> &map);

   void writeToInferiorHeap(process *theProc, const intCounter &src) const;

 public:
   sampledIntCounterReqNode(rawTime64 iValue, int iCounterId,
                            metricDefinitionNode *iMi, bool computingCost);
  ~sampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.  A bit of a complication
      // since disable() needs an arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &map) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool doNotSample=false);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
#endif
      // allocates from inferior heap; initializes it; instruments
      // DYNINSTsampleValues

   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const {
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      //assert(counterPtr != NULL); // NULL until insertInstrumentation()
      return (Address)counterPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const {assert(0); return 0;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &map);
     // the fork syscall duplicates all instrumentation (except on AIX), which is a
     // problem when we determine that a certain mi shouldn't be propagated to the child
     // process.  Hence this routine (to remove unwanted instr of DYNINSTsampleValues())
};
#endif

#ifdef SHM_SAMPLING
class sampledShmIntCounterReqNode : public dataReqNode {
 // intCounter for use when shm-sampling.  Allocated in the shm segment heap.
 // Sampled using shm sampling.
 private:
   // The following fields are always properly initialized in ctor:
   int theSampleId; // obsolete with shm sampling; can be removed.
#if defined(MT_THREAD)
   pdThread *thr_;
#endif

   rawTime64 initialValue; // needed when dup()'ing

   // The following fields are NULL until insertInstrumentation() called:
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // Since we don't use these, making them privates ensures they're not used.
   sampledShmIntCounterReqNode &operator=(const sampledShmIntCounterReqNode &);
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
                               process *childProc, metricDefinitionNode *,
			       int iCounterId, const process *parentProc);

 public:
#if defined(MT_THREAD)
   sampledShmIntCounterReqNode(pdThread *thr, rawTime64 iValue, int iCounterId,
                               metricDefinitionNode *iMi, bool computingCost,
                               bool doNotSample, unsigned, unsigned);
#else
   sampledShmIntCounterReqNode(rawTime64 iValue, int iCounterId,
                                metricDefinitionNode *iMi, bool computingCost,
                                bool doNotSample);
#endif
  ~sampledShmIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool);
#endif
      // allocates from inferior heap; initializes it, etc.

   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}
   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const;
   pdThread *getThread() {return thr_;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};
#endif

class nonSampledIntCounterReqNode : public dataReqNode {
 // intCounter for predicates (because they don't need to be sampled).
 // Allocated in the conventional heap with inferiorMalloc().
 private:
   // The following fields are always properly initialized in ctor:
   int theSampleId;
   rawTime64 initialValue; // needed when dup()'ing

   // The following is NULL until insertInstrumentation() called:   
   intCounter *counterPtr;		/* NOT in our address space !!!! */

   // Since we don't use these, disallow:
   nonSampledIntCounterReqNode &operator=(const nonSampledIntCounterReqNode &);
   nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &);

   // private fork-ctor called by dup():
   nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &src,
                               process *childProc, metricDefinitionNode *,
			       int iCounterId);

   void writeToInferiorHeap(process *theProc, const intCounter &src) const;

 public:
   nonSampledIntCounterReqNode(rawTime64 iValue, int iCounterId,
                               metricDefinitionNode *iMi, bool computingCost);
  ~nonSampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool doNotSample=false);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
#endif
      // allocates from inferior heap; initializes it

   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const {
      //assert(counterPtr != NULL); // NULL until insertInstrumentation()
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      return (Address)counterPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const {assert(0); return 0;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};

#ifndef SHM_SAMPLING
class sampledTimerReqNode : public dataReqNode {
 // tTimer for use when not shm sampling.  Allocated in the conventional heap with
 // inferiorMalloc().  Sampled the old-fasioned way: by instrumenting
 // DYNINSTsampleValues to call DYNINSTreportTimer.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;
   timerType theTimerType;

   // The following fields are NULL until insertInstrumentation():
   tTimer *timerPtr;			/* NOT in our address space !!!! */
   instInstance *sampler;		/* function to sample value */

   // since we don't use these, disallow:
   sampledTimerReqNode(const sampledTimerReqNode &);
   sampledTimerReqNode &operator=(const sampledTimerReqNode &);

   // fork ctor:
   sampledTimerReqNode(const sampledTimerReqNode &src, process *childProc,
		       metricDefinitionNode *, int iCounterId,
		       const dictionary_hash<instInstance*,instInstance*> &map);

   void writeToInferiorHeap(process *theProc, const tTimer &dataSrc) const;

 public:
   sampledTimerReqNode(timerType iType, int iCounterId,
                       metricDefinitionNode *iMi, bool computingCost);
  ~sampledTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &map) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool doNotSample=false);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
#endif
   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const {
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      //assert(timerPtr != NULL); // NULL until insertInstrumentation()
      return (Address)timerPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const {assert(0); return 0;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &map);
     // the fork syscall duplicates all instrumentation (except on AIX), which is a
     // problem when we determine that a certain mi shouldn't be propagated to the child
     // process.  Hence this routine (to remove unwanted instr of DYNINSTsampleValues())
};
#endif

#ifdef SHM_SAMPLING
class sampledShmWallTimerReqNode : public dataReqNode {
 // wall tTimer for use when shm sampling.  Allocated in the shm segment heap.
 // Sampled using shm-sampling.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;
#if defined(MT_THREAD)
   pdThread *thr_;
#endif

   // The following fields are NULL until insertInstrumentatoin():
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // since we don't use these, disallow:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &);
   sampledShmWallTimerReqNode &operator=(const sampledShmWallTimerReqNode &);

   // fork ctor:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src, 
			      process *childProc,
			      metricDefinitionNode *, int iCounterId,
			      const process *parentProc);

 public:
#if defined(MT_THREAD)
   sampledShmWallTimerReqNode(pdThread *thr, int iCounterId,
                              metricDefinitionNode *iMi, bool computingCost,
			      unsigned, unsigned);
#else
   sampledShmWallTimerReqNode(int iCounterId,
                              metricDefinitionNode *iMi, bool computingCost);
#endif
  ~sampledShmWallTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool doNotSample=false);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
#endif
   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const;
   pdThread *getThread() {return thr_;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};

class sampledShmProcTimerReqNode : public dataReqNode {
 // process tTimer for use when shm sampling.  Allocated in the shm segment heap.
 // Sampled using shm-sampling.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;
#if defined(MT_THREAD)
   pdThread *thr_;
#endif

   // The following fields are NULL until insertInstrumentatoin():
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // since we don't use these, disallow:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &);
   sampledShmProcTimerReqNode &operator=(const sampledShmProcTimerReqNode &);

   // fork ctor:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src, 
			      process *childProc,
			      metricDefinitionNode *, int iCounterId,
			      const process *parentProc);

 public:
#if defined(MT_THREAD)
   sampledShmProcTimerReqNode(pdThread *thr, int iCounterId,
			      metricDefinitionNode *iMi, bool computingCost,
			      unsigned, unsigned);
#else
   sampledShmProcTimerReqNode(int iCounterId,
                             metricDefinitionNode *iMi, bool computingCost);
#endif
  ~sampledShmProcTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

#if defined(MT_THREAD)
   bool insertInstrumentation(pdThread *, process *, metricDefinitionNode *, bool doNotSample=false);
#else
   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
#endif
   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}
#if defined(MT_THREAD)
   int getThreadId() const;
   pdThread *getThread() {return thr_;}
#endif

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};
#endif

/* ************************************************************************ */

class instReqNode {
friend bool toDeletePrimitiveMDN(metricDefinitionNode *prim);

public:
  instReqNode(instPoint*, AstNode *, callWhen, callOrder order);
 ~instReqNode();

  instReqNode() {
     // needed by Vector class
    ast = NULL; 
    point=NULL; 
    instance = NULL; 
    rinstance = NULL; 
  }

  instReqNode(const instReqNode &src) {
     point = src.point;
     when = src.when;
     order = src.order;
     instance = src.instance;
     rinstance = src.rinstance;
     ast = assignAst(src.ast);
  }
  instReqNode &operator=(const instReqNode &src) {
     if (this == &src)
        return *this;

     point = src.point;
     ast = assignAst(src.ast);
     when = src.when;
     order = src.order;
     instance = src.instance;
     rinstance = src.rinstance;

     return *this;
  }

  bool insertInstrumentation(process *theProc, 
                             returnInstance *&retInstance,
                             bool &deferred);

  void disable(const vector<Address> &pointsToCheck);
  timeLength cost(process *theProc) const;

  static instReqNode forkProcess(const instReqNode &parent,
                                 const dictionary_hash<instInstance*,instInstance*> &);
     // should become a copy-ctor...or at least, a non-static member fn.

  bool unFork(dictionary_hash<instInstance*, instInstance*> &map) const;
     // The fork syscall duplicates all trampolines from the parent into the child. For
     // those mi's which we don't want to propagate to the child, this creates a
     // problem.  We need to remove instrumentation code from the child.  This routine
     // does that.  "map" maps instInstances of the parent to those in the child.

  instInstance *getInstance() const { return instance; }
  returnInstance *getRInstance() const { return rinstance; }

#if defined(MT_THREAD)
  bool triggerNow(process *theProc, int mid, int thrId);
#else
  bool triggerNow(process *theProc, int mid);
#endif
  static void triggerNowCallbackDispatch(process * /*theProc*/,
						    void *userData, void *returnValue)
	  { ((instReqNode*)userData)->triggerNowCallback( returnValue ); }
  void triggerNowCallback(void *returnValue);

  bool triggeredInStackFrame(pd_Function *stack_fn,
                             Address pc,
                             process *p);
  
  instPoint *Point() {return point;}
  AstNode* Ast()  {return ast;}
  callWhen When() {return when;}

private:
  instPoint	*point;
  AstNode	*ast;
  callWhen	when;
  callOrder	order;
  instInstance	*instance; // undefined until insertInstrumentation() calls addInstFunc
  returnInstance *rinstance;

  // Counts the number of rpcs which have successfully completed 
  // for this node.  This is needed because we may need to manually 
  // trigger multiple times for recursive functions.
  int rpcCount;
#if defined(MT_THREAD)
  vector<int> manuallyTriggerTIDs;
#endif
};

// A function which is called by the continue mechanism in the process
// object.  Used to determine the initialStartTime of a mdn.
void mdnContinueCallback(timeStamp timeOfCont);

// metricDefinitionNode type, AGG_MDN previously known as aggregate, 
// COMP_MDN previously known as component (non-aggregate), PRIM_MDN
// is new: COMP_MDN is decomposed into several constraint PRIM_MDN
// and one metric PRIM_MDN. One PRIM_MDN is the collection of modifications
// to ONE variable (constraint varaible or metric variable) except
// that in metric, several temp variables could also be modified.
//
// This is the best (smallest) unit to do reuse optimization 
// (instrumentation) because PRIM_MDN is about a variable and all
// modifications to that variable. We can reuse that variable
// instead of having a new one if the same modification snippets
// would be inserted at the same instrument points.
//
// BUT this is not true for "replace" constraints, in which case,
// metric could only be at COMP_MDN level instead of decomposing
// into PRIM_MDN level.

// ning: Should separate constraint prim from metric prim?
//   because even if we can reuse, constraint var is not sampled
//   while metric prim is sampled.  or have two separate
//   allMIPrimitives for them.
// THR_LEV is for threaded paradyn only
typedef enum {AGG_MDN, COMP_MDN, PRIM_MDN, THR_LEV} MDN_TYPE;


// metricDefinitionNode type for MT_THREAD version:
// AGG_LEV   is similar to AGG_MDN.
//
// PROC_COMP is similar to COMP_MDN, except that here, if metric
//   prim is the same, the same proc_comp will be shared;
//   multiple names should be memorized.
//
// PROC_PRIM is similar to PRIM_MDN, except that here, two
//   separate allMIPrimitives will be used for constraint and
//   metric respectively;  metric prim will have only one
//   aggregator.
//
// THR_LEV is new, it stores variables for each thread;
//   metric thr is registered in allMIComponents also;
//   thr_lev has only one aggregator -- a metric prim;
//   if it has component, it has only one -- the proc_
//   comp also for its metric prim.
#ifdef notdefined
#if defined(MT_THREAD)
typedef enum {AGG_LEV, PROC_COMP, PROC_PRIM, THR_LEV} AGG_LEVEL;
#endif
#endif

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
friend timeLength guessCost(string& metric_name, vector<u_int>& focus) ;

friend int startCollecting(string&, vector<u_int>&, int id, 
			   vector<process *> &procsToContinue); // called by dynrpc.C
friend bool toDeletePrimitiveMDN(metricDefinitionNode *prim); // used in mdl.C
#if defined(MT_THREAD)
friend bool checkMetricMIPrimitives(string metric_flat_name, metricDefinitionNode *& metric_prim,
			     string name, vector< vector<string> >& comp_focus, int processIdx);
#endif

private:
  /* unique id for a counter or timer */
  static int counterId;

public:

  // styles are enumerated in aggregation.h
  metricDefinitionNode(process *p, const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op, MDN_TYPE mdntype);

  // NON_MT_THREAD version:
  // for primitive (real non-aggregate, per constraint var or metric var) mdn's
  // flat name should include process id
  //
  // for component (per-process) (non-aggregate, now aggregate) mdn's
  // difference: it now has parts too (become aggregate)

  metricDefinitionNode(const string& metric_name, 
		       const vector< vector<string> >& foc,
		       const string& cat_name,
		       vector<metricDefinitionNode*>& parts,
		       aggregateOp agg_op, MDN_TYPE mdntype = AGG_MDN);

  // NON_MT_THREAD version:
  // for aggregate (not component) mdn's

  ~metricDefinitionNode();
  void okayToSample();
  void disable();
  void cleanup_drn();
  void updateValue(timeStamp, pdSample);
  void forwardSimpleValue(timeStamp, timeStamp, pdSample);

  int getMId() const { return id_; }

  bool isTopLevelMDN() const {
    return (mdn_type_ == AGG_MDN);
  }

  const string &getMetName() const { return met_; }
  const string &getFullName() const { return flat_name_; }
  const vector< vector<string> > &getFocus() const {return focus_;}
  const vector< vector<string> > &getComponentFocus() const { return component_focus; }

  process *proc() const { return proc_; }
  
  vector<dataReqNode *> getDataRequests();
  vector<metricDefinitionNode *>& getComponents() { 
    return components; 
  }
  void addPart(metricDefinitionNode* part);
  void addPartDummySample(metricDefinitionNode* part);  // special purpose
#if defined(MT_THREAD)
  void reUseIndexAndLevel(unsigned &p_allocatedIndex, unsigned &p_allocatedLevel);
  void addParts(vector<metricDefinitionNode*>& parts);

  //AGG_LEVEL getLevel() { return aggLevel; }
  // vector<dataReqNode *> getDataRequests() { return dataRequests; }      
  void addThread(pdThread *thr);
  void deleteThread(pdThread *thr);
  void duplicateInst(metricDefinitionNode *from, metricDefinitionNode *to);
  void duplicateInst(metricDefinitionNode *mn) ;
  
  void setMetricRelated(unsigned type, bool computingCost, vector<string> * temp_ctr, 
			vector<T_dyninstRPC::mdl_constraint*> flag_cons,
			vector<T_dyninstRPC::mdl_constraint*> base_use) {
    assert(COMP_MDN == mdn_type_);

    type_thr          = type;
    computingCost_thr = computingCost;
    temp_ctr_thr      = temp_ctr;
    flag_cons_thr     = flag_cons;
    base_use_thr      = base_use;
  }
  //AGG_LEVEL getMdnType(void) const { return aggLevel; }
#endif

  MDN_TYPE getMdnType(void) const { return mdn_type_; }


  friend ostream& operator<<(ostream&s, const metricDefinitionNode &m);

  // careful in use!
  // NON_MT_THREAD version:  only for PRIM mdn
  // MT_THREAD version:  only for PROC_PRIM or THR_LEV
  bool nonNull() const { return (instRequests.size() || dataRequests.size()); }
  int getSizeOfInstRequests() const { return instRequests.size(); }
  void setStartTime(timeStamp t, bool resetCompStartTime = false);
  bool insertInstrumentation(pd_Function *&func, bool &deferred);

  void setInitialActualValue(pdSample s);
  void sendInitialActualValue(pdSample s);
  bool sentInitialActualValue() {  return _sentInitialActualValue; }
  void sentInitialActualValue(bool v) {  _sentInitialActualValue = v; }
  static void updateAllAggInterval(timeLength width);
  void updateAggInterval(timeLength width) {
    aggregator.changeAggIntervalWidth(width);
  }

  // needed
  timeLength cost() const;
  bool checkAndInstallInstrumentation();

  timeLength originalCost() const { return originalCost_; }

  // The following routines are (from the outside world's viewpoint)
  // the heart of it all.  They append to dataRequets or instRequests, so that
  // a future call to metricDefinitionNode::insertInstrumentation() will
  // "do their thing".  The MDL calls these routines.
#if defined(MT_THREAD)
  dataReqNode *addSampledIntCounter(pdThread *thr, rawTime64 initialValue, 
                                    bool computingCost,
                                    bool doNotSample=false);
#else
  dataReqNode *addSampledIntCounter(rawTime64 initialValue, bool computingCost,
				     bool doNotSample=false);
#endif
  dataReqNode *addUnSampledIntCounter(rawTime64 initialValue, bool computingCost);
#if defined(MT_THREAD)
  dataReqNode *addWallTimer(bool computingCost, pdThread *thr=NULL);
  dataReqNode *addProcessTimer(bool computingCost, pdThread *thr=NULL);
#else
  dataReqNode *addWallTimer(bool computingCost);
  dataReqNode *addProcessTimer(bool computingCost);
#endif
  // inline void addInst(instPoint *point, AstNode *, callWhen when, 
  //                     callOrder o);
  void addInst(instPoint *point, AstNode *, callWhen when, 
	       callOrder o);

  // propagate this aggregate mi to a newly started process p (not for processes
  // started via fork or exec, just for those started "normally")
  void propagateToNewProcess(process *p);  

  metricDefinitionNode *forkProcess(process *child,
                                    const dictionary_hash<instInstance*,instInstance*> &map) const;
     // called when it's determined that an mi should be propagated from the
     // parent to the child.  "this" is a component mi, not an aggregator mi.
  bool unFork(dictionary_hash<instInstance*, instInstance*> &map,
	      bool unForkInstRequests, bool unForkDataRequests);
     // the fork() sys call copies all trampoline code, so the child process can be
     // left with code that writes to counters/timers that don't exist (in the case
     // where we don't propagate the mi to the new process).  In such cases, we must
     // remove instrumentation from the child process.  That's what this routine is
     // for.  It looks at the instReqNodes of the mi, which are in place in the parent
     // process, and removes them from the child process.  "this" is a component mi
     // representing the parent process.  "map" maps instInstance's of the parent to
     // those of the child.

  static void handleFork(const process *parent, process *child,
                         dictionary_hash<instInstance*, instInstance*> &map);
     // called once per fork.  "map" maps all instInstance's of the parent
     // process to the corresponding copy in the child process...we'll delete some
     // instrumentation in the child process if we find that some instrumentation
     // in the parent doesn't belong in the child.

  static void handleExec(process *);
     // called once per exec, once the "new" process has been bootstrapped.
     // We decide which mi's that were present in the pre-exec process should be
     // carried over to the new process.  For those that should, the instrumentation
     // is actually inserted.  For others, the component mi in question is removed from
     // the system.

  // remove an instance from an aggregate metric
  void removeThisInstance();

  bool anythingToManuallyTrigger() const;

  void adjustManuallyTrigger();
  void adjustManuallyTrigger(vector<Address> stack_pcs); // should make it private
#if defined(MT_THREAD)
  void adjustManuallyTrigger0();
#endif
  void manuallyTrigger(int);
  void manuallyTrigger(int, int);

#if defined(MT_THREAD)
  void propagateId(int);
  bool& needData(void) { return needData_; }
#endif
  bool inserted(void)     { return inserted_; }
  bool installed(void)    { return installed_; }

  bool isInitialActualValueSet() { return !mdnInitActualVal.isNaN(); }
  bool isStartTimeSet()    { return mdnStartTime.isInitialized(); }
  timeStamp getStartTime() { return mdnStartTime; }

  dataReqNode* getFlagDRN(void);

  metricDefinitionNode* matchInMIPrimitives();
  bool condMatch(metricDefinitionNode *mn,
		 vector<dataReqNode*> &data_tuple1,
		 vector<dataReqNode*> &data_tuple2);

#if defined(MT_THREAD)
  metricDefinitionNode * getMetricPrim() {
    // should be the last of its components
    assert(mdn_type_ == COMP_MDN);
    unsigned csize = components.size();
    return (components[csize-1]);
  }
  metricDefinitionNode * getThrComp(string tname) {
    assert(mdn_type_ == PRIM_MDN);
    unsigned csize = components.size();
    assert(csize == thr_names.size());

    for (unsigned u=0; u<csize; u++)
      if (tname == thr_names[u])
	return components[u];

    return NULL;
  }
  metricDefinitionNode * getProcComp() {
    assert(mdn_type_ == PRIM_MDN);

    return aggregators[0];
  }
  // --- ---
  void rmCompFlatName(unsigned u) {
    assert(COMP_MDN == mdn_type_);
    unsigned size = comp_flat_names.size();
    assert(u < size);
    
    extern dictionary_hash<string, metricDefinitionNode*> allMIComponents;
    if (allMIComponents.defines(comp_flat_names[u])) {
      allMIComponents.undef(comp_flat_names[u]);
    }
    
    comp_flat_names[u] = comp_flat_names[size-1];
    comp_flat_names.resize(size-1);
  }

  void addCompFlatName(string proc_flat_name) {
    assert(COMP_MDN == mdn_type_);
    comp_flat_names += proc_flat_name;
  }

  void addThrName(string thr_name) {
    assert(PRIM_MDN == mdn_type_);
    thr_names += thr_name;
  }
#endif
  
private:
  // Since we don't define these, make sure they're not used:
  metricDefinitionNode &operator=(const metricDefinitionNode &src);
  metricDefinitionNode(const metricDefinitionNode &src);

 public:
  void removeComponent(metricDefinitionNode *comp);

 private:
  friend void mdnContinueCallback(timeStamp timeOfCont);

  void updateWithDeltaValue(timeStamp startTime, timeStamp sampleTime, 
			    pdSample value);
  void tryAggregation();

  void endOfDataCollection();
  void removeFromAggregate(metricDefinitionNode *comp, int deleteComp = 1);

  // this function checks if we need to do stack walk
  // if all returnInstance's overwrite only 1 instruction, no stack walk necessary
  bool needToWalkStack() const;

  metricStyle metStyle() { return EventCounter; }

  // @@ METRIC FIELD STARTS FROM HERE :
#if defined(MT_THREAD)
  //  AGG_LEVEL              aggLevel;// level of aggregation.
                                  // AGG_LEV:    top level (aggregate)
                                  // THR_LEV:    same as below, only those for metric are added to allMIComponents
                                  // PROC_COMP:
                                  // PROC_PRIM:  process component level (previously non aggregate
                                  //             but now aggregate);  for constraint and metric
                                  // THR_LEV:    thread component level. It has no instrumentation associated,
                                  //             only dataReqNodes and sampling data;  for constraint and metric
  bool			needData_ ;
#endif
  MDN_TYPE              mdn_type_;

  aggregateOp           aggOp;
  bool			inserted_;
  bool                  installed_;

  string met_;			     // what type of metric
  vector< vector<string> > focus_;
  vector< vector<string> > component_focus; // defined for component mdn's only
  string flat_name_;

  // comments only for NON_MT_THREAD version:
  // for aggregate metrics and component (non-aggregate) metrics
  // for component metrics: the last is "base", others are all constraints
  vector<metricDefinitionNode*>   components;	
  sampleAggregator aggregator;    // current aggregate value
  //  aggregator should be consistent with components to some extents
  //  should be added or removed if necessary;
  //  also added in AGG_LEV constructor and addPart
  timeStamp mdnStartTime;    // the time that this metric started

  pdSample mdnInitActualVal;  // the initial actual value for this mdn

  // for component (non-aggregate) and primitive metrics
  vector<dataReqNode*>	dataRequests;  //  for THR_LEV only

  vector<instReqNode> instRequests;    //  for PROC_PRIM only
  vector<returnInstance *> returnInsts;//  for PROC_PRIM only, follow instRequests

  vector<instReqNode *> manuallyTriggerNodes;
                                       //  for PROC_PRIM only, follow instRequests

  bool _sentInitialActualValue;        //  true when initial actual value has
                                       //  been sent to the front-end
  pdSample cumulativeValue;            //  seems only for THR_LEV, from which actual data is collected

                                       //  aggregators and samples should always be consistent
                                       //  added in AGG_LEV constructor and addPart
  // which metricDefinitionNode depend on this value.
  vector<metricDefinitionNode*>   aggregators;
  // remember, there can be more than one.  E.g. if cpu for whole program and
  // cpu for process 100 are chosen, then we'll have just one component mi which is
  // present in two aggregate mi's (cpu/whole and cpu/proc-100).
  
  vector<aggComponent *> samples;
  // defined for component mi's only -- one sample for each aggregator, usually
  // allocated with "aggregateMI.aggregator.newComponent()".
  // samples[i] is the sample of aggregators[i].

#if defined(MT_THREAD)
                                       //  following 5 memorizing stuff --- for PROC_COMP only
  // data required to add threads - naim
  unsigned type_thr;
  bool computingCost_thr;
  vector<string> *temp_ctr_thr;
  vector<T_dyninstRPC::mdl_constraint*> flag_cons_thr;
  // could be multiple mdl_constraints
  vector<T_dyninstRPC::mdl_constraint*>  base_use_thr;

                                       //  following 4 --- for PROC_COMP only
  vector<string> comp_flat_names;      //  should be consistent with PROC_COMP's aggregators

  vector<string> thr_names;            //  for PROC_PRIM only, remember names of each of its threads (tid + start_func_name)
                                       //  should be consistent with PROC_PRIM's components
#endif
  
  int id_;			       //  unique id for this "AGG_LEV" metricDefinitionNoe; no meaning for other kinds of mdn
  timeLength originalCost_;
  
  process *proc_;                      //  for NON_AGG_LEV (never changed once initialized)

  metricStyle style_;                  //  never changed once initialized


  // CONSISTENCY GROUPS
  // aggregators, samples (comp_flat_names for PROC_COMP)  \  complicated relations between them
  // components, (aggregator) (thr_names for THR_LEV)           /  should be kept cleanly if possible
  // SPECIALS:  AGG_LEV  --- id_
  //          PROC_COMP  --- temp_ctr_thr... , comp_flat_names
  //          PROC_PRIM  --- instRequests... , thr_names
  //            THR_LEV  --- dataRequests, cumulativeValue
  
  metricDefinitionNode* handleExec();
  // called by static void handleExec(process *), for each component mi
  // returns new component mi if propagation succeeded; NULL if not.
  void oldCatchUp();
  bool checkAndInstallInstrumentation(vector<Address>& pc);
};

class defInst {
 public:
  defInst(string& metric_name, vector<u_int>& focus, int id, 
          pd_Function *func, unsigned attempts);

  string metric() { return metric_name_; }
  vector<u_int>& focus() { return focus_; }
  int id() { return id_; }
  pd_Function *func() { return func_; }
  unsigned numAttempts() { return attempts_; }
  void failedAttempt() { if (attempts_ > 0) attempts_--; }
  
 private:
  string metric_name_;
  vector<u_int> focus_; 
  int id_;
  pd_Function *func_;
  unsigned attempts_;
};

ostream& operator<<(ostream&s, const metricDefinitionNode &m);

//class defInst {
// public:
//  defInst(unsigned, vector<T_dyninstRPC::focusStruct>&, vector<string>&,
//	  vector<u_int>&, u_int&, u_int&);

//  unsigned index() { return index_; }
//  vector<T_dyninstRPC::focusStruct> focus() { return focus_; }
//  vector<string>& metric() { return metric_; }
//  vector<u_int>& ids() { return ids_; }
//  u_int did() { return did_; }
//  u_int rid() { return rid_; }

// private:
//  unsigned index_;
//  vector<T_dyninstRPC::focusStruct> focus_; 
//  vector<string> metric_;
//  vector<u_int> ids_; 
//  u_int did_;
//  u_int rid_;
//};

// inline void metricDefinitionNode::addInst(instPoint *point, AstNode *ast,
//					  callWhen when,
//					  callOrder o) {
//  if (!point) return;
//
//  instReqNode temp(point, ast, when, o);
//  instRequests += temp;
// };

// allMIs: all aggregate (as opposed to component) metricDefinitionNodes
extern dictionary_hash<unsigned, metricDefinitionNode*> allMIs;

// allMIComponents: all component (as opposed to aggregate) metricDefinitionNodes,
// indexed by the component's unique flat_name.
extern dictionary_hash<string, metricDefinitionNode*> allMIComponents;

// allMIPrimitives: all primitives (variable with all its modifications) metricDefinitionNodes,
// indexed by the primitive's unique flat_name (metric primitive should have the same unique
// flat_name as its component
extern dictionary_hash<string, metricDefinitionNode*> allMIPrimitives;

#if defined(MT_THREAD)
extern dictionary_hash<string, metricDefinitionNode*> allMIinstalled;
#endif

// don't access this directly, consider private
extern timeLength currentPredictedCost;

// Access currentPredictedCost through these functions, should probably
// be included in some class or namespace in the future
inline timeLength &getCurrentPredictedCost() {
  return currentPredictedCost;
}
inline void setCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost = tl;
}
inline void addCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost += tl;
}
inline void subCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost -= tl;
}

#ifndef SHM_SAMPLING
extern void processCost(process *proc, traceHeader *h, costUpdate *s);
#endif

extern void reportInternalMetrics(bool force);

bool toDeletePrimitiveMDN(metricDefinitionNode *prim);

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
timeLength guessCost(string& metric_name, vector<u_int>& focus);


/*
 * process a sample ariving from an inferior process
 *
 */
#ifndef SHM_SAMPLING
void processSample(int pid, traceHeader*, traceSample *);
#endif

bool AstNode_condMatch(AstNode* a1, AstNode* a2,
		       vector<dataReqNode*> &data_tuple1, // initialization?
		       vector<dataReqNode*> &data_tuple2,
		       vector<dataReqNode*> datareqs1,
		       vector<dataReqNode*> datareqs2);

#endif


// need to make dataReqNode return their type info


 
