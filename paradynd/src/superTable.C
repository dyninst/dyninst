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

// $Id: superTable.C,v 1.14 2002/04/23 18:58:45 schendel Exp $
// The superTable class consists of an array of baseTable elements
// (superVectors) and it represents the ThreadTable in paradynd. For more
// info, please look at the .h file for this class.

#include <sys/types.h>
#include "common/h/Types.h"
#include "common/h/headers.h"
#include "paradynd/src/superTable.h"
#include "rtinst/h/rtinst.h" // for time64 and MAX_NUMBER_OF_THREADS
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/processMetFocusNode.h"

superTable::superTable(process *proc,
		       unsigned maxNumberOfIntCounters,
		       unsigned maxNumberOfWallTimers,
		       unsigned maxNumberOfProcTimers,
		       unsigned i_currMaxNumberOfThreads) :
                       currMaxNumberOfThreads(i_currMaxNumberOfThreads),
		       inferiorProcess(proc)
{
  // Note: there should only be *one* instance of this class

  if(proc->is_multithreaded()) {
#if defined(MT_THREAD)
    maxNumberOfThreads = MAX_NUMBER_OF_THREADS;
    maxNumberOfLevels = MAX_NUMBER_OF_LEVELS;
#endif
  } else {
    maxNumberOfThreads = 1;
    maxNumberOfLevels = 3;
  }

  numberOfCounterLevels = 1;
  numberOfWallTimerLevels = 1;
  numberOfProcTimerLevels = 1;
  numberOfCurrentLevels = numberOfCounterLevels + numberOfWallTimerLevels +
                          numberOfProcTimerLevels;
  if(proc->is_multithreaded()) {
    proc->numOfCurrentLevels_is = numberOfCurrentLevels;
  }
  		                                         
  unsigned nCol, nRow, nElems;
  if(proc->is_multithreaded()) {
    nCol = maxNumberOfThreads;
    nRow = maxNumberOfLevels/numberOfCurrentLevels; 
    numberOfCurrentThreads = 0;
    inferiorProcess->numOfCurrentThreads_is = numberOfCurrentThreads;

#if defined(MT_THREAD)
    proc->threadMap = new hashTable(maxNumberOfThreads, 
				    currMaxNumberOfThreads, 0);
#endif
    nElems = maxNumberOfIntCounters/(nCol*nRow);
    theIntCounterBaseTable = 
      new baseTable<intCounterHK, intCounter>(inferiorProcess, 
				      currMaxNumberOfThreads, 1, 0, nElems, 0);
    
    nElems = maxNumberOfWallTimers/(nCol*nRow);
    theWallTimerBaseTable = 
      new baseTable<wallTimerHK, tTimer>(inferiorProcess,
				      currMaxNumberOfThreads, 1, 1, nElems, 1);

    nElems = maxNumberOfProcTimers/(nCol*nRow);
    theProcTimerBaseTable = 
      new baseTable<processTimerHK, tTimer>(inferiorProcess,
				      currMaxNumberOfThreads, 1, 2, nElems, 2);
  } else {
    nCol = 1;
    nRow = 1;
    nElems = (maxNumberOfIntCounters/nCol)/(maxNumberOfLevels/3);
    theIntCounterBaseTable = new baseTable<intCounterHK, intCounter>(
				    inferiorProcess, nCol, nRow, 0, nElems, 0);
    nElems = (maxNumberOfWallTimers/nCol)/(maxNumberOfLevels/3);
    theWallTimerBaseTable = new baseTable<wallTimerHK, tTimer>(
                                    inferiorProcess, nCol, nRow, 1, nElems, 1);
    nElems = (maxNumberOfProcTimers/nCol)/(maxNumberOfLevels/3);
    theProcTimerBaseTable = new baseTable<processTimerHK, tTimer>(
				    inferiorProcess, nCol, nRow, 2, nElems, 2);
  }
}

