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

using namespace Dyninst;

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

        MachRegister reg() const { return _reg; }
        void setReg(MachRegister r) { _reg = r; }

        StackAnalysis::Height regHeight() const { return _regHeight; }
        void setRegHeight(const StackAnalysis::Height &h) { _regHeight = h; }

        StackAnalysis::Definition regDef() const { return _regDef; }
        void setRegDef(const StackAnalysis::Definition &d) { _regDef = d; }

        StackAnalysis::Height readHeight() const { return _readHeight; }
        void setReadHeight(const StackAnalysis::Height &h) { _readHeight = h; }

        StackAccessType type() const { return _type; }
        void setType(StackAccessType t) { _type = t; }

        signed long disp() const { return _disp; }
        void setDisp(signed long d) { _disp = d; }

        bool skipReg() const { return _skipReg; }
        void setSkipReg(bool s) { _skipReg = s; }

        std::string format();
    private:
        MachRegister _reg;
        StackAnalysis::Height _regHeight;
        StackAnalysis::Definition _regDef;
        StackAnalysis::Height _readHeight;
        StackAccessType _type;
        signed long _disp;
        bool _skipReg;
};

typedef std::map<MachRegister, std::set<StackAccess*> > Accesses;

bool isDebugType(StackAccess::StackAccessType t);

int getAccessSize(InstructionAPI::Instruction insn);

bool getAccesses(ParseAPI::Function *func,
                 ParseAPI::Block *block,
                 Address addr,
                 InstructionAPI::Instruction insn,
                 Accesses *accesses,
                 std::set<Address> &defPointsToMod,
                 bool analyzeDefinition = false);

bool getMemoryOffset(ParseAPI::Function *func,
                     ParseAPI::Block *block,
                     InstructionAPI::Instruction insn,
                     Address addr,
                     const MachRegister &reg,
                     const StackAnalysis::Height &height,
                     const StackAnalysis::Definition &def,
                     StackAccess *&ret,
                     Architecture arch,
                     bool analyzeDefintion = false);


#endif
