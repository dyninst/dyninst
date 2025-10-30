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
#include "capstone/riscv.h"
#include "categories.h"
#include "debug.h"
#include "entryIDs.h"
#include "registers/riscv64_regs.h"
#include "syscalls.h"
#include "type_conversion.h"
#include "riscv/decoder.h"
#include "riscv/opcode_xlat.h"
#include "riscv/register_xlat.h"

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

  std::map<riscv_reg, implicit_state> implicit_registers(cs_detail const *);

  struct eflags_t final {
    Dyninst::MachRegister reg;
    implicit_state state;
  };

  std::vector<eflags_t> expand_flags(cs_insn const *, riscv_reg, cs_mode);

  struct mem_op {
    Dyninst::MachRegister reg;
    bool read, written;

    mem_op(Dyninst::MachRegister reg_, bool read_, bool written_) : reg{reg_}, read{read_}, written{written_} {}
  };

  std::vector<mem_op> get_implicit_memory_ops(entryID, Dyninst::Architecture);
}

namespace Dyninst { namespace InstructionAPI {

  riscv_decoder::riscv_decoder(Dyninst::Architecture a) : InstructionDecoderImpl(a) {

    // Currently we only support RV64
    mode = (cs_mode)(CS_MODE_RISCV64 | CS_MODE_RISCVC);

    cs_open(CS_ARCH_RISCV, this->mode, &disassembler.handle);
    cs_option(disassembler.handle, CS_OPT_DETAIL, CS_OPT_ON);
    disassembler.insn = cs_malloc(disassembler.handle);
  }


  riscv_decoder::~riscv_decoder() {
    cs_free(disassembler.insn, 1);
    cs_close(&disassembler.handle);
  }

  Instruction riscv_decoder::decode(InstructionDecoder::buffer &buf) {
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

    entryID e = riscv::translate_opcode(static_cast<riscv_insn>(disassembler.insn->id));
    auto op = Operation(e, disassembler.insn->mnemonic, m_Arch);
    buf.start += disassembler.insn->size;
    unsigned int decodedSize = buf.start - code;
    Instruction insn(std::move(op), decodedSize, code, m_Arch);
    decode_operands(insn);
    return insn;
  }

  void riscv_decoder::decode_operands(Instruction& insn) {
    // Categories must be decoded before anything else since they are used
    // in the other decoding steps.
    insn.categories = riscv::decode_categories(insn, disassembler);

    /*
     * Handle aliases
     * Capstone currently does not support alias on RISC-V
     * Causing registers to be discarded
     * 
     * For example
     *
     *   ret
     *
     * is an alias of
     *
     *   c.jr zero, ra
     *
     * Capstone treat "ret" as an instruction that does not have any operands
     * .and discards "zero" and "ra"
     *
     * This can be removed once Capstone moves to 6.0.0
     */

     /*
      * Handle branch instructions as a special cases
      */
     {
       bool is_cfg = false;
       bool is_call = false;
       bool is_indirect = false;
       bool is_conditional = false;
       constexpr int32_t RD_SHIFT = 7;
       constexpr int32_t RD_MASK = 0x1f;
       switch(insn.getOperation().getID()) {
         case riscv64_op_jal: {
           unsigned int rd = 0;
           int32_t imm = 0;
           // If rd == x1 or rd == x0, Capstone will treat them as
           // j offset and jr offset respectively
           const uint32_t *insn_raw_ptr = reinterpret_cast<const uint32_t *>(insn.ptr());
           // The j pseudo instruction
           if ((((*insn_raw_ptr) >> RD_SHIFT) & RD_MASK) == 0) {
             rd = RISCV_REG_X0;
             isCall = false;
             assert(d->riscv.operands[0].type == RISCV_OP_IMM);
             imm = d->riscv.operands[0].imm;
             // The jr pseudo instruction
           } else if ((((*insn_raw_ptr) >> RD_SHIFT) & RD_MASK) == 1) {
             rd = RISCV_REG_X1;
             isCall = true;
             assert(d->riscv.operands[0].type == RISCV_OP_IMM);
             imm = d->riscv.operands[0].imm;
           } else {
             assert(d->riscv.operands[0].type == RISCV_OP_REG);
             rd = d->riscv.operands[0].reg;
             assert(d->riscv.operands[1].type == RISCV_OP_IMM);
             imm = d->riscv.operands[1].imm;
           }
           break;
         }
         default:
           break;
       }
     }
    

    /* Decode _explicit_ operands
     *
     * There are three types:
     *
     *   add r1, r2, r3    ; r1, r2 are both RISCV_OP_REG
     *   addi r1, r2, -64  ; -64 is RISCV_OP_IMM
     *   ld r1, (0x33)r2   ; r1 is RISCV_OP_REG, (0x33)r2 is RISCV_OP_MEM
     */
    auto *d = disassembler.insn->detail;
    for(uint8_t i = 0; i < d->riscv.op_count; ++i) {
      cs_riscv_op const &operand = d->riscv.operands[i];
      switch(operand.type) {
        case RISCV_OP_REG:
          decode_reg(insn, operand);
          break;
        case RISCV_OP_IMM:
          decode_imm(insn, operand);
          break;
        case RISCV_OP_MEM:
          decode_mem(insn, operand);
          break;
        case RISCV_OP_INVALID:
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
      riscv_reg reg = r.first;
      MachRegister mreg = riscv::translate_register(reg, this->mode);
      auto regAST = makeRegisterExpression(mreg);
      implicit_state s = r.second;
      insn.appendOperand(regAST, s.read, s.written, is_implicit);
    }
  }

  void riscv_decoder::decode_reg(Instruction &insn, cs_riscv_op const &operand) {
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;
    constexpr bool is_conditional = true;
    constexpr bool is_fallthrough = true;

    auto regAST = makeRegisterExpression(riscv::translate_register(operand.reg, mode));

    // It's an error if an operand is neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool is_read = ((operand.access & CS_AC_READ) != 0);
    bool is_written = ((operand.access & CS_AC_WRITE) != 0);

    if(!is_read && !is_written) {
      is_read = is_written = true;
    }

    insn.appendOperand(regAST, is_read, is_written, !is_implicit);
  }

  void riscv_decoder::decode_imm(Instruction &insn, cs_riscv_op const &operand) {
    // Capstone does not track the size of immediate
    // the biggest possible immediate for the RV64GC profile is 20 bits
    // which can be fit in 32 bit integer

    auto const type = size_to_type_signed(4); // 32 bit integer
    auto imm = Immediate::makeImmediate(Result(type, operand.imm));

    constexpr bool is_read = true;
    constexpr bool is_written = true;
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;

    insn.appendOperand(std::move(imm), !is_read, !is_written, !is_implicit);
    return;
  }

  void riscv_decoder::decode_mem(Instruction &insn, cs_riscv_op const &operand) {
  }
}}

namespace {

  std::map<riscv_reg, implicit_state> implicit_registers(cs_detail const *d) {
    std::map<riscv_reg, implicit_state> regs;
    for(int i = 0; i < d->regs_read_count; ++i) {
      regs.emplace(static_cast<riscv_reg>(d->regs_read[i]), implicit_state{true, false});
    }
    for(int i = 0; i < d->regs_write_count; ++i) {
      auto res = regs.emplace(static_cast<riscv_reg>(d->regs_write[i]), implicit_state{false, true});
      if(!res.second) {
        // Register already existed, so was read. Mark it written.
        res.first->second.written = true;
      }
    }
    return regs;
  }
}
