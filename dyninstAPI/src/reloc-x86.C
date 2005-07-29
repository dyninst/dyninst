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

/*
 * inst-x86.C - x86 dependent functions and code generator
 * $Id: reloc-x86.C,v 1.1 2005/07/29 19:23:06 bernat Exp $
 */

/* x86 */

#include <iomanip>

#include <limits.h>
#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/instPoint.h" 
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/instP.h" // class returnInstance
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "InstrucIter.h"

// for function relocation
//#include "dyninstAPI/src/func-reloc.h" 
#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/LocalAlteration-x86.h"

#include <sstream>

class ExpandInstruction;
class InsertNops;

void emitJump(unsigned disp32, unsigned char *&insn);

// Create a buffer of x86 instructon objects. These x86 instructions will
// contain pointers to the machine code 

#if 0
bool int_function::loadCode()
{
  // Check if already done
  if (code_) return true;

  assert(0 && "Probably not what we want");

  unsigned type, insnSize;
  instruction *insn;
  
  Address firstAddress = getAddress();
  
#ifdef DEBUG_FUNC_RELOC 
  cerr << "int_function::loadCode" << endl;
#endif
  
  code_ = (void *) new unsigned char[getSize()];

  // copy function to be relocated from application into instructions
  if (!proc()->readDataSpace((caddr_t)firstAddress, getSize(), code_, true))
    fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
  
   // first address of function
  unsigned char *p = (unsigned char *) code_;

  // last address of function
  unsigned end_of_function = (unsigned)(p + getSize());
  
  // iterate over all instructions in function
  while ( (unsigned) p < end_of_function ) {
      // new instruction object 
      insnSize = get_instruction(p, type);
      insn = new instruction(p, type, insnSize);   
      instructions.push_back(insn);

      /*
      // check for the following instruction sequence:
      //
      //  call (0)       (PC relative address, where the target of call (0)
      //                  is the next instruction
      //  pop  %ebx      (pops return address of call instruction off of the
      //                  stack and places it in the ebx reg. The value in
      //                  ebx becomes the address of the pop instruction
      //
      // This sequence is used to get the address of the currently 
      // executing instruction. Presently we don't relocate a function 
      // with this sequence of instructions 

      // A call instruction whose target is the next instruction, is generally
      // used to obtain the address of the next instruction. 
      // Presently we don't relocate a functions with such calls
      if ( isTrueCallInsn((const instruction)(*insn)) && 
      get_disp(insn) == 0 && *(p + insnSize) == 0x5b ) {

      needs_relocation_ = false;

      delete insn;
      return false;
      }
      */

      // update p so it points to the next machine code instruction
      p = p + insnSize; 
   }

   // Occasionally a function's size is not calculated correctly for 
   // int_function. In such cases, the last few bytes in the function
   // are "garbage", and the parsing done in the above while loop
   // interprets those bytes as an instruction. If the last byte of that 
   // "garbage" instruction is outside the bounds of the function, 
   // the sum of the insnSizes of the insn's that were parsed above will be 
   // greater than the size of the function being relocating. To keep the
   // sum of the insnSizes equal to the size of the int_function, we replace 
   // the "garbage" bytes with nop instructions, and ignore drop those bytes
   // that are outside of the function.   

   // # bytes of "garbage" at the end of the function 
   int garbage = (unsigned)p - end_of_function;

   // if "garbage" bytes are found
   if (garbage) { 
  
      // create a nop machine instruction
      unsigned char *nop = new unsigned char;
      emitSimpleInsn(0x90, nop);
      nop--;

      // create an x86 instruction for a nop
      insn = new instruction(nop, 0, 1);

      // replace "garbage" x86 instruction with a nop instruction
      delete instructions[instructions.size() - 1];
      instructions[instructions.size() - 1] = insn;
      
      // replace all "garbage" bytes up to the end of the function with nops
      for (int i = 0; i < garbage; i++) {   
	insn = new instruction(nop, 0, 1);
	instructions.push_back(insn);
      }  
      delete nop;
   }

   // buffer of x86 instructions
   numInstructions = instructions.size();

   return true;
} 
#endif

/****************************************************************************/
/****************************************************************************/

// Copy machine code from one location (in mutator) to another location
// (also in the mutator)
// Also updates the corresponding buffer of x86 instructions 
#if 0

