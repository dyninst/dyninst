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

#define INSIDE_INSTRUCTION_API

#include <stdio.h>
#include <string>
#include <signal.h>
#include "../h/InstructionCategories.h"
#include "../h/Instruction.h"
#include "../h/Register.h"
#include "Operation_impl.h"
#include "InstructionDecoder.h"
#include "Dereference.h"
#include <boost/iterator/indirect_iterator.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <functional>
#include <algorithm>

#include "common/src/arch-x86.h"
#include "dyninstversion.h"

using namespace std;
using namespace NS_x86;

#include "../../common/src/singleton_object_pool.h"
#include "ArchSpecificFormatters.h"

#define DECODE_OPERANDS() \
    do { \
        if (arch_decoded_from != Arch_cuda && arch_decoded_from != Arch_amdgpu_gfx908 && arch_decoded_from != Arch_amdgpu_gfx90a && arch_decoded_from != Arch_amdgpu_gfx940 && m_Operands.empty()) { \
            decodeOperands(); \
        }\
    }while(0) 

namespace Dyninst
{
    namespace InstructionAPI
    {

        static const int IAPI_major_version = DYNINST_MAJOR_VERSION;
        static const int IAPI_minor_version = DYNINST_MINOR_VERSION;
        static const int IAPI_maintenance_version = DYNINST_PATCH_VERSION;

        void Instruction::version(int& major, int& minor, int& maintenance)
        {
            major = IAPI_major_version;
            minor = IAPI_minor_version;
            maintenance = IAPI_maintenance_version;
        }

        int Instruction::numInsnsAllocated = 0;
        INSTRUCTION_EXPORT Instruction::Instruction(Operation what,
                size_t size, const unsigned char* raw,
                Dyninst::Architecture arch)
            : m_InsnOp(what), m_Valid(what.getID() != e_No_Entry), arch_decoded_from(arch),
            formatter(&ArchSpecificFormatter::getFormatter(arch))
        {
            copyRaw(size, raw);

#if defined(DEBUG_INSN_ALLOCATIONS)
            numInsnsAllocated++;
            if((numInsnsAllocated % 1000) == 0)
            {
                fprintf(stderr, "Instruction CTOR, %d insns allocated\n", numInsnsAllocated);
            }
#endif    
        }

        void Instruction::copyRaw(size_t size, const unsigned char* raw)
        {

            if(raw)
            {
                m_size = size;
                m_RawInsn.small_insn = 0;
                if(size <= sizeof(m_RawInsn.small_insn))
                {
                    memcpy(&m_RawInsn.small_insn, raw, size);
                }
                else
                {
                    m_RawInsn.large_insn = new unsigned char[size];
                    memcpy(m_RawInsn.large_insn, raw, size);
                }
            }
            else
            {
                m_size = 0;
                m_RawInsn.small_insn = 0;
            }
        }

        void Instruction::decodeOperands() const
        {
        if (!m_Valid) return;
            //m_Operands.reserve(5);
            InstructionDecoder dec(ptr(), size(), arch_decoded_from);
            dec.doDelayedDecode(this);
        }

        INSTRUCTION_EXPORT Instruction::Instruction() :
            m_Valid(false), m_size(0), arch_decoded_from(Arch_none), formatter(nullptr)
        {
#if defined(DEBUG_INSN_ALLOCATIONS)
            numInsnsAllocated++;
            if((numInsnsAllocated % 1000) == 0)
            {
                fprintf(stderr, "Instruction CTOR, %d insns allocated\n", numInsnsAllocated);
            }
#endif
        }

        INSTRUCTION_EXPORT Instruction::~Instruction()
        {

            if(m_size > sizeof(m_RawInsn.small_insn))
            {
                delete[] m_RawInsn.large_insn;
            }

#if defined(DEBUG_INSN_ALLOCATIONS)
            numInsnsAllocated--;
            if((numInsnsAllocated % 1000) == 0)
            {
                fprintf(stderr, "Instruction DTOR, %d insns allocated\n", numInsnsAllocated);
            }
#endif      
        }

        INSTRUCTION_EXPORT Instruction::Instruction(const Instruction& o) :
            m_Operands(o.m_Operands),
            m_InsnOp(o.m_InsnOp),
            m_Valid(o.m_Valid),
            arch_decoded_from(o.arch_decoded_from),
            formatter(o.formatter)

        {
            m_size = o.m_size;
            if(o.m_size > sizeof(m_RawInsn.small_insn))
            {
                m_RawInsn.large_insn = new unsigned char[o.m_size];
                memcpy(m_RawInsn.large_insn, o.m_RawInsn.large_insn, m_size);
            }
            else
            {
                m_RawInsn.small_insn = o.m_RawInsn.small_insn;
            }

            m_Successors = o.m_Successors;

#if defined(DEBUG_INSN_ALLOCATIONS)
            numInsnsAllocated++;
            if((numInsnsAllocated % 1000) == 0)
            {
                fprintf(stderr, "Instruction COPY CTOR, %d insns allocated\n", numInsnsAllocated);
            }
#endif
        }

