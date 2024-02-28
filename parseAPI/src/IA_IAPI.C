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

#include "common/src/vgannotations.h"
#include "dyntypes.h"
#include "registers/x86_regs.h"
#include "IA_IAPI.h"
#include "util.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"
#include "IndirectAnalyzer.h"
#include "util.h"
#include "dyntypes.h"
#include "instructionAPI/h/syscalls.h"
#include "instructionAPI/h/interrupts.h"

#include <deque>
#include <map>

#include "Register.h"
#include "IA_x86.h"
#include "IA_power.h"
#include "IA_aarch64.h"
#include "IA_amdgpu.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

std::map<Architecture, RegisterAST::Ptr> IA_IAPI::framePtr;
std::map<Architecture, RegisterAST::Ptr> IA_IAPI::stackPtr;
std::map<Architecture, RegisterAST::Ptr> IA_IAPI::thePC;


IA_IAPI::IA_IAPI(const IA_IAPI &rhs) 
    : InstructionAdapter(rhs),
    dec(rhs.dec),
    allInsns(rhs.allInsns),
    validCFT(rhs.validCFT),
    cachedCFT(rhs.cachedCFT),
    validLinkerStubState(rhs.validLinkerStubState),
    cachedLinkerStubState(rhs.cachedLinkerStubState),
    hascftstatus(rhs.hascftstatus),
    tailCalls(rhs.tailCalls) {
        //curInsnIter = allInsns.find(rhs.curInsnIter->first);
        curInsnIter = allInsns.end()-1;
    }

IA_IAPI &IA_IAPI::operator=(const IA_IAPI &rhs) {
    dec = rhs.dec;
    allInsns = rhs.allInsns;
    //curInsnIter = allInsns.find(rhs.curInsnIter->first);
    curInsnIter = allInsns.end()-1;
    validCFT = rhs.validCFT;
    cachedCFT = rhs.cachedCFT;
    validLinkerStubState = rhs.validLinkerStubState;
    cachedLinkerStubState = rhs.cachedLinkerStubState;
    hascftstatus = rhs.hascftstatus;
    tailCalls = rhs.tailCalls;

    // InstructionAdapter members
    current = rhs.current;
    previous = rhs.previous;
    parsedJumpTable = rhs.parsedJumpTable;
    successfullyParsedJumpTable = rhs.successfullyParsedJumpTable;
    isDynamicCall_ = rhs.isDynamicCall_;
    checkedDynamicCall_ = rhs.checkedDynamicCall_;
    isInvalidCallTarget_ = rhs.isInvalidCallTarget_;
    checkedInvalidCallTarget_ = rhs.checkedInvalidCallTarget_;
    _obj = rhs._obj;
    _cr = rhs._cr;
    _isrc = rhs._isrc;
    _curBlk = rhs._curBlk;

    return *this;
}

IA_IAPI* IA_IAPI::makePlatformIA_IAPI(Architecture arch,
        InstructionDecoder dec_, 
        Address where_,
        CodeObject * o,
        CodeRegion * r,
        InstructionSource *isrc,
        Block * curBlk_) {
    switch (arch) {
        case Arch_x86:
        case Arch_x86_64:
            return new IA_x86(dec_, where_, o, r, isrc, curBlk_);	    
        case Arch_ppc32:
        case Arch_ppc64:
            return new IA_power(dec_, where_, o, r, isrc, curBlk_);
        case Arch_aarch64:
            return new IA_aarch64(dec_, where_, o, r, isrc, curBlk_);
        case Arch_amdgpu_gfx908:
        case Arch_amdgpu_gfx90a:
        case Arch_amdgpu_gfx940:

            return new IA_amdgpu(dec_, where_, o, r, isrc, curBlk_);
 
        default:
            assert(!"unimplemented architecture");
    }
    return NULL;
}				      

std::once_flag IA_IAPI::ptrInit;