void int_function::copyInstruction(instruction &newInsn, instruction &oldInsn, 
				   unsigned &codeOffset) {
  
  // Not sure what to make of this function... 
  assert(0);
#if 0
  unsigned insnSize = oldInsn.size(); 
  const unsigned char *oldPtr = oldInsn.ptr();
  unsigned tmp = codeOffset;

  // iterate over each byte of the machine instruction, copying it
  for (unsigned i = 0; i < insnSize; i++) {     
    relocatedCode[codeOffset] = *(oldPtr + i);
    codeOffset++;
  }

  // update x86 instruction corresponding to machine code instruction
  newInsn = *(new instruction(&relocatedCode[tmp], oldInsn.type(), insnSize));
#endif
}   
#endif

/****************************************************************************/
/****************************************************************************/

// update displacement of expanded instruction

#if 0
int int_function::expandInstructions(LocalAlterationSet &alteration_set, 
                                    instruction &insn, 
                                    Address offset,
                                    instruction &newCodeInsn)
{
   int oldDisp = 0, newDisp = 0, extra_offset = 0;
   unsigned char *oldInsn = 0, *newInsn = 0;
 
   unsigned insnType = insn.type(); 

   // location (in mutator) instruction was originally located at
   oldInsn = const_cast<unsigned char *> (insn.ptr());

   // location (in mutator) instruction is being relocated to (temporarily)
   newInsn = const_cast<unsigned char *> (newCodeInsn.ptr());

   // old displacement from instruction to target
   oldDisp = get_disp(&insn);

   // change in displacement of target  
   extra_offset = alteration_set.getShift(offset + oldDisp) - 
      alteration_set.getShift(offset);

   if (insnType & REL_B) {
      /* replace with rel32 instruction, opcode is one byte. */
      if (*oldInsn == JCXZ) {
         *newInsn++ = *oldInsn; *newInsn++ = 2; // jcxz 2
         *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
         *newInsn++ = 0xE9;                        // jmp rel32

         // change in insn size is 7
         *((int *)newInsn) = oldDisp + extra_offset - 7;

         newInsn += sizeof(int);
         return true;
      } else {
         unsigned newSz=UINT_MAX;
         if (insnType & IS_JCC) {
            /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
               0x0F followed by (old opcode + 16) */
            unsigned char opcode = *oldInsn++;
            *newInsn++ = 0x0F;
            *newInsn++ = opcode + 0x10;
            newDisp = oldDisp + extra_offset - 4;   // change in insn size is 4
            newSz = 6;
         } else {
            if (insnType & IS_JUMP) {
               /* change opcode to 0xE9 */
               oldInsn++;
               *newInsn++ = 0xE9;
               newDisp = oldDisp + extra_offset - 3;  // change in insn size= 3
               newSz = 5;
            }
            assert(newSz!=UINT_MAX);
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int); 
            return true;
         }
      }
   } else {
      if (insnType & REL_W) {
         assert(insnType & PREFIX_OPR);
         if (insnType & PREFIX_SEG)
            *newInsn++ = *oldInsn++;
	
         /* opcode is unchanged, just relocate the displacement */
   
         if (*oldInsn == (unsigned char)0x0F)
            *newInsn++ = *oldInsn++;
         *newInsn++ = *oldInsn++;
         newDisp = oldDisp + extra_offset - 1;  // change in insn size is 1
         *((int *)newInsn) = newDisp;
         newInsn += sizeof(int);
         return true;
      } else {
         // should never get here
         assert (insnType & REL_D);
         assert (0); 
      }
   }
   return false;   
}
#endif

/****************************************************************************/
/****************************************************************************/

// given the Address adr, calculate the offset in the buffer code[], 
// of the x86 instruction that begins at adr. Return -1 if adr is 
// a byte in the middle of an instruction and not the first byte of 
// the instruction
#if 0
int int_function::getArrayOffset(Address adr, instruction code[]) {  
   unsigned i;
   assert(0);
   // Need to: look up addr in basic block, run an InstrucIter, and
   // grab the insn number.
   // Or, see if we actually need this.

   return 0;
}
#endif
/****************************************************************************/
/****************************************************************************/
// originalOffset:      offset (in bytes) of the machine insn from the 
//                      beginning of the original function
//
// newOffset:           offset (in bytes) of the machine insn from the 
//                      beginning of the relocated and expanded function 
//
// originalArrayOffset: offset (in # of instructions) of the x86 instruction 
//                      corresponding to the instPoint, from the beginning of 
//                      the buffer corresponding to the original function.
//
// newArrayOffset:      offset (in # of instructions) of the x86 instruction 
//                      corresponding to the instPoint, from the beginning of 
//                      the buffer corresponding to the expanded and relocate
//                      function.
//
// adr:                 absolute Address of the instruction in the expanded 
//                      and relocated function.


