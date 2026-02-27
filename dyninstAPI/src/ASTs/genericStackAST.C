#include "codegen.h"
#include "genericStackAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

std::string genericStackAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "StackGeneric/" << std::hex << this;
  ret << std::endl;
  return ret.str();
}

#ifndef cap_stack_mods

bool genericStackAST::generateCode_phase2(codeGen &, bool, Dyninst::Address &,
                                              Dyninst::Register &) {
  return false;
}

#else

bool genericStackAST::generateCode_phase2(codeGen &gen, bool, Dyninst::Address &,
                                              Dyninst::Register &) {
  gen.setInsertNaked(true);
  gen.setModifiedStackFrame(true);

  // No code generation necessary

  return true;
}

#endif

}}
