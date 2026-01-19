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

// $Id: ast.C,v 1.209 2008/09/15 18:37:49 jaw Exp $

#include "addressSpace.h"
#include "ast.h"
#include "binaryEdit.h"
#include "BPatch.h"
#include "BPatch_collections.h"
#include "BPatch_function.h"
#include "BPatch_libInfo.h" // For instPoint->BPatch_point mapping
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "Buffer.h"
#include "debug.h"
#include "dyninst_visibility.h"
#include "emitter.h"
#include "function.h"
#include "image.h"
#include "inst.h"
#include "instPoint.h"
#include "Instruction.h"
#include "mapped_module.h"
#include "mapped_object.h"
#include "RegisterConversion.h"
#include "registerSpace.h"
#include "regTracker.h"
#include "ast_helpers.h"


using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using PatchAPI::Point;

static bool isPowerOf2(Dyninst::Address addr) {
  // Hacker's Delight, chapter 1
  if(addr == 0UL) {
    return false;
  }
  return (addr & (addr - 1UL)) == 0UL;
}

static bool IsSignedOperation(BPatch_type *l, BPatch_type *r) {
    if (l == NULL || r == NULL) return true;
    if (strstr(l->getName(), "unsigned") == NULL) return true;
    if (strstr(r->getName(), "unsigned") == NULL) return true;
    return false;
}

AstOperatorNode::AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r, AstNodePtr e) :
    AstNode(),
    op(opC),
    loperand(l),
    roperand(r),
    eoperand(e)
{
    // Optimization pass...
    if(!loperand) return;
    if(roperand) {
      if (op == plusOp) {
          if (loperand->getoType() == operandType::Constant) {
              // Swap left and right...
              AstNodePtr temp = loperand;
              loperand = roperand;
              roperand = temp;
          }
      }
      if (op == timesOp) {
          if (roperand->getoType() == operandType::undefOperandType) {
              // ...
         }
          else if (roperand->getoType() != operandType::Constant) {
              AstNodePtr temp = roperand;
              roperand = loperand;
              loperand = temp;
          }
          else {
              if (!isPowerOf2((Address)roperand->getOValue()) &&
                  isPowerOf2((Address)loperand->getOValue())) {
                  AstNodePtr temp = roperand;
                  roperand = loperand;
                  loperand = temp;
              }
          }
      }
    }

    if (loperand) children.push_back(loperand);
    if (roperand) children.push_back(roperand);
    if (eoperand) children.push_back(eoperand);
}

    // Direct operand
AstOperandNode::AstOperandNode(operandType ot, void *arg) :
    AstNode(),
    oType(ot),
    oVar(NULL),
    operand_()
{

    if (ot == operandType::ConstantString)
        oValue = (void *)P_strdup((char *)arg);
    else
        oValue = (void *) arg;

    if (operand_) children.push_back(operand_);
}

// And an indirect (say, a load)
AstOperandNode::AstOperandNode(operandType ot, AstNodePtr l) :
    AstNode(),
    oType(ot),
    oValue(NULL),
    oVar(NULL),
    operand_(l)
{
   if (operand_) children.push_back(operand_);
}

AstOperandNode::AstOperandNode(operandType ot, const image_variable* iv) :
  AstNode(),
  oType(ot),
  oValue(NULL),
  oVar(iv),
  operand_()
{
  assert(oVar);
  if (operand_) children.push_back(operand_);
}


