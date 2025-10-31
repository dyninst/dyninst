#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "decoder.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "Result.h"
#include "type_conversion.h"
#include "x86/register_xlat.h"

#include <map>
#include <vector>

namespace {

  namespace di = Dyninst::InstructionAPI;

  struct implicit_state final {
    bool read, written;
  };

  std::map<x86_reg, implicit_state> implicit_registers(cs_detail const *);

  struct eflags_t final {
    Dyninst::MachRegister reg;
    implicit_state state;
  };

  std::vector<eflags_t> expand_flags(cs_insn const *, x86_reg, cs_mode);

  struct mem_op {
    Dyninst::MachRegister reg;
    bool read, written;

    mem_op(Dyninst::MachRegister reg_, bool read_, bool written_) : reg{reg_}, read{read_}, written{written_} {}
  };

  std::vector<mem_op> get_implicit_memory_ops(entryID, Dyninst::Architecture);
}

namespace Dyninst { namespace InstructionAPI {

  void x86_decoder::decode_implicit(Instruction &insn) {
    /* Decode _implicit_ operands
     *
     * These are operands which are not part of the opcode. Some opcodes
     * have both explicit and implicit operands. For example,
     *
     *   add r1, r2       ; {e,r}flags is written to implicitly
     *   jmp -64          ; PC/IP is written to implicitly
     *
     * Some have only implicit:
     *
     *   pop  ; modifies stack pointer {e,r}sp
     */
    for(auto r : implicit_registers(disassembler.insn->detail)) {
      constexpr bool is_implicit = true;
      x86_reg reg = r.first;
      if(reg == X86_REG_EFLAGS || reg == X86_REG_FPSW) {
        for(auto fr : expand_flags(disassembler.insn, reg, this->mode)) {
          if(!fr.state.read && !fr.state.written) {
            continue; // don't report untouched registers
          }
          auto regAST = makeRegisterExpression(fr.reg);
          insn.appendOperand(std::move(regAST), fr.state.read, fr.state.written, is_implicit);
        }
        auto regAST = makeRegisterExpression(x86::translate_register(reg, this->mode));
        bool const is_written = cs_reg_write(disassembler.handle, disassembler.insn, reg);
        bool const is_read = cs_reg_read(disassembler.handle, disassembler.insn, reg);
        insn.appendOperand(std::move(regAST), is_read, is_written, is_implicit);
      } else {
        if(this->mode == CS_MODE_64) {
          // Capstone incorrectly reports %eip instead of %rip for conditional branches in 64-bit mode
          // See https://github.com/capstone-engine/capstone/issues/2691
          if(reg == X86_REG_EIP && insn.isBranch() && insn.isConditional()) {
            reg = X86_REG_RIP;
          }

          // Capstone incorrectly reports %eip instead of %rip for calls/interrupts in 64-bit mode
          // See https://github.com/capstone-engine/capstone/pull/2773
          if(insn.isCall() || insn.isInterrupt() || insn.isReturn()) {
            if(reg == X86_REG_EIP) {
              reg = X86_REG_RIP;
            } else if(reg == X86_REG_ESP) {
              reg = X86_REG_RSP;
            }
          }
        }
        MachRegister mreg = x86::translate_register(reg, this->mode);
        auto regAST = makeRegisterExpression(mreg);
        implicit_state s = r.second;
        insn.appendOperand(regAST, s.read, s.written, is_implicit);
      }
    }

    // Capstone doesn't track implicit memory reads/writes
    // See https://github.com/capstone-engine/capstone/pull/2578
    {
      auto const id = insn.getOperation().getID();
      constexpr bool is_implicit = true;
      auto const addr_size = disassembler.insn->detail->x86.addr_size;
      auto const type = size_to_type_unsigned(addr_size);

      for(auto const &op : get_implicit_memory_ops(id, this->m_Arch)) {
        auto reg = makeRegisterExpression(op.reg);
        auto expr = makeDereferenceExpression(std::move(reg), type);
        insn.appendOperand(std::move(expr), op.read, op.written, is_implicit);
      }

      // Return instructions (including return-from-interrupt) pop the return
      // address from the stack. Mark that as their ControlFlow Target.
      if(insn.isReturn()) {
        constexpr bool is_call = true;
        constexpr bool is_indirect = true;
        constexpr bool is_conditional = true;
        constexpr bool is_fallthrough = true;
        auto sp(makeRegisterExpression(MachRegister::getStackPointer(m_Arch)));
        auto expr = makeDereferenceExpression(std::move(sp), type);
        insn.addSuccessor(std::move(sp), !is_call, !is_indirect, !is_conditional, !is_fallthrough, is_implicit);
      }
    }

    // As a special case, 'push' also writes at the new value of the stack pointer
    {
      auto const addr_size = disassembler.insn->detail->x86.addr_size;
      auto const type = size_to_type_signed(addr_size);
      constexpr bool is_implicit = true;
      constexpr bool is_read = true;
      constexpr bool is_written = true;

      switch(insn.getOperation().getID()) {
        case e_push:
        case e_pusha:
        case e_pushf: {
          auto res = Result(type, -addr_size);
          auto sp = MachRegister::getStackPointer(this->m_Arch);
          auto reg = makeRegisterExpression(sp);
          auto offset = Immediate::makeImmediate(res);
          auto expr = makeAddExpression(reg, offset, type);
          insn.appendOperand(std::move(expr), !is_read, is_written, is_implicit);
        } break;
        default:
          break;
      }
    }
  }

}}

