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

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "categories.h"
#include "debug.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "syscalls.h"
#include "type_conversion.h"
#include "x86/decoder.h"
#include "x86/opcode_xlat.h"
#include "x86/register_xlat.h"

#include <algorithm>
#include <cstdint>

/***************************************************************************
 * The work here is based on
 *
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  June 2025
 *
 *  Intel Architecture Instruction Set Extensions and Future Features (IISE)
 *  May 2021
 *
 *  AMD64 Architecture Programmer’s Manual (AMDAPM)
 *  Revision 3.33
 *  November 2021
 *
 ***************************************************************************/

namespace di = Dyninst::InstructionAPI;

namespace {

  bool is_cft(di::Instruction const &insn) {
    return insn.isBranch() || insn.isCall();
  }

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

  x86_decoder::x86_decoder(Dyninst::Architecture a) : InstructionDecoderImpl(a) {

    mode = (a == Dyninst::Arch_x86_64) ? CS_MODE_64 : CS_MODE_32;

    cs_open(CS_ARCH_X86, this->mode, &disassembler.handle);
    cs_option(disassembler.handle, CS_OPT_DETAIL, CS_OPT_ON);
    disassembler.insn = cs_malloc(disassembler.handle);
  }


  x86_decoder::~x86_decoder() {
    cs_free(disassembler.insn, 1);
    cs_close(&disassembler.handle);
  }

  Instruction x86_decoder::decode(InstructionDecoder::buffer &buf) {
    auto *code = buf.start;
    size_t codeSize = buf.end - buf.start;
    uint64_t cap_addr = 0;

    // The iterator form of disassembly allows reuse of the instruction object, reducing
    // the number of memory allocations.
    if(!cs_disasm_iter(disassembler.handle, &code, &codeSize, &cap_addr, disassembler.insn)) {
      // Gap parsing can trigger this case. In particular, when it encounters prefixes in an invalid
      // order. Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67,
      // etc) we'll reject the instruction as invalid and send it back with no entry.  Since this is
      // a common byte sequence to see in, for example, ASCII strings, we want to simply accept this
      // and move on.
      decode_printf("Failed to disassemble instruction at %p: %s\n", code, cs_strerror(cs_errno(disassembler.handle)));
      return {};
    }

    entryID e = x86::translate_opcode(static_cast<x86_insn>(disassembler.insn->id));
    auto op = Operation(e, disassembler.insn->mnemonic, m_Arch);
    buf.start += disassembler.insn->size;
    unsigned int decodedSize = buf.start - code;
    Instruction insn(std::move(op), decodedSize, code, m_Arch);
    decode_operands(insn);
    return insn;
  }

