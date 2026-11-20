// Checks that DataflowAPI::StackVisitor, which SymEval::expand runs to simplify stack
// accesses, folds extensions and extracts of constants to the value they compute: a sign
// extension extends (keeping the sign in all 64 bits), a zero extension keeps the
// operand's bits, and bits from 64 up of a constant are zero.

#include "ConstantAST.h"
#include "RoseAST.h"
#include "dataflowAPI/src/SymEvalVisitors.h"
#include "stackanalysis.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
  namespace df = Dyninst::DataflowAPI;
  using Dyninst::AST;
  using Op = df::ROSEOperation;

  AST::Ptr c(uint64_t v, size_t size) { return df::ConstantAST::create(df::Constant(v, size)); }

  AST::Ptr extend(Op::Op o, AST::Ptr a, uint64_t to) {
    return df::RoseAST::create(df::ROSEOperation(o), a, c(to, 64));
  }

  AST::Ptr extract(AST::Ptr a, uint64_t from, uint64_t to) {
    return df::RoseAST::create(df::ROSEOperation(Op::extractOp, to - from), a, c(from, 64), c(to, 64));
  }

  struct test_case {
    char const *text;
    AST::Ptr expr;
    uint64_t expected;
  };

  std::string hex(uint64_t v) {
    std::ostringstream s;
    s << "0x" << std::hex << v;
    return s.str();
  }
}

int main() {
  // clang-format off
  std::vector<test_case> const cases = {
    {"sext64(0x800, 12 bits)",  extend(Op::signExtendOp, c(0x800, 12), 64),   0xfffffffffffff800},
    {"sext32(0x8000, 16 bits)", extend(Op::signExtendOp, c(0x8000, 16), 32),  0xffffffffffff8000},
    {"sext64(0x7ff, 12 bits)",  extend(Op::signExtendOp, c(0x7ff, 12), 64),   0x7ff},
    {"zext64(0x1ff, 8 bits)",   extend(Op::extendOp, c(0x1ff, 8), 64),        0xff},
    {"bits 64..65 of 5",        extract(c(5, 64), 64, 65),                    0},
    {"bits 4..8 of 0xf3",       extract(c(0xf3, 64), 4, 8),                   0xf},
  };
  // clang-format on

  bool ok = true;
  for(auto const &t : cases) {
    Dyninst::StackAnalysis::Height sp, fp;
    df::StackVisitor sv(0, nullptr, sp, fp);
    std::string const before = t.expr->format();
    AST::Ptr const folded = t.expr->accept(&sv);
    if(!folded || folded->getID() != AST::V_ConstantAST) {
      std::cerr << t.text << ": does not fold to a constant: " << (folded ? folded->format() : "null") << "\n  "
                << before << '\n';
      ok = false;
      continue;
    }
    uint64_t const got = df::ConstantAST::convert(folded)->val().val;
    if(got != t.expected) {
      std::cerr << t.text << ": folds to " << hex(got) << ", expected " << hex(t.expected) << "\n  " << before
                << '\n';
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