#define CALC_OFFSETS(ip)						      \
     originalOffset = ((ip->pointAddr() + imageBaseAddr) - mutatee);	      \
     originalArrayOffset = getArrayOffset(originalOffset + mutator, oldCode); \
     if (originalArrayOffset < 0) return false;				      \
     newOffset = originalOffset + alteration_set.getShift(originalOffset);    \
     newArrayOffset = originalArrayOffset +				      \
                      alteration_set.getInstPointShift(originalOffset);	      \
     adr = newAdr + newOffset;
 
/****************************************************************************/
/****************************************************************************/

// update info about instrumentation points
#if 0
bool int_function::fillInRelocInstPoints(int_function *originalFunc)
{  
#if 0
   unsigned retId = 0, callId = 0,arbitraryId = 0;
   int originalOffset, newOffset, originalArrayOffset, newArrayOffset;
   Address adr;
   
   if (!call_points_have_been_checked)
      checkCallPoints();
   
   instPoint *point = 0; 
   
   assert(newAdr);

#ifdef DEBUG_FUNC_RELOC    
   cerr << "fillInRelocInstPoints called for " << prettyName() << endl;
   cerr << std::hex << " mutator = 0x" << mutator << " mutatee = 0x"
        << mutatee << " newAdr = 0x" << std::hex << newAdr << endl;
#endif
   
   Address imageBaseAddr;
   if (!proc->getBaseAddress(owner, imageBaseAddr))
      abort();
   
   alteration_set.Collapse();
  
   //  Add inst point corresponding to func entry....
   //   Assumes function has single entry point  
   if (funcEntry_ != NULL) {      
      //  figure out how far entry inst point is from beginning of function..
      CALC_OFFSETS(funcEntry_)
         
      point = new instPoint(funcEntry_->getID(), this, owner,adr-imageBaseAddr,
                            functionEntry, newCode[newArrayOffset]);
      
#ifdef DEBUG_FUNC_RELOC    
      cerr << std::dec << " added entry point at originalOffset = " 
           << originalOffset << ", newOffset = " << newOffset << endl;
#endif
      
      assert(point != NULL);
      
      int numAddedInstr = 0;
      LocalAlteration *alter =
         alteration_set.getAlterationAtOffset(originalOffset);
      if(alter)
         numAddedInstr = alter->numInstrAddedAfter();
      
      instrAroundPt(point, newCode, funcEntry_->insnsBefore(), 
                    funcEntry_->insnsAfter() + numAddedInstr,
                    EntryPt, newArrayOffset);  
      
      if (location && location == funcEntry_) {
         location = point;
      }
      
      // update reloc_info with new instPoint
      reloc_info->addFuncEntry(point);
      assert(reloc_info->funcEntry());
   }

    // Add inst points corresponding to func exits....
   for(retId=0;retId < funcReturns.size(); retId++)
   {
      CALC_OFFSETS(funcReturns[retId])
         
      unsigned int orig_id = funcReturns[retId]->getID();   
      point = new instPoint(orig_id, this, owner, adr-imageBaseAddr,
                            functionExit, newCode[newArrayOffset]);
      
#ifdef DEBUG_FUNC_RELOC
      cerr << std::dec << " added return point at originalOffset = " 
           << originalOffset << ", newOffset = " << newOffset << endl;
#endif

      assert(point != NULL);
      
      int numAddedInstr = 0;
      LocalAlteration *alter =
         alteration_set.getAlterationAtOffset(originalOffset);
      if(alter)
         numAddedInstr = alter->numInstrAddedAfter();
      
      instrAroundPt(point, newCode,funcReturns[retId]->insnsBefore(), 
                    funcReturns[retId]->insnsAfter() + numAddedInstr,
                    ReturnPt, newArrayOffset);
      
      if (location && (location == funcReturns[retId])) {
         location = point;
      } 
      
      // update reloc_info with new instPoint
      reloc_info->addFuncReturn(point);
   } 

   // Add inst points corresponding to func call sites....
   for(callId=0;callId<calls.size();callId++)
   {
      CALC_OFFSETS(calls[callId])

      unsigned int orig_id = calls[callId]->getID();
      point = new instPoint(orig_id, this, owner, adr-imageBaseAddr, callSite,
                            newCode[newArrayOffset]);
      
#ifdef DEBUG_FUNC_RELOC
      cerr << std::dec << " added call site at originalOffset = "
           << originalOffset << ", newOffset = " << newOffset << endl;
#endif

      assert(point != NULL);
      
      int numAddedInstr = 0;
      LocalAlteration *alter =
         alteration_set.getAlterationAtOffset(originalOffset);
      if(alter) numAddedInstr = alter->numInstrAddedAfter();
      
      instrAroundPt(point, newCode, calls[callId]->insnsBefore(), 
                    calls[callId]->insnsAfter() + numAddedInstr,
                    CallPt, newArrayOffset);
      
      if (location && (location == calls[callId])) {
         location = point;
      }
      
      // update reloc_info with new instPoint
      reloc_info->addFuncCall(point);
   }

   for(arbitraryId=0;arbitraryId < arbitraryPoints.size();arbitraryId++)
   {  
      CALC_OFFSETS(arbitraryPoints[arbitraryId]);

      unsigned int orig_id = arbitraryPoints[arbitraryId]->getID();
      point = new instPoint(orig_id, this, owner, adr-imageBaseAddr,
                            otherPoint, newCode[newArrayOffset], true);

      assert(point != NULL);
      
      int numAddedInstr = 0;
      LocalAlteration *alter =
         alteration_set.getAlterationAtOffset(originalOffset);
      if(alter) numAddedInstr = alter->numInstrAddedAfter();
      
      instrAroundPt(point, newCode,funcReturns[arbitraryId]->insnsBefore(),
                    funcReturns[arbitraryId]->insnsAfter() + numAddedInstr,
                    ReturnPt, newArrayOffset);
      
      if (location && (location == arbitraryPoints[arbitraryId]))
         location = point;
      
      reloc_info->addArbitraryPoint(point);
   }
#endif
  return true;    
}
#endif