  void x86_decoder::decode_operands(Instruction& insn) {
    // Categories must be decoded before anything else since they are used
    // in the other decoding steps.
    insn.categories = x86::decode_categories(insn, disassembler);

    /* Decode _explicit_ operands
     *
     * There are three types:
     *
     *   add r1, r2       ; r1, r2 are both X86_OP_REG
     *   jmp -64          ; -64 is X86_OP_IMM
     *   mov r1, [0x33]   ; r1 is X86_OP_REG, 0x33 is X86_OP_MEM
     */
    auto *d = disassembler.insn->detail;
    for(uint8_t i = 0; i < d->x86.op_count; ++i) {
      cs_x86_op const &operand = d->x86.operands[i];
      switch(operand.type) {
        case X86_OP_REG:
          decode_reg(insn, operand);
          break;
        case X86_OP_IMM:
          decode_imm(insn, operand);
          break;
        case X86_OP_MEM:
          decode_mem(insn, operand);
          break;
        case X86_OP_INVALID:
          decode_printf("[0x%lx %s %s] has an invalid operand.\n", disassembler.insn->address,
                        disassembler.insn->mnemonic, disassembler.insn->op_str);
          break;
      }
    }

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
    for(auto r : implicit_registers(d)) {
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

    // Capstone doesn't fully track eflags for interrupt instructions
    if(insn.isInterrupt()) {
      auto add_reg = [insn, this](MachRegister reg, bool isread, bool iswritten) {
        constexpr bool is_implicit = true;
        auto regAST = makeRegisterExpression(reg);
        insn.appendOperand(regAST, isread, iswritten, is_implicit);
      };
      constexpr bool is_read = true;
      constexpr bool is_written = true;
      auto ac = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::ac : Dyninst::x86_64::ac;
      auto flagc = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::flagc : Dyninst::x86_64::flagc;
      auto flagd = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::flagd : Dyninst::x86_64::flagd;
      auto vm = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::vm : Dyninst::x86_64::vm;
      auto vif = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::vif : Dyninst::x86_64::vif;
      auto of = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::of : Dyninst::x86_64::of;
      auto flags = (this->m_Arch == Dyninst::Arch_x86) ? Dyninst::x86::flags : Dyninst::x86_64::flags;

      auto const id = insn.getOperation().getID();
      if(id == e_int || id == e_int3) {
        add_reg(ac, !is_read, is_written);
        add_reg(flagc, is_read, !is_written);
        add_reg(flagd, is_read, !is_written);
        add_reg(vm, is_read, is_written);
        add_reg(vif, !is_read, is_written);
        add_reg(flags, is_read, is_written);
      } else if(id == e_into) {
        add_reg(ac, !is_read, is_written);
        add_reg(flagc, is_read, !is_written);
        add_reg(flagd, is_read, !is_written);
        add_reg(vm, is_read, is_written);
        add_reg(of, is_read, !is_written);
        add_reg(flags, is_read, is_written);
      } // int1 does not use eflags
    }
  }

  void x86_decoder::decode_reg(Instruction &insn, cs_x86_op const &operand) {
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;
    constexpr bool is_conditional = true;
    constexpr bool is_fallthrough = true;

    auto regAST = makeRegisterExpression(x86::translate_register(operand.reg, mode));

    const bool isCall = insn.isCall();
    if(insn.isBranch() || isCall) {
      insn.addSuccessor(regAST, isCall, is_indirect, !is_conditional, !is_fallthrough);
      return;
    }

    // It's an error if an operand is neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool is_read = ((operand.access & CS_AC_READ) != 0);
    bool is_written = ((operand.access & CS_AC_WRITE) != 0);

    if(!is_read && !is_written) {
      is_read = is_written = true;
    }

    // Capstone read/write isn't populated for 'dx' for the I/O input string instructions
    switch(insn.getOperation().getID()) {
      case e_insb:
      case e_insw:
      case e_insd:
      case e_outsb:
      case e_outsw:
      case e_outsd:
        is_read = true;
        is_written = false;
        break;
      default:
        break;
    }

    insn.appendOperand(regAST, is_read, is_written, !is_implicit);
  }

  void x86_decoder::decode_imm(Instruction &insn, cs_x86_op const &operand) {
    auto const type = size_to_type_signed(operand.size);
    auto imm = Immediate::makeImmediate(Result(type, operand.imm));

    constexpr bool is_read = true;
    constexpr bool is_written = true;
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;

    if(!is_cft(insn)) {
      insn.appendOperand(std::move(imm), !is_read, !is_written, !is_implicit);
      return;
    }

    auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

    auto const is_call_insn = insn.isCall();
    auto const is_conditional_insn = insn.isConditional();
    auto const usesRelativeAddressing = cs_insn_group(disassembler.handle, disassembler.insn, CS_GRP_BRANCH_RELATIVE);
    bool const is_fallthrough_insn = insn.allowsFallThrough();

    if(usesRelativeAddressing) {
      insn.appendOperand(imm, is_read, !is_written, !is_implicit);
      insn.appendOperand(IP, is_read, is_written, is_implicit);
      // Capstone adjusts the offset to account for the current instruction's length, so we can
      // just create an addition AST expression here.
      auto target(makeAddExpression(IP, imm, s64));
      insn.addSuccessor(std::move(target), is_call_insn, !is_indirect, is_conditional_insn, is_fallthrough_insn);
    } else {
      insn.addSuccessor(std::move(imm), is_call_insn, !is_indirect, is_conditional_insn, is_fallthrough_insn);
    }
    if(is_conditional_insn) {
      constexpr bool is_call = true;
      constexpr bool is_conditional = true;
      insn.addSuccessor(std::move(IP), !is_call, !is_indirect, is_conditional, is_fallthrough_insn);
    }
  }

  void x86_decoder::decode_mem(Instruction &insn, cs_x86_op const &operand) {
    /*
     *  SDM 3.7.5 Specifying an Offset
     *
     *  The offset part of a memory address can be specified directly as a static value (called
     *  a displacement) or through an address computation made up of one or more of the following
     *  components:
     *
     *  - Displacement: An 8-, 16-, or 32-bit value.
     *  - Base:         The value in a general-purpose register.
     *  - Index:        The value in a general-purpose register.
     *  - Scale factor: A value of 2, 4, or 8 that is multiplied by the index value.
     *
     *  Offset = Base + (Index * Scale) + Displacement
     *
     *  The offset which results from adding these components is called an effective address. Each
     *  of these components can have either a positive or negative (2s complement) value, with the
     *  exception of the scaling factor.
     */

    auto disp = [&]() -> Expression::Ptr {
      // Capstone: Displacement value, valid if encoding.disp_offset != 0
      if(disassembler.insn->detail->x86.encoding.disp_offset == 0) {
        return {};
      }
      auto const size = disassembler.insn->detail->x86.encoding.disp_size;
      auto const type = size_to_type_signed(size);
      return Immediate::makeImmediate(Result(type, operand.mem.disp));
    }();

    auto base = [&]() -> Expression::Ptr {
      if(operand.mem.base == X86_REG_INVALID) {
        return {};
      }
      auto const basereg = x86::translate_register(operand.mem.base, this->mode);
      return makeRegisterExpression(basereg);
    }();

    auto index = [&]() -> Expression::Ptr {
      if(operand.mem.index == X86_REG_INVALID) {
        return {};
      }
      if(operand.mem.index == X86_REG_EIZ || operand.mem.index == X86_REG_RIZ) {
        // {E,R}IZ is a pseudo register used when the index register is explicitly
        // set to zero in the opcode. It's used by decoders, but has no meaning
        // for the ISA.
        return {};
      }
      auto reg = x86::translate_register(operand.mem.index, this->mode);
      return makeRegisterExpression(reg);
    }();

    auto scaled_index = [&]() -> Expression::Ptr {
      // SDM: A scale factor may be used only when an index also is used
      if(!index) {
        return {};
      }
      auto scale = Immediate::makeImmediate(Result(u8, operand.mem.scale));

      // The size of the Index register *should* be the same as the size
      // of the operand, but I can't find a requirement for that in the
      // SDM so don't assume any sizes here.
      auto reg = x86::translate_register(operand.mem.index, this->mode);
      auto const type = size_to_type_signed(reg.size());
      return makeMultiplyExpression(std::move(index), std::move(scale), type);
    }();

    // Calculate `(Index * Scale) + Displacement`
    auto rhs = [&]() -> Expression::Ptr {
      if(!scaled_index) {
        return disp;
      }
      if(!disp) {
        return scaled_index;
      }

      // As with scaled_index, this shouldn't be necessary but be careful.
      auto const index_reg = x86::translate_register(operand.mem.index, this->mode);
      auto const disp_size = disassembler.insn->detail->x86.encoding.disp_size;
      auto const size = std::max(index_reg.size(), static_cast<std::uint32_t>(disp_size));
      auto const type = size_to_type_signed(size);
      return makeAddExpression(std::move(scaled_index), std::move(disp), type);
    }();

    auto effectiveAddr = [&]() -> Expression::Ptr {
      if(!rhs) {
        return base;
      }
      if(!base) {
        return rhs;
      }
      auto const type = size_to_type_signed(operand.size);
      return makeAddExpression(std::move(base), std::move(rhs), type);
    }();

    auto finalAddress = [&]() -> Expression::Ptr {
      if(operand.mem.segment == X86_REG_INVALID) {
        return effectiveAddr;
      }

      // Real-Address Mode `Seg + effectiveAddr`
      auto const reg = x86::translate_register(operand.mem.segment, this->mode);
      auto segReg = makeRegisterExpression(reg);

      // It's an absolute address, so it's unsigned
      auto const type = size_to_type_unsigned(operand.size);
      return makeAddExpression(std::move(segReg), std::move(effectiveAddr), type);
    }();

//      std::cerr << "Got memory ref '" << finalAddress->format() << "'\n";

    constexpr bool is_implicit = true;

    // Special cases that read and write, but Capstone only has one access specifier
    {
      constexpr bool is_read = true;
      constexpr bool is_written = true;

      switch(insn.getOperation().getID()) {
        case e_cmpxchg:
        case e_cmpxchg16b:
        case e_cmpxchg8b:
        case e_xchg: {
          // exchange instructions read and write their memory operand
          auto const type = size_to_type_signed(operand.size);
          auto deref = makeDereferenceExpression(std::move(finalAddress), type);
          insn.appendOperand(std::move(deref), is_read, is_written, !is_implicit);
          return;
        }
        case e_lea: {
          // LEA (Load Effective Address) does not dereference its memory operand
          insn.appendOperand(std::move(finalAddress), !is_read, !is_written, !is_implicit);
          return;
        }
        default:
          break;
      }
    }

    // Capstone may report register operands as neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool is_read = ((operand.access & CS_AC_READ) != 0);
    bool is_written = ((operand.access & CS_AC_WRITE) != 0);
    if(!is_read && !is_written) {
      is_read = is_written = true;
    }

    // has a control flow target (e.g., `call [rbx + rax * 2 + 0xff]`)
    if(is_cft(insn)) {
      auto const is_call = insn.isCall();
      auto const is_fallthrough = insn.allowsFallThrough();
      auto const type = size_to_type_signed(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.addSuccessor(std::move(expr), is_call, true, false, is_fallthrough, !is_implicit);
      return;
    }

    // x87 instruction
    if(cs_insn_group(disassembler.handle, disassembler.insn, X86_GRP_FPU)) {
      auto const type = size_to_type_float(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
      return;
    }

    // vector instruction
    if(insn.isVector()) {
      auto const type = size_to_type_memory(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
      return;
    }

    is_written = [is_written, &insn]() {
      switch(insn.getOperation().getID()) {
        // String comparisons write to their destination registers, but not to memory
        case e_cmpsb:
        case e_cmpsw:
        case e_cmpsd:
        case e_cmpsq:
        case e_lodsb:
        case e_lodsw:
        case e_lodsd:
        case e_lodsq:
        case e_outsb:
        case e_outsw:
        case e_outsd:
          return false;
        default:
          return is_written;
      }
    }();

    is_read = [is_read, &insn]() {
      switch(insn.getOperation().getID()) {
        // 'Input from Port to String' instructions read their source registers, but not from memory
        case e_insb:
        case e_insw:
        case e_insd:
          return false;
        default:
          return is_read;
      }
    }();

    // basic instruction (e.g., `mov rax, ds:[rbx + rax * 2 + 0xff]`)
    auto const type = size_to_type_signed(operand.size);
    auto expr = makeDereferenceExpression(std::move(finalAddress), type);
    insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
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

    switch(id) {
      // Only reads stack
      case e_pop:
      case e_popal:
      case e_popaw:
      case e_popf:
      case e_popfd:
      case e_popfq:
      case e_ret:
      case e_retf:
      case e_iret:
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
        ops.emplace_back(sp, is_read, is_written);
        ops.emplace_back(ebp, is_read, is_written);
        break;

      default:
        break;
    }

    return ops;
  }

}
