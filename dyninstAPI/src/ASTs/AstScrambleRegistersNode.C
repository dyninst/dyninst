#include "ast_helpers.h"
#include "AstScrambleRegistersNode.h"
#include "codegen.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

#ifndef DYNINST_CODEGEN_ARCH_X86_64

bool AstScrambleRegistersNode::generateCode_phase2(codeGen &, bool, Dyninst::Address &,
                                                   Dyninst::Register &) {
  return true;
}

#else

bool AstScrambleRegistersNode::generateCode_phase2(codeGen &gen, bool, Address &,
                                                   Dyninst::Register &) {
  for(int i = 0; i < gen.rs()->numGPRs(); i++) {
    registerSlot *reg = gen.rs()->GPRs()[i];
    if(reg->encoding() != REGNUM_RBP && reg->encoding() != REGNUM_RSP) {
      gen.codeEmitter()->emitLoadConst(reg->encoding(), -1, gen);
    }
  }
  return true;
}

#endif

}}