void IA_IAPI::initASTs()
{
    std::call_once(IA_IAPI::ptrInit, [&]{
            if(framePtr.empty())
            {
                framePtr[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86)));
                framePtr[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_x86_64)));
                framePtr[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_ppc32)));
                framePtr[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_ppc64)));
                framePtr[Arch_aarch64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_aarch64)));
                framePtr[Arch_amdgpu_gfx908] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_amdgpu_gfx908)));
                framePtr[Arch_amdgpu_gfx90a] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_amdgpu_gfx90a)));
                framePtr[Arch_amdgpu_gfx940] = RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(Arch_amdgpu_gfx940)));
            }
            if(stackPtr.empty())
            {
                stackPtr[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86)));
                stackPtr[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_x86_64)));
                stackPtr[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_ppc32)));
                stackPtr[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_ppc64)));
                stackPtr[Arch_aarch64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_aarch64)));
                stackPtr[Arch_amdgpu_gfx908] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_amdgpu_gfx908)));
                stackPtr[Arch_amdgpu_gfx90a] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_amdgpu_gfx90a)));
                stackPtr[Arch_amdgpu_gfx940] = RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(Arch_amdgpu_gfx940)));
            }
            if(thePC.empty())
            {
                thePC[Arch_x86] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_x86)));
                thePC[Arch_x86_64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
                thePC[Arch_ppc32] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
                thePC[Arch_ppc64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_ppc64)));
                thePC[Arch_aarch64] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_aarch64)));
                thePC[Arch_amdgpu_gfx908] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_amdgpu_gfx908)));
                thePC[Arch_amdgpu_gfx90a] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_amdgpu_gfx90a)));
                thePC[Arch_amdgpu_gfx940] = RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(Arch_amdgpu_gfx940)));
            }
            ANNOTATE_HAPPENS_BEFORE(&IA_IAPI::ptrInit);
    });
    ANNOTATE_HAPPENS_AFTER(&IA_IAPI::ptrInit);
}

IA_IAPI::IA_IAPI(InstructionDecoder dec_, 
        Address where_,
        CodeObject * o,
        CodeRegion * r,
        InstructionSource *isrc,
        Block * curBlk_) :
    InstructionAdapter(where_, o, r, isrc, curBlk_), 
    dec(dec_),
    validCFT(false), 
    cachedCFT(std::make_pair(false, 0)),
    validLinkerStubState(false),
    cachedLinkerStubState(false)
{
    hascftstatus.first = false;
    tailCalls.clear();

    //boost::tuples::tie(curInsnIter, boost::tuples::ignore) = allInsns.insert(std::make_pair(current, dec.decode()));
    curInsnIter =
        allInsns.insert(
                allInsns.end(),
                std::make_pair(current, dec.decode()));

    initASTs();
}

    void
IA_IAPI::reset(
        InstructionDecoder dec_,
        Address start,
        CodeObject *o,
        CodeRegion *r,
        InstructionSource *isrc,
        Block * curBlk_)
{
    // reset the base
    InstructionAdapter::reset(start,o,r,isrc,curBlk_);

    dec = dec_;
    validCFT = false;
    cachedCFT = make_pair(false, 0);
    validLinkerStubState = false; 
    hascftstatus.first = false;
    tailCalls.clear();

    allInsns.clear();

    curInsnIter =
        allInsns.insert(
                allInsns.end(),
                std::make_pair(current, dec.decode()));

    initASTs();
}


void IA_IAPI::advance()
{
    //    if(!curInsn()) {
    //        parsing_printf("..... WARNING: failed to advance InstructionAdapter at 0x%lx, allInsns.size() = %d\n", current,
    //                       allInsns.size());
    //        return;
    //    }
    InstructionAdapter::advance();
    current += curInsn().size();

    curInsnIter =
        allInsns.insert(
                allInsns.end(),
                std::make_pair(current, dec.decode()));

    //    if(!curInsn())
    //    {
    //        parsing_printf("......WARNING: after advance at 0x%lx, curInsn() NULL\n", current);
    //    }
    validCFT = false;
    validLinkerStubState = false;
    hascftstatus.first = false;
    tailCalls.clear();
}

