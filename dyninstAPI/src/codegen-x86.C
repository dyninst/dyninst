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

#include <assert.h>
#include <stdio.h>
#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <map>
#include <string>
#include "common/src/ia32_locations.h"
#include "codegen.h"
#include "util.h"
#include "debug.h"
#include "addressSpace.h"

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "emit-x86.h"
#include "inst-x86.h"
#include "pcrel.h"

#include "StackMod/StackAccess.h"

#include "unaligned_memory_access.h"

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

//Copy all prefixes but the Operand-Size and Dyninst::Address-Size prefixes (0x66 and 0x67)
unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn, 
                              unsigned insnType) 
{
    unsigned retval = 0;
    unsigned nPrefixes = count_prefixes(insnType);

    for (unsigned u = 0; u < nPrefixes; u++) {
        if (*origInsn == 0x66 || *origInsn == 0x67)
        {
            origInsn++;
            continue;
        }
        retval++;
        *newInsn++ = *origInsn++;
    }
    return retval;
}

//Copy all prefixes but the Operand-Size and Dyninst::Address-Size prefixes (0x66 and 0x67)
// Returns the number of bytes copied
unsigned copy_prefixes_nosize_or_segments(const unsigned char *&origInsn, unsigned char *&newInsn, 
                              unsigned insnType) 
{
    unsigned retval = 0;
  unsigned nPrefixes = count_prefixes(insnType);
  if (0 == nPrefixes) {
      return 0;
  }

  // duplicate prefixes are possible and are not identified by count_prefixes
  unsigned nWithDups = 0; 
  while(true) {
     if ((*origInsn) >= 0x64 && (*origInsn) <= 0x67) {
        origInsn++;
     }
     else {
         if (nWithDups >= nPrefixes) {
            break;
         }
         *newInsn++ = *origInsn++;
         retval++;
     }
     nWithDups++;
  }

  return retval;
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


void insnCodeGen::generateIllegal(codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x0f);
    append_memory_as_byte(insn, 0x0b);
    SET_PTR(insn, gen);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xCC);
    SET_PTR(insn, gen);
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */

void insnCodeGen::generateBranch(codeGen &gen,
                                 Dyninst::Address fromAddr, Dyninst::Address toAddr)
{
  GET_PTR(insn, gen);
  long disp;

  disp = toAddr - (fromAddr + 2);
  if (is_disp8(disp)) {
     append_memory_as_byte(insn, 0xEB);
     *((signed char*) insn) = (signed char) disp;
     insn += sizeof(signed char);
     SET_PTR(insn, gen);
     return;
  }
  /*  
  disp = toAddr - (fromAddr + 4);
  if (is_disp16(disp) && gen.addrSpace()->getAddressWidth() != 8) {
     append_memory_as_byte(insn, 0x66);
     append_memory_as_byte(insn, 0xE9);
     *((signed short*) insn) = (signed short) disp;
     insn += sizeof(signed short);
     SET_PTR(insn, gen);
     return;
  }
  */
  disp = toAddr - fromAddr;
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
   // Branches have sizes...
   if (disp32 >= 0)
      assert ((unsigned)disp32 < (unsigned(1)<<31));
   else
      assert ((unsigned)(-disp32) < (unsigned(1)<<31));

   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xE9);

   // 5 for a 5-byte branch.
   append_memory_as(insn, int32_t{disp32 - 5});
  
   SET_PTR(insn, gen);
   return;
}


// Unified the 64-bit push between branch and call
void insnCodeGen::generatePush64(codeGen &gen, Dyninst::Address val)
{
  GET_PTR(insn, gen);
#if 0
  for (int i = 3; i >= 0; i--) {
    uint16_t word = static_cast<unsigned short>((val >> (16 * i)) & 0xffff);
    append_memory_as_byte(insn, 0x66); // operand size override
    append_memory_as_byte(insn, 0x68); // push immediate (16-bits b/c of prefix)
    append_memory_as(insn, uint32_t{word});
  }
#endif
  // NOTE: The size of this generated instruction(+1 for the ret) is stored in CALL_ABS64_SZ
  unsigned int high = static_cast<unsigned int>(val >> 32);
  unsigned int low = static_cast<unsigned int>(val);

  // push the low 4
  append_memory_as_byte(insn, 0x68);
  append_memory_as(insn, uint32_t{low});

  // move the high 4 to rsp+4
  append_memory_as_byte(insn, 0xC7);
  append_memory_as_byte(insn, 0x44);
  append_memory_as_byte(insn, 0x24);
  append_memory_as_byte(insn, 0x04);
  append_memory_as(insn, uint32_t{high});

  SET_PTR(insn, gen);
}

