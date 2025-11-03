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

#if !defined(INSTRUCTION_DECODER_X86_H)
#define INSTRUCTION_DECODER_X86_H

#include "InstructionDecoderImpl.h"
#include "common/src/ia32_locations.h"

namespace NS_x86 {
  struct ia32_operand;
  class ia32_instruction;
}

namespace Dyninst { namespace InstructionAPI {

  class InstructionDecoder_x86 : public InstructionDecoderImpl {
    friend class Instruction;

  public:
    DYNINST_EXPORT InstructionDecoder_x86(Architecture a);

    DYNINST_EXPORT virtual ~InstructionDecoder_x86();

  private:
    DYNINST_EXPORT InstructionDecoder_x86(const InstructionDecoder_x86& o);

  public:
    DYNINST_EXPORT virtual Instruction decode(InstructionDecoder::buffer& b);

  protected:
    bool decodeOneOperand(const InstructionDecoder::buffer& b, const NS_x86::ia32_operand& operand,
                          int& imm_index, const Instruction* insn_to_complete, bool isRead,
                          bool isWritten, bool isImplicit);

    Expression::Ptr makeSIBExpression(const InstructionDecoder::buffer& b);
    Expression::Ptr makeModRMExpression(const InstructionDecoder::buffer& b, unsigned int opType);
    Expression::Ptr getModRMDisplacement(const InstructionDecoder::buffer& b);
    MachRegister makeRegisterID(unsigned int intelReg, unsigned int opType,
                                bool isExtendedReg = false);
    Expression::Ptr decodeImmediate(unsigned int opType, const unsigned char* immStart,
                                    bool isSigned = false);
    Result_Type makeSizeType(unsigned int opType);

  private:
    void doIA32Decode(InstructionDecoder::buffer& b);
    bool isDefault64Insn();
    void decodeOpcode(InstructionDecoder::buffer&);

    bool decodeOperands(const Instruction* insn_to_complete);

    ia32_locations* locs;
    NS_x86::ia32_instruction* decodedInstruction;
    bool sizePrefixPresent;
    bool addrSizePrefixPresent;
    bool is64BitMode;
  };
}}

#endif
