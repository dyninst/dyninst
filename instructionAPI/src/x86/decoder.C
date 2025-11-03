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
#include "Operation_impl.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "type_conversion.h"
#include "x86/decoder.h"
#include "x86/opcode_xlat.h"


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
    auto *code = static_cast<uint8_t const*>(buf.start);
    auto codeSize = static_cast<size_t>(buf.end - buf.start);
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

    // cs_disasm_iter moves 'code' to the position in the buffer following the decoded instruction.
    // We need the original decoded bytes for 'Instruction', so move it back.
    code = buf.start;
    buf.start += disassembler.insn->size;

    auto prefix = [&](){
      auto const rep_prefix = disassembler.insn->detail->x86.prefix[0];
      switch(rep_prefix) {
        case X86_PREFIX_0:
          return prefixEntryID::prefix_none;
        case X86_PREFIX_REP: /* aliases X86_PREFIX_REPE */
          return prefixEntryID::prefix_rep;
        case X86_PREFIX_REPNE:
          return prefixEntryID::prefix_repnz;
      }
      decode_printf("Unknown Capstone prefix 0x%x\n", rep_prefix);
      return prefixEntryID::prefix_none;
    }();

    entryID e = x86::translate_opcode(static_cast<x86_insn>(disassembler.insn->id));

    auto op = Operation(e, prefix, disassembler.insn->mnemonic, this->m_Arch);
    auto insn = Instruction(std::move(op), disassembler.insn->size, code, this->m_Arch);
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
          break;
        case X86_OP_INVALID:
          decode_printf("[0x%lx %s %s] has an invalid operand.\n", disassembler.insn->address,
                        disassembler.insn->mnemonic, disassembler.insn->op_str);
          break;
      }
    }
  }

}}
