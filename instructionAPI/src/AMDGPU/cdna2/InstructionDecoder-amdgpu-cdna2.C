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
#include "InstructionDecoder-amdgpu-cdna2.h"

namespace Dyninst {
	namespace InstructionAPI {
		typedef void (InstructionDecoder_amdgpu_cdna2::*operandFactory)();

		typedef amdgpu_cdna2_insn_entry amdgpu_cdna2_insn_table[];
		typedef amdgpu_mask_entry amdgpu_decoder_table[];

		const std::array<std::string, 16> InstructionDecoder_amdgpu_cdna2::condNames = { {
			"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
			"lt", "gt", "le", "al", "nv",
		} };

		const char* InstructionDecoder_amdgpu_cdna2::bitfieldInsnAliasMap(entryID e) {
			switch(e) {
				default: assert(!"no alias for entryID");
			};
		}
		const char* InstructionDecoder_amdgpu_cdna2::condInsnAliasMap(entryID e) {
			switch(e) {
				default: assert(!"no alias for entryID");
			};
		}

#include "amdgpu_cdna2_insn_entry.h"
		struct amdgpu_mask_entry {
			unsigned int mask;
			std::size_t branchCnt;
			const std::pair<unsigned int,unsigned int>* nodeBranches;
			int insnTableIndex;

			static const amdgpu_decoder_table main_decoder_table;
			static const std::pair<unsigned int,unsigned int> branchTable[];
		};

#include "amdgpu_cdna2_opcode_tables.C"

		InstructionDecoder_amdgpu_cdna2::InstructionDecoder_amdgpu_cdna2(Architecture a)
			: InstructionDecoderImpl(a), 
            insn_size(0), immLen(0) , num_elements(1) , isSMEM(false), isLoad(false), isStore(false),isBuffer(false),
            isScratch(false) , isBranch(false), isConditional(false) ,isCall(false), isModifyPC(false)
		{
		}

		InstructionDecoder_amdgpu_cdna2::~InstructionDecoder_amdgpu_cdna2() {
		}


		using namespace std;
		void InstructionDecoder_amdgpu_cdna2::NOTHING() {
		}

		Result_Type InstructionDecoder_amdgpu_cdna2::makeSizeType(unsigned int) {
			assert(0); //not implemented
			return u32;
		}

		// ****************
		// decoding opcodes
		// ****************

		MachRegister InstructionDecoder_amdgpu_cdna2::makeAmdgpuRegID(MachRegister base, unsigned int encoding , unsigned int) {
			return MachRegister(base.val() + encoding);

		}

		Expression::Ptr InstructionDecoder_amdgpu_cdna2::makePCExpr() {
			MachRegister baseReg = amdgpu_cdna2::pc_all;
			return makeRegisterExpression(baseReg);
		}

