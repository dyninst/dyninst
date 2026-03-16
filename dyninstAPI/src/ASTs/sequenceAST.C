#include "ast_helpers.h"
#include "BPatch.h"
#include "BPatch_type.h"
#include "codegen.h"
#include "registerSpace.h"
#include "sequenceAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

bool sequenceAST::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                                          Dyninst::Register &retReg) {
  RETURN_KEPT_REG(retReg);
  Dyninst::Register tmp = Dyninst::Null_Register;
  Dyninst::Address unused = Dyninst::ADDR_NULL;

  if(children.size() == 0) {
    return true;
  }

  for(unsigned i = 0; i < children.size() - 1; i++) {
    if(!children[i]->generateCode_phase2(gen, noCost, unused, tmp)) {
      ERROR_RETURN;
    }
    gen.rs()->freeRegister(tmp);
    tmp = Dyninst::Null_Register;
  }

  // We keep the last one
  if(!children.back()->generateCode_phase2(gen, noCost, unused, retReg)) {
    ERROR_RETURN;
  }

  decUseCount(gen);

  return true;
}

BPatch_type *sequenceAST::checkType(BPatch_function *func) {
  BPatch_type *ret = NULL;
  BPatch_type *sType = NULL;
  bool errorFlag = false;

  assert(BPatch::bpatch != NULL); /* We'll use this later. */

  if(getType()) {
    // something has already set the type for us.
    // this is likely an expression for array access
    ret = const_cast<BPatch_type *>(getType());
    return ret;
  }

  for(unsigned i = 0; i < children.size(); i++) {
    sType = children[i]->checkType(func);
    if(sType == BPatch::bpatch->type_Error) {
      errorFlag = true;
    }
  }

  ret = sType;

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

std::string sequenceAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Seq/" << std::hex << this << std::dec << "()" << std::endl;
  for(unsigned i = 0; i < children.size(); ++i) {
    ret << indent << children[i]->format(indent + "  ");
  }
  return ret.str();
}

}}
