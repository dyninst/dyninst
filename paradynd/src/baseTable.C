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

// $Id: baseTable.C,v 1.9 2002/04/05 19:39:04 schendel Exp $
// The superTable class consists of an array of superVectors

#include <sys/types.h>
#include "common/h/headers.h"
#include "common/h/Types.h"
#include "baseTable.h"
#include "rtinst/h/rtinst.h" // for time64

template <class HK, class RAW>
baseTable<HK, RAW>::baseTable(process *proc, unsigned nColumns, unsigned nRows,
			      unsigned level, unsigned iheapNumElems,
			      unsigned isubHeapIndex):
		    numberOfColumns(nColumns),
		    numberOfRows(0),
		    heapNumElems(iheapNumElems),
		    subHeapIndex(isubHeapIndex),
		    inferiorProcess(proc)
{
#if defined(MT_THREAD)
  addRows(level,nRows,true);
#else
  addRows(level,nRows);
#endif
}

template <class HK, class RAW>
baseTable<HK, RAW>::baseTable(const baseTable<HK,RAW> *parentSuperTable,
			      process *proc):
		    numberOfColumns(parentSuperTable->numberOfColumns),
		    numberOfRows(parentSuperTable->numberOfRows),
		    heapNumElems(parentSuperTable->heapNumElems),
		    subHeapIndex(parentSuperTable->subHeapIndex),
		    inferiorProcess(proc)
{
  assert(parentSuperTable != NULL);
  assert(numberOfRows == parentSuperTable->theBaseTable.size());
  for (unsigned i=0; i<numberOfRows; i++) {
    theBaseTable += new superVector<HK,RAW>(parentSuperTable->theBaseTable[i], inferiorProcess, subHeapIndex);
    levelMap += parentSuperTable->levelMap[i];
  }
}

template <class HK, class RAW>
#if defined(MT_THREAD)
void baseTable<HK, RAW>::addRows(unsigned level, unsigned nRows,
				 bool calledFromBaseTableConst)
#else
void baseTable<HK, RAW>::addRows(unsigned level, unsigned nRows)
#endif
{
  // we assume that it is valid to add nRows, i.e. maxNumberOfLevels 
  // in the superTable class is greater than or equal to level+nRows.
  for (unsigned i=0; i<nRows; i++) {
    numberOfRows++;
#if defined(MT_THREAD)
    theBaseTable += new superVector<HK, RAW>(inferiorProcess,heapNumElems,
					     level+i,
					     subHeapIndex,
					     numberOfColumns,
					     calledFromBaseTableConst);
#else
    theBaseTable += new superVector<HK, RAW>(inferiorProcess,heapNumElems,subHeapIndex,
					     numberOfColumns);

#endif
    levelMap += level+i;
  } 
}

template <class HK, class RAW>
baseTable<HK, RAW>::~baseTable() 
{
  for (unsigned i=0; i<theBaseTable.size(); i++) {
    delete theBaseTable[i];
  }
}

template <class HK, class RAW>
#if defined(MT_THREAD)
bool baseTable<HK, RAW>::alloc(unsigned thr_pos, const RAW &iRawValue,
#else
bool baseTable<HK, RAW>::alloc(const RAW &iRawValue,
#endif
			       const HK &iHouseKeepingValue,
			       unsigned &allocatedIndex,
			       unsigned &allocatedLevel,
			       bool doNotSample)
{
  assert(theBaseTable.size() > 0);
  assert(theBaseTable.size() == levelMap.size());
  
  // we try to find a superVector where to place the new data element.
  bool foundFreeLevel=false;
  for (unsigned i=0; i<theBaseTable.size(); i++) {
    assert(theBaseTable[i] != NULL);
#if defined(MT_THREAD)
    // if allocated!=UI32_MAX, we will re-use this position because we are
    // trying to enable a counter/timer that has been already allocated - naim
    if (allocatedLevel == UI32_MAX || allocatedIndex == UI32_MAX) {
      allocatedLevel=levelMap[i];    
      foundFreeLevel=theBaseTable[i]->alloc(thr_pos,iRawValue,iHouseKeepingValue,allocatedIndex,doNotSample);
    } else if (allocatedLevel == levelMap[i]) {
      foundFreeLevel=theBaseTable[i]->alloc(thr_pos,iRawValue,iHouseKeepingValue,allocatedIndex,doNotSample);
    }
#else
    allocatedLevel=levelMap[i];    
    foundFreeLevel=theBaseTable[i]->alloc(iRawValue,iHouseKeepingValue,allocatedIndex,doNotSample);
#endif
    if (foundFreeLevel) return(true);
  }
  // At this point, we don't have any free spot in the current allocated levels, so
  // we need to add a new one, but the superTable class has to request it.
  return(false);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::setBaseAddrInApplic(RAW *addr)
{
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     assert(theBaseTable[i]);
     theBaseTable[i]->setBaseAddrInApplic(addr,levelMap[i]);
   }
}


template <class HK, class RAW>
bool baseTable<HK, RAW>::doMajorSample()
{
   bool ok=true;
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     ok = ok && theBaseTable[i]->doMajorSample();
   }
   return(ok);
}

