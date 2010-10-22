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

#include <assert.h>
#include <stdio.h>
#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <map>
#include <string>
#include "common/h/Types.h"
#include "codegen.h"
#include "util.h"
#include "debug.h"

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "emit-x86.h"
#include "process.h"
#include "inst-x86.h"
#include "instructionAPI/h/RegisterIDs.h"
#include "pcrel.h"

#include "dyninstAPI/src/pcrel.h"

using namespace std;
using namespace boost::assign;
using namespace Dyninst::InstructionAPI;

#define MODRM_MOD(x) ((x) >> 6)
#define MODRM_RM(x) ((x) & 7)
#define MODRM_REG(x) (((x) & (7 << 3)) >> 3)
#define MODRM_SET_MOD(x, y) (x) = static_cast<unsigned char>((x) | ((y) << 6))
#define MODRM_SET_RM(x, y) (x) = static_cast<unsigned char>((x) | (y))
#define MODRM_SET_REG(x, y) (x) = static_cast<unsigned char>((x) | ((y) << 3))

#define REX_ISREX(x) (((x) >> 4) == 4)
#define REX_W(x) ((x) & 0x8)
#define REX_R(x) ((x) & 0x4)
#define REX_X(x) ((x) & 0x2)
#define REX_B(x) ((x) & 0x1)

#define REX_INIT(x) ((x) = 0x40)
#define REX_SET_W(x, v) ((x) = static_cast<unsigned char>((x) | ((v) ? 0x8 : 0)))
#define REX_SET_R(x, v) ((x) = static_cast<unsigned char>((x) | ((v) ? 0x4 : 0)))
#define REX_SET_X(x, v) ((x) = static_cast<unsigned char>((x) | ((v) ? 0x2 : 0)))
#define REX_SET_B(x, v) ((x) = static_cast<unsigned char>((x) | ((v) ? 0x1 : 0)))
unsigned copy_prefixes(const unsigned char *&origInsn, unsigned char *&newInsn, unsigned insnType) {
  unsigned nPrefixes = count_prefixes(insnType);

  for (unsigned u = 0; u < nPrefixes; u++)
     *newInsn++ = *origInsn++;
  return nPrefixes;
}

//Copy all prefixes but the Operand-Size and Address-Size prefixes (0x66 and 0x67)
unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn, 
                              unsigned insnType) 
{
  unsigned nPrefixes = count_prefixes(insnType);

  for (unsigned u = 0; u < nPrefixes; u++) {
     if (*origInsn == 0x66 || *origInsn == 0x67)
     {
        origInsn++;
        continue;
     }
     *newInsn++ = *origInsn++;
  }
  return nPrefixes;
}

bool convert_to_rel8(const unsigned char*&origInsn, unsigned char *&newInsn) {
  if (*origInsn == 0x0f)
    origInsn++;

  // We think that an opcode in the 0x8? range can be mapped to an equivalent
  // opcode in the 0x7? range...

  if ((*origInsn >= 0x70) &&
      (*origInsn < 0x80)) {
    *newInsn++ = *origInsn++;
    return true;
  }

  if ((*origInsn >= 0x80) &&
      (*origInsn < 0x90)) {
     *newInsn++ = static_cast<unsigned char>(*origInsn++ - 0x10);
     return true;
  }

  if (0xE0 <= *origInsn || *origInsn <= 0xE3) {
    *newInsn++ = *origInsn++;
    return true;
  }

  // Oops...
  fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
  assert(0 && "Unhandled jump conversion case!");
  return false;
}

bool convert_to_rel32(const unsigned char*&origInsn, unsigned char *&newInsn) {
  if (*origInsn == 0x0f)
    origInsn++;
  *newInsn++ = 0x0f;

  // We think that an opcode in the 0x7? range can be mapped to an equivalent
  // opcode in the 0x8? range...

  if ((*origInsn >= 0x70) &&
      (*origInsn < 0x80)) {
    *newInsn++ = static_cast<unsigned char>(*origInsn++ + 0x10);
    return true;
  }

  if ((*origInsn >= 0x80) &&
      (*origInsn < 0x90)) {
    *newInsn++ = *origInsn++;
    return true;
  }

  // TODO handle loops

  // Oops...
  fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
  assert(0 && "Unhandled jump conversion case!");
  return false;
}


// We keep an array-let that represents various fixed
// insns
unsigned char illegalRep[2] = {0x0f, 0x0b};
unsigned char trapRep[1] = {0xCC};