superTable::superTable(const superTable &parentSuperTable, process *proc) :
          maxNumberOfThreads(parentSuperTable.maxNumberOfThreads),
          maxNumberOfLevels(parentSuperTable.maxNumberOfLevels),
          numberOfCurrentLevels(parentSuperTable.numberOfCurrentLevels),
          numberOfCounterLevels(parentSuperTable.numberOfCounterLevels),
          numberOfWallTimerLevels(parentSuperTable.numberOfWallTimerLevels),
          numberOfProcTimerLevels(parentSuperTable.numberOfProcTimerLevels),
          numberOfCurrentThreads(parentSuperTable.numberOfCurrentThreads),
          currMaxNumberOfThreads(parentSuperTable.currMaxNumberOfThreads),
          inferiorProcess(proc)
{
  if(proc->is_multithreaded()) {
#if defined(MT_THREAD)
    proc->threadMap = 
      new hashTable(parentSuperTable.inferiorProcess->threadMap);
#endif
  }
  theIntCounterBaseTable = new baseTable<intCounterHK, intCounter>(
			          parentSuperTable.theIntCounterBaseTable,
				  inferiorProcess);
  theWallTimerBaseTable  = new baseTable<wallTimerHK, tTimer>(
                                  parentSuperTable.theWallTimerBaseTable,
				  inferiorProcess);
  theProcTimerBaseTable  = new baseTable<processTimerHK, tTimer>(
                                  parentSuperTable.theProcTimerBaseTable,
				  inferiorProcess);
}

superTable::~superTable() 
{
  delete theIntCounterBaseTable;
  delete theWallTimerBaseTable;
  delete theProcTimerBaseTable;
}

/*  LEVEL SPECIFIC CODE, for when a level is allocated,
    in case it's needed in the future
    ---------------------------------------------------------------
    if (numberOfCurrentLevels+1 <= maxNumberOfLevels) numberOfCurrentLevels++;
    else return(false);
    numberOfCounterLevels++;
    
    inferiorProcess->numOfCurrentLevels_is++;
    if(inferiorProcess->is_multithreaded()) {
    theIntCounterBaseTable->addRows(numberOfCurrentLevels-1, 1);
    } else {
    theIntCounterBaseTable->addRows(numberOfCurrentLevels, 1);
    }
*/

inst_var_index superTable::allocateForInstVar(inst_var_type varType) {
  inst_var_index allocatedVarIndex = 0;
  switch(varType) {
    case Counter: 
      allocatedVarIndex = 
	theIntCounterBaseTable->allocateForInstVar(int(Counter));
      break;
    case WallTimer:
      allocatedVarIndex = 
	theWallTimerBaseTable->allocateForInstVar(int(WallTimer));
      break;
    case ProcTimer:
      allocatedVarIndex = 
	theProcTimerBaseTable->allocateForInstVar(int(ProcTimer));
      break;
  }
  return allocatedVarIndex;
}

void superTable::createCounterVar(inst_var_type varType,
				  inst_var_index varIndex, unsigned thrPos,
				  const intCounter &iValue, 
				  const intCounterHK &iHKValue) {
  assert(varType == Counter);
  theIntCounterBaseTable->createThrInstVar(unsigned(Counter), varIndex, 
					   thrPos, iValue, iHKValue);
}

void superTable::createWallTimerVar(inst_var_type varType,
				    inst_var_index varIndex, unsigned thrPos,
				    const tTimer &iValue, 
				    const wallTimerHK &iHKValue) {
  assert(varType == WallTimer);
  theWallTimerBaseTable->createThrInstVar(unsigned(WallTimer), varIndex, 
					  thrPos, iValue, iHKValue);
}

void superTable::createProcTimerVar(inst_var_type varType,
				    inst_var_index varIndex, unsigned thrPos,
				    const tTimer &iValue, 
				    const processTimerHK &iHKValue) {
  assert(varType == ProcTimer);
  theProcTimerBaseTable->createThrInstVar(unsigned(ProcTimer), varIndex, 
					  thrPos, iValue, iHKValue);
}

void superTable::setBaseAddrInApplic(unsigned type, void *addr)
{
  switch (type) {
    case Counter:
      theIntCounterBaseTable->setBaseAddrInApplic((intCounter *)addr);
      break;
    case WallTimer:
      theWallTimerBaseTable->setBaseAddrInApplic((tTimer *)addr);
      break;
    case ProcTimer:
      theProcTimerBaseTable->setBaseAddrInApplic((tTimer *)addr);
      break;
    default:
      assert(0);
  }
}

void superTable::markVarAsSampled(inst_var_type varType, 
				  inst_var_index varIndex,
				  unsigned thrPos) const
{
  switch (varType) {
    case Counter:
      theIntCounterBaseTable->markVarAsSampled(int(Counter), varIndex, thrPos);
            break;
    case WallTimer:
      theWallTimerBaseTable->markVarAsSampled(int(WallTimer), varIndex,thrPos);
      break;
    case ProcTimer:
      theProcTimerBaseTable->markVarAsSampled(int(ProcTimer), varIndex,thrPos);
      break;
    default:
      assert(0);
  }  
}