/****************************************************************************/
/****************************************************************************/

// returns the number of instructions that function rewriting will insert 
int InsertNops::numInstrAddedAfter() {
    return sizeNopRegion;
}

// size (in bytes) of x86 nop instruction
int InsertNops::sizeOfNop() {
    return 1;
}

/****************************************************************************/
/****************************************************************************/

// Insert nops after the machine instruction pointed to by
// oldInstructions[oldOffset]

bool InsertNops::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                                  Address /* newBaseAdr */, Address &newAdr, 
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,  
                                  int /* newDisp */, unsigned &codeOffset,
                                  unsigned char *code) {
  assert(0);
#if 0
   unsigned char *insn = 0;
   
   // copy the instruction we are inserting nops after into relocatedCode 
   function->copyInstruction(newInstructions[newOffset], 
                             oldInstructions[oldOffset], 
                             codeOffset);
   newOffset++;  
   
   // add nops
   for (int i=0;i<sizeNopRegion;i++) {
      
      // pointer to the machine code
      insn = (unsigned char *)(&code[codeOffset]);
      
      // write nop to relocatedCode
      emitSimpleInsn(0x90, insn);
      // emit simple insn increments insn, so we need to decrement insn
      insn--;  
      
      // add instruction corresponding to nop to buffer of instructions
      newInstructions[newOffset] =
         *(new instruction ((const unsigned char *)insn, 0, 1));
      
      newOffset++;
      codeOffset++;
   }
   oldAdr += oldInstructions[oldOffset].size();
   newAdr += oldInstructions[oldOffset].size() + sizeNopRegion;
   oldOffset++;
#endif   
   return true;
}

/****************************************************************************/
/****************************************************************************/

// expand and relocate a call or jump instruction that uses relative addressing


// oldOffset (offset into old buffer of x6 instructions)
// newOffset (offset into new buffer of x86 instructions)