void insnCodeGen::generateIllegal(codeGen &gen) {
    instruction insn;
    insn.setInstruction(illegalRep);
    generate(gen,insn);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    instruction insn;
    insn.setInstruction(trapRep);
    generate(gen,insn);
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */

void insnCodeGen::generateBranch(codeGen &gen,
                                 Address fromAddr, Address toAddr)
{
  GET_PTR(insn, gen);
  long disp;

  disp = toAddr - (fromAddr + 2);
  if (is_disp8(disp)) {
     *insn++ = 0xEB;
     *((signed char*) insn) = (signed char) disp;
     insn += sizeof(signed char);
     SET_PTR(insn, gen);
     return;
  }
  /*  
  disp = toAddr - (fromAddr + 4);
  if (is_disp16(disp) && gen.addrSpace()->getAddressWidth() != 8) {
     *insn++ = 0x66;
     *insn++ = 0xE9;
     *((signed short*) insn) = (signed short) disp;
     insn += sizeof(signed short);
     SET_PTR(insn, gen);
     return;
  }
  */
  disp = toAddr - (fromAddr + 5);
  if (is_disp32(disp) || gen.addrSpace()->getAddressWidth() == 4) {
     generateBranch(gen, disp);
     return;
  }
  
  if(gen.addrSpace()->getAddressWidth() == 8)
  {
      generateBranch64(gen, toAddr);
  }
  else
  {
      generateBranch32(gen, toAddr);
  }
  return;
}

void insnCodeGen::generateBranch(codeGen &gen,
                                 int disp32)
{
   if (disp32 >= 0)
      assert ((unsigned)disp32 < unsigned(1<<31));
   else
      assert ((unsigned)(-disp32) < unsigned(1<<31));
   GET_PTR(insn, gen);
   *insn++ = 0xE9;
   *((int *)insn) = disp32;
   insn += sizeof(int);
  
   SET_PTR(insn, gen);
   return;
}


// Unified the 64-bit push between branch and call
void insnCodeGen::generatePush64(codeGen &gen, Address val)
{
  GET_PTR(insn, gen);
  for (int i = 3; i >= 0; i--) {
    short word = static_cast<unsigned char>((val >> (16 * i)) & 0xffff);
    *insn++ = 0x66; // operand size override
    *insn++ = 0x68; // push immediate (16-bits b/c of prefix)
    *(short *)insn = word;
    insn += 2;
  }
  SET_PTR(insn, gen);
}

void insnCodeGen::generateBranch64(codeGen &gen, Address to)
{
    // "long jump" - generates sequence to jump to any 64-bit address
    // pushes the value on the stack (using 4 16-bit pushes) the uses a 'RET'

  generatePush64(gen, to);

  GET_PTR(insn, gen);
  *insn++ = 0xC3; // RET
  SET_PTR(insn, gen);

}

void insnCodeGen::generateBranch32(codeGen &gen, Address to)
{
   // "long jump" - generates sequence to jump to any 32-bit address
   emitPushImm(to, gen);
   
   GET_PTR(insn, gen);
   *insn++ = 0xC3; // RET
   SET_PTR(insn, gen);
}

void insnCodeGen::generateCall(codeGen &gen,
                               Address from,
                               Address target)
{
  //assert(target);
  long disp = target - (from + CALL_REL32_SZ);
  
  if (is_disp32(disp)) {
    GET_PTR(insn, gen);
    *insn++ = 0xE8;
    *((int *)insn) = (int) disp;
    insn += sizeof(int);
    SET_PTR(insn, gen);
  }
  else {
    // Wheee....
    // We use a technique similar to our 64-bit
    // branch; push the return addr onto the stack (from),
    // then push to, then return. 
    
    // This looks like
    // A: push D
    // B: push <...>
    // C: return
    // D:

#if defined(arch_x86_64)
    // So we need to know where D is off of "from"
    if(gen.addrSpace()->getAddressWidth() == 8)
    {
        generatePush64(gen, from+CALL_ABS64_SZ);
        generateBranch64(gen, target);
    }
    else
    {
        emitPushImm(from+CALL_ABS32_SZ, gen);
        generateBranch32( gen, target);
    }
#else
    emitPushImm(from+CALL_ABS32_SZ, gen);
    generateBranch32( gen, target);
#endif
  }
}

void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
    // Be more efficient here...
    while (size) {
        GET_PTR(insn, gen);
        *insn++ = NOP;
        SET_PTR(insn, gen);
        size -= sizeof(unsigned char);
    }
}

void insnCodeGen::generate(codeGen &gen,instruction & insn) {
    assert(insn.ptr());
    assert(insn.size());
    memcpy(gen.cur_ptr(), insn.ptr(), insn.size());
    gen.moveIndex(insn.size());
}

pcRelJump::pcRelJump(patchTarget *t, const instruction &i, bool copyPrefixes) :
   pcRelRegion(i),
   addr_targ(0x0),
   targ(t),
   copy_prefixes_(copyPrefixes)
{
}

