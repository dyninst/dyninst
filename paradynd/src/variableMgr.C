/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: variableMgr.C,v 1.17 2004/04/07 20:20:36 bernat Exp $

#include <sys/types.h>
#include "common/h/Types.h"
#include "common/h/headers.h"
#include "paradynd/src/variableMgr.h"
#include "rtinst/h/rtinst.h" // for time64
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "paradynd/src/shmMgr.h"
#include "paradynd/src/varTable.h"
#include "paradynd/src/varInstanceHKs.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/varInstance.h"

variableMgr::variableMgr(pd_process *proc, shmMgr *shmMgr_, 
			 unsigned maxNumberOfThreads) : 
  maxNumberOfThreads(maxNumberOfThreads), applicProcess(proc), 
  theShmMgr(*shmMgr_)
{
  // One instance per process object.
   if(proc->multithread_capable())
      maxNumberOfThreads = MAX_NUMBER_OF_THREADS;
   else
      maxNumberOfThreads = 1;

#ifdef PAPI
  varTables.resize(5);
#else
  varTables.resize(3);
#endif

  varTables[Counter] = new varTable<intCounterHK>(*this);
  varTables[WallTimer] = new varTable<wallTimerHK>(*this);
  varTables[ProcTimer] = new varTable<processTimerHK>(*this);
#ifdef PAPI
  varTables[HwTimer] = new varTable<hwTimerHK>(*this);
  varTables[HwCounter] = new varTable<hwCounterHK>(*this);
#endif
  // Insert other timer/counter types here...

  // Preallocate for the varTables
  // What's a good number? Well... start with 200 and see?
  int prealloc_size;
  if(proc->multithread_capable())
     prealloc_size = 50;
  else 
     prealloc_size = 200;

  for (unsigned i = 0; i < varTables.size(); i++)
    theShmMgr.preMalloc(varTables[i]->getVarSize()*maxNumberOfThreads, prealloc_size);

}

// Fork constructor
variableMgr::variableMgr(const variableMgr &par, pd_process *proc,
			 shmMgr *shmMgr_) :
  maxNumberOfThreads(par.maxNumberOfThreads),
  applicProcess(proc),
  theShmMgr(*shmMgr_)
{
#ifdef PAPI
  varTables.resize(5);
#else
  varTables.resize(3);
#endif
  for(unsigned i=0; i<par.varTables.size(); i++) {
    switch(i) {
      case Counter:
	varTables[Counter] = new varTable<intCounterHK>(
            *dynamic_cast<varTable<intCounterHK>*>(par.varTables[i]), *this);
	break;
      case WallTimer:
	varTables[WallTimer] = new varTable<wallTimerHK>(
            *dynamic_cast<varTable<wallTimerHK>*>(par.varTables[i]), *this);
	break;
      case ProcTimer:
	varTables[ProcTimer] = new varTable<processTimerHK>(
            *dynamic_cast<varTable<processTimerHK>*>(par.varTables[i]), *this);
	break;
#ifdef PAPI
      case HwTimer:
	varTables[HwTimer] = new varTable<hwTimerHK>(
            *dynamic_cast<varTable<hwTimerHK>*>(par.varTables[i]), *this);
	break;
      case HwCounter:
	varTables[HwCounter] = new varTable<hwCounterHK>(
            *dynamic_cast<varTable<hwCounterHK>*>(par.varTables[i]), *this);
	break;
#endif
      default:
	assert(false);
    }
  }
}

variableMgr::~variableMgr() 
{
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    delete varTables[iter];
  }
}

inst_var_index variableMgr::allocateForInstVar(inst_var_type varType, HwEvent* hw)
{
  return varTables[varType]->allocateVar(hw);
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

void variableMgr::free(inst_var_type varType, 
		       inst_var_index varIndex)
{
  varTables[varType]->free(varIndex);
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

void variableMgr::deleteThread(pd_thread *thr)
{
  assert(applicProcess->multithread_capable());
  assert(thr);
  unsigned thrIndex = thr->get_index();

  // This will just go through and zero out all the data. 
  for (unsigned iter = 0; iter < varTables.size(); iter++)
    varTables[iter]->deleteThread(thrIndex);
}

void variableMgr::initializeVarsAfterFork() {
  for (unsigned iter = 0; iter < varTables.size(); iter++) {
    varTables[iter]->initializeVarsAfterFork();
  }
}