		void InstructionDecoder_amdgpu_cdna2::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal,
				int immLen_ = 16) {
			Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
			int64_t offset = sign_extend64(immLen_, immVal * 4);

			Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

			insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional,
					false);
			if (bIsConditional || branchIsCall) {
				insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
			}

		}

		Expression::Ptr InstructionDecoder_amdgpu_cdna2::makeFallThroughExpr() {
			// TODO: while s_call_B64 is always 4 bytes, it is not clear whether all instructions that has a fall through branch are 4 bytes long
			return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
		}


		bool InstructionDecoder_amdgpu_cdna2::decodeOperands(const Instruction *) {
			assert(0 && "decodeOperands deprecated for amdgpu");
			return true;
		}

		Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeSGPRorM0(unsigned int offset){
			if( offset <= 104)
				return makeRegisterExpression(makeAmdgpuRegID(amdgpu_cdna2::s0,offset));
			if (offset == 124)
				return makeRegisterExpression(amdgpu_cdna2::m0);
            cerr << " unknown offset in sgpr or m0 " << offset << endl; 
			assert(0 && "shouldn't reach here");
		}


        uint32_t InstructionDecoder_amdgpu_cdna2::decodeOPR_LITERAL(){
            if (!useImm){
                    useImm = true;
                    immLen = 4;
                    if(insn_size == 4)
                        immLiteral = imm_at_32;
                    else if(insn_size ==8)
                        immLiteral = imm_at_64;
                    else
                        assert(0 && "unsupported instruction size");

            } 
            return immLiteral;
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_LABEL(uint64_t input){
        	Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
			int64_t offset = sign_extend64(immLen, input * 4);
			Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));
			return makeAddExpression(lhs, rhs, s64);
	
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_SIMM4(uint64_t input){
		    return Immediate::makeImmediate(Result(s8, input));
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_SIMM8(uint64_t input){
		    return Immediate::makeImmediate(Result(s8, input));
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_SIMM16(uint64_t input){
		    return Immediate::makeImmediate(Result(s16, input));
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_SIMM32(uint64_t input){
		    return Immediate::makeImmediate(Result(s32, input));
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::decodeOPR_WAITCNT(uint64_t input){
		    return Immediate::makeImmediate(Result(s16, input));
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::makeRegisterExpression(MachRegister registerID){
            if(registerID == amdgpu_cdna2::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID);
        }
        Expression::Ptr InstructionDecoder_amdgpu_cdna2::makeRegisterExpression(MachRegister registerID, uint32_t low, uint32_t high ){
            if(registerID == amdgpu_cdna2::src_literal){
                return Immediate::makeImmediate(Result(u32,decodeOPR_LITERAL()));
            }
            return InstructionDecoderImpl::makeRegisterExpression(registerID, low, high );
        }



        void Dyninst::InstructionAPI::InstructionDecoder_amdgpu_cdna2::finalizeENC_VINTRPOperands(){
        
        }
#include "amdgpu_cdna2_decoder_impl.C"
#include "decodeOperands.C"
#include "finalizeOperands.C"
		inline unsigned int InstructionDecoder_amdgpu_cdna2::get32bit(InstructionDecoder::buffer &b,unsigned int offset ){
			assert(offset %4 ==0 );
            if(b.start + offset + 4 < b.end)
			    return b.start[offset+3] << 24 | b.start[offset + 2] << 16 | b.start[offset +1 ] << 8 | b.start [offset];
            return 0;
		}


		void InstructionDecoder_amdgpu_cdna2::reset(){
			immLen = 0;
			insn_size = 0;
			num_elements =1;
			isBranch = false;
			isConditional = false;
			isModifyPC =false;
			isSMEM = false;
			isLoad = false ;
			isStore =false;
			isBuffer =false ;
			isScratch = false;
			insn = insn_high = insn_long = 0;
			useImm = false;
			isCall = false;
			if (!getenv("DEBUG_DECODE"))
				cout.setstate(ios_base::badbit);
		}
		// here we assemble the first 64 bit (if available) as an instruction

		void InstructionDecoder_amdgpu_cdna2::setupInsnWord(InstructionDecoder::buffer &b) {
			reset();
			if (b.start > b.end)
				return;
			insn = get32bit(b,0);

			imm_at_32 = insn_high = get32bit(b,4);
			imm_at_64 = get32bit(b,8);

			insn_long = ( ((uint64_t) insn_high) << 32) | insn;
            cout << " setup insn_long = " <<  std::hex << insn_long << endl;

		}
		void InstructionDecoder_amdgpu_cdna2::decodeOpcode(InstructionDecoder::buffer &b) {
			setupInsnWord(b);
			mainDecode();
			b.start += insn_in_progress->size();
		}
		
		void InstructionDecoder_amdgpu_cdna2::debug_instr(){
			cout << "decoded instruction " <<  insn_in_progress->getOperation().mnemonic << " " << std::hex << insn_long << " insn_family = " << instr_family 
				<< "  length = " <<  insn_in_progress->size()<< endl << endl;

		}

		Instruction InstructionDecoder_amdgpu_cdna2::decode(InstructionDecoder::buffer &b) {
			setupInsnWord(b);
			mainDecode();
			if(entryToCategory(insn_in_progress->getOperation().getID())==c_BranchInsn){
                //cout << "Is Branch Instruction !! , name = " << insn_in_progress -> getOperation().mnemonic << endl;
				//std::mem_fun(decode_lookup_table[instr_family])(this);
			}
			debug_instr();
			cout.clear();
			b.start += insn_in_progress->size();
			return *insn_in_progress;
		}

		void InstructionDecoder_amdgpu_cdna2::doDelayedDecode(const Instruction *insn_to_complete) {

			InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
			setupInsnWord(b);
			mainDecode();
			debug_instr();
			cout.clear();
			Instruction* iptr = const_cast<Instruction*>(insn_to_complete);
            *iptr = *(insn_in_progress.get());
		}
	}
}



