// Checks that SymbolicExpression::SimplifyAnAST, which jump-table, call-target and bound
// analysis use, folds operations on constants to the value they compute.  A constant's
// value is its low 'size' bits (size 0 means 64); a sign extension keeps its sign in all
// 64 bits, whatever its size.  SimplifyRoot takes a 32-bit constant whose bit 31 is clear
// as a 64-bit one, so the 32-bit rows use values with bit 31 set.

#include "ConstantAST.h"
#include "RoseAST.h"
#include "parseAPI/src/SymbolicExpression.h"

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

  AST::Ptr op(Op::Op o, AST::Ptr a) { return df::RoseAST::create(df::ROSEOperation(o), a); }

  AST::Ptr op(Op::Op o, AST::Ptr a, AST::Ptr b) { return df::RoseAST::create(df::ROSEOperation(o), a, b); }

  AST::Ptr op(Op::Op o, AST::Ptr a, AST::Ptr b, AST::Ptr d) {
    return df::RoseAST::create(df::ROSEOperation(o), a, b, d);
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
  uint64_t const minus1 = ~uint64_t{0};

  // clang-format off
  std::vector<test_case> const cases = {
    // The negation of a constant is -c, at its width.
    {"-(2)",                             op(Op::negateOp, c(2, 64)),                               minus1 - 1},
    {"-(0x80000001), 32 bits",           op(Op::negateOp, c(0x80000001, 32)),                      0x7fffffff},

    // A sum is reduced to its width.
    {"0xffffffff + 0x80000001, 32 bits", op(Op::addOp, c(0xffffffff, 32), c(0x80000001, 32)),      0x80000000},

    // Shifts keep the width of the shifted value; a shift by the width or more gives 0.
    {"sext64(0x1000 << 3, 16 bits)",     op(Op::signExtendOp, op(Op::shiftLOp, c(0x1000, 16), c(3, 64)), c(64, 64)),
                                                                                                    0xffffffffffff8000},
    {"1 << 64",                          op(Op::shiftLOp, c(1, 64), c(64, 64)),                    0},
    {"0x1ffffffff >> 4, 32 bits",        op(Op::shiftROp, c(0x1ffffffff, 32), c(4, 64)),           0x0fffffff},
    {"0x80000001 rotl 1, 32 bits",       op(Op::rotateLOp, c(0x80000001, 32), c(1, 64)),           3},

    // A sign extension extends from any width and keeps the sign in all 64 bits.
    {"sext64(0x800, 12 bits)",           op(Op::signExtendOp, c(0x800, 12), c(64, 64)),            0xfffffffffffff800},
    {"sext32(0x8000, 16 bits)",          op(Op::signExtendOp, c(0x8000, 16), c(32, 64)),           0xffffffffffff8000},
    // Power's addi: a 16-bit immediate sign-extended to 32 bits, added to a 64-bit address.
    {"0x100f0000 + sext32(-24668)",      op(Op::addOp, c(0x100f0000, 64),
                                            op(Op::signExtendOp, extract(c(40868, 32), 0, 16), c(32, 64))),
                                                                                                    0x100e9fa4},
    // A zero extension keeps the operand's bits only.
    {"zext64(0x1ff, 8 bits)",            op(Op::extendOp, c(0x1ff, 8), c(64, 64)),                 0xff},

    // Bits from 64 up of a constant are zero.
    {"bits 64..65 of 5",                 extract(c(5, 64), 64, 65),                                0},
    {"concat(5, 1), 5 holding 64 bits",  op(Op::concatOp, c(5, 64), c(1, 8)),                      5},

    // A product is folded at its full width where its bits are taken.
    {"high half of -1 * 2, unsigned",    extract(extract(op(Op::uMultOp, c(minus1, 64), c(2, 64)), 0, 128), 64, 128), 1},
    {"high half of -1 * 2, signed",      extract(extract(op(Op::sMultOp, c(minus1, 64), c(2, 64)), 0, 128), 64, 128), minus1},
    {"low half of -1 * 2, signed",       extract(op(Op::sMultOp, c(minus1, 64), c(2, 64)), 0, 64), minus1 - 1},
    {"3 * 5, 8 bits each",               op(Op::uMultOp, c(3, 8), c(5, 8)),                        15},

    // Tests of a value read only its width.
    {"~0, size 0",                       op(Op::invertOp, c(0, 0)),                                minus1},
    {"0x100 == 0, 8 bits",               op(Op::equalToZeroOp, c(0x100, 8)),                       1},
    {"ite(2 as 1 bit, 7, 9)",            op(Op::ifOp, c(2, 1), c(7, 64), c(9, 64)),                9},
  };
  // clang-format on

  bool ok = true;
  SymbolicExpression se;
  se.cs = nullptr;
  se.cr = nullptr;
  for(auto const &t : cases) {
    std::string const before = t.expr->format();
    AST::Ptr const folded = se.SimplifyAnAST(t.expr, 0);
    if(folded->getID() != AST::V_ConstantAST) {
      std::cerr << t.text << ": does not fold to a constant: " << folded->format() << "\n  " << before << '\n';
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
