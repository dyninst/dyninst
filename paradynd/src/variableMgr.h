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

// $Id: variableMgr.h,v 1.2 2002/05/04 21:47:04 schendel Exp $
// The variableMgr class is the top-level view of the actual
// shared, sampled counters and timers. The provides a logical way 
// to reference the counters and timers (called variables) for creation,
// deletion, and sampling.
//
// Organization: the variableMgr consists of several varTables. Each
// varTable is associated with a single type of variable (counter, procTimer,
// wallTimer). The variableMgr basically just routes creation/deletion
// and sampling requests to the appropriate varTable.

#ifndef _VARIABLE_MGR_H_
#define _VARIABLE_MGR_H_

#include "common/h/Vector.h"
#include "paradynd/src/variableMgrTypes.h"

class process;
class pdThread;
class shmMgr;
class baseVarTable;
class threadMetFocusNode_Val;

class variableMgr {
 private:
  unsigned maxNumberOfThreads; // Set at creation time and unchangeable

  // This is where we'd add another type to be sampled. 
  vector<baseVarTable *> varTables;

  // One variableMgr per process.
  process *applicProcess;
  shmMgr &theShmMgr;

  static const unsigned garbageCollectionThreshhold;
  unsigned memUsed_lastGarbageCollect;
  
  // since we don't define them, make sure they're not used:
  variableMgr(const variableMgr &);
  variableMgr &operator=(const variableMgr &);
  void garbageCollect();

 public:
  variableMgr(process *proc, shmMgr *shmMgr, 
	      unsigned i_currMaxNumberOfThreads);
  variableMgr(const variableMgr &parentVarMgr, process *proc,
	      shmMgr *shmMgr_);
  
  ~variableMgr();

  // only used by a MT process
  unsigned getMaxNumberOfThreads() { return maxNumberOfThreads; }
  shmMgr &getShmMgr() { return theShmMgr; }
  process *getApplicProcess() { return applicProcess; }
  
  inst_var_index allocateForInstVar(inst_var_type varType);
  
  void markVarAsSampled(inst_var_type varType, inst_var_index varIndex,
		       unsigned thrPos, threadMetFocusNode_Val *thrNval) const;
  
  void markVarAsNotSampled(inst_var_type varType, inst_var_index varIndex,
			   unsigned thrPos) const;
  
  void makePendingFree(inst_var_type varType, inst_var_index varIndex,
		       const vector<Address> &trampsUsing);
  
  void *shmVarDaemonAddr(inst_var_type varType, inst_var_index varIndex) const;
  
  void *shmVarApplicAddr(inst_var_type varType, inst_var_index varIndex) const;

  // does doMajorSample for three types (intCounter, wallTimer and procTimer)
  bool doMajorSample();
  
  // does doMinorSample for three types (intCounter, wallTimer and procTimer)
  bool doMinorSample();
  
  void handleExec();
  void forkHasCompleted();
  
  unsigned getCurrentNumberOfThreads();

  // Might not need these anymore.
  void addThread(pdThread *thr);
  
  void deleteThread(pdThread *thr);
};

#endif