void superTable::markVarAsNotSampled(inst_var_type varType,
				     inst_var_index varIndex,
				     unsigned thrPos) const
{
  switch (varType) {
    case Counter:
      theIntCounterBaseTable->markVarAsNotSampled(int(Counter), varIndex, 
						  thrPos);
      break;
    case WallTimer:
      theWallTimerBaseTable->markVarAsNotSampled(int(WallTimer), varIndex, 
						 thrPos);

      break;
    case ProcTimer:
      theProcTimerBaseTable->markVarAsNotSampled(int(ProcTimer), varIndex,
						 thrPos);
      break;
    default:
      assert(0);
  }  
}

void *superTable::index2LocalAddr(inst_var_type varType,
				  inst_var_index varIndex,
				  unsigned thrPos) const
{
  void *ptr = NULL;
  switch (varType) {
    case Counter:
      ptr = theIntCounterBaseTable->index2LocalAddr(int(Counter), varIndex, 
						    thrPos);
      break;
    case WallTimer:
      ptr = theWallTimerBaseTable->index2LocalAddr(int(WallTimer), varIndex, 
						   thrPos);

      break;
    case ProcTimer:
      ptr = theProcTimerBaseTable->index2LocalAddr(int(ProcTimer), varIndex, 
						   thrPos);
      break;
    default:
      assert(0);
  }  
  return ptr;
}

void *superTable::index2InferiorAddr(inst_var_type varType, 
				     inst_var_index varIndex,
				     unsigned thrPos) const
{
  void *ptr = NULL;
  switch (varType) {
    case Counter:
      ptr = theIntCounterBaseTable->index2InferiorAddr(int(Counter), varIndex,
						       thrPos);
      break;
    case WallTimer:
      ptr = theWallTimerBaseTable->index2InferiorAddr(int(WallTimer), varIndex,
						      thrPos);
      break;
    case ProcTimer:
      ptr = theProcTimerBaseTable->index2InferiorAddr(int(ProcTimer), varIndex,
						      thrPos);
      break;
    default:
      assert(0);
  }  
  return ptr;
}

void *superTable::getHouseKeeping(inst_var_type varType, 
				  inst_var_index varIndex,
				  unsigned thrPos)
{
  void *ptr = NULL;
  switch (varType) {
    case 0:
      // intCounter
      ptr = (void*) theIntCounterBaseTable->getHouseKeeping(int(Counter),
							    varIndex, thrPos);
      break;
    case 1:
      // wallTimer
      ptr = (void*) theWallTimerBaseTable->getHouseKeeping(int(WallTimer),
							   varIndex, thrPos);
      break;
    case 2:
      // procTimer
      ptr = (void*) theProcTimerBaseTable->getHouseKeeping(int(ProcTimer),
							   varIndex, thrPos);
      break;
    default:
      assert(0);
  }  
  return ptr;
}

void superTable::makePendingFree(inst_var_type varType, 
				 inst_var_index varIndex,
				 unsigned thrPos,
				 const vector<Address> &trampsUsing)
{
  switch (varType) {
    case Counter:
      theIntCounterBaseTable->makePendingFree(int(Counter), varIndex, thrPos,
					      trampsUsing);

      break;
    case WallTimer:
      theWallTimerBaseTable->makePendingFree(int(WallTimer), varIndex, thrPos,
					     trampsUsing);
      break;
    case ProcTimer:
      theProcTimerBaseTable->makePendingFree(int(ProcTimer), varIndex, thrPos,
					     trampsUsing);
      break;
    default:
      assert(0);
  }  
}

bool superTable::doMajorSample()
{
  bool ok1, ok2, ok3;
  ok1 = theIntCounterBaseTable->doMajorSample();
  ok2 = theWallTimerBaseTable->doMajorSample();
  ok3 = theProcTimerBaseTable->doMajorSample();
  return (ok1 && ok2 && ok3);
}

bool superTable::doMinorSample()
{
  bool ok1, ok2, ok3;
  ok1 = theIntCounterBaseTable->doMinorSample();
  ok2 = theWallTimerBaseTable->doMinorSample();
  ok3 = theProcTimerBaseTable->doMinorSample();
  return (ok1 && ok2 && ok3);
}

