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

#include <sstream>

#include "debug.h"

#include "Instruction.h"
#include "InstructionCategories.h"
#include "InstructionDecoder.h"
#include "Expression.h"
#include "Result.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "CFG.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "ABI.h"
#include "slicing.h"
#include "SymEval.h"

#include "StackAccess.h"

using namespace std;
using namespace Dyninst;

std::string StackAccess::printStackAccessType(StackAccess::StackAccessType t)
{
    switch(t) {
        case StackAccess::StackAccessType::READ:
            return "READ";
        case StackAccess::StackAccessType::WRITE:
            return "WRITE";
        case StackAccess::StackAccessType::SAVED:
            return "SAVED";
        case StackAccess::StackAccessType::READWRITE:
            return "READWRITE";
        case StackAccess::StackAccessType::REGHEIGHT:
            return "REGHEIGHT";
        case StackAccess::StackAccessType::DEBUGINFO_LOCAL:
            return "DEBUGINFO_LOCAL";
        case StackAccess::StackAccessType::DEBUGINFO_PARAM:
            return "DEBUGINFO_PARAM";
        case StackAccess::StackAccessType::UNKNOWN:
            return "UNKNOWN";
        case StackAccess::StackAccessType::MISUNDERSTOOD:
            return "MISUNDERSTOOD";
        default:
            return "NOT RECOGNIZED ACCESS TYPE";
    }
}

std::string StackAccess::format()
{
    std::stringstream ret;
    ret << "Access to " << _readHeight.height()
        << " from " << _reg.name()
        << " (at " << _regHeight.height()
        << ", defined: " << _regDef.format() << ")"
        << ", insn disp = " << _disp
        << ", is " << printStackAccessType(_type);
    return ret.str();
}

bool isDebugType(StackAccess::StackAccessType t)
{
    return (t==StackAccess::StackAccessType::DEBUGINFO_LOCAL ||
            t==StackAccess::StackAccessType::DEBUGINFO_PARAM);
}

int getAccessSize(InstructionAPI::Instruction insn)
{
    std::vector<InstructionAPI::Operand> operands;
    insn.getOperands(operands);
    int accessSize = 0;
    for (unsigned i = 0; i < operands.size(); i++) {
        InstructionAPI::Expression::Ptr value = operands[i].getValue();

        if (accessSize == 0) {
            accessSize = value->size();
        } else {
            accessSize = min(accessSize, value->size());
        }
    }

    return accessSize;
}

// FreeBSD is missing a MINLONG and MAXLONG
#if defined(os_freebsd)
#if defined(arch_64bit)
#define MINLONG INT64_MIN
#define MAXLONG INT64_MAX
#else
#define MINLONG INT32_MIN
#define MAXLONG INT32_MAX
#endif
#endif
class detectToppedLoc : public InstructionAPI::Visitor {
private:
    typedef std::vector<std::pair<Absloc, StackAnalysis::DefHeightSet> >
        DefHeights;
    bool defined;
    bool containsToppedReg;
    DefHeights defHeights;

    std::deque<long> results;  // Stack for calculations

    // Values used for calculations in results
    static const long top = MAXLONG;
    static const long bottom = MINLONG;
    static const long determinable = 0;

public:
    detectToppedLoc(DefHeights &h) : defined(true), containsToppedReg(false),
        defHeights(h) {}

    bool isDefined() {
        return defined && results.size() == 1;
    }

    bool isBottomed() {
        assert(isDefined());
        return containsToppedReg && results.back() == bottom;
    }

    bool isTopped() {
        assert(isDefined());
        return containsToppedReg && results.back() != bottom;
    }

