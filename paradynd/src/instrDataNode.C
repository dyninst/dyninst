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

#include "common/h/Types.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/pdThread.h"


extern pdDebug_ostream sampleVal_cerr;
extern pdDebug_ostream metric_cerr;


instrDataNode::instrDataNode(process *proc_, unsigned type,
			     inst_var_index var_index, bool arg_dontInsertData)
  : proc(proc_), varIndex(var_index), thrNodeClientSet(false), 
    dontInsertData_(arg_dontInsertData)
{ 
  switch (type) {
    case MDL_T_COUNTER:
      varType = Counter;
      break;
    case MDL_T_WALL_TIMER:
      varType = WallTimer;
      break;
    case MDL_T_PROC_TIMER:
      varType = ProcTimer;
      break;
    case MDL_T_NONE:
      // just to keep mdl apply allocate a dummy un-sampled counter.
      varType = Counter;
      break;
    default:
      assert(0);  break;
  }
}


// the disable method should be called before this destructor
instrDataNode::~instrDataNode() {
}

// obligatory definition of static member vrble:
int instrDataNode::counterId=0;
extern process *global_proc;

Address instrDataNode::getInferiorPtr() const {
  // counterPtr could be NULL if we are building AstNodes just to compute
  // the cost - naim 2/18/97
  // NOTE:
  // this routine will dissapear because we can't compute the address
  // of the counter/timer without knowing the thread id - naim 3/17/97
  //
  Address varAddr;
  if(dontInsertData_) {
    varAddr = 0;
  } else {
    variableMgr &varMgr = proc->getVariableMgr();
    // we assume there is only one thread
    varAddr = (Address)varMgr.shmVarApplicAddr(varType, varIndex);
  }
  return varAddr;
}

void instrDataNode::print() {
   cerr << "D:" << (void*)this << "\n";
}

void instrDataNode::startSampling(unsigned thrPos,
				  threadMetFocusNode_Val *thrClient) { 
  thrNodeClientSet = true;
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsSampled(varType, varIndex, thrPos, thrClient);
}

void instrDataNode::stopSampling(unsigned thrPos) {
  thrNodeClientSet = false;
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsNotSampled(varType, varIndex, thrPos);
}

void instrDataNode::disableAndDelete(vector<addrVecType> pointsToCheck) {
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }
  
  if(! dontInsertData_) {
    variableMgr &varMgr = proc->getVariableMgr();
    varMgr.makePendingFree(varType, varIndex, trampsMaybeUsing);
  }
  delete this;
}