bool ExpandInstruction::RewriteFootprint(Address /* oldBaseAdr */, 
                                         Address &oldAdr, 
                                         Address /* newBaseAdr */, 
                                         Address &newAdr, 
                                         instruction oldInstructions[], 
                                         instruction newInstructions[], 
                                         int &oldOffset, int &newOffset, 
                                         int newDisp, unsigned &codeOffset,
                                         unsigned char* code)
{
#ifdef DEBUG_FUNC_RELOC
   cerr << "ExpandInstruction::RewriteFootprint" <<endl;
   cerr << " newDisp = " << newDisp << endl;
   cerr << " oldOffset = " << oldOffset << endl;
#endif 
 
   unsigned char *oldInsn = 0; 
   unsigned char *newInsn = (unsigned char *)(&code[codeOffset]);
   const unsigned char *tmpInsn = const_cast<const unsigned char *> (newInsn);

   instruction insn = oldInstructions[oldOffset];
   unsigned oldInsnType = insn.type();
   int oldInsnSize = insn.size();

   int sizeChange = 0;
   bool rtn = false;

   oldInsn = const_cast<unsigned char *> (insn.ptr());

   if (oldInsnType & REL_B) {

      /* replace with rel32 instruction, opcode is one byte. */
      if (*oldInsn == JCXZ) {
         *newInsn++ = *oldInsn; *newInsn++ = 2;   // jcxz 2
         *newInsn++ = 0xEB; *newInsn++ = 5;       // jmp 5
         *newInsn++ = 0xE9;                       // jmp rel32

         *((int *)newInsn) = newDisp;   
         newInsn+= sizeof(int);  // move pointer to end of insn
         sizeChange = 7;
         rtn = true;
      } else {
         unsigned newSz=UINT_MAX;
         if (oldInsnType & IS_JCC) {
            /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
               0x0F followed by (old opcode + 16) */
            unsigned char opcode = *oldInsn++;
            *newInsn++ = 0x0F;
            *newInsn++ = opcode + 0x10;

            sizeChange = 4;
            newSz = 6;
         } else {
            if (oldInsnType & IS_JUMP) {
               /* change opcode to 0xE9 */
               oldInsn++;
               *newInsn++ = 0xE9;

               sizeChange = 3;
               newSz = 5;
            }
         }     
         assert(newSz!=UINT_MAX);
         *((int *)newInsn) = newDisp; 
         newInsn += sizeof(int);
         rtn = true;
      }
   } else {
      if (oldInsnType & REL_W) {
         assert(oldInsnType & PREFIX_OPR);
         if (oldInsnType & PREFIX_SEG)
            *newInsn++ = *oldInsn++;
	
         /* opcode is unchanged, just relocate the displacement */
   
         if (*oldInsn == (unsigned char)0x0F)
            *newInsn++ = *oldInsn++;
         *newInsn++ = *oldInsn++;

         *((int *)newInsn) = newDisp;
         newInsn += sizeof(int);
         sizeChange = 1;
         rtn = true;
      } else {
         // should never get here
         assert (oldInsnType & REL_D);
         assert (0); 
      }
   }

   oldAdr += oldInsnSize;
   newAdr += (oldInsnSize + sizeChange);
   oldOffset++;
   newOffset++;
   codeOffset += (oldInsnSize + sizeChange);

#ifdef DEBUG_FUNC_RELOC
   cerr << "rewrote footprint from " << oldInsnSize << " to "
        << oldInsnSize + sizeChange << endl;  
#endif 

   unsigned newInsnType, newInsnSize;
   newInsnSize = get_instruction(tmpInsn, newInsnType);
   newInstructions[newOffset - 1] =
      *(new instruction(tmpInsn, newInsnType, newInsnSize));

   return rtn;   
}
  
/****************************************************************************/
/****************************************************************************/

// Change call to adr+5 to 
//   push  %eip

bool PushEIP::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                               Address /* newBaseAdr */, Address &newAdr, 
                               instruction oldInstructions[], 
                               instruction newInstructions[], 
                               int &oldInsnOffset, int &newInsnOffset,  
                               int /* newDisp */, unsigned &codeOffset,
                               unsigned char *code) 
{

#ifdef DEBUG_FUNC_RELOC
   cerr << "PushEIP::RewriteFootprint" <<endl;
#endif 

   instruction oldInsn = oldInstructions[oldInsnOffset]; 
   unsigned char *insn = (unsigned char *)(&code[codeOffset]);

   // Push OpCode
   *insn = 0x68;

   // address of instruction following call, in original function location
   *((unsigned int *)(insn+1)) = oldAdr + 5;

   oldAdr += oldInsn.size();  // size of call instruction
   newAdr += 5;               // push is 5 bytes
   oldInsnOffset++;
   newInsnOffset++;
   codeOffset += 5;
 
   // Generate x86 instruction object, and place it in buffer of instructions
   unsigned newInsnType, newInsnSize;
   newInsnSize = get_instruction(const_cast<const unsigned char *> (insn), 
                                 newInsnType);
   newInstructions[newInsnOffset - 1] =
      *(new instruction(const_cast<const unsigned char *> (insn),
                        newInsnType, newInsnSize));

   return true;
}

/****************************************************************************/
/****************************************************************************/

// Change call to adr+5 to 
//   push  (adr+5)
//   mov   (adr+5), %ebx

