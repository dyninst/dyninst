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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "Annotatable.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/arch.h"

#include "BPatch_instruction.h"
#include "BPatch_memoryAccess_NP.h"

#include "parseAPI/src/InstrucIter.h"
#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/src/debug_parse.h"

#include "dyninstAPI/src/legacy-instruction.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

//some more function used to identify the properties of the instruction
/** is the instruction a save instruction
 * @param i the instruction value 
 */
bool InstrucIter::isASaveInstruction()
{
  const instruction i = getInstruction();
  if(((*i).resti.op == RESTop) && ((*i).resti.op3 == SAVEop3))
    return true;
  return false;  
}

/** is the instruction a restore instruction
 * @param i the instruction value 
 */
bool InstrucIter::isARestoreInstruction()
{
  const instruction i = getInstruction();
  if(((*i).resti.op == RESTop) && ((*i).resti.op3 == RESTOREop3))
    return true;
  return false;  
}

/** is the instruction used to return from the functions
 * @param i the instruction value 
 */
bool InstrucIter::isAReturnInstruction()
{
  const instruction i = getInstruction();

  if(((*i).resti.op == 0x2) && ((*i).resti.op3 == 0x38) &&
     ((*i).resti.rd == 0) && ((*i).resti.i == 0x1) &&
     (((*i).resti.rs1 == 0xf) || ((*i).resti.rs1 == 0x1f)) &&
     (((*i).resti.simm13 == 8) || ((*i).resti.simm13 == 12)))
    return true;
  return false;
}

/* The setup functions for works sharing either do a MOV
   with 0x101 (Section) or 0x100 (Do/for) */
bool InstrucIter::isAOMPDoFor()
{
  const instruction i = getInstruction();
  
  if ( ((*i).resti.op3 == 2) &&  ((*i).resti.simm13 == 0x100))
    return true;
  else
    return false;
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
  const instruction i = getInstruction();

  if(((*i).resti.op == 0x2) && ((*i).resti.op3 == 0x38) &&
     ((*i).resti.rd == 0) && ((*i).resti.rs1 != 0xf) && 
     ((*i).resti.rs1 != 0x1f))
  {
    if((!(*i).resti.i && ((*i).restix.rs2 == 0)) ||
       ((*i).resti.i && ((*i).resti.simm13 == 0)))
      return true;
  }
  return false;
}

/** is the instruction a conditional branch instruction 
 * @param i the instruction value 
 */ 
bool InstrucIter::isACondBranchInstruction()
{
  const instruction i = getInstruction();

  if(((*i).branch.op == 0) &&
     ((*i).branch.op2 == 2 || (*i).branch.op2 == 6) &&
     ((*i).branch.cond != 0) && ((*i).branch.cond != 8))
    return true;
  return false;
}


// Use for diagnosing a loop body in OpenMP
bool InstrucIter::isACondBLEInstruction()
{
  const instruction i = getInstruction();
  
  if(((*i).branch.op == 0) &&
     ((*i).branch.op2 == 2 && (*i).branch.cond == 2))
    return true;
  return false;
}

/** is the instruction an unconditional branch instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isAJumpInstruction()
{
  const instruction i = getInstruction();
  
  if(((*i).branch.op == 0) &&
     ((*i).branch.op2 == 2 || (*i).branch.op2 == 6) &&
     ((*i).branch.cond == 8))
    return true;
  return false;
}
/** is the instruction a call instruction 
 * @param i the instruction value 
 */
bool InstrucIter::isACallInstruction()
{
  const instruction i = getInstruction();
  
  if((*i).call.op == 0x1)
    return true;
  return false;
}

bool InstrucIter::isAnneal()
{
  const instruction i = getInstruction();

  if((*i).branch.anneal)
    return true;
  return false;
}

void initOpCodeInfo()
{
  // none needed on SPARC
}

