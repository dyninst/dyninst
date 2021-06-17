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
#include "dyn_regs.h"

namespace Dyninst {
    namespace InstructionAPI {

        class ArchSpecificFormatter {
        public:
            virtual std::string getInstructionString(std::vector <std::string>) = 0;
            virtual std::string formatImmediate(std::string) = 0;
            virtual std::string formatDeref(std::string) = 0;
            virtual std::string formatRegister(std::string) = 0;
            virtual std::string formatBinaryFunc(std::string, std::string, std::string);
            virtual ~ArchSpecificFormatter() = default;
	    ArchSpecificFormatter& operator=(const ArchSpecificFormatter&) = default;
            static INSTRUCTION_EXPORT ArchSpecificFormatter& getFormatter(Dyninst::Architecture a);

        };

        class PPCFormatter : public ArchSpecificFormatter {
        public:
            PPCFormatter();

            virtual std::string getInstructionString(std::vector <std::string>);
            virtual std::string formatImmediate(std::string);
            virtual std::string formatDeref(std::string);
            virtual std::string formatRegister(std::string);
            virtual std::string formatBinaryFunc(std::string, std::string, std::string);

        };

        class ArmFormatter : public ArchSpecificFormatter {
        public:
            ArmFormatter();

            virtual std::string getInstructionString(std::vector <std::string>);
            virtual std::string formatImmediate(std::string);
            virtual std::string formatDeref(std::string);
            virtual std::string formatRegister(std::string);
            virtual std::string formatBinaryFunc(std::string, std::string, std::string);

        private:
            std::map<std::string, std::string> binaryFuncModifier;
        };

        class AmdgpuFormatter : public ArchSpecificFormatter {
        public:
            AmdgpuFormatter();

            virtual std::string getInstructionString(std::vector <std::string>);
            virtual std::string formatImmediate(std::string);
            virtual std::string formatDeref(std::string);
            virtual std::string formatRegister(std::string);
            virtual std::string formatBinaryFunc(std::string, std::string, std::string);

        private:
            std::map<std::string, std::string> binaryFuncModifier;
        };


        class x86Formatter : public ArchSpecificFormatter {
        public:
            x86Formatter();

            virtual std::string getInstructionString(std::vector <std::string>);
            virtual std::string formatImmediate(std::string);
            virtual std::string formatDeref(std::string);
            virtual std::string formatRegister(std::string);
            virtual std::string formatBinaryFunc(std::string, std::string, std::string);
        };

    }
}

#endif //DYNINST_ARCHSPECIFICFORMATTERS_H
