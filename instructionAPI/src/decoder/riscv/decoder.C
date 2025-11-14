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

#include "arch-riscv64.h"
#include "capstone/capstone.h"
#include "capstone/riscv.h"
#include "categories.h"
#include "debug.h"
#include "decoder/riscv/decoder.h"
#include "decoder/riscv/mem_xlat.h"
#include "decoder/riscv/opcode_xlat.h"
#include "decoder/riscv/register_xlat.h"
#include "decoder/type_conversion.h"
#include "entryIDs.h"
#include "registers/riscv64_regs.h"

#include <algorithm>
#include <cstdint>

namespace di = Dyninst::InstructionAPI;

namespace {

bool is_cft(di::Instruction const &insn) {
  return insn.isBranch() || insn.isCall() || insn.isReturn();
}

bool is_pc_relative(di::Instruction const &insn) {
  return is_cft(insn) || insn.getOperation().getID() == riscv64_op_auipc;
}

struct implicit_state final {
  bool read, written;
};

std::map<riscv_reg, implicit_state> implicit_registers(cs_detail const *);
cs_riscv_op make_reg_op(int32_t, uint8_t);
cs_riscv_op make_imm_op(int64_t);

struct eflags_t final {
  Dyninst::MachRegister reg;
  implicit_state state;
};

} // namespace