// TODO: maybe figure out a closed form for these functions
const unsigned int fpBytes[][2] = { { 4, 4 }, { 4, 8 }, { 16, 16 }, { 8, 8 } };
const unsigned int intBytes[] = { 4, 1, 2, 8 };
const unsigned int fishyBytes[] = { 0, 1, 8, 4 };

#define btst(x, bit) ((x) & (1<<(bit)))

#define MK_LDi0(bytes, in, rs1, rs2) (new BPatch_memoryAccess(new internal_instruction(i),current, true, false, (bytes), 0, (rs1), (rs2)))
#define MK_STi0(bytes, in, rs1, rs2) (new BPatch_memoryAccess(new internal_instruction(i),current, false, true, (bytes), 0, (rs1), (rs2)))
#define MK_LDi1(bytes, in, rs1, simm13) (new BPatch_memoryAccess(new internal_instruction(i),current, true, false, (bytes), (simm13), (rs1), -1))
#define MK_STi1(bytes, in, rs1, simm13) (new BPatch_memoryAccess(new internal_instruction(i),current, false, true, (bytes), (simm13), (rs1), -1))
#define MK_PFi0(in, rs1, rs2, f) (new BPatch_memoryAccess(new internal_instruction(i),current, false, false, true, 0, (rs1), (rs2), \
                                           0, -1, -1, (f)))
#define MK_PFi1(in, rs1, simm13, f) (new BPatch_memoryAccess(new internal_instruction(i),current, false, false, true, (simm13), (rs1), -1, \
                                              0, -1, -1, (f)))

#define MK_LD(bytes, in) ((**in).rest.i ? MK_LDi1(bytes, in, (**in).resti.rs1, (**in).resti.simm13) : \
                                      MK_LDi0(bytes, in, (**in).rest.rs1, (**in).rest.rs2))
#define MK_ST(bytes, in) ((**in).rest.i ? MK_STi1(bytes, in, (**in).resti.rs1, (**in).resti.simm13) : \
                                      MK_STi0(bytes, in, (**in).rest.rs1, (**in).rest.rs2))
#define MK_MA(bytes, in, load, store) \
          ((**in).rest.i ? \
              new BPatch_memoryAccess(new internal_instruction(i), current,(load), (store), (bytes), (**in).resti.simm13, (**in).resti.rs1, -1) \
              : \
              new BPatch_memoryAccess(new internal_instruction(i), current,(load), (store), (bytes), 0, (**in).rest.rs1, (**in).rest.rs2))
#define MK_PF(in) ((**in).rest.i ? MK_PFi1(in, (**in).resti.rs1, (**in).resti.simm13, (**in).resti.rd) : \
                               MK_PFi0(in, (**in).rest.rs1, (**in).rest.rs2, (**in).rest.rd))


