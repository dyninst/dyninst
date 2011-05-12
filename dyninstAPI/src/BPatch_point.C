/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#ifdef rs6000_ibm_aix4_1
#include <memory.h>
#endif

#include "common/h/headers.h"

#define BPATCH_FILE

#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_collections.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_libInfo.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "instP.h"
#include "baseTramp.h"
#include "miniTramp.h"
#include "function.h"

#include "BPatch_memoryAccessAdapter.h"

#include "BPatch_edge.h"
#include "ast.h"
#include "mapped_module.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include "mapped_object.h"

/*
 * Private constructor, insn
 */
BPatch_point::BPatch_point(BPatch_addressSpace *_addSpace, 
                           BPatch_function *_func, instPoint *_point,
                           instPoint *_secondary,
                           BPatch_procedureLocation _pointType,
                           AddressSpace *as) :
   addSpace(_addSpace), lladdSpace(as), func(_func), 
   point(_point), secondaryPoint(_secondary),
   pointType(_pointType), memacc(NULL), dynamic_point_monitor_func(NULL),
   edge_(NULL)
{
   assert(point->func() == _func->lowlevel_func());

   if (_pointType == BPatch_subroutine)
      dynamic_call_site_flag = 2; // dynamic status unknown
   else
      dynamic_call_site_flag = 0; // not a call site, so not a dynamic call site.
   // I'd love to have a "loop" constructor, but the code structure
   // doesn't work right. We create an entry point as a set of edge points,
   // but not all edge points are loop points.
   loop = NULL;
   
   // And check to see if there's already instrumentation there (from a fork, say)
   
   // TODO: we either need to change BPatch_points to match the new instPoint model,
   // or look up any other instPoints that might be in the area. I'd suggest 
   // changing BPatch_points, because otherwise we get all sorts of weird from the
   // function/block entry + first insn problem.
   for (instPoint::iterator iter = point->begin(); iter != point->end(); ++iter) {
      BPatchSnippetHandle *handle = new BPatchSnippetHandle(addSpace);
      handle->addMiniTramp(*iter);
      preSnippets.push_back(handle);
   }
}

/*
 * Private constructor, edge
 */
BPatch_point::BPatch_point(BPatch_addressSpace *_addSpace, 
                           BPatch_function *_func, 
                           BPatch_edge *_edge, instPoint *_point,
                           AddressSpace *as) :
   addSpace(_addSpace), lladdSpace(as), func(_func), 
   point(_point), secondaryPoint(NULL),
   pointType(BPatch_locInstruction), memacc(NULL),
   dynamic_call_site_flag(0), dynamic_point_monitor_func(NULL),edge_(_edge)
{
  // I'd love to have a "loop" constructor, but the code structure
  // doesn't work right. We create an entry point as a set of edge points,
  // but not all edge points are loop points.
  loop = NULL;
  // We override that later... also, a single edge could be multiple loops. We need
  // "virtual" points.   

  // And check to see if there's already instrumentation there (from a fork, say)

  for (instPoint::iterator iter = point->begin(); iter != point->end(); ++iter) {
     BPatchSnippetHandle *handle = new BPatchSnippetHandle(addSpace);
     handle->addMiniTramp(*iter);
     preSnippets.push_back(handle);
  }
}

/*
 * BPatch_point::setLoop
 *
 * For a BPatch_point representing a loop instrumentation site,
 * set the loop that it represents.
 */

void BPatch_point::setLoop(BPatch_basicBlockLoop *l) {
  // We currently can use a single BPatch_point to represent
  // multiple loops. This is a problem, since we would really
  // like the points to label a unique loop. On the other hand,
  // then multiple points would share the same physical address..
  // not good.


  // Point must be for a loop.
  assert(pointType == BPatch_locLoopEntry ||
	 pointType == BPatch_locLoopExit ||
	 pointType == BPatch_locLoopStartIter ||
	 pointType == BPatch_locLoopEndIter);

  loop = l;
}

/*
 * BPatch_point::getPointType
 *
 * Returns type of BPatch_point
 */

BPatch_procedureLocation BPatch_point::getPointTypeInt() 
{ 
   return pointType; 
}

/*
 * BPatch_point::getLoop
 *
 * Returns loop if of appropriate type
 */

BPatch_basicBlockLoop *BPatch_point::getLoopInt() 
{ 
   return loop; 
}

/*
 * BPatch_point::getAddressSpace
 *
 * Returns the point's address space
 */

