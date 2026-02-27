#include "codegen.h"
#include "debug.h"
#include "instPoint.h"
#include "variableAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

void variableAST::setVariableAST(codeGen &gen) {
  if(!ranges_) {
    return;
  }

  // oneTimeCode. Set the AST at the beginning of the function??
  if(!gen.point()) {
    index = 0;
    return;
  }

  // Offset of inst point from function base address
  Dyninst::Address addr = gen.point()->addr_compat();

  bool found = false;
  for(unsigned i = 0; i < ranges_->size(); i++) {
    if((*ranges_)[i].first <= addr && addr <= (*ranges_)[i].second) {
      index = i;
      found = true;
    }
  }

  if(!found) {
    ast_cerr << "Error: unable to find AST representing variable at " << std::hex << addr << '\n'
             << "Pointer " << this << '\n'
             << "Options are: \n";

    for(auto &r : *ranges_) {
      ast_cerr << "\t" << std::hex << r.first << "-" << r.second << '\n';
    }
  }
  assert(found);
}

std::string variableAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Var/" << std::hex << this << std::dec << "(" << children.size() << ")"
      << std::endl;

  for(auto &c : children) {
    ret << indent << c->format(indent + "  ");
  }

  return ret.str();
}

}}
