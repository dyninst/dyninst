/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

#include "arch.h"
#include "arch-ia32.h"
#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"

#include "BPatch_Set.h"

//some more function used to identify the properties of the instruction
/**  the instruction used to return from the functions
  * @param i the instruction value 
  */
bool InstrucIter::isALeaveInstruction()
{
    return insn.isLeave();
}

bool InstrucIter::isAReturnInstruction()
{
    return insn.isReturn();
}

extern const unsigned char*
skip_headers(const unsigned char*,bool&,bool&);

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */

bool InstrucIter::isAIndirectJumpInstruction()
{
    if((insn.type() & IS_JUMP) &&
       (insn.type() & INDIR))
    {
        /* since there are two is_jump and indirect instructions
           we are looking for the one with the indirect register
           addressing mode one of which ModR/M contains 4 in its
           reg/opcode field */
        bool isWordAddr,isWordOp;
        const unsigned char* ptr =
            skip_headers(insn.ptr(),isWordAddr,isWordOp);
        assert(*ptr == 0xff);
        ptr++;
        if((*ptr & 0x38) == 0x20) 
            return true;
    }
    return false;
}

bool InstrucIter::isStackFramePreamble()
{
    // test for
    // one:  push   %ebp
    // two:  mov    %esp,%ebp
    
    instruction next_insn;
    next_insn.getNextInstruction( insn.ptr() + insn.size() );
    
    const unsigned char *p, *q;
    p = insn.ptr();
    q = next_insn.ptr();
    return (insn.size() == 1
            && p[0] == 0x55
            && next_insn.size() == 2
            && q[0] == 0x89
            && q[1] == 0xe5);    
}

bool InstrucIter::isFramePush()
{
    // test for
    // push %ebp
    return (insn.size() == 1 && insn.ptr()[0] == 0x55);
}

bool InstrucIter::isFrameSetup()
{
    //test for
    // movl %esp,%ebp
    return (insn.size() == 2 && 
            insn.ptr()[0] == 0x89 && insn.ptr()[1] == 0xe5);
}

/** is the instruction a conditional branch instruction 
 * @param i the instruction value 
 */ 
bool InstrucIter::isACondBranchInstruction()
{
    if(insn.type() & IS_JCC)
        return true;
    return false;
}

/** is the instruction an unconditional branch instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isAJumpInstruction()
{
    insn.getNextInstruction( instPtr );
    if((insn.type() & IS_JUMP) &&
       !(insn.type() & INDIR) && 
       !(insn.type() & PTR_WX))
        return true;
    return false;
}

/** is the instruction a call instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isACallInstruction()
{
    return insn.isCall();
}

bool InstrucIter::isIndir() 
{
    return insn.isIndir();
}

bool InstrucIter::isANopInstruction()
{
    return insn.isNop();
}

bool InstrucIter::isAnneal()
{
    return true;
}

/** function which returns the offset of control transfer instructions
 * @param i the instruction value 
  */
Address InstrucIter::getBranchTargetAddress( Address pos )
{
    return insn.getTarget( currentAddress );
}

Address InstrucIter::getBranchTarget()
{
    return insn.getTarget( currentAddress );
}

void initOpCodeInfo()
{
}

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
    static unsigned int log2[] = { 0, 0, 1, 1, 2 };
    
    // TODO 16-bit registers
    
    int nac = 0;
    
    ia32_memacc mac[3];
    ia32_condition cnd;
    ia32_instruction i(mac, &cnd);
    
    const unsigned char* addr = insn.ptr();
    BPatch_memoryAccess* bmap = BPatch_memoryAccess::none;
    
#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300
    ia32_decode(IA32_DECODE_MEMACCESS|IA32_DECODE_CONDITION, addr, i);
#else
  ia32_decode<(IA32_DECODE_MEMACCESS|IA32_DECODE_CONDITION)>(addr, i);
#endif
  
  bool first = true;

  for(int j=0; j<3; ++j) {
    const ia32_memacc& mac = i.getMac(j);
    const ia32_condition& cond = i.getCond();
    int bmapcond = cond.is ? cond.tttn : -1;
    if(mac.is) {
      if(first) {
        if(mac.prefetch) {
          if(mac.prefetchlvl > 0) // Intel
            bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(), 
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchlvl);
          else // AMD
            bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchstt + IA32AMDprefetch);
        }
        else switch(mac.sizehack) { // translation to pseudoregisters
        case 0:
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					 mac.read, mac.write,
                                         mac.size, mac.imm, mac.regs[0], mac.regs[1], mac.scale, 
                                         bmapcond, mac.nt);
          break;
        case shREP: // use ECX register to compute final size as mac.size * ECX
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, 1 , log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPESCAS:
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNESCAS:
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_NESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPECMPS:
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ECMPS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNECMPS:
          bmap = new BPatch_memoryAccess(getInstruction().ptr(), getInstruction().size(),
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_NECMPS, log2[mac.size],
                                         bmapcond, false);
          break;
        default:
          assert(!"Unknown size hack");
        }
        first = false;
      }
      else
        switch(mac.sizehack) { // translation to pseudoregisters
        case 0:
          bmap->set2nd(mac.read, mac.write, mac.size, mac.imm,
                       mac.regs[0], mac.regs[1], mac.scale);
          break;
        case shREP: // use ECX register to compute final size as mac.size * ECX
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
	      0, -1, 1 , log2[mac.size],
                       bmapcond, false);
          break;
        case shREPESCAS:
        case shREPNESCAS:
          assert(!"Cannot happen");
          break;
        case shREPECMPS:
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                       0, -1, IA32_ECMPS, log2[mac.size],
                       bmapcond, false);
          break;
        case shREPNECMPS:
          //fprintf(stderr, "In set2nd[shREPNECMPS]!!!\n");
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                       0, -1, IA32_NECMPS, log2[mac.size],
                       bmapcond, false);
          break;
        default:
          assert(!"Unknown size hack");
        }
      ++nac;
    }
  }
  assert(nac < 3);
  
  return bmap;
}