// VG(09/20/01): SPARC V9 decoding after the architecture manual.
BPatch_memoryAccess* InstrucIter::isLoadOrStore()
{
  instruction *i = getInsnPtr();

  if((**i).rest.op != 0x3) // all memory opcodes have op bits 11
    return BPatch_memoryAccess::none;

  unsigned int op3 = (**i).rest.op3;

  if(btst(op3, 4)) { // bit 4 set means alternate space
    if((**i).rest.i) {
      //logLine("SPARC: Alternate space instruction using ASI register currently unhandled...");
      return BPatch_memoryAccess::none;
    }
    else if((**i).rest.unused != ASI_PRIMARY) { // unused is actually imm_asi
      //logLine("SPARC: Alternate space instruction using ASI != PRIMARY currently unhandled...");
      return BPatch_memoryAccess::none;
    }
    // else it is handled below assuming that endianness is big
  }

  if(btst(op3, 5)) { // bit 5 set means dig more
    if(btst(op3, 3)) { // bit 3 set means PREFETCH or CAS(X)
      // Actually CAS(X) is not implemented, it a synthetic
      // instruction that should be coded as CAS(X)A on ASI_P.
      assert(btst(op3,2)); // Catch reserved opcodes
      if(btst(op3, 0)) { // PREFETCH
        assert(!btst(op3, 1)); // Catch reserved opcode
        return MK_PF(i);
      }
      else { // CAS(X)A
        // XXX: the manual seems to have a bug not listed in the errata:
        // it claims that CASA uses the *word* in r[rs1] as address. IMHO
        // the address should always be a doubleword on V9...
        unsigned int b = btst(op3, 1) ? 8 : 4;
        // VG(12/08/01): CAS(X)A uses rs2 as value not address...
        return new BPatch_memoryAccess(new internal_instruction(i), current,true, 
                                       true, b, 0, (**i).resti.rs1, -1);
      }
    }
    else { // bit 3 zero (1u0xyz) means fp memory op
      bool isStore = btst(op3, 2); // bit 2 gives L/S
      // bits 0-1 encode #bytes except for state register ops,
      // where the number of bits is given by bit 0 from rd
      unsigned int b = fpBytes[op3 & 0x3][(**i).rest.rd & 0x1];
      return isStore ? MK_ST(b, i) : MK_LD(b, i);
    }
  }
  else { // bit 5 zero means interger memory op
    // bit 2 almost gives L/S, except LDSTUB (load-store) and SWAP
    // also look like a store. (SWAP is deprecated on V9)
    bool isStore = btst(op3, 2);
    // bit 3 gives signed/unsigned for LOADS, but we ignore that;
    // for stores, it has no clear meaning: there are 5 pure store
    // opcodes, two more that are also loads, and one is reserved.
    // (see p. 269 in manual)
    if(isStore && btst(op3, 3)) { // fishy
      bool isLoad = btst(op3, 0); // SWAP & LDSTUB are both load and store
      unsigned int b = fishyBytes[op3 & 0x3];
      assert(b); // Catch reserved opcode
      return MK_MA(b, i, isLoad, isStore);
    }
    // else simple store, therefore continue
    unsigned int b = intBytes[op3 & 0x3]; // bits 0-1 encode #bytes
    return isStore ? MK_ST(b, i) : MK_LD(b, i);
  }
  return BPatch_memoryAccess::none;
}

/** function which returns the offset of control transfer instructions
 * @param i the instruction value 
 */
Address InstrucIter::getBranchTargetOffset()
{
  const instruction i = getInstruction();

  return i.getOffset();
}

Address InstrucIter::getBranchTargetAddress(bool *) {
  const instruction i = getInstruction();
  return i.getTarget(current);
}

bool InstrucIter::getMultipleJumpTargets(std::set<Address>& result){
  Address oldCurrent = current;
  instruction src = getInstruction();
  while(hasPrev()){
    instruction check = getInstruction();
    // Check if the destination register of the sethi ins matches with the src of the indirect jump :giri 2/14/2007
    if(((*check).sethi.op == 0x0) && 
       ((*check).sethi.op2 == 0x4) &&
       ((*check).sethi.rd == (*src).resti.rs1))
    {
      register signed offset = (*check).sethi.imm22 << 10;
      ++(*this);
      check = getInstruction();
      --(*this);
      if(((*check).resti.op == 0x2) &&
	 ((*check).resti.op3 == 0x2) &&
	 ((*check).resti.i == 0x1)){
	register signed lowData = (*check).resti.simm13 & 0x3ff;
	offset |= lowData;

	while(true) { // Probably should calculate max table size
	  // as on other platforms.
	  void *targetPtr = NULL;
	  if(!instructions_->isValidAddress(offset)) return false;
	  if(!instructions_->isCode(offset)) return false;
	  targetPtr = instructions_->getPtrToInstruction(offset);
		    
	  if (targetPtr == NULL) return false;

	  // This is a horrid way to catch the end of the table;
	  // however, I don't know enough about SPARC to fix it.
		    
	  Address target = *((Address *)targetPtr);
	  bool valid = true;

	  // I've seen this as well, when they pad
	  // jump tables.
      if (target == instructions_->offset()) {
	      valid = false;
	  }
	  else {
	    if (!instructions_->getPtrToInstruction(target))
	      valid = false;
	  }

	  instruction check(target);
	  if (check.valid()) {
	    // Apparently, we've hit the end of the... something.
	    valid = false;
	  }
	  if (target == 0) {
	    // What?
	    valid = false;
	  }
	  if (!valid) {
	    break;
	  }
	  
	  parsing_printf("\t0x%lx => 0x%lx\n", offset, target);
          result.insert(target);
	  offset += instruction::size();
	}

	setCurrentAddress(oldCurrent);
	return !result.empty();
      }
    }
    --(*this);
  }
  setCurrentAddress(oldCurrent);
  return false;
}
bool InstrucIter::delayInstructionSupported(){
  return true;
}