namespace Dyninst {
namespace InstructionAPI {

riscv_decoder::riscv_decoder(Dyninst::Architecture a)
    : InstructionDecoderImpl(a) {

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
  size_t code_size = buf.end - buf.start;
  uint64_t cap_addr = 0;

  // The iterator form of disassembly allows reuse of the instruction object,
  // reducing the number of memory allocations.
  if (!cs_disasm_iter(disassembler.handle, &code, &code_size, &cap_addr,
                      disassembler.insn)) {
    decode_printf("Failed to disassemble instruction at %p: %s\n",
                  (const void *)code,
                  cs_strerror(cs_errno(disassembler.handle)));
    return {};
  }

  entryID e =
      riscv::translate_opcode(static_cast<riscv_insn>(disassembler.insn->id));
  auto op = Operation(e, disassembler.insn->mnemonic, m_Arch);
  auto code_ptr = buf.start;
  buf.start += disassembler.insn->size;
  unsigned int decodedSize = buf.start - code_ptr;
  Instruction insn(std::move(op), decodedSize, code_ptr, m_Arch);
  decode_operands(insn);
  return insn;
}

void riscv_decoder::decode_operands(Instruction &insn) {
  auto *d = disassembler.insn->detail;

  // Pseudo instructions
  auto const operands = restore_pseudo_insn_operands(insn, disassembler);
  insn.categories = riscv::decode_categories(insn, disassembler, operands);

  /* Decode _explicit_ operands
   *
   * There are three types:
   *
   *   add r1, r2, r3    ; r1, r2 are both RISCV_OP_REG
   *   addi r1, r2, -64  ; -64 is RISCV_OP_IMM
   *   ld r1, (0x33)r2   ; r1 is RISCV_OP_REG, (0x33)r2 is RISCV_OP_MEM
   */

  for (auto &operand : operands) {
    switch (operand.type) {
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
      decode_printf("[0x%lx %s %s] has an invalid operand.\n",
                    disassembler.insn->address, disassembler.insn->mnemonic,
                    disassembler.insn->op_str);
      break;
    }
  }

  // Handle implicit operands
  for (auto r : implicit_registers(d)) {
    constexpr bool is_implicit = true;
    riscv_reg reg = r.first;
    MachRegister mreg = riscv::translate_register(reg, this->mode);
    auto regAST = makeRegisterExpression(mreg);
    implicit_state s = r.second;
    insn.appendOperand(regAST, s.read, s.written, is_implicit);
  }

  // Special case: Capstone does not handle implicit pc registers
  if (is_pc_relative(insn)) {
    add_pc_operands(insn);
  }
  // Finally, add successors to branch instructions
  if (is_cft(insn)) {
    add_branch_insn_successors(insn, operands);
  }
}

void riscv_decoder::decode_reg(Instruction &insn, cs_riscv_op const &operand) {
  constexpr bool is_implicit = true;

  riscv_reg reg = static_cast<riscv_reg>(operand.reg);
  auto regAST = makeRegisterExpression(riscv::translate_register(reg, mode));

  // It's an error if an operand is neither read nor written.
  // In this case, we mark it as both read and written to be conservative.
  bool is_read = ((operand.access & CS_AC_READ) != 0);
  bool is_written = ((operand.access & CS_AC_WRITE) != 0);

  if (!is_read && !is_written) {
    is_read = is_written = true;
  }

  insn.appendOperand(regAST, is_read, is_written, !is_implicit);
}

void riscv_decoder::decode_imm(Instruction &insn, cs_riscv_op const &operand) {

  // Capstone does not track the size of immediate

  auto type = size_to_type_signed(RISCV_IMM_SIZE);
  auto imm = Immediate::makeImmediate(Result(type, operand.imm));

  constexpr bool is_read = true;
  constexpr bool is_written = true;
  constexpr bool is_implicit = true;

  insn.appendOperand(std::move(imm), !is_read, !is_written, !is_implicit);
  return;
}

void riscv_decoder::decode_mem(Instruction &insn, cs_riscv_op const &operand) {
  const entryID eid = insn.getOperation().getID();
  const int8_t size = riscv::mem_size(eid);
  auto const type = size_to_type_signed(size);
  const bool is_implicit = true;

  auto disp = Immediate::makeImmediate(Result(type, operand.mem.disp));
  auto base = makeRegisterExpression(riscv::translate_register(
      static_cast<riscv_reg>(operand.mem.base), this->mode));
  auto add = makeAddExpression(std::move(base), std::move(disp), type);
  auto deref = makeDereferenceExpression(std::move(add), type);
  insn.appendOperand(std::move(deref), riscv::is_mem_load(eid),
                     riscv::is_mem_store(eid), !is_implicit);
}

void riscv_decoder::add_branch_insn_successors(
    Instruction &insn, const std::vector<cs_riscv_op> &operands) {
  constexpr bool is_call = true;
  constexpr bool is_indirect = true;
  constexpr bool is_conditional = true;
  constexpr bool is_fallthrough = true;

  auto const pc = makeRegisterExpression(MachRegister::getPC(this->m_Arch));
  auto const reg_type = size_to_type_signed(RISCV_REG_SIZE);
  auto const imm_type = size_to_type_signed(RISCV_IMM_SIZE);
  switch (insn.getOperation().getID()) {
  case riscv64_op_jal: {
    MachRegister link_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    auto const rd = makeRegisterExpression(link_reg);
    auto const imm =
        Immediate::makeImmediate(Result(imm_type, operands[1].imm));
    auto const target = makeAddExpression(std::move(pc), imm, imm_type);
    const bool is_call_insn = link_reg != riscv64::zero;
    const Instruction::CFT cft(std::move(target), is_call_insn, !is_indirect, !is_conditional, !is_fallthrough);
    insn.addSuccessor(std::move(cft));
    break;
  }
  case riscv64_op_jalr: {
    MachRegister link_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    auto rd = makeRegisterExpression(link_reg);
    MachRegister target_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[1].reg), this->mode);
    auto const rs = makeRegisterExpression(target_reg);
    auto const imm =
        Immediate::makeImmediate(Result(imm_type, operands[1].imm));
    auto const target = makeAddExpression(std::move(rs), std::move(imm),
                                          std::max(reg_type, imm_type));
    const bool is_call_insn = link_reg != riscv64::zero;
    const Instruction::CFT cft(std::move(target), is_call_insn, is_indirect, !is_conditional, !is_fallthrough);
    insn.addSuccessor(std::move(cft));
    break;
  }
  case riscv64_op_c_j: {
    auto const imm =
        Immediate::makeImmediate(Result(imm_type, operands[0].imm));
    auto const target = makeAddExpression(std::move(pc), std::move(imm),
                                          std::max(reg_type, imm_type));
    const Instruction::CFT cft(std::move(target), !is_call, !is_indirect, !is_conditional, !is_fallthrough);
    insn.addSuccessor(std::move(cft));
    break;
  }
  case riscv64_op_c_jr: {
    MachRegister target_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    auto const rs = makeRegisterExpression(target_reg);
    const Instruction::CFT cft(std::move(rs), !is_call, is_indirect, !is_conditional, !is_fallthrough);
    insn.addSuccessor(std::move(cft));
    break;
  }
  case riscv64_op_c_jalr: {
    MachRegister target_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    auto const rs = makeRegisterExpression(target_reg);
    const Instruction::CFT cft(std::move(rs), is_call, is_indirect, !is_conditional, !is_fallthrough);
    insn.addSuccessor(std::move(cft));
    break;
  }
  case riscv64_op_beq:
  case riscv64_op_bne:
  case riscv64_op_blt:
  case riscv64_op_bge:
  case riscv64_op_bltu:
  case riscv64_op_bgeu: {
    MachRegister cmp_reg1 = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    MachRegister cmp_reg2 = riscv::translate_register(
        static_cast<riscv_reg>(operands[1].reg), this->mode);
    auto const rs1 = makeRegisterExpression(cmp_reg1);
    auto const rs2 = makeRegisterExpression(cmp_reg2);
    auto const imm =
        Immediate::makeImmediate(Result(imm_type, operands[2].imm));
    auto const target = makeAddExpression(std::move(pc), std::move(imm),
                                          std::max(reg_type, imm_type));
    if (operands[0].reg == RISCV_REG_ZERO &&
        operands[1].reg == RISCV_REG_ZERO) {
        const Instruction::CFT cft(std::move(target), !is_call, !is_indirect, !is_conditional, !is_fallthrough);
        insn.addSuccessor(std::move(cft));
    } else {
        const Instruction::CFT cft1(std::move(target), !is_call, !is_indirect, is_conditional, is_fallthrough);
        insn.addSuccessor(std::move(cft1));
        const Instruction::CFT cft2(std::move(pc), !is_call, !is_indirect, is_conditional, is_fallthrough);
        insn.addSuccessor(std::move(cft2));
    }
    break;
  }
  case riscv64_op_c_beqz:
  case riscv64_op_c_bnez: {
    MachRegister cmp_reg = riscv::translate_register(
        static_cast<riscv_reg>(operands[0].reg), this->mode);
    auto const rs = makeRegisterExpression(cmp_reg);
    auto const imm =
        Immediate::makeImmediate(Result(imm_type, operands[1].imm));
    auto const target = makeAddExpression(std::move(pc), std::move(imm),
                                          std::max(reg_type, imm_type));
    const Instruction::CFT cft1(std::move(target), !is_call, !is_indirect, is_conditional, is_fallthrough);
    insn.addSuccessor(std::move(cft1));
    const Instruction::CFT cft2(std::move(pc), !is_call, !is_indirect, is_conditional, is_fallthrough);
    insn.addSuccessor(std::move(cft2));
    break;
  }
  default: {
    break;
  }
  }
}

