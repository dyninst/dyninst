#include "SymEval.h"
#include "slicing.h"
using namespace Dyninst;
using namespace DataflowAPI;

// We extend the default ASTVisitor to check whether the AST is a constant
class ConstVisitor: public ASTVisitor {
public:
  // the three overloads left alone keep the base class's behaviour
  using ASTVisitor::visit;

  bool resolved;
  Address target;
  ConstVisitor() : resolved(true), target(0){}

  // We reach a constant node and record its value
  virtual AST::Ptr visit(DataflowAPI::ConstantAST* ast)
  {
    target = ast->val().val;
    return AST::Ptr();
  }

  // If the AST contains a variable
  // or an operation, then the control flow target cannot
  // be resolved through constant propagation
  virtual AST::Ptr visit(DataflowAPI::VariableAST *)
  {
    resolved = false;
    return AST::Ptr();
  }
  virtual AST::Ptr visit(DataflowAPI::RoseAST* ast)
  {
    resolved = false;

    // Recursively visit all children
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i)  {
      ast->child(i)->accept(this);
    }
    return AST::Ptr();
  }
};

Address ExpandSlice(GraphPtr slice, Assignment::Ptr pcAssign)
{
  Result_t symRet;
  SymEval::expand(slice, symRet);

  // We get AST representing the jump target
  AST::Ptr pcExp = symRet[pcAssign];

  // We analyze the AST to see if it can actually be resolved by constant propagation
  ConstVisitor cv;
  pcExp->accept(&cv);
  if (cv.resolved) return cv.target;
  return 0;
}