Address InstrucIter::peekPrev() {
  // What about delay slots?
  Address ret = current-instruction::size();
  return ret;
}

Address InstrucIter::peekNext() {
  // Delay slot?
  Address ret = current + instruction::size();
  return ret;
}

void InstrucIter::setCurrentAddress(Address addr){
  current = addr;
  //initializeInsn();
}

instruction InstrucIter::getInstruction(){
  return insn;
}

instruction *InstrucIter::getInsnPtr() {
  instruction *insnPtr = new instruction(insn);
  return insnPtr;
}

// Check to see if we make a stack frame; in Sparc terms,
// execute a save instruction
bool InstrucIter::isStackFramePreamble() {
  assert(instPtr);
  while (!isAReturnInstruction() &&
	 !isACondBranchInstruction() &&
	 !isACallInstruction() &&
	 !isADynamicCallInstruction() &&
	 !isAJumpInstruction() &&
	 insn.valid()) {
    if (insn.isInsnType(SAVEmask, SAVEmatch)) 
      return true;
    (*this)++;
  }
  return false;
}

bool InstrucIter::isADynamicCallInstruction() {
  instruction i = getInstruction();
  return i.isInsnType(CALLImask, CALLImatch);
}

void InstrucIter::getAndSkipDSandAgg(instruction* &ds,
				     instruction* &agg) {
  assert(instPtr);
  instruction insn = getInstruction();
  if (!insn.isDCTI())
    return;
  // Get the next two instructions, by address
  // since we don't know where we are in the bbl.

  ds = NULL;
  agg = NULL;
    
  void *dsPtr = NULL;
  void *aggPtr = NULL;


  assert(instructions_); 
  if (!instructions_->isValidAddress(current + instruction::size())) {
    fprintf(stderr, "Error: addr 0x%x is not valid!\n",
	    instructions_);
  }
  else
  {
    dsPtr = instructions_->getPtrToInstruction(current + instruction::size());
    aggPtr = instructions_->getPtrToInstruction(current + 2*instruction::size());
    

  }


  // Skip delay slot...
  (*this)++;

  assert(dsPtr);
  ds = new instruction(*(unsigned int *)dsPtr);
    
  /* Cases where an unimp 0 actually follows a delay slot */
  if (!aggPtr)
  {
    (*this)++;
    agg = NULL;
    return;
  }      

  agg = new instruction(*(unsigned int *)aggPtr);
  if (!agg->valid()) {
    if ((**agg).raw != 0x0) {
      // Skip aggregate...
      (*this)++;
      return;
    }
  }
  // Otherwise, not what we want.
  delete agg;
  agg = NULL;
  return;
}

bool InstrucIter::isTstInsn()
{
  const instruction i = getInstruction();

  if ((*i).resti.op3 == 18)
    return true;

  return false;
}

bool InstrucIter::isDelaySlot()
{
  assert(instPtr);
  return insn.isDCTI();
}

bool InstrucIter::isFrameSetup()
{
  return false;
}

bool InstrucIter::isALeaveInstruction()
{
  return false;
}

bool InstrucIter::isFramePush()
{
  return false;
}

bool InstrucIter::isAnAllocInstruction()
{
  return false;
}

bool InstrucIter::isAnInterruptInstruction()
{
    // TODO: not implemented
    return false;
}

