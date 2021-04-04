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
#include "InstructionDecoder-amdgpu-vega.h"

namespace Dyninst {
	namespace InstructionAPI {
		typedef void (InstructionDecoder_amdgpu_vega::*operandFactory)();

		typedef amdgpu_insn_entry amdgpu_insn_table[];
		typedef amdgpu_mask_entry amdgpu_decoder_table[];

		const std::array<std::string, 16> InstructionDecoder_amdgpu_vega::condNames = {
			"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
			"lt", "gt", "le", "al", "nv",
		};

		const char* InstructionDecoder_amdgpu_vega::bitfieldInsnAliasMap(entryID e) {
			switch(e) {
				default: assert(!"no alias for entryID");
			};
		}
		const char* InstructionDecoder_amdgpu_vega::condInsnAliasMap(entryID e) {
			switch(e) {
				default: assert(!"no alias for entryID");
			};
		};

#include "amdgpu_insn_entry.h"
		struct amdgpu_mask_entry {
			unsigned int mask;
			std::size_t branchCnt;
			const std::pair<unsigned int,unsigned int>* nodeBranches;
			int insnTableIndex;

			static const amdgpu_decoder_table main_decoder_table;
			static const std::pair<unsigned int,unsigned int> branchTable[];
		};

#include "amdgpu_opcode_tables.C"

		InstructionDecoder_amdgpu_vega::InstructionDecoder_amdgpu_vega(Architecture a)
			: InstructionDecoderImpl(a), 
            insn_size(0), immLen(0) , num_elements(1) , isSMEM(false), isLoad(false), isStore(false),isBuffer(false),
            isScratch(false) , isBranch(false), isConditional(false) ,isCall(false), isModifyPC(false)
		{
		}

		InstructionDecoder_amdgpu_vega::~InstructionDecoder_amdgpu_vega() {
		}


		using namespace std;
		void InstructionDecoder_amdgpu_vega::NOTHING() {
		}

		Result_Type InstructionDecoder_amdgpu_vega::makeSizeType(unsigned int) {
			assert(0); //not implemented
			return u32;
		}

		// ****************
		// decoding opcodes
		// ****************

		MachRegister InstructionDecoder_amdgpu_vega::makeAmdgpuRegID(MachRegister base, unsigned int encoding , unsigned int len) {
			MachRegister realBase = base;
			if (base == amdgpu_vega::sgpr0){
				switch(len){
					case 2:
						realBase = amdgpu_vega::sgpr_vec2_0;
						break;
					case 4:
						realBase = amdgpu_vega::sgpr_vec4_0;
						break;
					case 8:
						realBase = amdgpu_vega::sgpr_vec8_0;
						break;
					case 16:
						realBase = amdgpu_vega::sgpr_vec16_0;
						break;
				}
			}else if (base == amdgpu_vega::vgpr0){
				switch(len){
					case 2:
						realBase = amdgpu_vega::vgpr_vec2_0;
						break;
					case 4:
						realBase = amdgpu_vega::vgpr_vec4_0;
						break;
					case 8:
						realBase = amdgpu_vega::vgpr_vec8_0;
						break;
					case 16:
						realBase = amdgpu_vega::vgpr_vec16_0;
						break;
				}

			}
			return MachRegister(realBase.val() + encoding);

		}

		Expression::Ptr InstructionDecoder_amdgpu_vega::makePCExpr() {
			MachRegister baseReg = amdgpu_vega::pc;

			return makeRegisterExpression(baseReg);
		}

