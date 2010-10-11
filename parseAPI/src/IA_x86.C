/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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


#include "IA_IAPI.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"
#include "dataflowAPI/h/slicing.h"
#include "dataflowAPI/h/SymEval.h"
//#include "StackTamperVisitor.h"

#include <deque>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;


bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const
{
    if(i->getOperation().getID() == e_mov)
    {
        if(i->readsMemory() || i->writesMemory())
        {
            parsing_printf("%s[%d]: discarding insn %s as stack frame preamble, not a reg-reg move\n",
                           FILE__, __LINE__, i->format().c_str());
            //return false;
        }
        if(i->isRead(stackPtr[_isrc->getArch()]) &&
           i->isWritten(framePtr[_isrc->getArch()]))
        {
            if((unsigned) i->getOperand(0).getValue()->size() == _isrc->getAddressWidth())
            {
                return true;
            }
            else
            {
                parsing_printf("%s[%d]: discarding insn %s as stack frame preamble, size mismatch for %d-byte addr width\n",
                               FILE__, __LINE__, i->format().c_str(), _isrc->getAddressWidth());
            }
        }
    }
    return false;
}

bool IA_IAPI::isNop() const
{
    Instruction::Ptr ci = curInsn();

    // TODO: add LEA no-ops
    assert(ci);
    if(ci->getOperation().getID() == e_nop)
        return true;
    if(ci->getOperation().getID() == e_lea)
    {
        std::set<Expression::Ptr> memReadAddr;
        ci->getMemoryReadOperands(memReadAddr);
        std::set<RegisterAST::Ptr> writtenRegs;
        ci->getWriteSet(writtenRegs);
        
        if(memReadAddr.size() == 1 && writtenRegs.size() == 1)
        {
            if(**(memReadAddr.begin()) == **(writtenRegs.begin()))
            {
                return true;
            }
            // TODO: check for zero displacement--do we want to bind here?
        }
    }
    return false;
}


bool IA_IAPI::isThunk() const {
  // Before we go a-wandering, check the target
    if (!_isrc->isValidAddress(getCFT()))
    {
        parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                       getCFT());
        return false;
    }

    const unsigned char *target =
            (const unsigned char *)_isrc->getPtrToInstruction(getCFT());
  // We're decoding two instructions: possible move and possible return.
  // Check for move from the stack pointer followed by a return.
    InstructionDecoder targetChecker(target, 
            2*InstructionDecoder::maxInstructionLength, _isrc->getArch());
    Instruction::Ptr thunkFirst = targetChecker.decode();
    Instruction::Ptr thunkSecond = targetChecker.decode();
    parsing_printf("... checking call target for thunk, insns are %s, %s\n", thunkFirst->format().c_str(),
                   thunkSecond->format().c_str());
    if(thunkFirst && (thunkFirst->getOperation().getID() == e_mov))
    {
        if(thunkFirst->isRead(stackPtr[_isrc->getArch()]))
        {
            parsing_printf("... checking second insn\n");
            if(!thunkSecond) {
                parsing_printf("...no second insn\n");
                return false;
            }
            if(thunkSecond->getCategory() != c_ReturnInsn)
            {
                parsing_printf("...insn %s not a return\n", thunkSecond->format().c_str());
                return false;
            }
            return true;
        }
    }
    parsing_printf("... real call found\n");
    return false;
}

#if 0
// Replaced by Kevin's version in IA_IAPI.C
bool IA_IAPI::simulateJump() const
{
  // We conclude that a call is a jump if the basic block it targets
  // contains a pop of the return address. 
  // 
  // For example: 
  // 2958:       83 ec 04                sub    $0x4,%esp
  // 295b:       e8 00 00 00 00          call   2960 <_fini+0xc>
  // 2960:       5b                      pop    %ebx
  // 2961:       81 c3 ac 07 00 00       add    $0x7ac,%ebx
  // 2967:       e8 a4 e5 ff ff          call   f10 <exit@plt+0x40>

  // The first call (call 0) is considered a jump because its immediate target
  // is a pop.
  
  if (!_isrc->isValidAddress(getCFT())) {
    return false;
  }

  const unsigned char *target =
    (const unsigned char *)_isrc->getPtrToInstruction(getCFT());

  // Arbitrarily try four instructions. 
  int maxInsns = 4;

  InstructionDecoder targetChecker(target, 
				   maxInsns*InstructionDecoder::maxInstructionLength, _isrc->getArch());
  bool isGetPC = false;
  for (int i = 0; i < maxInsns; ++i) {
    Instruction::Ptr insn = targetChecker.decode();
    if (!insn) break;

    if (insn->getOperation().getID() == e_pop) {
      // Nifty...
      isGetPC = true;
      break;
    }
    // Any other write of the stack pointer == baaaaad
    if (insn->isWritten(stackPtr[_isrc->getArch()])) {
      break;
    }
  }

  return isGetPC;
}
#endif