bool IA_IAPI::retreat()
{
    //    if(!curInsn()) {
    //        parsing_printf("..... WARNING: failed to retreat InstructionAdapter at 0x%lx, allInsns.size() = %d\n", current,
    //                       allInsns.size());
    //        return false;
    //    }
    InstructionAdapter::retreat();
    allInsns_t::iterator remove = curInsnIter;
    if(curInsnIter != allInsns.begin()) {
        --curInsnIter;
        allInsns.erase(remove);
        current = curInsnIter->first;
        if(curInsnIter != allInsns.begin()) {
            allInsns_t::iterator pit = curInsnIter;
            --pit;
            previous = curInsnIter->first;
        } else {
            previous = -1;
        }
    } else {
        parsing_printf("..... WARNING: cowardly refusal to retreat past first instruction at 0x%lx\n", current);
        return false;
    }

    /* blind duplication -- nate */
    validCFT = false;
    validLinkerStubState = false;
    hascftstatus.first = false;
    tailCalls.clear();
    return true;
} 



size_t IA_IAPI::getSize() const
{
    return curInsn().size();
}

bool IA_IAPI::hasCFT() const
{
    parsing_printf("hasCFT called\n");
    if(hascftstatus.first) {
        parsing_printf("\t Returning cached entry: %d\n",hascftstatus.second);
        return hascftstatus.second;
    }
    InsnCategory c = curInsn().getCategory();
    hascftstatus.second = false;
    if(c == c_BranchInsn ||
            c == c_ReturnInsn) {
        if ( likely ( ! (_obj->defensiveMode() && isNopJump()) ) ) {
            parsing_printf("\t branch or return, ret true\n");
            hascftstatus.second = true;
        }
    }
    else if(c == c_CallInsn) {
        if(isRealCall()) {
            hascftstatus.second = true;
        }
        else if(isDynamicCall()) {
            hascftstatus.second = true;
        }
        else if(simulateJump()) {
            hascftstatus.second = true;
        }
    }
    else if(c == c_SysEnterInsn) 
    {
        hascftstatus.second = true;
    }
    else if (c == c_SyscallInsn)
    {
        hascftstatus.second = true;
    }

    hascftstatus.first = true;
    return hascftstatus.second;
}

bool IA_IAPI::isAbort() const
{
    entryID e = curInsn().getOperation().getID();
    return e == e_int3 ||
        e == e_hlt ||
        e == e_ud2;
}

bool IA_IAPI::isInvalidInsn() const
{
    entryID e = curInsn().getOperation().getID();
    if(e == e_No_Entry)
    {
        parsing_printf("...WARNING: un-decoded instruction at 0x%lx\n", current);
        return true;
    }
    return false;
}

/* This function determines if a given instruction is weird enough that we've
 * probably veered into non-code bytes and are parsing garbage.  
 * note: yes, some of the code in here is does low-level things like 
 *       grab instruction bytes directly instead of relying on the parseAPI,
 *       but since this code executes for every parsed instruction, it needs
 *       to be efficient.  
 */