    virtual void visit(InstructionAPI::BinaryFunction *bf) {
        if (!defined) return;

        long arg1 = results.back();
        results.pop_back();
        long arg2 = results.back();
        results.pop_back();

        if (bf->isAdd()) {
            if (arg1 == bottom || arg2 == bottom) {
                results.push_back((long) bottom);
            } else {
                results.push_back((long) top);
            }
        } else if (bf->isMultiply()) {
            if (arg1 == bottom) {
                if (arg2 != top && arg2 != bottom && arg2 != 1) {
                    results.push_back((long) top);
                } else {
                    results.push_back((long) bottom);
                }
            } else if (arg2 == bottom) {
                if (arg1 != top && arg1 != bottom && arg1 != 1) {
                    results.push_back((long) top);
                } else {
                    results.push_back((long) bottom);
                }
            } else {
                results.push_back((long) top);
            }
        } else {
            defined = false;
        }
    }

    virtual void visit(InstructionAPI::Immediate *imm) {
        if (!defined) return;

        results.push_back(imm->eval().convert<long>());
    }

    virtual void visit(InstructionAPI::RegisterAST *r) {
        if (!defined) return;

        MachRegister reg = r->getID();
        if (reg == x86::eip || reg == x86_64::eip || reg == x86_64::rip) {
            results.push_back((long) determinable);
            return;
        }

        Absloc regLoc(reg);
        for (auto iter = defHeights.begin(); iter != defHeights.end(); iter++) {
            if (regLoc == iter->first) {
                if (iter->second.isTopSet()) {
                    containsToppedReg = true;
                    results.push_back((long) top);
                } else {
                    results.push_back((long) bottom);
                }
                return;
            }
        }
        // If reg isn't in defHeights, its height is TOP
        containsToppedReg = true;
        results.push_back((long) top);
    }

    virtual void visit(InstructionAPI::Dereference *) {
        defined = false;
    }
};


bool defsSameHeights(const StackAnalysis::DefHeightSet &dhSet) {
    if (dhSet.size() == 0) return true;
    const StackAnalysis::Height &h = dhSet.begin()->height;
    for (auto iter = dhSet.begin(); iter != dhSet.end(); iter++) {
        if (h != iter->height) return false;
    }
    return true;
}


