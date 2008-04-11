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
#include "boost/assign/list_of.hpp"
#include <set>

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

#include "BPatch_instruction.h"
#include "BPatch_memoryAccess_NP.h"

//some more function used to identify the properties of the instruction
/**  the instruction used to return from the functions
 * @param i the instruction value 
 */
bool InstrucIter::isALeaveInstruction()
{
  assert(getInsnPtr());
  return getInstruction().isLeave();
}

bool InstrucIter::isAReturnInstruction()
{
  assert(getInsnPtr());
  return getInstruction().isReturn();
}

/** is the instruction used to return from the functions,
    dependent upon a condition register
    * @param i the instruction value 
    */
bool InstrucIter::isACondReturnInstruction()
{
  return false; // Not implemented yet
}

/** is the instruction an indirect jump instruction 
 * @param i the instruction value 
 */

bool InstrucIter::isAIndirectJumpInstruction()
{
  assert(getInsnPtr());
  if((getInstruction().type() & IS_JUMP) && (getInstruction().type() & INDIR))
  {
    /* since there are two is_jump and indirect instructions
       we are looking for the one with the indirect register
       addressing mode one of which ModR/M contains 4 in its
       reg/opcode field */
    const unsigned char* ptr = getInstruction().op_ptr();
    assert(*ptr == 0xff);
    ptr++;
    if((*ptr & 0x38) == 0x20) 
      return true;
  }
  return false;
}

bool InstrucIter::isStackFramePreamble(int & /*unused*/)
{
  instruction i = getInstruction();
  return ::isStackFramePreamble( i );
}

bool InstrucIter::isFramePush()
{
  assert(getInsnPtr());
  // test for
  // push %ebp (or push %rbp for 64-bit)
  return (getInstruction().size() == 1 && getInstruction().ptr()[0] == 0x55);
}

bool InstrucIter::isFrameSetup()
{
  assert(getInsnPtr());
  //test for
  // movl %esp,%ebp

  // 64-bit:
  // movq %rsp, %rbp

  if (!ia32_is_mode_64()) {
    return (getInstruction().size() == 2 && 
            getInstruction().ptr()[0] == 0x89 && getInstruction().ptr()[1] == 0xe5);
  }
  else {
    return (getInstruction().size() == 3 &&
	    getInstruction().ptr()[0] == 0x48 &&
	    getInstruction().ptr()[1] == 0x89 &&
	    getInstruction().ptr()[2] == 0xe5);
  }
}

/** is the instruction a conditional branch instruction 
 * @param i the instruction value 
 */ 
bool InstrucIter::isACondBranchInstruction()
{
  assert(getInsnPtr());
  if(getInstruction().type() & IS_JCC)
    return true;
  return false;
}

/** is the instruction an unconditional branch instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isAJumpInstruction()
{
  assert(getInsnPtr());
  getInstruction().setInstruction( (unsigned char *)instPtr );
  if((getInstruction().type() & IS_JUMP) &&
     !(getInstruction().type() & INDIR) && 
     !(getInstruction().type() & PTR_WX))
    return true;
  return false;
}

/** is the instruction a call instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isACallInstruction()
{
  assert(getInsnPtr());
  return getInstruction().isCall();
}

bool InstrucIter::isADynamicCallInstruction()
{
  assert(getInsnPtr());
  return getInstruction().isCall() && getInstruction().isIndir();
}

bool InstrucIter::isSyscall() {
    assert(getInsnPtr());
    return getInstruction().isSysCallInsn();
}

bool InstrucIter::isANopInstruction()
{
  assert(getInsnPtr());
  return getInstruction().isNop();
}

bool InstrucIter::isAnAbortInstruction()
{
  assert(getInsnPtr());
  const unsigned char *ptr = getInstruction().op_ptr();

  // FIXME this all needs to be more general!
  // hlt
  return(*ptr == 0xf4 || getInstruction().isIllegal());
}

bool InstrucIter::isAnAllocInstruction()
{
  return false;
}

bool InstrucIter::isAnneal()
{
  return true;
}

bool InstrucIter::isDelaySlot()
{
  return false;
}

/** function which returns the offset of control transfer instructions
 * @param i the instruction value 
 */
Address InstrucIter::getBranchTargetOffset()
{
  assert(getInsnPtr());
  // getTarget returns displacement+address parameter
  return getInstruction().getTarget(0);
}

Address InstrucIter::getBranchTargetAddress(bool *)
{
  assert(getInsnPtr());
  return getInstruction().getTarget(current);
}

