#include "addressSpace.h"
#include "ast_helpers.h"
#include "BPatch.h"
#include "BPatch_function.h"
#include "codegen.h"
#include "debug.h"
#include "emitter.h"
#include "function.h"
#include "functionCallAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

bool functionCallAST::initRegisters(codeGen &gen) {
  // For now, we only care if we should save everything. "Everything", of course,
  // is platform dependent. This is the new location of the clobberAllFuncCalls
  // that had previously been in emitCall.

  bool ret = true;

  // First, check children
  for(unsigned i = 0; i < children.size(); i++) {
    if(!children[i]->initRegisters(gen)) {
      ret = false;
    }
  }

  if(callReplace_) {
    return true;
  }

  // We also need a function object.
  func_instance *callee = func_;
  if(!callee) {
    // Painful lookup time
    callee = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
  }
  assert(callee);

  // Marks registers as used based on the callee's behavior
  // This means we'll save them if necessary (also, lets us use
  // them in our generated code because we've saved, instead
  // of saving others).
  assert(gen.codeEmitter());
  gen.codeEmitter()->clobberAllFuncCall(gen.rs(), callee);

  return ret;
}

bool functionCallAST::generateCode_phase2(codeGen &gen, bool noCost, Address &,
                                      Dyninst::Register &retReg) {
  // We call this anyway... not that we'll ever be kept.
  // Well... if we can somehow know a function is entirely
  // dependent on arguments (a flag?) we can keep it around.
  RETURN_KEPT_REG(retReg);

  // VG(11/06/01): This platform independent fn calls a platfrom
  // dependent fn which calls it back for each operand... Have to
  // fix those as well to pass location...

  func_instance *use_func = func_;

  if(!use_func && !func_addr_) {
    // We purposefully don't cache the func_instance object; the AST nodes
    // are process independent, and functions kinda are.
    use_func = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
    if(!use_func) {
      fprintf(stderr, "ERROR: failed to find function %s, unable to create call\n",
              func_name_.c_str());
    }
    assert(use_func); // Otherwise we've got trouble...
  }

  Dyninst::Register tmp = 0;

  if(use_func && !callReplace_) {
    tmp = emitFuncCall(callOp, gen, children, noCost, use_func);
  } else if(use_func && callReplace_) {
    tmp = emitFuncCall(funcJumpOp, gen, children, noCost, use_func);
  } else {
    char msg[256];
    sprintf(msg, "%s[%d]:  internal error:  unable to find %s", __FILE__, __LINE__,
            func_name_.c_str());
    showErrorCallback(100, msg);
    assert(0); // can probably be more graceful
  }

  // TODO: put register allocation here and have emitCall just
  // move the return result.
  if(tmp == Dyninst::Null_Register) {
    // Happens in function replacement... didn't allocate
    // a return register.
  } else if(retReg == Dyninst::Null_Register) {
    // emitFuncCall allocated tmp; we can use it, but let's see
    //  if we should keep it around.
    retReg = tmp;
    // from allocateAndKeep:
    if(useCount > 1) {
      // If use count is 0 or 1, we don't want to keep
      // it around. If it's > 1, then we can keep the node
      // (by construction) and want to since there's another
      // use later.
      gen.tracker()->addKeptRegister(gen, this, retReg);
    }
  } else if(retReg != tmp) {
    emitImm(orOp, tmp, 0, retReg, gen, noCost, gen.rs());
    gen.rs()->freeRegister(tmp);
  }
  decUseCount(gen);
  return true;
}

BPatch_type *functionCallAST::checkType(BPatch_function *func) {
  BPatch_type *ret = NULL;
  bool errorFlag = false;

  assert(BPatch::bpatch != NULL); /* We'll use this later. */

  unsigned i;
  for(i = 0; i < children.size(); i++) {
    BPatch_type *opType = children[i]->checkType(func);
    /* XXX Check operands for compatibility */
    if(opType == BPatch::bpatch->type_Error) {
      errorFlag = true;
    }
  }
  /* XXX Should set to return type of function. */
  ret = BPatch::bpatch->type_Untyped;

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

std::string functionCallAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Call/" << hex << this << dec;
  if(func_) {
    ret << "(" << func_->name() << ")";
  } else {
    ret << "(" << func_name_ << ")";
  }
  ret << endl;
  indent += "  ";
  for(unsigned i = 0; i < children.size(); ++i) {
    ret << indent << children[i]->format(indent + "  ");
  }

  return ret.str();
}

functionCallAST::Ptr functionCallAST::namedCall(std::string name, std::vector<codeGenASTPtr> &args, AddressSpace *addrSpace) {

  if(!addrSpace) {
    return namedCall(std::move(name), args);
  }

  func_instance *ifunc = addrSpace->findOnlyOneFunction(name);
  if(ifunc == NULL) {
    ast_printf("%s[%d]: Can't find function %s\n", FILE__, __LINE__, name.c_str());
    return functionCallAST::Ptr();
  }

  return boost::make_shared<functionCallAST>(ifunc, args);
}

}}