bool getAccesses(ParseAPI::Function *func,
                 ParseAPI::Block *block,
                 Address addr,
                 InstructionAPI::Instruction insn,
                 Accesses *accesses,
                 std::set<Address> &defPointsToMod,
                 bool analyzeDefinition)
{
    stackmods_printf("\t\t getAccesses %s, 0x%lx @ 0x%lx: %s\n",
        func->name().c_str(), block->start(), addr, insn.format().c_str());

    Architecture arch = insn.getArch();
    std::set<InstructionAPI::RegisterAST::Ptr> readRegs;
    insn.getReadSet(readRegs);
    StackAnalysis sa(func);
    std::vector<std::pair<Absloc, StackAnalysis::Height> > heights;
    std::vector<std::pair<Absloc, StackAnalysis::DefHeightSet> > defHeights;
    sa.findDefinedHeights(block, addr, heights);
    sa.findDefHeightPairs(block, addr, defHeights);

    if (insn.getOperation().getID() == e_ret_far ||
        insn.getOperation().getID() == e_ret_near) {
        return true;
    }

    unsigned int gpr;
    if (arch == Arch_x86) {
        gpr = x86::GPR;
    } else if (arch == Arch_x86_64) {
        gpr = x86_64::GPR;
    } else {
        assert(0);
    }

    std::set<long> extraStackOffsetsToCheck;

    int word_size = func->isrc()->getAddressWidth();

    // If this instruction is a call, check if any stack pointers are possibly
    // being passed as parameters.  If so, we don't know what the callee will
    // access through that pointer and need to return false.
    if (insn.getCategory() == InstructionAPI::c_CallInsn) {
        // Check parameter registers for stack pointers
        ABI *abi = ABI::getABI(word_size);
        const bitArray &callParamRegs = abi->getParameterRegisters();
        for (auto iter = abi->getIndexMap()->begin();
            iter != abi->getIndexMap()->end(); iter++) {
            const MachRegister &reg = iter->first;
            if (reg.regClass() == gpr && callParamRegs.test(iter->second)) {
                // This register is used as a parameter. Check if it contains a
                // stack pointer.
                const StackAnalysis::DefHeightSet &dhSet =
                    sa.findDefHeight(block, addr, Absloc(reg));
                if (!dhSet.isTopSet()) {
                    for (auto dhIter = dhSet.begin(); dhIter != dhSet.end();
                        dhIter++) {
                        const StackAnalysis::Definition &def = dhIter->def;
                        const StackAnalysis::Height &h = dhIter->height;
                        if (def.addr != 0) {
                            defPointsToMod.insert(def.addr);
                            if (!h.isTop() && !h.isBottom()) {
                                // Check another level of indirection.
                                // If the location this pointer points to also
                                // contains a stack height, we need to modify
                                // its definition as well.
                                extraStackOffsetsToCheck.insert(h.height());
                            }
                        } else {
                            return false;
                        }
                    }
                }
            }
        }

        // Check parameters passed on stack for stack pointers
        const StackAnalysis::Height &sp = sa.findSP(block, addr);
        if (!sp.isTop() && !sp.isBottom()) {
            // Check most recent words on stack for stack pointers.  We check
            // last 7 words as a reasonable medium between conservatism and
            // liberalism.
            long lb = sp.height();
            long ub = sp.height() + word_size * 7;
            for (auto iter = heights.begin(); iter != heights.end();
                iter++) {
                const Absloc &loc = iter->first;
                const StackAnalysis::Height &h = iter->second;

                if (loc.type() != Absloc::Stack) continue;
                long stackOff = loc.off();

                if ((extraStackOffsetsToCheck.find(stackOff) !=
                    extraStackOffsetsToCheck.end() ||
                    (stackOff < ub && stackOff >= lb))
                    && !h.isTop()) {
                    const StackAnalysis::DefHeightSet &dhSet =
                        sa.findDefHeight(block, addr, loc);
                    for (auto dhIter = dhSet.begin(); dhIter != dhSet.end();
                        dhIter++) {
                        const StackAnalysis::Definition &def = dhIter->def;
                        if (def.addr != 0) {
                            defPointsToMod.insert(def.addr);
                        } else {
                            return false;
                        }
                    }
                }
            }
        } else {
            // Check all stack locations for stack pointers since we don't know
            // where RSP is pointing on the stack.
            for (auto iter = heights.begin(); iter != heights.end(); iter++) {
                const Absloc &loc = iter->first;
                const StackAnalysis::Height &h = iter->second;
                if (loc.type() == Absloc::Stack && !h.isTop()) {
                    const StackAnalysis::DefHeightSet &dhSet =
                        sa.findDefHeight(block, addr, loc);
                    for (auto dhIter = dhSet.begin(); dhIter != dhSet.end();
                        dhIter++) {
                        const StackAnalysis::Definition &def = dhIter->def;
                        if (def.addr != 0) {
                            defPointsToMod.insert(def.addr);
                        } else {
                            return false;
                        }
                    }
                }
            }
        }
    }

    for (auto iter = defHeights.begin(); iter != defHeights.end(); ++iter) {
        // Only consider registers, not tracked memory locations
        if (iter->first.type() != Absloc::Register) continue;
        const MachRegister &curReg = iter->first.reg();

        // Skip the PC
        if (curReg == MachRegister::getPC(arch)) {
            continue;
        }

        // Skip flags
        if (curReg.regClass() != gpr) {
            continue;
        }

        const StackAnalysis::DefHeightSet &dhSet = iter->second;
        for (auto dhIter = dhSet.begin(); dhIter != dhSet.end(); dhIter++) {
            const StackAnalysis::Definition &curDef = dhIter->def;
            const StackAnalysis::Height &curHeight = dhIter->height;
            StackAccess* access = NULL;
            if (getMemoryOffset(func,
                        block,
                        insn,
                        addr,
                        curReg,
                        curHeight,
                        curDef,
                        access,
                        arch,
                        analyzeDefinition)) {
                if (curHeight.isBottom()) {
                    stackmods_printf("\t\t\t\t INVALID: Found access based on "
                        "register we don't understand (%s = %s)\n",
                        curReg.name().c_str(), curHeight.format().c_str());
                    // Once we've found a bad access, stop looking!
                    return false;
                } else {
                    if (access->readHeight().height() > 20480) {
                        stackmods_printf("\t\t\t\t Found bogus %s. Skipping.\n",
                            access->format().c_str());
                        continue;
                    }
                    (*accesses)[curReg].insert(access);
//                accesses->insert(make_pair(curReg, access));
                }
            }
        }

        // If there are multiple possible heights that can be accessed, verify
        // that we have definitions for all of them.  Also add the definitions
        // to defsToMod.
        if (accesses->find(curReg) != accesses->end() &&
            (*accesses)[curReg].size() > 1) {
            stackmods_printf("\t\t\t\t Multiple access locs possible!\n");
            if (!defsSameHeights(dhSet)) {
                for (auto dhIter = dhSet.begin(); dhIter != dhSet.end();
                    dhIter++) {
                    const StackAnalysis::Definition &d = dhIter->def;
                    if (d.type != StackAnalysis::Definition::DEF ||
                        d.addr == 0) {
                        stackmods_printf("\t\t\t\t INVALID: Multiple accesses "
                            "possible; missing corresponding definition(s)\n");
                        return false;
                    }
                    defPointsToMod.insert(d.addr);
                }
            } else {
                stackmods_printf("\t\t\t\t All definitions have same height\n");
            }
        }
    }

    // Fail if any stack heights are being written out to topped locations.
    // Since StackAnalysis tops loads from unresolved locations, we have to
    // fail if we write out stack heights to unresolved locations. We also fail
    // if an accessed location has a stack height base and an unknown offset.
    if (insn.writesMemory()) {
        std::set<InstructionAPI::Expression::Ptr> writeOperands;
        insn.getMemoryWriteOperands(writeOperands);
        assert(writeOperands.size() == 1);

        detectToppedLoc dtl(defHeights);
        (*writeOperands.begin())->apply(&dtl);
        if (dtl.isTopped()) {
            // We are writing to a topped location.
            // Check if any of the registers involved in the write contain
            // stack heights.
            for (auto regIter = readRegs.begin(); regIter != readRegs.end();
                regIter++) {
                Absloc regLoc((*regIter)->getID());

                for (auto hIter = defHeights.begin(); hIter != defHeights.end();
                    hIter++) {
                    if (hIter->first == regLoc &&
                        !hIter->second.isTopSet()) {
                        stackmods_printf("\t\t\t\tINVALID: Writing stack "
                            "height to topped location\n");
                        return false;
                    }
                }
            }
        } else if (dtl.isBottomed()) {
            stackmods_printf("\t\t\t\tINVALID: Writing to unknown stack "
                "location\n");
            return false;
        }
    } else if (insn.readsMemory()) {
        std::set<InstructionAPI::Expression::Ptr> readOperands;
        insn.getMemoryReadOperands(readOperands);
        for (auto rIter = readOperands.begin(); rIter != readOperands.end();
            rIter++) {
            detectToppedLoc dtl(defHeights);
            (*rIter)->apply(&dtl);
            if (dtl.isBottomed()) {
                stackmods_printf("\t\t\t\tINVALID: Reading unknown stack "
                    "location\n");
                return false;
            }
        }
    }

    // If we are analyzing a definition and aren't able to find the exact
    // height of the definition, we won't be able to do any stack modifications.
    if (analyzeDefinition && (accesses->size() != 1 ||
        accesses->begin()->second.size() != 1)) {
        stackmods_printf("\t\t\t\tINVALID: Unable to determine height of "
            "definition\n");
        return false;
    }

    return true;
}