		void InstructionDecoder_amdgpu_vega::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal,
				int immLen = 16) {
			Expression::Ptr lhs = makeAddExpression(makePCExpr(),Immediate::makeImmediate(Result(s48,4)),s48);
			int64_t offset = sign_extend64(immLen, immVal * 4);

			Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

			insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional,
					false);
			if (bIsConditional || branchIsCall) {
				insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
			}

		}

		Expression::Ptr InstructionDecoder_amdgpu_vega::makeFallThroughExpr() {
			// TODO: while s_call_B64 is always 4 bytes, it is not clear whether all instructions that has a fall through branch are 4 bytes long
			return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
		}


		bool InstructionDecoder_amdgpu_vega::decodeOperands(const Instruction *) {
			assert(0 && "decodeOperands deprecated for amdgpu");
			return true;
		}

		Expression::Ptr InstructionDecoder_amdgpu_vega::decodeSGPRorM0(unsigned int offset){
			if( offset <= 104)
				return makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::sgpr0,offset));
			if (offset == 124)
				return makeRegisterExpression(amdgpu_vega::m0);
            cerr << " unknown offset in sgpr or m0 " << offset << endl; 
			assert(0 && "shouldn't reach here");
		}


		void InstructionDecoder_amdgpu_vega::finalizeFLATOperands(){
			layout_flat & layout = insn_layout.flat;

			Expression::Ptr addr_ast = 
				makeTernaryExpression(
						makeRegisterExpression(amdgpu_vega::address_mode_32), // TODO: type needs to be fixed
						makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.addr)),
						makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.addr,2)),
						u64
						);
			switch(layout.seg){
				case 1:
					insn_in_progress->getOperation().mnemonic += "_scratch";
					break;
				case 2:
					insn_in_progress->getOperation().mnemonic += "_global";
					break;
				default:
					break;
			}

			insn_in_progress->appendOperand(addr_ast,false,true);

		}
		void InstructionDecoder_amdgpu_vega::finalizeMUBUFOperands(){
			layout_mubuf & layout = insn_layout.mubuf;
			MachRegister vsharp = makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.srsrc<<2,4);
			Expression::Ptr const_base_ast   = makeRegisterExpression(vsharp,0,47); 
			Expression::Ptr const_stride_ast = makeRegisterExpression(vsharp,48,65); 
			Expression::Ptr num_records = makeRegisterExpression(vsharp,66,97); 
			Expression::Ptr add_tid_enable = makeRegisterExpression(vsharp,98,98); 
			Expression::Ptr swizzle_enable = makeRegisterExpression(vsharp,99,99); 
			Expression::Ptr element_size = makeRegisterExpression(vsharp,100,101); 
			Expression::Ptr index_stride = makeRegisterExpression(vsharp,102,103); 

			Expression::Ptr offset_expr = Immediate::makeImmediate(Result(u32,0)) ;
			Expression::Ptr index_expr = Immediate::makeImmediate(Result(u32,0)) ;

			if(layout.idxen){
				if(layout.offen){
					index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
					offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr+1));
				}else{
					index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
				}

			}else{
				if(layout.offen){
					offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
				}else{
					// do nothing
				}
			}


			Expression::Ptr index_ast = 
				makeAddExpression(
						index_expr
						,
						makeTernaryExpression(
							add_tid_enable,
							makeRegisterExpression(amdgpu_vega::tid),
							Immediate::makeImmediate(Result(u32,0)),
							u32
							),
						u32
						);

			Expression::Ptr offset_ast = 
				makeAddExpression(
						offset_expr,
						Immediate::makeImmediate(Result(u32,layout.offset)),
						u32
						);

			Expression::Ptr buffer_offset = makeAddExpression(
					offset_ast,
					makeMultiplyExpression(
						const_stride_ast,
						index_ast,
						u64
						),
					u64         
					);

			Expression::Ptr sgpr_offset_ast = decodeSSRC(layout.soffset);
			Expression::Ptr addr_ast = makeAddExpression(
					makeAddExpression(
						const_base_ast,
						sgpr_offset_ast,
						u64),
					buffer_offset,
					u64
					);

			Expression::Ptr vdata_ast = makeRegisterExpression(
					makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vdata,
						num_elements// TODO: This depends on number of elements, which is available from opcode
						));

			insn_in_progress->appendOperand(addr_ast,true,true);
			insn_in_progress->appendOperand(vdata_ast,true,true);

			if(layout.lds){
				Expression::Ptr lds_offset_ast = makeRegisterExpression(amdgpu_vega::m0,0,15);
				Expression::Ptr mem_offset_ast = makeRegisterExpression(
						makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.soffset));
				Expression::Ptr inst_offset_ast = Immediate::makeImmediate(Result(u32,layout.offset));
				Expression::Ptr lds_addr_ast =
					makeAddExpression(
							makeAddExpression(
								makeAddExpression(
									makeRegisterExpression(amdgpu_vega::lds_base),
									lds_offset_ast,
									u32
									),
								inst_offset_ast,
								u32
								),
							makeMultiplyExpression(
								makeRegisterExpression(amdgpu_vega::tid),
								Immediate::makeImmediate(Result(u16,4)),
								u32
								),
							u32
							);
				insn_in_progress->getOperation().mnemonic+="_lds";
				insn_in_progress->appendOperand(lds_addr_ast,false,true);

			}
		}


		void InstructionDecoder_amdgpu_vega::finalizeMTBUFOperands(){
			layout_mtbuf & layout = insn_layout.mtbuf;
			MachRegister vsharp = makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.srsrc<<2,4);
			Expression::Ptr const_base_ast   = makeRegisterExpression(vsharp,0,47); 
			Expression::Ptr const_stride_ast = makeRegisterExpression(vsharp,48,65); 
			Expression::Ptr num_records = makeRegisterExpression(vsharp,66,97); 
			Expression::Ptr add_tid_enable = makeRegisterExpression(vsharp,98,98); 
			Expression::Ptr swizzle_enable = makeRegisterExpression(vsharp,99,99); 
			Expression::Ptr element_size = makeRegisterExpression(vsharp,100,101); 
			Expression::Ptr index_stride = makeRegisterExpression(vsharp,102,103); 

			Expression::Ptr offset_expr = Immediate::makeImmediate(Result(u32,0)) ;
			Expression::Ptr index_expr = Immediate::makeImmediate(Result(u32,0)) ;

			if(layout.idxen){
				if(layout.offen){
					index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
					offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr+1));
				}else{
					index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
				}

			}else{
				if(layout.offen){
					offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vaddr));
				}else{
					// do nothing
				}
			}


			Expression::Ptr index_ast = 
				makeAddExpression(
						index_expr
						,
						makeTernaryExpression(
							add_tid_enable,
							makeRegisterExpression(amdgpu_vega::tid),
							Immediate::makeImmediate(Result(u32,0)),
							u32
							),
						u32
						);

			Expression::Ptr offset_ast = 
				makeAddExpression(
						offset_expr,
						Immediate::makeImmediate(Result(u32,layout.offset)),
						u32
						);

			Expression::Ptr buffer_offset = makeAddExpression(
					offset_ast,
					makeMultiplyExpression(
						const_stride_ast,
						index_ast,
						u64
						),
					u64         
					);

			Expression::Ptr sgpr_offset_ast = decodeSGPRorM0(layout.soffset);
			Expression::Ptr addr_ast = makeAddExpression(
					makeAddExpression(
						const_base_ast,
						sgpr_offset_ast,
						u64),
					buffer_offset,
					u64
					);

			Expression::Ptr vdata_ast = makeRegisterExpression(
					makeAmdgpuRegID(amdgpu_vega::vgpr0,layout.vdata,
						num_elements// TODO: This depends on number of elements, which is available from opcode
						));

			insn_in_progress->appendOperand(addr_ast,true,true);
			insn_in_progress->appendOperand(vdata_ast,true,true);


		}
		void InstructionDecoder_amdgpu_vega::finalizeVOP1Operands(){

			layout_vop1 & layout = insn_layout.vop1;
			//cout << " finalizing vop1 operands , vdst = " << std::dec << layout.vdst << " src0 = " << layout.src0 <<endl;

			insn_in_progress->appendOperand(decodeVDST(layout.vdst),false,true);
			insn_in_progress->appendOperand(decodeSSRC(layout.src0),true,false);

		}


		void InstructionDecoder_amdgpu_vega::finalizeSOP1Operands(){

			layout_sop1 & layout = insn_layout.sop1;

			if(isBranch){
				if(isModifyPC){
					Expression::Ptr new_pc_ast;
					new_pc_ast = makeRegisterExpression(
							makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.ssrc0,2)
							);

					// TODO : addSuccessors commented out to avoid jump table analysis aborts
					if(insn_in_progress->getOperation().operationID == amdgpu_op_s_setpc_b64){
                        
                        //RegisterAST::Ptr tmpqq = boost::dynamic_pointer_cast<RegisterAST>(new_pc_ast);
                        //std::cout << "setting pc to offset " << std::hex <<tmpqq->getID()<< std::endl; 
                        // non fall through branches are added as Successor
						//insn_in_progress->appendOperand(new_pc_ast,true,false);
					    insn_in_progress->addSuccessor(new_pc_ast, false, false, false, false);
					}else if(insn_in_progress->getOperation().operationID == amdgpu_op_s_swappc_b64){
						Expression::Ptr store_pc_ast;
    					store_pc_ast = makeRegisterExpression(
							makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.sdst,2)
							);

                    
					    // for swap pc we assume it is a call, so we add the fall through expr	
                        // iscall , is indirect, isconditional , isfall through
                        insn_in_progress->addSuccessor(new_pc_ast, false, true, false, false);
                        insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
						insn_in_progress->appendOperand(store_pc_ast,false,true);
					}
				}
			}else{
                if(insn_in_progress->getOperation().operationID == amdgpu_op_s_getpc_b64){

					Expression::Ptr dst_sgpr_pair_ast;
                    dst_sgpr_pair_ast  = makeRegisterExpression(
							makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.sdst,2)
							);
				    insn_in_progress->appendOperand(dst_sgpr_pair_ast,false,true);
                }else{
				    insn_in_progress->appendOperand(decodeSSRC(layout.sdst),false,true);
				    insn_in_progress->appendOperand(decodeSSRC(layout.ssrc0),true,false);
                }
			}

		}

		void InstructionDecoder_amdgpu_vega::finalizeSOPPOperands(){

			layout_sopp & layout = insn_layout.sopp;

			if(isBranch){
				if(!isModifyPC){	
					//cout << "calling make branch target "<< endl;
					makeBranchTarget(isCall,isConditional,layout.simm16);
				}	

			}

		}

		void InstructionDecoder_amdgpu_vega::finalizeSOPKOperands(){

			layout_sopp & layout = insn_layout.sopp;

			if(isBranch){
				if(!isModifyPC){	
					//cout << "calling make branch target "<< endl;

					makeBranchTarget(isCall,isConditional,layout.simm16);
				}	

			}

		}




		void InstructionDecoder_amdgpu_vega::finalizeSOP2Operands(){

			layout_sop2 & layout = insn_layout.sop2;
		    auto opID = insn_in_progress->getOperation().operationID ;	

            switch(opID){
                case amdgpu_op_s_add_u32:
                case amdgpu_op_s_addc_u32:
			        /*Expression::Ptr src0_expr = decodeSSRC(layout.ssrc0);
			        Expression::Ptr src1_expr = decodeSSRC(layout.ssrc1);
			        Expression::Ptr dst_expr = decodeSSRC(layout.sdst);
                
			        Expression::Ptr sum_expr = makeAddExpression(src0_expr,src1_expr,u32);

                    break;*/
                default:
                    insn_in_progress->appendOperand(decodeSSRC(layout.sdst),false,true);
                    insn_in_progress->appendOperand(decodeSSRC(layout.ssrc1),true,false);
                    insn_in_progress->appendOperand(decodeSSRC(layout.ssrc0),true,false);
                    break;
            }

		}
		void InstructionDecoder_amdgpu_vega::finalizeSMEMOperands(){
			layout_smem & layout = insn_layout.smem;

			if(IS_LD_ST()){
				Expression::Ptr offset_expr ;
				Expression::Ptr inst_offset_expr ;
				if(layout.imm==0){
					unsigned int indexing_offset = layout.soe ? layout.soffset : layout.offset;
					offset_expr = decodeSGPRorM0(indexing_offset);
					inst_offset_expr = Immediate::makeImmediate(Result(u64,0));
				}else{
					inst_offset_expr = Immediate::makeImmediate(Result(u32,layout.offset));
					offset_expr = layout.soe ? 
						decodeSGPRorM0(layout.soffset) : Immediate::makeImmediate(Result(u64,0));
				}
				//                cout << "layout.sbase = " << std::hex << layout.sbase << endl;
				//MachRegister mr = makeAmdgpuRegID(amdgpu_vega::sgpr0,4,2);
				//                cout << " shouldn't it be " << amdgpu_vega::sgpr_vec2_0 << " " << mr << endl; 
				Expression::Ptr sbase_expr = makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.sbase << 1,2));
				if(isScratch)
					offset_expr = makeMultiplyExpression(offset_expr,
							Immediate::makeImmediate(Result(u64,64)),u64);

				Expression::Ptr remain_expr = makeAddExpression(inst_offset_expr,offset_expr,u64);
				Expression::Ptr addr_expr = makeDereferenceExpression(makeAddExpression(sbase_expr,remain_expr,u64),u64);



				Expression::Ptr sdata_expr = makeRegisterExpression(makeAmdgpuRegID(amdgpu_vega::sgpr0,layout.sdata,num_elements));


				insn_in_progress->appendOperand(sdata_expr,true,false);

				insn_in_progress->appendOperand(addr_expr,false,true);

			}

		}

		void InstructionDecoder_amdgpu_vega::finalizeSOPCOperands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeVOPCOperands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeVOP2Operands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeVINTRPOperands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeDSOperands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeVOP3ABOperands() {
		}
		void InstructionDecoder_amdgpu_vega::finalizeVOP3POperands() {
		}

		bool InstructionDecoder_amdgpu_vega::decodeOperands(const amdgpu_insn_entry & insn_entry) {
			if(insn_entry.operandCnt!=0){
				for (std::size_t i =0 ; i < insn_entry.operandCnt; i++){
					std::mem_fun(insn_entry.operands[i])(this);
				}
			}
			return true;
		}


		inline unsigned int get32bit(InstructionDecoder::buffer &b,unsigned int offset ){
			assert(offset %4 ==0 );
			return b.start[offset+3] << 24 | b.start[offset + 2] << 16 | b.start[offset +1 ] << 8 | b.start [offset];
		}

		void InstructionDecoder_amdgpu_vega::insnSize(unsigned int insn_size) {
			this->insn_size = insn_size;
		}


		Expression::Ptr InstructionDecoder_amdgpu_vega::decodeVDST(unsigned int index){
			index += 255;
            return decodeSSRC(index);
		} 


		Expression::Ptr InstructionDecoder_amdgpu_vega::decodeVSRC(unsigned int index){
			if (index > 255)
				index = 0;
			MachRegister mr = makeAmdgpuRegID(amdgpu_vega::vgpr0,index & 0xff);
			return makeRegisterExpression(mr);
		} 

		Expression::Ptr InstructionDecoder_amdgpu_vega::decodeSSRC(unsigned int index){
			if (index < 102){
				MachRegister mr = makeAmdgpuRegID(amdgpu_vega::sgpr0,index);
				return makeRegisterExpression(mr);
			}else if(index ==102){
				return makeRegisterExpression(amdgpu_vega::flat_scratch_lo);
			}else if(index ==103){
				return makeRegisterExpression(amdgpu_vega::flat_scratch_hi);
			}else if(index ==104){
				return makeRegisterExpression(amdgpu_vega::xnack_mask_lo);
			}else if(index ==105){
				return makeRegisterExpression(amdgpu_vega::xnack_mask_hi);
			}else if ( 106 == index ){
				return makeRegisterExpression(amdgpu_vega::vcc_lo);
			}else if ( 107 == index ){
				return makeRegisterExpression(amdgpu_vega::vcc_hi);
			}else if (107 < index && index < 124){
				MachRegister mr = makeAmdgpuRegID(amdgpu_vega::ttmp0,index-108);
				return makeRegisterExpression(mr);
			}else if (124 == index ){
				return makeRegisterExpression(amdgpu_vega::m0);
			}else if ( 126 == index ){
				return makeRegisterExpression(amdgpu_vega::exec_lo);
			}else if ( 127 == index ){
				return makeRegisterExpression(amdgpu_vega::exec_hi);
			}else if( 128 <= index && index <= 192){
				return Immediate::makeImmediate(Result(u32, unsign_extend32(32,index - 128 )));
			}else if( 193 <= index && index <= 208 ){
				return Immediate::makeImmediate(Result(s32, sign_extend32(32,-(index - 192) )));
			}else if( 209 <= index && index <= 234){
				assert ( 0 && "reserved index " ) ;
			}else if( 235 == index){
				return makeRegisterExpression(amdgpu_vega::shared_base);
			}else if( 236 == index){
				return makeRegisterExpression(amdgpu_vega::shared_limit);
			}else if( 237 == index){
				return makeRegisterExpression(amdgpu_vega::private_base);
			}else if( 238 == index){
				return makeRegisterExpression(amdgpu_vega::private_limit);
			}else if( 239 == index){
				return makeRegisterExpression(amdgpu_vega::pops_exiting_wave_id);
			}else if (240 == index){
				return Immediate::makeImmediate(Result(sp_float, 0.5) );
			}else if (241 == index){
				return Immediate::makeImmediate(Result(sp_float, -0.5) );
			}else if (242 == index){
				return Immediate::makeImmediate(Result(sp_float, 1.0) );
			}else if (243 == index){
				return Immediate::makeImmediate(Result(sp_float, -1.0) );
			}else if (244 == index){
				return Immediate::makeImmediate(Result(sp_float, 2.0) );
			}else if (245 == index){
				return Immediate::makeImmediate(Result(sp_float, -2.0) );
			}else if (246 == index){
				return Immediate::makeImmediate(Result(sp_float, 4.0) );
			}else if (247 == index){
				return Immediate::makeImmediate(Result(sp_float, -4.0) );
			}else if (248 == index){
				/*
				 * TODO: 1/(2PI) = 0.15915494
				 * The exact value used is 
				 * half:
				 *      0x3118
				 * single:
				 *      0x3e22f983
				 * double:
				 *      0x3fc45f306dc9c882
				 */
				return Immediate::makeImmediate(Result(dp_float, (1.0f / (3.1415926535*2)) ) );
			}else if (249 == index  || 250 == index){
				assert ( 0 && "reserved index " ) ;
			}else if ( 251 == index ){
				return makeRegisterExpression(amdgpu_vega::vccz);
			}else if ( 252 == index ){
				return makeRegisterExpression(amdgpu_vega::execz);
			}else if ( 253 == index ){
				return makeRegisterExpression(amdgpu_vega::scc);
			}else if ( 254 == index ){
				assert ( 0 && "reserved index " ) ;
			}
			else if(index == 0xff){
				//std::cerr << "\nusing imm " << imm << std::endl;
				return Immediate::makeImmediate(Result(u32, unsign_extend32(32,immLiteral )));
			}else if( 256 <= index  && index <= 511){
				MachRegister mr = makeAmdgpuRegID(amdgpu_vega::vgpr0,index >>8);
				return makeRegisterExpression(mr);
			} 

			cerr << "WARNING UNKNOWN REGISTER : "  << std::dec << index << " " <<(108 <= index && index <= 123 )<<endl;
			//return makeRegisterExpression(amdgpu_vega::sgpr0);

			assert(0 && "unknown register value ");
		}