std::vector<cs_riscv_op> riscv_decoder::restore_pseudo_insn_operands(
    Instruction &insn, di::riscv_decoder::disassem const &dis) {
  const auto operands = dis.insn->detail->riscv.operands;
  const auto op_count = dis.insn->detail->riscv.op_count;
  std::vector<cs_riscv_op> res = {operands, operands + op_count};
  switch (insn.getOperation().getID()) {
  case riscv64_op_addi: {
    if (op_count == 0) {
      // nop -> addi zero, zero, 0
      res = {make_reg_op(RISCV_REG_ZERO, CS_AC_WRITE),
             make_reg_op(RISCV_REG_ZERO, CS_AC_READ), make_imm_op(0)};
      insn.getOperation().updateMnemonic("addi");
    }
    if (op_count == 2) {
      // mv rd, rs -> addi rd, rs, 0
      res = {operands[0], operands[1], make_imm_op(0)};
      insn.getOperation().updateMnemonic("addi");
    }
    break;
  }
  case riscv64_op_addiw: {
    if (op_count == 2) {
      // sext.w rd, rs -> addiw rd, rs, 0
      res = {operands[0], operands[1], make_imm_op(0)};
      insn.getOperation().updateMnemonic("addiw");
    }
    break;
  }
  case riscv64_op_andi: {
    if (op_count == 2) {
      // zext.b rd, rs -> andi rd, rs, 255
      res = {operands[0], operands[1], make_imm_op(255)};
      insn.getOperation().updateMnemonic("andi");
    }
    break;
  }
  case riscv64_op_xori: {
    if (op_count == 2) {
      // not rd, rs -> xori rd, rs, -1
      res = {operands[0], operands[1], make_imm_op(-1)};
      insn.getOperation().updateMnemonic("xori");
    }
    break;
  }
  case riscv64_op_sub: {
    if (op_count == 2) {
      // neg rd, rs -> sub rd, x0, rs
      // negw rd, rs -> subw rd, x0, rs
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("sub");
    }
    break;
  }
  case riscv64_op_subw: {
    if (op_count == 2) {
      // neg rd, rs -> sub rd, x0, rs
      // negw rd, rs -> subw rd, x0, rs
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("subw");
    }
    break;
  }
  case riscv64_op_sltiu: {
    if (op_count == 2) {
      // seqz rd, rs -> sltiu rd, rs, 1
      res = {operands[0], operands[1], make_imm_op(1)};
      insn.getOperation().updateMnemonic("sltiu");
    }
    break;
  }
  case riscv64_op_sltu: {
    if (op_count == 2) {
      // snez rd, rs -> sltu rd, x0, rs
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("sltu");
    }
    break;
  }
  case riscv64_op_slt: {
    if (op_count == 2) {
      // sltz rd, rs -> slt rd, rs, x0
      // sgtz rd, rs -> slt rd, x0, rs

      // Determine whether it is sltz or sgtz
      // It is impossible from telling those two apart without extracting the
      // raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc_raw =
          ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs2_enc_raw =
          ((*insn_raw_ptr) >> REG_RS2_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs1_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[0].reg), mode) &
                           0xff;
      int32_t rs2_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[1].reg), mode) &
                           0xff;
      // Compare the raw value of rs1 with operands[0] and rs2 with operands[1]
      // If they match, this means that we're using sltz
      // Otherwise, we're using sltz
      if (rs1_enc_raw == rs1_enc_od && rs2_enc_raw == rs2_enc_od) {
        // sltz rd, rs -> slt rd, rs, x0
        res = {operands[0], operands[1],
               make_reg_op(RISCV_REG_ZERO, CS_AC_READ)};
      } else {
        // sgtz rd, rs -> slt rd, x0, rs
        res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ),
               operands[1]};
      }
      insn.getOperation().updateMnemonic("slt");
    }
    break;
  }
  case riscv64_op_fsgnj_s: {
    if (op_count == 2) {
      // fmv.h frd, frs -> fsgnj.h frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnj.h");
    }
    break;
  }
  case riscv64_op_fsgnjx_s: {
    if (op_count == 2) {
      // fabs.h frd, frs -> fsgnjx.h frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnjx.h");
    }
    break;
  }
  case riscv64_op_fsgnjn_s: {
    if (op_count == 2) {
      // fneg.h frd, frs -> fsgnjn.h frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnjn.h");
    }
    break;
  }
  case riscv64_op_fsgnj_d: {
    if (op_count == 2) {
      // fmv.s frd, frs -> fsgnj.s frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnj.s");
    }
    break;
  }
  case riscv64_op_fsgnjx_d: {
    if (op_count == 2) {
      // fmv.s frd, frs -> fsgnjx.s frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnjx.s");
    }
    break;
  }
  case riscv64_op_fsgnjn_d: {
    if (op_count == 2) {
      // fneg.s frd, frs -> fsgnjn.s frd, frs, frs
      res = {operands[0], operands[1], operands[1]};
      insn.getOperation().updateMnemonic("fsgnjn.s");
    }
    break;
  }
  case riscv64_op_jal: {
    if (op_count == 1) {
      // We must inspect rd to tell apart the j and jal pseudo instruction
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rd_enc = ((*insn_raw_ptr) >> REG_RD_ENC_OFFSET) & REG_ENC_MASK;
      // j offset -> jal x0, offset
      if (rd_enc == GPR_ZERO) {
        res = {make_reg_op(RISCV_REG_ZERO, CS_AC_WRITE), operands[0]};
      }
      // jal offset -> jal x1, offset
      else if (rd_enc == GPR_RA) {
        res = {make_reg_op(RISCV_REG_RA, CS_AC_WRITE), operands[0]};
      }
      insn.getOperation().updateMnemonic("jal");
    }
    break;
  }
  case riscv64_op_jalr: {
    if (op_count == 0) {
      // ret -> jalr x0, x1, 0
      res = {make_reg_op(RISCV_REG_ZERO, CS_AC_WRITE),
             make_reg_op(RISCV_REG_RA, CS_AC_READ), make_imm_op(0)};
    }
    if (op_count == 1) {
      // We must inspect rd to tell apart the j and jal pseudo instruction
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rd_enc = ((*insn_raw_ptr) >> REG_RD_ENC_OFFSET) & REG_ENC_MASK;
      // jr rs -> jalr x0, rs, 0
      if (rd_enc == GPR_ZERO) {
        res = {make_reg_op(RISCV_REG_ZERO, CS_AC_WRITE), operands[0],
               make_imm_op(0)};
      }
      // jalr rs -> jalr x1, rs, 0
      else if (rd_enc == GPR_RA) {
        res = {make_reg_op(RISCV_REG_RA, CS_AC_WRITE), operands[0],
               make_imm_op(0)};
      }
    }
    insn.getOperation().updateMnemonic("jalr");
    break;
  }
  case riscv64_op_beq: {
    if (op_count == 2) {
      // beqz rs, offset -> beq rs, x0, offset
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("beq");
    }
    break;
  }
  case riscv64_op_bne: {
    if (op_count == 2) {
      // bnez rs, offset -> bne rs, x0, offset
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("bne");
    }
    break;
  }
  case riscv64_op_blt: {
    if (op_count == 2) {
      // bltz rs, offset -> blt rs, x0, offset
      // bgtz rs, offset -> blt x0, rs, offset

      // Determine where x0 should go
      // It is impossible from telling blez vs bgez and bltz vs bgtz apart
      // without extracting the raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc = ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      if (rs1_enc == GPR_ZERO) {
        res = {make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[0],
               operands[1]};
      } else {
        res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ),
               operands[1]};
      }
      res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[1]};
      insn.getOperation().updateMnemonic("blt");
    }
    if (op_count == 3) {
      // bgt rs, rt, offset -> blt rt, rs, offset

      // Determine whether it is blt or bgt
      // It is impossible from telling those two apart without extracting the
      // raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc_raw =
          ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs2_enc_raw =
          ((*insn_raw_ptr) >> REG_RS2_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs1_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[0].reg), mode) &
                           0xff;
      int32_t rs2_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[1].reg), mode) &
                           0xff;
      // Compare the raw value of rs1 with operands[0] and rs2 with operands[1]
      // If they do not match, this means that we're using these 4 pseudo
      // instructions
      if (rs1_enc_raw == rs2_enc_od && rs2_enc_raw == rs1_enc_od) {
        // swap rs1 and rs2
        res = {operands[1], operands[0], operands[2]};
        insn.getOperation().updateMnemonic("blt");
      }
    }
    break;
  }
  case riscv64_op_bge: {
    if (op_count == 2) {
      // blez rs, offset -> bge x0, rs, offset
      // bgez rs, offset -> bge rs, x0, offset

      // Determine where x0 should go
      // It is impossible from telling blez vs bgez
      // without extracting the raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc = ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      if (rs1_enc == GPR_ZERO) {
        res = {make_reg_op(RISCV_REG_ZERO, CS_AC_READ), operands[0],
               operands[1]};
      } else {
        res = {operands[0], make_reg_op(RISCV_REG_ZERO, CS_AC_READ),
               operands[1]};
      }
      insn.getOperation().updateMnemonic("bge");
    }
    if (op_count == 3) {
      // ble rs, rt, offset -> bge rt, rs, offset

      // Determine whether it is bge or ble
      // It is impossible from telling those two apart without extracting the
      // raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc_raw =
          ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs2_enc_raw =
          ((*insn_raw_ptr) >> REG_RS2_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs1_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[0].reg), mode) &
                           0xff;
      int32_t rs2_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[1].reg), mode) &
                           0xff;
      // Compare the raw value of rs1 with operands[0] and rs2 with operands[1]
      // If they do not match, this means that we're using these 4 pseudo
      // instructions
      if (rs1_enc_raw == rs2_enc_od && rs2_enc_raw == rs1_enc_od) {
        // swap rs1 and rs2
        res = {operands[1], operands[0], operands[2]};
        insn.getOperation().updateMnemonic("bge");
      }
    }
    break;
  }
  case riscv64_op_bltu: {
    if (op_count == 3) {
      // bgtu rs, rt, offset -> bltu rt, rs, offset

      // Determine whether it is bltu or bgtu
      // It is impossible from telling those two apart without extracting the
      // raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc_raw =
          ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs2_enc_raw =
          ((*insn_raw_ptr) >> REG_RS2_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs1_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[0].reg), mode) &
                           0xff;
      int32_t rs2_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[1].reg), mode) &
                           0xff;
      // Compare the raw value of rs1 with operands[0] and rs2 with operands[1]
      // If they do not match, this means that we're using these 4 pseudo
      // instructions
      if (rs1_enc_raw == rs2_enc_od && rs2_enc_raw == rs1_enc_od) {
        // swap rs1 and rs2
        res = {operands[1], operands[0], operands[2]};
        insn.getOperation().updateMnemonic("bltu");
      }
    }
    break;
  }
  case riscv64_op_bgeu: {
    if (op_count == 3) {
      // bleu rs, rt, offset -> bgeu rt, rs, offset

      // Determine whether it is bgeu or bleu
      // It is impossible from telling those two apart without extracting the
      // raw registers
      const uint32_t *insn_raw_ptr =
          reinterpret_cast<const uint32_t *>(insn.ptr());
      int32_t rs1_enc_raw =
          ((*insn_raw_ptr) >> REG_RS1_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs2_enc_raw =
          ((*insn_raw_ptr) >> REG_RS2_ENC_OFFSET) & REG_ENC_MASK;
      int32_t rs1_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[0].reg), mode) &
                           0xff;
      int32_t rs2_enc_od = riscv::translate_register(
                               static_cast<riscv_reg>(operands[1].reg), mode) &
                           0xff;
      // Compare the raw value of rs1 with operands[0] and rs2 with operands[1]
      // If they do not match, this means that we're using these 4 pseudo
      // instructions
      if (rs1_enc_raw == rs2_enc_od && rs2_enc_raw == rs1_enc_od) {
        // swap rs1 and rs2
        res = {operands[1], operands[0], operands[2]};
        insn.getOperation().updateMnemonic("bgeu");
      }
    }
    break;
  }
  default: {
    break;
  }
  }
  return res;
}

