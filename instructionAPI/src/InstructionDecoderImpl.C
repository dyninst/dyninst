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
#include "common/src/singleton_object_pool.h"
#include "InstructionDecoder-x86.h"
#include "InstructionDecoder-power.h"
#include "InstructionDecoder-aarch64.h"
#include "AMDGPU/gfx908/InstructionDecoder-amdgpu-gfx908.h"
#include "AMDGPU/gfx90a/InstructionDecoder-amdgpu-gfx90a.h"
#include "AMDGPU/gfx940/InstructionDecoder-amdgpu-gfx940.h"

#include "BinaryFunction.h"
#include "Dereference.h"
#include "Ternary.h"

using namespace std;
namespace Dyninst
{
    namespace InstructionAPI
    {
        boost::shared_ptr<Instruction> InstructionDecoderImpl::makeInstruction(entryID opcode, const char* mnem,
            unsigned int decodedSize, const unsigned char* raw)
        {
            Operation tmp(opcode, mnem, m_Arch);
            return make_shared(singleton_object_pool<Instruction>::construct(tmp, decodedSize, raw, m_Arch));
        }


        Instruction InstructionDecoderImpl::decode(InstructionDecoder::buffer& b)
        {
            //setMode(m_Arch == Arch_x86_64);
            const unsigned char* start = b.start;
            decodeOpcode(b);
            unsigned int decodedSize = b.start - start;

            return Instruction(m_Operation, decodedSize, start, m_Arch);
        }

        InstructionDecoderImpl::Ptr InstructionDecoderImpl::makeDecoderImpl(Architecture a)
        {
            switch(a)
            {
                case Arch_x86:
                case Arch_x86_64:
                    return Ptr(new InstructionDecoder_x86(a));
                case Arch_ppc32:
                case Arch_ppc64:
                    return Ptr(new InstructionDecoder_power(a));
                case Arch_aarch32:
                case Arch_aarch64:
                    return Ptr(new InstructionDecoder_aarch64(a));
                case Arch_amdgpu_gfx908:
                    return Ptr(new InstructionDecoder_amdgpu_gfx908(a));
               case Arch_amdgpu_gfx90a:
                    return Ptr(new InstructionDecoder_amdgpu_gfx90a(a));
               case Arch_amdgpu_gfx940:
                    return Ptr(new InstructionDecoder_amdgpu_gfx940(a));
                default:
                    assert(0);
                    return Ptr();
            }
        }
        Expression::Ptr InstructionDecoderImpl::makeAddExpression(Expression::Ptr lhs,
                Expression::Ptr rhs, Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());

            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, adder));
        }
        Expression::Ptr InstructionDecoderImpl::makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, multiplier));
        }
        Expression::Ptr InstructionDecoderImpl::makeLeftShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr leftShifter(new BinaryFunction::leftShiftResult());
            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, leftShifter));
        }
        Expression::Ptr InstructionDecoderImpl::makeRightArithmeticShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr rightArithmeticShifter(new BinaryFunction::rightArithmeticShiftResult());
            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, rightArithmeticShifter));
        }
        Expression::Ptr InstructionDecoderImpl::makeRightLogicalShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr rightLogicalShifter(new BinaryFunction::rightLogicalShiftResult());
            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, rightLogicalShifter));
        }
        Expression::Ptr InstructionDecoderImpl::makeRightRotateExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                Result_Type resultType)
        {
            BinaryFunction::funcT::Ptr rightRotator(new BinaryFunction::rightRotateResult());
            return make_shared(singleton_object_pool<BinaryFunction>::construct(lhs, rhs, resultType, rightRotator));
        }

        Expression::Ptr InstructionDecoderImpl::makeTernaryExpression(Expression::Ptr cond, Expression::Ptr first, Expression::Ptr second,Result_Type result_type){
            return make_shared(singleton_object_pool<TernaryAST>::construct(cond,first,second,result_type));
        }

        Expression::Ptr InstructionDecoderImpl::makeDereferenceExpression(Expression::Ptr addrToDereference,
                Result_Type resultType)
        {
            return make_shared(singleton_object_pool<Dereference>::construct(addrToDereference, resultType));
        }
        Expression::Ptr InstructionDecoderImpl::makeRegisterExpression(MachRegister registerID, uint32_t num_elements )
        {
            int newID = registerID.val();
            int minusArch = newID & ~(registerID.getArchitecture());
            int convertedID = minusArch | m_Arch;
            MachRegister converted(convertedID);
            return make_shared(singleton_object_pool<RegisterAST>::construct(converted, 0, registerID.size() * 8,num_elements));
        }
        

        Expression::Ptr InstructionDecoderImpl::makeRegisterExpression(MachRegister registerID, unsigned int start , unsigned int end)
        {
            int newID = registerID.val();
            int minusArch = newID & ~(registerID.getArchitecture());
            int convertedID = minusArch | m_Arch;
            MachRegister converted(convertedID);
            return make_shared(singleton_object_pool<RegisterAST>::construct(converted, start, end));
        }


        Expression::Ptr InstructionDecoderImpl::makeRegisterExpression(MachRegister registerID, Result_Type extendFrom)
        {
            int newID = registerID.val();
            int minusArch = newID & ~(registerID.getArchitecture());
            int convertedID = minusArch | m_Arch;
            MachRegister converted(convertedID);
            return make_shared(singleton_object_pool<RegisterAST>::construct(converted, 0, registerID.size() * 8, extendFrom));
        }
		Expression::Ptr InstructionDecoderImpl::makeMaskRegisterExpression(MachRegister registerID)
        {
            int newID = registerID.val();
            int minusArch = newID & ~(registerID.getArchitecture());
            int convertedID = minusArch | m_Arch;
            MachRegister converted(convertedID);
            return make_shared(singleton_object_pool<MaskRegisterAST>::construct(converted, 0, registerID.size() * 8));
        }
    }
}

