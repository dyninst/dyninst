#include "ast_helpers.h"
#include "AstAddrNode.h"
#include "codegen.h"
#include "opcode.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

bool AstActualAddrNode::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                                            Dyninst::Register &retReg) {
  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen, noCost);
  }
  if(retReg == Dyninst::Null_Register) {
    return false;
  }

  emitVload(loadConstOp, gen.currAddr(), retReg, retReg, gen, noCost);

  return true;
}

namespace AddressNode {

  AstNodePtr actual() {
    static auto node = boost::make_shared<AstActualAddrNode>();
    return node;
  }

}

}}
