#include "ast_helpers.h"
#include "addressAST.h"
#include "codegen.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

bool originalAddressAST::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                                              Dyninst::Register &retReg) {
  RETURN_KEPT_REG(retReg);

  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen, noCost);
  }

  if(retReg == Dyninst::Null_Register) {
    return false;
  }

  emitVload(loadConstOp, gen.point()->addr_compat(), retReg, retReg, gen, noCost);

  return true;
}

namespace AddressAST {

  originalAddressAST::Ptr original() {
    static auto node = boost::make_shared<originalAddressAST>();
    return node;
  }

}

}}
