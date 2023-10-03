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

#if !defined(_ROSE_INSN_FACTORY_H_)
#define _ROSE_INSN_FACTORY_H_

#include "entryIDs.h"
#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "external/rose/armv8InstructionEnum.h"
#include "external/rose/amdgpuInstructionEnum.h"
#include "Visitor.h"
#include "Instruction.h"
#include "common/h/util.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include <stddef.h>
#include <string>

#include <stdint.h>


class SgAsmInstruction;

class SgAsmx86Instruction;

class SgAsmExpression;

class SgAsmPowerpcInstruction;

class SgAsmOperandList;

class SgAsmx86RegisterReferenceExpression;

class SgAsmPowerpcRegisterReferenceExpression;

namespace Dyninst {
    namespace InstructionAPI {
        class RegisterAST;

        class Dereference;

        class Immediate;

        class BinaryFunction;

        class Expression;

        class Operand;

        class Instruction;
    }

    namespace DataflowAPI {
        class RoseInsnFactory {
        protected:
            typedef boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr;
            typedef boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;
            uint64_t _addr = 0 ;
        public:
            DATAFLOW_EXPORT RoseInsnFactory(void) { }

            DATAFLOW_EXPORT virtual ~RoseInsnFactory(void) { }

            DATAFLOW_EXPORT virtual SgAsmInstruction *convert(const InstructionAPI::Instruction &insn, uint64_t addr);

        protected:
            virtual SgAsmInstruction *createInsn() = 0;

            virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem) = 0;

            virtual void setSizes(SgAsmInstruction *insn) = 0;

            virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands) = 0;

            virtual void massageOperands(const InstructionAPI::Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands) = 0;

            virtual SgAsmExpression *convertOperand(const ExpressionPtr expression, int64_t addr, size_t insnSize);

            friend class ExpressionConversionVisitor;

            virtual Architecture arch() { return Arch_none; }
        };

        class RoseInsnX86Factory : public RoseInsnFactory {
        public:
            DATAFLOW_EXPORT RoseInsnX86Factory(Architecture arch) : a(arch) { }

            DATAFLOW_EXPORT virtual ~RoseInsnX86Factory() { }

        private:
            Architecture a;

            virtual SgAsmInstruction *createInsn();

            virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);

            virtual void setSizes(SgAsmInstruction *insn);

            virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);

            virtual void massageOperands(const InstructionAPI::Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands);

            X86InstructionKind convertKind(entryID opcode, prefixEntryID prefix);

            virtual Architecture arch() { return a; }
        };

        class RoseInsnPPCFactory : public RoseInsnFactory {
        public:
            DATAFLOW_EXPORT RoseInsnPPCFactory(void) { }

            DATAFLOW_EXPORT virtual ~RoseInsnPPCFactory(void) { }

        private:
            virtual SgAsmInstruction *createInsn();

            virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);

            virtual void setSizes(SgAsmInstruction *insn);

            virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);

            virtual void massageOperands(const InstructionAPI::Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands);

            PowerpcInstructionKind convertKind(entryID opcode, std::string mnem);

            PowerpcInstructionKind makeRoseBranchOpcode(entryID iapi_opcode, bool isAbsolute, bool isLink);

            virtual Architecture arch() { return Arch_ppc32; }
            PowerpcInstructionKind kind;
        };

        class RoseInsnArmv8Factory : public RoseInsnFactory {
        public:
            DATAFLOW_EXPORT RoseInsnArmv8Factory(Architecture arch) : a(arch) { }

            DATAFLOW_EXPORT virtual ~RoseInsnArmv8Factory() { }

        private:
            Architecture a;

            virtual SgAsmInstruction *createInsn();

            virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);

            virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);

            virtual void massageOperands(const InstructionAPI::Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands);

            virtual void setSizes(SgAsmInstruction *insn);

            ARMv8InstructionKind convertKind(entryID opcode);

            virtual Architecture arch() { return a; }
        };

        class RoseInsnAMDGPUFactory : public RoseInsnFactory {
        public:
            DATAFLOW_EXPORT RoseInsnAMDGPUFactory(Architecture arch) : a(arch) { }

            DATAFLOW_EXPORT virtual ~RoseInsnAMDGPUFactory() { }

        private:
            Architecture a;

            virtual SgAsmInstruction *createInsn();

            virtual void setOpcode(SgAsmInstruction *insn, entryID opcode, prefixEntryID prefix, std::string mnem);

            virtual bool handleSpecialCases(entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands);

            virtual void massageOperands(const InstructionAPI::Instruction &insn,
                                         std::vector<InstructionAPI::Operand> &operands );

            virtual void setSizes(SgAsmInstruction *insn);

            AMDGPUInstructionKind convertKind(entryID opcode);

            virtual Architecture arch() { return a; }
        };
       
    }
}

#endif