BPatch_addressSpace *BPatch_point::getAddressSpaceInt()
{
   return addSpace;
}

/*
 * BPatch_point::getFunction
 *
 * Returns function to which this BPatch_point belongs
 */

BPatch_function *BPatch_point::getFunctionInt()
{
   return func;
}

/*
 * BPatch_point::getCalledFunction
 *
 * For a BPatch_point representing a call site, returns a pointer to a
 * BPatch_function that represents the function being called.  If the point
 * isn't a call site, returns NULL.
 */
BPatch_function *BPatch_point::getCalledFunctionInt()
{
   assert(point);

   if (!func->getModule()->isValid()) {
	   return NULL;
   }
   if (addSpace->getType() == TRADITIONAL_PROCESS) {
       BPatch_process *proc = dynamic_cast<BPatch_process *>(addSpace);
       mapped_object *obj = func->getModule()->lowlevel_mod()->obj();
       if (proc->lowlevel_process()->mappedObjIsDeleted(obj)) {
		   return NULL;
	   }
   }
   
   if (point->type() != instPoint::PreCall &&
       point->type() != instPoint::PostCall) {
       parsing_printf("findCallee failed in getCalledFunction- not a call site\n");
	   return NULL;
   }
   
   func_instance *_func = point->block()->callee();
   
   if (!_func) {
       parsing_printf("findCallee failed in getCalledFunction\n");
	   return NULL;
   }
   return addSpace->findOrCreateBPFunc(_func, NULL);
}

std::string BPatch_point::getCalledFunctionNameInt() {
	assert(point->block());
	return point->block()->obj()->getCalleeName(point->block());
}

/*  BPatch_point::getCFTargets
 *  Returns true if the point corresponds to a control flow
 *  instruction whose target can be statically determined, in which
 *  case "target" is set to the targets of the control flow instruction
 */
bool BPatch_point::getCFTargets(BPatch_Vector<Address> &targets)
{
   assert(0 && "TODO");
   return false;
#if 0
    bool ret = true;
    if (point->isDynamic()) {
        if (point->getSavedTargets(targets)) {
            return true;
        } else {
            return false;
        }
    }
    switch(point->getPointType()) 
    {
      case callSite: 
      {
        Address targ = point->callTarget();
        if (targ) {
            targets.push_back(targ);
        } else {
            ret = false;
        }
        break;
      }
      case abruptEnd:
        targets.push_back( point->block()->end() );
        break;
      default: 
      { // branch or jump. 
        // don't miss targets to invalid addresses 
        // (these get linked to the sink block by ParseAPI)
        using namespace ParseAPI;
        Address baseAddr = point->block()->start() 
                         - point->block()->llb()->start();
        Block::edgelist & trgs = point->block()->llb()->targets();
        Block::edgelist::iterator iter = trgs.begin();
        mapped_object *obj = point->func()->obj();
        Architecture arch = point->proc()->getArch();

        for ( ; iter != trgs.end(); iter++) {
            if ( ! (*iter)->sinkEdge() ) {
                targets.push_back( baseAddr + (*iter)->trg()->start() );
            } else {
                // if this is a cond'l branch taken or direct 
                // edge, decode the instruction to get its target, 
                // otherwise we won't find a target for this insn
                switch((*iter)->type()) {
                case INDIRECT:
                    break;
                case COND_NOT_TAKEN:
                case FALLTHROUGH:
                    if (point->proc()->proc() && 
                        BPatch_defensiveMode == 
                        point->proc()->proc()->getHybridMode()) 
                    {
                        assert( 0 && "should be an abrupt end point");
                    }
                    break;
                default:
                { // this is a cond'l taken or jump target
#if defined(cap_instruction_api)
                using namespace InstructionAPI;
                RegisterAST::Ptr thePC = RegisterAST::Ptr
                    ( new RegisterAST( MachRegister::getPC( arch ) ) );

                void *ptr = obj->getPtrToInstruction(point->addr());
                assert(ptr);
                InstructionDecoder dec
                    (ptr, InstructionDecoder::maxInstructionLength, arch);
                Instruction::Ptr insn = dec.decode();
                Expression::Ptr trgExpr = insn->getControlFlowTarget();
                    // FIXME: templated bind()
                trgExpr->bind(thePC.get(), 
                              Result(s64, point->block()->llb()->lastInsnOffset()));
                Result actualTarget = trgExpr->eval();
                if(actualTarget.defined)
                {
                    Address targ = actualTarget.convert<Address>() + baseAddr;
                    targets.push_back( targ );
                }
                break;
                } // end default case
                } // end edge-type switch
#endif
            }
        }
        break;
      }
    }

    return ret;
#endif
}

