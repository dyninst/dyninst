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

bool AstSnippetNode::generateCode_phase2(codeGen &gen,
                                         bool,
                                         Address &,
                                         Dyninst::Register &) {
   Buffer buf(gen.currAddr(), 1024);
   if (!snip_->generate(gen.point(), buf)) return false;
   gen.copy(buf.start_ptr(), buf.size());
   return true;
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

