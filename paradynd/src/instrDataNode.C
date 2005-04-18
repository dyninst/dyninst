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

#include "common/h/Types.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/pd_process.h"
#include "dyninstAPI/src/BPatch_typePrivate.h"


instrDataNode::instrDataNode(pd_process *proc_, unsigned type,
			     bool arg_dontInsertData, HwEvent* hw_event)
  : proc(proc_), thrNodeClientSet(false), dontInsertData_(arg_dontInsertData)
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
    case MDL_T_HW_TIMER:
      varType = HwTimer;
      break;
    case MDL_T_HW_COUNTER:
      varType = HwCounter;
      break;
    case MDL_T_NONE:
      // just to keep mdl apply allocate a dummy un-sampled counter.
      varType = Counter;
      break;
    default:
      assert(0);  break;
  }

  hw = hw_event;
  variableMgr &varMgr = proc->getVariableMgr();
  varIndex = varMgr.allocateForInstVar(varType, hw_event);
  refCount = 0;
  varExpr = NULL;
}

instrDataNode::instrDataNode(const instrDataNode &par, pd_process *childProc) :
  proc(childProc), varType(par.varType), varIndex(par.varIndex),
  thrNodeClientSet(par.thrNodeClientSet), 
  dontInsertData_(par.dontInsertData_), 
  refCount(0), // has 0 references since it's a new data node
  varExpr(NULL)
{
}

// the disable method should be called before this destructor
instrDataNode::~instrDataNode() {
  // Should call deletion method of the variable mgr
  //cerr << "~instrDataNode, " << this << ", pid: " << proc->getPid() 
  //     << ", type: " << varType 
  //     << ", index: " << varIndex << "\n";
  if(! dontInsertData_) {
    variableMgr &varMgr = proc->getVariableMgr();
    varMgr.free(varType, varIndex);
  }
  if (varExpr) delete varExpr;
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
    assert(varAddr != 0);
  }
  return varAddr;
}

BPatch_variableExpr *instrDataNode::getVariableExpr()
{
  if (varExpr) return varExpr;

  BPatch_type *base_type = NULL;
  char *type_str = NULL;

  switch (varType) {
  case Counter:
    type_str = "intCounter";
    break;
  case WallTimer:
  case ProcTimer:
    type_str = "tTimer";
    break;
  case HwTimer:
    type_str = "tHwTimer";
    break;
  case HwCounter:
    type_str = "tHwCounter";
    break;
  default:
    assert(0);
    break;
  }


  char type_str_ptr_name[128];
/*  PDSEP -- CLEAN ME
  base_type = proc->get_dyn_process()->getImage()->findType(type_str);
  if (!base_type) {
    fprintf(stderr, "%s[%d]:  FIXME:  cannot find %s data type!\n",
            __FILE__, __LINE__, type_str);
    // try making this type up (we really just need a placeholder for its size)
    base_type = new BPatch_typeScalar(getSize(), type_str);
  }
  else {
*/
  base_type = new BPatch_typeScalar(getSize(), type_str);

  // get address of counter/timer:
  void *counterAddr = (void *) getInferiorPtr();

  char counterName[128];
  sprintf(counterName, "%s-%p", type_str, counterAddr);
  
  if( ! proc->multithread_capable() ) {
    varExpr = new BPatch_variableExpr( counterName, proc->get_bprocess(), counterAddr, base_type );
    return varExpr;
    }

  sprintf(type_str_ptr_name, "%sPtr", type_str);
  //  make a new type that is an array of intCounter (or tTimer, etc)
  BPatch_type *base_type_ptr_type = new BPatch_typeArray(base_type, 0, 1024,
                                                          type_str_ptr_name);

  // "create" a variable at the counter address.
  varExpr = new BPatch_variableExpr(counterName, proc->get_bprocess(),
                                    counterAddr, base_type_ptr_type);
  return varExpr;
}

unsigned instrDataNode::getSize() const {
  switch (varType) {
  case Counter:
    return sizeof(intCounter);
    break;
  case WallTimer:
  case ProcTimer:
    return sizeof(tTimer);
    break;
  case HwTimer:
    return sizeof(tHwTimer);
    break;
  case HwCounter:
    return sizeof(tHwCounter);
    break;
  default:
    assert(0);
    break;
  }
  return 0;
}

void instrDataNode::print() {
   cerr << "D:" << (void*)this << "\n";
}

void instrDataNode::prepareForSampling(unsigned thrPos,
				       threadMetFocusNode_Val *thrClient) { 
  thrNodeClientSet = true;
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsSampled(varType, varIndex, thrPos, thrClient);
}

void instrDataNode::stopSampling(unsigned thrPos) {
  assert(!dontInsertData_);
  thrNodeClientSet = false;
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsNotSampled(varType, varIndex, thrPos);
}

void instrDataNode::incRefCount()
{
  refCount++;
}

int instrDataNode::decRefCount()
{
  int newcount = --refCount;
  if (refCount == 0)
    delete this;
  return newcount;
}
