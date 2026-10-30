// Checks the AMDGPU (gfx908) scalar adds and PC writes by value: SymEval expands what an
// instruction writes to a register, the scalar registers it reads are replaced by known
// values, and SymbolicExpression::SimplifyAnAST, which call-target analysis uses, must fold
// the expression to the value the instruction computes.
//
// s_add_u32 and s_addc_u32 are 32-bit adds whose carry out of bit 31 is SCC; the sequence
// from s_getpc_b64 through s_swappc_b64 below is the one call_fn in
// parseAPI_IndirectControlFlowAnalysis_908.gpubin uses to call a function 0x18d4 below it.

#include "Absloc.h"
#include "ConstantAST.h"
#include "InstructionDecoder.h"
#include "RoseAST.h"
#include "SymEval.h"
#include "VariableAST.h"
#include "parseAPI/src/SymbolicExpression.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {
  namespace gfx = Dyninst::amdgpu_gfx908;
  namespace df = Dyninst::DataflowAPI;
  using Dyninst::AST;

  constexpr uint64_t insn_addr = 0x6140;

  using regs = std::map<Dyninst::MachRegister, uint64_t>;

  struct test_case {
    char const *text;
    std::vector<uint32_t> words; // the instruction, then its literal if it has one
    regs in;
    Dyninst::MachRegister where;
    uint64_t expected;
  };

  std::string hex(uint64_t v) {
    std::ostringstream s;
    s << "0x" << std::hex << v;
    return s.str();
  }

  // e with the registers in 'in' replaced by constants, or null if it reads a register
  // not in 'in' or memory.
  AST::Ptr with_constants(AST::Ptr const &e, regs const &in) {
    switch(e->getID()) {
      case AST::V_VariableAST: {
        Dyninst::MachRegister const reg = df::VariableAST::convert(e)->val().reg.absloc().reg();
        auto const i = in.find(reg);
        if(i == in.end()) {
          return nullptr;
        }
        return df::ConstantAST::create(df::Constant(i->second, reg.size() * 8));
      }
      case AST::V_RoseAST: {
        df::RoseAST::Ptr const r = df::RoseAST::convert(e);
        if(r->val().op == df::ROSEOperation::derefOp) {
          return nullptr;
        }
        AST::Children kids;
        for(unsigned i = 0; i < r->numChildren(); ++i) {
          AST::Ptr const k = with_constants(r->child(i), in);
          if(!k) {
            return nullptr;
          }
          kids.push_back(k);
        }
        return df::RoseAST::create(r->val(), kids);
      }
      default:
        return e;
    }
  }

  bool check(test_case const &c) {
    using namespace Dyninst;
    std::vector<unsigned char> bytes;
    for(uint32_t w : c.words) {
      for(int i = 0; i < 4; ++i) {
        bytes.push_back(static_cast<unsigned char>(w >> (8 * i)));
      }
    }
    InstructionAPI::InstructionDecoder dec(bytes.data(), bytes.size(), Arch_amdgpu_gfx908);
    InstructionAPI::Instruction const insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << c.text << ": could not decode\n";
      return false;
    }
    Assignment::Ptr a = Assignment::makeAssignment(insn, insn_addr, nullptr, nullptr, AbsRegion(Absloc(c.where)));
    AST::Ptr const e = DataflowAPI::SymEval::expand(a, false).first;
    if(!e) {
      std::cerr << c.text << ": " << c.where.name() << " not written\n";
      return false;
    }
    AST::Ptr const k = with_constants(e, c.in);
    if(!k) {
      std::cerr << c.text << ": " << c.where.name() << " reads an unset register or memory\n  " << e->format() << '\n';
      return false;
    }
    SymbolicExpression se;
    se.cs = nullptr;
    se.cr = nullptr;
    AST::Ptr const folded = se.SimplifyAnAST(k, insn_addr);
    if(folded->getID() != AST::V_ConstantAST) {
      std::cerr << c.text << ": " << c.where.name() << " does not fold: " << folded->format() << "\n  "
                << e->format() << '\n';
      return false;
    }
    df::Constant const v = DataflowAPI::ConstantAST::convert(folded)->val();
    size_t const width = c.where.size() * 8;
    uint64_t const got = width < 64 ? v.val & ((uint64_t{1} << width) - 1) : v.val;
    if(got != c.expected) {
      std::cerr << c.text << ": " << c.where.name() << " = " << hex(got) << ", expected " << hex(c.expected)
                << "\n  " << e->format() << '\n';
      return false;
    }
    return true;
  }
}

int main() {
  // clang-format off
  std::vector<test_case> const cases = {
    // call_fn: s4 = low half of the PC (0x6150) + 0xffffe72c, which carries; s5 = 0 - 1 + carry.
    {"s_add_u32 s4, s4, 0xffffe72c",   {0x8004ff04, 0xffffe72c}, {{gfx::s4, 0x6150}},                          gfx::s4,      0x487c},
    {"s_add_u32 s4, s4, 0xffffe72c",   {0x8004ff04, 0xffffe72c}, {{gfx::s4, 0x6150}},                          gfx::src_scc, 1},
    {"s_addc_u32 s5, s5, -1",          {0x8205ff05, 0xffffffff}, {{gfx::s5, 0}, {gfx::src_scc, 1}},            gfx::s5,      0},
    {"s_addc_u32 s5, s5, -1",          {0x8205ff05, 0xffffffff}, {{gfx::s5, 0}, {gfx::src_scc, 1}},            gfx::src_scc, 1},
    {"s_swappc_b64 s[30:31], s[4:5]",  {0xbe9e1e04},             {{gfx::s4, 0x487c}, {gfx::s5, 0}},            gfx::pc_all,  0x487c},

    // No carry out of bit 31.
    {"s_add_u32 s8, s24, 16",          {0x80089018},             {{gfx::s24, 0}},                              gfx::s8,      0x10},
    {"s_add_u32 s8, s24, 16",          {0x80089018},             {{gfx::s24, 0}},                              gfx::src_scc, 0},
    {"s_add_u32 s8, s24, 16",          {0x80089018},             {{gfx::s24, 0xfffffff0}},                     gfx::src_scc, 1},
    // The carry of s_addc_u32 is the carry out of bit 31 of its 32-bit sum.
    {"s_addc_u32 s9, s25, 0",          {0x82098019},             {{gfx::s25, 0xffffffff}, {gfx::src_scc, 1}},  gfx::s9,      0},
    {"s_addc_u32 s9, s25, 0",          {0x82098019},             {{gfx::s25, 0xffffffff}, {gfx::src_scc, 1}},  gfx::src_scc, 1},
    {"s_addc_u32 s9, s25, 0",          {0x82098019},             {{gfx::s25, 0x7fffffff}, {gfx::src_scc, 1}},  gfx::src_scc, 0},

    // The low half of the target is zero-extended: bit 31 set does not reach the high half.
    {"s_setpc_b64 s[4:5]",             {0xbe801d04},             {{gfx::s4, 0x80000000}, {gfx::s5, 0}},        gfx::pc_all,  0x80000000},
    {"s_setpc_b64 s[4:5]",             {0xbe801d04},             {{gfx::s4, 0x80000000}, {gfx::s5, 1}},        gfx::pc_all,  0x180000000},
    {"s_swappc_b64 s[30:31], s[4:5]",  {0xbe9e1e04},             {{gfx::s4, 0x80000000}, {gfx::s5, 0}},        gfx::pc_all,  0x80000000},
  };
  // clang-format on

  bool ok = true;
  for(auto const &c : cases) {
    ok = check(c) && ok;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
