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

// $Id: superTable.C,v 1.8 2002/02/05 18:33:21 schendel Exp $
// The superTable class consists of an array of baseTable elements (superVectors)
// and it represents the ThreadTable in paradynd. For more info, please look at
// the .h file for this class. 

#include <sys/types.h>
#include <limits.h>
#include "common/h/headers.h"
#include "paradynd/src/superTable.h"
#include "rtinst/h/rtinst.h" // for time64 and MAX_NUMBER_OF_THREADS
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"

superTable::superTable(process *proc,
		       unsigned maxNumberOfIntCounters,
		       unsigned maxNumberOfWallTimers,
#if defined(MT_THREAD)
		       unsigned maxNumberOfProcTimers,
		       unsigned i_currMaxNumberOfThreads) :
                       currMaxNumberOfThreads(i_currMaxNumberOfThreads),
#else
		       unsigned maxNumberOfProcTimers) : 
#endif
		       inferiorProcess(proc)
{
  // Note: there should only be *one* instance of this class

#if defined(MT_THREAD)
  maxNumberOfThreads = MAX_NUMBER_OF_THREADS;
  maxNumberOfLevels = MAX_NUMBER_OF_LEVELS;
#else
  maxNumberOfThreads = 1;
  maxNumberOfLevels = 3;
#endif
  numberOfCounterLevels = 1;
  numberOfWallTimerLevels = 1;
  numberOfProcTimerLevels = 1;
  numberOfCurrentLevels = numberOfCounterLevels + numberOfWallTimerLevels +
                          numberOfProcTimerLevels;
#if defined(MT_THREAD)
  proc->numOfCurrentLevels_is = numberOfCurrentLevels;
#endif
  		                                         
  unsigned nCol, nRow, nElems;
#if defined(MT_THREAD)
  nCol = maxNumberOfThreads;
  nRow = maxNumberOfLevels/numberOfCurrentLevels; 
  numberOfCurrentThreads = 0;
  inferiorProcess->numOfCurrentThreads_is = numberOfCurrentThreads;

  proc->threadMap = new hashTable(maxNumberOfThreads,currMaxNumberOfThreads,0);
  nElems = maxNumberOfIntCounters/(nCol*nRow);
  theIntCounterSuperTable = new baseTable<intCounterHK, intCounter>(inferiorProcess,currMaxNumberOfThreads,1,0,nElems,0);

  nElems = maxNumberOfWallTimers/(nCol*nRow);
  theWallTimerSuperTable  = new baseTable<wallTimerHK, tTimer>(inferiorProcess,currMaxNumberOfThreads,1,1,nElems,1);

  nElems = maxNumberOfProcTimers/(nCol*nRow);
  theProcTimerSuperTable  = new baseTable<processTimerHK, tTimer>(inferiorProcess,currMaxNumberOfThreads,1,2,nElems,2);
#else
  nCol = 1;
  nRow = 1;
  nElems = (maxNumberOfIntCounters/nCol)/(maxNumberOfLevels/3);
  theIntCounterSuperTable = new baseTable<intCounterHK, intCounter>(inferiorProcess,nCol,nRow,0,nElems,0);
  nElems = (maxNumberOfWallTimers/nCol)/(maxNumberOfLevels/3);
  theWallTimerSuperTable  = new baseTable<wallTimerHK, tTimer>(inferiorProcess,nCol,nRow,1,nElems,1);
  nElems = (maxNumberOfProcTimers/nCol)/(maxNumberOfLevels/3);
  theProcTimerSuperTable  = new baseTable<processTimerHK, tTimer>(inferiorProcess,nCol,nRow,2,nElems,2);
#endif
}

superTable::superTable(const superTable &parentSuperTable, process *proc) :
		       maxNumberOfThreads(parentSuperTable.maxNumberOfThreads),
		       maxNumberOfLevels(parentSuperTable.maxNumberOfLevels),
		       numberOfCurrentLevels(parentSuperTable.numberOfCurrentLevels),
		       numberOfCounterLevels(parentSuperTable.numberOfCounterLevels),
		       numberOfWallTimerLevels(parentSuperTable.numberOfWallTimerLevels),
		       numberOfProcTimerLevels(parentSuperTable.numberOfProcTimerLevels),
#if defined(MT_THREAD)
		       numberOfCurrentThreads(parentSuperTable.numberOfCurrentThreads),
		       currMaxNumberOfThreads(parentSuperTable.currMaxNumberOfThreads),
