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
#include "arch-x86.h"
#include "util.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "InstrucIter.h"
#include "inst-x86.h"

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

/** is the instruction an indirect jump instruction 
  * @param i the instruction value 
  */

bool InstrucIter::isAIndirectJumpInstruction()
{
    if((insn.type() & IS_JUMP) && (insn.type() & INDIR))
    {
        /* since there are two is_jump and indirect instructions
           we are looking for the one with the indirect register
           addressing mode one of which ModR/M contains 4 in its
           reg/opcode field */
        const unsigned char* ptr = insn.op_ptr();
        assert(*ptr == 0xff);
        ptr++;
        if((*ptr & 0x38) == 0x20) 
            return true;
    }
    return false;
}

bool InstrucIter::isStackFramePreamble(int & /*unused*/)
{
    return ::isStackFramePreamble( insn );
}

bool InstrucIter::isFramePush()
{
    // test for
    // push %ebp (or push %rbp for 64-bit)
    return (insn.size() == 1 && insn.ptr()[0] == 0x55);
}

bool InstrucIter::isFrameSetup()
{
    //test for
    // movl %esp,%ebp

    // 64-bit:
    // movq %rsp, %rbp

  if (!ia32_is_mode_64()) {
    return (insn.size() == 2 && 
            insn.ptr()[0] == 0x89 && insn.ptr()[1] == 0xe5);
  }
  else {
    return (insn.size() == 3 &&
	    insn.ptr()[0] == 0x48 &&
	    insn.ptr()[1] == 0x89 &&
	    insn.ptr()[2] == 0xe5);
  }
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
    insn.setInstruction( (unsigned char *)instPtr );
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
Address InstrucIter::getBranchTargetOffset()
{
    // getTarget returns displacement+address parameter
    return insn.getTarget(0);
}

Address InstrucIter::getBranchTargetAddress()
{
    return insn.getTarget(current);
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

// return target addresses from a jump table
// tableInsn - instruction that load address of table entry into a register,
//             the displacement will give us the table's base address
// maxSwitchInsn - compare instrction that does a range check on the
//                 jump table index, gives us number of entries in immediate
bool InstrucIter::getMultipleJumpTargets(pdvector<Address>& result,
					 instruction& tableInsn, 
					 instruction& maxSwitchInsn,
                                         bool isAddressInJmp )
{ 
    int addrWidth;
    if (proc_)
        addrWidth = proc_->getAddressWidth();
    else 
        addrWidth = img_->getAddressWidth();

    Address backupAddress = current;
    
    unsigned maxSwitch = 0;

    ia32_prefixes pref;
    const unsigned char* ptr = skip_headers(maxSwitchInsn.ptr(), &pref);
    bool isWordAddr = pref.getAddrSzPrefix();
    bool isWordOp = pref.getOperSzPrefix();
    
    //get the imm value from the compare instruction and store it in 
    //maxSwitch
    if( *ptr == 0x3d )
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
    else if( *ptr == 0x3c )
    {
        ptr++;
        maxSwitch = *ptr;
        maxSwitch++;
    }
    else if( *ptr == 0x83 || *ptr == 0x80 || *ptr == 0x81 ) 
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
            else if( insn_hasDisp32( modRM ) )
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
    ptr = tableInsn.op_ptr();
    if(isAddressInJmp || (!isAddressInJmp && (*ptr == 0x8b)))
    {
        ptr++;
        if(
	   ( ((*ptr & 0xc7) == 0x04) &&
	     ( ((*(ptr+1) & 0xc7) == 0x85) || ((*(ptr+1) & 0xc7) == 0xc5) ) ) ||
           ((*ptr & 0xc7) == 0x80) )
        {
            if((*ptr & 0xc7) == 0x80)
                ptr += 1;
            else
                ptr += 2;
            
            if(isWordAddr) {
                jumpTable |= *(ptr+1);
                jumpTable <<= 8;
                jumpTable |= *ptr;

	    } else {
		jumpTable = *(const int *)ptr;
	    }
        }
    }

    if(!jumpTable)
    {
        result += backupAddress;	
        return false;
    }

    for(unsigned int i=0;i<maxSwitch;i++)
    {
        Address tableEntry = jumpTable + (i * addrWidth);
        int jumpAddress = 0;
        if( proc_ &&
            proc_->isValidAddress( tableEntry ) ) {
            if (addrWidth == sizeof(Address))
                jumpAddress = *(const Address *)proc_->getPtrToInstruction(tableEntry);
            else
                jumpAddress = *(const int *)proc_->getPtrToInstruction(tableEntry);
        }
        else if (img_ &&
                 img_->isValidAddress(tableEntry)) {
            if (addrWidth == sizeof(Address))
                jumpAddress = *(const Address *)img_->getPtrToInstruction(tableEntry);
            else
                jumpAddress = *(const int *)img_->getPtrToInstruction(tableEntry);
        }
        if (jumpAddress)
            result.push_back(jumpAddress);
    }
    return true;
}

bool InstrucIter::delayInstructionSupported()
{
    return false;
}

Address InstrucIter::peekPrev()
{
    if (prevInsns.size()) {
        return prevInsns.back().prevAddr;
    }
    else 
        return 0;
}

Address InstrucIter::peekNext() {
    Address tmp = current;
    tmp += insn.size();
    return tmp;
}

void InstrucIter::setCurrentAddress(Address addr)
{
    // Make sure the new addr is aligned
    // This is unsafe if we're looking at anything more than a basic block;
    // best-effort.

    if (current < addr) {
        while (current != addr) {
            if (current > addr) {
                // We missed; oops.
                current = addr;
                break;
            }
            assert(current < addr);
            (*this)++;
        }
    }
    else if (current > addr) {
        while (current != addr) {
            if (current < addr) {
                current = addr;
                break;
            }
            (*this)--;
        }
    }
    initializeInsn();
}

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
bool InstrucIter::isInstruction()
{
    return false;
}
#endif

instruction InstrucIter::getInstruction()
{
    return insn;
}

instruction *InstrucIter::getInsnPtr() {
    instruction *insnPtr = new instruction(insn);
    return insnPtr;
}

instruction InstrucIter::getNextInstruction()
{
    instruction next_insn;
    next_insn.setInstruction( insn.ptr() + insn.size() );
    
    return next_insn;
}

instruction InstrucIter::getPrevInstruction()
{
    instruction prev_insn;
    prev_insn.setInstruction((unsigned char *) prevInsns.back().prevPtr);

    return prev_insn;
}    

// Prefix...
Address InstrucIter::operator++()
{
    previous prev;
    prev.prevAddr = current;
    prev.prevPtr = instPtr;
    prevInsns.push_back(prev);

    Address tmp = (Address) instPtr;
    tmp += insn.size();
    instPtr = (void *) tmp;
    current += insn.size();
    insn.setInstruction( (unsigned char *) instPtr );
    
    return current;
}

// Prefix...
Address InstrucIter::operator--()
{
    if (prevInsns.size()) {
        instPtr = prevInsns.back().prevPtr;
        current = prevInsns.back().prevAddr;
        insn.setInstruction((unsigned char *) instPtr);
        prevInsns.pop_back();
        return current;
    }
    else 
        return 0;
}

// Postfix...
Address InstrucIter::operator++(int)
{
    Address ret = current;
    ++(*this);
    return ret;
}

// Postfix...
Address InstrucIter::operator--(int)
{
    Address ret = current;
    --(*this);
    return ret;
}

Address InstrucIter::operator*()
{
    return current;
}

void parseRegisters(int * readArr, int * writeArr,
		    instruction * i,ia32_instruction *ii,int whichOp, int readOrWrite)
{
  ia32_entry * entry = ii->getEntry();
  ia32_prefixes * pref = ii->getPrefix();

  char * addr = (char *) i->ptr();

  char * addrPtr = addr;
  int hasSIB = 0;
  int hasModRM = 0;
  int opCodeSize = 1;
  
  unsigned mod = 0;
  unsigned reg = 0;
  unsigned rm = 0;

  addrPtr += ii->getPrefixCount();

  if (*addrPtr == 0x0f)
    opCodeSize = 2;

  if (entry->operands[whichOp].admet == am_G || 
      entry->operands[whichOp].admet == am_R ||
      entry->operands[whichOp].admet == am_E)
    {
      hasModRM = 1;
      addrPtr += opCodeSize;
      char modByte = *addrPtr;
      mod = (modByte >> 6) & 0x03;
      reg = (modByte >> 3) & 0x07;
      rm =  modByte & 0x07;
      if ((mod != 3) && (rm == 4))
	hasSIB = 1;
      //printf("mod - %d, reg - %d, rm - %d\n", mod, reg, rm);
      addrPtr++;
    }

  int rIndex =0; 
  int wIndex =0;
  for (rIndex = 0; rIndex < 3 && readArr[rIndex] != -1; rIndex++)
    {}

  for (wIndex = 0; wIndex < 3 && writeArr[wIndex] != -1; wIndex++)
    {}

  if (entry->operands[whichOp].admet == am_G)
    {
      if (pref->rexR())
	reg = reg+8;
      if (readOrWrite == READ_OP)
	readArr[rIndex] = reg;
      else if (readOrWrite == WRITE_OP)
	writeArr[wIndex] = reg;
    }
  else if (entry->operands[whichOp].admet == am_R)
    {
      if (pref->rexB())
	rm = rm+8;
      if (readOrWrite == READ_OP)
	readArr[rIndex] = rm;
      else if (readOrWrite == WRITE_OP)
	writeArr[wIndex] = rm;

    }
  else if (entry->operands[whichOp].admet == am_E)
    {
      int regi = -1; /* modRM reg 1 */
      int regi2 = -1; /* SIB reg 1 */
      int regi3 = -1; /* SIB reg 2 */
      if (rm == 0)
	regi = REGNUM_RAX;
      else if (rm == 1)
	regi = REGNUM_RCX;
      else if (rm == 2)
	regi = REGNUM_RDX;
      else if (rm == 3)
	regi = REGNUM_RBX;
      else if (rm == 5 && mod != 0)
	regi = REGNUM_RBP;
      else if (rm == 5 && mod == 0)
	regi = -1; /* RIP + disp32 */
      else if (rm == 6)
	regi = REGNUM_RSI;
      else if (rm == 7)
	regi = REGNUM_RDI;      
      else if (mod == 3 && rm == 4)
	regi = REGNUM_RBP;
      else if (mod != 3 && rm == 4)
	{
	  char sibByte = *addrPtr;
	  //unsigned scale = (sibByte >> 6) & 0x03;
	  unsigned ind = (sibByte >> 3) & 0x07;
	  unsigned base =  sibByte & 0x07;
	  
	  if (ind == 0)
	    regi2 = REGNUM_RAX;
	  else if (ind == 1)
	    regi2 = REGNUM_RCX;
	  else if (ind == 2)
	    regi2 = REGNUM_RDX;
	  else if (ind == 3)
	    regi2 = REGNUM_RBX;
	  else if (ind == 4 && pref->rexX()) 
	    regi2 = REGNUM_RSP;
	  else if (ind == 5)
	    regi2 = REGNUM_RBP;
	  else if (ind == 6)
	    regi2 = REGNUM_RSI;
	  else if (ind == 7)
	    regi2 = REGNUM_RDI;

	  if (base == 0)
	    regi3 = REGNUM_RAX;
	  else if (base == 1)
	    regi3 = REGNUM_RCX;
	  else if (base == 2)
	    regi3 = REGNUM_RDX;
	  else if (base == 3)
	    regi3 = REGNUM_RBX;
	  else if (base == 4)
	    regi = REGNUM_RSP;
	  else if (base == 5 && mod !=0 )
	    regi3 = REGNUM_RBP;
	  else if (base == 6)
	    regi3 = REGNUM_RSI;
	  else if (base == 7)
	    regi3 = REGNUM_RDI;

	  if (pref->rexX() && regi2 !=- 1)
	    regi2 = regi2+8;
	  readArr[rIndex] = regi2;
	  rIndex++;

	  if (pref->rexB() && regi3 != -1)
	    regi3 = regi3+8;
	  readArr[rIndex] = regi3;

	  //printf("SIB scale - %d, ind - %d, base - %d\n",scale,ind,base);
	}
      
      if (!hasSIB)
	{
	  if (pref->rexB())
	    regi = regi+8;
	  readArr[rIndex] = regi;
	}
    }
  else if (entry->operands[whichOp].admet == am_reg)
    {
      int ot = entry->operands[whichOp].optype;
      int regi = -1;
      if (ot == r_eAX || ot == r_EAX)
	regi = 0;
      else if (ot == r_eBX || ot == r_EBX)
	regi = 3;
      else if(ot == r_eCX || ot == r_ECX)
	regi = 1;
      else if(ot == r_eDX || ot == r_EDX)
	regi = 2;
      else if(ot == r_eSP || ot == r_ESP)
	regi = 4;
      else if (ot == r_eBP || ot == r_EBP)
	regi = 5;
      else if (ot == r_eSI || ot == r_ESI)
	regi = 6;
      else if(ot == r_eDI || ot == r_EDI)
	  regi = 7;

      if (pref->rexB())
	regi = regi+8;
      if (readOrWrite == READ_OP)
	readArr[rIndex] = regi;
      else if (readOrWrite == WRITE_OP)
	writeArr[wIndex] = regi;

    }  
}


void InstrucIter::readWriteRegisters(int * readRegs, int * writeRegs)
{
  instruction i = getInstruction();
 
  ia32_instruction ii;
  
  const unsigned char * addr = i.ptr();
#if defined(os_windows)
  ia32_decode(0, addr,ii);
#else
  ia32_decode<0>(addr,ii);
#endif

  ia32_entry * entry = ii.getEntry();

  for (int a = 0; a < 3; a++)
    {
      if (entry->operands[a].admet == am_G || /* GPR, selected by reg field (6) */
	  entry->operands[a].admet == am_R || /* GPR, selected by mod field (13)*/
	  entry->operands[a].admet == am_reg || /* implicit register (20)*/
	  entry->operands[a].admet == am_E) /*register or memory location (4) */
	{
	  if (a == 0)
	    {
	      if (entry->opsema == s1R || entry->opsema == s1RW || entry->opsema == s1R2R ||
		  entry->opsema == s1RW2R || entry->opsema == s1RW2RW || entry->opsema == s1RW2R3R ||
		  entry->opsema == s1RW2RW3R)
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,READ_OP);
		}
	      if(entry->opsema == s1W || entry->opsema == s1RW || entry->opsema == s1W2R ||
		      entry->opsema == s1RW2R || entry->opsema == s1RW2RW || entry->opsema == s1W2R3R || 
		      entry->opsema == s1W2W3R || entry->opsema == s1RW2R3R || entry->opsema == s1RW2RW3R ||
		      entry->opsema == s1W2RW3R || entry->opsema == s1W2R3R)
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,WRITE_OP);
		}
	    }
	  else if (a == 1)
	    {
	      if (entry->opsema == s1R2R || entry->opsema == s1W2R || entry->opsema == s1RW2R ||
		  entry->opsema == s1RW2RW || entry->opsema == s1W2R3R || entry->opsema == s1RW2R3R ||
		  entry->opsema == s1RW2RW3R || entry->opsema == s1W2RW3R || entry->opsema == s1W2R3RW)
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,READ_OP);
		}
	      if(entry->opsema == s1RW2RW || entry->opsema == s1W2W3R || entry->opsema == s1W2RW3R ||
		      entry->opsema == s1RW2RW3R )
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,WRITE_OP);
		}
	    }
	  else if (a == 2)
	    {
	      if (entry->opsema == s1W2R3R || entry->opsema == s1W2W3R || entry->opsema == s1W2RW3R ||
		  entry->opsema == s1W2R3RW || entry->opsema == s1RW2R3R || entry->opsema == s1RW2RW3R)
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,READ_OP);
		}
	      if( entry->opsema == s1W2R3RW )
		{
		  parseRegisters(readRegs,writeRegs,&i,&ii,a,WRITE_OP);
		}
	    }
	}
    }
  /*
  printf("\n");

  printf("Read Regs ...");
  for (int a = 0; a < 3; a++)
    {
      printf("%d ", readRegs[a]);
    }
  printf("\nWrite Regs ...");
  
  for (int a = 0; a < 3; a++)
    {
      printf("%d ",writeRegs[a]);
    }
  printf("\n");
  */
}
