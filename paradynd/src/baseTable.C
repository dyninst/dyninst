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

// $Id: baseTable.C,v 1.12 2002/04/23 18:58:43 schendel Exp $
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
  addRows(level, nRows, true);
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
   assert(numberOfRows == parentSuperTable->superVectorBuf.size());
   for (unsigned i=0; i<numberOfRows; i++) {
      superVector<HK,RAW> *newSuperVec = new superVector<HK,RAW>(
	    parentSuperTable->superVectorBuf[i],inferiorProcess, subHeapIndex);
      superVectorBuf.push_back(newSuperVec);
					      
      levelMap.push_back(parentSuperTable->levelMap[i]);
   }
}

template <class HK, class RAW>

void baseTable<HK, RAW>::addRows(unsigned level, unsigned nRows,
				 bool calledFromBaseTableConst)
{
  // we assume that it is valid to add nRows, i.e. maxNumberOfLevels 
  // in the superTable class is greater than or equal to level+nRows.
  for (unsigned i=0; i<nRows; i++) {
    numberOfRows++;
    superVector<HK, RAW> *newSuperVec = 
       new superVector<HK, RAW>(inferiorProcess, heapNumElems, level+i,
				subHeapIndex, numberOfColumns,
				calledFromBaseTableConst);
    superVectorBuf.push_back(newSuperVec);
    levelMap.push_back(level+i);
  } 
}

template <class HK, class RAW>
baseTable<HK, RAW>::~baseTable() 
{
  for (unsigned i=0; i<superVectorBuf.size(); i++) {
    delete superVectorBuf[i];
  }
}

template <class HK, class RAW>
inst_var_index baseTable<HK, RAW>::allocateForInstVar(unsigned level)
{
  assert(superVectorBuf.size() == levelMap.size());
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  return superVectorBuf[i]->allocateForInstVar();
}

template <class HK, class RAW>
void baseTable<HK, RAW>::createThrInstVar(unsigned level, 
					  inst_var_index varIndex,
					  unsigned thrPos, const RAW &iValue,
					  const HK &iHKValue) {
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  superVectorBuf[i]->createThrInstVar(varIndex, thrPos, iValue, iHKValue);
}

template <class HK, class RAW>
    void baseTable<HK, RAW>::markVarAsSampled(unsigned level,
					      inst_var_index varIndex,
					      unsigned thrPos) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  superVectorBuf[i]->markVarAsSampled(varIndex, thrPos);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::markVarAsNotSampled(unsigned level,
					     inst_var_index varIndex,
					     unsigned threadPos) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  superVectorBuf[i]->markVarAsNotSampled(varIndex, threadPos);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::setBaseAddrInApplic(RAW *addr)
{
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     assert(superVectorBuf[i]);
     superVectorBuf[i]->setBaseAddrInApplic(addr, levelMap[i]);
   }
}

template <class HK, class RAW>
bool baseTable<HK, RAW>::doMajorSample()
{
   bool ok=true;
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     ok = ok && superVectorBuf[i]->doMajorSample();
   }
   return ok;
}

template <class HK, class RAW>
bool baseTable<HK, RAW>::doMinorSample()
{
   bool ok=true;
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     ok = ok && superVectorBuf[i]->doMinorSample();
   }
   return ok;
}

template <class HK, class RAW>
RAW *baseTable<HK, RAW>::index2LocalAddr(unsigned level,
					 inst_var_index varIndex,
					 unsigned threadPos) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  return superVectorBuf[i]->index2LocalAddr(varIndex, threadPos);
}

template <class HK, class RAW>
RAW *baseTable<HK, RAW>::index2InferiorAddr(unsigned level,
					    inst_var_index varIndex,
					    unsigned threadPos) const
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  return superVectorBuf[i]->index2InferiorAddr(varIndex, threadPos);
}

template <class HK, class RAW>
HK *baseTable<HK, RAW>::getHouseKeeping(unsigned level,
					inst_var_index varIndex,
					unsigned threadPos)
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());

  return superVectorBuf[i]->getHouseKeeping(varIndex, threadPos);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::initializeHKAfterFork(unsigned level, 
					       inst_var_index varIndex,
					       const HK &iHouseKeepingValue)
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  superVectorBuf[i]->initializeHKAfterFork(varIndex, iHouseKeepingValue);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::makePendingFree(unsigned level,
					 inst_var_index varIndex,
					 unsigned threadPos, 
					 const vector<Address> &trampsUsing)
{
  unsigned i;
  for (i=0; i<levelMap.size(); i++) {
    if (levelMap[i] == level) break;
  }
  assert(i<levelMap.size());
  superVectorBuf[i]->makePendingFree(varIndex, threadPos, trampsUsing);
}

template <class HK, class RAW>
void baseTable<HK, RAW>::handleExec()
{
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     superVectorBuf[i]->handleExec();
   }
}


template <class HK, class RAW>
void baseTable<HK, RAW>::forkHasCompleted()
{
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     superVectorBuf[i]->forkHasCompleted();
   }
}

template <class HK, class RAW>
void baseTable<HK, RAW>::addColumns(unsigned from, unsigned to)
{
   numberOfColumns += to-from;
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     superVectorBuf[i]->addColumns(from, to, subHeapIndex, levelMap[i]);
   }
}

template <class HK, class RAW>
void baseTable<HK, RAW>::addThread(unsigned pos, unsigned pd_pos)
{
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     superVectorBuf[i]->addThread(pos, pd_pos, subHeapIndex, levelMap[i]);
   }
}

template <class HK, class RAW>
void baseTable<HK, RAW>::deleteThread(unsigned pos, unsigned pd_pos)
{
   for (unsigned i=0; i<superVectorBuf.size(); i++) {
     superVectorBuf[i]->deleteThread(pos, pd_pos, levelMap[i]);
   }
}