#endif
		       inferiorProcess(proc)
{
#if defined(MT_THREAD)
  proc->threadMap = new hashTable(parentSuperTable.inferiorProcess->threadMap);
#endif
  theIntCounterSuperTable = new baseTable<intCounterHK, intCounter>(parentSuperTable.theIntCounterSuperTable,inferiorProcess);
  theWallTimerSuperTable  = new baseTable<wallTimerHK, tTimer>(parentSuperTable.theWallTimerSuperTable,inferiorProcess);
  theProcTimerSuperTable  = new baseTable<processTimerHK, tTimer>(parentSuperTable.theProcTimerSuperTable,inferiorProcess);
}

superTable::~superTable() 
{
  delete theIntCounterSuperTable;
  delete theWallTimerSuperTable;
  delete theProcTimerSuperTable;
}

#if defined(MT_THREAD)
bool superTable::allocIntCounter(unsigned thr_pos, const intCounter &iRawValue,
#else
bool superTable::allocIntCounter(const intCounter &iRawValue,
#endif
				 const intCounterHK &iHouseKeepingValue,
				 unsigned &allocatedIndex,
				 unsigned &allocatedLevel,
				 bool doNotSample)
{
#if defined(MT_THREAD)
#if defined(TEST_DEL_DEBUG)
  if (allocatedIndex != UINT_MAX && allocatedLevel!=UINT_MAX) {
    sprintf(errorLine,"=====> in allocIntCounter, re-using index=%d, level=%d",allocatedIndex,allocatedLevel);
  } else {
    sprintf(errorLine,"=====> in allocIntCounter, not-reusing index and level");
  }
  cerr << errorLine << endl;;
#endif

  if (!theIntCounterSuperTable->alloc(thr_pos, iRawValue,iHouseKeepingValue,
				      allocatedIndex,
				      allocatedLevel,doNotSample))
#else
  if (!theIntCounterSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
				      allocatedLevel,doNotSample))
#endif
  {
    if (numberOfCurrentLevels+1 <= maxNumberOfLevels) numberOfCurrentLevels++;
    else return(false);
    numberOfCounterLevels++;
#if defined(MT_THREAD)
    inferiorProcess->numOfCurrentLevels_is++;
    theIntCounterSuperTable->addRows(numberOfCurrentLevels-1,1);
    bool ret = (theIntCounterSuperTable->alloc(thr_pos,iRawValue,iHouseKeepingValue,
					  allocatedIndex,
				          allocatedLevel,doNotSample));
    return ret ;
#else
    theIntCounterSuperTable->addRows(numberOfCurrentLevels,1);
    return(theIntCounterSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
                                          allocatedLevel,doNotSample));
#endif
  }
  else  return(true);
}