Address BPatch_point::getCallFallThroughAddr()
{
    assert(point);
    using namespace InstructionAPI;
    if (!point->block()) return 0;
    edge_instance *fte = point->block()->getFallthrough();
    if (fte && !fte->sinkEdge()) return fte->trg()->start();
    else return point->block()->end();
}

bool BPatch_point::getSavedTargets(vector<Address> & targs)
{
   assert(0);
   return false;
}

void BPatch_point::attachMemAcc(BPatch_memoryAccess *newMemAcc) {
    if (memacc) {
        // Change if we ever want to override these...
        assert(newMemAcc == memacc);
    }
    else
        memacc = newMemAcc;
}

const BPatch_memoryAccess *BPatch_point::getMemoryAccessInt()
{
    if (!func->getModule()->isValid()) return NULL;

    if (memacc) { 
        return memacc;
    }
    //    fprintf(stderr, "No memory access recorded for 0x%lx, grabbing now...\n",
    //      point->addr());
    assert(point);
    // Try to find it... we do so through an InstrucIter
    Dyninst::InstructionAPI::Instruction::Ptr i = getInsnAtPointInt();
    BPatch_memoryAccessAdapter converter;
    
    attachMemAcc(converter.convert(i, point->insnAddr(), point->proc()->getAddressWidth() == 8));
    return memacc;
}

InstructionAPI::Instruction::Ptr BPatch_point::getInsnAtPointInt()
{
    return point->insn();
}


const BPatch_Vector<BPatchSnippetHandle *> BPatch_point::getCurrentSnippetsInt() 
{
    allSnippets.clear();

    if (!func->getModule()->isValid()) return allSnippets;


    for (unsigned pre = 0; pre < preSnippets.size(); pre++) 
        allSnippets.push_back(preSnippets[pre]);
    for (unsigned post = 0; post < postSnippets.size(); post++) 
        allSnippets.push_back(postSnippets[post]);
    
    return allSnippets;
}

const BPatch_Vector<BPatchSnippetHandle *> 
BPatch_point::getCurrentSnippetsByWhen(BPatch_callWhen when) 
{
    if (when == BPatch_callBefore)
        return preSnippets;
    else {
        assert(when == BPatch_callAfter);
        return postSnippets;
    }
}

#include "registerSpace.h"

#if defined(cap_liveness)
bool BPatch_point::getLiveRegistersInt(std::vector<BPatch_register> &liveRegs)
{
    // Good question: pre- or post-instruction? I'm going to assume pre-instruction.

   bitArray live = point->liveRegisters();
   
   std::vector<BPatch_register> allRegs;
   addSpace->getRegisters(allRegs); 
   
   for (unsigned i = 0; i < allRegs.size(); i++) {
      if (live[allRegs[i].number_])
         liveRegs.push_back(allRegs[i]);
   }
   return true;
   
}
#else
bool BPatch_point::getLiveRegistersInt(std::vector<BPatch_register> &)
{
    // Oops
    return false;
}
#endif


/*
 * BPatch_point::getAddress
 *
 * Returns the original address of the first instruction at this point.
 */
void *BPatch_point::getAddressInt()
{
    return (void *)point->nextExecutedAddr();
}


/*
 * BPatch_point::usesTrap_NP
 *
 * Returns true if this point is or would be instrumented with a trap, rather
 * than a jump to the base tramp, false otherwise.  On platforms that do not
 * use traps (everything other than x86), it always returns false;
 *
 */
bool BPatch_point::usesTrap_NPInt()
{
   assert(point);
   return false;
   //return point->usesTrap();
}

/*
 * BPatch_point::isDynamic
 *
 * Returns true if this point is a dynamic control transfer site.
 *
 */
bool BPatch_point::isDynamicInt()
{
   if (!point) return false;

   switch (point->type()) {
      case instPoint::PreCall:
      case instPoint::PostCall:
         return point->block()->containsDynamicCall();
         break;
      case instPoint::Edge: 
         return point->edge()->sinkEdge();
         break;
      default:
         return false;
   }
}

/*
 * BPatch_point::monitorCalls(BPatch_function *userCBFunc)
 *
 * Insert function call to user-defined callback function
 * at dynamic call site.
 *
 * Returns false if BPatch_point is not a dynamic call site.
 *
 */