pcRelJump::pcRelJump(Address target, const instruction &i, bool copyPrefixes) :
   pcRelRegion(i),
   addr_targ(target),
   targ(NULL),
   copy_prefixes_(copyPrefixes)
{
}

Address pcRelJump::get_target() 
{
   if (targ)
      return targ->get_address();
   return addr_targ;
}

pcRelJump::~pcRelJump()
{
}

unsigned pcRelJump::apply(Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   unsigned char *orig_loc;
        
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;
    if (copy_prefixes_) {
   copy_prefixes(origInsn, newInsn, insnType);
    }
   SET_PTR(newInsn, *gen);
    
   insnCodeGen::generateBranch(*gen, addr, get_target());
   REGET_PTR(newInsn, *gen);
   return (unsigned) (newInsn - orig_loc);
}

unsigned pcRelJump::maxSize()
{
   unsigned prefixes = count_prefixes(orig_instruc.type());
#if defined(arch_x86_64)
   if (gen->addrSpace()->getAddressWidth() == 8)
      return prefixes + JUMP_ABS64_SZ;
#endif
   return prefixes + JUMP_SZ;
}

bool pcRelJump::canPreApply()
{
   return gen->startAddr() && (!targ || get_target());
}


pcRelJCC::pcRelJCC(patchTarget *t, const instruction &i) :
   pcRelRegion(i),
   addr_targ(0x0),
   targ(t)
{
}

pcRelJCC::pcRelJCC(Address target, const instruction &i) :
   pcRelRegion(i),
   addr_targ(target),
   targ(NULL)
{
}

Address pcRelJCC::get_target() 
{
   if (targ)
      return targ->get_address();
   return addr_targ;
}

pcRelJCC::~pcRelJCC()
{
}

unsigned pcRelJCC::apply(Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   Address target = get_target();
   Address potential;
   signed long disp;
   codeBufIndex_t start = gen->getIndex();
   GET_PTR(newInsn, *gen);

   copy_prefixes_nosize(origInsn, newInsn, insnType); 
   
   //8-bit jump
   potential = addr + 2;
   disp = target - potential;
   if (is_disp8(disp)) {
      convert_to_rel8(origInsn, newInsn);
      *newInsn++ = (signed char) disp;
      SET_PTR(newInsn, *gen);
      return (unsigned) gen->getIndex() - start;
   }

   //Can't convert short E0-E3 loops/jumps to 32-bit equivalents
   if (*origInsn < 0xE0 || *origInsn > 0xE3) {
     /*
      //16-bit jump
      potential = addr + 5;
      disp = target - potential;
      if (is_disp16(disp) && gen->addrSpace()->getAddressWidth() != 8) {
         *newInsn++ = 0x66; //Prefix to shift 32-bit to 16-bit
         convert_to_rel32(origInsn, newInsn);
         *((signed short *) newInsn) = (signed short) disp;
         newInsn += 2;
         SET_PTR(newInsn, *gen);
         return (unsigned) (newInsn - orig_loc);
      }
     */
      //32-bit jump
      potential = addr + 6;
      disp = target - potential;
      if (is_disp32(disp)) {
         convert_to_rel32(origInsn, newInsn);
         *((signed int *) newInsn) = (signed int) disp;
         newInsn += 4;
         SET_PTR(newInsn, *gen);
         return (unsigned) gen->getIndex() - start;
      }
   }
   
   // We use a three-step branch system that looks like so:
   //   jump conditional <A> 
   // becomes
   //   jump conditional <B>
   //   jump <C>
   //   B: jump <A>
   //   C: ... next insn

   // Moves as appropriate...
   convert_to_rel8(origInsn, newInsn);
   // We now want a 2-byte branch past the branch at B
   *newInsn++ = 2;
   
   // Now for the branch to C - <jumpSize> unconditional branch
   *newInsn++ = 0xEB; 
   SET_PTR(newInsn, *gen);
    // We now want to 1) move forward a byte (the offset we haven't filled
   // in yet) and track that we want to fill it in once we're done.
   codeBufIndex_t jump_to_c_offset_index = gen->getIndex();
   gen->moveIndex(1);
   codeBufIndex_t jump_from_index = gen->getIndex();

   // Original address is a little skewed... 
   // We've moved past the original address (to the tune of nPrefixes + 2 (JCC) + 2 (J))
   Address currAddr = addr + (unsigned) gen->getIndex() - start;
   insnCodeGen::generateBranch(*gen, currAddr, target);
   codeBufIndex_t done = gen->getIndex();

   // Go back and generate the branch _around_ the offset we just calculated
   gen->setIndex(jump_to_c_offset_index);
   REGET_PTR(newInsn, *gen);

   //Go back and fill in the size of the jump at B into the 'jump <C>'
   // The -1 is because 
   *newInsn = gen->getDisplacement(jump_from_index, done);
   SET_PTR(newInsn, *gen);
   gen->setIndex(done);

   return gen->getIndex() - start;
}

