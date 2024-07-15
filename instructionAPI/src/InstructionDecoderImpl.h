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

#if !defined(INSTRUCTION_DECODER_IMPL_H)
#define INSTRUCTION_DECODER_IMPL_H

#include <stdint.h>
#include "Expression.h"
#include "Architecture.h"
#include "Operation_impl.h"
#include "entryIDs.h"
#include "Instruction.h"
#include "InstructionDecoder.h" // buffer...anything else?

namespace Dyninst
{
namespace InstructionAPI
{
class InstructionDecoderImpl
{
    public:
        typedef boost::shared_ptr<InstructionDecoderImpl> Ptr;
      
        InstructionDecoderImpl(Architecture a) : m_Arch(a) {}
        virtual ~InstructionDecoderImpl() {}
        virtual Instruction decode(InstructionDecoder::buffer& b);
        virtual void doDelayedDecode(const Instruction* insn_to_complete) = 0;
        virtual void setMode(bool is64) = 0;
        static Ptr makeDecoderImpl(Architecture a);

    protected:
      
        virtual bool decodeOperands(const Instruction* insn_to_complete) = 0;

        virtual void decodeOpcode(InstructionDecoder::buffer&) = 0;
      
        virtual Expression::Ptr makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
        virtual Expression::Ptr makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
        virtual Expression::Ptr makeLeftShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
        virtual Expression::Ptr makeRightArithmeticShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
        virtual Expression::Ptr makeRightLogicalShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
		virtual Expression::Ptr makeRightRotateExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType);
        virtual Expression::Ptr makeDereferenceExpression(Expression::Ptr addrToDereference, Result_Type resultType);



        virtual Expression::Ptr makeRegisterExpression(MachRegister reg, uint32_t num_elements = 1);
        // added version to support loading partial values out of register
        virtual Expression::Ptr makeRegisterExpression(MachRegister reg, unsigned int start , unsigned int end);
        virtual Expression::Ptr makeMaskRegisterExpression(MachRegister reg);
        virtual Expression::Ptr makeRegisterExpression(MachRegister reg, Result_Type extendFrom);
        virtual Result_Type makeSizeType(unsigned int opType) = 0;
        // added to support ternary value 
        virtual Expression::Ptr makeTernaryExpression(Expression::Ptr cond, Expression::Ptr first, Expression::Ptr second, Result_Type resultType);
        //Instruction* makeInstruction(entryID opcode, const char* mnem, unsigned int decodedSize,const unsigned char* raw);
        boost::shared_ptr<Instruction> makeInstruction(entryID opcode, const char* mnem, unsigned int decodedSize,
                                     const unsigned char* raw);
      
    protected:
        Operation m_Operation;
        Architecture m_Arch;

};

}
}

#endif //!defined(INSTRUCTION_DECODER_IMPL_H)