#include "amdgpu_decoder_impl_vega.C"

		void InstructionDecoder_amdgpu_vega::reset(){
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

		void InstructionDecoder_amdgpu_vega::setupInsnWord(InstructionDecoder::buffer &b) {
			reset();
			if (b.start > b.end)
				return;
			insn = get32bit(b,0);
			if(b.start + 4 <= b.end)
				insn_long = get32bit(b,4);
			insn_high = insn_long;
			insn_long = (insn_long << 32) | insn;

		}
		void InstructionDecoder_amdgpu_vega::decodeOpcode(InstructionDecoder::buffer &b) {
			setupInsnWord(b);
			mainDecodeOpcode(b);
			b.start += insn_in_progress->size();
		}
		
		void InstructionDecoder_amdgpu_vega::debug_instr(){
			cout << "\ndecoded instruction " <<  insn_in_progress->getOperation().mnemonic 
				<< "  length = " <<  insn_in_progress->size()<< endl;

		}


		Instruction InstructionDecoder_amdgpu_vega::decode(InstructionDecoder::buffer &b) {
			setupInsnWord(b);
			mainDecodeOpcode(b);
			if(entryToCategory(insn_in_progress->getOperation().getID())==c_BranchInsn){
				std::mem_fun(decode_lookup_table[instr_family])(this);
			}
			//debug_instr();
			cout.clear();
			b.start += insn_in_progress->size();
			return *insn_in_progress;
		}

		void InstructionDecoder_amdgpu_vega::doDelayedDecode(const Instruction *insn_to_complete) {
			InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
			setupInsnWord(b);
			mainDecode(b);
			//debug_instr();
			cout.clear();
			Instruction* iptr = const_cast<Instruction*>(insn_to_complete);
            *iptr = *(insn_in_progress.get());
			b.start += insn_in_progress->size();
		}
	};
};



