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

// $Id: varTable.C,v 1.8 2002/12/20 07:50:07 jaw Exp $
// The superTable class consists of an array of superVectors

#include <sys/types.h>
#include "common/h/headers.h"
#include "common/h/Types.h"
#include "rtinst/h/rtinst.h" // for time64
#include "paradynd/src/varTable.h"
#include "paradynd/src/varInstance.h"
#include "paradynd/src/varInstanceHKs.h"
#include "paradynd/src/variableMgr.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_process.h"

#include "papiMgr.h"

template <class HK>
varTable<HK>::varTable(const varTable<HK> &parent, variableMgr &vMgr) :
  baseVarTable(), varMgr(vMgr), freeIndexes(parent.freeIndexes)  
{
  // Copy over all variables
  for (unsigned i=0; i<parent.varInstanceBuf.size(); i++) {
    baseVarInstance *varInst = NULL;
    if(parent.varInstanceBuf[i] != NULL) {  // can be holes in varInstanceBuf
      varInst = new varInstance<HK>(*dynamic_cast<varInstance<HK>*>(
					     parent.varInstanceBuf[i]),
			    varMgr.getShmMgr(), varMgr.getApplicProcess());
    }
    varInstanceBuf.push_back(varInst);
  }
}

template <class HK>
varTable<HK>::~varTable() 
{
  for (unsigned iter=0; iter < varInstanceBuf.size(); iter++)
    delete varInstanceBuf[iter];
}

template <class HK>
inst_var_index varTable<HK>::getFreeIndex() {
  inst_var_index indexToUse;
  unsigned numFreeIndexes = freeIndexes.size();
  if(numFreeIndexes > 0) {
    indexToUse = freeIndexes[numFreeIndexes-1];
    freeIndexes.pop_back();  //pop off the last element
  } else {
    indexToUse = varInstanceBuf.size();
    varInstanceBuf.grow(indexToUse+1);
  }
  return indexToUse;
}

template <class HK>
inst_var_index varTable<HK>::allocateVar(HwEvent* hw)
{
  // We should check to see if there is a "free" varInstance, and reuse
  // it. For now, we monotonically increase (proof of concept and all that)

  inst_var_index new_index = getFreeIndex();
  // HK tells the varInstance how much memory to allocate.
 
  baseVarInstance *newVar = new varInstance<HK>(varMgr, HK::initValue, hw);
  varInstanceBuf[new_index] = newVar;
  return new_index;
}

template <class HK>
void varTable<HK>::markVarAsSampled(inst_var_index varIndex, unsigned thrPos,
				    threadMetFocusNode_Val *thrNval) 
{
  assert(varInstanceBuf[varIndex] != NULL);
  varInstanceBuf[varIndex]->markVarAsSampled(thrPos, thrNval);
}

template <class HK>
void varTable<HK>::markVarAsNotSampled(inst_var_index varIndex,
				       unsigned thrPos) 
{
  assert(varInstanceBuf[varIndex] != NULL);
  varInstanceBuf[varIndex]->markVarAsNotSampled(thrPos);
}

template <class HK>
void *varTable<HK>::shmVarDaemonAddr(inst_var_index varIndex) const 
{
  assert(varInstanceBuf[varIndex] != NULL);
  void *addr = varInstanceBuf[varIndex]->getBaseAddressInDaemon();
  return addr;
}

template <class HK>
void *varTable<HK>::shmVarApplicAddr(inst_var_index varIndex) const 
{
  assert(varInstanceBuf[varIndex] != NULL);
  void *addr = varInstanceBuf[varIndex]->getBaseAddressInApplic();
  return addr;
}


// Mark as deleted, clean up once we are sure that the
// process isn't using the counter any more.
template <class HK>
void varTable<HK>::free(inst_var_index varIndex)
{
  assert(varInstanceBuf[varIndex] != NULL);
  delete varInstanceBuf[varIndex];
  varInstanceBuf[varIndex] = NULL;
  freeIndexes.push_back(varIndex);
}

template <class HK>
bool varTable<HK>::doMajorSample() {
  bool entirelySuccessful = true;
  unsigned numVars = varInstanceBuf.size();
  for(unsigned iter=0; iter<numVars; iter++) {
    if(varInstanceBuf[iter] == NULL) continue;
    entirelySuccessful &= varInstanceBuf[iter]->doMajorSample();
  }

  return entirelySuccessful;
}

template <class HK>
bool varTable<HK>::doMinorSample() {
  bool successful = true;

  unsigned numVars = varInstanceBuf.size();  
  for(unsigned iter=0; iter<numVars; iter++) {
    if(varInstanceBuf[iter] == NULL) continue;
    successful &= varInstanceBuf[iter]->doMinorSample();
  }

  return successful;
}

template <class HK>
void varTable<HK>::handleExec()
{
  cerr << "Not implemented yet" << endl;
}


template <class HK>
void varTable<HK>::forkHasCompleted()
{
  cerr << "Not implemented yet" << endl;
}


template <class HK>
void varTable<HK>::deleteThread(unsigned thrPos) {
  for (unsigned iter=0; iter < varInstanceBuf.size(); iter++) {
    if(varInstanceBuf[iter] == NULL) continue;
    varInstanceBuf[iter]->deleteThread(thrPos);  
  }
}

template <class HK>
void varTable<HK>::initializeVarsAfterFork() {
}

template <>
void varTable<intCounterHK>::initializeVarsAfterFork() {
  // no initialization required for counters
}

template <>
void varTable<wallTimerHK>::initializeVarsAfterFork() {
  rawTime64 curWallTime = 0;
  if(varInstanceBuf.size() > 0)
    curWallTime = getRawWallTime();

  for(unsigned iter=0; iter<varInstanceBuf.size(); iter++) {
    if(varInstanceBuf[iter] == NULL) continue;
    varInstanceBuf[iter]->initializeVarsAfterFork(curWallTime);
  }

}

template <>
void varTable<processTimerHK>::initializeVarsAfterFork() {
  rawTime64 curProcTime = 0;
  if(varInstanceBuf.size() > 0)
    curProcTime = varMgr.getApplicProcess()->getRawCpuTime(0);

  for(unsigned iter=0; iter<varInstanceBuf.size(); iter++) {
    if(varInstanceBuf[iter] == NULL) continue;
    varInstanceBuf[iter]->initializeVarsAfterFork(curProcTime);
  }
}




/*
template <class HK>
bool varTable<HK>::doMinorSample()
{
  // This is used to clean up after a doMajorSample, above
  
  bool completedSample = true;
  //pdvector<unsigned> *varsLeft = new pdvector();

  // Run it backwards so that deleting elements from the vector is easier.
  for (unsigned iter = 0; iter < varsNeedingSample->size(); iter++) {

    bool success = HK::perform(varInstanceBuf[varsNeedingSample[iter]]);
    if (!success) varsLeft->push_back(iter);
    completedSample &= success;
  }
  delete varsNeedingSample;
  varsNeedingSample = varsLeft;
  return completedSample;
}
*/