unsigned pcRelJCC::maxSize()
{
   unsigned prefixes = count_prefixes(orig_instruc.type());
#if defined(arch_x86_64)
   if (gen->addrSpace()->getAddressWidth() == 8)
      return prefixes + JUMP_ABS64_SZ + 4;
#endif
   return prefixes + 2 + 2 + JUMP_SZ;
}

bool pcRelJCC::canPreApply()
{
   return gen->startAddr() && (!targ || get_target());
}

pcRelCall::pcRelCall(patchTarget *t, const instruction &i) :
   pcRelRegion(i),
   targ_addr(0x0),
   targ(t)
{
}

pcRelCall::pcRelCall(Address target, const instruction &i) :
   pcRelRegion(i),
   targ_addr(target),
   targ(NULL)
{
}

Address pcRelCall::get_target()
{
   if (targ)
      return targ->get_address();
   return targ_addr;
}

pcRelCall::~pcRelCall()
{
}

unsigned pcRelCall::apply(Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   unsigned char *orig_loc;
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;

   copy_prefixes_nosize(origInsn, newInsn, insnType);
   SET_PTR(newInsn, *gen);
   insnCodeGen::generateCall(*gen, addr, get_target());
   REGET_PTR(newInsn, *gen);
   return (unsigned) (newInsn - orig_loc);
}

unsigned pcRelCall::maxSize()
{
   unsigned prefixes = count_prefixes(orig_instruc.type());
#if defined(arch_x86_64)
   if (gen->addrSpace()->getAddressWidth() == 8)
      return prefixes + 2*JUMP_ABS64_SZ;
#endif
   return prefixes + JUMP_REL32_SZ;
}

bool pcRelCall::canPreApply()
{
   return gen->startAddr() && (!targ || get_target());
}

pcRelData::pcRelData(Address a, const instruction &i) :
   pcRelRegion(i),
   data_addr(a)
{
}

#define REL_DATA_MAXSIZE 2/*push r*/ + 10/*movImmToReg64*/ + 7/*orig insn*/ + 2/*pop r*/

#if !defined(arch_x86_64)
unsigned pcRelData::apply(Address) {
   assert(0);
   return 0;
}

#else
unsigned pcRelData::apply(Address addr) 
{
   // We may need to change these from 32-bit relative
   // to 64-bit absolute. This happens with the jumps and calls
   // as well, but it's better encapsulated there.
   
   // We have three options:
   // a) 32-bit relative (AKA "original").
   // b) 32-bit absolute version (where 32-bit relative would fail but we're low)
   // c) 64-bit absolute version
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   unsigned insnSz = orig_instruc.size();   
   bool is_data_abs64 = false;
   unsigned nPrefixes = count_prefixes(insnType);
   signed long newDisp = data_addr - addr;
   unsigned char *orig_loc;
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;

   // count opcode bytes (1 or 2)
   unsigned nOpcodeBytes = 1;
   if (*(origInsn + nPrefixes) == 0x0F)
      nOpcodeBytes = 2;
   
   Register pointer_reg = (Register)-1;
     
   if (!is_disp32(newDisp+insnSz) && !is_addr32(data_addr)) {
      // Case C: replace with 64-bit.
      is_data_abs64 = true;
      unsigned char mod_rm = *(origInsn + nPrefixes + nOpcodeBytes);
      pointer_reg = (mod_rm & 0x38) != 0 ? 0 : 3;
      SET_PTR(newInsn, *gen);
      emitPushReg64(pointer_reg, *gen);
      emitMovImmToReg64(pointer_reg, data_addr, true, *gen);
      REGET_PTR(newInsn, *gen);
   }

   const unsigned char* origInsnStart = origInsn;

   // In other cases, we can rewrite the insn directly; in the 64-bit case, we
   // still need to copy the insn
   copy_prefixes(origInsn, newInsn, insnType);

   if (*origInsn == 0x0F) {
      *newInsn++ = *origInsn++;
   }
     
   // And the normal opcode
   *newInsn++ = *origInsn++;
   
   if (is_data_abs64) {
      // change ModRM byte to use [pointer_reg]: requires
      // us to change last three bits (the r/m field)
      // to the value of pointer_reg
      unsigned char mod_rm = *origInsn++;
      assert(pointer_reg != (Register)-1);
      mod_rm = (mod_rm & 0xf8) + pointer_reg;
      *newInsn++ = mod_rm;
   }
   else if (is_disp32(newDisp+insnSz)) {
      // Whee easy case
      *newInsn++ = *origInsn++;
      // Size doesn't change....
      *((int *)newInsn) = (int)(newDisp - insnSz);
      newInsn += 4;
   }
   else if (is_addr32(data_addr)) {
      assert(!is_disp32(newDisp+insnSz));
      unsigned char mod_rm = *origInsn++;
      
      // change ModRM byte to use SIB addressing (r/m == 4)
      mod_rm = (mod_rm & 0xf8) + 4;
      *newInsn++ = mod_rm;
      
      // SIB == 0x25 specifies [disp32] addressing when mod == 0
      *newInsn++ = 0x25;
      
      // now throw in the displacement (the absolute 32-bit address)
      *((int *)newInsn) = (int)(data_addr);
      newInsn += 4;
   }
   else {
      // Should never be reached...
      assert(0);
   }
   
   // there may be an immediate after the displacement for RIP-relative
   // so we copy over the rest of the instruction here
   origInsn += 4;
   while (origInsn - origInsnStart < (int)insnSz)
      *newInsn++ = *origInsn++;
   
   SET_PTR(newInsn, *gen);
   
   if (is_data_abs64) {
      // Cleanup on aisle pointer_reg...
      assert(pointer_reg != (Register)-1);
      emitPopReg64(pointer_reg, *gen);
   }
   return (unsigned) (newInsn - orig_loc);
}
#endif