bool IA_IAPI::isGarbageInsn() const
{
    bool ret = false;
    // GARBAGE PARSING HEURISTIC
    if (unlikely(_obj->defensiveMode())) {
        entryID e = curInsn().getOperation().getID();
        switch (e) {
            case e_arpl:
                cerr << "REACHED AN ARPL AT "<< std::hex << current 
                    << std::dec <<" COUNTING AS INVALID" << endl;
                ret = true;
                break;
            case e_fisub:
                cerr << "REACHED A FISUB AT "<< std::hex << current 
                    << std::dec <<" COUNTING AS INVALID" << endl;
                ret = true;
                break;
            case e_into:
                cerr << "REACHED AN INTO AT "<< std::hex << current 
                    << std::dec <<" COUNTING AS INVALID" << endl;
                ret = true;
                break;
            case e_mov: {
                            set<RegisterAST::Ptr> regs;
                            curInsn().getWriteSet(regs);
                            for (set<RegisterAST::Ptr>::iterator rit = regs.begin();
                                    rit != regs.end(); rit++) 
                            {
                                if (Dyninst::isSegmentRegister((*rit)->getID().regClass())) {
                                    cerr << "REACHED A MOV SEGMENT INSN AT "<< std::hex 
                                        << current << std::dec <<" COUNTING AS INVALID" << endl;
                                    ret = true;
                                    break;
                                }
                            }
                            break;
                        }
            case e_add:
                        if (2 == curInsn().size() &&
                                0 == curInsn().rawByte(0) &&
                                0 == curInsn().rawByte(1))
                        {
                            cerr << "REACHED A 0x0000 INSTRUCTION "<< std::hex << current 
                                << std::dec <<" COUNTING AS INVALID" << endl;
                            ret = true;
                        }
                        break;
            case e_push: // pushes of segment registers do not occur frequently in real code (and crash Rose)
#if 0 // instructionAPI implementation
                        set<RegisterAST::Ptr> regs;
                        curInsn()->getWriteSet(regs);
                        for (set<RegisterAST::Ptr>::iterator rit = regs.begin();
                                rit != regs.end(); rit++) 
                        {
                            if (Dyninst::isSegmentRegister((*rit)->getID().regClass())) {
                                cerr << "REACHED A PUSH OF A SEGMENT REGISTER AT "<< std::hex 
                                    << current << std::dec <<" COUNTING AS INVALID" << endl;
                                ret = true;
                                break;
                            }
                        }
#else // faster raw-byte implementation 
                        switch (curInsn().rawByte(0)) {
                            case 0x06:
                            case 0x0e:
                            case 0x16:
                            case 0x1e:
                                ret = true;
                                cerr << "REACHED A PUSH OF A SEGMENT REGISTER "<< std::hex << current 
                                    << std::dec <<" COUNTING AS INVALID" << endl;
                                break;
                            case 0x0f:
                                if (2 == curInsn().size() &&
                                        ((0xa0 == curInsn().rawByte(1)) || (0xa8 == curInsn().rawByte(1))))
                                {
                                    ret = true;
                                    cerr << "REACHED A 2-BYTE PUSH OF A SEGMENT REGISTER "<< std::hex << current 
                                        << std::dec <<" COUNTING AS INVALID" << endl;
                                }
                                break;
                            default:
                                break;
                        }
#endif
            default:
                        break;
        }
    }
    return ret;
}	
bool IA_IAPI::isFrameSetupInsn() const
{
    return isFrameSetupInsn(curInsn());
}

bool IA_IAPI::isDynamicCall() const
{
    Instruction ci = curInsn();
    if(ci.isValid() && (ci.getCategory() == c_CallInsn))
    {
        Address addr;
        bool success;
        boost::tie(success, addr) = getCFT();
        if (!success) {
            parsing_printf("... Call 0x%lx is indirect\n", current);
            return true;
        }
    }
    return false;
}

bool IA_IAPI::isAbsoluteCall() const
{
    Instruction ci = curInsn();
    if(ci.getCategory() == c_CallInsn)
    {
        Expression::Ptr cft = ci.getControlFlowTarget();
        if(cft && boost::dynamic_pointer_cast<Immediate>(cft))
        {
            return true;
        }
        if (isDynamicCall()) {
            return true; // indirect call targets are absolute 
            // (though unknown for now)
        }
    }
    return false;
}

bool IA_IAPI::isBranch() const
{
    return curInsn().getCategory() == c_BranchInsn;
}
bool IA_IAPI::isCall() const
{
    return curInsn().getCategory() == c_CallInsn;
}

bool IA_IAPI::isInterruptOrSyscall() const
{
    return (Dyninst::InstructionAPI::isSoftwareInterrupt(curInsn()) || Dyninst::InstructionAPI::isSystemCall(curInsn()));
}

bool IA_IAPI::isSysEnter() const
{
    Instruction ci = curInsn();
    return (ci.getOperation().getID() == e_sysenter);
}

bool IA_IAPI::isIndirectJump() const {
    Instruction ci = curInsn();
    if(ci.getCategory() != c_BranchInsn) return false;
    if(ci.allowsFallThrough()) return false;
    bool valid;
    Address target;
    boost::tie(valid, target) = getCFT(); 
    if (valid) return false;
    parsing_printf("... indirect jump at 0x%lx, delay parsing it\n", current);
    return true;
}

void IA_IAPI::parseSyscall(std::vector<std::pair<Address, EdgeTypeEnum> >& outEdges) const
{
    parsing_printf("[%s:%d] Treating syscall as call to sink w/ possible FT edge to next insn at 0x%lx\n",
            FILE__, __LINE__, getAddr());
    outEdges.push_back(std::make_pair((Address)-1,CALL));
    outEdges.push_back(std::make_pair(getNextAddr(), CALL_FT));
}

