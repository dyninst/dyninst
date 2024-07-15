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

#include "IAPI_to_AST.h"

#include "Register.h"
#include "BinaryFunction.h"
#include "Immediate.h"
#include "Dereference.h"
#if defined(arch_x86) || defined(arch_x86_64)
#include "RegisterConversion.h"
#endif

using namespace Dyninst::InstructionAPI;

void ASTFactory::visit(BinaryFunction* b)
{
    AstNodePtr rhs = m_stack.back();
    m_stack.pop_back();
    AstNodePtr lhs = m_stack.back();
    m_stack.pop_back();
    if(b->isAdd())
    {
        m_stack.push_back(AstNode::operatorNode(
                plusOp,
                lhs,
                rhs));
    }
    else if(b->isMultiply())
    {
        m_stack.push_back(AstNode::operatorNode(
                timesOp,
        lhs,
        rhs));
    }
    else
    {
        assert(0);
    }
}

void ASTFactory::visit(Dereference* )
{
    AstNodePtr effaddr = m_stack.back();
    m_stack.pop_back();
	m_stack.push_back(AstNode::operandNode(AstNode::operandType::DataIndir, effaddr));
}

void ASTFactory::visit(Immediate* i)
{
    m_stack.push_back(AstNode::operandNode(AstNode::operandType::Constant,
                    (void*)(i->eval().convert<long>())));
}

void ASTFactory::visit(RegisterAST* r)
{
#if defined(arch_x86) || defined(arch_x86_64)  
    bool unused;
    m_stack.push_back(AstNode::operandNode(AstNode::operandType::origRegister,
                      (void*)(intptr_t)(convertRegID(r, unused))));
#else
    MachRegister reg = r->getID();
    reg = reg.getBaseRegister();
    Register astreg = reg.val() & ~reg.getArchitecture();
    m_stack.push_back(AstNode::operandNode(AstNode::origRegister,
                      (void*)(astreg)));
#endif
}
