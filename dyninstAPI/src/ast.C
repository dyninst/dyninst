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

bool AstSnippetNode::generateCode_phase2(codeGen &gen,
                                         bool,
                                         Address &,
                                         Dyninst::Register &) {
   Buffer buf(gen.currAddr(), 1024);
   if (!snip_->generate(gen.point(), buf)) return false;
   gen.copy(buf.start_ptr(), buf.size());
   return true;
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

