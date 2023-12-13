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
#include "InstructionDecoder-amdgpu-gfx90a.h"
#include <array>
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"

namespace Dyninst {
    namespace InstructionAPI {
        typedef void (InstructionDecoder_amdgpu_gfx90a::*operandFactory)();

        typedef amdgpu_gfx90a_insn_entry amdgpu_gfx90a_insn_table[];

        const std::array<std::string, 16> InstructionDecoder_amdgpu_gfx90a::condNames = { {
            "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
                "lt", "gt", "le", "al", "nv",
        } };

        const char* InstructionDecoder_amdgpu_gfx90a::bitfieldInsnAliasMap(entryID) {
            assert(!"no alias for entryID");
            return nullptr;
        }
        const char* InstructionDecoder_amdgpu_gfx90a::condInsnAliasMap(entryID) {
            assert(!"no alias for entryID");
            return nullptr;
        }

        using namespace std;
        Result_Type InstructionDecoder_amdgpu_gfx90a::makeSizeType(unsigned int) {
            assert(0); //not implemented
            return u32;
        }

        // ****************
        // decoding opcodes
        // ****************

        MachRegister InstructionDecoder_amdgpu_gfx90a::makeAmdgpuRegID(MachRegister base, unsigned int encoding , unsigned int) {
            return MachRegister(base.val() + encoding);

        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::makePCExpr() {
            MachRegister baseReg = amdgpu_gfx90a::pc_all;
            return makeRegisterExpression(baseReg);
        }

        void InstructionDecoder_amdgpu_gfx90a::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal,
                int immLen_ ) {
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

        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::makeFallThroughExpr() {
            // TODO: while s_call_B64 is always 4 bytes, it is not clear whether all instructions that has a fall through branch are 4 bytes long
            return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
        }


        bool InstructionDecoder_amdgpu_gfx90a::decodeOperands(const Instruction *) {
            assert(0 && "decodeOperands deprecated for amdgpu");
            return true;
        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::decodeSGPRorM0(unsigned int offset){
            if( offset <= 104)
                return makeRegisterExpression(makeAmdgpuRegID(amdgpu_gfx90a::s0,offset));
            if (offset == 124)
                return makeRegisterExpression(amdgpu_gfx90a::m0);
            cerr << " unknown offset in sgpr or m0 " << offset << endl; 
            assert(0 && "shouldn't reach here");
            return {};
        }



        void InstructionDecoder_amdgpu_gfx90a::processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout){
            if (layout.IMM ==0 ){
                if( layout.SOFFSET_EN ==0 ) {
                    insn_in_progress-> appendOperand( decodeSGPRorM0(layout.OFFSET), true , false );
                }else{
                    insn_in_progress-> appendOperand( decodeSGPRorM0(layout.SOFFSET), true , false );
                }
            }else{
                if( layout.SOFFSET_EN ==0 ) {
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s64,layout.OFFSET)),false ,false);
                }else{
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(s64,layout.OFFSET)),false,false);
                    insn_in_progress-> appendOperand( decodeSGPRorM0(layout.SOFFSET),true ,false);
                }
            }
        }
        uint32_t InstructionDecoder_amdgpu_gfx90a::decodeOPR_LITERAL(){
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
        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::decodeOPR_SDWA(){
            useImm = true;
            immLen = 4;
            if(insn_size == 4)
                immLiteral = imm_at_32;
            else if(insn_size ==8)
                immLiteral = imm_at_64;
            else
                assert(0 && "unsupported instruction size");
            extension = std::string("_SDWA");
            uint8_t reg_idx = immLiteral & 0xff;

            return makeRegisterExpression(makeAmdgpuRegID(amdgpu_gfx90a::v0,reg_idx),1);
        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::decodeOPR_LABEL(uint64_t input){
            Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
            // 16 bits immediate * 4 => 18 bits
            int64_t offset = sign_extend64(18, input * 4);
            Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));
            return makeAddExpression(lhs, rhs, s64);

        }

        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::makeRegisterExpression(MachRegister registerID, uint32_t num_elements){
            if(registerID == amdgpu_gfx90a::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID,num_elements);
        }
        Expression::Ptr InstructionDecoder_amdgpu_gfx90a::makeRegisterExpression(MachRegister registerID, uint32_t low, uint32_t high ){
            if(registerID == amdgpu_gfx90a::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID, low, high );
        }



        inline unsigned int InstructionDecoder_amdgpu_gfx90a::get32bit(InstructionDecoder::buffer &b,unsigned int offset ){
            assert(offset %4 ==0 );
            if(b.start + offset + 4 <= b.end)
                return b.start[offset+3] << 24 | b.start[offset + 2] << 16 | b.start[offset +1 ] << 8 | b.start [offset];
            return 0;
        }


        void InstructionDecoder_amdgpu_gfx90a::reset(){
            immLen = 0;
            insn_size = 0;
            isBranch = false;
            isConditional = false;
            isModifyPC =false;
            insn = insn_high = insn_long = 0;
            useImm = false;
            isCall = false;
            extension = std::string("");
        }
        // here we assemble the first 64 bit (if available) as an instruction

        void InstructionDecoder_amdgpu_gfx90a::setupInsnWord(InstructionDecoder::buffer &b) {
            reset();
            if (b.start > b.end)
                return;
            insn = get32bit(b,0);

            imm_at_32 = insn_high = get32bit(b,4);
            imm_at_64 = get32bit(b,8);

            insn_long = ( ((uint64_t) insn_high) << 32) | insn;
        }
        void InstructionDecoder_amdgpu_gfx90a::decodeOpcode(InstructionDecoder::buffer &b) {
            setupInsnWord(b);
            mainDecode();
            b.start += insn_in_progress->size();
        }

        void InstructionDecoder_amdgpu_gfx90a::debug_instr(){
            //	cout << "decoded instruction " <<  insn_in_progress->getOperation().mnemonic << " " << std::hex << insn_long << " insn_family = " << instr_family << "  length = " <<  insn_in_progress->size()<< endl << endl;
        }

        Instruction InstructionDecoder_amdgpu_gfx90a::decode(InstructionDecoder::buffer &b) {
            setupInsnWord(b);
            mainDecode();
            b.start += insn_in_progress->size();
            return *insn_in_progress;
        }

        void InstructionDecoder_amdgpu_gfx90a::doDelayedDecode(const Instruction *insn_to_complete) {

            InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
            setupInsnWord(b);
            mainDecode();
            Instruction* iptr = const_cast<Instruction*>(insn_to_complete);
            *iptr = *(insn_in_progress.get());
        }
    }
}



