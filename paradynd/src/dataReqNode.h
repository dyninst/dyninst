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
#include "superTableTypes.h"

class process;
class threadMetFocusNode_Val;
class instInstance;
class pdThread;

class dataReqNode {
 private:
  // Since we don't define these, make sure they're not used:
  dataReqNode &operator=(const dataReqNode &src);

  process *proc;

  inst_var_type varType;
  inst_var_index varIndex;
  unsigned threadPos;
  int theSampleId; 
  bool dontInsertData;

 protected:
  dataReqNode(process *proc_, inst_var_type varType_, inst_var_index varIndex_,
	      unsigned thrPos, int sampleId, bool dontInsertData_) 
     : proc(proc_), varType(varType_), varIndex(varIndex_), threadPos(thrPos),
     theSampleId(sampleId), dontInsertData(dontInsertData_)
  { }
    
 public:
  virtual ~dataReqNode() {};

  static inst_var_index allocateForInstVar(process *proc, 
					   inst_var_type varType);
  
  unsigned getLevel() const { return int(varType); }
  inst_var_index getVarIndex()  const { return varIndex;  }
  unsigned getThreadPos() const { return threadPos; }

  int getSampleId() const { return theSampleId; }
  bool getDontInsertData() const { return dontInsertData; }

  Address getInferiorPtr() const;

  void markAsSampled();
  void markAsNotSampled();

  // the opposite of insertShmVar.  Deinstruments, deallocates
  // from inferior heap, etc.
  void disable(const vector< vector<Address> > &pointsToCheck);

  void setThrNodeClient(threadMetFocusNode_Val *thrObj);

  //  virtual dataReqNode *dup(process *childProc, int iCounterId,
  //			   const dictionary_hash<instInstance*,instInstance*> &map
  //			   ) const = 0;
     // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

     // "map" provides a dictionary that maps instInstance's of the parent process to
     // those in the child process; dup() may find it useful. (for now, the shm
     // dataReqNodes have no need for it; the alarm sampled ones need it).

//  virtual bool unFork(dictionary_hash<instInstance*,instInstance*> &map) = 0;
     // the fork syscall duplicates all instrumentation (except on AIX),
     // which is a problem when we determine that a certain mi shouldn't be
     // propagated to the child process.  Hence this routine.
};

class sampledIntCounterReqNode : public dataReqNode {
 private:
   rawTime64 initialValue; // needed when dup()'ing

   // Since we don't use these, making them privates ensures they're not used.
   sampledIntCounterReqNode &operator=(const sampledIntCounterReqNode &);
   sampledIntCounterReqNode(const sampledIntCounterReqNode &);

   // private fork-ctor called by dup():
   sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
			    process *childProc,
			    int iCounterId, const process *parentProc);

 public:
   sampledIntCounterReqNode(process *proc, inst_var_index varIndex,
			    pdThread *thr, rawTime64 iValue,
			    int iCounterId, bool dontInsertData_);
  ~sampledIntCounterReqNode() {}
      // Hopefully, disable() has already been called.
      // A bit of a complication since disable() needs an
      // arg passed, which we can't do here.  Too bad.
  
   dataReqNode *dup(process *, int iCounterId,
                  const dictionary_hash<instInstance*,instInstance*> &) const;

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};


class sampledWallTimerReqNode : public dataReqNode {
 private:
   // fork ctor:
   sampledWallTimerReqNode(const sampledWallTimerReqNode &src, 
			   process *childProc, int iCounterId,
			   const process *parentProc);

 public:
   sampledWallTimerReqNode(process *proc, inst_var_index varIndex,
			   pdThread *thr, int iCounterId, 
			   bool dontInsertData_);
  ~sampledWallTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId,
                  const dictionary_hash<instInstance*,instInstance*> &) const;
   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};



class sampledProcTimerReqNode : public dataReqNode {
 private:
   // fork ctor:
   sampledProcTimerReqNode(const sampledProcTimerReqNode &src, 
			   process *proc, int iCounterId,
			   const process *parentProc);

 public:
   sampledProcTimerReqNode(process *proc, inst_var_index varIndex,
			   pdThread *thr, 
			   int iCounterId, bool dontInsertData_);
  ~sampledProcTimerReqNode() {}
      // hopefully, freeInInferior() has already been called
      // a bit of a complication since freeInInferior() needs an
      // arg passed, which we can't do here.  Too bad.

   dataReqNode *dup(process *childProc, int iCounterId,
		  const dictionary_hash<instInstance*,instInstance*> &) const;

   bool unFork(dictionary_hash<instInstance*,instInstance*> &) {return true;}
};



#endif
