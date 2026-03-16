#include "Buffer.h"
#include "codegen.h"
#include "instPoint.h"
#include "snippetAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

bool snippetAST::generateCode_phase2(codeGen &gen, bool, Dyninst::Address &,
                                         Dyninst::Register &) {
  Dyninst::Buffer buf(gen.currAddr(), 1024);
  if(!snip_->generate(gen.point(), buf)) {
    return false;
  }
  gen.copy(buf.start_ptr(), buf.size());
  return true;
}

}}
