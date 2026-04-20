#include "ast_helpers.h"
#include "addressAST.h"
#include "codegen.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

bool originalAddressAST::generateCode_phase2(codeGen &gen, Dyninst::Address &,
                                              Dyninst::Register &retReg) {
  RETURN_KEPT_REG(retReg);

  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen);
  }

  if(retReg == Dyninst::Null_Register) {
    return false;
  }

  emitVload(loadConstOp, gen.point()->addr_compat(), retReg, retReg, gen);

  return true;
}

namespace AddressAST {

  originalAddressAST::Ptr original() {
    static auto node = boost::make_shared<originalAddressAST>();
    return node;
  }

}

}}
