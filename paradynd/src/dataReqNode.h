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

#ifndef DATA_REQ_NODE
#define DATA_ROQ_NODE

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "rtinst/h/rtinst.h"

class process;
class threadMetFocusNode_Val;
class instInstance;
class pdThread;

class dataReqNode {
 private:
  // Since we don't define these, make sure they're not used:
  dataReqNode &operator=(const dataReqNode &src);
  dataReqNode (const dataReqNode &src);

 protected:
  dataReqNode() { }

 public:
  virtual ~dataReqNode() {};

  virtual Address getInferiorPtr(process *proc=NULL) const = 0;
  virtual unsigned getAllocatedIndex() const = 0;
  virtual unsigned getAllocatedLevel() const = 0;

  virtual int getSampleId() const = 0;

  virtual int getThreadId() const = 0;

     // Allocates stuff from inferior heap, instrumenting DYNINSTreportCounter
     // as appropriate.  
     // Returns true iff successful.

  virtual void disable(process *,
		       const vector< vector<Address> > &pointsToCheck) = 0;
     // the opposite of insertShmVar.  Deinstruments, deallocates
     // from inferior heap, etc.

  virtual void setThrNodeClient(threadMetFocusNode_Val *thrObj,
				process *proc) = 0;

  virtual dataReqNode *dup(process *childProc, int iCounterId,
			   const dictionary_hash<instInstance*,instInstance*> &map
			   ) const = 0;
     // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

     // "map" provides a dictionary that maps instInstance's of the parent process to
     // those in the child process; dup() may find it useful. (for now, the shm
     // dataReqNodes have no need for it; the alarm sampled ones need it).

  virtual bool unFork(dictionary_hash<instInstance*,instInstance*> &map) = 0;
     // the fork syscall duplicates all instrumentation (except on AIX),
     // which is a problem when we determine that a certain mi shouldn't be
     // propagated to the child process.  Hence this routine.
};



/*
 * Classes derived from dataReqNode
 *
 * There are a lot of classes derived from dataReqNode, so things can get a
 * little confusing.  Most of them are used in either shm_sampling mode or
 * not; few are used in both.  Here's a bit of documentation that should sort
 * things out:
 *
 * A) classes used in _both_ shm_sampling and non-shm-sampling
 *
 *    nonSampledIntCounterReqNode --
 *       provides an intCounter value that is never sampled.  Currently
 *       useful for predicates (constraint booleans).  The intCounter is
 *       allocated from the "conventional" heap (where tramps are allocated)
 *       using good old inferiorMalloc(), even when shm_sampling.  (In the
 *       future, we may provide a separate shm_sampling version, to make
 *       allocation faster [allocation in the shm seg heap is always faster])
 *
 * B) classes used only when shm_sampling
 *
 *    sampledShmIntCounterReqNode --
 *       provides an intCounter value that is sampled with shm sampling.  The
 *       intCounter is allocated from the shm segment (i.e., not with
 *       inferiorMalloc).
 *
 *    sampledShmWallTimerReqNode --
 *       provides a wall timer value that is sampled with shm sampling.  The
 *       tTimer is allocated from the shm segment (i.e., not with
 *       inferiorMalloc)
 *    
 *    sampledShmProcTimerReqNode --
 *       provides a process timer value that is sampled with shm sampling.
 *       The tTimer is allocated from the shm segment (i.e., not with
 *       inferiorMalloc)
 *    
 * C) classes used only when _not_ shm-sampling
 *    
 *    sampledIntCounterReqNode --
 *       provides an intCounter value that is sampled.  The intCounter is
 *       allocated in the conventional heap with inferiorMalloc().  Sampling
 *       is done the old-fasioned SIGALRM way: by instrumenting
 *       DYNINSTsampleValues to call DYNINSTreportTimer
 *
 *    sampledTimerReqNode --
 *       provides a tTimer value (can be wall or process timer) that is
 *       sampled.  The tTimer is allocated in the conventional heap with
 *       inferiorMalloc().  Sampling is done the old-fasioned SIGALRM way: by
 *       instrumenting DYNINSTsampleValues to call DYNINSTreportTimer.
 *
 */

