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
 * Revision 1.52  1997/06/23 17:05:07  tamches
 * slight changes to #include
 *
 * Revision 1.51  1997/06/16 22:07:15  ssuen
 * Added `int deleteComp = 1' to end of argument list for function
 *   metricDefinitionNode::removeFromAggregate
 * to control whether or not to `delete' the `comp' argument.
 *
 * Revision 1.50  1997/06/05 18:04:20  naim
 * Cleaning up dataReqNodes for certain cases of deletion of metricDefinitionNodes
 * - naim
 *
 * Revision 1.49  1997/05/08 00:12:16  mjrg
 * changes for Visual C++ compiler: added return to functions
 *
 * Revision 1.48  1997/05/07 19:01:27  naim
 * Getting rid of old support for threads and turning it off until the new
 * version is finished. Additionally, new superTable, baseTable and superVector
 * classes for future support of multiple threads. The fastInferiorHeap class has
 * also changed - naim
 *
 * Revision 1.47  1997/04/21 16:56:15  hseom
 * added support for trace data
 *
 * Revision 1.46  1997/03/18 19:45:57  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 * Revision 1.45  1997/02/26 23:46:43  mjrg
 * First part of WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C file
 *
 * Revision 1.44  1997/02/24 14:22:58  naim
 * Minor fix to my previous commit - naim
 *
 * Revision 1.43  1997/02/21 20:15:59  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.42  1997/01/27 19:41:07  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.41  1997/01/15 01:11:34  tamches
 * completely revamped fork & exec -- they now work for shm sampling.
 *
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
 */

#ifndef METRIC_H
#define METRIC_H

#include "util/h/String.h"
// trace data streams
#include "util/h/ByteArray.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/aggregateSample.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/inst.h" // for "enum callWhen"

class instInstance; // enough since we only use instInstance* in this file

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

  virtual unsigned getInferiorPtr(process *proc=NULL) const = 0;
  virtual unsigned getAllocatedIndex() const = 0;
  virtual unsigned getAllocatedLevel() const = 0;

  virtual int getSampleId() const = 0;

  virtual bool insertInstrumentation(process *, metricDefinitionNode *, bool) = 0;
     // Allocates stuff from inferior heap, instrumenting DYNINSTreportCounter
     // as appropriate.  
     // Returns true iff successful.

  virtual void disable(process *,
		       const vector< vector<unsigned> > &pointsToCheck) = 0;
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
 *    shmSampledIntCounterReqNode --
 *       provides an intCounter value that is sampled with shm sampling.  The
 *       intCounter is allocated from the shm segment (i.e., not with inferiorMalloc).
 *
 *    shmSampledWallTimerReqNode --
 *       provides a wall timer value that is sampled with shm sampling.  The tTimer is
 *       allocated from the shm segment (i.e., not with inferiorMalloc)
 *    
 *    shmSampledProcTimerReqNode --
 *       provides a process timer value that is sampled with shm sampling.  The tTimer
 *       is allocated from the shm segment (i.e., not with inferiorMalloc)
 *    
 * C) classes used only when _not_ shm-sampling
 *    
 *    alarmSampledIntCounterReqNode --
 *       provides an intCounter value that is sampled.  The intCounter is allocated
 *       in the conventional heap with inferiorMalloc().  Sampling is done the
 *       old-fasioned SIGALRM way: by instrumenting DYNINSTsampleValues to call
 *       DYNINSTreportTimer
 *
 *    alarmSampledTimerReqNode --
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
   int initialValue; // needed when dup()'ing

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
   sampledIntCounterReqNode(int iValue, int iCounterId,
                            metricDefinitionNode *iMi, bool computingCost);
  ~sampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.  A bit of a complication
      // since disable() needs an arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &map) const;

   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
      // allocates from inferior heap; initializes it; instruments
      // DYNINSTsampleValues

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const {
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      //assert(counterPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)counterPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}

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

   int initialValue; // needed when dup()'ing

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
   sampledShmIntCounterReqNode(int iValue, int iCounterId, 
                               metricDefinitionNode *iMi, bool computingCost,
                               bool doNotSample);
  ~sampledShmIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

   bool insertInstrumentation(process *, metricDefinitionNode *, bool);
      // allocates from inferior heap; initializes it, etc.

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}
   int getSampleId() const {return theSampleId;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};
#endif

class nonSampledIntCounterReqNode : public dataReqNode {
 // intCounter for predicates (because they don't need to be sampled).
 // Allocated in the conventional heap with inferiorMalloc().
 private:
   // The following fields are always properly initialized in ctor:
   int theSampleId;
   int initialValue; // needed when dup()'ing

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
   nonSampledIntCounterReqNode(int iValue, int iCounterId,
                               metricDefinitionNode *iMi, bool computingCost);
  ~nonSampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
      // allocates from inferior heap; initializes it

   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const {
      //assert(counterPtr != NULL); // NULL until insertInstrumentation()
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      return (unsigned)counterPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}

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

   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const {
      // counterPtr could be NULL if we are building AstNodes just to compute
      // the cost - naim 2/18/97
      //assert(timerPtr != NULL); // NULL until insertInstrumentation()
      return (unsigned)timerPtr;
   }

   unsigned getAllocatedIndex() const {assert(0); return 0;}
   unsigned getAllocatedLevel() const {assert(0); return 0;}

   int getSampleId() const {return theSampleId;}

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
   sampledShmWallTimerReqNode(int iCounterId,
                              metricDefinitionNode *iMi, bool computingCost);
  ~sampledShmWallTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};

