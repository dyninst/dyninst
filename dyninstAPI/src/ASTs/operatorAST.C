#include "ast_helpers.h"
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_collections.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_point.h"
#include "codegen.h"
#include "debug.h"
#include "dyntypes.h"
#include "mapped_object.h"
#include "operatorAST.h"
#include "registerSpace.h"

#include <iomanip>
#include <sstream>

namespace {
  bool isPowerOf2(Dyninst::Address addr) {
    if(addr == Dyninst::ADDR_NULL) {
      return false;
    }

    auto const x = static_cast<uint32_t>(addr);
    return (x & (x - 1UL)) == 0UL;
  }

  bool IsSignedOperation(BPatch_type *l, BPatch_type *r) {
    if(l == NULL || r == NULL) {
      return true;
    }
    if(strstr(l->getName(), "unsigned") == NULL) {
      return true;
    }
    if(strstr(r->getName(), "unsigned") == NULL) {
      return true;
    }
    return false;
  }
}

namespace Dyninst { namespace DyninstAPI {

operatorAST::operatorAST(opCode opC, codeGenASTPtr l, codeGenASTPtr r, codeGenASTPtr e)
    : codeGenAST(), op(opC), loperand(l), roperand(r), eoperand(e) {
  // Optimization pass...
  if(!loperand) {
    return;
  }
  if(roperand) {
    if(op == plusOp) {
      if(loperand->getoType() == operandType::Constant) {
        // Swap left and right...
        codeGenASTPtr temp = loperand;
        loperand = roperand;
        roperand = temp;
      }
    }
    if(op == timesOp) {
      if(roperand->getoType() == operandType::undefOperandType) {
        // ...
      } else if(roperand->getoType() != operandType::Constant) {
        codeGenASTPtr temp = roperand;
        roperand = loperand;
        loperand = temp;
      } else {
        if(!isPowerOf2((Address)roperand->getOValue()) &&
           isPowerOf2((Address)loperand->getOValue())) {
          codeGenASTPtr temp = roperand;
          roperand = loperand;
          loperand = temp;
        }
      }
    }
  }

  if(loperand) {
    children.push_back(loperand);
  }
  if(roperand) {
    children.push_back(roperand);
  }
  if(eoperand) {
    children.push_back(eoperand);
  }
}

bool operatorAST::initRegisters(codeGen &g) {
  bool ret = true;
  for(unsigned i = 0; i < children.size(); i++) {
    if(!children[i]->initRegisters(g)) {
      ret = false;
    }
  }

#if !defined(DYNINST_CODEGEN_ARCH_I386)
  // Override: if we're trying to save to an original
  // register, make sure it's saved on the stack.
  if(loperand) {
    if(op == storeOp) {
      if(loperand->getoType() == operandType::origRegister) {
        Dyninst::Address origReg = (Dyninst::Address)loperand->getOValue();
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

bool operatorAST::generateOptimizedAssignment(codeGen &gen, int size_, bool noCost) {
  (void)size_;
  if(!(loperand && roperand)) {
    return false;
  }

  // Recognize the common case of 'a = a op constant' and try to
  //  generate optimized code for this case.
  Dyninst::Address laddr;

  if(loperand->getoType() == operandType::DataAddr) {
    laddr = (Dyninst::Address)loperand->getOValue();
  } else {
    if(loperand->getoType() == operandType::variableValue) {
      boost::shared_ptr<operandAST> lnode =
          boost::dynamic_pointer_cast<operandAST>(loperand);

      int_variable *var = lnode->lookUpVar(gen.addrSpace());
      if(!var || gen.addrSpace()->needsPIC(var)) {
        return false;
      }
      laddr = var->getAddress();
    } else {
      // Deal with global writes for now.
      return false;
    }
  }

  if(roperand->getoType() == operandType::Constant) {
    // Looks like 'global = constant'
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
    if(laddr >> 32 || ((Dyninst::Address)roperand->getOValue()) >> 32 || size_ == 8) {
      // Make sure value and address are 32-bit values.
      return false;
    }

#endif
    int imm = (int)(long)roperand->getOValue();
    emitStoreConst(laddr, (int)imm, gen, noCost);
    loperand->decUseCount(gen);
    roperand->decUseCount(gen);
    return true;
  }

  operatorAST *roper = dynamic_cast<operatorAST *>(roperand.get());
  if(!roper) {
    return false;
  }

  if(roper->op != plusOp && roper->op != minusOp) {
    return false;
  }

  operandAST *arithl = dynamic_cast<operandAST *>(roper->loperand.get());
  operandAST *arithr = dynamic_cast<operandAST *>(roper->roperand.get());
  if(!arithl || !arithr) {
    return false;
  }

  codeGenAST *const_oper = NULL;
  if(arithl->getoType() == operandType::DataAddr && arithr->getoType() == operandType::Constant &&
     laddr == (Dyninst::Address)arithl->getOValue()) {
    const_oper = arithr;
  } else if(arithl->getoType() == operandType::variableValue &&
            arithr->getoType() == operandType::Constant) {
    Dyninst::Address addr = 0;
    int_variable *var = arithl->lookUpVar(gen.addrSpace());
    if(!var || gen.addrSpace()->needsPIC(var)) {
      return false;
    }
    addr = var->getAddress();
    if(addr == laddr) {
      const_oper = arithr;
    }
  } else if(arithr->getoType() == operandType::DataAddr &&
            arithl->getoType() == operandType::Constant &&
            laddr == (Dyninst::Address)arithr->getOValue() && roper->op == plusOp) {
    const_oper = arithl;
  } else if(arithl->getoType() == operandType::variableValue &&
            arithr->getoType() == operandType::Constant) {
    Dyninst::Address addr = 0;
    int_variable *var = arithl->lookUpVar(gen.addrSpace());
    if(!var || gen.addrSpace()->needsPIC(var)) {
      return false;
    }
    addr = var->getAddress();
    if(addr == laddr) {
      const_oper = arithl;
    }
  } else {
    return false;
  }

  long int imm = (long int)const_oper->getOValue();
  if(roper->op == plusOp) {
    emitAddSignedImm(laddr, imm, gen, noCost);
  } else {
    emitSubSignedImm(laddr, imm, gen, noCost);
  }

  loperand->decUseCount(gen);
  roper->roperand->decUseCount(gen);
  roper->loperand->decUseCount(gen);
  roper->decUseCount(gen);

  return true;
}
#else
bool operatorAST::generateOptimizedAssignment(codeGen &, int, bool) {
  return false;
}
#endif

bool operatorAST::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                                          Dyninst::Register &retReg) {
  if(!loperand) {
    return false;
  }

  retAddr = ADDR_NULL; // We won't be setting this...
                       // retReg may have a value or be the (register) equivalent of NULL.
                       // In either case, we don't touch it...

  RETURN_KEPT_REG(retReg);

  Dyninst::Address addr = ADDR_NULL;

  Dyninst::Register src1 = Dyninst::Null_Register;
  Dyninst::Register src2 = Dyninst::Null_Register;

  Dyninst::Register right_dest = Dyninst::Null_Register;
  Dyninst::Register tmp = Dyninst::Null_Register;

  switch(op) {
    case branchOp: {
      assert(loperand->getoType() == operandType::Constant);
      unsigned offset = (Dyninst::Register)(long)loperand->getOValue();
      // We are not calling loperand->generateCode_phase2,
      // so we decrement its useCount by hand.
      // Would be nice to allow register branches...
      loperand->decUseCount(gen);
      (void)emitA(branchOp, 0, 0, (Dyninst::Register)offset, gen, rc_no_control, noCost);
      retReg = Dyninst::Null_Register; // No return register
      break;
    }
    case ifOp: {
      if(!roperand) {
        return false;
      }
      // This ast cannot be shared because it doesn't return a register
      if(!loperand->generateCode_phase2(gen, noCost, addr, src1)) {
        ERROR_RETURN;
      }
      REGISTER_CHECK(src1);
      codeBufIndex_t ifIndex = gen.getIndex();

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
      if(!roperand->generateCode_phase2(gen, noCost, addr, src2)) {
        ERROR_RETURN;
      }
      gen.rs()->freeRegister(src2);
      gen.tracker()->decreaseAndClean(gen);
      gen.rs()->unifyTopRegStates(gen); // Join the registerState for the if

      // Is there an else clause?  If yes, generate branch over it
      codeBufIndex_t elseSkipStart = 0;
      codeBufIndex_t elseSkipIndex = gen.getIndex();
      size_t preelse_patches_size = 0, postelse_patches_size = 0;
      if(eoperand) {
        gen.rs()->pushNewRegState(); // Create registerState for else
        preelse_patches_size = gen.allPatches().size();
        elseSkipStart = emitA(branchOp, 0, 0, 0, gen, rc_no_control, noCost);
        postelse_patches_size = gen.allPatches().size();
      }

      // Now that we've generated the "then" section, rewrite the if
      // conditional branch.
      codeBufIndex_t elseStartIndex = gen.getIndex();

      if(preif_patches_size != postif_patches_size) {
        assert(postif_patches_size > preif_patches_size);
        ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
        for(unsigned i = preif_patches_size; i < postif_patches_size; i++) {
          gen.allPatches()[i].setTarget(&if_targ);
        }
        for(unsigned i = preif_patches_size; i < postif_patches_size; i++) {
          gen.allPatches()[i].applyPatch();
        }
      } else {
        gen.setIndex(ifIndex);
        // call emit again now with correct offset.
        // This backtracks over current code.
        // If/when we vectorize, we can do this in a two-pass arrangement
        (void)emitA(op, src1_copy, 0,
                    (Dyninst::Register)codeGen::getDisplacement(thenSkipStart, elseStartIndex), gen,
                    rc_no_control, noCost);
        // Now we can free the register
        // Dyninst::Register has already been freed; we're just re-using it.
        // gen.rs()->freeRegister(src1);

        gen.setIndex(elseStartIndex);
      }

      if(eoperand) {
        // If there's an else clause, we need to generate code for it.
        gen.tracker()->increaseConditionalLevel();
        if(!eoperand->generateCode_phase2(gen, noCost, addr, src2)) {
          ERROR_RETURN;
        }
        gen.rs()->freeRegister(src2);
        gen.tracker()->decreaseAndClean(gen);
        gen.rs()->unifyTopRegStates(gen); // Join the registerState for the else

        // We also need to fix up the branch at the end of the "true"
        // clause to jump around the "else" clause.
        codeBufIndex_t endIndex = gen.getIndex();
        if(preelse_patches_size != postelse_patches_size) {
          assert(postif_patches_size > preif_patches_size);
          ifTargetPatch else_targ(endIndex + gen.startAddr());
          for(unsigned i = preelse_patches_size; i < postelse_patches_size; i++) {
            gen.allPatches()[i].setTarget(&else_targ);
          }
          for(unsigned i = preelse_patches_size; i < postelse_patches_size; i++) {
            gen.allPatches()[i].applyPatch();
          }
        } else {
          gen.setIndex(elseSkipIndex);
          emitA(branchOp, 0, 0,
                (Dyninst::Register)codeGen::getDisplacement(elseSkipStart, endIndex), gen,
                rc_no_control, noCost);
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

      BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
      BPatch_point *bpoint = bproc->findOrCreateBPPoint(
          NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));

      const BPatch_memoryAccess *ma = bpoint->getMemoryAccess();
      assert(ma);
      int cond = ma->conditionCode_NP();
      if(cond > -1) {
        codeBufIndex_t startIndex = gen.getIndex();
        emitJmpMC(cond, 0 /* target, changed later */, gen);
        codeBufIndex_t fromIndex = gen.getIndex();
        // Add the snippet to the tracker, as AM has indicated...
        gen.tracker()->increaseConditionalLevel();
        // generate code with the right path
        if(!loperand->generateCode_phase2(gen, noCost, addr, src1)) {
          ERROR_RETURN;
        }
        gen.rs()->freeRegister(src1);
        gen.tracker()->decreaseAndClean(gen);
        codeBufIndex_t endIndex = gen.getIndex();
        // call emit again now with correct offset.
        gen.setIndex(startIndex);
        emitJmpMC(cond, codeGen::getDisplacement(fromIndex, endIndex), gen);
        gen.setIndex(endIndex);
      } else {
        if(!loperand->generateCode_phase2(gen, noCost, addr, src1)) {
          ERROR_RETURN;
        }
        gen.rs()->freeRegister(src1);
      }

      break;
    }
    case whileOp: {
      if(!roperand) {
        return false;
      }
      codeBufIndex_t top = gen.getIndex();

      // BEGIN from ifOp
      if(!loperand->generateCode_phase2(gen, noCost, addr, src1)) {
        ERROR_RETURN;
      }
      REGISTER_CHECK(src1);
      codeBufIndex_t startIndex = gen.getIndex();

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
      if(!roperand->generateCode_phase2(gen, noCost, addr, src2)) {
        ERROR_RETURN;
      }
      gen.rs()->freeRegister(src2);
      gen.tracker()->decreaseAndClean(gen);
      gen.rs()->unifyTopRegStates(gen); // Join the registerState for the if

      // END from ifOp

      (void)emitA(branchOp, 0, 0, codeGen::getDisplacement(gen.getIndex(), top), gen, rc_no_control,
                  noCost);

      // BEGIN from ifOp

      // Now that we've generated the "then" section, rewrite the if
      // conditional branch.
      codeBufIndex_t elseStartIndex = gen.getIndex();

      if(preif_patches_size != postif_patches_size) {
        assert(postif_patches_size > preif_patches_size);
        ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
        for(unsigned i = preif_patches_size; i < postif_patches_size; i++) {
          gen.allPatches()[i].setTarget(&if_targ);
        }
        for(unsigned i = preif_patches_size; i < postif_patches_size; i++) {
          gen.allPatches()[i].applyPatch();
        }
      } else {
        gen.setIndex(startIndex);
        // call emit again now with correct offset.
        // This backtracks over current code.
        // If/when we vectorize, we can do this in a two-pass arrangement
        (void)emitA(ifOp, src1_copy, 0,
                    (Dyninst::Register)codeGen::getDisplacement(thenSkipStart, elseStartIndex), gen,
                    rc_no_control, noCost);
        // Now we can free the register
        // Dyninst::Register has already been freed; we're just re-using it.
        // gen.rs()->freeRegister(src1);

        gen.setIndex(elseStartIndex);
      }
      // END from ifOp
      retReg = Dyninst::Null_Register;
      break;
    }
    case getAddrOp: {
      switch(loperand->getoType()) {
        case operandType::variableAddr:
          if(retReg == Dyninst::Null_Register) {
            retReg = allocateAndKeep(gen, noCost);
          }
          assert(loperand->getOVar());
          loperand->emitVariableLoad(loadConstOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                     gen.point(), gen.addrSpace());
          break;
        case operandType::variableValue:
          if(retReg == Dyninst::Null_Register) {
            retReg = allocateAndKeep(gen, noCost);
          }
          assert(loperand->getOVar());
          loperand->emitVariableLoad(loadOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                     gen.point(), gen.addrSpace());
          break;
        case operandType::DataAddr: {
          addr = reinterpret_cast<Dyninst::Address>(loperand->getOValue());
          if(retReg == Dyninst::Null_Register) {
            retReg = allocateAndKeep(gen, noCost);
          }
          assert(!loperand->getOVar());
          emitVload(loadConstOp, addr, retReg, retReg, gen, noCost, gen.rs(), size, gen.point(),
                    gen.addrSpace());
        } break;
        case operandType::FrameAddr: {
          // load the address fp + addr into dest
          if(retReg == Dyninst::Null_Register) {
            retReg = allocateAndKeep(gen, noCost);
          }
          Dyninst::Register temp = gen.rs()->getScratchRegister(gen, noCost);
          addr = (Dyninst::Address)loperand->getOValue();
          emitVload(loadFrameAddr, addr, temp, retReg, gen, noCost, gen.rs(), size, gen.point(),
                    gen.addrSpace());
          break;
        }
        case operandType::RegOffset: {
          assert(loperand->operand());

          // load the address reg + addr into dest
          if(retReg == Dyninst::Null_Register) {
            retReg = allocateAndKeep(gen, noCost);
          }
          addr = (Dyninst::Address)loperand->operand()->getOValue();

          emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), retReg, gen, noCost,
                    gen.rs(), size, gen.point(), gen.addrSpace());
          break;
        }
        case operandType::DataIndir:
          // taking address of pointer de-ref returns the original
          //    expression, so we simple generate the left child's
          //    code to get the address
          if(!loperand->operand()->generateCode_phase2(gen, noCost, addr, retReg)) {
            ERROR_RETURN;
          }
          // Broken refCounts?
          break;
        case operandType::origRegister:
          // Added 2NOV11 Bernat - some variables live in original registers,
          // and so we need to be able to dereference their contents.
          if(!loperand->generateCode_phase2(gen, noCost, addr, retReg)) {
            ERROR_RETURN;
          }
          break;
        default:
          cerr << "Uh oh, unknown loperand type in getAddrOp: "
               << static_cast<uint64_t>(loperand->getoType()) << std::endl;
          cerr << "\t Generating ast " << std::hex << this << std::endl;
          assert(0);
      }
      break;
    }
    case storeOp: {
      if(!roperand) {
        return false;
      }
      bool result = generateOptimizedAssignment(gen, size, noCost);
      if(result) {
        break;
      }

      // This ast cannot be shared because it doesn't return a register
      if(!roperand->generateCode_phase2(gen, noCost, addr, src1)) {
        fprintf(stderr, "ERROR: failure generating roperand\n");
        ERROR_RETURN;
      }
      REGISTER_CHECK(src1);
      // We will access loperand's children directly. They do not expect
      // it, so we need to bump up their useCounts
      loperand->fixChildrenCounts();

      src2 = gen.rs()->allocateRegister(gen, noCost);
      switch(loperand->getoType()) {
        case operandType::variableValue:
          loperand->emitVariableStore(storeOp, src1, src2, gen, noCost, gen.rs(), size, gen.point(),
                                      gen.addrSpace());
          loperand->decUseCount(gen);
          break;
        case operandType::DataAddr:
          addr = (Dyninst::Address)loperand->getOValue();
          assert(loperand->getOVar() == NULL);
          emitVstore(storeOp, src1, src2, addr, gen, noCost, gen.rs(), size, gen.point(),
                     gen.addrSpace());
          // We are not calling generateCode for the left branch,
          // so need to decrement the refcount by hand
          loperand->decUseCount(gen);
          break;
        case operandType::FrameAddr:
          addr = (Dyninst::Address)loperand->getOValue();
          emitVstore(storeFrameRelativeOp, src1, src2, addr, gen, noCost, gen.rs(), size,
                     gen.point(), gen.addrSpace());
          loperand->decUseCount(gen);
          break;
        case operandType::RegOffset: {
          assert(loperand->operand());
          addr = (Dyninst::Address)loperand->operand()->getOValue();

          // This is cheating, but I need to pass 4 data values into emitVstore, and
          // it only allows for 3.  Prepare the dest address in scratch register src2.

          emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), src2, gen, noCost,
                    gen.rs(), size, gen.point(), gen.addrSpace());

          // Same as DataIndir at this point.
          emitV(storeIndirOp, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(),
                gen.addrSpace());
          loperand->decUseCount(gen);
          break;
        }
        case operandType::DataIndir: {
          // store to a an expression (e.g. an array or field use)
          // *(+ base offset) = src1
          if(!loperand->operand()->generateCode_phase2(gen, noCost, addr, tmp)) {
            ERROR_RETURN;
          }
          REGISTER_CHECK(tmp);

          // tmp now contains address to store into
          emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(),
                gen.addrSpace());
          gen.rs()->freeRegister(tmp);
          loperand->decUseCount(gen);
          break;
        }
        case operandType::origRegister:
          gen.rs()->writeProgramRegister(gen, (Dyninst::Register)(long)loperand->getOValue(), src1,
                                         getSize());
          loperand->decUseCount(gen);
          break;
        case operandType::Param:
        case operandType::ParamAtCall:
        case operandType::ParamAtEntry: {
          boost::shared_ptr<operandAST> lnode =
              boost::dynamic_pointer_cast<operandAST>(loperand);
          emitR(getParamOp, (Dyninst::Address)lnode->oValue, src1, src2, gen, noCost, gen.point(),
                gen.addrSpace()->multithread_capable());
          loperand->decUseCount(gen);
          break;
        }
        case operandType::ReturnVal:
          emitR(getRetValOp, Dyninst::Null_Register, src1, src2, gen, noCost, gen.point(),
                gen.addrSpace()->multithread_capable());
          loperand->decUseCount(gen);
          break;
        case operandType::ReturnAddr:
          emitR(getRetAddrOp, Dyninst::Null_Register, src1, src2, gen, noCost, gen.point(),
                gen.addrSpace()->multithread_capable());
          break;
        default: {
          // Could be an error, could be an attempt to load based on an arithmetic expression
          // Generate the left hand side, store the right to that address
          if(!loperand->generateCode_phase2(gen, noCost, addr, tmp)) {
            ERROR_RETURN;
          }
          REGISTER_CHECK(tmp);

          emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(),
                gen.addrSpace());
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
      if(!roperand) {
        return false;
      }
      if(!roperand->generateCode_phase2(gen, noCost, addr, src1)) {
        ERROR_RETURN;
      }
      if(!loperand->generateCode_phase2(gen, noCost, addr, src2)) {
        ERROR_RETURN;
      }
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
    default: {
      if(!roperand) {
        return false;
      }
      bool signedOp = IsSignedOperation(loperand->getType(), roperand->getType());
      src1 = Dyninst::Null_Register;
      right_dest = Dyninst::Null_Register;
      if(!loperand->generateCode_phase2(gen, noCost, addr, src1)) {
        ERROR_RETURN;
      }
      REGISTER_CHECK(src1);

      if((roperand->getoType() == operandType::Constant) &&
         doNotOverflow((int64_t)roperand->getOValue())) {
        if(retReg == Dyninst::Null_Register) {
          retReg = allocateAndKeep(gen, noCost);
          ast_printf("Operator node, const RHS, allocated register %u\n", retReg.getId());
        } else {
          ast_printf("Operator node, const RHS, keeping register %u\n", retReg.getId());
        }

        emitImm(op, src1, (RegValue)roperand->getOValue(), retReg, gen, noCost, gen.rs(), signedOp);

        if(src1 != Dyninst::Null_Register) {
          gen.rs()->freeRegister(src1);
        }

        // We do not .generateCode for roperand, so need to update its
        // refcounts manually
        roperand->decUseCount(gen);
      } else {
        if(!roperand->generateCode_phase2(gen, noCost, addr, right_dest)) {
          ERROR_RETURN;
        }
        REGISTER_CHECK(right_dest);
        if(retReg == Dyninst::Null_Register) {
          retReg = allocateAndKeep(gen, noCost);
        }
        emitV(op, src1, right_dest, retReg, gen, noCost, gen.rs(), size, gen.point(),
              gen.addrSpace(), signedOp);
        if(src1 != Dyninst::Null_Register) {
          // Don't free inputs until afterwards; we have _no_ idea
          gen.rs()->freeRegister(src1);
        }
        // what the underlying code might do with a temporary register.
        if(right_dest != Dyninst::Null_Register) {
          gen.rs()->freeRegister(right_dest);
        }
      }
    }
  }
  decUseCount(gen);
  return true;
}

BPatch_type *operatorAST::checkType(BPatch_function *func) {
  BPatch_type *ret = NULL;
  BPatch_type *lType = NULL, *rType = NULL, *eType = NULL;
  bool errorFlag = false;

  assert(BPatch::bpatch != NULL); /* We'll use this later. */

  if((loperand || roperand) && getType()) {
    // something has already set the type for us.
    // this is likely an expression for array access
    ret = const_cast<BPatch_type *>(getType());
    return ret;
  }

  if(loperand) {
    lType = loperand->checkType(func);
  }

  if(roperand) {
    rType = roperand->checkType(func);
  }

  if(eoperand) {
    eType = eoperand->checkType(func);
  }
  (void)eType; // unused...

  if(lType == BPatch::bpatch->type_Error || rType == BPatch::bpatch->type_Error) {
    errorFlag = true;
  }

  switch(op) {
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
      if(lType != NULL && rType != NULL) {
        if(!lType->isCompatible(rType)) {
          fprintf(stderr, "WARNING: LHS type %s not compatible with RHS type %s\n",
                  lType->getName(), rType->getName());
          errorFlag = true;
        }
      }
      break;
  }
  assert(ret != NULL);

  if(errorFlag && doTypeCheck) {
    ret = BPatch::bpatch->type_Error;
  } else if(errorFlag) {
    ret = BPatch::bpatch->type_Untyped;
  }

  // remember what type we are
  setType(ret);

  return ret;
}

// Check if the node can be kept at all. Some nodes (e.g., storeOp)
// can not be cached. In fact, there are fewer nodes that can be cached.
bool operatorAST::canBeKept() const {
  switch(op) {
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
  if(loperand && !loperand->canBeKept()) {
    return false;
  }
  if(roperand && !roperand->canBeKept()) {
    return false;
  }
  if(eoperand && !eoperand->canBeKept()) {
    return false;
  }

  return true;
}

std::string operatorAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Op/" << std::hex << this << std::dec << "(" << format_opcode(op) << ")"
      << std::endl;
  if(loperand) {
    ret << indent << loperand->format(indent + "  ");
  }
  if(roperand) {
    ret << indent << roperand->format(indent + "  ");
  }
  if(eoperand) {
    ret << indent << eoperand->format(indent + "  ");
  }

  return ret.str();
}

}}