AstCallNode::AstCallNode(func_instance *func,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(0),
    func_(func),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        children.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(func_instance *func) :
    AstNode(),
    func_addr_(0),
    func_(func),
    callReplace_(true),
    constFunc_(false)
{
}

AstCallNode::AstCallNode(const std::string &func,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_name_(func),
    func_addr_(0),
    func_(NULL),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        children.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(Address addr,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(addr),
    func_(NULL),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        children.push_back(args[i]);
    }
}

AstSequenceNode::AstSequenceNode(std::vector<AstNodePtr > &sequence) :
    AstNode()
{
    for (unsigned i = 0; i < sequence.size(); i++) {
        children.push_back(sequence[i]);
    }
}

AstVariableNode::AstVariableNode(vector<AstNodePtr>&ast_wrappers, vector<pair<Dyninst::Offset, Dyninst::Offset> > *ranges) :
    ranges_(ranges), index(0)
{
   children = ast_wrappers;
   assert(!children.empty());
}

AstMemoryNode::AstMemoryNode(memoryType mem,
                             unsigned which,
                             int size_) :
    AstNode(),
    mem_(mem),
    which_(which) {

    assert(BPatch::bpatch != NULL);
    assert(BPatch::bpatch->stdTypes != NULL);


    switch(mem_) {
    case EffectiveAddr:
        switch (size_) {
            case 1:
                bptype = BPatch::bpatch->stdTypes->findType("char");
                break;
            case 2:
                bptype = BPatch::bpatch->stdTypes->findType("short");
                break;
            case 4:
                bptype = BPatch::bpatch->stdTypes->findType("int");
                break;
            default:
                bptype = BPatch::bpatch->stdTypes->findType("long");
        }
        break;
    case BytesAccessed:
        bptype = BPatch::bpatch->stdTypes->findType("int");
        break;
    default:
        assert(!"Naah...");
    }
    size = bptype->getSize();
    doTypeCheck = BPatch::bpatch->isTypeChecked();
}

#if defined(cap_stack_mods)

bool AstStackGenericNode::generateCode_phase2(codeGen& gen, bool, Address&, Dyninst::Register&)
{
    gen.setInsertNaked(true);
    gen.setModifiedStackFrame(true);

    // No code generation necessary

    return true;
}
#else

bool AstStackGenericNode::generateCode_phase2(codeGen&, bool, Address&, Dyninst::Register&)
{
    return false;
}
#endif

bool AstOperatorNode::initRegisters(codeGen &g) {
    bool ret = true;
    for (unsigned i = 0; i < children.size(); i++) {
        if (!children[i]->initRegisters(g))
            ret = false;
    }

#if !defined(DYNINST_CODEGEN_ARCH_I386)
    // Override: if we're trying to save to an original
    // register, make sure it's saved on the stack.
    if(loperand) {
      if (op == storeOp) {
        if (loperand->getoType() == operandType::origRegister) {
          Address origReg = (Address) loperand->getOValue();
          // Mark that register as live so we are sure to save it.
          registerSlot *r = (*(g.rs()))[origReg];
          r->liveState = registerSlot::live;
        }
      }
    }
#endif
    return ret;
}

#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
bool AstOperatorNode::generateOptimizedAssignment(codeGen &gen, int size_, bool noCost)
{
   (void) size_;
   if(!(loperand && roperand)) { return false; }

   //Recognize the common case of 'a = a op constant' and try to
   // generate optimized code for this case.
   Address laddr;

   if (loperand->getoType() == operandType::DataAddr)
   {
      laddr = (Address) loperand->getOValue();
   }
   else
   {
      if(loperand->getoType() == operandType::variableValue)
      {
         boost::shared_ptr<AstOperandNode> lnode =
            boost::dynamic_pointer_cast<AstOperandNode>(loperand);

         int_variable* var = lnode->lookUpVar(gen.addrSpace());
         if (!var || gen.addrSpace()->needsPIC(var))
            return false;
         laddr = var->getAddress();
      }
      else
      {
         //Deal with global writes for now.
         return false;
      }

   }

   if (roperand->getoType() == operandType::Constant) {
      //Looks like 'global = constant'
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
     if (laddr >> 32 || ((Address) roperand->getOValue()) >> 32 || size_ == 8) {
       // Make sure value and address are 32-bit values.
       return false;
     }


#endif
      int imm = (int) (long) roperand->getOValue();
      emitStoreConst(laddr, (int) imm, gen, noCost);
      loperand->decUseCount(gen);
      roperand->decUseCount(gen);
      return true;
   }

   AstOperatorNode *roper = dynamic_cast<AstOperatorNode *>(roperand.get());
   if (!roper)
      return false;

   if (roper->op != plusOp && roper->op != minusOp)
      return false;

   AstOperandNode *arithl = dynamic_cast<AstOperandNode *>(roper->loperand.get());
   AstOperandNode *arithr = dynamic_cast<AstOperandNode *>(roper->roperand.get());
   if (!arithl || !arithr)
      return false;

   AstNode *const_oper = NULL;
   if (arithl->getoType() == operandType::DataAddr && arithr->getoType() == operandType::Constant &&
       laddr == (Address) arithl->getOValue())
   {
      const_oper = arithr;
   }
   else if (arithl->getoType() == operandType::variableValue && arithr->getoType() == operandType::Constant)
   {
      Address addr = 0;
      int_variable* var = arithl->lookUpVar(gen.addrSpace());
      if (!var || gen.addrSpace()->needsPIC(var))
         return false;
      addr = var->getAddress();
      if (addr == laddr) {
         const_oper = arithr;
      }
   }
   else if (arithr->getoType() == operandType::DataAddr && arithl->getoType() == operandType::Constant &&
            laddr == (Address) arithr->getOValue() && roper->op == plusOp)
   {
      const_oper = arithl;
   }
   else if (arithl->getoType() == operandType::variableValue && arithr->getoType() == operandType::Constant)
   {
      Address addr = 0;
      int_variable* var = arithl->lookUpVar(gen.addrSpace());
      if(!var || gen.addrSpace()->needsPIC(var))
         return false;
      addr = var->getAddress();
      if (addr == laddr) {
         const_oper = arithl;
      }
   }
   else
   {
      return false;
   }

   long int imm = (long int) const_oper->getOValue();
   if (roper->op == plusOp) {
      emitAddSignedImm(laddr, imm, gen, noCost);
   }
   else {
      emitSubSignedImm(laddr, imm, gen, noCost);
   }

   loperand->decUseCount(gen);
   roper->roperand->decUseCount(gen);
   roper->loperand->decUseCount(gen);
   roper->decUseCount(gen);

   return true;
}
#else
bool AstOperatorNode::generateOptimizedAssignment(codeGen &, int, bool)
{
   return false;
}
#endif

bool AstOperatorNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &retAddr,
                                          Dyninst::Register &retReg) {
   if(!loperand) { return false; }

   retAddr = ADDR_NULL; // We won't be setting this...
   // retReg may have a value or be the (register) equivalent of NULL.
   // In either case, we don't touch it...

	RETURN_KEPT_REG(retReg);


   Address addr = ADDR_NULL;

   Dyninst::Register src1 = Dyninst::Null_Register;
   Dyninst::Register src2 = Dyninst::Null_Register;

   Dyninst::Register right_dest = Dyninst::Null_Register;
   Dyninst::Register tmp = Dyninst::Null_Register;

   switch(op) {
      case branchOp: {
         assert(loperand->getoType() == operandType::Constant);
         unsigned offset = (Dyninst::Register) (long) loperand->getOValue();
         // We are not calling loperand->generateCode_phase2,
         // so we decrement its useCount by hand.
         // Would be nice to allow register branches...
         loperand->decUseCount(gen);
         (void)emitA(branchOp, 0, 0, (Dyninst::Register)offset, gen, rc_no_control, noCost);
         retReg = Dyninst::Null_Register; // No return register
         break;
      }
      case ifOp: {
         if(!roperand) { return false; }
         // This ast cannot be shared because it doesn't return a register
         if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         codeBufIndex_t ifIndex= gen.getIndex();

         size_t preif_patches_size = gen.allPatches().size();
         codeBufIndex_t thenSkipStart = emitA(op, src1, 0, 0, gen, rc_before_jump, noCost);

         size_t postif_patches_size = gen.allPatches().size();

	 // We can reuse src1 for the body of the conditional; however, keep the value here
	 // so that we can use it for the branch fix below.
         Dyninst::Register src1_copy = src1;
         gen.rs()->freeRegister(src1);

         // The flow of control forks. We need to add the forked node to
         // the path
         gen.tracker()->increaseConditionalLevel();
         if (!roperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         gen.rs()->freeRegister(src2);
         gen.tracker()->decreaseAndClean(gen);
         gen.rs()->unifyTopRegStates(gen); //Join the registerState for the if

         // Is there an else clause?  If yes, generate branch over it
         codeBufIndex_t elseSkipStart = 0;
         codeBufIndex_t elseSkipIndex = gen.getIndex();
         size_t preelse_patches_size = 0, postelse_patches_size = 0;
         if (eoperand) {
            gen.rs()->pushNewRegState(); //Create registerState for else
            preelse_patches_size = gen.allPatches().size();
            elseSkipStart = emitA(branchOp, 0, 0, 0,
                                  gen, rc_no_control, noCost);
            postelse_patches_size = gen.allPatches().size();
         }

         // Now that we've generated the "then" section, rewrite the if
         // conditional branch.
         codeBufIndex_t elseStartIndex = gen.getIndex();

         if (preif_patches_size != postif_patches_size) {
            assert(postif_patches_size > preif_patches_size);
            ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].setTarget(&if_targ);
            }
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].applyPatch();
            }
         }
         else {
            gen.setIndex(ifIndex);
            // call emit again now with correct offset.
            // This backtracks over current code.
            // If/when we vectorize, we can do this in a two-pass arrangement
            (void) emitA(op, src1_copy, 0,
                         (Dyninst::Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                         gen, rc_no_control, noCost);
            // Now we can free the register
            // Dyninst::Register has already been freed; we're just re-using it.
            //gen.rs()->freeRegister(src1);

            gen.setIndex(elseStartIndex);
         }

         if (eoperand) {
            // If there's an else clause, we need to generate code for it.
            gen.tracker()->increaseConditionalLevel();
            if (!eoperand->generateCode_phase2(gen,
                                               noCost,
                                               addr,
                                               src2)) ERROR_RETURN;
            gen.rs()->freeRegister(src2);
            gen.tracker()->decreaseAndClean(gen);
            gen.rs()->unifyTopRegStates(gen); //Join the registerState for the else

            // We also need to fix up the branch at the end of the "true"
            // clause to jump around the "else" clause.
            codeBufIndex_t endIndex = gen.getIndex();
            if (preelse_patches_size != postelse_patches_size) {
               assert(postif_patches_size > preif_patches_size);
               ifTargetPatch else_targ(endIndex + gen.startAddr());
               for (unsigned i=preelse_patches_size; i < postelse_patches_size; i++) {
                  gen.allPatches()[i].setTarget(&else_targ);
               }
               for (unsigned i=preelse_patches_size; i < postelse_patches_size; i++) {
                  gen.allPatches()[i].applyPatch();
               }
            }
            else {
               gen.setIndex(elseSkipIndex);
               emitA(branchOp, 0, 0,
                     (Dyninst::Register) codeGen::getDisplacement(elseSkipStart, endIndex),
                     gen, rc_no_control, noCost);
               gen.setIndex(endIndex);
            }
         }
         retReg = Dyninst::Null_Register;
         break;
      }
      case ifMCOp: {
         assert(gen.point());

         // TODO: Right now we get the condition from the memory access info,
         // because scanning for memory accesses is the only way to detect these
         // conditional instructions. The right way(TM) would be to attach that
         // info directly to the point...
         // Okay. The info we need is stored in the BPatch_point. We have the instPoint.
         // Yay.

         BPatch_addressSpace *bproc = (BPatch_addressSpace *) gen.addrSpace()->up_ptr();
         BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));

         const BPatch_memoryAccess* ma = bpoint->getMemoryAccess();
         assert(ma);
         int cond = ma->conditionCode_NP();
         if(cond > -1) {
            codeBufIndex_t startIndex = gen.getIndex();
            emitJmpMC(cond, 0 /* target, changed later */, gen);
            codeBufIndex_t fromIndex = gen.getIndex();
            // Add the snippet to the tracker, as AM has indicated...
            gen.tracker()->increaseConditionalLevel();
            // generate code with the right path
            if (!loperand->generateCode_phase2(gen,
                                               noCost,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
            gen.tracker()->decreaseAndClean(gen);
            codeBufIndex_t endIndex = gen.getIndex();
            // call emit again now with correct offset.
            gen.setIndex(startIndex);
            emitJmpMC(cond, codeGen::getDisplacement(fromIndex, endIndex), gen);
            gen.setIndex(endIndex);
         }
         else {
            if (!loperand->generateCode_phase2(gen,
                                               noCost,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
         }

         break;
      }
      case whileOp: {
        if(!roperand) { return false; }
        codeBufIndex_t top = gen.getIndex(); 

        // BEGIN from ifOp       
        if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         codeBufIndex_t startIndex= gen.getIndex();

         size_t preif_patches_size = gen.allPatches().size();
         codeBufIndex_t thenSkipStart = emitA(ifOp, src1, 0, 0, gen, rc_before_jump, noCost);

         size_t postif_patches_size = gen.allPatches().size();

	 // We can reuse src1 for the body of the conditional; however, keep the value here
	 // so that we can use it for the branch fix below.
         Dyninst::Register src1_copy = src1;
         gen.rs()->freeRegister(src1);

         // The flow of control forks. We need to add the forked node to
         // the path
         gen.tracker()->increaseConditionalLevel();
         if (!roperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         gen.rs()->freeRegister(src2);
         gen.tracker()->decreaseAndClean(gen);
         gen.rs()->unifyTopRegStates(gen); //Join the registerState for the if
         
         // END from ifOp

         (void) emitA(branchOp, 0, 0, codeGen::getDisplacement(gen.getIndex(), top),
                      gen, rc_no_control, noCost);

         //BEGIN from ifOp

         // Now that we've generated the "then" section, rewrite the if
         // conditional branch.
         codeBufIndex_t elseStartIndex = gen.getIndex();

         if (preif_patches_size != postif_patches_size) {
            assert(postif_patches_size > preif_patches_size);
            ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].setTarget(&if_targ);
            }
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].applyPatch();
            }
         }
         else {
            gen.setIndex(startIndex);
            // call emit again now with correct offset.
            // This backtracks over current code.
            // If/when we vectorize, we can do this in a two-pass arrangement
            (void) emitA(ifOp, src1_copy, 0,
                         (Dyninst::Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                         gen, rc_no_control, noCost);
            // Now we can free the register
            // Dyninst::Register has already been freed; we're just re-using it.
            //gen.rs()->freeRegister(src1);

            gen.setIndex(elseStartIndex);
         }
         // END from ifOp
         retReg = Dyninst::Null_Register;
         break;
      }
      case getAddrOp: {
         switch(loperand->getoType()) {
            case operandType::variableAddr:
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               assert (loperand->getOVar());
               loperand->emitVariableLoad(loadConstOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                          gen.point(), gen.addrSpace());
               break;
            case operandType::variableValue:
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               assert (loperand->getOVar());
               loperand->emitVariableLoad(loadOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                          gen.point(), gen.addrSpace());
               break;
            case operandType::DataAddr:
               {
                  addr = reinterpret_cast<Address>(loperand->getOValue());
                  if (retReg == Dyninst::Null_Register) {
                     retReg = allocateAndKeep(gen, noCost);
                  }
                  assert(!loperand->getOVar());
                  emitVload(loadConstOp, addr, retReg, retReg, gen,
                            noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               }
               break;
            case operandType::FrameAddr: {
               // load the address fp + addr into dest
               if (retReg == Dyninst::Null_Register)
                  retReg = allocateAndKeep(gen, noCost);
               Dyninst::Register temp = gen.rs()->getScratchRegister(gen, noCost);
               addr = (Address) loperand->getOValue();
               emitVload(loadFrameAddr, addr, temp, retReg, gen,
                         noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               break;
            }
            case operandType::RegOffset: {
               assert(loperand->operand());

               // load the address reg + addr into dest
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               addr = (Address) loperand->operand()->getOValue();

               emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), retReg, gen,
                         noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               break;
            }
            case operandType::DataIndir:
               // taking address of pointer de-ref returns the original
               //    expression, so we simple generate the left child's
               //    code to get the address
               if (!loperand->operand()->generateCode_phase2(gen,
                                                             noCost,
                                                             addr,
                                                             retReg)) ERROR_RETURN;
               // Broken refCounts?
               break;
            case operandType::origRegister:
               // Added 2NOV11 Bernat - some variables live in original registers,
               // and so we need to be able to dereference their contents.
               if (!loperand->generateCode_phase2(gen, noCost, addr, retReg)) ERROR_RETURN;
               break;
            default:
               cerr << "Uh oh, unknown loperand type in getAddrOp: " << static_cast<uint64_t>(loperand->getoType()) << endl;
               cerr << "\t Generating ast " << hex << this << dec << endl;
               assert(0);
         }
         break;
      }
      case storeOp: {
        if(!roperand) { return false; }
	bool result = generateOptimizedAssignment(gen, size, noCost);
         if (result)
            break;

         // This ast cannot be shared because it doesn't return a register
         if (!roperand->generateCode_phase2(gen,
                                            noCost,
                                            addr,
                                            src1))  {
            fprintf(stderr, "ERROR: failure generating roperand\n");
            ERROR_RETURN;
         }
         REGISTER_CHECK(src1);
         // We will access loperand's children directly. They do not expect
         // it, so we need to bump up their useCounts
         loperand->fixChildrenCounts();

         src2 = gen.rs()->allocateRegister(gen, noCost);
         switch (loperand->getoType()) {
            case operandType::variableValue:
               loperand->emitVariableStore(storeOp, src1, src2, gen,
                                           noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               loperand->decUseCount(gen);
               break;
            case operandType::DataAddr:
               addr = (Address) loperand->getOValue();
               assert(loperand->getOVar() == NULL);
               emitVstore(storeOp, src1, src2, addr, gen,
                          noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               // We are not calling generateCode for the left branch,
               // so need to decrement the refcount by hand
               loperand->decUseCount(gen);
               break;
            case operandType::FrameAddr:
               addr = (Address) loperand->getOValue();
               emitVstore(storeFrameRelativeOp, src1, src2, addr, gen,
                          noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               loperand->decUseCount(gen);
               break;
            case operandType::RegOffset: {
               assert(loperand->operand());
               addr = (Address) loperand->operand()->getOValue();

               // This is cheating, but I need to pass 4 data values into emitVstore, and
               // it only allows for 3.  Prepare the dest address in scratch register src2.

               emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), src2,
                         gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());

               // Same as DataIndir at this point.
               emitV(storeIndirOp, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               loperand->decUseCount(gen);
               break;
            }
            case operandType::DataIndir: {
               // store to a an expression (e.g. an array or field use)
               // *(+ base offset) = src1
               if (!loperand->operand()->generateCode_phase2(gen,
                                                             noCost,
                                                             addr,
                                                             tmp)) ERROR_RETURN;
               REGISTER_CHECK(tmp);

               // tmp now contains address to store into
               emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               gen.rs()->freeRegister(tmp);
               loperand->decUseCount(gen);
               break;
            }
            case operandType::origRegister:
               gen.rs()->writeProgramRegister(gen, (Dyninst::Register)(long)loperand->getOValue(),
                                              src1, getSize());
               loperand->decUseCount(gen);
               break;
            case operandType::Param:
            case operandType::ParamAtCall:
            case operandType::ParamAtEntry: {
               boost::shared_ptr<AstOperandNode> lnode =
                  boost::dynamic_pointer_cast<AstOperandNode>(loperand);
               emitR(getParamOp, (Address)lnode->oValue,
                     src1, src2, gen, noCost, gen.point(),
                     gen.addrSpace()->multithread_capable());
               loperand->decUseCount(gen);
               break;
            }
            case operandType::ReturnVal:
               emitR(getRetValOp, Dyninst::Null_Register,
                     src1, src2, gen, noCost, gen.point(),
                     gen.addrSpace()->multithread_capable());
               loperand->decUseCount(gen);
               break;
            case operandType::ReturnAddr:
                emitR(getRetAddrOp, Dyninst::Null_Register,
                      src1, src2, gen, noCost, gen.point(),
                      gen.addrSpace()->multithread_capable());
                break;
            default: {
               // Could be an error, could be an attempt to load based on an arithmetic expression
               // Generate the left hand side, store the right to that address
               if (!loperand->generateCode_phase2(gen, noCost, addr, tmp)) ERROR_RETURN;
               REGISTER_CHECK(tmp);

               emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               gen.rs()->freeRegister(tmp);
               break;
            }
         }
         gen.rs()->freeRegister(src1);
         gen.rs()->freeRegister(src2);
         retReg = Dyninst::Null_Register;
         break;
      }
      case storeIndirOp: {
        if(!roperand) { return false; }
         if (!roperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         if (!loperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         REGISTER_CHECK(src2);
         emitV(op, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
         gen.rs()->freeRegister(src1);
         gen.rs()->freeRegister(src2);
         retReg = Dyninst::Null_Register;
         break;
      }
      case plusOp:
      case minusOp:
      case xorOp:
      case timesOp:
      case divOp:
      case orOp:
      case andOp:
      case eqOp:
      case neOp:
      case lessOp:
      case leOp:
      case greaterOp:
      case geOp:
      default:
      {
         if(!roperand) { return false; }
         bool signedOp = IsSignedOperation(loperand->getType(), roperand->getType());
         src1 = Dyninst::Null_Register;
         right_dest = Dyninst::Null_Register;
            if (!loperand->generateCode_phase2(gen,
                                               noCost, addr, src1)) ERROR_RETURN;
            REGISTER_CHECK(src1);

         if ((roperand->getoType() == operandType::Constant) &&
             doNotOverflow((int64_t)roperand->getOValue())) {
            if (retReg == Dyninst::Null_Register) {
               retReg = allocateAndKeep(gen, noCost);
               ast_printf("Operator node, const RHS, allocated register %u\n", retReg.getId());
            }
            else
               ast_printf("Operator node, const RHS, keeping register %u\n", retReg.getId());

            emitImm(op, src1, (RegValue) roperand->getOValue(), retReg, gen, noCost, gen.rs(), signedOp);

            if (src1 != Dyninst::Null_Register)
               gen.rs()->freeRegister(src1);

            // We do not .generateCode for roperand, so need to update its
            // refcounts manually
            roperand->decUseCount(gen);
         }
         else {
               if (!roperand->generateCode_phase2(gen, noCost, addr, right_dest)) ERROR_RETURN;
               REGISTER_CHECK(right_dest);
            if (retReg == Dyninst::Null_Register) {
               retReg = allocateAndKeep(gen, noCost);
            }
            emitV(op, src1, right_dest, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace(), signedOp);
            if (src1 != Dyninst::Null_Register) {
               // Don't free inputs until afterwards; we have _no_ idea
               gen.rs()->freeRegister(src1);
            }
            // what the underlying code might do with a temporary register.
            if (right_dest != Dyninst::Null_Register)
               gen.rs()->freeRegister(right_dest);
         }
      }
   }
	decUseCount(gen);
   return true;
}

bool AstOperandNode::generateCode_phase2(codeGen &gen, bool noCost,
                                         Address &,
                                         Dyninst::Register &retReg) {
	RETURN_KEPT_REG(retReg);


    Address addr = ADDR_NULL;
    Dyninst::Register src = Dyninst::Null_Register;

   // Allocate a register to return
   if (retReg == Dyninst::Null_Register) {
       retReg = allocateAndKeep(gen, noCost);
   }

   Dyninst::Register temp;
   int tSize;
   int len;
   BPatch_type *Type;
   switch (oType) {
#if defined (DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
   case operandType::Constant: {
     assert(oVar == NULL);
     // Move constant into retReg
     Emitter *emitter = gen.emitter();
     const uint32_t immediateValue = (uint32_t)((uint64_t)this->getOValue());
     emitter->emitMovLiteral(retReg, immediateValue, gen);
     break;
   }
#else
   case operandType::Constant:
     assert(oVar == NULL);
     emitVload(loadConstOp, (Address)oValue, retReg, retReg, gen,
		 noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
#endif
   case operandType::DataIndir:
      if (!operand_->generateCode_phase2(gen, noCost, addr, src)) ERROR_RETURN;
      REGISTER_CHECK(src);
      Type = const_cast<BPatch_type *> (getType());
      // Internally generated calls will not have type information set
      if(Type)
         tSize = Type->getSize();
      else
         tSize = sizeof(long);
      emitV(loadIndirOp, src, 0, retReg, gen, noCost, gen.rs(), tSize, gen.point(), gen.addrSpace());
      gen.rs()->freeRegister(src);
      break;
   case operandType::origRegister:
      gen.rs()->readProgramRegister(gen, (Dyninst::Register)(long)oValue, retReg, size);
       //emitLoadPreviousStackFrameRegister((Address) oValue, retReg, gen,
       //size, noCost);
       break;
   case operandType::variableAddr:
     assert(oVar);
     emitVariableLoad(loadConstOp, retReg, retReg, gen,
		 noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
   case operandType::variableValue:
     assert(oVar);
     emitVariableLoad(loadOp, retReg, retReg, gen,
        noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
   case operandType::ReturnVal:
       src = emitR(getRetValOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case operandType::ReturnAddr:
       src = emitR(getRetAddrOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case operandType::Param:
   case operandType::ParamAtCall:
   case operandType::ParamAtEntry: {
       opCode paramOp = undefOp;
       switch(oType) {
           case operandType::Param:
               paramOp = getParamOp;
               break;
           case operandType::ParamAtCall:
               paramOp = getParamAtCallOp;
               break;
           case operandType::ParamAtEntry:
               paramOp = getParamAtEntryOp;
               break;
           default:
               assert(0);
               break;
       }
       src = emitR(paramOp, (Address)oValue, Dyninst::Null_Register,
                   retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       }
       break;
   case operandType::DataAddr:
       assert(oVar == NULL);
       addr = reinterpret_cast<Address>(oValue);
       emitVload(loadOp, addr, retReg, retReg, gen, noCost, NULL, size, gen.point(), gen.addrSpace());
       break;
   case operandType::FrameAddr:
       addr = (Address) oValue;
       temp = gen.rs()->allocateRegister(gen, noCost);
       emitVload(loadFrameRelativeOp, addr, temp, retReg, gen, noCost, gen.rs(),
               size, gen.point(), gen.addrSpace());
       gen.rs()->freeRegister(temp);
       break;
   case operandType::RegOffset:
       // Prepare offset from value in any general register (not just fp).
       // This AstNode holds the register number, and loperand holds offset.
       assert(operand_);
       addr = (Address) operand_->getOValue();
       emitVload(loadRegRelativeOp, addr, (long)oValue, retReg, gen, noCost,
               gen.rs(), size, gen.point(), gen.addrSpace());
       break;
   case operandType::ConstantString:
       // XXX This is for the std::string type.  If/when we fix the std::string type
       // to make it less of a hack, we'll need to change this.
       len = strlen((char *)oValue) + 1;

       addr = (Address) gen.addrSpace()->inferiorMalloc(len, dataHeap); //dataheap

       if (!gen.addrSpace()->writeDataSpace((char *)addr, len, (char *)oValue)) {
           ast_printf("Failed to write string constant into mutatee\n");
           return false;
       }

       if(!gen.addrSpace()->needsPIC())
       {
          emitVload(loadConstOp, addr, retReg, retReg, gen, noCost, gen.rs(),
                  size, gen.point(), gen.addrSpace());
       }
       else
       {
          gen.codeEmitter()->emitLoadShared(loadConstOp, retReg, NULL, true, size, gen, addr);
       }
       break;
   default:
       fprintf(stderr, "[%s:%d] ERROR: Unknown operand type %d in AstOperandNode generation\n",
               __FILE__, __LINE__, static_cast<int>(oType));
       return false;
       break;
   }
	decUseCount(gen);
   return true;
}

bool AstMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                        Address &,
                                        Dyninst::Register &retReg) {

	RETURN_KEPT_REG(retReg);

    const BPatch_memoryAccess* ma;
    const BPatch_addrSpec_NP *start;
    const BPatch_countSpec_NP *count;
    if (retReg == Dyninst::Null_Register)
        retReg = allocateAndKeep(gen, noCost);
    switch(mem_) {
    case EffectiveAddr: {

        // VG(11/05/01): get effective address
        // VG(07/31/02): take care which one
        // 1. get the point being instrumented & memory access info
        assert(gen.point());

        BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
        BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
        if (bpoint == NULL) {
            fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
                    (void*)gen.point(), gen.point()->insnAddr());
        }
        assert(bpoint);
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal( "Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
            assert(0);
        }
        if(which_ >= ma->getNumberOfAccesses()) {
            bpfatal( "Attempt to instrument non-existent memory access number.\n");
            bpfatal( "Consider using filterPoints()...\n");
            assert(0);
        }
        start = ma->getStartAddr(which_);
        emitASload(start, retReg, 0, gen, noCost);
        break;
    }
    case BytesAccessed: {
        // 1. get the point being instrumented & memory access info
        assert(gen.point());

        BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
        BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal("Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
            assert(0);
        }
        if(which_ >= ma->getNumberOfAccesses()) {
            bpfatal( "Attempt to instrument non-existent memory access number.\n");
            bpfatal( "Consider using filterPoints()...\n");
            assert(0);
        }
        count = ma->getByteCount(which_);
        emitCSload(count, retReg, gen, noCost);
        break;
    }
    default:
        assert(0);
    }
	decUseCount(gen);
    return true;
}

bool AstCallNode::initRegisters(codeGen &gen) {
    // For now, we only care if we should save everything. "Everything", of course,
    // is platform dependent. This is the new location of the clobberAllFuncCalls
    // that had previously been in emitCall.

    bool ret = true;

    // First, check children
    for (unsigned i = 0; i < children.size(); i++) {
        if (!children[i]->initRegisters(gen))
            ret = false;
    }

    if (callReplace_) return true;
    
    // We also need a function object.
    func_instance *callee = func_;
    if (!callee) {
        // Painful lookup time
        callee = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
    }
    assert(callee);

    // Marks registers as used based on the callee's behavior
    // This means we'll save them if necessary (also, lets us use
    // them in our generated code because we've saved, instead
    // of saving others).
    assert(gen.codeEmitter());
    gen.codeEmitter()->clobberAllFuncCall(gen.rs(), callee);

    return ret;

}

bool AstCallNode::generateCode_phase2(codeGen &gen, bool noCost,
                                      Address &,
                                      Dyninst::Register &retReg) {
	// We call this anyway... not that we'll ever be kept.
	// Well... if we can somehow know a function is entirely
	// dependent on arguments (a flag?) we can keep it around.
	RETURN_KEPT_REG(retReg);

    // VG(11/06/01): This platform independent fn calls a platfrom
    // dependent fn which calls it back for each operand... Have to
    // fix those as well to pass location...

    func_instance *use_func = func_;

    if (!use_func && !func_addr_) {
        // We purposefully don't cache the func_instance object; the AST nodes
        // are process independent, and functions kinda are.
        use_func = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
        if (!use_func) {
            fprintf(stderr, "ERROR: failed to find function %s, unable to create call\n",
                    func_name_.c_str());
        }
        assert(use_func); // Otherwise we've got trouble...
    }

    Dyninst::Register tmp = 0;

    if (use_func && !callReplace_) {
        tmp = emitFuncCall(callOp, gen, children,
                           noCost, use_func);
    }
    else if (use_func && callReplace_) {
	tmp = emitFuncCall(funcJumpOp, gen, children,
                           noCost, use_func);
    }
    else {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
                __FILE__, __LINE__, func_name_.c_str());
        showErrorCallback(100, msg);
        assert(0);  // can probably be more graceful
    }

	// TODO: put register allocation here and have emitCall just
	// move the return result.
    if (tmp == Dyninst::Null_Register) {
        // Happens in function replacement... didn't allocate
        // a return register.
    }
    else if (retReg == Dyninst::Null_Register) {
        //emitFuncCall allocated tmp; we can use it, but let's see
        // if we should keep it around.
        retReg = tmp;
        // from allocateAndKeep:
        if (useCount > 1) {
            // If use count is 0 or 1, we don't want to keep
            // it around. If it's > 1, then we can keep the node
            // (by construction) and want to since there's another
            // use later.
            gen.tracker()->addKeptRegister(gen, this, retReg);
        }
    }
    else if (retReg != tmp) {
        emitImm(orOp, tmp, 0, retReg, gen, noCost, gen.rs());
        gen.rs()->freeRegister(tmp);
    }
    decUseCount(gen);
    return true;
}

bool AstSequenceNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &,
                                          Dyninst::Register &retReg) {
    RETURN_KEPT_REG(retReg);
    Dyninst::Register tmp = Dyninst::Null_Register;
    Address unused = ADDR_NULL;

    if (children.size() == 0) {
      // Howzat???
      return true;
    }

    for (unsigned i = 0; i < children.size() - 1; i++) {
      if (!children[i]->generateCode_phase2(gen,
                                               noCost,
                                               unused,
                                               tmp)) ERROR_RETURN;
        gen.rs()->freeRegister(tmp);
        tmp = Dyninst::Null_Register;
    }

    // We keep the last one
    if (!children.back()->generateCode_phase2(gen, noCost, unused, retReg)) ERROR_RETURN;

	decUseCount(gen);

    return true;
}

bool AstVariableNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &addr,
                                          Dyninst::Register &retReg) {
    return children[index]->generateCode_phase2(gen, noCost, addr, retReg);
}

bool AstOriginalAddrNode::generateCode_phase2(codeGen &gen,
                                              bool noCost,
                                              Address &,
                                              Dyninst::Register &retReg) {
    RETURN_KEPT_REG(retReg);
    if (retReg == Dyninst::Null_Register) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == Dyninst::Null_Register) return false;

    emitVload(loadConstOp,
              (Address) gen.point()->addr_compat(),
              retReg, retReg, gen, noCost);
    return true;
}

bool AstActualAddrNode::generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            Address &,
                                            Dyninst::Register &retReg) {
    if (retReg == Dyninst::Null_Register) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == Dyninst::Null_Register) return false;

    emitVload(loadConstOp,
              (Address) gen.currAddr(),
              retReg, retReg,
              gen, noCost);

    return true;
}

bool AstDynamicTargetNode::generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            Address & retAddr,
                                            Dyninst::Register &retReg)
{
    if (gen.point()->type() != instPoint::PreCall &&
       gen.point()->type() != instPoint::FuncExit &&
       gen.point()->type() != instPoint::PreInsn)
       return false;

   InstructionAPI::Instruction insn = gen.point()->block()->getInsn(gen.point()->block()->last());
   if (insn.isReturn()) {
      // if this is a return instruction our AST reads the top stack value
      if (retReg == Dyninst::Null_Register) {
         retReg = allocateAndKeep(gen, noCost);
      }
      if (retReg == Dyninst::Null_Register) return false;

#if defined(DYNINST_CODEGEN_ARCH_I386)
        emitVload(loadRegRelativeOp,
                  (Address)0,
                  REGNUM_ESP,
                  retReg,
                  gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_X86_64)
        emitVload(loadRegRelativeOp,
                  (Address)0,
                  REGNUM_RSP,
                  retReg,
                  gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_POWER) // KEVINTODO: untested
        emitVload(loadRegRelativeOp,
                  (Address) sizeof(Address),
                  REG_SP,
                  retReg,
                  gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
			//#warning "This function is not implemented yet!"
			assert(0);
#else
        assert(0);
#endif
      return true;
   }
   else {// this is a dynamic ctrl flow instruction, have
      // getDynamicCallSiteArgs generate the necessary AST
      std::vector<AstNodePtr> args;
      if (!gen.addrSpace()->getDynamicCallSiteArgs(insn, gen.point()->block()->last(), args)) {
         return false;
      }
      if (!args[0]->generateCode_phase2(gen, noCost, retAddr, retReg)) {
         return false;
      }
      return true;
   }
}

bool AstScrambleRegistersNode::generateCode_phase2(codeGen &gen,
 						  bool ,
						  Address&,
						  Dyninst::Register& )
{
   (void)gen; // unused
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
   for (int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if (reg->encoding() != REGNUM_RBP && reg->encoding() != REGNUM_RSP)
          gen.codeEmitter()->emitLoadConst(reg->encoding() , -1, gen);
   }
#endif
   return true;
}

BPatch_type *AstOperatorNode::checkType(BPatch_function* func) {
    BPatch_type *ret = NULL;
    BPatch_type *lType = NULL, *rType = NULL, *eType = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    if ((loperand || roperand) && getType()) {
	// something has already set the type for us.
	// this is likely an expression for array access
       ret = const_cast<BPatch_type *>(getType());
       return ret;
    }

    if (loperand) lType = loperand->checkType(func);

    if (roperand) rType = roperand->checkType(func);

    if (eoperand) eType = eoperand->checkType(func);
    (void)eType; // unused...

    if (lType == BPatch::bpatch->type_Error ||
        rType == BPatch::bpatch->type_Error)
       errorFlag = true;

    switch (op) {
    case ifOp:
    case whileOp:
        // XXX No checking for now.  Should check that loperand
        // is boolean.
        ret = BPatch::bpatch->type_Untyped;
        break;
    case noOp:
        ret = BPatch::bpatch->type_Untyped;
        break;
    case funcJumpOp:
        ret = BPatch::bpatch->type_Untyped;
        break;
    case getAddrOp:
        // Should set type to the infered type not just void *
        //  - jkh 7/99
        ret = BPatch::bpatch->stdTypes->findType("void *");
        assert(ret != NULL);
        break;
    default:
        // XXX The following line must change to decide based on the
        // types and operation involved what the return type of the
        // expression will be.
        ret = lType;
        if (lType != NULL && rType != NULL) {
            if (!lType->isCompatible(rType)) {
                fprintf(stderr, "WARNING: LHS type %s not compatible with RHS type %s\n",
                        lType->getName(), rType->getName());
                errorFlag = true;
            }
        }
        break;
    }
    assert (ret != NULL);

    if (errorFlag && doTypeCheck) {
       ret = BPatch::bpatch->type_Error;
    } else if (errorFlag) {
       ret = BPatch::bpatch->type_Untyped;
    }

    // remember what type we are
    setType(ret);

    return ret;
}

BPatch_type *AstOperandNode::checkType(BPatch_function* func)
{
    BPatch_type *ret = NULL;
    BPatch_type *type = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    if (operand_ && getType()) {
       // something has already set the type for us.
       // this is likely an expression for array access
       ret = const_cast<BPatch_type *>(getType());
       return ret;
    }

    if (operand_) type = operand_->checkType(func);

    if (type == BPatch::bpatch->type_Error)
       errorFlag = true;

    if (oType == operandType::DataIndir) {
        // XXX Should really be pointer to lType -- jkh 7/23/99
        ret = BPatch::bpatch->type_Untyped;
    }
    else if ((oType == operandType::Param) || (oType == operandType::ParamAtCall) ||
             (oType == operandType::ParamAtEntry) || (oType == operandType::ReturnVal)
             || (oType == operandType::ReturnAddr)) {
      if(func)
      {
	switch(oType)
	{
	case operandType::ReturnVal:
	  {
	    ret = func->getReturnType();
	    if(!ret || (ret->isCompatible(BPatch::bpatch->builtInTypes->findBuiltInType("void")))) {
		  if(ret) {
		      errorFlag = true;
		  }
	      ret = BPatch::bpatch->type_Untyped;
	    }
	    break;
	  }
	default:
	  ret = BPatch::bpatch->type_Untyped;
	}

      }
      else
      {
	// If we don't have a function context, then ignore types
        ret = BPatch::bpatch->type_Untyped;
      }
    }
    else if (oType == operandType::origRegister) {
        ret = BPatch::bpatch->type_Untyped;
    }
    else {
        ret = const_cast<BPatch_type *>(getType());
    }
    assert(ret != NULL);

    if (errorFlag && doTypeCheck) {
       ret = BPatch::bpatch->type_Error;
    } else if (errorFlag) {
       ret = BPatch::bpatch->type_Untyped;
    }

    // remember what type we are
    setType(ret);

    return ret;

}


BPatch_type *AstCallNode::checkType(BPatch_function* func) {
    BPatch_type *ret = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    unsigned i;
    for (i = 0; i < children.size(); i++) {
        BPatch_type *opType = children[i]->checkType(func);
        /* XXX Check operands for compatibility */
        if (opType == BPatch::bpatch->type_Error) {
            errorFlag = true;
        }
    }
    /* XXX Should set to return type of function. */
    ret = BPatch::bpatch->type_Untyped;

    assert(ret != NULL);

    if (errorFlag && doTypeCheck) {
       ret = BPatch::bpatch->type_Error;
    } else if (errorFlag) {
       ret = BPatch::bpatch->type_Untyped;
    }

    // remember what type we are
    setType(ret);

    return ret;
}

BPatch_type *AstSequenceNode::checkType(BPatch_function* func) {
    BPatch_type *ret = NULL;
    BPatch_type *sType = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    if (getType()) {
	// something has already set the type for us.
	// this is likely an expression for array access
	ret = const_cast<BPatch_type *>(getType());
	return ret;
    }

    for (unsigned i = 0; i < children.size(); i++) {
        sType = children[i]->checkType(func);
        if (sType == BPatch::bpatch->type_Error)
            errorFlag = true;
    }

    ret = sType;

    assert(ret != NULL);

    if (errorFlag && doTypeCheck) {
	ret = BPatch::bpatch->type_Error;
    } else if (errorFlag) {
	ret = BPatch::bpatch->type_Untyped;
    }

    // remember what type we are
    setType(ret);

    return ret;
}

// Check if the node can be kept at all. Some nodes (e.g., storeOp)
// can not be cached. In fact, there are fewer nodes that can be cached.
bool AstOperatorNode::canBeKept() const {
    switch (op) {
    case plusOp:
    case minusOp:
    case xorOp:
    case timesOp:
    case divOp:
    case neOp:
    case noOp:
    case orOp:
    case andOp:
		break;
    default:
        return false;
    }

    // The switch statement is a little odd, but hey.
    if (loperand && !loperand->canBeKept()) return false;
    if (roperand && !roperand->canBeKept()) return false;
    if (eoperand && !eoperand->canBeKept()) return false;

    return true;
}

bool AstOperandNode::canBeKept() const {

    switch (oType) {
    case operandType::DataIndir:
    case operandType::RegOffset:
    case operandType::origRegister:
    case operandType::DataAddr:
    case operandType::variableValue:
        return false;
    default:
		break;
    }
    if (operand_ && !operand_->canBeKept()) return false;
    return true;
}

bool AstCallNode::canBeKept() const {
    if (constFunc_) {
        for (unsigned i = 0; i < children.size(); i++) {
            if (!children[i]->canBeKept()) {
                fprintf(stderr, "AST %p: labelled const func but argument %u cannot be kept!\n",
                        (const void*)this, i);
                return false;
            }
        }
        return true;
    }
    return false;

}

void AstVariableNode::setVariableAST(codeGen &gen){
    if(!ranges_)
        return;
    if(!gen.point())    //oneTimeCode. Set the AST at the beginning of the function??
    {
        index = 0;
        return;
    }
    Address addr = gen.point()->addr_compat();     //Dyninst::Offset of inst point from function base address
    bool found = false;
    for(unsigned i=0; i< ranges_->size();i++){
       if((*ranges_)[i].first<=addr && addr<=(*ranges_)[i].second) {
          index = i;
          found = true;
       }
    }
    if (!found) {
       cerr << "Error: unable to find AST representing variable at " << hex << addr << dec << endl;
       cerr << "Pointer " << hex << this << dec << endl;
       cerr << "Options are: " << endl;
       for(unsigned i=0; i< ranges_->size();i++){
          cerr << "\t" << hex << (*ranges_)[i].first << "-" << (*ranges_)[i].second << dec << endl;
       }
    }
    assert(found);
}

bool AstOperandNode::usesAppRegister() const {
   if (oType == operandType::FrameAddr ||
       oType == operandType::RegOffset ||
       oType == operandType::origRegister ||
       oType == operandType::Param ||
       oType == operandType::ParamAtEntry ||
       oType == operandType::ParamAtCall ||
       oType == operandType::ReturnVal)
   {
      return true;
   }

   if (operand_ && operand_->usesAppRegister()) return true;
   return false;
}

int_variable* AstOperandNode::lookUpVar(AddressSpace* as)
{
  mapped_module *mod = as->findModule(oVar->pdmod()->fileName());
  if(mod && mod->obj())// && (oVar->pdmod() == mod->pmod()))
  {
      int_variable* tmp = mod->obj()->findVariable(const_cast<image_variable*>(oVar));
      return tmp;
  }
  return NULL;
}

void AstOperandNode::emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen& gen,
				      bool noCost, registerSpace* rs,
				      int size_, const instPoint* point, AddressSpace* as)
{
  int_variable* var = lookUpVar(as);
  if(var && !as->needsPIC(var))
  {
    emitVload(op, var->getAddress(), src2, dest, gen, noCost, rs, size_, point, as);
  }
  else
  {
    gen.codeEmitter()->emitLoadShared(op, dest, oVar, (var!=NULL),size_, gen, 0);
  }
}

void AstOperandNode::emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen& gen,
				      bool noCost, registerSpace* rs,
				      int size_, const instPoint* point, AddressSpace* as)
{
  int_variable* var = lookUpVar(as);
  if (var && !as->needsPIC(var))
  {
    emitVstore(op, src1, src2, var->getAddress(), gen, noCost, rs, size_, point, as);
  }
  else
  {
    gen.codeEmitter()->emitStoreShared(src1, oVar, (var!=NULL), size_, gen);
  }
}

bool AstSnippetNode::generateCode_phase2(codeGen &gen,
                                         bool,
                                         Address &,
                                         Dyninst::Register &) {
   Buffer buf(gen.currAddr(), 1024);
   if (!snip_->generate(gen.point(), buf)) return false;
   gen.copy(buf.start_ptr(), buf.size());
   return true;
}

std::string AstStackGenericNode::format(std::string indent) {
    std::stringstream ret;
    ret << indent << "StackGeneric/" << hex << this;
    ret << endl;
    return ret.str();
}

std::string AstOperatorNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Op/" << hex << this << dec << "(" << format_opcode(op) << ")" << endl;
   if (loperand) ret << indent << loperand->format(indent + "  ");
   if (roperand) ret << indent << roperand->format(indent + "  ");
   if (eoperand) ret << indent << eoperand->format(indent + "  ");

   return ret.str();
}