using namespace InstructionAPI;

// Copied and modified from parseAPI/src/IA_x86Details.C
class zeroAllGPRegisters : public InstructionAPI::Visitor
{
    public:
        zeroAllGPRegisters(Address ip, ParseAPI::Function* f, ParseAPI::Block* b, InstructionAPI::Instruction i, bool z = false) :
            defined(true), m_ip(ip), func(f), block(b), insn(i), zero(z) {
                if (func) {
                    StackAnalysis tmp(func);
                    sa = tmp;
                }
            }

        virtual ~zeroAllGPRegisters() {}
        bool defined;
        std::deque<long> results;
        Address m_ip;
        ParseAPI::Function* func;
        ParseAPI::Block* block;
        InstructionAPI::Instruction insn;
        StackAnalysis sa;
        bool zero;
        long getResult() {
            if(results.empty()) return 0;
            return results.front();
        }
        bool isDefined() {
            return defined && (results.size() == 1);
        }
        virtual void visit(BinaryFunction* b)
        {
            if(!defined) return;
            long arg1 = results.back();
            results.pop_back();
            long arg2 = results.back();
            results.pop_back();
            if(b->isAdd())
            {
                results.push_back(arg1+arg2);
            }
            else if(b->isMultiply())
            {
                results.push_back(arg1*arg2);
            }
            else
            {
                defined = false;
            }
        }
        virtual void visit(Immediate* i)
        {
            if(!defined) return;
            results.push_back(i->eval().convert<long>());
        }
        virtual void visit(RegisterAST* r)
        {
            if(!defined) return;
            if(r->getID() == x86::eip ||
               r->getID() == x86_64::eip ||
               r->getID() == x86_64::rip)
            {
                results.push_back(m_ip);
                return;
            }

            if (!zero) {
                if (func) {
                    StackAnalysis::Height height = sa.find(block, m_ip,
                        Absloc(r->getID()));
                    if (!height.isBottom() && !height.isTop()) {
                        stackmods_printf("\t\t\t found %s height = %ld\n",
                            r->getID().name().c_str(), height.height());
                        results.push_back(height.height());
                        return;
                    }
                }
            }

            results.push_back(0);
        }
        virtual void visit(Dereference* )
        {
            defined = false;
        }

};