bool IA_IAPI::isTailCall(Function * context,unsigned int) const
{
    if(tailCall.first) {
        parsing_printf("\tReturning cached tail call check result: %d\n", tailCall.second);
        return tailCall.second;
    }
    tailCall.first = true;

    if(curInsn()->getCategory() == c_BranchInsn &&
       _obj->findFuncByEntry(_cr,getCFT()))
    {
        parsing_printf("\tjump to 0x%lx, TAIL CALL\n", getCFT());
        tailCall.second = true;
        return tailCall.second;
    }

    if (curInsn()->getCategory() == c_BranchInsn) {
        std::set<CodeRegion*> tregs;
        _obj->cs()->findRegions(getCFT(),tregs);
        if (tregs.size() && tregs.end() == tregs.find(context->region())) {
            tailCall.second = true;
            parsing_printf("\tjump to new region parsed as tail call\n");
            mal_printf("\tjump to %lx in new region parsed as tail call\n", 
                       getCFT());
            return tailCall.second;
        }
    }

    if(allInsns.size() < 2) {
        tailCall.second = false;
        parsing_printf("\ttoo few insns to detect tail call\n");
        return tailCall.second;
    }

    if(curInsn()->getCategory() == c_BranchInsn ||
       curInsn()->getCategory() == c_CallInsn)
    {
        std::map<Address, Instruction::Ptr>::const_iterator prevIter =
                allInsns.find(current);
        --prevIter;
        Instruction::Ptr prevInsn = prevIter->second;
        if(prevInsn->getOperation().getID() == e_leave)
        {
            parsing_printf("\tprev insn was leave, TAIL CALL\n");
            tailCall.second = true;
            return tailCall.second;
        }
        if(prevInsn->getOperation().getID() == e_pop)
        {
            if(prevInsn->isWritten(framePtr[_isrc->getArch()]))
            {
                parsing_printf("\tprev insn was %s, TAIL CALL\n", prevInsn->format().c_str());
                tailCall.second = true;
                return tailCall.second;
            }
            parsing_printf("\tprev insn was %s, not tail call\n", prevInsn->format().c_str());
        }
    }
    tailCall.second = false;
    return tailCall.second;
}

bool IA_IAPI::savesFP() const
{
    Instruction::Ptr ci = curInsn();
    if(ci->getOperation().getID() == e_push)
    {
        return(ci->isRead(framePtr[_isrc->getArch()]));
    }
    return false;
}

bool IA_IAPI::isStackFramePreamble() const
{
    if(savesFP())
    {
        InstructionDecoder tmp(dec);
        std::vector<Instruction::Ptr> nextTwoInsns;
        nextTwoInsns.push_back(tmp.decode());
        nextTwoInsns.push_back(tmp.decode());
        if(isFrameSetupInsn(nextTwoInsns[0]) ||
           isFrameSetupInsn(nextTwoInsns[1]))
        {
            return true;
        }
    }
    return false;
}

bool IA_IAPI::cleansStack() const
{
    Instruction::Ptr ci = curInsn();
    return (ci->getCategory() == c_ReturnInsn) &&
            ci->getOperand(0).getValue();

}

bool IA_IAPI::isReturnAddrSave() const
{
    // not implemented on non-power
    return false;
}

/* returns true if the call leads to:
 * -an invalid instruction (or immediately branches/calls to an invalid insn)
 * -a block not ending in a return instruction that pops the return address 
 *  off of the stack
 */