        INSTRUCTION_EXPORT const Instruction& Instruction::operator=(const Instruction& rhs)
        {
            m_Operands = rhs.m_Operands;
            //m_Operands.reserve(rhs.m_Operands.size());
            //std::copy(rhs.m_Operands.begin(), rhs.m_Operands.end(), std::back_inserter(m_Operands));
            if(m_size > sizeof(m_RawInsn.small_insn))
            {
                delete[] m_RawInsn.large_insn;
            }

            m_size = rhs.m_size;
            if(rhs.m_size > sizeof(m_RawInsn.small_insn))
            {
                m_RawInsn.large_insn = new unsigned char[rhs.m_size];
                memcpy(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size);
            }
            else
            {
                m_RawInsn.small_insn = rhs.m_RawInsn.small_insn;
            }


            m_InsnOp = rhs.m_InsnOp;
            m_Valid = rhs.m_Valid;
            formatter = rhs.formatter;
            arch_decoded_from = rhs.arch_decoded_from;
            m_Successors = rhs.m_Successors;
            return *this;
        }    

        INSTRUCTION_EXPORT bool Instruction::isValid() const
        {
            return m_Valid;
        }

        INSTRUCTION_EXPORT Operation& Instruction::getOperation()
        {
            return m_InsnOp;
        }
        INSTRUCTION_EXPORT const Operation& Instruction::getOperation() const
        {
            return m_InsnOp;
        }

        INSTRUCTION_EXPORT void Instruction::getOperands(std::vector<Operand>& operands) const
        {

            DECODE_OPERANDS();
            std::copy(m_Operands.begin(), m_Operands.end(), std::back_inserter(operands));
        }

        INSTRUCTION_EXPORT std::vector<Operand> Instruction::getDisplayOrderedOperands() const
        {
            DECODE_OPERANDS();

            std::vector<Operand> operands;
            auto operandsInserter{std::back_inserter(operands)};
            auto isNotImplicitPred = [](const Operand & x){return !x.isImplicit();};

            if (formatter->operandPrintOrderReversed())  {
                copy_if(m_Operands.crbegin(), m_Operands.crend(), operandsInserter, isNotImplicitPred);
            }  else  {
                copy_if(m_Operands.cbegin(), m_Operands.cend(), operandsInserter, isNotImplicitPred);
            }

            return operands;
        }

        INSTRUCTION_EXPORT Operand Instruction::getOperand(int index) const
        {
            DECODE_OPERANDS();
            if(index < 0 || index >= (int)(m_Operands.size()))
            {
                // Out of range = empty operand
                return Operand(Expression::Ptr(), false, false);
            }
            std::list<Operand>::const_iterator found = m_Operands.begin();
            std::advance(found, index);
            return *found;
        }

        INSTRUCTION_EXPORT const void* Instruction::ptr() const
        {
            if(m_size > sizeof(m_RawInsn.small_insn))
            {
                return m_RawInsn.large_insn;
            }
            else
            {
                return reinterpret_cast<const void*>(&m_RawInsn.small_insn);
            }
        }
        INSTRUCTION_EXPORT unsigned char Instruction::rawByte(unsigned int index) const
        {
            if(index >= m_size) return 0;
            if(m_size > sizeof(m_RawInsn.small_insn))
            {
                return m_RawInsn.large_insn[index];
            }
            else
            {
                return reinterpret_cast<const unsigned char*>(&m_RawInsn.small_insn)[index];
            }
        }

        INSTRUCTION_EXPORT size_t Instruction::size() const
        {
            return m_size;

        }

        INSTRUCTION_EXPORT void Instruction::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const
        {
            DECODE_OPERANDS();
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                curOperand->getReadSet(regsRead);
            }
            std::copy(m_InsnOp.implicitReads().begin(), m_InsnOp.implicitReads().end(),
                    std::inserter(regsRead, regsRead.begin()));

        }