// This places the address of the instruction after the call on the stack, 
// and then copies that address into the %ebx register
bool PushEIPmov::RewriteFootprint(Address /* oldBaseAdr */, Address &oldAdr, 
                                  Address /* newBaseAdr */, Address &newAdr, 
                                  instruction oldInstructions[], 
                                  instruction newInstructions[], 
                                  int &oldInsnOffset, int &newInsnOffset,  
                                  int /* newDisp */, unsigned &codeOffset,
                                  unsigned char *code) 
{
#ifdef DEBUG_FUNC_RELOC
   cerr << "PushEIPmov::RewriteFootprint" <<endl;
#endif 

   instruction oldInsn = oldInstructions[oldInsnOffset]; 
   unsigned oldInsnSize = oldInsn.size();

   unsigned char *movInsn = (unsigned char *)(&code[codeOffset]);
   unsigned int movSize = 5;

   // Generate mov (adr + 5), %ebx instruction
   // mov OpCode
   *movInsn++ = 0xb8 + dst_reg;

   // address of instruction following call
   *((unsigned int *)movInsn) = (unsigned int)(oldAdr + 5);   
   movInsn--;
 
   // Generate x86 instruction object for the mov and place it in the new 
   // buffer of x86 instruction objects
   unsigned newInsnType, newInsnSize;

   newInsnSize = get_instruction(const_cast<const unsigned char *> (movInsn), 
                                 newInsnType);

   newInstructions[newInsnOffset] = *(new instruction(movInsn, newInsnType,
                                                      newInsnSize));

   // Update offsets and addresses
   oldAdr        += oldInsnSize;        // read oldInsnSize bytes
   newAdr        += movSize;            // wrote movSize bytes
   oldInsnOffset += 1;                  // one old instruction
   newInsnOffset += 1;                  // one new instruction
   codeOffset    += movSize;            // wrote movSize bytes
  
   return true;
}

/****************************************************************************/
/****************************************************************************/

