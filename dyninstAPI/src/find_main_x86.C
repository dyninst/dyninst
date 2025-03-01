#include "CFG.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "dataflowAPI/h/SymEval.h"
#include "debug.h"
#include "DynAST.h"
#include "dyntypes.h"
#include "findMain.h"
#include "Function.h"
#include "SymEval.h"
#include "Symtab.h"

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;
  namespace df = Dyninst::DataflowAPI;

  class FindMainVisitor final : public Dyninst::ASTVisitor {
    using ASTVisitor::visit;

  public:
    bool resolved{false};
    bool hardFault{false};
    Dyninst::Address target{};

    Dyninst::AST::Ptr visit(df::RoseAST* r) override {
      Dyninst::AST::Children newKids;
      for(unsigned i = 0; i < r->numChildren(); i++) {
        newKids.push_back(r->child(i)->accept(this));
      }

      if(r->val().op == df::ROSEOperation::addOp) {
        auto const_ast = Dyninst::AST::V_ConstantAST;
        assert(newKids.size() == 2);
        if(newKids[0]->getID() == const_ast && newKids[1]->getID() == const_ast) {
          auto c1 = df::ConstantAST::convert(newKids[0]);
          auto c2 = df::ConstantAST::convert(newKids[1]);
          if(!hardFault) {
            target = c1->val().val + c2->val().val;
            resolved = true;
          }
          return df::ConstantAST::create(df::Constant(c1->val().val + c2->val().val));
        }
      } else {
        startup_printf("%s[%d] unhandled FindMainVisitor operation %d\n", FILE__, __LINE__, r->val().op);
      }

      return df::RoseAST::create(r->val(), newKids);
    }

    ASTPtr visit(df::ConstantAST* c) override {
      /* We can only handle constant values */
      if(!target && !hardFault) {
        resolved = true;
        target = c->val().val;
      }

      return c->ptr();
    }

    ASTPtr visit(df::VariableAST* v) override {
      /* If we visit a variable node, we can't do any analysis */
      hardFault = true;
      resolved = false;
      target = 0;
      return v->ptr();
    }
  };

  Dyninst::Address find_main(pa::Function* func, pa::Block* b) {
    pa::Block::Insns insns;
    b->getInsns(insns);

    if(insns.size() < 2UL) {
      startup_printf("findMain: not enough instructions in block 0x%lx\n", b->start());
      return Dyninst::ADDR_NULL;
    }

    // To get the second-to-last instruction, which loads the address of main
    auto iit = insns.end();
    --iit;
    --iit;

    std::vector<Assignment::Ptr> assignments;
    Dyninst::AssignmentConverter assign_convert(true, false);
    assign_convert.convert(iit->second, iit->first, func, b, assignments);

    if(assignments.empty()) {
      return Dyninst::ADDR_NULL;
    }

    Assignment::Ptr assignment = assignments[0];
    std::pair<AST::Ptr, bool> res = DataflowAPI::SymEval::expand(assignment, false);
    AST::Ptr ast = res.first;
    if(!ast) {
      startup_printf("findMain: cannot expand %s from instruction %s\n", assignment->format().c_str(),
                     assignment->insn().format().c_str());
      return Dyninst::ADDR_NULL;
    }

    FindMainVisitor fmv;
    ast->accept(&fmv);
    if(fmv.resolved) {
      return fmv.target;
    }

    return Dyninst::ADDR_NULL;
  }

}}}