bool getMemoryOffset(ParseAPI::Function *func,
                     ParseAPI::Block *block,
                     Instruction insn,
                     Address addr,
                     const MachRegister &reg,
                     const StackAnalysis::Height &height,
                     const StackAnalysis::Definition &def,
                     StackAccess *&ret,
                     Architecture arch,
                     bool analyzeDefinition)
{
    stackmods_printf("\t\t\t getMemoryOffset for %s; checking reg %s = %s\n",
        insn.format().c_str(), reg.name().c_str(), height.format().c_str());

    InstructionAPI::RegisterAST* regAST = new InstructionAPI::RegisterAST(reg);
    InstructionAPI::RegisterAST::Ptr regASTPtr = InstructionAPI::RegisterAST::Ptr(regAST);

    std::vector<InstructionAPI::Operand> operands;
    insn.getOperands(operands);

    signed long disp = 0;  // Stack height of access
    signed long offset = 0;  // Offset from the base register used in the access
    bool isOffsetSet = false;

    // Determine how memory is accessed
    StackAccess::StackAccessType type = StackAccess::StackAccessType::UNKNOWN;
    if (analyzeDefinition) {
        type = StackAccess::StackAccessType::DEFINITION;
    } else if (insn.readsMemory() && insn.writesMemory()) {
        type = StackAccess::StackAccessType::READWRITE;
    } else if (insn.readsMemory()) {
        type = StackAccess::StackAccessType::READ;
    } else if (insn.writesMemory()) {
        type = StackAccess::StackAccessType::WRITE;
    }

    // If memory is not accessed, no need to find an offset
    if (type == StackAccess::StackAccessType::UNKNOWN) {
        return false;
    }

    for (unsigned i = 0; i < operands.size(); i++) {
        stackmods_printf("\t\t\t\t operand[%u] = %s\n", i,
            operands[i].getValue()->format(insn.getArch()).c_str());

        // Set match if reg is read or written
        bool match = false;
        if (reg.isValid()) {
            std::set<InstructionAPI::RegisterAST::Ptr> regsRead;
            operands[i].getReadSet(regsRead);
            for (auto iter = regsRead.begin(); iter != regsRead.end(); ++iter) {
                InstructionAPI::RegisterAST::Ptr cur = *iter;
                if (*cur == *regASTPtr) {
                    stackmods_printf("\t\t\t\t\t reads reg\n");
                    match = true;
                }
            }
            std::set<InstructionAPI::RegisterAST::Ptr> regsWrite;
            operands[i].getWriteSet(regsWrite);
            for (auto iter = regsWrite.begin(); iter != regsWrite.end();
                ++iter) {
                InstructionAPI::RegisterAST::Ptr cur = *iter;
                if (*cur == *regASTPtr) {
                    stackmods_printf("\t\t\t\t\t writes reg\n");
                    match = true;
                }
            }
        }

        // If the passed-in register is read/written...
        if (match || !reg.isValid()) {
            InstructionAPI::Expression::Ptr val = operands[i].getValue();
            if (!val) break;

            // Won't find an offset for an immediate
            if (dynamic_cast<InstructionAPI::Immediate*>(val.get())) {
                continue;
            }

            // Won't find an offset for a registerAST.  However, we do want to
            // record a push (e.g., callee-saved registers).
            if (dynamic_cast<InstructionAPI::RegisterAST*>(val.get()) &&
                insn.getOperation().getID() != e_push) {
                continue;
            }

            // If we have a dereference, extract the child
            if (dynamic_cast<InstructionAPI::Dereference*>(val.get())) {
                vector<InstructionAPI::InstructionAST::Ptr> children;
                val->getChildren(children);
                if (children.size() == 1) {
                    InstructionAPI::InstructionAST::Ptr child =
                        children.front();
                    val = boost::dynamic_pointer_cast<InstructionAPI::
                        Expression>(child);
                }
            }

            // Copied from parseAPI/src/IA_x86Details.C
            // IA_x86Details::findTableInsn()
            zeroAllGPRegisters z(addr, func, block, insn, false);
            val->apply(&z);
            if (z.isDefined()) {
                // At this point, z.getResult() contains the exact stack height
                // being accessed.
                zeroAllGPRegisters z2(addr, func, block, insn, true);
                val->apply(&z2);

                isOffsetSet = true;
                offset = z.getResult();

                if (z2.isDefined()) {
                    // At this point, z2.getResult() contains the difference
                    // between the stack height being accessed and the height of
                    // the base register used in the address calculation.
                    disp = z2.getResult();
                } else {
                    disp = 0;
                }

                stackmods_printf("\t\t\t\t found offset %ld, disp = %ld, "
                    "type = %d\n", offset, disp, static_cast<int>(type));
            }
        }
    }

    std::stringstream msg;
    if (isOffsetSet) {
        ret = new StackAccess();
        ret->setRegHeight(height);
        ret->setRegDef(def);
        ret->setReg(reg);
        ret->setType(type);
        ret->setDisp(disp);

        if (ret->disp() != 0 && arch == Arch_x86) {
            // Fix the signed issue
            signed long fixedOffset;
            Offset MAX = 0xffffffff;
            if (disp > (long)MAX/2) {
                fixedOffset = -1*(MAX - disp + 1);
                disp = fixedOffset;
                ret->setDisp(disp);
            }
        }

        ret->setReadHeight(StackAnalysis::Height(offset));

        // Stackanalysis is reporting the heights at the start of the
        // instruction, not the end; for push, we want to record the final
        // state, which is where the value is actually stored
        if (insn.getOperation().getID() == e_push) {
            long width;
            if (arch == Arch_x86) width = 4;
            else width = 8;
            ret->setRegHeight(ret->regHeight() - width);
            ret->setReadHeight(ret->readHeight() - width);
            ret->setType(StackAccess::StackAccessType::SAVED);
        }
    }

    return isOffsetSet;
}