void insnCodeGen::generateBranch64(codeGen &gen, Dyninst::Address to)
{
    // "long jump" - generates sequence to jump to any 64-bit address
    // pushes the value on the stack (using 4 16-bit pushes) the uses a 'RET'

  generatePush64(gen, to);

  GET_PTR(insn, gen);
  append_memory_as_byte(insn, 0xC3); // RET
  SET_PTR(insn, gen);

}

void insnCodeGen::generateBranch32(codeGen &gen, Dyninst::Address to)
{
   // "long jump" - generates sequence to jump to any 32-bit address
   emitPushImm(to, gen);
   
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xC3); // RET
   SET_PTR(insn, gen);
}

void insnCodeGen::generateCall(codeGen &gen,
                               Dyninst::Address from,
                               Dyninst::Address target)
{
  //assert(target);
  long disp = target - (from + CALL_REL32_SZ);
  
  if (is_disp32(disp)) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xE8);
    append_memory_as(insn, static_cast<int32_t>(disp));
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
        append_memory_as_byte(insn, NOP);
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

pcRelJump::pcRelJump(Dyninst::Address target, const instruction &i, bool copyPrefixes) :
   pcRelRegion(i),
   addr_targ(target),
   targ(NULL),
   copy_prefixes_(copyPrefixes)
{
}

Dyninst::Address pcRelJump::get_target()
{
   if (targ)
      return targ->get_address();
   return addr_targ;
}

pcRelJump::~pcRelJump()
{
}

