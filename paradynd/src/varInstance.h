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

#ifndef _VAR_INSTANCE_H_
#define _VAR_INSTANCE_H_


#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "paradynd/src/variableMgrTypes.h"

class threadMetFocusNode_Val;
class Frame;

class baseVarInstance {
 public:
  virtual ~baseVarInstance() { }
  virtual void allocateThreadVars(const vector<unsigned> &thrPosBuf) = 0;
  virtual void *elementAddressInDaemon(unsigned thrPos) = 0;
  virtual void *elementAddressInApplic(unsigned thrPos) = 0;
  virtual bool doMajorSample() = 0;
  virtual bool doMinorSample() = 0;
  virtual void markVarAsSampled(unsigned thrPos, 
				threadMetFocusNode_Val *thrNval) = 0;
  virtual void markVarAsNotSampled(unsigned thrPos) = 0;
  virtual void makePendingFree(unsigned thrPos, 
			       const vector<Address> &trampsUsing) = 0;
  virtual bool attemptToFree(const vector<Frame> &stackWalk) = 0;
  virtual void deleteThread(unsigned thrPos) = 0;
};

class variableMgr;
class process;
class shmMgr;


template <class HK>
class varInstance : public baseVarInstance {
 public:
  typedef typename HK::rawType RAWTYPE;
 private:
  unsigned numElems;
  void *baseAddrInDaemon;
  void *baseAddrInApplic;
  vector<HK*> hkBuf;
  bool elementsToBeSampled;  // true if currentSamplingSet > 0
  process *proc;
  RAWTYPE  initValue;
  shmMgr &theShmMgr;

  vector<unsigned> permanentSamplingSet;
  vector<unsigned> currentSamplingSet;
  vector<element_state> elemStates;
  void createHKifNotPresent(unsigned thrPos);
  void setElemState(unsigned thrPos, element_state st);
  element_state getElemState(unsigned thrPos) {
    if(thrPos+1 > elemStates.size()) {
      return elemFree;
    }
    return elemStates[thrPos];
  }
  bool removeFromSamplingSet(vector<unsigned> *set, unsigned thrPosToRemove);

 public:
  varInstance(variableMgr &varMgr, const RAWTYPE &initValue);
  void allocateThreadVars(const vector<unsigned> &thrPosBuf);
  void *elementAddressInDaemon(unsigned thrPos) {
    RAWTYPE *baseAddr = static_cast<RAWTYPE *>(baseAddrInDaemon);
    return static_cast<void*>(baseAddr + thrPos);  // ptr arith
  }
  void *elementAddressInApplic(unsigned thrPos) {
    RAWTYPE *baseAddr = static_cast<RAWTYPE *>(baseAddrInApplic);
    return static_cast<void*>(baseAddr + thrPos);  // ptr arith
  }
  bool doMajorSample() {
    if(permanentSamplingSet.size() == 0) return true;
    else elementsToBeSampled = true;

    currentSamplingSet = permanentSamplingSet;
    return doMinorSample();
  }
  bool doMinorSample();
  void markVarAsSampled(unsigned thrPos, threadMetFocusNode_Val *thrNval);
  void markVarAsNotSampled(unsigned thrPos);
  void makePendingFree(unsigned thrPos, const vector<Address> &trampsUsing);
  bool attemptToFree(const vector<Frame> &stackWalk);
  void deleteThread(unsigned thrPos);
};



#endif



