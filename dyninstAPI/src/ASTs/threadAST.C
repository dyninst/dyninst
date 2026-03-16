#include "debug.h"
#include "threadAST.h"

#include <boost/make_shared.hpp>

namespace Dyninst { namespace DyninstAPI {

// The output depends solely on input parameters, so can
// be guaranteed to not change if executed multiple times
// in the same sequence - AKA "can be kept".
bool threadAST::canBeKept() const {
  for(auto &&c : children) {
    if(!c->canBeKept()) {
      ast_printf("AST %p: labelled const func but argument %s cannot be kept!\n",
                 reinterpret_cast<const void *>(this), c->format("").c_str());
      return false;
    }
  }
  return true;
}


// We use one of these across all platforms, since it
// devolves into a process-specific function node.
// However, this lets us delay that until code generation
// when we have the process pointer. If we get multiples,
// we'll screw up our pointer-based common subexpression
// elimination.
threadAST::Ptr threadAST::index() {
  static threadAST::Ptr idx = boost::make_shared<threadAST>();
  return idx;
}

}}
