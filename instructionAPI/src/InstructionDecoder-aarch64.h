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
                int sign_extend(int in)
                {
                    assert(0); //not verified
                    return (in << (32 - size)) >> (32 - size);
                }

                // opcodes
                void mainDecode();
                int findInsnTableIndex(unsigned int);

                unsigned int insn;
                Instruction* insn_in_progress;
                
				//TODO: Following needed?
				bool isRAWritten;
                bool invertBranchCondition;
                bool isFPInsn;
                bool bcIsConditional;
                template< int lowBit, int highBit>
                Expression::Ptr makeBranchTarget();
                Expression::Ptr makeFallThroughExpr();
        };
    }
}