std::string AstOperandNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Oper/" << hex << this << dec << "(" << format_operand(oType) << "/" << oValue << ")" << endl;
   if (operand_) ret << indent << operand_->format(indent + "  ");

   return ret.str();
}


std::string AstCallNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Call/" << hex << this << dec;
   if (func_) ret << "(" << func_->name() << ")";
   else ret << "(" << func_name_ << ")";
   ret << endl;
   indent += "  ";
   for (unsigned i = 0; i < children.size(); ++i) {
      ret << indent << children[i]->format(indent + "  ");
   }

   return ret.str();
}

std::string AstSequenceNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Seq/" << hex << this << dec << "()" << endl;
   for (unsigned i = 0; i < children.size(); ++i) {
      ret << indent << children[i]->format(indent + "  ");
   }
   return ret.str();
}


std::string AstVariableNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Var/" << hex << this << dec << "(" << children.size() << ")" << endl;
   for (unsigned i = 0; i < children.size(); ++i) {
      ret << indent << children[i]->format(indent + "  ");
   }

   return ret.str();
}

std::string AstMemoryNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Mem/" << hex << this << dec << "("
       << ((mem_ == EffectiveAddr) ? "EffAddr" : "BytesAcc")
       << ")" << endl;

   return ret.str();
}

bool AstOperandNode::initRegisters(codeGen &g) {
    bool ret = true;
    for (unsigned i = 0; i < children.size(); i++) {
        if (!children[i]->initRegisters(g))
            ret = false;
    }

    // If we're an origRegister, override its state as live.
    if (oType == operandType::origRegister) {
       Address origReg = (Address) oValue;
       // Mark that register as live so we are sure to save it.
       registerSlot *r = (*(g.rs()))[origReg];
       r->liveState = registerSlot::live;
    }

    return ret;
}