void IA_IAPI::parseSysEnter(std::vector<std::pair<Address, EdgeTypeEnum> >& outEdges) const
{
    IA_IAPI* scratch = this->clone();

    do {
        scratch->advance();
    } while(scratch->isNop());
    if(scratch->curInsn().getCategory() == c_BranchInsn)
    {
        parsing_printf("[%s:%d] Detected Linux-ish sysenter idiom at 0x%lx\n",
                FILE__, __LINE__, getAddr());
        outEdges.push_back(std::make_pair(scratch->getAddr(), COND_NOT_TAKEN));
        scratch->advance();
        outEdges.push_back(std::make_pair(scratch->getAddr(), CALL_FT));
    }
    else
    {
        parsing_printf("[%s:%d] Treating sysenter as call to kernel w/normal return to next insn at 0x%lx\n",
                FILE__, __LINE__, getAddr());
        outEdges.push_back(std::make_pair(getNextAddr(), CALL_FT));
    }
    delete scratch;
}

bool DEBUGGABLE(void) { return true; }

void IA_IAPI::getNewEdges(std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
        Function* context,
        Block* currBlk,
        unsigned int num_insns,
        dyn_hash_map<Address, std::string> *plt_entries,
        const set<Address>& knownTargets) const
{
    Instruction ci = curInsn();

    // Only call this on control flow instructions!
    if(ci.getCategory() == c_CallInsn)
    {
        bool success; 
        Address target;
        boost::tie(success, target) = getCFT();
        bool callEdge = true;
        bool ftEdge = true;
        if( success && !isDynamicCall() )
        {
            if ( ! isRealCall() )
                callEdge = false;

            if ( simulateJump() ) 
            {
                outEdges.push_back(std::make_pair(target, DIRECT));
                callEdge = false;
                ftEdge = false;
            }
        }

        if ( unlikely(_obj->defensiveMode()) )
        {
            if (!success || isDynamicCall()) 
            {
                std::string empty;
                if ( ! isIATcall(empty) )
                    ftEdge = false;
            }
            else if ( ! _isrc->isValidAddress(target) )
            {
                ftEdge = false;
            }
        }

        if (callEdge) {
            if (success) {                
                outEdges.push_back(std::make_pair(target, CALL));
            } else {
                // Cannot use address 0 to represent indirect call,
                // because in .a files, address 0 can be legit function.
                outEdges.push_back(std::make_pair(std::numeric_limits<Address>::max(), CALL));
            }
        }

        if (ftEdge)
            outEdges.push_back(std::make_pair(getAddr() + getSize(), CALL_FT));

        // Any call site could be in try blocks
        set<Address> catchStarts;
        if (_obj->cs()->findCatchBlockByTryRange(getAddr(), catchStarts)) {
            for (auto ait = catchStarts.begin(); ait != catchStarts.end(); ++ait) {
                outEdges.push_back(std::make_pair(*ait, CATCH));
            }
        }
        return;
    }
    else if(ci.getCategory() == c_BranchInsn)
    {
        if(ci.allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getCFT().second,
                        COND_TAKEN));
            outEdges.push_back(std::make_pair(getNextAddr(), COND_NOT_TAKEN));
            return;
        }

        bool valid;
        Address target;
        boost::tie(valid, target) = getCFT(); 
        // Direct jump
        if (valid) 
        {

            if(!isTailCall(context, DIRECT, num_insns, knownTargets))
            {
                if(plt_entries->find(target) == plt_entries->end())
                {
                    outEdges.push_back(std::make_pair(target,DIRECT));
                }
                else
                {
                    parsing_printf("%s[%d]: PLT tail call to %lx (%s)\n", 
                            FILE__, __LINE__, target,
                            (*plt_entries)[target].c_str());
                    outEdges.push_back(std::make_pair(target, DIRECT));
                    tailCalls[DIRECT] = true;
                }
            }
            else
            {
                parsing_printf("%s[%d]: tail call to %lx\n", 
                        FILE__, __LINE__, target);
                outEdges.push_back(std::make_pair(target, DIRECT));
            }
            return;
        }
        else
        {
            parsing_printf("... indirect jump at 0x%lx\n", current);
            if( num_insns == 2 ) {
                // Handle a pernicious indirect tail call idiom here
                // What we've seen is mov (%rdi), %rax; jmp *%rax
                // Anything that tries to go enum->jump table *should* need more than two
                // instructions and has not been seen in the wild....
                if(currBlk == context->entry()) 
                {
                    parsing_printf("\tIndirect branch as 2nd insn of entry block, treating as tail call\n");
                    parsing_printf("%s[%d]: indirect tail call %s at 0x%lx\n", FILE__, __LINE__,
                            ci.format().c_str(), current);
                    outEdges.push_back(std::make_pair((Address)-1,INDIRECT));
                    tailCalls[INDIRECT]=true;
                    // Fix the cache, because we're dumb.
                    tailCalls[DIRECT]=true;
                    return;
                }
                else
                {
                    parsing_printf("... uninstrumentable due to 0 size\n");
                    return;
                }
            }
            if(isTailCall(context, INDIRECT, num_insns, knownTargets)) {
                parsing_printf("%s[%d]: indirect tail call %s at 0x%lx\n", FILE__, __LINE__,
                        ci.format().c_str(), current);
                outEdges.push_back(std::make_pair((Address)-1,INDIRECT));
                tailCalls[INDIRECT] = true;
                return;
            }
            parsing_printf("%s[%d]: jump table candidate %s at 0x%lx\n", FILE__, __LINE__,
                    ci.format().c_str(), current);
            parsedJumpTable = true;

            successfullyParsedJumpTable = parseJumpTable(context, currBlk, outEdges);

            parsing_printf("Parsed jump table\n");
            if(!successfullyParsedJumpTable || outEdges.empty()) {
                outEdges.push_back(std::make_pair((Address)-1,INDIRECT));
                parsing_printf("%s[%d]: unparsed jump table %s at 0x%lx in function %s UNINSTRUMENTABLE\n", FILE__, __LINE__,
                        ci.format().c_str(), current, context->name().c_str());
            }
            return;
        }
    }
    else if(ci.getCategory() == c_ReturnInsn)
    {
        parsing_printf("%s[%d]: return candidate %s at 0x%lx\n", FILE__, __LINE__,
                ci.format().c_str(), current);
        if(ci.allowsFallThrough())
        {
            outEdges.push_back(std::make_pair(getNextAddr(), FALLTHROUGH));
        }
        else if (!isReturn(context, currBlk)) {
            // If BLR is not a return, then it is a jump table
            parsedJumpTable = true;
            parsing_printf("%s[%d]: BLR jump table candidate %s at 0x%lx\n", FILE__, __LINE__,
                    ci.format().c_str(), current);
            successfullyParsedJumpTable = parseJumpTable(context, currBlk, outEdges);
            parsing_printf("Parsed BLR jump table\n");
            if(!successfullyParsedJumpTable || outEdges.empty()) {
                parsing_printf("%s[%d]: BLR unparsed jump table %s at 0x%lx in function %s UNINSTRUMENTABLE\n", 
                        FILE__, __LINE__, ci.format().c_str(), current, context->name().c_str());
                outEdges.push_back(std::make_pair((Address)-1,INDIRECT));
            }
        }

        parsing_printf("Returning from parse out edges\n");
        return;
    }
    else if(isSysEnter())
    {
        parseSysEnter(outEdges);
        return;
    } else if (DEBUGGABLE() && Dyninst::InstructionAPI::isSystemCall(curInsn())) {
        parseSyscall(outEdges);
        return;
    }

    fprintf(stderr, "Unhandled instruction %s\n", ci.format().c_str());
    assert(0);
}

