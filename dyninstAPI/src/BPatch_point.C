/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "common/src/headers.h"

#define BPATCH_FILE

#include "BPatch_snippet.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "BPatch_collections.h"
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_libInfo.h"
#include "image.h"
#include "instPoint.h"
#include "instP.h"
#include "baseTramp.h"
#include "function.h"
#include "addressSpace.h"
#include "dynProcess.h"
#include "debug.h"

#include "BPatch_memoryAccessAdapter.h"

#include "BPatch_edge.h"
#include "ast.h"
#include "mapped_module.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include "mapped_object.h"
#include "Snippet.h"

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
   pointType(_pointType), memacc(NULL),
   edge_(NULL)
{
   assert(point->func() == _func->lowlevel_func());

   // I'd love to have a "loop" constructor, but the code structure
   // doesn't work right. We create an entry point as a set of edge points,
   // but not all edge points are loop points.
   loop = NULL;

   // And check to see if there's already instrumentation there (from a fork, say)

   // TODO: we either need to change BPatch_points to match the new instPoint model,
   // or look up any other instPoints that might be in the area. I'd suggest
   // changing BPatch_points, because otherwise we get all sorts of weird from the
   // function/block entry + first insn problem.
   for (instPoint::instance_iter iter = point->begin(); iter != point->end(); ++iter) {
      BPatchSnippetHandle *handle = new BPatchSnippetHandle(addSpace);
      handle->addInstance(*iter);
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
   edge_(_edge)
{
  // I'd love to have a "loop" constructor, but the code structure
  // doesn't work right. We create an entry point as a set of edge points,
  // but not all edge points are loop points.
  loop = NULL;
  // We override that later... also, a single edge could be multiple loops. We need
  // "virtual" points.

  // And check to see if there's already instrumentation there (from a fork, say)
   for (instPoint::instance_iter iter = point->begin(); iter != point->end(); ++iter) {
      BPatchSnippetHandle *handle = new BPatchSnippetHandle(addSpace);
      handle->addInstance(*iter);
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

BPatch_procedureLocation BPatch_point::getPointType()
{
   return pointType;
}

/*
 * BPatch_point::getLoop
 *
 * Returns loop if of appropriate type
 */

BPatch_basicBlockLoop *BPatch_point::getLoop()
{
   return loop;
}

/*
 * BPatch_point::getAddressSpace
 *
 * Returns the point's address space
 */

BPatch_addressSpace *BPatch_point::getAddressSpace()
{
   return addSpace;
}

/*
 * BPatch_point::getFunction
 *
 * Returns function to which this BPatch_point belongs
 */

BPatch_function *BPatch_point::getFunction()
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
BPatch_function *BPatch_point::getCalledFunction()
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
   if (_func->getPowerPreambleFunc() != NULL) {
       func_instance * preambleFunc = _func->getPowerPreambleFunc();
       return addSpace->findOrCreateBPFunc(preambleFunc, NULL);
   }


   return addSpace->findOrCreateBPFunc(_func, NULL);
}

std::string BPatch_point::getCalledFunctionName() {
	assert(point->block());
	return point->block()->obj()->getCalleeName(point->block());
}

/*
 * BPatch_point::getBlock
 *
 * Returns block to which this BPatch_point belongs
 */

BPatch_basicBlock *BPatch_point::getBlock()
{
   if (!point) return NULL;
   block_instance *llblock = point->block();
   if (!llblock) return NULL;
   return func->getCFG()->findBlock(llblock);

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

void BPatch_point::attachMemAcc(BPatch_memoryAccess *newMemAcc) {
    if (memacc) {
        // Change if we ever want to override these...
        assert(newMemAcc == memacc);
    }
    else
        memacc = newMemAcc;
}

const BPatch_memoryAccess *BPatch_point::getMemoryAccess()
{
    if (!func->getModule()->isValid()) return NULL;

    if (memacc) {
        return memacc;
    }
    //    fprintf(stderr, "No memory access recorded for 0x%lx, grabbing now...\n",
    //      point->addr());
    assert(point);
    Dyninst::InstructionAPI::Instruction i = getInsnAtPoint();
    if (!i.isValid()) return NULL;
    BPatch_memoryAccessAdapter converter;

    attachMemAcc(converter.convert(i, point->insnAddr(), point->proc()->getAddressWidth() == 8));
    return memacc;
}

InstructionAPI::Instruction BPatch_point::getInsnAtPoint()
{
    return point->insn();
}


const BPatch_Vector<BPatchSnippetHandle *> BPatch_point::getCurrentSnippets()
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
BPatch_point::getCurrentSnippets(BPatch_callWhen when)
{
    if (when == BPatch_callBefore)
        return preSnippets;
    else {
        assert(when == BPatch_callAfter);
        return postSnippets;
    }
}

#include "registerSpace.h"

bool BPatch_point::getLiveRegisters(std::vector<BPatch_register> &liveRegs)
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

/*
 * BPatch_point::getAddress
 *
 * Returns the original address of the first instruction at this point.
 */
void *BPatch_point::getAddress()
{
    return (void *)point->addr_compat();
}


/*
 * BPatch_point::usesTrap_NP
 *
 * Returns true if this point is or would be instrumented with a trap, rather
 * than a jump to the base tramp, false otherwise.  On platforms that do not
 * use traps (everything other than x86), it always returns false;
 *
 */
bool BPatch_point::usesTrap_NP()
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
bool BPatch_point::isDynamic()
{
   if (!point) return false;

   switch (point->type()) {
      case instPoint::PreCall:
      case instPoint::PostCall:
         return point->block()->containsDynamicCall();
      case instPoint::EdgeDuring:
         switch(point->edge()->type()) {
            case ParseAPI::INDIRECT:
               return true;
            case ParseAPI::CALL:
               return point->edge()->src()->containsDynamicCall();
            default:
               return false;
          }
         return false;
      case instPoint::FuncExit:
      case instPoint::BlockEntry:
      case instPoint::BlockExit:
         return false;
      default:
         if (point->addr() == point->block()->last()) {
             if (point->block()->containsCall()) {
                 return point->block()->containsDynamicCall();
             }
             PatchAPI::PatchBlock::edgelist trgs = point->block()->targets();
             for (PatchAPI::PatchBlock::edgelist::iterator eit = trgs.begin();
                  eit != trgs.end(); 
                  eit++)
             {
                 if ((*eit)->type() == ParseAPI::INDIRECT) {
                     return true;
                 }
             }
         }
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
void *BPatch_point::monitorCalls( BPatch_function * user_cb )
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
    BPatch::bpatch->info->registerMonitoredPoint(this);
  }
  // The callback takes two arguments: the first is the (address of the) callee,
  // the second the (address of the) callsite.

  InstructionAPI::Instruction insn = point->block()->getInsn(point->block()->last());
  std::vector<AstNodePtr> args;
  if (!lladdSpace->getDynamicCallSiteArgs(insn, point->block()->last(), args))
      return NULL;
  if (args.size() != 2)
      return NULL;


  // construct function call and insert
  func_instance * fb = func_to_use->lowlevel_func();

  // Monitoring function
  AstNodePtr ast = AstNode::funcCallNode(fb, args);

  Dyninst::PatchAPI::InstancePtr res = point->pushBack(ast);

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

  dynamic_point_monitor_func = res;

  //  Return pointer to handle as unique id, user does not need to know its a
  //  miniTramp.

  return (void*) res.get();
} /* end monitorCalls() */

bool BPatch_point::stopMonitoring()
{
  if (!dynamic_point_monitor_func) {
    bperr("%s[%d]:  call site not currently monitored", __FILE__, __LINE__);
    return false;
  }
  bool ret;
  ret = uninstrument(dynamic_point_monitor_func);

  dynamic_point_monitor_func = Dyninst::PatchAPI::InstancePtr();
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

int BPatch_point::getDisplacedInstructions(int /*maxSize*/, void* /*insns*/)
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
  //  Right now we don't allow
  //          BPatch_callBefore + BPatch_exit
  //          BPatch_callAfter + BPatch_entry
  //
  //  These combinations are intended to be used to mark the point that
  //      is the last, first valid point where the local variables are
  //      valid.  This is different than the first/last instruction of
  //      a subroutine which is what the other combinations of BPatch_entry
  //      and BPatch_exit refer to.
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
         preSnippets.insert(preSnippets.begin(), handle);
      }
      else {
         preSnippets.push_back(handle);
      }
   else {
      if (order == BPatch_firstSnippet) {
         postSnippets.insert(postSnippets.begin(), handle);
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

instPoint *BPatch_point::getPoint(BPatch_callWhen when) const {
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


Dyninst::PatchAPI::Point *Dyninst::PatchAPI::convert(const BPatch_point *p, BPatch_callWhen when) {
  return p->getPoint(when);
}