#if defined(MT_THREAD)
bool superTable::allocWallTimer(unsigned thr_pos, const tTimer &iRawValue,
#else
bool superTable::allocWallTimer(const tTimer &iRawValue,
#endif
				const wallTimerHK &iHouseKeepingValue,
				unsigned &allocatedIndex,
				unsigned &allocatedLevel)
{
#if defined(MT_THREAD)
#if defined(TEST_DEL_DEBUG)
  if (allocatedIndex != UINT_MAX && allocatedLevel!=UINT_MAX) {
    sprintf(errorLine,"=====> in allocWallTimer, re-using index=%d, level=%d",allocatedIndex,allocatedLevel);
  } else {
    sprintf(errorLine,"=====> in allocWallTimer, not-reusing index and level");
  }
  cerr << errorLine <<endl;;
#endif
  if (!theWallTimerSuperTable->alloc(thr_pos,iRawValue,iHouseKeepingValue, allocatedIndex,
#else
  if (!theWallTimerSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
#endif
				     allocatedLevel))
  {
    if (numberOfCurrentLevels+1 <= maxNumberOfLevels) numberOfCurrentLevels++;
    else return(false);
    numberOfWallTimerLevels++;
    theWallTimerSuperTable->addRows(numberOfCurrentLevels,1);
#if defined(MT_THREAD)
    inferiorProcess->numOfCurrentLevels_is++;
    return(theWallTimerSuperTable->alloc(thr_pos,iRawValue,iHouseKeepingValue, allocatedIndex,
#else
    return(theWallTimerSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
#endif
				         allocatedLevel));
  }
  else return(true);
}


#if defined(MT_THREAD)
bool superTable::allocProcTimer(unsigned thr_pos, const tTimer &iRawValue,
#else
bool superTable::allocProcTimer(const tTimer &iRawValue,
#endif
				const processTimerHK &iHouseKeepingValue,
				unsigned &allocatedIndex,
				unsigned &allocatedLevel)
{
#if defined(MT_THREAD)
#if defined(TEST_DEL_DEBUG)
  if (allocatedIndex != UINT_MAX && allocatedLevel!=UINT_MAX) {
    sprintf(errorLine,"=====> in allocProcTimer, re-using index=%d, level=%d",allocatedIndex,allocatedLevel);
  } else {
    sprintf(errorLine,"=====> in allocProcTimer, not-reusing index and level");
  }
  cerr << errorLine << endl;
#endif
  if (!theProcTimerSuperTable->alloc(thr_pos,iRawValue,iHouseKeepingValue, allocatedIndex,
#else
  if (!theProcTimerSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
#endif
				     allocatedLevel))
  {
    if (numberOfCurrentLevels+1 <= maxNumberOfLevels) numberOfCurrentLevels++;
    else return(false);
    numberOfProcTimerLevels++;
    theProcTimerSuperTable->addRows(numberOfCurrentLevels,1);
#if defined(MT_THREAD)
    inferiorProcess->numOfCurrentLevels_is++;
    return(theProcTimerSuperTable->alloc(thr_pos,iRawValue,iHouseKeepingValue,allocatedIndex,
#else
    return(theProcTimerSuperTable->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,
#endif
					 allocatedLevel));
  }
  else return(true);
}

void superTable::setBaseAddrInApplic(unsigned type, void *addr)
{
  switch (type) {
    case 0:
      // intCounter
      theIntCounterSuperTable->setBaseAddrInApplic((intCounter *)addr);
      break;
    case 1:
      // wallTimer
      theWallTimerSuperTable->setBaseAddrInApplic((tTimer *)addr);
      break;
    case 2:
      // procTimer
      theProcTimerSuperTable->setBaseAddrInApplic((tTimer *)addr);
      break;
    default:
      assert(0);
  }
}

void *superTable::index2LocalAddr(unsigned type, 
				  unsigned position,
				  unsigned allocatedIndex,
				  unsigned allocatedLevel) const
{
  switch (type) {
    case 0:
      // intCounter
      return(theIntCounterSuperTable->index2LocalAddr(position,allocatedIndex,allocatedLevel));
      break;
    case 1:
      // wallTimer
      return(theWallTimerSuperTable->index2LocalAddr(position,allocatedIndex,allocatedLevel));
      break;
    case 2:
      // procTimer
      return(theProcTimerSuperTable->index2LocalAddr(position,allocatedIndex,allocatedLevel));
      break;
    default:
      assert(0);
  }  
  return ((void*)0);
}

void *superTable::index2InferiorAddr(unsigned type, 
				     unsigned position,
				     unsigned allocatedIndex,
				     unsigned allocatedLevel) const
{
  switch (type) {
    case 0:
      // intCounter
      return(theIntCounterSuperTable->index2InferiorAddr(position,allocatedIndex,allocatedLevel));
      break;
    case 1:
      // wallTimer
      return(theWallTimerSuperTable->index2InferiorAddr(position,allocatedIndex,allocatedLevel));
      break;
    case 2:
      // procTimer
      return(theProcTimerSuperTable->index2InferiorAddr(position,allocatedIndex,allocatedLevel));
      break;
    default:
      assert(0);
  }  
  return ((void*)0);
}

#if defined(MT_THREAD)
void superTable::makePendingFree(pdThread *thr, unsigned type,
#else
void superTable::makePendingFree(unsigned type,
#endif
				 unsigned allocatedIndex,
				 unsigned allocatedLevel, 
				 const vector<Address> &trampsUsing)
{
#if defined(MT_THREAD)
  assert(thr);
  unsigned pd_pos = thr->get_pd_pos();
#endif
  switch (type) {
    case 0:
      // intCounter
#if defined(MT_THREAD) //are these baseTable
      theIntCounterSuperTable->makePendingFree(pd_pos,allocatedIndex,allocatedLevel,trampsUsing);
#else
      theIntCounterSuperTable->makePendingFree(allocatedIndex,allocatedLevel,trampsUsing);
#endif
      break;
    case 1:
      // wallTimer
#if defined(MT_THREAD)
      theWallTimerSuperTable->makePendingFree(pd_pos,allocatedIndex,allocatedLevel,trampsUsing);
#else
      theWallTimerSuperTable->makePendingFree(allocatedIndex,allocatedLevel,trampsUsing);
#endif
      break;
    case 2:
      // procTimer
#if defined(MT_THREAD)
      theProcTimerSuperTable->makePendingFree(pd_pos,allocatedIndex,allocatedLevel,trampsUsing);
#else
      theProcTimerSuperTable->makePendingFree(allocatedIndex,allocatedLevel,trampsUsing);
#endif
      break;
    default:
      assert(0);
  }  
}

bool superTable::doMajorSample()
{
  bool ok1, ok2, ok3;
  ok1 = theIntCounterSuperTable->doMajorSample();
  ok2 = theWallTimerSuperTable->doMajorSample();
  ok3 = theProcTimerSuperTable->doMajorSample();
  return(ok1 && ok2 && ok3);
}

bool superTable::doMinorSample()
{
  bool ok1, ok2, ok3;
  ok1 = theIntCounterSuperTable->doMinorSample();
  ok2 = theWallTimerSuperTable->doMinorSample();
  ok3 = theProcTimerSuperTable->doMinorSample();
  return(ok1 && ok2 && ok3);
}

void superTable::
initializeHKAfterForkIntCounter(unsigned allocatedIndex,
				unsigned allocatedLevel,
				const intCounterHK &iHouseKeepingValue)
{
  theIntCounterSuperTable->initializeHKAfterFork(allocatedIndex,
						 allocatedLevel,
						 iHouseKeepingValue);
}

void superTable::
initializeHKAfterForkWallTimer(unsigned allocatedIndex,
			       unsigned allocatedLevel,
			       const wallTimerHK &iHouseKeepingValue)
{
  theWallTimerSuperTable->initializeHKAfterFork(allocatedIndex,
					        allocatedLevel,
					        iHouseKeepingValue);
}

void superTable::
initializeHKAfterForkProcTimer(unsigned allocatedIndex,
			       unsigned allocatedLevel,
			       const processTimerHK &iHouseKeepingValue)
{
  theProcTimerSuperTable->initializeHKAfterFork(allocatedIndex,
					        allocatedLevel,
					        iHouseKeepingValue);
}

void superTable::handleExec()
{
  theIntCounterSuperTable->handleExec();
  theWallTimerSuperTable->handleExec();
  theProcTimerSuperTable->handleExec();
}

void superTable::forkHasCompleted()
{
  theIntCounterSuperTable->forkHasCompleted();
  theWallTimerSuperTable->forkHasCompleted();
  theProcTimerSuperTable->forkHasCompleted();
}

#if defined(MT_THREAD)
unsigned superTable::getCurrentNumberOfThreads()
{
  return(numberOfCurrentThreads);
}

bool superTable::increaseMaxNumberOfThreads()
{
  unsigned columnsToAdd;
  if (2*currMaxNumberOfThreads < maxNumberOfThreads) {
    inferiorProcess->threadMap->addToFreeList(currMaxNumberOfThreads,currMaxNumberOfThreads);
    columnsToAdd = currMaxNumberOfThreads;
    currMaxNumberOfThreads = 2*currMaxNumberOfThreads;
  } else if (currMaxNumberOfThreads < maxNumberOfThreads) {
    inferiorProcess->threadMap->addToFreeList(currMaxNumberOfThreads,maxNumberOfThreads-currMaxNumberOfThreads);
    columnsToAdd = maxNumberOfThreads-currMaxNumberOfThreads;
    currMaxNumberOfThreads = maxNumberOfThreads;
  } else {
    // we run out of space - naim
    return(false);
  }
  unsigned from, to;
  from = currMaxNumberOfThreads-columnsToAdd;
  to = currMaxNumberOfThreads;
  theIntCounterSuperTable->addColumns(from,to);
  theWallTimerSuperTable->addColumns(from,to);
  theProcTimerSuperTable->addColumns(from,to);  
  return(true);
}

void superTable::addThread(pdThread *thr)
{
  unsigned pos, pd_pos;
  assert(thr);
  pos = thr->get_pos();
  pd_pos = thr->get_pd_pos();
  numberOfCurrentThreads++;
  inferiorProcess->numOfCurrentThreads_is++;
  theIntCounterSuperTable->addThread(pos,pd_pos);
  theWallTimerSuperTable->addThread(pos,pd_pos);
  theProcTimerSuperTable->addThread(pos,pd_pos);  
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"=====> allMIComponentsWithThreads size=%d\n",inferiorProcess->allMIComponentsWithThreads.size());
  logLine(errorLine);
#endif
  for (unsigned i=0;i<inferiorProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = inferiorProcess->allMIComponentsWithThreads[i];
    if (mi)  
      mi->addThread(thr); 
  }
}

void superTable::deleteThread(pdThread *thr)
{
  unsigned pos, pd_pos;
  assert(thr);
  pos = thr->get_pos();
  pd_pos = thr->get_pd_pos();
  numberOfCurrentThreads--;
  inferiorProcess->numOfCurrentThreads_is--;
  theIntCounterSuperTable->deleteThread(pos,pd_pos);
  theWallTimerSuperTable->deleteThread(pos,pd_pos);
  theProcTimerSuperTable->deleteThread(pos,pd_pos);  
  for (unsigned i=0;i<inferiorProcess->allMIComponentsWithThreads.size();i++) {
    processMetFocusNode *mi = inferiorProcess->allMIComponentsWithThreads[i];
    if (mi) {
      mi->deleteThread(thr);
    }
  }
}
#endif