bool IA_IAPI::isFakeCall() const
{
    assert(_obj->defensiveMode());

    if (isDynamicCall()) {
        return false;
    }

    // get func entry
    bool tampers = false;
    Address entry = getCFT();
    if ( ! _cr->contains(entry) ) {
        return false;
    }

    if ( ! _isrc->isCode(entry) ) {
        mal_printf("WARNING: found function call at %lx "
                   "to invalid address %lx %s[%d]\n", current, 
                   entry, FILE__,__LINE__);
        return false;
    }

    // get instruction at func entry
    const unsigned char* bufPtr =
     (const unsigned char *)(_cr->getPtrToInstruction(entry));
    Offset entryOff = entry - _cr->offset();
    InstructionDecoder newdec( bufPtr,
                              _cr->length() - entryOff,
                              _cr->getArch() );
    IA_IAPI * ah = new IA_IAPI(newdec, entry, _obj, _cr, _isrc);
    Instruction::Ptr insn = ah->curInsn();

    // follow ctrl transfers until you get a block containing non-ctrl 
    // transfer instructions, or hit a return instruction
    while (insn->getCategory() == c_CallInsn ||
           insn->getCategory() == c_BranchInsn) 
    {
        entry = ah->getCFT();
        if ( ! _cr->contains(entry) || ! _isrc->isCode(entry) ) {
            mal_printf("WARNING: found call to function at %lx that "
                       "leaves to %lx, out of the code region %s[%d]\n", 
                       current, entry, FILE__,__LINE__);
            return false;
        }
        bufPtr = (const unsigned char *)(_cr->getPtrToInstruction(entry));
        entryOff = entry - _cr->offset();
        newdec = InstructionDecoder(bufPtr, 
                                    _cr->length() - entryOff, 
                                    _cr->getArch());
        delete ah;
        ah = new IA_IAPI(newdec, entry, _obj, _cr, _isrc);
        insn = ah->curInsn();
    }

    // calculate instruction stack deltas for the block, leaving the iterator
    // at the last ins'n if it's a control transfer, or after calculating the 
    // last instruction's delta if we run off the end of initialized memory
    int stackDelta = 0;
    int addrWidth = _isrc->getAddressWidth();
    static Expression::Ptr theStackPtr
        (new RegisterAST(MachRegister::getStackPointer(_isrc->getArch())));
    Address curAddr = entry;

    while(true) {

        // exit condition 1
        if (insn->getCategory() == c_CallInsn ||
            insn->getCategory() == c_ReturnInsn ||
            insn->getCategory() == c_BranchInsn) 
        {
            break;
        }

        // calculate instruction delta
        if(insn->isWritten(theStackPtr)) {
            entryID what = insn->getOperation().getID();
            int sign = 1;
            switch(what) 
            {
            case e_push:
                sign = -1;
            case e_pop: {
                int size = insn->getOperand(0).getValue()->size();
                stackDelta += sign * size;
                if (1 == sign) {
                    mal_printf("pop ins'n at %lx in func at %lx changes sp "
                               "by %d. %s[%d]\n", ah->getAddr(), entry,
                               sign * size, FILE__, __LINE__);
                }
                break;
            }
            case e_pusha:
            case e_pushad:
                sign = -1;
            case e_popa:
            case e_popad:
                if (1 == sign) {
                    mal_printf("popad ins'n at %lx in func at %lx changes sp "
                               "by %d. %s[%d]\n", ah->getAddr(), 
                               entry, 8 * sign * addrWidth, FILE__, __LINE__);
                }
                stackDelta += sign * 8 * addrWidth;
                break;
            case e_pushf:
            case e_pushfd:
                sign = -1;
            case e_popf:
            case e_popfd:
                stackDelta += sign * 4;
                if (1 == sign) {
                    mal_printf("popf ins'n at %lx in func at %lx changes sp "
                               "by %d. %s[%d]\n", ah->getAddr(), entry, 
                               sign * 4, FILE__, __LINE__);
                }
                break;
            case e_enter:
                //mal_printf("Saw enter instruction at %lx in isFakeCall, "
                //           "quitting early, assuming not fake "
                //           "%s[%d]\n",curAddr, FILE__,__LINE__);
                //KEVIN: unhandled case, but not essential for correct analysis
                return false;
                break;
            case e_leave:
                mal_printf("WARNING: saw leave instruction "
                           "at %lx that is not handled by isFakeCall %s[%d]\n",
                           curAddr, FILE__,__LINE__);
                //KEVIN: unhandled, not essential for correct analysis, would
                // be a red flag if there wasn't an enter ins'n first and 
                // we didn't end in a return instruction
                break;
            case e_sub:
                //mal_printf("Saw subtract from stack ptr at %lx in "
                //           "isFakeCall, quitting early, assuming not fake "
                //           "%s[%d]\n",curAddr, FILE__,__LINE__);
                //KEVIN: unhandled case, but not essential for correct analysis
                return false;
                break;
            case e_add: {
                mal_printf("in isFakeCall, add ins'n "
                           "at %lx (in first block of function at "
                           "%lx) modifies the sp. %s[%d]\n", 
                           ah->getAddr(), entry, FILE__, __LINE__);
                std::vector<Operand> opers;
                insn->getOperands(opers);
                // if the insn uses a register (other than sp, which it writes)
                // we won't be able to get a result out of the instruction
                for (unsigned oidx = 0; oidx < opers.size(); oidx++) {
                    if (!opers[oidx].isWritten()) {
                        Expression::Ptr expr = opers[oidx].getValue();
                        if (Arch_x86 == _isrc->getArch() || 
                            Arch_ppc32 == _isrc->getArch()) 
                        {
                            expr->bind(theStackPtr.get(),Result(s32,0));
                        } else {
                            expr->bind(theStackPtr.get(),Result(s64,0));
                        }
                        Result val = expr->eval();
                        if (val.defined) {
                            stackDelta += val.convert<Address>();
                            break;
                        }
                    }
                }
                break; 
            }
            default: {
                //KEVINTODO: remove this assert
                fprintf(stderr,"WARNING: in isFakeCall non-push/pop "
                        "ins'n at %lx (in first block of function at "
                        "%lx) modifies the sp by an unknown amount. "
                        "%s[%d]\n", ah->getAddr(), entry, 
                        FILE__, __LINE__);
                assert(0); // what stack-altering instruction is this?
                break;
            } // end default block
            }
        }

        if (stackDelta > 0) {
            tampers=true;
        }

        // exit condition 2
        ah->advance();
        Instruction::Ptr next = ah->curInsn();
        if (NULL == next) {
            break;
        }
        curAddr += insn->size();
        insn = next;
    } 

    // not a fake call if it ends w/ a return instruction
    if (insn->getCategory() == c_ReturnInsn) {
        return false;
    }

    // if the stack delta is positive or the return address has been replaced
    // with an absolute value, it's a fake call, since in both cases 
    // the return address is gone and we cannot return to the caller
    if ( 0 < stackDelta || tampers ) {
        mal_printf("Found fake call at %lx to %lx, where the first block "
                   "pops off the return address %s[%d]\n", 
                   current, entry, FILE__,__LINE__);
        return true;
    }

    return false;
}