void initOpCodeInfo()
{
}

BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  assert(getInsnPtr());
  static unsigned int log2[] = { 0, 0, 1, 1, 2, 2, 2, 2, 3 };
    
  // TODO 16-bit registers
    
  int nac = 0;
    
  ia32_memacc mac[3];
  ia32_condition cnd;
  ia32_instruction i(mac, &cnd);
    
  const unsigned char* addr = getInstruction().ptr();
  BPatch_memoryAccess* bmap = BPatch_memoryAccess::none;
    
  ia32_decode(IA32_DECODE_MEMACCESS|IA32_DECODE_CONDITION, addr, i);
  
  bool first = true;

  for(int j=0; j<3; ++j) {
    ia32_memacc& mac = const_cast<ia32_memacc&>(i.getMac(j));
    const ia32_condition& cond = i.getCond();
    int bmapcond = cond.is ? cond.tttn : -1;
    if(mac.is) {

      // here, we can set the correct address for RIP-relative addressing
      if (mac.regs[0] == mRIP) {
	mac.imm = peekNext() + mac.imm;
      }

      if(first) {
        if(mac.prefetch) {
          if(mac.prefetchlvl > 0) // Intel
            bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchlvl);
          else // AMD
	    bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchstt + IA32AMDprefetch);
        }
        else switch(mac.sizehack) { // translation to pseudoregisters
        case 0:
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					 mac.read, mac.write,
                                         mac.size, mac.imm, mac.regs[0], mac.regs[1], mac.scale, 
                                         bmapcond, mac.nt);
          break;
        case shREP: // use ECX register to compute final size as mac.size * ECX
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
                                         mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, 1 , log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPESCAS:
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNESCAS:
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_NESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPECMPS:
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ECMPS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNECMPS:
	  bmap = new BPatch_memoryAccess(getInsnPtr(), current,
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

// return target addresses from a jump table
// tableInsn - instruction that load address of table entry into a register,
//             the displacement will give us the table's base address
// maxSwitchInsn - compare instrction that does a range check on the
//                 jump table index, gives us number of entries in immediate
// branchInsn - the branch instruction, which we need for the condition code
//              that will help us avoid off-by-one errors in our target
//              calculation
bool InstrucIter::getMultipleJumpTargets(BPatch_Set<Address>& result,
                                         instruction& tableInsn, 
                                         instruction& maxSwitchInsn,
                                         instruction& branchInsn,
                                         bool isAddressInJmp,
					 Address tableOffsetFromThunk)
{ 
  int addrWidth;
  if(!instructions_)
  {
    fprintf(stderr, "InstrucIter::getMultipleJumpTargets() called on invalid iter (no instruction source)\n");
    assert(instructions_);
  }
  
  addrWidth = instructions_->getAddressWidth();

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
  }
  else if( *ptr == 0x3c )
  {
    ptr++;
    maxSwitch = *ptr;
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
    }
  }
    
  if( !maxSwitch )
  {
    result += backupAddress;	
    return false;
  }

  parsing_printf("\tmaxSwitch set to %d\n",maxSwitch);

  const unsigned char * p = branchInsn.op_ptr();
  if( *p == 0x0f ) {
    // skip to second byte of opcode
    p++;
  }
  // Test whether we have a ja or jg; if so, we want to add one to
  // the switch index. The Portland Group compiler generally uses jge,
  // so the value is an accurate count of the entries in the jump table,
  // while gcc uses ja, so the value is one less than the number of entries
  // in the table. *This is assuming the jump table is indexed from zero,
  // which is the only type of jump table we can currently handle*.
  //
  // Fact: we really should be testing whether this is an upper bound
  // before even getting into this code.
  if( (*p & 0x0f) == 0x07 || (*p & 0x0f) == 0x0f ) {
    maxSwitch++;
  } 

    /* XXX addresses into jump tables are computed using some form
       of addressing like the following one:

                movslq (%rdx,%rax,4),%rax

       If working with quadword addresses, the index (in rax) is going
       to be a multiple of 2, not a multiple of 1. So, maxSwitch is actually
       twice as large as it should be.

       HOWEVER: that instruction could also be encoded as
                movslq (%rdx,%rax,8),%rax
       in case of which indexing would be a multiple of 1 again.
       I have never seen this, but it is possible.

       So, this hack reduces the number of jump table entries by 1/2
       when working with quadword addresses. More comprehensive
       analysis should be implemented to figure out whether to
       apply this adjustment or not.

       Questions, see nater@cs
    */
    if(addrWidth == 8) {
        maxSwitch = maxSwitch >> 1;
        parsing_printf("\tmaxSwitch revised to %d\n",maxSwitch);
    }
    
  Address jumpTable = 0;
  ptr = tableInsn.op_ptr();

  if(isAddressInJmp || (!isAddressInJmp && (*ptr == 0x8b)))
  {
    ptr++;

    if(
       ( ((*ptr & 0xc7) == 0x04) &&
	 ( ((*(ptr+1) & 0xc7) == 0x85) || ((*(ptr+1) & 0xc7) == 0xc5) ) ) ||
       ((*ptr & 0xc7) == 0x80) ||
       ((*ptr & 0xc7) == 0x84))
    {
      if((*ptr & 0xc7) == 0x80)
	ptr += 1;
      else
	ptr += 2;
            
      if(isWordAddr) {
	jumpTable |= *(ptr+1);
	jumpTable <<= 8;
	jumpTable |= *ptr;
	//fprintf(stderr,"okay... %lx\n",jumpTable);

      } else {
	jumpTable = *(const int *)ptr;
      }
    }
  }

  parsing_printf("\tjumpTable set to 0x%lx\n",jumpTable);

  if(!jumpTable && !tableOffsetFromThunk)
  {
    result += backupAddress;	
    return false;
  }
  jumpTable += tableOffsetFromThunk;
  parsing_printf("\tjumpTable revised to 0x%lx\n",jumpTable);
  if( !instructions_->isValidAddress(jumpTable) )
  {
    // If the "jump table" has a start address that is outside
    // of the valid range of the binary, we can say with high
    // probability that we have misinterpreted some other
    // construct (such as a function pointer comparison & tail
    // call, for example) as a jump table. Give up now.
    result += backupAddress;
    return false;
  }
  for(unsigned int i=0;i<maxSwitch;i++)
  {
    Address tableEntry = jumpTable + (i * addrWidth);
    int jumpAddress = 0;
  
    
    if(instructions_->isValidAddress(tableEntry))
    {
      if(addrWidth == sizeof(Address))
      {
	jumpAddress = *(const Address *)instructions_->getPtrToInstruction(tableEntry);
      }
      else
      {
	jumpAddress = *(const int *)instructions_->getPtrToInstruction(tableEntry);
      }
    }

    parsing_printf("\tentry %d [0x%lx] -> 0x%x\n",i,tableEntry,jumpAddress);

    if (jumpAddress)
    {
      if(tableOffsetFromThunk)
      {
	jumpAddress += tableOffsetFromThunk;
      }
      result += jumpAddress;
    }
    
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
    return prevInsns.back().first;
  }
  else 
    return 0;
}

