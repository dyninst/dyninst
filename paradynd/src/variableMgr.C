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

// $Id: variableMgr.C,v 1.2 2002/05/04 21:47:03 schendel Exp $

#include <sys/types.h>
#include "common/h/Types.h"
#include "common/h/headers.h"
#include "paradynd/src/variableMgr.h"
#include "rtinst/h/rtinst.h" // for time64 and MAX_NUMBER_OF_THREADS
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/shmMgr.h"
#include "paradynd/src/varTable.h"
#include "paradynd/src/varInstanceHKs.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/varInstance.h"


// do garbage collection for every 200K allocated in shmMgr
const unsigned variableMgr::garbageCollectionThreshhold(200000);

variableMgr::variableMgr(process *proc, shmMgr *shmMgr_, 
			 unsigned maxNumberOfThreads) : 
  maxNumberOfThreads(maxNumberOfThreads), applicProcess(proc), 
  theShmMgr(*shmMgr_), memUsed_lastGarbageCollect(0)
{
  // One instance per process object.
#if defined(MT_THREAD)
  maxNumberOfThreads = MAX_NUMBER_OF_THREADS;
#endif
  varTables.resize(3);
  varTables[Counter] = new varTable<intCounterHK>(*this);
  varTables[WallTimer] = new varTable<wallTimerHK>(*this);
  varTables[ProcTimer] = new varTable<processTimerHK>(*this);
  // Insert other timer/counter types here...

}

// Fork constructor
/*
variableMgr::variableMgr(const variableMgr &parentVarMgr, process *proc,
			 shmMgr *shmMgr_) :
          maxNumberOfThreads(parentVarMgr.maxNumberOfThreads),
          applicProcess(proc),
	  theShmMgr(*shmMgr_),
	  memUsed_lastGarbageCollect(0)
{

}
*/

variableMgr::~variableMgr() 
{
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    delete varTables[iter];
  }
}

inst_var_index variableMgr::allocateForInstVar(inst_var_type varType)
{
  unsigned curMemUsed = theShmMgr.memAllocated();
  // for every additional 200K mem allocated, do garbage collection
  if(int(curMemUsed) - int(memUsed_lastGarbageCollect) > 
     int(garbageCollectionThreshhold)) {
    garbageCollect();
    memUsed_lastGarbageCollect = curMemUsed;
  }

  return varTables[varType]->allocateVar();
}

void variableMgr::markVarAsSampled(inst_var_type varType, 
				   inst_var_index varIndex, unsigned thrPos, 
				   threadMetFocusNode_Val *thrNval) const {
  varTables[varType]->markVarAsSampled(varIndex, thrPos, thrNval);
}

void variableMgr::markVarAsNotSampled(inst_var_type varType,
				      inst_var_index varIndex,
				      unsigned thrPos) const {
  varTables[varType]->markVarAsNotSampled(varIndex, thrPos);
}

void *variableMgr::shmVarDaemonAddr(inst_var_type varType,
				    inst_var_index varIndex) const {
  return varTables[varType]->shmVarDaemonAddr(varIndex);
}

void *variableMgr::shmVarApplicAddr(inst_var_type varType, 
				    inst_var_index varIndex) const {
  return varTables[varType]->shmVarApplicAddr(varIndex);
}

void variableMgr::makePendingFree(inst_var_type varType, 
				 inst_var_index varIndex,
				 const vector<Address> &trampsUsing)
{
  varTables[varType]->makePendingFree(varIndex, trampsUsing);
}

void variableMgr::garbageCollect() {
  bool needToCont = (applicProcess->status() == running);
#ifdef DETACH_ON_THE_FLY
  if ( !applicProcess->reattachAndPause())
#else
  if ( !applicProcess->pause() )
#endif
  {
    cerr << "garbage collect -- pause failed" << endl;
    return;
  }

  vector<Frame> stackWalk;
  applicProcess->walkStack(applicProcess->getActiveFrame(), stackWalk);
  // We can continue right away because we want to know if a variable has a
  // minitramp associated with it which is currently being run.  In the case
  // that it's in the minitramp now but by the time we get to freeing the
  // variable it's no longer in the minitramp, we'll still not free the
  // variable.  This is the same result if we had left the processes paused.
  if( needToCont && (applicProcess->status() != running)) {
#ifdef DETACH_ON_THE_FLY
    applicProcess->detachAndContinue();
#else
    applicProcess->continueProc();
#endif
  }
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    varTables[iter]->garbageCollect(stackWalk);
  }
}

bool variableMgr::doMajorSample()
{
  bool samplePerformed = true;
  
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    samplePerformed &= varTables[iter]->doMajorSample();
  }

  return samplePerformed;
}

bool variableMgr::doMinorSample()
{
  bool samplePerformed = true;
  
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    samplePerformed &= varTables[iter]->doMinorSample();
  }

  return samplePerformed;
}

void variableMgr::handleExec()
{
  for (unsigned iter = 0; iter < varTables.size(); iter++)
    varTables[iter]->handleExec();
}

void variableMgr::forkHasCompleted()
{
  for (unsigned iter = 0; iter < varTables.size(); iter++)
    varTables[iter]->forkHasCompleted();
}

void variableMgr::addThread(pdThread *thr)
{
  unsigned pos, pd_pos;
  pos = thr->get_pos();
  pd_pos = thr->get_pd_pos();

  // Should this be here? Probably not.
#if defined(MT_THREAD)
  for (unsigned i=0;i<applicProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = applicProcess->allMIComponentsWithThreads[i];
    if (mi)  
      mi->addThread(thr); 
  }
#endif

}

void variableMgr::deleteThread(pdThread *thr)
{
  assert(applicProcess->is_multithreaded());
  assert(thr);
  unsigned thrPos = thr->get_pd_pos();

#if defined(MT_THREAD)
  for (unsigned i=0;i<applicProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = applicProcess->allMIComponentsWithThreads[i];
    if (mi) {
      mi->deleteThread(thr);
    }
  }
#endif
  // This will just go through and zero out all the data. 
  for (unsigned iter = 0; iter < varTables.size(); iter++)
    varTables[iter]->deleteThread(thrPos);
}