const char* IA_IAPI::isIATcall() const
{
    if (!isDynamicCall()) {
        return NULL;
    }

    if (!curInsn()->readsMemory()) {
        return NULL;
    }

    std::set<Expression::Ptr> memReads;
    curInsn()->getMemoryReadOperands(memReads);
    if (memReads.size() != 1) {
        return NULL;
    }

    Result memref = (*memReads.begin())->eval();
    if (!memref.defined) {
        return NULL;
    }
    Address entryAddr = memref.convert<Address>();

    // convert to a relative address
    if (_obj->cs()->loadAddress() < entryAddr) {
        entryAddr -= _obj->cs()->loadAddress();
    }
    
    if (!_obj->cs()->isValidAddress(entryAddr)) {
        return NULL;
    }

    // calculate the address of the ASCII string pointer, 
    // skip over the IAT entry's two-byte hint
    void * asciiPtr = _obj->cs()->getPtrToInstruction(entryAddr);
    if (!asciiPtr) {
        return NULL;
    }
    Address funcAsciiAddr = 2 + *(Address*) asciiPtr;
    if (!_obj->cs()->isValidAddress(funcAsciiAddr)) {
        return NULL;
    }

    // see if it's really a string that could be a function name
    char *funcAsciiPtr = (char*) _obj->cs()->getPtrToData(funcAsciiAddr);
    if (!funcAsciiPtr) {
        return NULL;
    }
    char cur = 'a';
    int count=0;
    do {
        cur = funcAsciiPtr[count];
        count++;
    }while (count < 100 && 
            _obj->cs()->isValidAddress(funcAsciiAddr+count) &&
            ((cur >= 'A' && cur <= 'z') ||
             (cur >= '0' && cur <= '9')));
    if (cur != 0 || count <= 1) 
        return NULL;

    mal_printf("found IAT call at %lx to %s\n", current, funcAsciiPtr);
    return funcAsciiPtr;
}

bool IA_IAPI::isNopJump() const
{
    InsnCategory cat = curInsn()->getCategory();
    if(c_BranchInsn == cat && current+1 == getCFT()) {
        return true;
    }
    return false;
}

bool IA_IAPI::isLinkerStub() const
{
    // No need for linker stubs on x86 platforms.
    return false;
}