void riscv_decoder::add_pc_operands(Instruction &insn) {
  constexpr bool is_read = true;
  constexpr bool is_write = true;
  constexpr bool is_implicit = true;
  const entryID eid = insn.getOperation().getID();

  auto const pc = makeRegisterExpression(MachRegister::getPC(this->m_Arch));
  switch (eid) {
  case riscv64_op_auipc: {
    insn.appendOperand(pc, is_read, !is_write, is_implicit);
    break;
  }
  case riscv64_op_jalr:
  case riscv64_op_c_jr:
  case riscv64_op_c_jalr: {
    insn.appendOperand(pc, !is_read, is_write, is_implicit);
    break;
  }
  case riscv64_op_jal:
  case riscv64_op_c_j:
  case riscv64_op_beq:
  case riscv64_op_bne:
  case riscv64_op_blt:
  case riscv64_op_bge:
  case riscv64_op_bltu:
  case riscv64_op_bgeu:
  case riscv64_op_c_beqz:
  case riscv64_op_c_bnez: {
    insn.appendOperand(pc, is_read, is_write, is_implicit);
    break;
  }
  default: {
    break;
  }
  }
}
} // namespace InstructionAPI
} // namespace Dyninst

namespace {

std::map<riscv_reg, implicit_state> implicit_registers(cs_detail const *d) {
  std::map<riscv_reg, implicit_state> regs;
  for (int i = 0; i < d->regs_read_count; ++i) {
    regs.emplace(static_cast<riscv_reg>(d->regs_read[i]),
                 implicit_state{true, false});
  }
  for (int i = 0; i < d->regs_write_count; ++i) {
    auto res = regs.emplace(static_cast<riscv_reg>(d->regs_write[i]),
                            implicit_state{false, true});
    if (!res.second) {
      // Register already existed, so was read. Mark it written.
      res.first->second.written = true;
    }
  }
  return regs;
}

cs_riscv_op make_reg_op(int32_t reg, uint8_t access) {
  cs_riscv_op op{};
  op.type = RISCV_OP_REG;
  op.reg = reg;
  op.access = access;
  return op;
}

cs_riscv_op make_imm_op(int64_t imm) {
  cs_riscv_op op{};
  op.type = RISCV_OP_IMM;
  op.imm = imm;
  op.access = CS_AC_READ;
  return op;
}
} // namespace
