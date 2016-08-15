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

    static int binary_function_att(const Expression* exp, uint64_t* imm_val)
    {
        /* 0 = no failure, -1 = failure */
        int success = 0;

        /* Is this a binary function? */
        const BinaryFunction* root = dynamic_cast<const BinaryFunction*>(exp);

        if(root)
        {
            /* This is a sub binary function */
            std::vector<Expression::Ptr> children;
            /* Get the children for this function */
            root->getChildren(children);

            /* There must be two children here */
            if(children.size() != 2)
                return -1;

            /* Analyze the children */
            for(unsigned int x = 0;x < children.size();x++)
            {
                Expression* expression = &(*children[x]);

                /* Is this child somehow also a binary function? */
                BinaryFunction* child = dynamic_cast<BinaryFunction*>(expression);

                if(child)
                {
                    /* This child is a binary function, analyze this node seperately */
                    std::vector<Expression::Ptr> arg_children;
                    child->getChildren(arg_children);

                    if(arg_children.size() == 2)
                    {
                        success |= binary_function_att(child, imm_val);

                        /* Did this operation fail? */
                        if(success)
                            return success;

                    } else {
                        /* Invalid configuration */
                        return -1;
                    }
                } else {
                    /* This is a leaf, so it should be an imm or register */
                    Immediate* i = dynamic_cast<Immediate*>(expression);
                    RegisterAST* r = dynamic_cast<RegisterAST*>(expression);

                    /* It cannot be both an imm and a register. */
                    assert(i != nullptr || r != nullptr);
                    assert(!(i && r));

                    if(i)
                    {
                        // std::cout << "IMM VAL WAS " << *imm_val;
                        const InstructionAPI::Result current(u64, *imm_val);
                        const InstructionAPI::Result ret = i->eval() + current;
                        *imm_val = ret.convert<uint64_t>();
                        // std::cout << "IMM VAL IS NOW " << *imm_val << std::endl;
                    }
                }
            }
        } else {
            success = -1;
        }

        return success;
    }

    INSTRUCTION_EXPORT std::string Operand::format(Architecture arch, Address addr) const
    {
        if(!op_value) return "ERROR: format() called on empty operand!";

        if (addr) {

            Expression::Ptr thePC = Expression::Ptr(
                    new RegisterAST(MachRegister::getPC(arch)));

            op_value->bind(thePC.get(), Result(u32, addr));
            Result res = op_value->eval();
            if (res.defined) {
                stringstream ret;
                ret << hex << res.convert<unsigned>() << dec;
                return ret.str();
            }
        }

        /**
         * If this is a jump or IP relative load/store, this will be a 
         * binary function, so we have to parse the AST from hand
         */
        unsigned long imm_val = 0x0;
        if(!binary_function_att(&(*op_value), &imm_val))
        {
            std::stringstream ss;
            ss << "0x" << hex << imm_val;
            return ss.str();
        }

        return op_value->format();
    }
    INSTRUCTION_EXPORT Expression::Ptr Operand::getValue() const
    {
      return op_value;
    }
  };
};