bool IA_IAPI::isIPRelativeBranch() const
{
    // These don't exist on IA32...
#if !defined(arch_x86_64)
    return false;
#endif
    Instruction ci = curInsn();

    bool valid;
    Address target;
    boost::tie(valid, target) = getCFT();

    if(ci.getCategory() == c_BranchInsn &&
            !valid) {
        Expression::Ptr cft = ci.getControlFlowTarget();
        if(cft->isUsed(thePC[_isrc->getArch()]))
        {
            parsing_printf("\tIP-relative indirect jump to %s at 0x%lx\n",
                    cft->format(ci.getArch()).c_str(), current);
            return true;
        }
    }
    return false;

}

const Instruction & IA_IAPI::curInsn() const
{
    return curInsnIter->second;
}

bool IA_IAPI::isLeave() const
{
    Instruction ci = curInsn();
    return ci.isValid() && (ci.getOperation().getID() == e_leave);
}

bool IA_IAPI::isDelaySlot() const
{
    return false;
}

const Instruction& IA_IAPI::getInstruction() const
{
    return curInsn();
}

bool IA_IAPI::isRealCall() const
{
    // Obviated by simulateJump
    bool success;
    Address addr;
    boost::tie(success, addr) = getCFT();
    if (success &&
            (addr == getNextAddr())) {
        parsing_printf("... getting PC\n");
        return false;
    }
    if(isThunk()) {
        return false;
    }
    return true;
}