namespace {

  std::map<x86_reg, implicit_state> implicit_registers(cs_detail const *d) {
    std::map<x86_reg, implicit_state> regs;
    for(int i = 0; i < d->regs_read_count; ++i) {
      regs.emplace(static_cast<x86_reg>(d->regs_read[i]), implicit_state{true, false});
    }
    for(int i = 0; i < d->regs_write_count; ++i) {
      auto res = regs.emplace(static_cast<x86_reg>(d->regs_write[i]), implicit_state{false, true});
      if(!res.second) {
        // Register already existed, so was read. Mark it written.
        res.first->second.written = true;
      }
    }
    return regs;
  }

  // clang-format off
  std::vector<eflags_t> expand_eflags(uint64_t eflags, cs_mode m) {
    /*
     *  There are six possible types of access modeled by Capstone:
     *
     *    MODIFY    - written to
     *    PRIOR     - this is never used in Capstone
     *    RESET     - set to zero
     *    SET       - written to
     *    TEST      - read from
     *    UNDEFINED - no guarantees about the state of the flag
     *
     */

    #define READS(REG)            !!(eflags & X86_EFLAGS_TEST_##REG )
    #define WRITES(REG)           !!(eflags & (X86_EFLAGS_SET_##REG | X86_EFLAGS_RESET_##REG | X86_EFLAGS_MODIFY_##REG))
    #define WRITES_NOT_SETS(REG)  !!(eflags & (                       X86_EFLAGS_RESET_##REG | X86_EFLAGS_MODIFY_##REG))

    if(m == CS_MODE_64) {
      return {
        {Dyninst::x86_64::af,  {READS(AF), WRITES(AF)}},
        {Dyninst::x86_64::cf,  {READS(CF), WRITES(CF)}},
        {Dyninst::x86_64::sf,  {READS(SF), WRITES(SF)}},
        {Dyninst::x86_64::zf,  {READS(ZF), WRITES(ZF)}},
        {Dyninst::x86_64::pf,  {READS(PF), WRITES(PF)}},
        {Dyninst::x86_64::of,  {READS(OF), WRITES(OF)}},
        {Dyninst::x86_64::tf,  {READS(TF), WRITES_NOT_SETS(TF)}},
        {Dyninst::x86_64::if_, {READS(IF), WRITES(IF)}},
        {Dyninst::x86_64::df,  {READS(DF), WRITES(DF)}},
        {Dyninst::x86_64::nt_, {READS(NT), WRITES_NOT_SETS(NT)}},
        {Dyninst::x86_64::rf,  {READS(RF), WRITES_NOT_SETS(RF)}}
      };
    }

    return {
      {Dyninst::x86::af,  {READS(AF), WRITES(AF)}},
      {Dyninst::x86::cf,  {READS(CF), WRITES(CF)}},
      {Dyninst::x86::sf,  {READS(SF), WRITES(SF)}},
      {Dyninst::x86::zf,  {READS(ZF), WRITES(ZF)}},
      {Dyninst::x86::pf,  {READS(PF), WRITES(PF)}},
      {Dyninst::x86::of,  {READS(OF), WRITES(OF)}},
      {Dyninst::x86::tf,  {READS(TF), WRITES_NOT_SETS(TF)}},
      {Dyninst::x86::if_, {READS(IF), WRITES(IF)}},
      {Dyninst::x86::df,  {READS(DF), WRITES(DF)}},
      {Dyninst::x86::nt_, {READS(NT), WRITES_NOT_SETS(NT)}},
      {Dyninst::x86::rf,  {READS(RF), WRITES_NOT_SETS(RF)}}
    };

    #undef READS
    #undef WRITES
    #undef WRITES_NOT_SETS
  }