Address InstrucIter::peekNext() {
  assert(getInsnPtr());
  Address tmp = current;
  tmp += getInstruction().size();
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

      // Copied from ++(*this) - it was making copies and thus slow.
      prevInsns.push_back(std::make_pair(current, instPtr));
      current = peekNext();
      initializeInsn();
    }
  }
  else if (current > addr) {
    while (current != addr) {
      if (current < addr) {
	current = addr;
	break;
      }
      // See above
      if(hasPrev()) {
          //assert(instructions_ && instructions_->isValidAddress(peekPrev()));
          current = peekPrev();
          instPtr = prevInsns.back().second;
          prevInsns.pop_back();
      }
      
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

bool operandIsRead(int opsema, int operand)
{
  switch(operand)
  {
  case 0:
    return opsema == s1R || opsema == s1RW || opsema == s1R2R ||
    opsema == s1RW2R || opsema == s1RW2RW || opsema == s1RW2R3R ||
    opsema == s1RW2RW3R;
  case 1:
    return opsema == s1R2R || opsema == s1W2R || opsema == s1RW2R ||
    opsema == s1RW2RW || opsema == s1W2R3R || opsema == s1RW2R3R ||
    opsema == s1RW2RW3R || opsema == s1W2RW3R || opsema == s1W2R3RW;
  case 2:
    return opsema == s1W2R3R || opsema == s1W2W3R || opsema == s1W2RW3R ||
    opsema == s1W2R3RW || opsema == s1RW2R3R || opsema == s1RW2RW3R;
  default:
    return false;
  };
}

bool operandIsWritten(int opsema, int operand)
{
  switch(operand)
  {
  case 0:
    return opsema == s1W || opsema == s1RW || opsema == s1W2R ||
    opsema == s1RW2R || opsema == s1RW2RW || opsema == s1W2R3R || 
    opsema == s1W2W3R || opsema == s1RW2R3R || opsema == s1RW2RW3R ||
    opsema == s1W2RW3R || opsema == s1W2R3R;
  case 1:
    return opsema == s1RW2RW || opsema == s1W2W3R || opsema == s1W2RW3R ||
    opsema == s1RW2RW3R;
  case 2:
    return opsema == s1W2R3RW;
  default:
    return false;
  };
}
static RegisterID IntelRegTable[][8] = {
  {
    r_AL, r_CL, r_DL, r_BL, r_AH, r_CH, r_DH, r_BH
  },
  {
    r_AX, r_eCX, r_DX, r_eBX, r_eSP, r_eBP, r_eSI, r_eDI
  },
  {
    r_EAX, r_ECX, r_EDX, r_EBX, r_ESP, r_EBP, r_ESI, r_EDI
  },
  {
    r_ES, r_CS, r_SS, r_DS, r_FS, r_GS, r_Reserved, r_Reserved
  },
  {
    r_R8, r_R9, r_R10, r_R11, r_R12, r_R13, r_R14, r_R15
  }
  
};

RegisterID makeRegisterID(unsigned int intelReg, unsigned int opType, bool is64bitMode, bool isExtendedRegister)
{
  if(is64bitMode && isExtendedRegister)
  {
    return IntelRegTable[4][intelReg];
  }
  
  switch(opType)
  {
  case op_b:
    // since we're doing this for liveness, up-cast to 32-bit registers
    //    return IntelRegTable[0][intelReg];
    {
      if(intelReg < 4 || is64bitMode)
      {
	return IntelRegTable[2][intelReg];
      }
      else
      {
	return IntelRegTable[2][intelReg - 4];
      }
    }
    
  case op_d:
  case op_si:
  case op_w:
  default:
    return IntelRegTable[2][intelReg];
    break;
  }
}

const unsigned int modrm_use_sib = 0x04;
const unsigned int sib_base_only = 0x04;

void addSIBRegisters(std::set<RegisterID>& regs, const ia32_locations& locs, unsigned int opType)
{
  unsigned scale;
  Register index, base;
  decode_SIB(locs.sib_byte, scale, index, base);
  regs.insert(makeRegisterID(base, opType, locs.rex_byte, locs.rex_b));
  if(index != sib_base_only)
  {
    regs.insert(makeRegisterID(index, opType, locs.rex_byte, locs.rex_x));
  }
}


void addModRMRegisters(std::set<RegisterID>& regs, const ia32_locations& locs, unsigned int opType)
{
  if(locs.modrm_rm != modrm_use_sib)
  {
    regs.insert(makeRegisterID(locs.modrm_rm, opType, locs.rex_byte, locs.rex_b));
  }
  else
  {
    addSIBRegisters(regs, locs, opType);
  }
}


void parseRegisters(std::set<RegisterID>& readArray, std::set<RegisterID>& writeArray,
		    ia32_instruction& ii, int opsema)
{
  ia32_entry * entry = ii.getEntry();
  const ia32_locations& locs(ii.getLocationInfo());
  if(!entry) return;
  for(int i = 0; i < 3; ++i)
  {
    ia32_operand operand = entry->operands[i];
    bool isRead = operandIsRead(opsema, i);
    bool isWritten = operandIsWritten(opsema, i);
    switch(operand.admet)
    {
      // mod r/m byte addressing, mod and r/m fields
    case am_E:
      switch(locs.modrm_mod)
      {
	// modrm_mod is a 2-bit value.  We know that in the 00, 01, and 10 cases, am_E
	// will dereference whatever modrm_mod and modrm_rm dictate, and in the 11 case
	// it will directly access the register they determine.  Therefore, we treat the other
	// three cases as read-only regardless of whether the memory is read/written.
      case 0x03:
	if(isRead) 
	{
	  readArray.insert(makeRegisterID(locs.modrm_rm, operand.optype, locs.rex_byte, locs.rex_b));
	} 
	if(isWritten)
	{
	  writeArray.insert(makeRegisterID(locs.modrm_rm, operand.optype, locs.rex_byte, locs.rex_b));
	}
	break;
      default:
	addModRMRegisters(readArray, locs, operand.optype);
      };
      break;
      // mod r/m byte, reg field, general register
    case am_G:
      if(isRead) 
      {
	readArray.insert(makeRegisterID(locs.modrm_reg, operand.optype, locs.rex_byte, locs.rex_r));
      }
      if(isWritten)
      {
	writeArray.insert(makeRegisterID(locs.modrm_reg, operand.optype, locs.rex_byte, locs.rex_r));
      }
      break;
      // same as am_E, except that we ignore the mod field and assume it's 11b
    case am_M:
        // mod r/m must refer to memory
        // so it must have a mod field of 0, 1, or 2
        // and it will only read the registers used in the effective address calcs
        switch(locs.modrm_mod) {
        case 0x03:
            // technically a can't happen, but don't assert because some punk can
            // do unnatural things
            break;
        default:
            addModRMRegisters(readArray, locs, operand.optype);
            break;
        }
        break;
    case am_R:
      if(isRead) 
      {
	readArray.insert(makeRegisterID(locs.modrm_rm, operand.optype, locs.rex_byte, locs.rex_b));
      } 
      if(isWritten)
      {
	writeArray.insert(makeRegisterID(locs.modrm_rm, operand.optype, locs.rex_byte, locs.rex_b));
      }
      break;
    case am_S:
      // segment register in modRM reg field
      if(isRead) 
      {
	if(locs.rex_r)
	{
	  readArray.insert(IntelRegTable[4][locs.modrm_reg]);
	}
	else
	{
	  readArray.insert(IntelRegTable[3][locs.modrm_reg]);
	}
      }
      if(isWritten) 
      {
	if(locs.rex_r)
	{
	  writeArray.insert(IntelRegTable[4][locs.modrm_reg]);
	}
	else
	{
	  writeArray.insert(IntelRegTable[3][locs.modrm_reg]);
	}
      }
      break;
    case am_W:
        // This could be memory as per the mod r/m byte.  We want to do the same thing as the
        // am_E case, except that if it comes back with "register" we don't care
        // as the FP saves are still done all-or-nothing.
        switch(locs.modrm_mod) {
        case 0x03:
            // working directly with a floating point register
            break;
        default:
            // we read anything that's used in the effective address calcs
            addModRMRegisters(readArray, locs, operand.optype);
            break;
        };
        break;
    case am_X:
      // memory addressed by DS:SI, so we read those regardless of semantics
      readArray.insert(r_DS);
      readArray.insert(r_ESI);
      break;
    case am_Y:
      // memory addressed by ES:DI, so again read those regardless of operand semantics
      readArray.insert(r_ES);
      readArray.insert(r_EDI);
      break;
    case am_reg:
      if(isRead) 
      {
	readArray.insert(RegisterID(operand.optype));
      }
      if(isWritten)
      {
	writeArray.insert(RegisterID(operand.optype));
      }
      break;
    default:
      // do nothing
      break;
    };
  }
}


Address InstrucIter::getCallTarget()
{
  return getInstruction().getTarget(current);
}


bool InstrucIter::isFPWrite()
{
  instruction i = getInstruction();
  ia32_instruction ii;
  
  const unsigned char * addr = i.ptr();
       
  ia32_decode(0, addr,ii);    
  ia32_entry * entry = ii.getEntry();

  assert(entry != NULL);
  
  /* X87 Floating Point Operations ... we don't care about the specifics */
  if (entry->otable == t_coprocEsc)
    return true;

  for ( int a = 0; a <  3; a++)
  {
    if (entry->operands[a].admet == am_P || /*64-bit MMX selected by ModRM reg field */
	entry->operands[a].admet == am_Q || /*64-bit MMX selected by ModRM byte */
	entry->operands[a].admet == am_V || /*128-bit XMM selected by ModRM reg field*/
	entry->operands[a].admet == am_W )  /*128-bit XMM selected by ModRM byte */
    {
      return true;
    }
  }
  return false;
}

#define FPOS 16
void InstrucIter::readWriteRegisters(int* readRegs, int* writeRegs)
{
  // deprecating this; use getAllRegistersUsedAndDefined instead
  // start with doing nothing, then we'll go around removing it and converting the call sites and stuff
  //assert(0);
}

using namespace boost::assign;

map<RegisterID, Register> reverseRegisterLookup = map_list_of
(r_EAX, REGNUM_RAX)
    (r_ECX, REGNUM_RCX)
    (r_EDX, REGNUM_RDX)
    (r_EBX, REGNUM_RBX)
    (r_ESP, REGNUM_RSP)
    (r_EBP, REGNUM_RBP)
    (r_ESI, REGNUM_RSI)
    (r_EDI, REGNUM_RDI)
    (r_R8, REGNUM_R8)
    (r_R9, REGNUM_R9)
    (r_R10, REGNUM_R10)
    (r_R11, REGNUM_R11)
    (r_R12, REGNUM_R12)
    (r_R13, REGNUM_R13)
    (r_R14, REGNUM_R14)
    (r_R15, REGNUM_R15)
    (r_DummyFPR, REGNUM_DUMMYFPR)
    (r_OF, REGNUM_OF)
    (r_SF, REGNUM_SF)
    (r_ZF, REGNUM_ZF)
    (r_AF, REGNUM_AF)
    (r_PF, REGNUM_PF)
    (r_CF, REGNUM_CF)
    (r_TF, REGNUM_TF)
    (r_IF, REGNUM_IF)
    (r_DF, REGNUM_DF)
    (r_NT, REGNUM_NT)
    (r_RF, REGNUM_RF)
    (r_AH, REGNUM_RAX)
    (r_BH, REGNUM_RBX)
    (r_CH, REGNUM_RCX)
    (r_DH, REGNUM_RDX)
    (r_AL, REGNUM_RAX)
    (r_BL, REGNUM_RBX)
    (r_CL, REGNUM_RCX)
    (r_DL, REGNUM_RDX)
    (r_eAX, REGNUM_RAX)
    (r_eBX, REGNUM_RBX)
    (r_eCX, REGNUM_RCX)
    (r_eDX, REGNUM_RDX)
    (r_AX, REGNUM_RAX)
    (r_DX, REGNUM_RDX)
    (r_eSP, REGNUM_RSP)
    (r_eBP, REGNUM_RBP)
    (r_eSI, REGNUM_RSI)
    (r_eDI, REGNUM_RDI)
    // These are wrong, need to extend to make cmpxch8b work right
    (r_EDXEAX, REGNUM_RAX)
    (r_ECXEBX, REGNUM_RCX)
    (r_CS, REGNUM_IGNORED)
    (r_DS, REGNUM_IGNORED)
    (r_ES, REGNUM_IGNORED)
    (r_FS, REGNUM_IGNORED)
    (r_GS, REGNUM_IGNORED)
    (r_SS, REGNUM_IGNORED)
;

Register dummyConverter(RegisterID toBeConverted)
{
    map<RegisterID, Register>::const_iterator found = 
        reverseRegisterLookup.find(toBeConverted);
    if(found == reverseRegisterLookup.end()) {
        fprintf(stderr, "Register ID %d not found in reverseRegisterLookup!\n", toBeConverted);
        assert(!"Bad register ID");
    }
    return found->second;
}

void InstrucIter::getAllRegistersUsedAndDefined(std::set<Register> &used, 
                                                std::set<Register> &defined)
{
    // We need to map between the internal register encoding and the one expected
    // by codegen.
    std::set<RegisterID> localUsed;
    std::set<RegisterID> localDefined;

    ia32_locations locs;
    ia32_memacc mac[3];
    ia32_condition cond;
  ia32_instruction detailedInsn(mac, &cond, &locs);
  instruction tmp = getInstruction();
  ia32_decode(IA32_FULL_DECODER, tmp.ptr(), detailedInsn);
  ia32_entry * entry = detailedInsn.getEntry();  
  if(entry)
  {
    entry->flagsUsed(localUsed, localDefined);
    unsigned int opsema = entry->opsema & ((1<<FPOS) -1);//0xFF;
    parseRegisters(localUsed, localDefined, detailedInsn, opsema);
  }
  if(isFPWrite())
  {
      localDefined.insert(r_DummyFPR);
  }
  if(detailedInsn.getPrefixCount())
  {
    unsigned char repPrefix = detailedInsn.getPrefix()->getPrefix(RepGroup);
    switch(repPrefix)
    {
    case PREFIX_REPNZ:
      localUsed.insert(r_ZF);
      // fall through
    case PREFIX_REP:
      localUsed.insert(r_DF);
      localUsed.insert(r_ECX);
      localDefined.insert(r_ECX);
      break;
    default:
      break;
    }
  }
  std::transform(localUsed.begin(), localUsed.end(), inserter(used, used.begin()),
                 dummyConverter);
  std::transform(localDefined.begin(), localDefined.end(), inserter(defined, defined.begin()),
                 dummyConverter);


}
