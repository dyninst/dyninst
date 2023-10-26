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

#ifndef DYNINST_ARCHSPECIFICFORMATTERS_H
#define DYNINST_ARCHSPECIFICFORMATTERS_H

#include <map>
#include <vector>
#include <string>
#include "Architecture.h"
#include "registers/MachRegister.h"

namespace Dyninst {
    namespace InstructionAPI {

        class ArchSpecificFormatter {
        public:
            virtual std::string getInstructionString(const std::vector <std::string>&) const;
            virtual std::string formatImmediate(const std::string&) const = 0;
            virtual std::string formatDeref(const std::string&)  const= 0;
            virtual std::string formatRegister(const std::string&)  const= 0;
            virtual std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const;
            virtual bool        operandPrintOrderReversed() const;
            virtual ~ArchSpecificFormatter() = default;
            ArchSpecificFormatter& operator=(const ArchSpecificFormatter&) = default;
            static INSTRUCTION_EXPORT ArchSpecificFormatter& getFormatter(Dyninst::Architecture a);

        };

        class PPCFormatter : public ArchSpecificFormatter {
        public:
            PPCFormatter();

            std::string formatImmediate(const std::string&) const override;
            std::string formatDeref(const std::string&) const override;
            std::string formatRegister(const std::string&) const override;
            std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override;

        };

        class ArmFormatter : public ArchSpecificFormatter {
        public:
            ArmFormatter();

            std::string formatImmediate(const std::string&) const override;
            std::string formatDeref(const std::string&) const override;
            std::string formatRegister(const std::string&) const override;
            std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override;

        private:
            std::map<std::string, std::string> binaryFuncModifier;
        };

        class AmdgpuFormatter : public ArchSpecificFormatter {
        public:
            AmdgpuFormatter();

            std::string formatImmediate(const std::string&) const override;
            std::string formatDeref(const std::string&) const override;
            std::string formatRegister(const std::string&) const override;
            std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override;
            // Helper function for formatting consecutive registers that are displayed as a single operand
            // Called when architecture is passed to Instruction.format.
            static std::string formatRegister(MachRegister m_Reg, uint32_t num_elements, unsigned m_Low , unsigned m_High );
        private:
            std::map<std::string, std::string> binaryFuncModifier;
        };


        class x86Formatter : public ArchSpecificFormatter {
        public:
            x86Formatter();

            std::string getInstructionString(const std::vector <std::string>&) const override;
            std::string formatImmediate(const std::string&) const override;
            std::string formatDeref(const std::string&) const override;
            std::string formatRegister(const std::string&) const override;
            std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override;
            bool        operandPrintOrderReversed() const override;
        };

    }
}

#endif //DYNINST_ARCHSPECIFICFORMATTERS_H