  std::vector<eflags_t> expand_fpflags(uint64_t eflags, cs_mode m) {
    /*
     *  There are six possible types of access modeled by Capstone:
     *
     *    MODIFY    - written to
     *    PRIOR     - this is never used in Capstone
     *    RESET     - set to zero
     *    SET       - written to
     *    TEST      - read from
     *    UNDEFINED - no guarantees about the state of the flag
     *
     */
    #define READS(REG)  !!(eflags & X86_FPU_FLAGS_TEST_##REG )
    #define WRITES(REG) !!(eflags & (X86_FPU_FLAGS_SET_##REG | X86_FPU_FLAGS_RESET_##REG | X86_FPU_FLAGS_MODIFY_##REG))

    if(m == CS_MODE_64) {
      return {
        {Dyninst::x86_64::cr0,  {READS(C0), WRITES(C0)}},
        {Dyninst::x86_64::cr1,  {READS(C1), WRITES(C1)}},
        {Dyninst::x86_64::cr2,  {READS(C2), WRITES(C2)}},
        {Dyninst::x86_64::cr3,  {READS(C3), WRITES(C3)}},
      };
    }

    return {
        {Dyninst::x86::cr0,  {READS(C0), WRITES(C0)}},
        {Dyninst::x86::cr1,  {READS(C1), WRITES(C1)}},
        {Dyninst::x86::cr2,  {READS(C2), WRITES(C2)}},
        {Dyninst::x86::cr3,  {READS(C3), WRITES(C3)}},
    };
    #undef READS
    #undef WRITES
  }

  // clang-format on

  std::vector<eflags_t> expand_flags(cs_insn const *insn, x86_reg reg, cs_mode mode) {
    /*
     * Capstone doesn't currently have a way to represent the individual registers
     * written by the f[u]comi[p] x87 compare instructions.
     */
    auto const flags = [&insn, reg]() -> uint64_t {
      switch(insn->id) {
        case X86_INS_FCOMI:
        case X86_INS_FCOMPI:
        case X86_INS_FUCOMI:
        case X86_INS_FUCOMPI:
          return X86_EFLAGS_RESET_OF | X86_EFLAGS_RESET_SF | X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_RESET_AF |
                 X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_CF;
      }
      if(reg == X86_REG_EFLAGS) {
        return insn->detail->x86.eflags;
      }
      return insn->detail->x86.fpu_flags;
    }();

    if(reg == X86_REG_EFLAGS) {
      return expand_eflags(flags, mode);
    }
    return expand_fpflags(flags, mode);
  }

  std::vector<mem_op> get_implicit_memory_ops(entryID id, Dyninst::Architecture arch) {
    bool const is_64 = (arch == Dyninst::Arch_x86_64);

    std::vector<mem_op> ops{};

    constexpr bool is_read = true;
    constexpr bool is_written = true;

    auto sp = Dyninst::MachRegister::getStackPointer(arch);
    auto ebp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;

    auto ss = is_64 ? Dyninst::x86_64::ss : Dyninst::x86::ss;

    switch(id) {
      // Only reads stack
      case e_pop:
      case e_popal:
      case e_popaw:
      case e_popf:
      case e_popfd:
      case e_popfq:
        ops.emplace_back(sp, is_read, !is_written);
        break;

      // Only writes stack
      case e_call:
      case e_lcall:
      case e_push:
      case e_pushal:
      case e_pushf:
      case e_int3:
      case e_int:
      case e_int1:
      case e_into:
      case e_enter:
        ops.emplace_back(sp, !is_read, is_written);
        break;

      // All others
      case e_leave:
        ops.emplace_back(sp, is_read, !is_written);
        ops.emplace_back(ebp, is_read, !is_written);
        break;

      case e_ret:
      case e_retf:
      case e_iret:
        ops.emplace_back(sp, is_read, !is_written);
        ops.emplace_back(ss, is_read, !is_written);
        break;

      default:
        break;
    }

    return ops;
  }
}
