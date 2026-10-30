/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Checks the riscv64 instruction semantics by value: SymEval expands what an instruction
// writes to a register or to memory, the expression is evaluated with a1, a2, a5, pc and
// one memory word set to known values, and the result is compared with what the RISC-V
// specification gives for those inputs.
//
// SymEval's expressions record a width only on extract, signExtend and deref, so the
// evaluation infers the others from the operands, keeps sums and negations at 128 bits,
// and applies a width only where an operation depends on it.

#include "Absloc.h"
#include "ConstantAST.h"
#include "InstructionDecoder.h"
#include "RoseAST.h"
#include "SymEval.h"
#include "VariableAST.h"
#include "registers/riscv64_regs.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
  namespace rv = Dyninst::riscv64;
  namespace df = Dyninst::DataflowAPI;

  __extension__ typedef unsigned __int128 u128;
  __extension__ typedef __int128 s128;

  constexpr uint64_t insn_addr = 0x1000;
  constexpr uint64_t mem_addr = 0x2000;

  struct value {
    u128 v;
    size_t width;
  };

  u128 mask(size_t width) {
    return width >= 128 ? ~u128{0} : (u128{1} << width) - 1;
  }

  u128 bits(value const &x) {
    return x.v & mask(x.width);
  }

  s128 signed_bits(value const &x) {
    u128 const b = bits(x);
    if(x.width == 0 || x.width >= 128 || !((b >> (x.width - 1)) & 1)) {
      return static_cast<s128>(b);
    }
    return static_cast<s128>(b | ~mask(x.width));
  }

  struct machine {
    std::map<Dyninst::MachRegister, uint64_t> regs;
    uint64_t memory; // the doubleword at mem_addr
  };

  value eval(Dyninst::AST::Ptr const &e, machine const &m);

  uint64_t constant_of(Dyninst::AST::Ptr const &e) {
    if(e->getID() != Dyninst::AST::V_ConstantAST) {
      throw std::runtime_error("expected a constant, got " + e->format());
    }
    return df::ConstantAST::convert(e)->val().val;
  }

  value eval_op(df::RoseAST::Ptr const &r, machine const &m) {
    using Op = df::ROSEOperation;
    auto kid = [&](unsigned i) { return eval(r->child(i), m); };
    switch(r->val().op) {
      case Op::extractOp: {
        value const a = kid(0);
        uint64_t const begin = constant_of(r->child(1));
        uint64_t const end = constant_of(r->child(2));
        return {(a.v >> begin) & mask(end - begin), end - begin};
      }
      case Op::signExtendOp: {
        value const a = kid(0);
        size_t const width = constant_of(r->child(1));
        return {static_cast<u128>(signed_bits(a)) & mask(width), width};
      }
      case Op::invertOp: {
        value const a = kid(0);
        return {~a.v & mask(a.width), a.width};
      }
      case Op::negateOp: {
        value const a = kid(0);
        return {u128{0} - a.v, a.width};
      }
      case Op::equalToZeroOp:
        return {bits(kid(0)) == 0 ? u128{1} : u128{0}, 1};
      case Op::addOp: {
        value const a = kid(0), b = kid(1);
        return {a.v + b.v, std::max(a.width, b.width)};
      }
      case Op::andOp: {
        value const a = kid(0), b = kid(1);
        return {bits(a) & bits(b), std::max(a.width, b.width)};
      }
      case Op::orOp: {
        value const a = kid(0), b = kid(1);
        return {bits(a) | bits(b), std::max(a.width, b.width)};
      }
      case Op::xorOp: {
        value const a = kid(0), b = kid(1);
        return {bits(a) ^ bits(b), std::max(a.width, b.width)};
      }
      case Op::concatOp: { // the first operand is the low part
        value const a = kid(0), b = kid(1);
        return {bits(a) | (bits(b) << a.width), a.width + b.width};
      }
      case Op::shiftLOp: {
        value const a = kid(0);
        u128 const n = bits(kid(1));
        return {n >= a.width ? 0 : (bits(a) << n) & mask(a.width), a.width};
      }
      case Op::shiftROp: {
        value const a = kid(0);
        u128 const n = bits(kid(1));
        return {n >= a.width ? 0 : bits(a) >> n, a.width};
      }
      case Op::shiftRArithOp: {
        value const a = kid(0);
        u128 const n = std::min(bits(kid(1)), static_cast<u128>(a.width - 1));
        return {static_cast<u128>(signed_bits(a) >> n) & mask(a.width), a.width};
      }
      case Op::ifOp: // only the selected operand is evaluated
        return bits(kid(0)) != 0 ? kid(1) : kid(2);
      case Op::sMultOp: {
        value const a = kid(0), b = kid(1);
        size_t const width = a.width + b.width;
        return {static_cast<u128>(signed_bits(a) * signed_bits(b)) & mask(width), width};
      }
      case Op::uMultOp: {
        value const a = kid(0), b = kid(1);
        size_t const width = a.width + b.width;
        return {(bits(a) * bits(b)) & mask(width), width};
      }
      case Op::sDivOp:
      case Op::sModOp:
      case Op::uDivOp:
      case Op::uModOp: {
        value const a = kid(0), b = kid(1);
        if(bits(b) == 0) {
          throw std::runtime_error("division by zero in " + r->format());
        }
        auto const op = r->val().op;
        u128 q;
        if(op == Op::sDivOp || op == Op::sModOp) {
          s128 const x = signed_bits(a), y = signed_bits(b);
          q = static_cast<u128>(op == Op::sDivOp ? x / y : x % y);
        } else {
          q = op == Op::uDivOp ? bits(a) / bits(b) : bits(a) % bits(b);
        }
        return {q & mask(a.width), a.width};
      }
      case Op::derefOp: {
        u128 const addr = bits(eval(r->child(0), m)) & mask(64);
        if(addr != mem_addr) {
          throw std::runtime_error("read of unknown memory in " + r->format());
        }
        size_t const width = r->val().size * 8;
        return {u128{m.memory} & mask(width), width};
      }
      default:
        throw std::runtime_error("no evaluation for " + r->val().format());
    }
  }

  value eval(Dyninst::AST::Ptr const &e, machine const &m) {
    switch(e->getID()) {
      case Dyninst::AST::V_ConstantAST: {
        df::Constant const &c = df::ConstantAST::convert(e)->val();
        size_t const width = c.size ? c.size : 64;
        return {u128{c.val} & mask(width), width};
      }
      case Dyninst::AST::V_VariableAST: {
        Dyninst::MachRegister const reg =
            df::VariableAST::convert(e)->val().reg.absloc().reg();
        auto const i = m.regs.find(reg);
        if(i == m.regs.end()) {
          throw std::runtime_error("read of unset register " + reg.name());
        }
        return {i->second, 64};
      }
      case Dyninst::AST::V_RoseAST:
        return eval_op(df::RoseAST::convert(e), m);
      default:
        throw std::runtime_error("unexpected node " + e->format());
    }
  }

  std::string hex(uint64_t v) {
    std::ostringstream s;
    s << "0x" << std::hex << v;
    return s.str();
  }

  // 'where' is a register, or memory when mem_width is not 0: then the instruction must
  // write mem_width bytes of 'expected' to mem_addr.
  struct test_case {
    char const *text;
    uint32_t encoding;
    uint64_t a1, a2, a5, memory;
    Dyninst::MachRegister where;
    size_t mem_width;
    uint64_t expected;
  };

  Dyninst::MachRegister const mem{};
  uint64_t const minus1 = ~uint64_t{0};

  // clang-format off
  test_case const cases[] = {
    // text                   encoding    a1                   a2                   a5      memory       where   mem expected
    // Instructions this test does not cover a fix for; they check the evaluation.
    {"add   a0, a1, a2",      0x00c58533, 5,                   7,                   0,      0,           rv::a0, 0,  12},
    {"sub   a0, a1, a2",      0x40c58533, 5,                   7,                   0,      0,           rv::a0, 0,  0xfffffffffffffffe},
    {"blt   a1, a2, 8",       0x00c5c463, minus1,              1,                   0,      0,           rv::pc, 0,  insn_addr + 8},

    // Loads and stores reach memory at the address in rs1 (a2).
    {"sw    a1, 0(a2)",       0x00b62023, 0x1122334455667788,  mem_addr,            0,      0,           mem,    4,  0x55667788},
    {"ld    a0, 0(a2)",       0x00063503, 0,                   mem_addr,            0,      0x1122334455667788, rv::a0, 0, 0x1122334455667788},

    // 32-bit operations take and give 32 bits.
    {"addiw a0, a1, 1",       0x0015851b, 0x7fffffff,          0,                   0,      0,           rv::a0, 0,  0xffffffff80000000},
    {"addw  a0, a1, a2",      0x00c5853b, 0x40000000,          0x40000000,          0,      0,           rv::a0, 0,  0xffffffff80000000},
    {"subw  a0, a1, a2",      0x40c5853b, 0x80000000,          0,                   0,      0,           rv::a0, 0,  0xffffffff80000000},
    {"slliw a0, a1, 3",       0x0035951b, 0x10000000,          0,                   0,      0,           rv::a0, 0,  0xffffffff80000000},
    {"mulw  a0, a1, a2",      0x02c5853b, 0x80000000,          1,                   0,      0,           rv::a0, 0,  0xffffffff80000000},

    // Shift amounts: 6 bits for the 64-bit shifts, 5 for the 32-bit ones.
    {"sll   a0, a1, a2",      0x00c59533, 1,                   40,                  0,      0,           rv::a0, 0,  0x10000000000},
    {"slli  a0, a1, 40",      0x02859513, 1,                   0,                   0,      0,           rv::a0, 0,  0x10000000000},
    {"sllw  a0, a1, a2",      0x00c5953b, 1,                   31,                  0,      0,           rv::a0, 0,  0xffffffff80000000},
    {"sraw  a0, a1, a2",      0x40c5d53b, 0x80000000,          4,                   0,      0,           rv::a0, 0,  0xfffffffff8000000},

    // The low and high halves of the 128-bit product, of signed or unsigned operands.
    {"mul   a0, a1, a2",      0x02c58533, minus1,              0x8000000000000000,  0,      0,           rv::a0, 0,  0x8000000000000000},
    {"mulh  a0, a1, a2",      0x02c59533, minus1,              2,                   0,      0,           rv::a0, 0,  minus1},

    // jalr jumps to a5 + 8 with bit 0 cleared and links the next instruction.
    {"jalr  ra, 8(a5)",       0x008780e7, 0,                   0,                   0x1001, 0,           rv::pc, 0,  0x1008},
    {"jalr  ra, 8(a5)",       0x008780e7, 0,                   0,                   0x1001, 0,           rv::ra, 0,  insn_addr + 4},

    // sc stores rs2 (a1) at the address in rs1 (a2).
    {"sc.w  a0, a1, (a2)",    0x18b6252f, 0x1122334455667788,  mem_addr,            0,      0xdeadbeef,  mem,    4,  0x55667788},
    {"sc.d  a0, a1, (a2)",    0x18b6352f, 0x1122334455667788,  mem_addr,            0,      0xdeadbeef,  mem,    8,  0x1122334455667788},
  };
  // clang-format on

  // Reports the case and returns false if the instruction does not write the expected
  // value.
  bool check(test_case const &c) {
    using namespace Dyninst;
    std::array<unsigned char, 4> bytes{};
    for(int i = 0; i < 4; ++i) {
      bytes[i] = static_cast<unsigned char>(c.encoding >> (8 * i));
    }
    InstructionAPI::InstructionDecoder dec(bytes.data(), bytes.size(), Arch_riscv64);
    InstructionAPI::Instruction const insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << c.text << ": could not decode\n";
      return false;
    }

    AbsRegion const where =
        c.mem_width ? AbsRegion(Absloc::Heap) : AbsRegion(Absloc(c.where));
    Assignment::Ptr a = Assignment::makeAssignment(insn, insn_addr, nullptr, nullptr, where);
    AST::Ptr const e = DataflowAPI::SymEval::expand(a, false).first;
    std::string const name = c.mem_width ? "memory" : c.where.name();
    if(!e) {
      std::cerr << c.text << ": " << name << " not written\n";
      return false;
    }

    machine const m{{{rv::a1, c.a1}, {rv::a2, c.a2}, {rv::a5, c.a5}, {rv::pc, insn_addr}},
                    c.memory};
    try {
      if(c.mem_width) {
        AST::Ptr const address = a->out().generator();
        if(!address) {
          std::cerr << c.text << ": no address for the memory write\n";
          return false;
        }
        uint64_t const at = static_cast<uint64_t>(bits(eval(address, m)));
        if(at != mem_addr) {
          std::cerr << c.text << ": writes memory at " << hex(at) << ", expected "
                    << hex(mem_addr) << '\n';
          return false;
        }
      }
      size_t const width = c.mem_width ? c.mem_width * 8 : 64;
      uint64_t const got = static_cast<uint64_t>(eval(e, m).v & mask(width));
      if(got != c.expected) {
        std::cerr << c.text << ": " << name << " = " << hex(got) << ", expected "
                  << hex(c.expected) << "\n  " << e->format() << '\n';
        return false;
      }
    } catch(std::exception const &x) {
      std::cerr << c.text << ": " << x.what() << "\n  " << e->format() << '\n';
      return false;
    }
    return true;
  }
}

int main() {
  bool ok = true;
  for(auto const &c : cases) {
    ok = check(c) && ok;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