std::map<Address, bool> IA_IAPI::thunkAtTarget;


bool IA_IAPI::isConditional() const
{
    return curInsn().allowsFallThrough();
}

bool IA_IAPI::simulateJump() const
{
    // obfuscated programs simulate jumps by calling into a block that 
    // discards the return address from the stack, we check for these
    // fake calls in malware mode
    if (_obj->defensiveMode() && !isDynamicCall()) {
        return isFakeCall();
    }
    // TODO: we don't simulate jumps on x86 architectures; add logic as we need it.                
    return false;
}

std::pair<bool, Address> IA_IAPI::getFallthrough() const 
{
    return make_pair(true, curInsnIter->first + curInsnIter->second.size());
}

std::pair<bool, Address> IA_IAPI::getCFT() const
{
    if(validCFT) return cachedCFT;
    Expression::Ptr callTarget = curInsn().getControlFlowTarget();
    if (!callTarget) return make_pair(false, 0);
    // FIXME: templated bind(),dammit!
    callTarget->bind(thePC[_isrc->getArch()].get(), Result(s64, current));
    parsing_printf("%s[%d]: binding PC %s in %s to 0x%lx...", FILE__, __LINE__,
            thePC[_isrc->getArch()]->format(curInsn().getArch()).c_str(), curInsn().format().c_str(), current);

    Result actualTarget = callTarget->eval();

    if(actualTarget.defined)
    {
        cachedCFT = std::make_pair(true, actualTarget.convert<Address>());
        parsing_printf("SUCCESS (CFT=0x%lx)\n", cachedCFT.second);
    }
    else
    {
        cachedCFT = std::make_pair(false, 0); 
        parsing_printf("FAIL (CFT=0x%lx), callTarget exp: %s\n",
                cachedCFT.second,callTarget->format(curInsn().getArch()).c_str());
    }
    validCFT = true;

    if(isLinkerStub()) {
        parsing_printf("Linker stub detected: Correcting CFT.  (CFT=0x%lx)\n",
                cachedCFT.second);
    }

    return cachedCFT;
}

bool IA_IAPI::isRelocatable(InstrumentableLevel lvl) const
{
    Instruction ci = curInsn();
    if(ci.isValid() && (ci.getCategory() == c_CallInsn))
    {
        if(!isDynamicCall())
        {
            bool valid; Address addr;
            boost::tie(valid, addr) = getCFT();
            assert(valid);
            if(!_isrc->isValidAddress(addr))
            {
                parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                        addr);
                return false;
            }
        }
    }
    if(lvl == HAS_BR_INDIR)
    {
        return false;
    }
    return true;
}

bool IA_IAPI::parseJumpTable(Dyninst::ParseAPI::Function * currFunc,
        Dyninst::ParseAPI::Block* currBlk,
        std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) const
{

    IndirectControlFlowAnalyzer icfa(currFunc, currBlk);
    bool ret = icfa.NewJumpTableAnalysis(outEdges);

    parsing_printf("Jump table parser returned %d, %lu edges\n", ret, outEdges.size());
    for (auto oit = outEdges.begin(); oit != outEdges.end(); ++oit) parsing_printf("edge target at %lx\n", oit->first);
    // Update statistics 
    currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_COUNT);
    if (!ret) currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_FAIL);

    return ret;

}



InstrumentableLevel IA_IAPI::getInstLevel(Function * context, unsigned int num_insns) const
{
    InstrumentableLevel ret = InstructionAdapter::getInstLevel(context, num_insns);
    /*    if(ret == HAS_BR_INDIR && isIPRelativeBranch())
          {
          return NORMAL;
          }*/
    return ret;
}
