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

#include "InstructionDecoderImpl.h"
#include <iostream>
#include "Immediate.h"
#include "dyn_regs.h"

namespace Dyninst {
    namespace InstructionAPI
    {

#if defined(__GNUC__)
#define insn_printf(format, ...) \
        do{ \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ## __VA_ARGS__); \
        }while(0)
#endif

#define AARCH64_INSN_LENGTH	32

        struct aarch64_insn_entry;
		struct aarch64_mask_entry;

        class InstructionDecoder_aarch64 : public InstructionDecoderImpl
        {
            friend struct aarch64_insn_entry;
			friend struct aarch64_mask_entry;

            public:
                InstructionDecoder_aarch64(Architecture a);
                virtual ~InstructionDecoder_aarch64();
                virtual void decodeOpcode(InstructionDecoder::buffer& b);
                virtual Instruction::Ptr decode(InstructionDecoder::buffer& b);
				virtual void setMode(bool);

                virtual bool decodeOperands(const Instruction* insn_to_complete);
                virtual void doDelayedDecode(const Instruction* insn_to_complete);

                using InstructionDecoderImpl::makeRegisterExpression;
                
                #define	IS_INSN_ADDSUB_EXT(I)		(field<24, 28>(I) == 0x0B && field<21, 21>(I) == 1)
                #define	IS_INSN_ADDSUB_SHIFT(I)		(field<24, 28>(I) == 0x0B && field<21, 21>(I) == 0)
                #define	IS_INSN_ADDSUB_IMM(I)		(field<24, 28>(I) == 0x11)
                #define	IS_INSN_MOVEWIDE_IMM(I)		(field<23, 28>(I) == 0x25)
                #define	IS_INSN_LOGICAL_SHIFT(I)	(field<24, 28>(I) == 0x0A)
                #define	IS_INSN_LOADSTORE_REG(I)	(field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 1)
                #define	IS_INSN_LOGICAL_IMM(I)		(field<23, 28>(I) == 0x24)
                
            private:
                virtual Result_Type makeSizeType(unsigned int opType);

                // inherit from ppc is not sematically consistent with aarch64 manual
                template <int start, int end>
                int field(unsigned int raw)
                {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                    return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
                }

                template <int size>
                s32val sign_extend32(int in)
                {	
					s32val val = 0|in;
								
                    return (val << (32 - size)) >> (32 - size);
                }
                
                template <int size>
                s64val sign_extend64(int in)
                {					
					s64val val = 0|in;
					
                    return (val << (64 - size)) >> (64 - size);
                }
                
                template<int size>
                u32val unsign_extend32(int in)
                {	
					u32val mask = (!0);
								
                    return (mask>>(32-size)) & in;
				}
				
				template<int size>
                u64val unsign_extend32(int in)
                {	
					u64val mask = (!0);
								
                    return (mask>>(64-size)) & in;
				}
				
                // opcodes
                void mainDecode();
                int findInsnTableIndex(unsigned int);

                unsigned int insn;
                Instruction* insn_in_progress;
                				
				bool hasHw;
				int hw;
				void processHwFieldInsn();
				
				bool hasShift;
				int shiftField;
				int shiftTargetVal;
				int shiftTargetLen;
                int shiftAmount;
				int shiftLen;
				void processShiftFieldShiftedInsn();
				void processShiftFieldImmInsn();				
				
				bool hasOption;
				int optionField;
				void processOptionFieldExtendedInsn();
				void processOptionFieldLSRegOffsetInsn();
				
				bool isSystemInsn;
				int op0, op1, op2, crn, crm;
				void processSystemInsn();
				
				int sField;
				
				MachRegister makeAarch64RegID(MachRegister, unsigned int);
				Expression::Ptr makeRdExpr();
				Expression::Ptr makeRnExpr();
				Expression::Ptr makeRmExpr();
				Expression::Ptr makeRaExpr();
				Expression::Ptr makeRsExpr();
				
				void Rd();
				void Rn();
				void Rm();
				void sf();
				template<unsigned int startBit, unsigned int endBit> void option();
				void shift();
				void hw();
				/*template<unsigned int startBit, unsigned int endBit>
				void N()
				{
				}*/
				void Rt();
				void Rt2();
				void op1();
				void op2();
				template<unsigned int startBit, unsigned int endBit> void cond();
				void nzcv();
				void CRm();
				void CRn();
				template<unsigned int startBit, unsigned int endBit> void S();
				void Ra();
				void o0();
				void b5();
				void b40();
				/*void sz()
				{
				}*/
				void Rs();
				template<unsigned int startBit, unsigned int endBit> void imm();

				//TODO: Following needed?
				bool isRAWritten;
                bool invertBranchCondition;
                bool isFPInsn;
                bool is64Bit;
                bool bcIsConditional;
                template< int lowBit, int highBit>
                Expression::Ptr makeBranchTarget();
                Expression::Ptr makeFallThroughExpr();
        };
    }
}