unsigned pcRelData::maxSize() 
{
   unsigned prefixes = count_prefixes(orig_instruc.type());
   return REL_DATA_MAXSIZE + prefixes;
}

bool pcRelData::canPreApply()
{
   return (gen->startAddr() != 0x0);
}

bool insnCodeGen::generate(codeGen &gen,
                           instruction & insn,
                           AddressSpace *addrSpace,
                           Address origAddr, // Could be kept in the instruction class.
                           Address newAddr,
                           patchTarget *fallthroughOverride,
                           patchTarget *targetOverride) 
{
   // We grab the maximum space we might need
   GET_PTR(insnBuf, gen);

   /* 
      Relative address instructions need to be modified. The relative address
      can be a 8, 16, or 32-byte displacement relative to the next instruction.
      Since we are relocating the instruction to a different area, we have
      to replace 8 and 16-byte displacements with 32-byte displacements.
       
      All relative address instructions are one or two-byte opcode followed
      by a displacement relative to the next instruction:
       
      CALL rel16 / CALL rel32
      Jcc rel8 / Jcc rel16 / Jcc rel32
      JMP rel8 / JMP rel16 / JMP rel32
       
      The only two-byte opcode instructions are the Jcc rel16/rel32,
      all others have one byte opcode.
       
      The instruction JCXZ/JECXZ rel8 does not have an equivalent with rel32
      displacement. We must generate code to emulate this instruction:
       
      JCXZ rel8
       
      becomes
       
      A0: JCXZ 2 (jump to A4)
      A2: JMP 5  (jump to A9)
      A4: JMP rel32 (relocated displacement)
      A9: ...
       
   */

   const unsigned char *origInsn = insn.ptr();
   unsigned insnType = insn.type();
   unsigned insnSz = insn.size();
   // This moves as we emit code
   unsigned char *newInsn = insnBuf;

   // Check to see if we're doing the "get my PC" via a call
   // We do this first as those aren't "real" jumps.
   if (insn.isCall() && !insn.isCallIndir()) {
      // A possibility...
      // Two types: call(0) (AKA call(me+5));
      // or call to a move/return combo.

      // First, call(me)
      Address target = insn.getTarget(origAddr);
      // Big if tree: "if we go to the next insn"
      // Also could do with an instrucIter... or even have
      // parsing label it for us. 
      // TODO: label in parsing (once)
        
      if (target == (origAddr + insn.size())) {
         if(addrSpace->proc())
         {
            *newInsn = 0x68; // Push; we're replacing "call 0" with "push original IP"
            newInsn++;	  
            Address EIP = origAddr + insn.size();
            unsigned int *temp = (unsigned int *) newInsn;
            *temp = EIP;
            // No 9-byte jumps...
            assert(sizeof(unsigned int) == 4); // should be a compile-time assert
            newInsn += sizeof(unsigned int);
            assert((newInsn - insnBuf) == 5);
            SET_PTR(newInsn, gen);
            goto done;
         }
         else
         {
            *newInsn = 0xE8;
            newInsn++;
            unsigned int *temp = (uint32_t *) newInsn;
            *temp = 0;
            newInsn += sizeof(uint32_t);
            Address offset = origAddr - newAddr;
            *newInsn = 0x81;
            newInsn++;
            *newInsn = 0x04;
            newInsn++;
            *newInsn = 0x24;
            newInsn++;
            temp =  (uint32_t *) newInsn;
            *temp = offset;
            newInsn += sizeof(uint32_t);	  
            assert((newInsn - insnBuf) == 12);
            SET_PTR(newInsn, gen);
            goto done;	  
         }	
      }
      else if (addrSpace->isValidAddress(target)) {
         // Get us an instrucIter
          const unsigned char* buf = reinterpret_cast<const unsigned char*>(addrSpace->getPtrToInstruction(target));
          
          InstructionDecoder d(buf, 2* InstructionDecoder::maxInstructionLength,
			       addrSpace->getAddressWidth() == 8 ?
			       Dyninst::Arch_x86_64 : Dyninst::Arch_x86);
          Instruction::Ptr firstInsn = d.decode();
          Instruction::Ptr secondInsn = d.decode();
          if(firstInsn && firstInsn->getOperation().getID() == e_mov
             && firstInsn->readsMemory() && !firstInsn->writesMemory()
             && secondInsn && secondInsn->getCategory() == c_ReturnInsn)
          {
              // We need to fake this by figuring out the register
            // target (assuming we're moving stack->reg),
            // and constructing an immediate with the value of the
            // original address of the call (+size)
            // This was copied from function relocation code... I 
            // don't understand most of it -- bernat
              const unsigned char *ptr = (const unsigned char*)(firstInsn->ptr());
            unsigned char modrm = *(ptr + 1);
            unsigned char reg = static_cast<unsigned char>((modrm >> 3) & 0x3);
            // Source register... 
            if ((modrm == 0x0c) || (modrm == 0x1c)) {
               // Check source register (%esp == 0x24)
               if ((*(ptr + 2) == 0x24)) {
		 if(addrSpace->proc())
		 {
		   // Okay, put the PC into the 'reg'
		   Address EIP = origAddr + insn.size();
		   *newInsn = static_cast<unsigned char>(0xb8 + reg); // MOV family, destination of the register encoded by
		   // 'reg', source is an Iv immediate
		   newInsn++;
		   unsigned int *temp = (unsigned int *)newInsn;
		   *temp = EIP;
		   //assert(sizeof(unsigned int *)==4);
		   //newInsn += sizeof(unsigned int *);
		   newInsn += 4;  // fix for AMD64
		   SET_PTR(newInsn, gen);
		   goto done;
		 }
		 else
		 {
		   *newInsn = 0xE8;
		   newInsn++;
		   unsigned int *temp = (uint32_t *) newInsn;
		   *temp = 0;
		   newInsn += sizeof(unsigned int);
		   Address offset = origAddr - newAddr;
		   *newInsn = 0x81;
		   newInsn++;
		   *newInsn = 0x04;
		   newInsn++;
		   *newInsn = 0x24;
		   newInsn++;
		   temp =  (uint32_t*) newInsn;
		   *temp = offset;
		   newInsn += sizeof(uint32_t);	  
		   *newInsn = static_cast<unsigned char>(0x58 + reg); // POP family
		   newInsn++;
		   SET_PTR(newInsn, gen);
		   goto done;
		 }
		 
               }
            }
         }
      }
      else {
         parsing_printf("Warning: call at 0x%lx did not have a valid "
                 "calculated target addr 0x%lx\n", origAddr, target);
         /* These need to be debug messages -- a call 0 is common in static binaries
          * fprintf(stderr, "Warning: call at 0x%lx did not have a valid "
          *       "calculated target addr 0x%lx\n", origAddr, target);
          */
      }
   }

   if (!((insnType & REL_B) ||
	 (insnType & REL_W) ||
	 (insnType & REL_D) ||
	 (insnType & REL_D_DATA))) {
     // Normal insn, not addr relative
     for (unsigned u = 0; u < insnSz; u++)
       *newInsn++ = *origInsn++;
     SET_PTR(newInsn, gen);
     goto done;
   }

   // We're addr-relative...
   Address orig_target;
   if (dynamic_cast<toAddressPatch *>(targetOverride)) {
      //targetOverride's address is known now, we'll keep the address around
      // rather than the targetOverride.
      orig_target = targetOverride->get_address();
      targetOverride = NULL;
   }
   else {
      orig_target = origAddr + insn.size() + get_disp(&insn);
   }
   
#if defined(arch_x86_64)
   if (insnType & REL_D_DATA) {
      // Create pcRelRegion that eventually will call pcRelData::apply
      pcRelData *pcr_data = new pcRelData(orig_target, insn);
      assert(pcr_data);
      gen.addPCRelRegion(pcr_data);
      goto done;
   }
#endif
   
   if (insnType & IS_JUMP) {
     // Create pcRelRegion that eventually will call pcRelJump::apply
     pcRelJump *pcr_jump;
     if (targetOverride) {
        pcr_jump = new pcRelJump(targetOverride, insn);
     }
     else {
        pcr_jump = new pcRelJump(orig_target, insn);
     }
     assert(pcr_jump);
     gen.addPCRelRegion(pcr_jump);
     goto done;
   }

   if (insnType & IS_JCC) {
     // Create pcRelRegion that eventually will call pcRelJCC::apply
     pcRelJCC *pcr_jcc;
     if (targetOverride) {
        pcr_jcc = new pcRelJCC(targetOverride, insn);
     }
     else {
        pcr_jcc = new pcRelJCC(orig_target, insn);
     }
     assert(pcr_jcc);
     gen.addPCRelRegion(pcr_jcc);
     goto done;
   }

   if (insnType & IS_CALL) {
     // Create pcRelRegion that eventually will call pcRelCall::apply
     pcRelCall *pcr_call;
     if (targetOverride) {
        pcr_call = new pcRelCall(targetOverride, insn);
     }
     else {
        pcr_call = new pcRelCall(orig_target, insn);
     }
     assert(pcr_call);
     gen.addPCRelRegion(pcr_call);
     goto done;
   }

   //If we get here, then we either didn't handle a case of PC-relative instructions,
   // or we mis-identified an instruction as PC-relative.
   assert(0);
   return false;

 done:
   if (fallthroughOverride) {
       pcRelJump *ft_jump = new pcRelJump(fallthroughOverride, insn, false);
       assert(ft_jump);
       gen.addPCRelRegion(ft_jump);
   }

   return true;
}

