#include "ast_helpers.h"
#include "block.h"
#include "codegen.h"
#include "instPoint.h"
#include "Instruction.h"
#include "jumpTargetAST.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

bool dynamicJumpTargetAST::generateCode_phase2(codeGen &gen, bool noCost, Address &retAddr,
                                               Dyninst::Register &retReg) {
  if(gen.point()->type() != instPoint::PreCall && gen.point()->type() != instPoint::FuncExit &&
     gen.point()->type() != instPoint::PreInsn) {
    return false;
  }

  InstructionAPI::Instruction insn = gen.point()->block()->getInsn(gen.point()->block()->last());
  if(insn.isReturn()) {
    // if this is a return instruction our AST reads the top stack value
    if(retReg == Dyninst::Null_Register) {
      retReg = allocateAndKeep(gen, noCost);
    }
    if(retReg == Dyninst::Null_Register) {
      return false;
    }

#if defined(DYNINST_CODEGEN_ARCH_I386)
    emitVload(loadRegRelativeOp, (Address)0, REGNUM_ESP, retReg, gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_X86_64)
    emitVload(loadRegRelativeOp, (Address)0, REGNUM_RSP, retReg, gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_POWER) // KEVINTODO: untested
    emitVload(loadRegRelativeOp, (Address)sizeof(Address), REG_SP, retReg, gen, noCost);
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
    // #warning "This function is not implemented yet!"
    assert(0);
#else
    assert(0);
#endif
    return true;
  } else { // this is a dynamic ctrl flow instruction, have
    // getDynamicCallSiteArgs generate the necessary AST
    std::vector<codeGenASTPtr> args;
    if(!gen.addrSpace()->getDynamicCallSiteArgs(insn, gen.point()->block()->last(), args)) {
      return false;
    }
    if(!args[0]->generateCode_phase2(gen, noCost, retAddr, retReg)) {
      return false;
    }
    return true;
  }
}

namespace jumpTargetAST {

  dynamicJumpTargetAST::Ptr dynamic() {
    static auto node = boost::make_shared<dynamicJumpTargetAST>();
    return node;
  }

}

}}