bool InstrucIter::isAnAbortInstruction()
{
  assert(instPtr);
  return insn.isIllegal();
}

int adjustFPRegNumbers(int reg, int* registers, int i/*next available cell in registers*/, int word_size) {
  if(word_size == SINGLE) {
    registers[i] = reg + FLOAT_OFFSET;
    return i+1;
  }
  else if(word_size == DOUBLE) {
    if(reg < 32) {
      registers[i] = reg + FLOAT_OFFSET;
      registers[i+1] = reg+1 + FLOAT_OFFSET;
      return i+2;
    }
    else {
      registers[i] = reg + FLOAT_OFFSET;
      return i+1;
    }
  }
  else if(word_size == QUAD) {
    if(reg>=32) {
      registers[i] = reg + FLOAT_OFFSET;
      registers[i+1] = reg+2 + FLOAT_OFFSET;
      return i+2;
    }
    else {
      registers[i] = reg + FLOAT_OFFSET;
      registers[i+1] = reg+1 + FLOAT_OFFSET;
      registers[i+2] = reg+2 + FLOAT_OFFSET;
      registers[i+3] = reg+3 + FLOAT_OFFSET;
      return i+4;
    }
  }
  else {
    fprintf(stderr,"Should have never reached here!\n");
    return i;
  }
}

int getRegisterNumber(int n, InsnRegister::RegisterType type) {
  if(type == InsnRegister::FloatReg)
    return n+FLOAT_OFFSET;
  // if GlobalIntReg, CoProcReg, SpecialReg, or NoneReg, return what we have
  return n;
}

void InstrucIter::readWriteRegisters(int* readRegs, int* writeRegs) 
{
   instruction insn = getInstruction();
   insn.get_register_operands();
   std::vector<InsnRegister> *read_regs_p = NULL;
   std::vector<InsnRegister> *write_regs_p = NULL;

   extern AnnotationClass<std::vector<InsnRegister> > RegisterReadSetAnno;

   bool have_read_regs = insn.getAnnotation(read_regs_p, RegisterReadSetAnno);
   bool have_write_regs = insn.getAnnotation(write_regs_p, RegisterReadSetAnno);
#if 0
   Annotatable<InsnRegister, register_read_set_a> &read_regs = insn;
   Annotatable<InsnRegister, register_write_set_a> &write_regs = insn;

  InsnRegister* reads = (InsnRegister*)malloc(sizeof(InsnRegister)*7);//[7];
  InsnRegister* writes = (InsnRegister*)malloc(sizeof(InsnRegister)*5);
  getInstruction().get_register_operands(reads, writes);
#endif

  int c=0;

  if (have_read_regs)
  {
     assert(read_regs_p);
     std::vector<InsnRegister> &read_regs = *read_regs_p;
     unsigned int reads_size = read_regs.size();
     assert(reads_size < 9);

     for (unsigned int i  = 0; i < reads_size; ++i) {

        InsnRegister &read_reg = read_regs[i];
        int regNum = read_reg.getNumber();
        if (regNum == -1)
           break;

        regNum = getRegisterNumber(regNum, read_reg.getType());

        if (regNum != 0) {
           for (unsigned int j=0, wC=read_reg.getWordCount(); j<wC; j++,c++) {
              readRegs[c] = regNum + j;
           }
        }
     }
  }

#if 0
  int c=0;
  int wC;
  int i, j;
  for(i=0; i<7; i++) {
    int regNum = reads[i].getNumber();
    if(regNum != -1) {
      regNum = getRegisterNumber(regNum, reads[i].getType());
      if(regNum != 0)
	for(j=0, wC=reads[i].getWordCount(); j<wC; j++,c++) {
	  readRegs[c] = regNum + j;
	}
    }
    else
      break;
  }
#endif

  c=0;
  if (have_write_regs)
  {
     assert(write_regs_p);
     std::vector<InsnRegister> &write_regs = *write_regs_p;
     unsigned int writes_size = write_regs.size();
     assert(writes_size < 7);

     for (unsigned int i  = 0; i < writes_size; ++i) {

        InsnRegister &write_reg = write_regs[i];
        int regNum = write_reg.getNumber();
        if (regNum == -1)
           break;

        regNum = getRegisterNumber(regNum, write_reg.getType());

        if (regNum != 0) {
           for (unsigned int j=0, wC=write_reg.getWordCount(); j<wC; j++,c++) {
              writeRegs[c] = regNum + j;
           }
        }
     }
  }

#if 0
  c=0;
  for(i=0; i<5; i++) {
     int regNum = writes[i].getNumber();
     if(regNum != -1) {
        regNum = getRegisterNumber(regNum, writes[i].getType());
        if(regNum != 0)
           for(j=0, wC=writes[i].getWordCount(); j<wC; j++,c++) {
	  writeRegs[c] = regNum + j;
	}
    }
    else
      break;
  }
#endif
}

