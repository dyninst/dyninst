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

#include "Expression.h"

namespace Dyninst
{
    namespace InstructionAPI
    {
        Expression::Expression(Result_Type t) :
            InstructionAST(), userSetValue(t)
        {
        } 
        Expression::Expression(MachRegister r) :
            InstructionAST()
        {
            switch(r.size())
            {
                case 1:
                    userSetValue = Result(u8);
                    break;
                case 2:
                    userSetValue = Result(u16);
                    break;
                case 4:
                    userSetValue = Result(u32);
                    break;
                case 6:
                    userSetValue = Result(u48);
                    break;
                case 8:
                    userSetValue = Result(u64);
                    break;
                case 10:
                    userSetValue = Result(dp_float);
                    break;
                case 16:
                    userSetValue = Result(dbl128);
                    break;
                case 32:
                    userSetValue = Result(m256);
                    break;
                case 64:
                    userSetValue = Result(m512);
                    break;
                case 0:
                    // Special case for bitfields
                    userSetValue = Result(bit_flag);
                    break;
                default:
                    assert(!"unexpected machine register size!");
            }
        }
        Expression::~Expression()
        {
        }
        const Result& Expression::eval() const
        {
            return userSetValue;
        }
        void Expression::setValue(const Result& knownValue) 
        {
            userSetValue = knownValue;
        }
        void Expression::clearValue()
        {
            userSetValue.defined = false;
        }
        int Expression::size() const
        {
            return userSetValue.size();
        }
        bool Expression::bind(Expression* expr, const Result& value)
        {
            //bool retVal = false;
            if(*expr == *this)
            {
                setValue(value);
                return true;
            }
            return false;
        }
        bool Expression::isFlag() const
        {
            return false;
        }
        bool DummyExpr::isStrictEqual(const InstructionAST& ) const
        {
            return true;
        }
        bool DummyExpr::checkRegID(MachRegister, unsigned int, unsigned int) const
        {
            return true;
        }

    }
}
