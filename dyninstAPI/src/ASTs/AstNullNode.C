#include "AstNullNode.h"
#include "codegen.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

bool AstNullNode::generateCode_phase2(codeGen &gen, bool, Dyninst::Address &retAddr,
                                      Dyninst::Register &retReg) {
  retAddr = Dyninst::ADDR_NULL;
  retReg = Dyninst::Null_Register;

  decUseCount(gen);

  return true;
}

std::string AstNullNode::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Null/" << std::hex << this << "()\n";
  return ret.str();
}

}}