void InstrucIter::adjustRegNumbers(int* readRegs, int* writeRegs,int window) {
  int i=0;
  if(isASaveInstruction()) {
    for(i=0; i<2; i++) {
      if(readRegs[i] <32 && readRegs[i] > 7)
	readRegs[i] += WIN_SIZE*(MAX_SETS - window);
    }
    writeRegs[0] += WIN_SIZE*(MAX_SETS - window-1);
  }
  else if(isARestoreInstruction()) {
    for(i=0; i<2; i++) {
      if(readRegs[i] <32 && readRegs[i] > 7)
	readRegs[i] += WIN_SIZE*(MAX_SETS - window);
    }
    writeRegs[0] += WIN_SIZE*(MAX_SETS - window+1);
  }
  else {
    for(i=0; i<4; i++) {
      if(readRegs[i] <32 && readRegs[i] > 7)
	readRegs[i] += WIN_SIZE*(MAX_SETS - window);
      if(writeRegs[i] <32 && writeRegs[i] > 7)
	writeRegs[i] += WIN_SIZE*(MAX_SETS - window);
    }
  }
}

int InstrucIter::adjustRegNumbers(int regNum, int window) {
  if(regNum <32 && regNum > 7)
    return regNum + WIN_SIZE*(MAX_SETS - window);
  return regNum;
}

bool InstrucIter::isANopInstruction()
{
   return false;
}

bool InstrucIter::isSyscall()
{
  // not implemented
  return false;
}

/*Tail call patterns:********************************************************/
static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isRestore());
}
static inline bool MovCallMovTC(instruction instr, instruction nexti) {
    return (instr.isCall() && nexti.isMovToO7());
}

/*
    Return bool value indicating whether instruction sequence
     found signals tail-call jmp; nop; sequence.  Note that this should 
     NOT include jmpl; nop;, ret; nop;, retl; nop;....

    Current heuristic to detect such sequences :
     look for jmp %reg, nop in function w/ no stack frame, if jmp, nop
     are last 2 instructions, return true (definate TC), at any other point,
     return false (not TC).  Otherwise, return false (no TC).
     w/ no stack frame....
    instr is instruction being examioned.
    nexti is instruction after
    addr is address of <instr>
    func is pointer to function class object describing function
     instructions come from....
 */