void *BPatch_point::monitorCallsInt( BPatch_function * user_cb ) 
{
  BPatch_function *func_to_use = user_cb;

  if (!func->getModule()->isValid()) {
    fprintf(stderr, "%s[%d]: invalid module, cannot monitor\n",
	    FILE__, __LINE__);
    return NULL;
  }
  if ( !isDynamic() ) {
    fprintf(stderr, "%s[%d]:  call site is not dynamic, cannot monitor\n", 
            __FILE__, __LINE__ );
    return NULL;
  }

  if ( dynamic_point_monitor_func ) {
    fprintf(stderr, "%s[%d]:  call site is already monitored\n", 
            __FILE__, __LINE__ );
    return NULL;
  }

  if (!func_to_use) {
    BPatch_image *bpi = addSpace->getImage();
    assert(bpi);
    //  if no user cb is provided, use the one in the rt lib
    BPatch_Vector<BPatch_function *> funcs;
    bpi->findFunction("DYNINSTasyncDynFuncCall", funcs);
    if (!funcs.size()) {
      fprintf(stderr, "%s[%d]:  cannot find function DYNINSTasyncDynFuncCall\n", FILE__, __LINE__);
      return NULL;
    }
    func_to_use = funcs[0];
  }
  // The callback takes two arguments: the first is the (address of the) callee,
  // the second the (address of the) callsite. 

  InstructionAPI::Instruction::Ptr insn = point->block()->getInsn(point->block()->last());
  pdvector<AstNodePtr> args;
  if (!lladdSpace->getDynamicCallSiteArgs(insn, point->block()->last(), args))
      return NULL;
  if (args.size() != 2)
      return NULL;


  // construct function call and insert
  func_instance * fb = func_to_use->lowlevel_func();

  // Monitoring function
  AstNodePtr ast = AstNode::funcCallNode(fb, args);
  

#if 0
  miniTramp *res = point->instrument(ast,
				     callPreInsn,
				     orderLastAtPoint,
				     true,
				     false);
#endif
  miniTramp *res = point->push_back(ast, true);

  if (addSpace->pendingInsertions == NULL) {
    // Trigger it now
    bool tmp;
    addSpace->finalizeInsertionSet(false, &tmp);
  }   
  
  if ( ! res ) {
     fprintf( stderr,"%s[%d]:  insertSnippet failed, cannot monitor call site\n",
               __FILE__, __LINE__ );
     return NULL;
  }

  //  Let asyncEventHandler know that we are being monitored
  if (getAsync()) {
      getAsync()->registerMonitoredPoint(this);
  }

  dynamic_point_monitor_func = res;

  //  Return pointer to handle as unique id, user does not need to know its a
  //  miniTramp.

  return (void*) res;
} /* end monitorCalls() */

bool BPatch_point::stopMonitoringInt()
{
  if (!dynamic_point_monitor_func) {
    bperr("%s[%d]:  call site not currently monitored", __FILE__, __LINE__);
    return false;
  }
  bool ret;
  ret = dynamic_point_monitor_func->uninstrument();

  dynamic_point_monitor_func = NULL;
  return ret;
}

/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */ 

int BPatch_point::getDisplacedInstructionsInt(int maxSize, void* insns)
{
   return 0;
}

