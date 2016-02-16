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
#include "InstructionDecoder.h"
#include "Expression.h"
#include "Register.h"
#include "Result.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "CFG.h"

#include "slicing.h"
#include "SymEval.h"

#include "StackAccess.h"

using namespace Dyninst;

std::string StackAccess::printStackAccessType(StackAccess::StackAccessType t)
{
    switch(t) {
        case(StackAccess::READ):
            return "READ";
        case(StackAccess::WRITE):
            return "WRITE";
        case(StackAccess::SAVED):
            return "SAVED";
        case(StackAccess::READWRITE):
            return "READWRITE";
        case(StackAccess::REGHEIGHT):
            return "REGHEIGHT";
        case(StackAccess::DEBUGINFO_LOCAL):
            return "DEBUGINFO_LOCAL";
        case(StackAccess::DEBUGINFO_PARAM):
            return "DEBUGINFO_PARAM";
        case(StackAccess::UNKNOWN):
            return "UNKNOWN";
        case (StackAccess::MISUNDERSTOOD):
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
        << " (at " << _regHeight.height() << ")"
        << ", insn disp = " << _disp
        << ", is " << printStackAccessType(_type);
    return ret.str();
}

bool isDebugType(StackAccess::StackAccessType t)
{
    return (t==StackAccess::DEBUGINFO_LOCAL ||
            t==StackAccess::DEBUGINFO_PARAM);
}

int getAccessSize(InstructionAPI::Instruction::Ptr insn)
{
    std::vector<InstructionAPI::Operand> operands;
    insn->getOperands(operands);
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

class detectToppedLoc : public InstructionAPI::Visitor {
private:
    typedef std::vector<std::pair<Absloc, StackAnalysis::Height>> Heights;
    bool topped;
    Heights heights;

public:
    detectToppedLoc(Heights &h) : topped(false), heights(h) {}

    bool isTopped() {
        return topped;
    }

    virtual void visit(InstructionAPI::BinaryFunction *) {}
    virtual void visit(InstructionAPI::Immediate *) {}
    virtual void visit(InstructionAPI::RegisterAST *r) {
        if (topped) return;

        MachRegister reg = r->getID();
        if (reg == x86::eip || reg == x86_64::eip || reg == x86_64::rip) {
            return;
        }

        Absloc regLoc(reg);
        for (auto iter = heights.begin(); iter != heights.end(); iter++) {
            if (regLoc == iter->first) {
                if (iter->second.isTop()) topped = true;
                return;
            }
        }
        topped = true;  // If reg isn't in heights, its height is TOP
    }
    virtual void visit(InstructionAPI::Dereference *) {}
};

bool getAccesses(ParseAPI::Function* func,
        ParseAPI::Block* block,
        Address addr,
        InstructionAPI::Instruction::Ptr insn,
        Accesses*& accesses)
{
    bool ret = true;

    stackmods_printf("\t\t getAccesses %s, 0x%lx @ 0x%lx: %s\n",
        func->name().c_str(), block->start(), addr, insn->format().c_str());

    Architecture arch = insn->getArch();
    std::set<InstructionAPI::RegisterAST::Ptr> readRegs;
    insn->getReadSet(readRegs);
    StackAnalysis sa(func);
    std::vector<std::pair<Absloc, StackAnalysis::Height> > heights;
    sa.findDefinedHeights(block, addr, heights);

    if (insn->getOperation().getID() == e_ret_far ||
        insn->getOperation().getID() == e_ret_near) {
        return true;
    }

    for (auto iter = heights.begin(); iter != heights.end(); ++iter) {
        // Only consider registers, not tracked memory locations
        if (iter->first.type() != Absloc::Register) continue;
        MachRegister curReg = iter->first.reg();

        unsigned int gpr;
        if (arch == Arch_x86) {
            gpr = x86::GPR;
        } else if (arch == Arch_x86_64) {
            gpr = x86_64::GPR;
        } else {
            assert(0);
        }

        // Skip the PC
        if (curReg == MachRegister::getPC(arch)) {
            continue;
        }

        // Skip flags
        if (curReg.regClass() != gpr) {
            continue;
        }

        StackAnalysis::Height curHeight = (*iter).second;
        StackAccess* access = NULL;
        if (getMemoryOffset(func,
                    block,
                    insn,
                    addr,
                    curReg,
                    curHeight,
                    access,
                    arch)) {
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
                accesses->insert(make_pair(curReg, access));
            }
        }
    }

    // Fail if any stack heights are being written out to topped locations.
    // Since StackAnalysis tops loads from unresolved locations, we have to
    // fail if we write out stack heights to unresolved locations.
    if (insn->writesMemory()) {
        std::set<InstructionAPI::Expression::Ptr> writeOperands;
        insn->getMemoryWriteOperands(writeOperands);
        assert(writeOperands.size() == 1);

        detectToppedLoc dtl(heights);
        (*writeOperands.begin())->apply(&dtl);
        if (dtl.isTopped()) {
            // We are writing to a topped location.
            // Check if any of the registers involved in the write contain
            // stack heights.
            for (auto regIter = readRegs.begin(); regIter != readRegs.end();
                regIter++) {
                Absloc regLoc((*regIter)->getID());

                for (auto hIter = heights.begin(); hIter != heights.end();
                    hIter++) {
                    if (hIter->first == regLoc && !hIter->second.isTop()) {
                        stackmods_printf("\t\t\t\tINVALID: Writing stack "
                            "height to topped location\n");
                        return false;
                    }
                }
            }
        }
    }


    return ret;
}

using namespace InstructionAPI;

// Copied and modified from parseAPI/src/IA_x86Details.C
class zeroAllGPRegisters : public InstructionAPI::Visitor
{
    public:
        zeroAllGPRegisters(Address ip, ParseAPI::Function* f, ParseAPI::Block* b, InstructionAPI::Instruction::Ptr i, bool z = false) :
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
        InstructionAPI::Instruction::Ptr insn;
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
                    StackAnalysis::Height height = sa.find(block, m_ip, r->getID());
                    if (!height.isBottom() && !height.isTop()) {
                        stackmods_printf("\t\t\t found %s height = %ld\n", r->getID().name().c_str(), height.height());
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

bool getMemoryOffset(ParseAPI::Function* func,
        ParseAPI::Block* block,
        InstructionAPI::InstructionPtr insn,
        Address addr,
        MachRegister reg,
        StackAnalysis::Height height,
        StackAccess*& ret,
        Architecture arch)
{

    stackmods_printf("\t\t\t getMemoryOffset for %s; checking reg %s = %s\n", insn->format().c_str(), reg.name().c_str(), height.format().c_str());

    bool skipReg = false;
    bool multChildrenInDeref = false;

    InstructionAPI::RegisterAST* regAST = new InstructionAPI::RegisterAST(reg);
    InstructionAPI::RegisterAST::Ptr regASTPtr = InstructionAPI::RegisterAST::Ptr(regAST);

    std::vector<InstructionAPI::Operand> operands;
    insn->getOperands(operands);

    signed long disp = 0;
    signed long offset = 0;
    bool isOffsetSet = false;

    // Determine how memory is accessed
    StackAccess::StackAccessType type = StackAccess::UNKNOWN;
    if (insn->readsMemory() && insn->writesMemory()) {
        type = StackAccess::READWRITE;
    } else if (insn->readsMemory()) {
        type = StackAccess::READ;
    } else if (insn->writesMemory()) {
        type = StackAccess::WRITE;
    }

    // If memory is not accessed, no need to find an offset
    if (type == StackAccess::UNKNOWN) {
        return false;
    }

    for (unsigned i = 0; i < operands.size(); i++) {

        stackmods_printf("\t\t\t\t operand[%d] = %s\n", i, operands[i].getValue()->format().c_str());

        // Get the read/written register sets
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
            for (auto iter = regsWrite.begin(); iter != regsWrite.end(); ++iter) {
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
            if (typeid(*val) == typeid(InstructionAPI::Immediate)) {
                continue;
            }

            // Won't find an offset for a registerAST
            // However, we do want to record a push (e.g., callee-saved registers)
            if (!(insn->getOperation().getID() == e_push)) {
                if (typeid(*val) == typeid(InstructionAPI::RegisterAST)) {
                    continue;
                }
            }

            // If we have a dereference, extract the child
            if (typeid(*val) == typeid(InstructionAPI::Dereference)) {
                vector<InstructionAPI::InstructionAST::Ptr> children;
                val->getChildren(children);
                if (children.size() == 1) {
                    InstructionAPI::InstructionAST::Ptr child = children.front();
                    val = boost::dynamic_pointer_cast<InstructionAPI::Expression>(child);
                }
            }

            // Copied from parseAPI/src/IA_x86Details.C IA_x86Details::findTableInsn()
            zeroAllGPRegisters z(addr, func, block, insn, false);
            val->apply(&z);
            if (z.isDefined()) {
                zeroAllGPRegisters z2(addr, func, block, insn, true);
                val->apply(&z2);

                isOffsetSet = true;
                offset = z.getResult();

                if ( (z2.isDefined()) &&
                     ( (z2.getResult()) ||
                       (z.getResult() == z2.getResult()))) {
                    disp = z2.getResult();
                } else {
                    disp = 0;
                }

                stackmods_printf("\t\t\t\t found offset %ld, disp = %ld, type = %d\n", offset, disp, type);

                // Weird check: is there more than one child in the dereference? If not, we may want to skip
                vector<InstructionAPI::InstructionAST::Ptr> children2;
                val->getChildren(children2);
                if (children2.size() > 1) {
                    stackmods_printf("\t\t\t\t\t Found >1 child in dereference.\n");
                    multChildrenInDeref = true;
                }
            }
        }
    }

    std::stringstream msg;
    if (isOffsetSet) {

        // Question: Where did this stack height come from?
        if (reg.isValid() &&
                /*!multChildrenInDeref && */
                (reg != MachRegister::getStackPointer(arch))) {


            bool performSlice = true;
            if (multChildrenInDeref && (disp==0)) {
                // We will never mark this as a skipReg, so avoid slicing
                performSlice = false;
            }

            if (multChildrenInDeref && (type==StackAccess::UNKNOWN)) {
                // We will never mark this as a skipReg, so avoid slicing
                performSlice = false;
            }

            if ((insn->getOperation().getID() == e_push)) {
                performSlice = false;
            }

            if (height.isBottom()) {
                performSlice = false;
            }

            if (performSlice) {
                std::vector<AbsRegion> ins;
                ins.push_back(AbsRegion(Absloc(reg)));
                Assignment::Ptr assign = Assignment::Ptr(new Assignment(insn,
                            addr,
                            func,
                            block,
                            ins,
                            AbsRegion(Absloc(reg))));
                Slicer slicer(assign, block, func);
                Slicer::Predicates defaultPredicates;
                stackmods_printf("\t\t\t\t Using slicing @ 0x%lx for %s\n", addr, reg.name().c_str());
                Graph::Ptr graph = slicer.backwardSlice(defaultPredicates);

                stackmods_printf("\t\t\t\t\t Slicing complete, graph size = %d\n", graph->size());

                // Graph must have single entry
                NodeIterator exitBegin, exitEnd;
                graph->exitNodes(exitBegin, exitEnd);

                if (*exitBegin == NULL) {
                    assert(0 && "Could not find exit node for backward slice");
                }

                std::set<Address> visited;

                // Graph must have straight-line path
                NodeIterator inBegin, inEnd;
                (*exitBegin)->ins(inBegin, inEnd);
                NodeIterator prev = inBegin;
                while ((*prev)->hasInEdges()) {
                    NodeIterator tmp = inBegin;

                    if (tmp != inEnd) {
                        stackmods_printf("\t\t\t\t\t Next level\n");
                        stackmods_printf("\t\t\t\t\t\t tmp: %s\n", (*tmp)->format().c_str());
                    }

                    // Check for cycles
                    if (visited.find((*tmp)->addr()) != visited.end()) {
                        skipReg = true;
                        break;
                    }
                    visited.insert((*tmp)->addr());

                    while (tmp != inEnd) {
                        NodeIterator next = tmp;
                        next++;

                        if (next == inEnd) {
                            break;
                        }
                        stackmods_printf("\t\t\t\t\t\t next: %s\n", (*next)->format().c_str());
                        if ((*next)->DOTname().compare("<NULL>") == 0) {
                            tmp++;
                            continue;
                        }

                        // Check for cycles
                        if (visited.find((*next)->addr()) != visited.end()) {
                            skipReg = true;
                            break;
                        }
                        visited.insert((*next)->addr());

                        if ((*tmp)->addr() != (*next)->addr()) {
                            skipReg = true;
                            break;
                        }
                        tmp++;
                    }

                    if (skipReg) break;

                    if (!(*inBegin)->hasInEdges()) {
                        break;
                    }
                    prev = inBegin;
                    (*inBegin)->ins(inBegin, inEnd);
                }

                stackmods_printf("\t\t\t\t Finished using backward slice to check for skipReg\n");

            }
        }

        if (skipReg && multChildrenInDeref && disp==0) {
            stackmods_printf("\t\t\t\t\t\t unset skipReg (multchildren && disp==0)\n");
            skipReg = false;
        }

        if (skipReg && multChildrenInDeref && type==StackAccess::UNKNOWN) {
            stackmods_printf("\t\t\t\t\t\t unset skipReg (multchildren && StackAccess::UNKNOWN\n");
            skipReg = false;
        }

        ret = new StackAccess();
        ret->setRegHeight(height);
        ret->setReg(reg);
        ret->setType(type);
        ret->setDisp(disp);
        ret->setSkipReg(skipReg);

        if (ret->disp() && arch == Arch_x86) {
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
        if (insn->getOperation().getID() == e_push) {
            long width;
            if (arch == Arch_x86) width = 4;
            else width = 8;
            ret->setRegHeight(ret->regHeight() - width);
            ret->setReadHeight(ret->readHeight() - width);
            ret->setType(StackAccess::SAVED);
        }
    }

    return isOffsetSet;
}

