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

#include "../h/Operand.h"
#include "../h/Dereference.h"
#include "../h/Register.h"
#include "../h/Immediate.h"
#include "../h/Expression.h"
#include "../h/BinaryFunction.h"
#include "../h/Result.h"
#include <iostream>

using namespace std;

namespace Dyninst
{
    namespace InstructionAPI
    {
        INSTRUCTION_EXPORT void Operand::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
        {
            std::set<InstructionAST::Ptr> useSet;
            // This thing returns something only for RegisterAST Expression
            op_value->getUses(useSet);
            std::set<InstructionAST::Ptr>::const_iterator curUse;
            for(curUse = useSet.begin(); curUse != useSet.end(); ++curUse)
            {
                RegisterAST::Ptr tmp = boost::dynamic_pointer_cast<RegisterAST>(*curUse);
                if(tmp) 
                {
                    if(m_isRead || !(*tmp == *op_value))
                        regsRead.insert(tmp);
                }
            }
        }
        INSTRUCTION_EXPORT void Operand::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
        {
            RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
            if(m_isWritten && op_as_reg)
            {
                regsWritten.insert(op_as_reg);
            }
        }

        INSTRUCTION_EXPORT RegisterAST::Ptr Operand::getPredicate() const
        {
            RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
            if (m_isTruePredicate || m_isFalsePredicate) {
                return op_as_reg;
            }
            return nullptr;
        }

        INSTRUCTION_EXPORT bool Operand::isRead(Expression::Ptr candidate) const
        {
            // The whole expression of a read, any subexpression of a write
            return op_value->isUsed(candidate) && (m_isRead || !(*candidate == *op_value));
        }
        INSTRUCTION_EXPORT bool Operand::isWritten(Expression::Ptr candidate) const
        {
            // Whole expression of a write
            return m_isWritten && (*op_value == *candidate);
        }    
        INSTRUCTION_EXPORT bool Operand::readsMemory() const
        {
            return (boost::dynamic_pointer_cast<Dereference>(op_value) && m_isRead);
        }
        INSTRUCTION_EXPORT bool Operand::writesMemory() const
        {
            return (boost::dynamic_pointer_cast<Dereference>(op_value) && m_isWritten);
        }
        INSTRUCTION_EXPORT void Operand::addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const
        {
            if(m_isRead && boost::dynamic_pointer_cast<Dereference>(op_value))
            {
                std::vector<Expression::Ptr> tmp;
                op_value->getChildren(tmp);
                for(std::vector<Expression::Ptr>::const_iterator curKid = tmp.begin();
                        curKid != tmp.end();
                        ++curKid)
                {
                    memAccessors.insert(*curKid);
                }
            }
        }
        INSTRUCTION_EXPORT void Operand::addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const
        {
            if(m_isWritten && boost::dynamic_pointer_cast<Dereference>(op_value))
            {
                std::vector<Expression::Ptr> tmp;
                op_value->getChildren(tmp);
                for(std::vector<Expression::Ptr>::const_iterator curKid = tmp.begin();
                        curKid != tmp.end();
                        ++curKid)
                {
                    memAccessors.insert(*curKid);
                }
            }
        }


        INSTRUCTION_EXPORT std::string Operand::format(Architecture arch, Address addr) const
        {
            if(!op_value) return "ERROR: format() called on empty operand!";
            if (addr) {
                Expression::Ptr thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(arch)));
                op_value->bind(thePC.get(), Result(u32, addr));
                Result res = op_value->eval();
                if (res.defined) {
                    char hex[20];
                    snprintf(hex, 20, "0x%lx", res.convert<uintmax_t>());
                    return string(hex);
                }
            }
            auto s = op_value->format(arch);
            if (!s.compare(0, 2, "##"))  {
                s.replace(0, 2, "0x0(");	// fix-up ##X to indirection syntax 0x0(X)
                s += ')';
            }
            return s;
        }

        INSTRUCTION_EXPORT Expression::Ptr Operand::getValue() const
        {
            return op_value;
        }
    }
}

