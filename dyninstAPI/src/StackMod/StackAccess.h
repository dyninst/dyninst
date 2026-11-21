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

#ifndef _STACKACCESS_H_
#define _STACKACCESS_H_

#include <map>
#include <set>
#include <string>
#include "registers/MachRegister.h"
#include "Instruction.h"
#include "stackanalysis.h"


class StackAccess {
    public:
        enum class StackAccessType {
            DEBUGINFO_LOCAL,
            DEBUGINFO_PARAM,
            SAVED,
            WRITE,
            UNKNOWN,
            READ,
            READWRITE,
            REGHEIGHT,
            DEFINITION,
            MISUNDERSTOOD
        };

        static std::string printStackAccessType(StackAccessType t);

        StackAccess() {
            _disp = 0;
            _skipReg = false;
        }

        ~StackAccess() {}

        Dyninst::MachRegister reg() const { return _reg; }
        void setReg(Dyninst::MachRegister r) { _reg = r; }

        Dyninst::StackAnalysis::Height regHeight() const { return _regHeight; }
        void setRegHeight(const Dyninst::StackAnalysis::Height &h) { _regHeight = h; }

        Dyninst::StackAnalysis::Definition regDef() const { return _regDef; }
        void setRegDef(const Dyninst::StackAnalysis::Definition &d) { _regDef = d; }

        Dyninst::StackAnalysis::Height readHeight() const { return _readHeight; }
        void setReadHeight(const Dyninst::StackAnalysis::Height &h) { _readHeight = h; }

        StackAccessType type() const { return _type; }
        void setType(StackAccessType t) { _type = t; }

        signed long disp() const { return _disp; }
        void setDisp(signed long d) { _disp = d; }

        bool skipReg() const { return _skipReg; }
        void setSkipReg(bool s) { _skipReg = s; }

        std::string format();
    private:
        Dyninst::MachRegister _reg;
        Dyninst::StackAnalysis::Height _regHeight;
        Dyninst::StackAnalysis::Definition _regDef;
        Dyninst::StackAnalysis::Height _readHeight;
        StackAccessType _type;
        signed long _disp;
        bool _skipReg;
};

typedef std::map<Dyninst::MachRegister, std::set<StackAccess*> > Accesses;

bool isDebugType(StackAccess::StackAccessType t);

int getAccessSize(Dyninst::InstructionAPI::Instruction insn);

bool getAccesses(Dyninst::ParseAPI::Function *func,
                 Dyninst::ParseAPI::Block *block,
                 Dyninst::Address addr,
                 Dyninst::InstructionAPI::Instruction insn,
                 Accesses *accesses,
                 std::set<Dyninst::Address> &defPointsToMod,
                 bool analyzeDefinition = false);

bool getMemoryOffset(Dyninst::ParseAPI::Function *func,
                     Dyninst::ParseAPI::Block *block,
                     Dyninst::InstructionAPI::Instruction insn,
                     Dyninst::Address addr,
                     const Dyninst::MachRegister &reg,
                     const Dyninst::StackAnalysis::Height &height,
                     const Dyninst::StackAnalysis::Definition &def,
                     StackAccess *&ret,
                     Dyninst::Architecture arch,
                     bool analyzeDefintion = false);


#endif