void superTable::
initializeHKAfterForkIntCounter(unsigned level,
				unsigned varIndex,
				const intCounterHK &iHouseKeepingValue)
{
  theIntCounterBaseTable->initializeHKAfterFork(level,
						varIndex,
						iHouseKeepingValue);
}

void superTable::
initializeHKAfterForkWallTimer(unsigned level,
			       inst_var_index varIndex,
			       const wallTimerHK &iHouseKeepingValue)
{
  theWallTimerBaseTable->initializeHKAfterFork(level,
					       varIndex,
					       iHouseKeepingValue);
}

void superTable::
initializeHKAfterForkProcTimer(unsigned level,
			       inst_var_index varIndex,
			       const processTimerHK &iHouseKeepingValue)
{
  theProcTimerBaseTable->initializeHKAfterFork(level,
					       varIndex,
					       iHouseKeepingValue);
}

void superTable::handleExec()
{
  theIntCounterBaseTable->handleExec();
  theWallTimerBaseTable->handleExec();
  theProcTimerBaseTable->handleExec();
}

void superTable::forkHasCompleted()
{
  theIntCounterBaseTable->forkHasCompleted();
  theWallTimerBaseTable->forkHasCompleted();
  theProcTimerBaseTable->forkHasCompleted();
}

unsigned superTable::getCurrentNumberOfThreads()
{
  return numberOfCurrentThreads;
}

bool superTable::increaseMaxNumberOfThreads()
{
  assert(inferiorProcess->is_multithreaded());
  unsigned columnsToAdd;
  if (2*currMaxNumberOfThreads < maxNumberOfThreads) {
#if defined(MT_THREAD)
    inferiorProcess->threadMap->addToFreeList(currMaxNumberOfThreads,
					      currMaxNumberOfThreads);
#endif
    columnsToAdd = currMaxNumberOfThreads;
    currMaxNumberOfThreads = 2*currMaxNumberOfThreads;
  } else if (currMaxNumberOfThreads < maxNumberOfThreads) {
#if defined(MT_THREAD)
    inferiorProcess->threadMap->addToFreeList(currMaxNumberOfThreads,
			      maxNumberOfThreads - currMaxNumberOfThreads);
#endif
    columnsToAdd = maxNumberOfThreads-currMaxNumberOfThreads;
    currMaxNumberOfThreads = maxNumberOfThreads;
  } else {
    // we run out of space - naim
    return(false);
  }
  unsigned from, to;
  from = currMaxNumberOfThreads-columnsToAdd;
  to = currMaxNumberOfThreads;
  theIntCounterBaseTable->addColumns(from, to);
  theWallTimerBaseTable->addColumns(from, to);
  theProcTimerBaseTable->addColumns(from, to);  
  return(true);
}

void superTable::addThread(pdThread *thr)
{
  assert(inferiorProcess->is_multithreaded());
  unsigned pos, pd_pos;
  assert(thr);
  pos = thr->get_pos();
  pd_pos = thr->get_pd_pos();
  numberOfCurrentThreads++;
  inferiorProcess->numOfCurrentThreads_is++;
#if defined(MT_THREAD)
  for (unsigned i=0;i<inferiorProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = inferiorProcess->allMIComponentsWithThreads[i];
    if (mi)  
      mi->addThread(thr); 
  }
#endif
  // Add the pointers after setting up the instrumentation -- otherwise the
  // thread will start using the counters/timers before they've been cleared.
  theIntCounterBaseTable->addThread(pos, pd_pos);
  theWallTimerBaseTable->addThread(pos, pd_pos);
  theProcTimerBaseTable->addThread(pos, pd_pos);  
}

void superTable::deleteThread(pdThread *thr)
{
  assert(inferiorProcess->is_multithreaded());
  unsigned pos, pd_pos;
  assert(thr);
  pos = thr->get_pos();
  pd_pos = thr->get_pd_pos();
  numberOfCurrentThreads--;
  inferiorProcess->numOfCurrentThreads_is--;
  theIntCounterBaseTable->deleteThread(pos, pd_pos);
  theWallTimerBaseTable->deleteThread(pos, pd_pos);
  theProcTimerBaseTable->deleteThread(pos, pd_pos);  
#if defined(MT_THREAD)
  for (unsigned i=0;i<inferiorProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = inferiorProcess->allMIComponentsWithThreads[i];
    if (mi) {
      mi->deleteThread(thr);
    }
  }
#endif
}

