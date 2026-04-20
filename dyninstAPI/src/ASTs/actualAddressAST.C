#include "ast_helpers.h"
#include "addressAST.h"
#include "codegen.h"
#include "opcode.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

bool actualAddressAST::generateCode_phase2(codeGen &gen, Dyninst::Address &,
                                            Dyninst::Register &retReg) {
  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen);
  }
  if(retReg == Dyninst::Null_Register) {
    return false;
  }

  emitVload(loadConstOp, gen.currAddr(), retReg, retReg, gen);

  return true;
}

namespace AddressAST {

  actualAddressAST::Ptr actual() {
    static auto node = boost::make_shared<actualAddressAST>();
    return node;
  }

}

}}