// This isn't a point member because it relies on instPoint.h, which
// we don't want to include in BPatch_point.h. If we had a public "enumerated types"
// header file this could move.
bool BPatchToInternalArgs(BPatch_point *point,
                          BPatch_callWhen when,
                          BPatch_snippetOrder order,
                          callWhen &ipWhen,
                          callOrder &ipOrder) {
    // Edge instrumentation: overrides inputs
  
  if (point->edge()) {
    if (when == BPatch_callAfter) {
      // Can't do this... there is no "before" or 
      // "after" for an edge
      return false;
    }
    switch(point->edge()->getType()) {
    case CondJumpTaken:
    case UncondJump:
      ipWhen = callBranchTargetInsn;
      break;
    case CondJumpNottaken:
    case NonJump:
      ipWhen = callPostInsn;
      break;
    default:
       fprintf(stderr, "Unknown edge type %d\n", point->edge()->getType());
      assert(0);
    }
  }
  else {
    // Instruction level
    if (when == BPatch_callBefore)
      ipWhen = callPreInsn;
    else if (when == BPatch_callAfter)
      ipWhen = callPostInsn;
    else if (when == BPatch_callUnset)
      ipWhen = callPreInsn;
  }
  
  
  if (order == BPatch_firstSnippet)
    ipOrder = orderFirstAtPoint;
  else if (order == BPatch_lastSnippet)
    ipOrder = orderLastAtPoint;
  else
    return false;
  
  //
    // Check for valid combinations of BPatch_procedureLocation & call*
    // 	Right now we don't allow
    //		BPatch_callBefore + BPatch_exit
    //		BPatch_callAfter + BPatch_entry
    //
    //	These combinations are intended to be used to mark the point that
    //      is the last, first valid point where the local variables are
    //      valid.  This is different than the first/last instruction of
    //      a subroutine which is what the other combinations of BPatch_entry
    //	    and BPatch_exit refer to.
    //
    if (when == BPatch_callBefore && point->getPointType() == BPatch_exit) {
        BPatch_reportError(BPatchSerious, 113,
                           "BPatch_callBefore at BPatch_exit not supported yet");
        return false;
    }
    if (when == BPatch_callAfter && point->getPointType() == BPatch_entry) {
        BPatch_reportError(BPatchSerious, 113,
                           "BPatch_callAfter at BPatch_entry not supported yet");
        return false;
    }
    
    if ((point->getPointType() == BPatch_exit)) {
        //  XXX - Hack! 
        //  The semantics of pre/post insn at exit are setup for the new
        //  defintion of using this to control before/after stack creation,
        //  but the lower levels of dyninst don't know about this yet.
        ipWhen = callPreInsn;
    }
    
    return true;
}

BPatch_procedureLocation BPatch_point::convertInstPointType_t(int intType)
{
    BPatch_procedureLocation ret = (BPatch_procedureLocation)-1;
    switch((instPoint::Type) intType) {
       case instPoint::FuncEntry:
          return BPatch_locEntry;
       case instPoint::FuncExit:
          return BPatch_locExit;
       case instPoint::PreCall:
       case instPoint::PostCall:
          return BPatch_locSubroutine;
       default:
          return BPatch_locInstruction;
    }
}

void BPatch_point::recordSnippet(BPatch_callWhen when,
                                 BPatch_snippetOrder order,
                                 BPatchSnippetHandle *handle) {
   if (when == BPatch_callUnset) {
      if (getPointType() == BPatch_exit)
         when = BPatch_callAfter;
      else
         when = BPatch_callBefore;
   }

   if (when == BPatch_callBefore)
      if (order == BPatch_firstSnippet) {
#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
         preSnippets.insert(preSnippets.begin(), handle);
#else
         preSnippets.push_front(handle);
#endif
      }
      else {
         preSnippets.push_back(handle);
      }
   else {
      if (order == BPatch_firstSnippet) {
#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
         postSnippets.insert(postSnippets.begin(), handle);
#else
         postSnippets.push_front(handle);
#endif
      }
      else {
         postSnippets.push_back(handle);
      }
   }        

}

// Removes snippet from datastructures, doesn't actually remove the 
// instrumentation.  Is invoked by BPatch_addressSpace::deleteSnippet
bool BPatch_point::removeSnippet(BPatchSnippetHandle *handle)
{
   bool foundHandle = false;

   std::vector<BPatchSnippetHandle *>::iterator iter;

   for (iter = allSnippets.begin(); iter != allSnippets.end(); ++iter) {
      if (*iter == handle) {
         allSnippets.erase(iter);
         foundHandle = true;
         break;
      }
   }

   for (iter = preSnippets.begin(); iter != preSnippets.end(); ++iter) {
      if (*iter == handle) {
         preSnippets.erase(iter);
         foundHandle = true;
         break;
      }
   }

   for (iter = postSnippets.begin(); iter != postSnippets.end(); ++iter) {
      if (*iter == handle) {
         postSnippets.erase(iter);
         foundHandle = true;
         break;
      }
   }

   return foundHandle;
}

AddressSpace *BPatch_point::getAS()
{
   return lladdSpace;
}

bool BPatch_point::isReturnInstruction()
{
   return point->type() == instPoint::FuncExit;
}

bool BPatch_point::patchPostCallArea()
{
    if (point->proc()->proc()) {
        return point->proc()->proc()->patchPostCallArea(point);
    }
    return false;
}

instPoint *BPatch_point::getPoint(BPatch_callWhen when) {
   switch(when) {
      case BPatch_callBefore:
      case BPatch_callUnset:
         return point;
      case BPatch_callAfter:
         return (secondaryPoint ? secondaryPoint : point);
   }
   assert(0);
   return NULL;
}
