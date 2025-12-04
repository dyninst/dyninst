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

#ifndef INSTRUCTIONAPI_RISCV_DECODER_H
#define INSTRUCTIONAPI_RISCV_DECODER_H

#include "InstructionDecoderImpl.h"
#include "capstone/capstone.h"
#include "capstone/riscv.h"

namespace Dyninst {
namespace InstructionAPI {

class InstructionDecoder_riscv64 final : public InstructionDecoderImpl {
public:
  struct disassem final {
    csh handle{};
    cs_insn *insn{};
  };

private:
  cs_mode mode{};
  disassem disassembler{};

public:
  InstructionDecoder_riscv64(Dyninst::Architecture a);
  InstructionDecoder_riscv64(InstructionDecoder_riscv64 const &) = delete;
  InstructionDecoder_riscv64 &operator=(InstructionDecoder_riscv64 const &) = delete;
  InstructionDecoder_riscv64(InstructionDecoder_riscv64 &&) = delete;
  InstructionDecoder_riscv64 &operator=(InstructionDecoder_riscv64 &&) = delete;
  ~InstructionDecoder_riscv64();

  Instruction decode(InstructionDecoder::buffer &) override;

private:
  void decode_operands(Instruction &);
  void decode_reg(Instruction &insn, cs_riscv_op const &operand, bool is_encoded);
  void decode_imm(Instruction &insn, cs_riscv_op const &operand, bool is_encoded);
  void decode_mem(Instruction &insn, cs_riscv_op const &operand, bool is_encoded);
  void add_branch_insn_successors(Instruction &,
                                  const std::vector<cs_riscv_op> &);
  void add_pc_operands(Instruction &);
  std::vector<cs_riscv_op> restore_pseudo_insn_operands(Instruction &,
                                                        std::vector<cs_riscv_op> &);
  std::vector<cs_riscv_op> restore_compressed_insn_operands(Instruction &,
                                                            std::vector<cs_riscv_op> &);
};

} // namespace InstructionAPI
} // namespace Dyninst

#endif