        INSTRUCTION_EXPORT void Instruction::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const
        { 
            DECODE_OPERANDS();
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                curOperand->getWriteSet(regsWritten);
            }
            std::copy(m_InsnOp.implicitWrites().begin(), m_InsnOp.implicitWrites().end(),
                    std::inserter(regsWritten, regsWritten.begin()));

        }

        INSTRUCTION_EXPORT bool Instruction::isRead(Expression::Ptr candidate) const
        {
            DECODE_OPERANDS();
            for(std::list<Operand >::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                // Check if the candidate is read as an explicit operand
                if(curOperand->isRead(candidate))
                {
                    return true;
                }
            }
            // Check if the candidate is read as an implicit operand
            return m_InsnOp.isRead(candidate);
        }

        INSTRUCTION_EXPORT bool Instruction::isWritten(Expression::Ptr candidate) const
        {
            DECODE_OPERANDS();

            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                if(curOperand->isWritten(candidate))
                {
                    return true;
                }
            }
            return m_InsnOp.isWritten(candidate);
        }

        INSTRUCTION_EXPORT bool Instruction::readsMemory() const
        {
            DECODE_OPERANDS();
            if(getCategory() == c_PrefetchInsn)
            {
                return false;
            }
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                if(curOperand->readsMemory())
                {
                    return true;
                }
            }
            return !m_InsnOp.getImplicitMemReads().empty();
        }

        INSTRUCTION_EXPORT bool Instruction::writesMemory() const
        {
            DECODE_OPERANDS();
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                if(curOperand->writesMemory())
                {
                    return true;
                }
            }
            return !m_InsnOp.getImplicitMemWrites().empty();
        }

        INSTRUCTION_EXPORT void Instruction::getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const
        {
            DECODE_OPERANDS();
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                curOperand->addEffectiveReadAddresses(memAccessors);
            }  
            std::copy(m_InsnOp.getImplicitMemReads().begin(), m_InsnOp.getImplicitMemReads().end(), std::inserter(memAccessors,
                        memAccessors.begin()));
        }

        INSTRUCTION_EXPORT void Instruction::getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const
        {
            DECODE_OPERANDS();
            for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
                    curOperand != m_Operands.end();
                    ++curOperand)
            {
                curOperand->addEffectiveWriteAddresses(memAccessors);
            }  
            std::copy(m_InsnOp.getImplicitMemWrites().begin(), m_InsnOp.getImplicitMemWrites().end(), std::inserter(memAccessors,
                        memAccessors.begin()));
        }

        INSTRUCTION_EXPORT Operand Instruction::getPredicateOperand() const
        {
            DECODE_OPERANDS();
            for(auto const &op : m_Operands) {
                if (op.isTruePredicate() || op.isFalsePredicate()) {
                    return op;
                }
            }

            return Operand(Expression::Ptr(), false, false);
        }
        INSTRUCTION_EXPORT bool Instruction::hasPredicateOperand() const
        {
            DECODE_OPERANDS();
            for(auto const &op : m_Operands) {
                if (op.isTruePredicate() || op.isFalsePredicate()) {
                    return true;
                }
            }

            return false;
        }

        INSTRUCTION_EXPORT Expression::Ptr Instruction::getControlFlowTarget() const
        {
            // We assume control flow transfer instructions have the PC as
            // an implicit write, and that we have decoded the control flow
            // target's full location as the first and only operand.
            // If this is not the case, we'll squawk for the time being...
            if(getCategory() == c_NoCategory ||
                    getCategory() == c_CompareInsn ||
                    getCategory() == c_PrefetchInsn)
            {
                return Expression::Ptr();
            }
            if(getCategory() == c_ReturnInsn)
            {
                return makeReturnExpression();
            }
            DECODE_OPERANDS();
            if(m_Successors.empty())
            {
                return Expression::Ptr();
            }
            return m_Successors.front().target;
        }

        INSTRUCTION_EXPORT ArchSpecificFormatter& Instruction::getFormatter() const {
            return *formatter;
        }

        INSTRUCTION_EXPORT std::string Instruction::format(Address addr) const
        {
            // if Arch_none, this is an error and the formatter is nullptr,
            // so return an error string (could also abort or except)
            if (arch_decoded_from == Arch_none)  {
                return "ERROR_NO_ARCH_SET_FOR_INSTRUCTION";
            }

            DECODE_OPERANDS();
            //remove this once ArchSpecificFormatter is extended for all architectures

            std::string opstr = m_InsnOp.format();
            opstr += " ";
            std::list<Operand>::const_iterator currOperand;
            std::vector<std::string> formattedOperands;
            for(currOperand = m_Operands.begin();
                    currOperand != m_Operands.end();
                    ++currOperand)
            {
                /* If this operand is implicit, don't put it in the list of operands. */
                if(currOperand->isImplicit())
                    continue;

                formattedOperands.push_back(currOperand->format(getArch(), addr));
            }

#if defined(DEBUG_READ_WRITE)      
            std::set<RegisterAST::Ptr> tmp;
            getReadSet(tmp);
            cout << "Read set:" << endl;
            for(std::set<RegisterAST::Ptr>::iterator i = tmp.begin();
                    i != tmp.end();
                    ++i)
            {
                cout << (*i)->format() << " ";
            }
            cout << endl;
            tmp.clear();
            getWriteSet(tmp);
            cout << "Write set:" << endl;
            for(std::set<RegisterAST::Ptr>::iterator i = tmp.begin();
                    i != tmp.end();
                    ++i)
            {
                cout << (*i)->format() << " ";
            }
            cout << endl;
            std::set<Expression::Ptr> mem;
            getMemoryReadOperands(mem);
            cout << "Read mem:" << endl;
            for(std::set<Expression::Ptr>::iterator i = mem.begin();
                    i != mem.end();
                    ++i)
            {
                cout << (*i)->format() << " ";
            }
            cout << endl;
            mem.clear();
            getMemoryWriteOperands(mem);
            cout << "Write mem:" << endl;
            for(std::set<Expression::Ptr>::iterator i = mem.begin();
                    i != mem.end();
                    ++i)
            {
                cout << (*i)->format() << " ";
            }
            cout << endl;
#endif // defined(DEBUG_READ_WRITE)

            return opstr + formatter->getInstructionString(formattedOperands);
        }

        INSTRUCTION_EXPORT bool Instruction::allowsFallThrough() const
        {
            switch(m_InsnOp.getID())
            {
                case e_ret_far:
                case e_ret_near:
                case e_iret:
                case e_jmp:
                case e_hlt:
                case e_sysret:
                case e_sysexit:
                case e_call:
                case e_syscall:
                case amdgpu_gfx908_op_S_SETPC_B64:
                case amdgpu_gfx908_op_S_SWAPPC_B64:
                case amdgpu_gfx90a_op_S_SETPC_B64:
                case amdgpu_gfx90a_op_S_SWAPPC_B64:
                case amdgpu_gfx940_op_S_SETPC_B64:
                case amdgpu_gfx940_op_S_SWAPPC_B64:
                    return false;
                case e_jae:
                case e_jb:
                case e_jb_jnaej_j:
                case e_jbe:
                case e_jcxz_jec:
                case e_jl:
                case e_jle:
                case e_jnb_jae_j:
                case e_ja:
                case e_jge:
                case e_jg:
                case e_jno:
                case e_jnp:
                case e_jns:
                case e_jne:
                case e_jo:
                case e_jp:
                case e_js:
                case e_je:
                    return true;
                default:
                    {
                        DECODE_OPERANDS();
                        for(cftConstIter targ = m_Successors.begin();
                                targ != m_Successors.end();
                                ++targ)
                        {
                            if(targ->isFallthrough) return true;
                        }
                        return m_Successors.empty();
                    }
            }
            // can't happen but make the compiler happy
            return false;
        }
        INSTRUCTION_EXPORT bool Instruction::isLegalInsn() const
        {
            return (m_InsnOp.getID() != e_No_Entry);
        }

        INSTRUCTION_EXPORT Architecture Instruction::getArch() const {
            return arch_decoded_from;
        }

        Expression::Ptr Instruction::makeReturnExpression() const
        {
            Expression::Ptr stackPtr = Expression::Ptr(new RegisterAST(MachRegister::getStackPointer(arch_decoded_from),
                        0, MachRegister::getStackPointer(arch_decoded_from).size()));
            Expression::Ptr retLoc = Expression::Ptr(new Dereference(stackPtr, u32));
            return retLoc;
        }
        INSTRUCTION_EXPORT InsnCategory Instruction::getCategory() const
        {
            if(m_InsnOp.isVectorInsn) return c_VectorInsn;
            InsnCategory c = entryToCategory(m_InsnOp.getID());
            if(c == c_BranchInsn && (arch_decoded_from == Arch_ppc32 || arch_decoded_from == Arch_ppc64))
            {
                DECODE_OPERANDS();
                for(cftConstIter cft = cft_begin();
                        cft != cft_end();
                        ++cft)
                {
                    if(cft->isCall)
                    {
                        return c_CallInsn;
                    }
                }
                if(m_InsnOp.getID() == power_op_bclr)
                {
                    return c_ReturnInsn;
                }
            }
            return c;
        }
        void Instruction::addSuccessor(Expression::Ptr e, 
                bool isCall, 
                bool isIndirect, 
                bool isConditional, 
                bool isFallthrough,
		bool isImplicit) const
        {
            CFT c(e, isCall, isIndirect, isConditional, isFallthrough);
            m_Successors.push_back(c);
            if (!isFallthrough) appendOperand(e, true, false, isImplicit);
        }

        void Instruction::appendOperand(Expression::Ptr e, 
                bool isRead, bool isWritten, bool isImplicit, bool trueP, bool falseP) const
        {
            m_Operands.push_back(Operand(e, isRead, isWritten, isImplicit, trueP, falseP));
        }


    }
}