#define SIB_SET_REG(x, y) ((x) |= ((y) & 7))
#define SIB_SET_INDEX(x, y) ((x) |= (((y) & 7) << 3))
#define SIB_SET_SS(x, y) ((x) | (((y) & 3) << 6))

/**
 * The comments and naming schemes in this function assume some familiarity with
 * the IA32/IA32e instruction encoding.  If you don't understand this, I suggest
 * you start with Chapter 2 of:
 *  _IA-32 Intel Architecture Software Developer's Manual, Volume 2a_
 * and appendix A of:
 *  _IA-32 Intel Architecture Software Developer's Manual, Volume 2b_
 *
 * This function takes an instruction that accesses memory, and emits a 
 * copy of that instruction that has the load/store replaces with a load/store
 * through a register.  For example, if this function were called with 'loadExpr = r12'
 * on the instruction 'mov 12(%rax)->%rbx', we would emit 'mov (%r12)->%rbx'.
 * Note that we strip off any displacements, indexs, etc...  The register is assumed
 * to contain the final address that will be loaded/stored.
 **/
bool insnCodeGen::generateMem(codeGen &gen,
                              instruction & insn,
                              Address /*origAddr*/, 
                              Address /*newAddr*/,
                              Register loadExpr,
                              Register storeExpr) 
{
   /**********
    * Check parameters
    **********/
   Register newreg = Null_Register;
   if (loadExpr != Null_Register && storeExpr != Null_Register) {
     cerr << "error 1" << endl;
      return false; //Can only do one memory replace per instruction now
   }
   else if (loadExpr == Null_Register && storeExpr == Null_Register)  {
      cerr << "Error in generateMem: loadExpr " << loadExpr << ", storeExpr " << storeExpr << endl;
      assert(0);
     return false; //None specified
   }
   else if (loadExpr != Null_Register && storeExpr == Null_Register) 
      newreg = loadExpr;
   else if (loadExpr == Null_Register && storeExpr != Null_Register) 
      newreg = storeExpr;

   /********************************
    * Section 1.  Read the instruction into our internal data structures.
    ********************************/
   GET_PTR(insnBuf, gen);
   const unsigned char *insn_ptr = insn.ptr();

   struct ia32_memacc memacc[3];
   struct ia32_condition cond;
   struct ia32_locations loc;

   ia32_entry *entry;
   ia32_instruction orig_instr(memacc, &cond, &loc);
   ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION,
               insn_ptr, orig_instr);
   entry = orig_instr.getEntry();

   if (orig_instr.getPrefix()->getPrefix(1) != 0)
      //The instruction accesses memory via segment registers.  Disallow.
     //cerr << "Error: insn uses segment regs" << endl;
     return false;
   if (loc.modrm_position == -1) {
      //Only supporting MOD/RM instructions now
      return false; 
   }

   if (loc.address_size == 1) {
     //Don't support 16-bit instructions yet
     //cerr << "Error: insn is 16-bit" << endl;
     return false;
   }

   if (loc.modrm_reg == 4 && !loc.rex_r) {
     //cerr << "Error: insn uses esp/rsp" << endl;
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      return false;
   }

   if (loc.modrm_mod == 3) {
     //cerr << "Error: insn doesn't use MOD/RM (2)" <<  endl;
      //This instruction doesn't use the MOD/RM to access memory
      return false;
   }
   
   /*********************************
    * Section 2.  We understand the instruction.  Output it
    *********************************/
   int emit_displacement = 0;
   int emit_mod = 0;
   int emit_sib = 0;
   int new_sib = 0;
   
   unsigned char *walker = insnBuf;
   
   /**
    * Handle two special cases, where we emit a memory instruction
    * that loads from/to RBP/R13 or RSP/R12
    **/
   if (newreg == REGNUM_RBP || newreg == REGNUM_R13) {
      //You can't encode rbp or r13 normally i.e. 'mov (%r13)->%rax'
      // Instead we encode using a displacement, so 'mov 0(%r13)->%rax'
      emit_displacement = 1;
      emit_mod = 1; //1 encodes the 0(%reg) format
   }
   
   if (newreg == REGNUM_RSP || newreg == REGNUM_R12) {
      //You can't encode rsp or r12 normally.  We'll emit a SIB instruction.
      // So instead of 'mov (%r12)->%rax' we'll emit 'mov (%r12,0,0)->%rax
      emit_sib = 1;
      SIB_SET_REG(new_sib, newreg & 7);
      SIB_SET_INDEX(new_sib, newreg & 4); //4 encodes to no-index
      //SIB_SET_SS(new_sib, 0); //Gives gcc warning statement w/ no effect
      loc.rex_x = 0; //If we emit a rex, don't extend the index.
   }
   
   /**
    * Emit prefixes
    **/

   //Emit all prefixes except for rex
   for (int i=0; i<loc.num_prefixes; i++) {
      if (i != loc.rex_position)
         *walker++ = insn_ptr[i];
   }

   unsigned char new_rex = 0;
   
   //Emit the rex
   if (loc.rex_position != -1 || (newreg & 8)) {
      //If there was no REX byte, and the high bit of the new register
      // is set, then we'll need to make a rex byte to encode that high bit.
      loc.rex_b = static_cast<unsigned char>(newreg & 8);
      REX_INIT(new_rex);
      REX_SET_W(new_rex, loc.rex_w);
      REX_SET_R(new_rex, loc.rex_r);

      REX_SET_X(new_rex, 0);

      REX_SET_B(new_rex, loc.rex_b); 
   }
   
   /**
    * Copy opcode
    **/
   for (int i=loc.num_prefixes; i<loc.num_prefixes+(int)loc.opcode_size; i++) {
     if ((insn_ptr[i] != 0xf0) &&
	 (insn_ptr[i] != 0xf2) &&
	 (insn_ptr[i] != 0xf3) &&
	 (insn_ptr[i] != 0x2e) &&
	 (insn_ptr[i] != 0x36) &&
	 (insn_ptr[i] != 0x3e) &&
	 (insn_ptr[i] != 0x26) &&
	 (insn_ptr[i] != 0x64) &&
	 (insn_ptr[i] != 0x65) &&
	 (insn_ptr[i] != 0x66) &&
	 (insn_ptr[i] != 0x67)) {
       if (new_rex != 0) {
	 *walker++ = new_rex;
	 new_rex = 0;
       }
     }
     *walker++ = insn_ptr[i];
   }
   
   /**
    * Emit MOD/RM byte
    **/
   unsigned char new_modrm = 0;
   MODRM_SET_MOD(new_modrm, emit_mod); 
   MODRM_SET_RM(new_modrm, newreg & 7); //The new register replacing the memaccess
   // Only the bottom 3 bits go here
   MODRM_SET_REG(new_modrm, loc.modrm_reg); //The bottom old register
   *walker++ = new_modrm;
   
   /**
    * Emit SIB byte
    **/
   if (emit_sib) {
      *walker++ = static_cast<unsigned char>(new_sib);
   }
   
   /**
    * Emit displacement
    **/
   if (emit_displacement) {
      *walker++ = 0x0; //We only need 0 displacements now.
   }
   
   /**
    * Emit immediate
    **/
   for (unsigned i=0; i<loc.imm_size[0]; i++)
   {
     *walker++ = insn_ptr[loc.imm_position[0] + i];
   }

   //Debug output.  Fix the end of testdump.c, compile it, the do an
   // objdump -D
   /*   static FILE *f = NULL;
   if (f == NULL)
   {
      f = fopen("testdump.c", "w+");
      if (!f)
         perror("Couldn't open");
      fprintf(f, "char buffer[] = {\n");
   }
   fprintf(f, "144, 144, 144, 144, 144, 144, 144, 144, 144,\n");
   for (unsigned i=0; i<orig_instr.getSize(); i++) {
      fprintf(f, "%u, ", (unsigned) insn_ptr[i]);
   }
   fprintf(f, "\n");
   for (int i=0; i<(walker-insnBuf); i++) {
      fprintf(f, "%u, ", (unsigned) insnBuf[i]);
   }
   fprintf(f, "\n");
   fprintf(f, "144, 144, 144, 144, 144, 144, 144, 144, 144,\n");*/
   SET_PTR(walker, gen);
   return true;
}