template <class HK, class RAW>
bool baseTable<HK, RAW>::doMinorSample()
{
   bool ok=true;
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     ok = ok && theBaseTable[i]->doMinorSample();
   }
   return(ok);
}

template <class HK, class RAW>
RAW *baseTable<HK, RAW>::index2LocalAddr(unsigned position,
					 unsigned allocatedIndex,
					 unsigned allocatedLevel) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == allocatedLevel) break;
  }
  assert(i<levelMap.size());
  return(theBaseTable[i]->index2LocalAddr(position,allocatedIndex));
}

template <class HK, class RAW>
RAW *baseTable<HK, RAW>::index2InferiorAddr(unsigned position,
					    unsigned allocatedIndex,
					    unsigned allocatedLevel) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == allocatedLevel) break;
  }
  assert(i<levelMap.size());
  return(theBaseTable[i]->index2InferiorAddr(position,allocatedIndex));
}

template <class HK, class RAW>
HK *baseTable<HK, RAW>::getHouseKeeping(unsigned position,
					unsigned allocatedIndex,
					unsigned allocatedLevel) const
{
  //cerr << "baseTable::getHouseKeeping, levelMap.size: " << levelMap.size()
  //   << ", allocatedLevel: " << allocatedLevel << "\n";
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == allocatedLevel) break;
  }
  assert(i<levelMap.size());

  return(theBaseTable[i]->getHouseKeeping(position,allocatedIndex));
}

template <class HK, class RAW>
void baseTable<HK, RAW>::initializeHKAfterFork(unsigned allocatedIndex, 
					       unsigned allocatedLevel,
					       const HK &iHouseKeepingValue)
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == allocatedLevel) break;
  }
  assert(i<levelMap.size());
  theBaseTable[i]->initializeHKAfterFork(allocatedIndex,iHouseKeepingValue);
}

template <class HK, class RAW>
#if defined(MT_THREAD)
void baseTable<HK, RAW>::makePendingFree(unsigned pd_pos,
					 unsigned allocatedIndex,
#else
void baseTable<HK, RAW>::makePendingFree(unsigned allocatedIndex,
#endif
					 unsigned allocatedLevel, 
					 const vector<Address> &trampsUsing)
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == allocatedLevel) break;
  }
  assert(i<levelMap.size());
#if defined(MT_THREAD) //are these superVector?
  theBaseTable[i]->makePendingFree(pd_pos,allocatedIndex,trampsUsing);
#else
  theBaseTable[i]->makePendingFree(allocatedIndex,trampsUsing);
#endif
}

template <class HK, class RAW>
void baseTable<HK, RAW>::handleExec()
{
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     theBaseTable[i]->handleExec();
   }
}


template <class HK, class RAW>
void baseTable<HK, RAW>::forkHasCompleted()
{
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     theBaseTable[i]->forkHasCompleted();
   }
}

#if defined(MT_THREAD)
template <class HK, class RAW>
void baseTable<HK, RAW>::addColumns(unsigned from, unsigned to)
{
   numberOfColumns += to-from;
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     theBaseTable[i]->addColumns(from,to,subHeapIndex,levelMap[i]);
   }
}

template <class HK, class RAW>
void baseTable<HK, RAW>::addThread(unsigned pos, unsigned pd_pos)
{
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     theBaseTable[i]->addThread(pos,pd_pos,subHeapIndex,levelMap[i]);
   }
}

template <class HK, class RAW>
void baseTable<HK, RAW>::deleteThread(unsigned pos, unsigned pd_pos)
{
   for (unsigned i=0; i<theBaseTable.size(); i++) {
     theBaseTable[i]->deleteThread(pos,pd_pos,levelMap[i]);
   }
}
#endif