AstAtomicOperationStmtNode::AstAtomicOperationStmtNode(opCode astOpcode, AstNodePtr variableNode,
                                                       AstNodePtr constantNode)
    : opcode(astOpcode), variable(variableNode), constant(constantNode) {}

std::string AstAtomicOperationStmtNode::format(std::string indent) {
    std::stringstream ret;
    ret << indent << "Op/" << hex << this << dec << "("
        << "atomic " << format_opcode(opcode) << ")" << endl;
    if (variable)
       ret << indent << variable->format(indent + "  ");
    if (constant)
       ret << indent << constant->format(indent + "  ");
    return ret.str();
}

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
bool AstAtomicOperationStmtNode::generateCode_phase2(codeGen &gen, bool noCost, Address &retAddr,
                                                     Dyninst::Register & /* retReg */) {
  // This has 2 operands - variable and constant.
  // Codegen for atomic add has the following steps:
  // 1. Evaluate constant -- load a register with the constant
  // 2. Evaluate variable -- load a register pair with the address of the variable.
  // 3. Emit s_atomic_add instruction - atomically add the constant to the variable.

  bool ret = true;

  Register src0 = Dyninst::Null_Register;
  if (!constant->generateCode_phase2(gen, noCost, retAddr, src0)) {
    fprintf(stderr, "WARNING: failed in generateCode internals!\n");
    ret = false;
  }
  assert(src0 != Dyninst::Null_Register);

  // Now generate code for the variable -- load a register pair with the address of the variable.
  AstOperandNode *variableOperand = dynamic_cast<AstOperandNode *>((AstNode *)variable.get());
  assert(variableOperand);
  assert(variableOperand->getoType() == operandType::AddressAsPlaceholderRegAndOffset);

  AstOperandNode *offset =
      dynamic_cast<AstOperandNode *>((AstNode *)variableOperand->operand().get());
  assert(offset);

  EmitterAmdgpuGfx908 *emitter = dynamic_cast<EmitterAmdgpuGfx908 *>(gen.emitter());
  assert(emitter);

  // TODO : Remove all hardcoded registers.
  emitter->emitMoveRegToReg(94, 88, gen);
  emitter->emitMoveRegToReg(95, 89, gen);
  emitter->emitAddConstantToRegPair(88, (Address)offset->getOValue(), gen);

  // Now we have s[88:89] with address of the variable. Emit appropriate atomic instruction.
  switch (opcode) {
  case plusOp:
    emitter->emitAtomicAdd(88, src0, gen);
    break;
  case minusOp:
    emitter->emitAtomicSub(88, src0, gen);
    break;
  default:
    assert(!"atomic operation for this opcode is not implemented");
  }

  return ret;
}
#else
bool AstAtomicOperationStmtNode::generateCode_phase2(codeGen & /* gen */, bool /* noCost */,
                                                     Address & /* retAddr */,
                                                     Dyninst::Register & /* retReg */) {
    cerr << "AstAtomicOperationStmtNode::generateCode_phase2 not implemented" << endl;
    return false;
}
#endif

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
int AstOperandNode::lastOffset = 0;
std::map<std::string, int> AstOperandNode::allocTable = {{"--init--", -1}};
#endif