unsigned pcRelJump::apply(Dyninst::Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   unsigned char *orig_loc;
  
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;
   if (copy_prefixes_) {
       addr += copy_prefixes(origInsn, newInsn, insnType);
       // Otherwise we will fail to account for them and
       // generate a branch that is +(# prefix bytes)
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

pcRelJCC::pcRelJCC(Dyninst::Address target, const instruction &i) :
   pcRelRegion(i),
   addr_targ(target),
   targ(NULL)
{
}

Dyninst::Address pcRelJCC::get_target()
{
   if (targ)
      return targ->get_address();
   return addr_targ;
}

pcRelJCC::~pcRelJCC()
{
}

unsigned pcRelJCC::apply(Dyninst::Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   Dyninst::Address target = get_target();
   Dyninst::Address potential;
   signed long disp;
   codeBufIndex_t start = gen->getIndex();
   GET_PTR(newInsn, *gen);

   addr += copy_prefixes_nosize_or_segments(origInsn, newInsn, insnType); 
   

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
         append_memory_as(newInsn, static_cast<int32_t>(disp));
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
   Dyninst::Address currAddr = addr + (unsigned) gen->getIndex() - start;
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

pcRelCall::pcRelCall(Dyninst::Address target, const instruction &i) :
   pcRelRegion(i),
   targ_addr(target),
   targ(NULL)
{
}

Dyninst::Address pcRelCall::get_target()
{
   if (targ)
      return targ->get_address();
   return targ_addr;
}

pcRelCall::~pcRelCall()
{
}

unsigned pcRelCall::apply(Dyninst::Address addr)
{
   const unsigned char *origInsn = orig_instruc.ptr();
   unsigned insnType = orig_instruc.type();
   unsigned char *orig_loc;
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;

   addr += copy_prefixes_nosize(origInsn, newInsn, insnType);
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

pcRelData::pcRelData(Dyninst::Address a, const instruction &i) :
   pcRelRegion(i),
   data_addr(a)
{
}

#define REL_DATA_MAXSIZE 2/*push r*/ + 10/*movImmToReg64*/ + 7/*orig insn*/ + 2/*pop r*/

#if !defined(arch_x86_64)
unsigned pcRelData::apply(Dyninst::Address) {
   assert(0);
   return 0;
}

#else
unsigned pcRelData::apply(Dyninst::Address addr)
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
   if ((*(origInsn + nPrefixes) == 0x0F) && (*(origInsn + nPrefixes + 1) == 0x38 || *(origInsn + nPrefixes + 1) == 0x3A))
   	  nOpcodeBytes = 3;
   
   Dyninst::Register pointer_reg = (Dyninst::Register)-1;
     
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
   addr += copy_prefixes(origInsn, newInsn, insnType);

   if (*origInsn == 0x0F) {
      append_memory_as_byte(newInsn, *origInsn++);
       // 3-byte opcode support
       if (*origInsn == 0x38 || *origInsn == 0x3A) {
           append_memory_as_byte(newInsn, *origInsn++);
       }
   }
     
   // And the normal opcode
   append_memory_as_byte(newInsn, *origInsn++);
   
   if (is_data_abs64) {
      // change ModRM byte to use [pointer_reg]: requires
      // us to change last three bits (the r/m field)
      // to the value of pointer_reg
      unsigned char mod_rm = *origInsn++;
      assert(pointer_reg != (Dyninst::Register)-1);
      mod_rm = (mod_rm & 0xf8) + pointer_reg;
      append_memory_as_byte(newInsn, mod_rm);
   }
   else if (is_disp32(newDisp+insnSz)) {
      // Whee easy case
      append_memory_as_byte(newInsn, *origInsn++);
      // Size doesn't change....
      append_memory_as(newInsn, static_cast<int32_t>(newDisp - insnSz));
   }
   else if (is_addr32(data_addr)) {
      assert(!is_disp32(newDisp+insnSz));
      unsigned char mod_rm = *origInsn++;
      
      // change ModRM byte to use SIB addressing (r/m == 4)
      mod_rm = (mod_rm & 0xf8) + 4;
      append_memory_as_byte(newInsn, mod_rm);
      
      // SIB == 0x25 specifies [disp32] addressing when mod == 0
      append_memory_as_byte(newInsn, 0x25);
      
      // now throw in the displacement (the absolute 32-bit address)
      append_memory_as(newInsn, static_cast<int32_t>(data_addr));
   }
   else {
      // Should never be reached...
      assert(0);
   }
   
   // there may be an immediate after the displacement for RIP-relative
   // so we copy over the rest of the instruction here
   origInsn += 4;
   while (origInsn - origInsnStart < (int)insnSz)
      append_memory_as_byte(newInsn, *origInsn++);
   
   SET_PTR(newInsn, *gen);
   
   if (is_data_abs64) {
      // Cleanup on aisle pointer_reg...
      assert(pointer_reg != (Dyninst::Register)-1);
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

#define SIB_SET_REG(x, y) ((x) |= ((y) & 7))
#define SIB_SET_INDEX(x, y) ((x) |= (((y) & 7) << 3))

/**
 * The comments and naming schemes in this function assume some familiarity with
 * the IA32/IA32e instruction encoding.  If you don't understand this, I suggest
 * you start with Chapter 2 of:
 *  _IA-32 Intel Dyninst::Architecture Software Developer's Manual, Volume 2a_
 * and appendix A of:
 *  _IA-32 Intel Dyninst::Architecture Software Developer's Manual, Volume 2b_
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
                              Dyninst::Address /*origAddr*/,
                              Dyninst::Address /*newAddr*/,
                              Dyninst::Register loadExpr,
                              Dyninst::Register storeExpr)
{
   /**********
    * Check parameters
    **********/
   Dyninst::Register newreg = Null_Register;
   if (loadExpr != Null_Register && storeExpr != Null_Register) {
       cerr << "can't rewrite insn\nerror 1" << endl;
      return false; //Can only do one memory replace per instruction now
   }
   else if (loadExpr == Null_Register && storeExpr == Null_Register)  {
      cerr << "can't rewrite insn\nError in generateMem: loadExpr " << loadExpr << ", storeExpr " << storeExpr << endl;
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
   class ia32_locations loc;

   ia32_instruction orig_instr(memacc, &cond, &loc);
    ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION, insn_ptr, orig_instr, (gen.width() == 8));

   if (orig_instr.getPrefix()->getPrefix(1) != 0) {
	   //The instruction accesses memory via segment registers.  Disallow.
      // That just takes all the fun out of it. Don't disallow, but still skip FS
	   if (orig_instr.getPrefix()->getPrefix(1) == 0x64) {
		   cerr << "Warning: insn at << uses segment regs: " << hex << (int) orig_instr.getPrefix()->getPrefix(1) << endl;
         return false;
      }
   }
   if (loc.modrm_position == -1) {
      //Only supporting MOD/RM instructions now
      return false; // e.g., leave, pushad instruction
   }

   //if (loc.address_size == 1) {
   //  //Don't support 16-bit instructions yet
   //  cerr << "Error: insn is 16-bit" << endl;
   //  return false;
   //}

   if (loc.modrm_reg == 4 && !loc.rex_r && loc.address_size != 1) {
      //cerr << "Error: insn uses esp/rsp" << endl;
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      //return false;
   }

   if (loc.modrm_mod == 3) {
      cerr << "can't rewrite insn\nError: insn doesn't use MOD/RM (2)" <<  endl;
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


bool insnCodeGen::modifyJump(Dyninst::Address targetAddr, NS_x86::instruction &insn, codeGen &gen) {
   Dyninst::Address from = gen.currAddr();

   const unsigned char *origInsn = insn.ptr();
   unsigned insnType = insn.type();
  
   GET_PTR(newInsn, gen);

   from += copy_prefixes(origInsn, newInsn, insnType);
   // Otherwise we will fail to account for them and
   // generate a branch that is +(# prefix bytes)

   SET_PTR(newInsn, gen);
    
   insnCodeGen::generateBranch(gen, from, targetAddr);
   return true;
}

bool insnCodeGen::modifyJcc(Dyninst::Address targetAddr, NS_x86::instruction &insn, codeGen &gen) {
   const unsigned char *origInsn = insn.ptr();
   unsigned insnType = insn.type();
   Dyninst::Address from = gen.currAddr();

   Dyninst::Address potential;
   signed long disp;
   codeBufIndex_t start = gen.getIndex();
   GET_PTR(newInsn, gen);

   from += copy_prefixes_nosize_or_segments(origInsn, newInsn, insnType); 
   

   //8-bit jump
   potential = from + 2;
   disp = targetAddr - potential;
   if (is_disp8(disp)) {
      convert_to_rel8(origInsn, newInsn);
      append_memory_as_byte(newInsn, disp);
      SET_PTR(newInsn, gen);
      return true;
   }

   //Can't convert short E0-E3 loops/jumps to 32-bit equivalents
   if (*origInsn < 0xE0 || *origInsn > 0xE3) {
     /*
      //16-bit jump
      potential = from + 5;
      disp = targetAddr - potential;
      if (is_disp16(disp) && gen.fromSpace()->getAddressWidth() != 8) {
         *newInsn++ = 0x66; //Prefix to shift 32-bit to 16-bit
         convert_to_rel32(origInsn, newInsn);
         *((signed short *) newInsn) = (signed short) disp;
         newInsn += 2;
         SET_PTR(newInsn, gen);
         return true;
      }
     */
      //32-bit jump
      potential = from + 6;
      disp = targetAddr - potential;
      if (is_disp32(disp)) {
         convert_to_rel32(origInsn, newInsn);
         append_memory_as(newInsn, static_cast<int32_t>(disp));
         SET_PTR(newInsn, gen);
         return true;
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
   append_memory_as_byte(newInsn, 2);
   
   // Now for the branch to C - <jumpSize> unconditional branch
   append_memory_as_byte(newInsn, 0xEB);
   SET_PTR(newInsn, gen);
    // We now want to 1) move forward a byte (the offset we haven't filled
   // in yet) and track that we want to fill it in once we're done.
   codeBufIndex_t jump_to_c_offset_index = gen.getIndex();
   gen.moveIndex(1);
   codeBufIndex_t jump_from_index = gen.getIndex();

   // Original address is a little skewed... 
   // We've moved past the original address (to the tune of nPrefixes + 2 (JCC) + 2 (J))
   Dyninst::Address currAddr = from + (unsigned) gen.getIndex() - start;
   insnCodeGen::generateBranch(gen, currAddr, targetAddr);
   codeBufIndex_t done = gen.getIndex();

   // Go back and generate the branch _around_ the offset we just calculated
   gen.setIndex(jump_to_c_offset_index);
   REGET_PTR(newInsn, gen);

   //Go back and fill in the size of the jump at B into the 'jump <C>'
   // The -1 is because 
   append_memory_as_byte(newInsn, gen.getDisplacement(jump_from_index, done));
   SET_PTR(newInsn, gen);
   gen.setIndex(done);
   return true;
}

bool insnCodeGen::modifyCall(Dyninst::Address targetAddr, NS_x86::instruction &insn, codeGen &gen) {
   // If we're within a 32-bit displacement, we reuse the original call. 
   // Otherwise we say "welp, sucks to be us", strip any prefixes, 
   // and do a 64-bit long thang

   const unsigned char *origInsn = insn.ptr();
   unsigned insnType = insn.type();
   codeBufIndex_t cur = gen.getIndex();

   // Let's try copying prefixes

   GET_PTR(newInsn, gen);
   copy_prefixes_nosize(origInsn, newInsn, insnType);
   SET_PTR(newInsn, gen);

   // If we're within 32-bits, then okay; otherwise rewind and use
   // 64-bit
   long disp = (targetAddr - (gen.currAddr() + CALL_REL32_SZ));
   if (is_disp32(disp)) {
      insnCodeGen::generateCall(gen, gen.currAddr(), targetAddr);
      return true;
   }

   // Otherwise suck monkey
   gen.setIndex(cur);
   insnCodeGen::generateCall(gen, gen.currAddr(), targetAddr);
   
   return true;
}

bool insnCodeGen::modifyData(Dyninst::Address targetAddr, instruction &insn, codeGen &gen)
{
    // We may need to change these from 32-bit relative
    // to 64-bit absolute. This happens with the jumps and calls
    // as well, but it's better encapsulated there.

    // We have three options:
    // a) 32-bit relative (AKA "original").
    // b) 32-bit absolute version (where 32-bit relative would fail but we're low)
    // c) 64-bit absolute version
    const unsigned char *origInsn = insn.ptr();
    const unsigned char* origInsnStart = origInsn;
    // unsigned insnType = insn.type();
    unsigned insnSz = insn.size();
    Dyninst::Address from = gen.currAddr();

    bool is_data_abs64 = false;
    signed long newDisp = targetAddr - from;
    GET_PTR(newInsn, gen);

    Dyninst::Register pointer_reg = (Dyninst::Register)-1;

    /******************************************* prefix/opcode ****************/

    ia32_locations loc;
    ia32_instruction instruct(NULL, NULL, &loc);

    /**
     * This information is generated during ia32_decode. To make this faster
     * We are only going to do the prefix and opcode decodings
     */
    if(!ia32_decode_prefixes(origInsn, instruct, gen.width() == 8))
        assert(!"Couldn't decode prefix of already known instruction!\n");

    /* get the prefix count */
    size_t pref_count = instruct.getSize();
    origInsn += pref_count;

    /* Decode the opcode */
    if(ia32_decode_opcode(0, origInsn, instruct, NULL, (gen.width() == 8)) < 0)
        assert(!"Couldn't decode opcode of already known instruction!\n");

    /* Calculate the amount of opcode bytes */
    size_t opcode_len = instruct.getLocationInfo().opcode_size;
    origInsn += opcode_len;

    /* Get the value of the Mod/RM byte */
    unsigned char mod_rm = *origInsn++;

#if defined(arch_x86_64)
    if (!is_disp32(newDisp+insnSz) && !is_addr32(targetAddr)) 
    {
        // Case C: replace with 64-bit.
        // This preamble must come before any writes to newInsn!
        is_data_abs64 = true;
        pointer_reg = (mod_rm & 0x38) != 0 ? 0 : 3;
        SET_PTR(newInsn, gen);
        emitPushReg64(pointer_reg, gen);
        emitMovImmToReg64(pointer_reg, targetAddr, true, gen);
        REGET_PTR(newInsn, gen);
    }
#endif

    /* copy the prefix and opcode bytes */
    memcpy(newInsn, origInsnStart, pref_count + opcode_len);
    newInsn += pref_count + opcode_len;

    if (is_data_abs64)
    {
        // change ModRM byte to use [pointer_reg]: requires
        // us to change last three bits (the r/m field)
        // to the value of pointer_reg

        mod_rm = (mod_rm & 0xf8) | pointer_reg;
        /* Set the new ModR/M byte of the new instruction */
        append_memory_as_byte(newInsn, mod_rm);
    } else if (is_disp32(newDisp + insnSz)) 
    {
        /* Instruction can remain a 32 bit instruction */

        /* Copy the ModR/M byte */
        append_memory_as_byte(newInsn, mod_rm);
        /* Use the new relative displacement */
        append_memory_as(newInsn, static_cast<int32_t>(newDisp - insnSz));
    } else if (is_addr32(targetAddr)) 
    {
        // change ModRM byte to use SIB addressing (r/m == 4)
        mod_rm = (mod_rm & 0xf8) + 4;
        append_memory_as_byte(newInsn, mod_rm);

        // SIB == 0x25 specifies [disp32] addressing when mod == 0
        append_memory_as_byte(newInsn, 0x25);

        // now throw in the displacement (the absolute 32-bit address)
        append_memory_as(newInsn, static_cast<int32_t>(targetAddr));
    } else {
        /* Impossible case */
        assert(0);
    }

    // there may be an immediate after the displacement for RIP-relative
    // so we copy over the rest of the instruction here
    origInsn += 4;
    while (origInsn - origInsnStart < (int)insnSz)
        append_memory_as_byte(newInsn, *origInsn++);

    SET_PTR(newInsn, gen);

#if defined(arch_x86_64)
    if (is_data_abs64) {
        // Cleanup on aisle pointer_reg...
        assert(pointer_reg != (Dyninst::Register)-1);
        emitPopReg64(pointer_reg, gen);
    }
#endif

    return true;
}

bool insnCodeGen::modifyDisp(signed long newDisp, instruction &insn, codeGen &gen, Dyninst::Architecture arch, Dyninst::Address addr) {

    relocation_cerr << "\t\tmodifyDisp "
        << std::hex << addr
        << std::dec << ", newDisp = " << newDisp << endl;

    const unsigned char* origInsn = insn.ptr();
    unsigned insnSz = insn.size();

    InstructionAPI::InstructionDecoder d2(origInsn, insnSz, arch);
    InstructionAPI::Instruction origInsnPtr = d2.decode();

    bool modifyDefinition = false;
    if (!origInsnPtr.readsMemory() && !origInsnPtr.writesMemory()) {
        // This instruction should be a definition
        modifyDefinition = true;
    }

    StackAccess* origAccess;
    signed long origDisp;
    if (!getMemoryOffset(NULL, NULL, origInsnPtr, addr, MachRegister(),
        StackAnalysis::Height(0), StackAnalysis::Definition(),  origAccess,
        arch, modifyDefinition)) {
        assert(0);
    } else {
        origDisp = origAccess->disp();
        relocation_cerr << "\t\tOld displacement: " << std::hex << 
            origDisp << " New: " << newDisp << std::dec << std::endl;
    }

    GET_PTR(newInsn, gen);

    const unsigned char* newInsnStart = newInsn;
    const unsigned char* origInsnStart = origInsn;

    /******************************************* prefix/opcode ****************/
   
    ia32_instruction instruct; 
    /**
     * This information is generated during ia32_decode. To make this faster
     * We are only going to do the prefix and opcode decodings
     */
    if(!ia32_decode_prefixes(origInsn, instruct, false))
        assert(!"Couldn't decode prefix of already known instruction!\n");

    /* get the prefix count */
    size_t pref_count = instruct.getSize();

    /* copy the prefix */
    memcpy(newInsn, origInsn, pref_count);
    newInsn += pref_count;
    origInsn += pref_count;

    /* Decode the opcode */
    if(ia32_decode_opcode(0, origInsn, instruct, NULL, (gen.width() == 8)) < 0)
        assert(!"Couldn't decode opcode of already known instruction!\n");

    /* Calculate the amount of opcode bytes */
    size_t opcode_len = instruct.getSize() - pref_count;

    /* Copy the opcode bytes */
    memcpy(newInsn, origInsn, opcode_len);
    newInsn += opcode_len;
    origInsn += opcode_len;

    /******************************************* modRM *************************/
    // Update displacement size (mod bits in ModRM), if necessary
    int expectedDifference = 0;
    unsigned char modrm = *origInsn++;
    unsigned char modrm_mod = MODRM_MOD(modrm);
    unsigned char modrm_rm = MODRM_RM(modrm);

    relocation_cerr << "\t\tModRM: " << std:: hex <<
        (unsigned int)modrm << " mod: " << (unsigned int)modrm_mod << 
        " rm: " << (unsigned int)modrm_rm
        << std::endl;

    int origDispSize = -1;

    if (origDisp != newDisp) {
        if (modrm_mod == 0) {
            if (modrm_rm == 5) {
                origDispSize = 32;
            } else {
                origDispSize = -1;
            }
        } else if (modrm_mod == 1) {
            origDispSize = 8;
        } else if (modrm_mod == 2) {
            origDispSize = 32;
        } else {
            origDispSize = -1;
        }

        // Switch modrm_mod if necessary
        if (origDispSize == -1) {
            // If we didn't have a displacement before, and we do now, need to handle it!
            if (is_disp8(newDisp)) {
                modrm = modrm + 0x40;
                expectedDifference = 1;
            } else if (is_disp32(newDisp)) {
                modrm = modrm + 0x80;
                expectedDifference = 4;
            }

        } else if (origDispSize == 8) {
            if (is_disp8(newDisp)) {
            } else if (is_disp32(newDisp)) {
                modrm = modrm + 0x40;
                expectedDifference = 3;
            }
        } else if (origDispSize == 32) {
            if (is_disp8(newDisp)) {
                modrm = modrm - 0x40;
                expectedDifference = -3;
            } else if (is_disp32(newDisp)) {
            }
        }
    }

    /******************************************* SIB ***************************/

    // Copy the SIB byte when necessary
    if (modrm_rm == 4) {

        unsigned char sib = *origInsn++;
        unsigned char sib_base = MODRM_RM(sib);

        // Check for displacement in the SIB
        if (sib_base == 5 && modrm_mod == 0) {
            origDispSize = 32;
        }

        // Copy MODRM byte
        append_memory_as_byte(newInsn, modrm);

        // Copy SIB byte
        append_memory_as_byte(newInsn, sib);
    } else {
        // Copy MODRM byte
        append_memory_as_byte(newInsn, modrm);

        // Skip SIB byte
    }

    /********************************* displacement ***************************/

    // Put the displacement back in
    if (origDisp != newDisp) {
        // Replace displacement
        if (is_disp8(newDisp)) {
            append_memory_as_byte(newInsn, newDisp);
        } else if (is_disp32(newDisp)) {
            append_memory_as(newInsn, static_cast<int32_t>(newDisp));
        } else {
            // Should never be reached...
            assert(0);
        }
    }

    if (origDispSize == -1) {
        // Do nothing
    } else if (origDispSize == 8) {
        origInsn += sizeof(uint8_t);
    } else if (origDispSize == 32) {
        origInsn += sizeof(uint32_t);
    } else {
        // Should never be reached
        assert(0);
    }

    /********************************* immedidate *****************************/

    // there may be an immediate after the displacement
    // so we copy over the rest of the instruction here
    while (origInsn - origInsnStart < (int)insnSz) {
        auto nextByte = Dyninst::read_memory_as<uint8_t>(origInsn);
        append_memory_as_byte(newInsn, nextByte);
    }

    /******************************** done ************************************/

    auto newInsnSz = newInsn - newInsnStart;
    InstructionAPI::InstructionDecoder d(newInsnStart, newInsnSz, arch);
    InstructionAPI::Instruction i = d.decode();

    if ((insnSz + expectedDifference) != newInsnSz) {
        relocation_cerr << "\t\tERROR: Old Size: " << std::dec << insnSz << " New size: " << newInsnSz << " Expected size: " << (insnSz + expectedDifference) << std::endl;
        return false;
    }

    // Validate
    StackAccess* newAccess = NULL;
    getMemoryOffset(NULL, NULL, i, addr, MachRegister(),
        StackAnalysis::Height(0), StackAnalysis::Definition(),  newAccess,
        arch, modifyDefinition);
    if (!newAccess) {
        if (newDisp != 0) {
            return false;
        }
    } else {
        if (newAccess->disp() != newDisp){
            return false;
        }
    }

    SET_PTR(newInsn, gen);

    relocation_cerr << "\t\tModify Disp success.\n";
    return true;
}