// This adds a jump from the end of the relocated function to the
// byte following the original function.
bool Fallthrough::RewriteFootprint(Address oldBaseAdr, Address &/*oldAdr*/, 
                                  Address /*newBaseAdr*/, Address &newAdr, 
				   instruction * /*oldInstructions[]*/, 
                                  instruction newInstructions[], 
                                  int &oldOffset, int &newOffset,  
				  int /*newDisp*/, unsigned &codeOffset,
                                  unsigned char *code) 
{
#ifdef DEBUG_FUNC_RELOC
  cerr << "Fallthrough::RewriteFootprint on " << function->prettyName() << endl;
#endif 
   
   //Create the jump
   Address origFuncEnd = oldBaseAdr + function->getSize();
   int displacement = origFuncEnd - (newAdr + branchsize_);

#ifdef DEBUG_FUNC_RELOC
   cerr << std::hex <<
     "  jump to " << origFuncEnd << " from " << newAdr <<
     " gives a displacement of " << std::dec << displacement << endl;
#endif

   unsigned char *jmp = code + codeOffset;
   emitJump(displacement, jmp);
   jmp = code + codeOffset;

   unsigned jmpInsnType;
   get_instruction((const unsigned char *) jmp, jmpInsnType);

   //Copy the jump
   newInstructions[newOffset] = instruction(jmp, jmpInsnType, branchsize_);
   codeOffset += branchsize_;
   newOffset++;

   //Update addresses
   newAdr += branchsize_;
   oldOffset++;
   return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Find overlapping instrumentation points 

// Cheap and cheesy sort routine

int basicBlockStartAddrSort(const void *b1, const void *b2) {
#if defined(os_windows)
  // HACK. We aren't exporting _anything_ as const on Windows,
  // so we need to de-constify the BPatch_basicBlocks before we
  // call getRelStart
  BPatch_basicBlock *block1 = const_cast<BPatch_basicBlock *>((const BPatch_basicBlock *)b1);
  BPatch_basicBlock *block2 = const_cast<BPatch_basicBlock *>((const BPatch_basicBlock *)b2);
#else
  const BPatch_basicBlock *block1 = (const BPatch_basicBlock *)b1;
  const BPatch_basicBlock *block2 = (const BPatch_basicBlock *)b2;
#endif

  
  if (block1->getRelStart() < block2->getRelStart()) return -1;
  if (block1->getRelStart() > block2->getRelStart()) return -1;
  return 0;
}

#if 0
bool int_function::PA_expandLoopBlocks(LocalAlterationSet *temp_alteration_set, 
				       process *proc)
{
  // Broken by absolute change. 

  assert(0);
  return true;
}
#endif

/****************************************************************************/
/****************************************************************************/

// Locate jumps with targets inside the footprint of an inst points. 
#if 0
bool int_function::PA_attachBranchOverlaps(
                           LocalAlterationSet *temp_alteration_set, 
                           Address /* baseAddress */, Address firstAddress, 
                           instruction loadedCode[],
                           unsigned numberOfInstructions, int codeSize)  {

#ifdef DEBUG_FUNC_RELOC
   cerr << "int_function::PA_attachBranchOverlaps called" <<endl;
   cerr << " codeSize = " << codeSize << endl;
   cerr << " numberOfInstructions = " << numberOfInstructions << endl;
#endif

   int instr_address;
   int disp, offset;
   instruction instr;
   
   // create and sort vector of instPoints
   pdvector<instPoint*> foo;
   sorted_ips_vector(foo);

   // Iterate over function instruction by instruction....
   for(unsigned i = 0; i < numberOfInstructions; i++) {       
      instr = loadedCode[i];
      instr_address = addressOfMachineInsn(&instr);
     
      // look for branch and call insns whose targets are inside the function.
      if (!branchInsideRange(instr, instr_address, firstAddress, 
                             firstAddress + codeSize) &&
          !trueCallInsideRange(instr, instr_address, firstAddress, 
                               firstAddress + codeSize)) {
         continue;
      } 

#ifdef DEBUG_FUNC_RELOC
      cerr << " branch at " << std::hex << (unsigned) instr_address 
           << " insn offset = "  << instr_address - firstAddress
           << " has target inside range of function" << endl;
#endif  

      disp = get_disp(&instr);

      // target of branch or call instruction 
      Address target = instr_address + disp;

      // Check if target is in the footprint of an inst point....
      instPoint *overlap = find_overlap(foo, target);
      if (overlap == NULL) continue;

      // offset of instruction from the beginning of the function 
      offset = overlap->addr() - firstAddress;

      temp_alteration_set->iterReset();

      // If multiple jumps have their target address within the same 
      // instPoint, we only want to add nops once. To do this we
      // iterate over the known LocalAlterations, checking if any already 
      // are already planning on inserting nops at this instPoint.
      LocalAlteration *alteration = temp_alteration_set->iterNext();
      while (alteration != NULL && alteration->getOffset() < offset) {
         alteration = temp_alteration_set->iterNext();
      }

      if (alteration == NULL || alteration->getOffset() != offset) {
         int shift = overlap->followingAddress() - target;

         InsertNops *nops = new InsertNops(this, offset, shift);
         temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
         cerr << " detected overlap between branch target and inst point"
              << ": offset " << target - firstAddress << " # bytes " 
              << overlap->firstAddress() - target << endl;
         cerr << " adding LocalAlteration" << endl;        
#endif
      }
   }
   return true;
}
#endif
/****************************************************************************/
/****************************************************************************/
#if 0
bool int_function::PA_attachGeneralRewrites(
                                   const image* owner,
                                   LocalAlterationSet *temp_alteration_set, 
                                   Address baseAddress, Address firstAddress,
                                   instruction* loadedCode,
                                   unsigned numInstructions, 
                                   int codeSize)
{
#ifdef DEBUG_FUNC_RELOC
   cerr << "int_function::PA_attachGeneralRewrites" << endl;
   cerr << " baseAddress = " << std::hex << baseAddress << endl;
   cerr << " firstAddress = " << std::hex << firstAddress << endl;
#endif

   int size;
   int offset;
   instruction instr;
   Address instr_address;

   // create and sort vector of instPoints
   pdvector<instPoint*> foo;
   sorted_ips_vector(foo);

   // loop over all consecutive pairs of instPoints
   for (unsigned i=0;i<foo.size();i++) {

      // check if instPoint has enough space for jump
      instPoint *ip = foo[i];
      if (ip->size() < JUMP_REL32_SZ) {
         offset = (ip->pointAddr() + baseAddress) - firstAddress; 

         InsertNops *nops = new InsertNops(this, offset, ip->extraBytes());
         temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
         cerr << "adding LocalAlteration for inserting nops" << endl;
         cerr << "ipAddress = " << std::hex << ip->pointAddr() + baseAddress
              << endl;
         cerr << "offset = "
              << (ip->pointAddr() + baseAddress) - firstAddress << endl;
#endif 
      }
   }

   // address of first instruction in function    
   // FIXME
   instr_address = getOffset() + baseAddress;
 
   // offset of instruction in function
   offset = 0;

   // size of previous instruction in function
   size = 0;

   // Iterate over all instructions looking for calls to adr+5
   for(unsigned j = 0; j < numInstructions; j++) {       
      instr = loadedCode[j];
      instr_address += size;
      // check if instruction is a relative addressed call
      if ( instr.isCall() && !instr.isCallIndir() ) {
         // Look for call to adr+5
         if (instr.getTarget(instr_address) == instr_address + 5) {
#ifdef DEBUG_FUNC_RELOC
            cerr << "adding localAlteration for call to next instruction" 
                 << " at offset " << offset << endl;
#endif              
            PushEIP *eip = new PushEIP(this, offset); 
            temp_alteration_set->AddAlteration(eip);
         }

         // Look for call to:
         //   mov   %esp, %ebx
         //   ret

         int targetAdr = instr.getTarget(instr_address-baseAddress);
         const unsigned char *insnPtr = instr.ptr();      
 
         if (*(insnPtr) == 0xe8) {
            if ( !owner->isValidAddress(targetAdr) ) {
               cerr << "ERROR: " << prettyName() << " has a call at " 
                    << instr_address - baseAddress << " with target "
                    << " outside the application's address space" << endl;
               return false;
            }

            // Get a pointer to the call target
            const unsigned char *target =
               (const unsigned char *)owner->getPtrToInstruction(targetAdr);

            // The target instruction is a mov (mem to reg)
            if (*(target) == 0x8b) {
               unsigned char modrm = *(target + 1);
               unsigned char reg = (modrm >> 3) & 0x3;

               // The source register of the mov is specified by a SIB byte 
               if ((modrm == 0x0c) || (modrm == 0x1c)) {
 
                  // The source register of the mov is the %esp register
                  // (0x24) and the instruction after the mov is a ret
                  // instruction (0xc3)
                  if ( (*(target + 2) == 0x24) && (*(target + 3) == 0xc3)) {

#ifdef DEBUG_FUNC_RELOC
                     cerr << std::hex
                          << "Adding PushEIPmov LocalAlteration at offset " 
                          << offset << " of " << prettyName() << endl;
#endif
                     PushEIPmov *eipMov = new PushEIPmov(this, offset, reg); 
                     temp_alteration_set->AddAlteration(eipMov);
                  }
               }
            }
         }
      }
      // iterated over another instruction
      size = instr.size();
      offset += size;
   }

   /**
    * Check if last instruction could fall through to the next
    * function.  Note that we could be more precise than this,
    * we really want to know if the last basic block falls through
    * and is reachable.
    **/
   instr = loadedCode[numInstructions-1];
   if (!instr.isReturn() && !instr.isUncondJump())
   {
#ifdef DEBUG_FUNC_RELOC
     cerr << std::hex 
	  << "Adding fallthrough LocalAlteration at offset " 
	  << offset << " of " << prettyName() << endl;
#endif
     Fallthrough *fall = new Fallthrough(this, codeSize);
     temp_alteration_set->AddAlteration(fall);
   }
   else
   {
#ifdef DEBUG_FUNC_RELOC
     cerr << "Decided not to add fallthrough for "
	  << prettyName() << endl;
#endif
   }

   return true;
}
#endif
/****************************************************************************/
/****************************************************************************/

// Check if an ExpandInstruction alteration has already been created 
// at an offset, with size, shift.
// This allows us to avoid adding a new ExpandInstruction LocalAlterations for
// an instruction each time we go through discoverAlterations. This prevents
// an infinite looping 

bool alreadyExpanded(int offset, int shift, LocalAlterationSet *alteration_set)
{
   bool already_expanded;
   LocalAlteration *alteration = 0;

   alteration_set->iterReset();

   // find the LocalAlteration at offset  
   do {
      alteration = alteration_set->iterNext();
   } while (alteration != NULL && alteration->getOffset() < offset);
  
   if ((alteration != NULL) && (alteration->getOffset() == offset)) { 
      alteration = dynamic_cast<ExpandInstruction *> (alteration);

      if (alteration == NULL || alteration->getShift() > shift) {
         already_expanded = false;
      }
      else {
         already_expanded = true;
      }
   } else {
      already_expanded = false;
   }
  
   alteration_set->iterReset();
   return already_expanded;
}

/****************************************************************************/
/****************************************************************************/

// It may be that in two different passes over a function, we note 
// two different LocalAlterations at the same offset. This function 
// reconciles any conflict between the LocalAlterations and merges them into 
// a single LocalAlteration, or ignores one of them,

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration)
{
   LocalAlteration *casted_alteration = 0, *casted_tempAlteration = 0; 

#ifdef DEBUG_FUNC_RELOC 
   cerr << "Function fixOverlappingAlterations called" << endl;
#endif

   // assert that there is indeed a conflict  
   assert (alteration->getOffset() == tempAlteration->getOffset()); 

   casted_alteration = dynamic_cast<ExpandInstruction *> (alteration); 
   casted_tempAlteration = dynamic_cast<ExpandInstruction *> (tempAlteration);
   if (casted_alteration != NULL) {
      return alteration;
   } else {
      if (casted_tempAlteration != NULL) {
         return tempAlteration;
      }
   }  
  
   casted_alteration = dynamic_cast<InsertNops *> (alteration); 
   casted_tempAlteration = dynamic_cast<InsertNops *> (tempAlteration); 
   if (casted_alteration && casted_tempAlteration) {
       if (alteration->getShift() >= tempAlteration->getShift()) {
	   return alteration;
       } else {
	   return tempAlteration;
       }
   }
   // All other alterations trump InsertNops
   if (casted_alteration || casted_tempAlteration) {
       return (casted_tempAlteration ? alteration
				     : tempAlteration);
   }

   return NULL;
}


