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


#include "paradynd/src/varInstance.h"
#include "paradynd/src/shmMgr.h"
#include "paradynd/src/pd_process.h"
 
template <class HK>
varInstance<HK>::varInstance(variableMgr &varMgr, const RAWTYPE &initValue_, HwEvent* hw)
  : numElems(varMgr.getMaxNumberOfThreads()),  hkBuf(1, NULL), 
    elementsToBeSampled(false), proc(varMgr.getApplicProcess()), 
    initValue(initValue_), theShmMgr(varMgr.getShmMgr()), hwEvent(hw)
{
  unsigned mem_amount = sizeof(RAWTYPE) * varMgr.getMaxNumberOfThreads();
  //  Address baseAddrInApp_;
  baseAddrInDaemon = (void*) theShmMgr.malloc(mem_amount);
  baseAddrInApplic = theShmMgr.getAddressInApplic(baseAddrInDaemon);
  //  baseAddrInApplic = (void*) baseAddrInApp_;
  
  for(unsigned i=0; i<numElems; i++) {
    RAWTYPE *curElem = static_cast<RAWTYPE*>( elementAddressInDaemon(i));
    (*curElem) = initValue;
  }
}

template <class HK>
varInstance<HK>::varInstance(const varInstance<HK> &par, shmMgr &sMgr, 
			     pd_process *p) : 
  numElems(par.numElems), baseAddrInApplic(par.baseAddrInApplic),
  // start with not sampling anything, let the sample requests be 
  // reinitiated, ... so leave hkBuf as empty
  elementsToBeSampled(false), proc(p), initValue(par.initValue), 
  theShmMgr(sMgr), hwEvent(par.hwEvent), permanentSamplingSet(), currentSamplingSet() {
   baseAddrInDaemon = theShmMgr.applicAddrToDaemonAddr(baseAddrInApplic);
}

template <class HK>
void varInstance<HK>::createHKifNotPresent(unsigned thrPos) {
   unsigned neededsize = thrPos + 1;
   unsigned oldsize = hkBuf.size();
   if(oldsize < neededsize) {
      unsigned newsize = oldsize;
      do {
	 newsize = (newsize + 1) * 4;  // eg. size: 1, 8, 36, 148, 596
      } while(newsize < neededsize);

      hkBuf.resize(newsize, true);
      // new vector data will be initialized to NULL, see call to constructor
   }
   if(hkBuf[thrPos] == NULL) {
      hkBuf[thrPos] = new HK();
      hkBuf[thrPos]->setHwEvent(hwEvent);
   }
}

template <class HK>
void varInstance<HK>::markVarAsSampled(unsigned thrPos, 
                                       threadMetFocusNode_Val *thrNval) {
  createHKifNotPresent(thrPos);
  if(hkBuf[thrPos]->getThrNodeVal() != NULL) 
     return;  // must already have been marked as sampled

  hkBuf[thrPos]->setThrClient(thrNval);

  permanentSamplingSet.push_back(thrPos);
  currentSamplingSet.push_back(thrPos);
  elementsToBeSampled = true;
}

template <class HK>
bool varInstance<HK>::removeFromSamplingSet(pdvector<unsigned> *set, 
                                            unsigned thrPosToRemove) {
  bool foundIt = false;
  pdvector<unsigned>::iterator itr = (*set).end();
  while(itr != (*set).begin()) {
     --itr;
     if(*itr == thrPosToRemove) {
        (*set).erase(itr);
        foundIt = true;
     }
  }
  return foundIt;
}

template <class HK>
void varInstance<HK>::markVarAsNotSampled(unsigned thrPos) {
  // Function removeFromSamplingSet will return true (ie. was deleted) except
  // in the case of sampling attempted to be turned off through the "unfork"
  // mechanish for forked processes.  This is because the sampling was never
  // turned on for the forked process, so it's already been deleted per se.
  if(removeFromSamplingSet(&permanentSamplingSet, thrPos)) {
    removeFromSamplingSet(&currentSamplingSet, thrPos);

    // if the sample is in the permanent sampling set (and now deleted), then
    // it's housekeeping object should also be defined
    hkBuf[thrPos]->setThrClient(NULL);
    delete hkBuf[thrPos];
    hkBuf[thrPos] = NULL;
  }
}

// returns true if all relevant elements successfully sampled
template <class HK>
bool varInstance<HK>::doMinorSample() {
  if(elementsToBeSampled == false)  return true;

  pdvector<unsigned>::iterator itr = currentSamplingSet.end();
  while(itr != currentSamplingSet.begin()) {
     --itr;
     // check that we don't run past the vector when iterating through it,
     // due to elements being removed (eg. when a thread exits)
     if(itr - currentSamplingSet.begin() >= (int)currentSamplingSet.size())
        continue;

     unsigned thrPos = *itr;
     void *voidVarAddr = elementAddressInDaemon(thrPos);
     RAWTYPE *shmVarAddr = static_cast<RAWTYPE*>( voidVarAddr);
     bool successful = hkBuf[thrPos]->perform(shmVarAddr, proc);
     if(successful) {
        currentSamplingSet.erase(itr);
     }
  }
  return (currentSamplingSet.size() == 0);
}

template <class HK>
varInstance<HK>::~varInstance() {
  theShmMgr.free((Address) baseAddrInDaemon);
}

template <class HK>
void varInstance<HK>::deleteThread(unsigned thrPos) {
   markVarAsNotSampled(thrPos);
   void *voidVarAddr = elementAddressInDaemon(thrPos);
   RAWTYPE *shmVarAddr = static_cast<RAWTYPE*>( voidVarAddr);
   (*shmVarAddr) = initValue;
}

template <class HK>
void varInstance<HK>::initializeVarsAfterFork(rawTime64 curRawTime) {
  for(unsigned i=0; i<numElems; i++) {
    RAWTYPE *curElem = static_cast<RAWTYPE*>( elementAddressInDaemon(i));
    HK::initializeAfterFork(curElem, curRawTime);
  }
}


