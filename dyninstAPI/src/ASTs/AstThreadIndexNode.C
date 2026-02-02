#include "AstThreadIndexNode.h"
#include "debug.h"

namespace Dyninst { namespace DyninstAPI {

// We use one of these across all platforms, since it
// devolves into a process-specific function node.
// However, this lets us delay that until code generation
// when we have the process pointer. If we get multiples,
// we'll screw up our pointer-based common subexpression
// elimination.
AstThreadIndexNode AstThreadIndexNode::node{};

// The output depends solely on input parameters, so can
// be guaranteed to not change if executed multiple times
// in the same sequence - AKA "can be kept".
bool AstThreadIndexNode::canBeKept() const {
  for(auto &&c : children) {
    if(!c->canBeKept()) {
      ast_printf("AST %p: labelled const func but argument %s cannot be kept!\n",
                 reinterpret_cast<const void *>(this), c->format("").c_str());
      return false;
    }
  }
  return true;
}

}}