static inline bool JmpNopTC(instruction instr, instruction nexti,
                Address /* addr */, Function *func) 
{

    if (!instr.isInsnType(JMPLmask, JMPLmatch)) {
        return 0;
    }

    assert((*instr).resti.op3 == 0x38);

    // only looking for jump instructions which don't overwrite a register
    //  with the PC which the jump comes from (g0 is hardwired to 0, so a write
    //  there has no effect?)....  
    //  instr should have gdb disass syntax : 
    //      jmp  %reg, 
    //  NOT jmpl %reg1, %reg2
    if ((*instr).resti.rd != REG_G(0)) {
        return 0;
    }

    // only looking for jump instructions in which the destination is
    //  NOT %i7 + 8/12/16 or %o7 + 8/12/16 (ret and retl synthetic 
    //  instructions, respectively)
    if ((*instr).resti.i == 1) {
        if ((*instr).resti.rs1 == REG_I(7) || (*instr).resti.rs1 == REG_O(7)) {
        // NOTE : some return and retl instructions jump to {io}7 + 12,
        //  or (io)7 + 16, not + 8, to have some extra space to store the size of a 
        //  return structure....
            if ((*instr).resti.simm13 == 0x8 || (*instr).resti.simm13 == 12 ||
            (*instr).resti.simm13 == 16) {
            return 0;
        }
        }
    }

    // jmp, foloowed by NOP....
    if (!nexti.isNop()) {
        return 0;
    }

    // in function w/o stack frame....
    if (!func->hasNoStackFrame()) {
        return 0;
    }

    /***
     XXX During recursive traversal parsing, it is not possible
         to know in advance the "end" of a function. Computing the
         end here is pointless, as further parsing may move it.
        
         The return value for this method does not matter anyway,
         as tail calls are not dealt with specially on SPARC; eliding
         the following code will only lead to a higher number of
         false-positive TC print statements, but no other ill effect.
         --nater 6/9/2010
  
    // if sequence is detected, but not at end of fn 
    //  (last 2 instructions....), return value indicating possible TC.
    //  This should (eventually) mark the fn as uninstrumenatble....
    if (addr != (func->getEndOffset() - 2*instruction::size())) {
        return 0;
    }
    ***/

    return 1;
}

/****************************************************************************/

// XXX We are currently not doing anything special on SPARC with regards
// to tail calls. Function relocation currently does not do any unwinding
// of tail calls, so this is not terribly important. However, the whole
// notion of what a tail call is at the parsing level and where it should
// be handled with regards to instrumentation needs to be revisited.
//
// Currently, this function returns false regardless of whether the
// instruction matches our tail call heuristics.
bool InstrucIter::isTailCall(Function * f)
{
    Address oldCurrent = current;
    instruction cur = getInstruction();
    ++(*this); 
    instruction next = getInstruction();
    if(CallRestoreTC(cur,next) ||
       MovCallMovTC(cur,next) ||
       JmpNopTC(cur,next,current,f))
    {
        parsing_printf("ERROR: tail call (?) not handled in %s at 0x%x\n",
            f->name().c_str(),current);
        setCurrentAddress(oldCurrent);
        return false;
    }

    setCurrentAddress(oldCurrent);
    return false;
}

bool InstrucIter::isIndirectTailCall(Function * /* f */)
{
    return false;
}

bool InstrucIter::isRealCall(InstructionSource * isrc,
    bool &validTarget,
    bool &simulateJump)
{
    simulateJump = false;
    validTarget = true;

    if(!isADynamicCallInstruction()) {
        Address callTarget = getBranchTargetAddress();
        if(!callTarget) 
            return true;

        if(!isrc->isValidAddress(callTarget)) {
            validTarget = false;
            return true;
        }

        // Test for call->return combo (getpc) 
        codeBuf_t * target = (codeBuf_t *)isrc->getPtrToInstruction(callTarget);
        instruction callTargetInsn;
        callTargetInsn.setInstruction(target);

        if(((*callTargetInsn).raw & 0xfffff000) == 0x81c3e000) {
            parsing_printf("[%s:%d] skipping call to retl at %lx\n",
                FILE__,__LINE__,current);
            return false;
        }
        
        // Test for alternative getpc:
        //  call, nop, <target>
        // or 
        //  call, nop, unimpl, <target> 
        //
        // (Note that we don't actually *test* for the pattern, just
        // for the offset to the target)

        if(callTarget == current + 2*instruction::size() ||
           callTarget == current + 3*instruction::size())
        {
            parsing_printf("[%s:%d] skipping \"get-my-pc\" combo at %lx\n",
                FILE__,__LINE__,current);
            simulateJump = true;
            return false;
        }
    }
    return true;
} 

// power-only
bool InstrucIter::isReturnValueSave() {
    return false;
}
