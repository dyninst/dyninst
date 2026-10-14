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

#include "Ternary.h"
#include "InstructionDecoder-amdgpu-gfx1250.h"
#include <array>
#include "registers/AMDGPU/amdgpu_gfx1250_regs.h"

namespace Dyninst {
    namespace InstructionAPI {
        typedef void (InstructionDecoder_amdgpu_gfx1250::*operandFactory)();

        typedef amdgpu_gfx1250_insn_entry amdgpu_gfx1250_insn_table[];

        const std::array<std::string, 16> InstructionDecoder_amdgpu_gfx1250::condNames = { {
            "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
                "lt", "gt", "le", "al", "nv",
        } };

        const char* InstructionDecoder_amdgpu_gfx1250::bitfieldInsnAliasMap(entryID) {
            assert(!"no alias for entryID");
            return nullptr;
        }
        const char* InstructionDecoder_amdgpu_gfx1250::condInsnAliasMap(entryID) {
            assert(!"no alias for entryID");
            return nullptr;
        }

        using namespace std;

        // ****************
        // decoding opcodes
        // ****************

        MachRegister InstructionDecoder_amdgpu_gfx1250::makeAmdgpuRegID(MachRegister base, unsigned int encoding , unsigned int) {
            return MachRegister(base.val() + encoding);

        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx1250::makePCExpr() {
            MachRegister baseReg = amdgpu_gfx1250::pc_all;
            return makeRegisterExpression(baseReg);
        }

        void InstructionDecoder_amdgpu_gfx1250::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal,
                int immLen_) {
            Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
            // * 4 => 2 more bits
            int64_t offset = sign_extend64(immLen_+2, immVal * 4);

            Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

            insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional,
                    false);
            if (bIsConditional || branchIsCall) {
                insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
            }

        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx1250::makeFallThroughExpr() {
            // TODO: while s_call_i64 is always 4 bytes, it is not clear whether all instructions that has a fall through branch are 4 bytes long
            return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
        }

        // gfx12 SMEM: a signed 24-bit IOFFSET plus an SOFFSET register, where
        // SOFFSET = NULL (124) means no register offset
        void InstructionDecoder_amdgpu_gfx1250::processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout){
            int64_t ioffset = sign_extend64(24, layout.IOFFSET);
            insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s32,ioffset)),false,false);
            if (layout.SOFFSET != 124)
                insn_in_progress->appendOperand(decodeOPR_SREG(layout.SOFFSET),true,false);
        }

        uint32_t InstructionDecoder_amdgpu_gfx1250::decodeOPR_LITERAL(){
            useImm = true;
            immLen = 4;
            if(insn_size == 4)
                immLiteral = imm_at_32;
            else if(insn_size ==8)
                immLiteral = imm_at_64;
            else
                assert(0 && "unsupported instruction size");

            return immLiteral;
        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx1250::decodeOPR_LABEL(uint64_t input){
            Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
            // 16 bits immediate * 4 => 18 bits
            int64_t offset = sign_extend64(18, input * 4);
            Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));
            return makeAddExpression(lhs, rhs, s64);

        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx1250::makeRegisterExpression(MachRegister registerID, uint32_t num_elements){
            if(registerID == amdgpu_gfx1250::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID,num_elements);
        }
        Expression::Ptr InstructionDecoder_amdgpu_gfx1250::makeRegisterExpression(MachRegister registerID, uint32_t low, uint32_t high ){
            if(registerID == amdgpu_gfx1250::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID, low, high );
        }

        inline unsigned int InstructionDecoder_amdgpu_gfx1250::get32bit(InstructionDecoder::buffer &b,unsigned int offset ){
            assert(offset %4 ==0 );
            if(b.start + offset + 4 <= b.end)
                return b.start[offset+3] << 24 | b.start[offset + 2] << 16 | b.start[offset +1 ] << 8 | b.start [offset];
            return 0;
        }


        void InstructionDecoder_amdgpu_gfx1250::reset(){
            immLen = 0;
            insn_size = 0;
            isBranch = false;
            isConditional = false;
            isModifyPC =false;
            insn = insn_high = insn_long = 0;
            insn_long2 = 0;
            useImm = false;
            isCall = false;
            extension = std::string("");
        }
        // here we assemble the first 128 bit (if available) as an instruction

        void InstructionDecoder_amdgpu_gfx1250::setupInsnWord(InstructionDecoder::buffer &b) {
            reset();
            if (b.start > b.end)
                return;
            insn = get32bit(b,0);

            imm_at_32 = insn_high = get32bit(b,4);
            imm_at_64 = get32bit(b,8);

            insn_long = ( ((uint64_t) insn_high) << 32) | insn;
            // bits 127:64: the trailing literal of 96-bit encodings and the
            // second half of 128-bit ones
            insn_long2 = ( ((uint64_t) get32bit(b,12)) << 32) | get32bit(b,8);
        }

        void InstructionDecoder_amdgpu_gfx1250::debug_instr(){
            //	cout << "decoded instruction " <<  insn_in_progress->getOperation().mnemonic << " " << std::hex << insn_long << " insn_family = " << instr_family << "  length = " <<  insn_in_progress->size()<< endl << endl;
        }

        Instruction InstructionDecoder_amdgpu_gfx1250::decode(InstructionDecoder::buffer &b) {
            setupInsnWord(b);
            mainDecode();
            b.start += insn_in_progress->size();
            return *insn_in_progress;
        }

    }
}