class sampledShmIntCounterReqNode : public dataReqNode {
 // intCounter for use when shm-sampling.  Allocated in the shm segment heap.
 // Sampled using shm sampling.
 private:
   // The following fields are always properly initialized in ctor:
   int theSampleId; // obsolete with shm sampling; can be removed.
   pdThread *thr_;

   rawTime64 initialValue; // needed when dup()'ing

   // The following fields are NULL until insertShmVar() called:
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // Since we don't use these, making them privates ensures they're not used.
   sampledShmIntCounterReqNode &operator=(const sampledShmIntCounterReqNode &);
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
                               process *childProc,
			       int iCounterId, const process *parentProc);
   // allocates from inferior heap; initializes it, etc.
   bool insertShmVar(pdThread *, process *, bool);

 public:
   sampledShmIntCounterReqNode(pdThread *thr, rawTime64 iValue, int iCounterId,
			       process *proc, bool dontInsertData,
                               bool doNotSample, unsigned, unsigned);
  ~sampledShmIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *, int iCounterId,
                   const dictionary_hash<instInstance*,instInstance*> &) const;

   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const;
   void setThrNodeClient(threadMetFocusNode_Val *thrObj, process *proc);

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}
   int getSampleId() const {return theSampleId;}
   int getThreadId() const;
   pdThread *getThread() {return thr_;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};



class sampledShmWallTimerReqNode : public dataReqNode {
 // wall tTimer for use when shm sampling.  Allocated in the shm segment heap.
 // Sampled using shm-sampling.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;
   pdThread *thr_;

   // The following fields are NULL until insertShmVar():
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // since we don't use these, disallow:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &);
   sampledShmWallTimerReqNode &operator=(const sampledShmWallTimerReqNode &);

   // fork ctor:
   sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src, 
			      process *childProc, int iCounterId,
			      const process *parentProc);
   bool insertShmVar(pdThread *, process *, bool doNotSample = false);

 public:
   sampledShmWallTimerReqNode(pdThread *thr, int iCounterId,
                              process *proc, bool dontInsertData,
			      unsigned, unsigned);
  ~sampledShmWallTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId,
                   const dictionary_hash<instInstance*,instInstance*> &) const;
   void disable(process *, const vector< vector<Address> > &);

   Address getInferiorPtr(process *) const;
   void setThrNodeClient(threadMetFocusNode_Val *thrObj, process *proc);

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}
   int getThreadId() const;
   pdThread *getThread() {return thr_;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};



class sampledShmProcTimerReqNode : public dataReqNode {
 // process tTimer for use when shm sampling.  Allocated in the shm segment heap.
 // Sampled using shm-sampling.
 private:
   // The following fields are always initialized in the ctor:   
   int theSampleId;
   pdThread *thr_;

   // The following fields are NULL until insertShmVar():
   unsigned allocatedIndex;
   unsigned allocatedLevel;

   unsigned position_;

   // since we don't use these, disallow:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &);
   sampledShmProcTimerReqNode &operator=(const sampledShmProcTimerReqNode &);

   // fork ctor:
   sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src, 
			      process *proc, int iCounterId,
			      const process *parentProc);
   bool insertShmVar(pdThread *, process *, bool doNotSample = false);

 public:
   sampledShmProcTimerReqNode(pdThread *thr, int iCounterId,
			      process *proc, bool dontInsertData,
			      unsigned, unsigned);
  ~sampledShmProcTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId,
		   const dictionary_hash<instInstance*,instInstance*> &) const;
   void disable(process *, const vector< vector<Address> > &);

   void setThrNodeClient(threadMetFocusNode_Val *thrObj, process *proc);
   Address getInferiorPtr(process *) const;

   unsigned getAllocatedIndex() const {return allocatedIndex;}
   unsigned getAllocatedLevel() const {return allocatedLevel;}

   unsigned getPosition() const {return position_;}

   int getSampleId() const {return theSampleId;}
   int getThreadId() const;
   pdThread *getThread() {return thr_;}

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};



#endif