BPatch_instruction *InstrucIter::getBPInstruction() {

  BPatch_memoryAccess *ma = isLoadOrStore();
  BPatch_instruction *in;

  if (ma != BPatch_memoryAccess::none)
    return ma;

  const instruction i = getInstruction();
  in = new BPatch_instruction(i.ptr(), i.size());

  return in;
}

bool InstrucIter::getMultipleJumpTargets(pdvector<Address>& result,
					 instruction& tableInsn, 
					 instruction& maxSwitchInsn,
                     bool isAddressInJmp )
{ 
    Address backupAddress = currentAddress;
    bool isWordAddr,isWordOp;
    
    unsigned maxSwitch = 0;
    const unsigned char* ptr = 
        skip_headers(maxSwitchInsn.ptr(),isWordAddr,isWordOp);
    
    if(*ptr == 0x3d)
    {
        ptr++;
        if(isWordOp)
        {
            maxSwitch |= *(ptr+1);
            maxSwitch <<= 8;
            maxSwitch |= *ptr;
        }
        else
            maxSwitch = *(const unsigned*)ptr;
        
        maxSwitch++;
    }
    else if(*ptr == 0x83)
    {
        ptr++;
        if((*ptr & 0x38) == 0x38)
        {
            unsigned modRM = *ptr;
            unsigned Mod,Reg,RM;
            bool hasSIB = insn_hasSIB(modRM,Mod,Reg,RM);
            ptr++;
            if(hasSIB)
                ptr++;
            if ( insn_hasDisp8(modRM) ) 
            {
                ptr++;
            }
            else if( insn_hasDisp32(modRM) )
            {
                ptr += 4;
            }
            maxSwitch = *ptr;
            maxSwitch++;
        }
    }
    if( !maxSwitch )
    {
        result += backupAddress;	
        return false;
    }
    
    Address jumpTable = 0;
    ptr = skip_headers(tableInsn.ptr(),isWordAddr,isWordOp);
    if(isAddressInJmp || (!isAddressInJmp && (*ptr == 0x8b)))
    {
        ptr++;
        if((((*ptr & 0xc7) == 0x04) &&
            ((*(ptr+1) & 0xc7) == 0x85)) ||
           ((*ptr & 0xc7) == 0x80))
        {
            if((*ptr & 0xc7) == 0x80)
                ptr += 1;
            else
                ptr += 2;
            
            if(isWordAddr)
            {
                jumpTable |= *(ptr+1);
                jumpTable <<= 8;
                jumpTable |= *ptr;
            }
            else
                jumpTable = *(const Address*)ptr;
        }
    }
    
    if(!jumpTable)
    {
        result += backupAddress;	
        return false;
    }
    
    for(unsigned int i=0;i<maxSwitch;i++)
    {
        Address tableEntry = jumpTable + (i * sizeof(Address));
        int jumpAddress = 0;
        if( addressImage->isValidAddress( tableEntry ) )
        {    
            jumpAddress = *(int*)addressImage->getPtrToInstruction(tableEntry);
            result.push_back( jumpAddress );
        }
    }   
    return true;
}

bool InstrucIter::delayInstructionSupported()
{
    return false;
}

bool InstrucIter::hasMore()
{
    if( currentAddress + insn.size() > range + baseAddress )
        return false;
    
    return true;
}

bool InstrucIter::hasPrev()
{
    if( currentAddress <= baseAddress )
        return false;
    
    return true;
}

Address InstrucIter::prevAddress()
{
    //incorrect, unnecessary
    Address i = currentAddress - 1;
    //		break;
    return i;
}

Address InstrucIter::nextAddress()
{
    instPtr += insn.size();
    currentAddress += insn.size();
    insn.getNextInstruction( instPtr );
    
    return currentAddress;
}


void InstrucIter::setCurrentAddress(Address addr)
{
    //don't need this on x86
    currentAddress = addr;
}

#if defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0)
bool InstrucIter::isInstruction()
{
    return false;
}
#endif

instruction InstrucIter::getInstruction()
{
    return insn;
}

instruction InstrucIter::getNextInstruction()
{
    instruction next_insn;
    next_insn.getNextInstruction( insn.ptr() + insn.size() );
    
    return next_insn;
}

instruction InstrucIter::getPrevInstruction()
{
    //incorrect, unnecessary
    return insn;
}    

Address InstrucIter::operator++()
{
    currentAddress = nextAddress();
    return currentAddress;
}

Address InstrucIter::operator--()
{
    //incorrect, unecessary
    currentAddress = prevAddress();
    return currentAddress;
}

Address InstrucIter::operator++(int)
{
    Address ret = currentAddress;
    currentAddress = nextAddress();
    return ret;
}

Address InstrucIter::operator--(int)
{
    //incorrect, unnecessary
    Address ret = currentAddress;
    currentAddress = prevAddress();
    return ret;
}

Address InstrucIter::operator*()
{
    return currentAddress;
}