class sampledShmProcTimerReqNode : public dataReqNode {
 // process tTimer for use when shm sampling.  Allocated in the shm segment heap.
 // Sampled using shm-sampling.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;

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
   sampledShmProcTimerReqNode(int iCounterId,
			      metricDefinitionNode *iMi, bool computingCost);
  ~sampledShmProcTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, metricDefinitionNode *, int iCounterId,
                    const dictionary_hash<instInstance*,instInstance*> &) const;

   bool insertInstrumentation(process *, metricDefinitionNode *, bool doNotSample=false);
   void disable(process *, const vector< vector<unsigned> > &);

   unsigned getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
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

  static instReqNode forkProcess(const instReqNode &parent,
                                 const dictionary_hash<instInstance*,instInstance*> &);
     // should become a copy-ctor...or at least, a non-static member fn.

  bool unFork(dictionary_hash<instInstance*, instInstance*> &map) const;
     // The fork syscall duplicates all trampolines from the parent into the child. For
     // those mi's which we don't want to propagate to the child, this creates a
     // problem.  We need to remove instrumentation code from the child.  This routine
     // does that.  "map" maps instInstances of the parent to those in the child.

  instInstance *getInstance() const { return instance; }

  bool anythingToManuallyTrigger() const {return manuallyTrigger;}
  bool triggerNow(process *theProc);

private:
  instPoint	*point;
  AstNode	*ast;
  callWhen	when;
  callOrder	order;
  instInstance	*instance; // undefined until insertInstrumentation() calls addInstFunc
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
   static int counterId;

public:
  // styles are enumerated in util/h/aggregation.h
  metricDefinitionNode(process *p, const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
		       const string& component_flat_name, int agg_style = aggSum);
     // for component (per-process) (non-aggregate) mdn's

  metricDefinitionNode(const string& metric_name, const vector< vector<string> >& foc,
		       const string& cat_name,
		       vector<metricDefinitionNode*>& parts,
		       int agg_op);
     // for aggregate (not component) mdn's

  ~metricDefinitionNode();
  void disable();
  void cleanup_drn();
  void updateValue(time64, sampleValue);
  void forwardSimpleValue(timeStamp, timeStamp, sampleValue,unsigned,bool);

  int getMId() const { return id_; }
  const string &getMetName() const { return met_; }
  const string &getFullName() const { return flat_name_; }
  const vector< vector<string> > &getFocus() const {return focus_;}
  const vector< vector<string> > &getComponentFocus() const {
     assert(!aggregate_); // defined for component mdn's only
     return component_focus;
  }

  process *proc() const { return proc_; }

  bool nonNull() const { return (instRequests.size() || dataRequests.size()); }
  bool insertInstrumentation();

  float cost() const;
  bool checkAndInstallInstrumentation();

  float originalCost() const { return originalCost_; }

  // The following routines are (from the outside world's viewpoint)
  // the heart of it all.  They append to dataRequets or instRequests, so that
  // a future call to metricDefinitionNode::insertInstrumentation() will
  // "do their thing".  The MDL calls these routines.
  dataReqNode *addSampledIntCounter(int initialValue, bool computingCost,
                                    bool doNotSample=false);
  dataReqNode *addUnSampledIntCounter(int initialValue, bool computingCost);
  dataReqNode *addWallTimer(bool computingCost);
  dataReqNode *addProcessTimer(bool computingCost);
  inline void addInst(instPoint *point, AstNode *, callWhen when, 
                      callOrder o,
		      bool manuallyTrigger);

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

  void manuallyTrigger();

private:
  // Since we don't define these, make sure they're not used:
  metricDefinitionNode &operator=(const metricDefinitionNode &src);
  metricDefinitionNode(const metricDefinitionNode &src);

  void removeComponent(metricDefinitionNode *comp);
  void endOfDataCollection();
  void removeFromAggregate(metricDefinitionNode *comp, int deleteComp = 1);

  void updateAggregateComponent();

  bool			aggregate_;
  int aggOp; // the aggregate operator
  bool			inserted_;
  bool                  installed_;
  string met_;			// what type of metric
  vector< vector<string> > focus_;
  vector< vector<string> > component_focus; // defined for component mdn's only
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
     // remember, there can be more than one.  E.g. if cpu for whole program and
     // cpu for process 100 are chosen, then we'll have just one component mi which is
     // present in two aggregate mi's (cpu/whole and cpu/proc-100).

  vector<sampleInfo *> samples;
     // defined for component mi's only -- one sample for each aggregator, usually
     // allocated with "aggregateMI.aggSample.newComponent()".
     // samples[i] is the sample of aggregators[i].

  int id_;				// unique id for this one 
  float originalCost_;

  process *proc_;

  string metric_name_;
  metricStyle style_; 

  metricDefinitionNode* handleExec();
     // called by static void handleExec(process *), for each component mi
     // returns new component mi if propagation succeeded; NULL if not.
};

inline void metricDefinitionNode::addInst(instPoint *point, AstNode *ast,
					  callWhen when,
					  callOrder o,
                                          bool manuallyTrigger) {
  if (!point) return;

  instReqNode temp(point, ast, when, o, manuallyTrigger);
  instRequests += temp;
};

// allMIs: all aggregate (as opposed to component) metricDefinitionNodes
extern dictionary_hash<unsigned, metricDefinitionNode*> allMIs;

// allMIComponents: all component (as opposed to aggregate) metricDefinitionNodes,
// indexed by the component's unique flat_name.
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
void processSample(int pid, traceHeader*, traceSample *);
#endif

#endif
