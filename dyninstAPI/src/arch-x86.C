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

// $Id: arch-x86.C,v 1.89 2008/09/04 21:06:07 bill Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual (2001 ed.)
//                                 - AMD x86-64 Architecture Programmer's Manual (rev 3.00, 1/2002)
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation

// Note: Unless specified "book" refers to Intel's manual

#include <assert.h>
#include <stdio.h>
#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <map>
#include <string>
#include "common/h/Types.h"
#include "arch.h"
#include "util.h"
#include "debug.h"

#include "InstructionDecoder.h"
#include "Instruction.h"

#include "addressSpace.h"
#include "binaryEdit.h"
#include "process.h"
#include "miniTramp.h"
#include "instPoint.h"
#include "symtab.h"
#include "baseTramp.h"

#include "emit-x86.h"
#include "inst-x86.h"

#include "instructionAPI/h/RegisterIDs.h"
#include "pcrel.h"

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

Address get_immediate_operand(instruction *instr)
{
    ia32_memacc mac[3];
    ia32_condition cond;
    ia32_locations loc;
    ia32_instruction detail(mac,&cond,&loc);

    ia32_decode(IA32_FULL_DECODER,(const unsigned char *)(instr->ptr()),detail);

    // now find the immediate value in the locations
    Address immediate = 0;

    switch(loc.imm_size[0]) {
        case 8:
            immediate = *(const unsigned long*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 4:
            immediate = *(const unsigned int*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 2:
            immediate = *(const unsigned short*)(instr->ptr()+loc.imm_position[0]);
            break;
        case 1:
            immediate = *(const unsigned char*)(instr->ptr()+loc.imm_position[0]);
            break;
        default:
            parsing_printf("%s[%u]:  invalid immediate size %d in insn\n",
                FILE__,__LINE__,loc.imm_size[0]);
            break;
    }

    return immediate;
}

// get the displacement of a relative jump or call

int get_disp(instruction *insn) {
  return displacement(insn->ptr(), insn->type());
}

int count_prefixes(unsigned insnType) {
  unsigned nPrefixes = 0;
  if (insnType & PREFIX_OPR)
    nPrefixes++;
  if (insnType & PREFIX_SEG)
    nPrefixes++;
  if (insnType & PREFIX_OPCODE)
    nPrefixes++;
  if (insnType & PREFIX_REX)
    nPrefixes++;
  if (insnType & PREFIX_INST)
    nPrefixes++;
  if (insnType & PREFIX_ADDR)
    nPrefixes++;
  return nPrefixes;
}

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

  if (*origInsn == 0xE3) {
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

  // Oops...
  fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
  assert(0 && "Unhandled jump conversion case!");
  return false;
}

bool isStackFramePrecheck_gcc( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

bool isStackFramePrecheck_msvs( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

/*
bool isStackFramePreamble( instruction& insn1 )
{       
    instruction insn2, insn3;
    insn2.setInstruction( insn1.ptr() + insn1.size() );       
    insn3.setInstruction( insn2.ptr() + insn2.size() );

    const unsigned char* p = insn1.op_ptr();
    const unsigned char* q = insn2.op_ptr();
    const unsigned char* r = insn3.op_ptr();
    
    unsigned Mod1_1 =  ( q[ 1 ] >> 3 ) & 0x07;
    unsigned Mod1_2 =  q[ 1 ] & 0x07;
    unsigned Mod2_1 =  ( r[ 1 ] >> 3 ) & 0x07;
    unsigned Mod2_2 =  r[ 1 ] & 0x07;

    if( insn1.size() != 1 )
    {
        return false;  //shouldn't need this, but you never know
    }
    
    if( p[ 0 ] == PUSHEBP  )
    {   
        // Looking for mov %esp -> %ebp in one of the two
        // following instructions.  There are two ways to encode 
        // mov %esp -> %ebp: as '0x8b 0xec' or as '0x89 0xe5'.  
        if( insn2.isMoveRegMemToRegMem() && 
            ((Mod1_1 == 0x05 && Mod1_2 == 0x04) ||
             (Mod1_1 == 0x04 && Mod1_2 == 0x05)))
            return true;

        if( insn3.isMoveRegMemToRegMem() && 
            ((Mod2_1 == 0x05 && Mod2_2 == 0x04) ||
             (Mod2_1 == 0x04 && Mod2_2 == 0x05)))
            return true;
    }
    
    return false;
}
*/
// We keep an array-let that represents various fixed
// insns
unsigned char illegalRep[2] = {0x0f, 0x0b};
unsigned char trapRep[1] = {0xCC};

instruction *instruction::copy() const {
    // Or should we copy? I guess it depends on who allocated
    // the memory...
    return new instruction(*this);
}

void instruction::generateIllegal(codeGen &gen) {
    instruction insn;
    insn.setInstruction(illegalRep);
    insn.generate(gen);
}

void instruction::generateTrap(codeGen &gen) {
    instruction insn;
    insn.setInstruction(trapRep);
    insn.generate(gen);
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */

void instruction::generateBranch(codeGen &gen,
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

void instruction::generateBranch(codeGen &gen,
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
void instruction::generatePush64(codeGen &gen, Address val)
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

void instruction::generateBranch64(codeGen &gen, Address to)
{
    // "long jump" - generates sequence to jump to any 64-bit address
    // pushes the value on the stack (using 4 16-bit pushes) the uses a 'RET'

  generatePush64(gen, to);

  GET_PTR(insn, gen);
  *insn++ = 0xC3; // RET
  SET_PTR(insn, gen);

}

void instruction::generateBranch32(codeGen &gen, Address to)
{
   // "long jump" - generates sequence to jump to any 32-bit address
   emitPushImm(to, gen);
   
   GET_PTR(insn, gen);
   *insn++ = 0xC3; // RET
   SET_PTR(insn, gen);
}

void instruction::generateCall(codeGen &gen,
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

void instruction::generateNOOP(codeGen &gen, unsigned size) {
    // Be more efficient here...
    while (size) {
        GET_PTR(insn, gen);
        *insn++ = NOP;
        SET_PTR(insn, gen);
        size -= sizeof(unsigned char);
    }
}

void instruction::generate(codeGen &gen) {
    assert(ptr_);
    assert(size_);
    memcpy(gen.cur_ptr(), ptr_, size_);
    gen.moveIndex(size_);
}

unsigned instruction::spaceToRelocate() const {
    // List of instructions that might be painful:
    // jumps (displacement will change)
    // call (displacement will change)
    // PC-relative ops

    // TODO: pc-relative ops

    // longJumpSize == size of code needed to get
    // anywhere in memory space.
#if defined(arch_x86_64)
    const int longJumpSize = JUMP_ABS64_SZ;
#else
    const int longJumpSize = JUMP_ABS32_SZ;
#endif


    // Assumption: rewriting calls into immediates will
    // not be larger than rewriting a call into a call...

    if (!((type() & REL_B) ||
	  (type() & REL_W) ||
	  (type() & REL_D) ||
	  (type() & REL_D_DATA))) {
      return size();
    }
    
    // Now that the easy case is out of the way...
    
    if (type() & IS_JUMP) {
      // Worst-case: prefixes + max-displacement branch
      return count_prefixes(type()) + longJumpSize;
    }
    if (type() & IS_JCC) {
      // Jump conditional; jump past jump; long jump
      return count_prefixes(type()) + 2 + 2 + longJumpSize;
    }
    if (type() & IS_CALL) {
      // Worst case is approximated by two long jumps (AMD64) or a REL32 (x86)
      unsigned size;
#if defined(arch_x86_64)
      size = 2*JUMP_ABS64_SZ+count_prefixes(type());
#else
      size = JUMP_SZ+count_prefixes(type());
#endif
      size = (size > CALL_RELOC_THUNK) ? size : CALL_RELOC_THUNK;
      return size;
    }
#if defined(arch_x86_64)
    if (type() & REL_D_DATA) {
      // Worst-case: replace RIP with push of IP, use, and cleanup
      // 8: unknown; previous constant
      return count_prefixes(type()) + size() + 8;
    }
#endif

    assert(0);
    return 0;
}

/*
static void addPatch(codeGen &gen, patchTarget *src, imm_location_t &imm)
{
   switch (imm.size) {
      case 1:
         relocPatch::patch_type_t ptype = imm.is_relative ? relocPatch::pcrel :
            relocPatch::abs;
         Dyninst::Offset off = imm.is_relative ? gen.getIndex() : 0;
         gen.addPatch(imm.words[i], src, imm.size, ptype, off);
         break;
      case 2:
         gen.addPatch(imm.words[0], src, imm.size, reloc_patch::abs_quad1);
         gen.addPatch(imm.words[1], src, imm.size, reloc_patch::abs_quad2);
         break;
      case 4:
         gen.addPatch(imm.words[0], src, imm.size, reloc_patch::abs_quad1);
         gen.addPatch(imm.words[1], src, imm.size, reloc_patch::abs_quad2);
         gen.addPatch(imm.words[2], src, imm.size, reloc_patch::abs_quad3);
         gen.addPatch(imm.words[3], src, imm.size, reloc_patch::abs_quad4);
         break;
      default:
         assert(0);
   }
}
*/


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
    
   instruction::generateBranch(*gen, addr, get_target());
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
   unsigned char *orig_loc;
   Address target = get_target();
   Address potential;
   signed long disp;
   GET_PTR(newInsn, *gen);
   orig_loc = newInsn;

   copy_prefixes_nosize(origInsn, newInsn, insnType); 
   
   //8-bit jump
   potential = addr + 2;
   disp = target - potential;
   if (is_disp8(disp)) {
      convert_to_rel8(origInsn, newInsn);
      *newInsn++ = (signed char) disp;
      SET_PTR(newInsn, *gen);
      return (unsigned) (newInsn - orig_loc);
   }

   //Can't convert E3 jumps to more than 8-bits
   if (*origInsn != 0xE3) {
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
         return (unsigned) (newInsn - orig_loc);
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
   
   // Now for the branch at B- <jumpSize> unconditional branch
   *newInsn++ = 0xEB; 
   unsigned char *fill_in_jumpsize = newInsn++;
   //*newInsn++ = (char) jumpSize(newDisp);
   
   // And the branch at C
   SET_PTR(newInsn, *gen);
   // Original address is a little skewed... 
   // We've moved past the original address (to the tune of nPrefixes + 2 (JCC) + 2 (J))
   Address currAddr = addr + (unsigned) (newInsn - orig_loc);
   instruction::generateBranch(*gen, currAddr, target);
   REGET_PTR(newInsn, *gen);

   //Go back and fill in the size of the jump at B into the 'jump <C>'
   signed char tmp = (signed char) (newInsn - fill_in_jumpsize) - 1;
   *fill_in_jumpsize = tmp;

   return (unsigned) (newInsn - orig_loc);
}

unsigned pcRelJCC::maxSize()
{
   unsigned prefixes = count_prefixes(orig_instruc.type());
#if defined(arch_x86_64)
   if (gen->addrSpace()->getAddressWidth() == 8)
      return prefixes + JUMP_ABS64_SZ + 4;
#endif
   return prefixes + JUMP_REL32_SZ;
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
   instruction::generateCall(*gen, addr, get_target());
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

bool instruction::generate(codeGen &gen,
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

   const unsigned char *origInsn = ptr();
   unsigned insnType = type();
   unsigned insnSz = size();
   // This moves as we emit code
   unsigned char *newInsn = insnBuf;

   // Check to see if we're doing the "get my PC" via a call
   // We do this first as those aren't "real" jumps.
   if (isCall() && !isCallIndir()) {
      // A possibility...
      // Two types: call(0) (AKA call(me+5));
      // or call to a move/return combo.

      // First, call(me)
      Address target = getTarget(origAddr);
      // Big if tree: "if we go to the next insn"
      // Also could do with an instrucIter... or even have
      // parsing label it for us. 
      // TODO: label in parsing (once)
        
      if (target == (origAddr + size())) {
         if(addrSpace->proc())
         {
            *newInsn = 0x68; // Push; we're replacing "call 0" with "push original IP"
            newInsn++;	  
            Address EIP = origAddr + size();
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
		   Address EIP = origAddr + size();
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
      orig_target = origAddr + size() + get_disp(this);
   }
   
#if defined(arch_x86_64)
   if (insnType & REL_D_DATA) {
      // Create pcRelRegion that eventually will call pcRelData::apply
      pcRelData *pcr_data = new pcRelData(orig_target, *this);
      assert(pcr_data);
      gen.addPCRelRegion(pcr_data);
      goto done;
   }
#endif
   
   if (insnType & IS_JUMP) {
     // Create pcRelRegion that eventually will call pcRelJump::apply
     pcRelJump *pcr_jump;
     if (targetOverride) {
        pcr_jump = new pcRelJump(targetOverride, *this);
     }
     else {
        pcr_jump = new pcRelJump(orig_target, *this);
     }
     assert(pcr_jump);
     gen.addPCRelRegion(pcr_jump);
     goto done;
   }

   if (insnType & IS_JCC) {
     // Create pcRelRegion that eventually will call pcRelJCC::apply
     pcRelJCC *pcr_jcc;
     if (targetOverride) {
        pcr_jcc = new pcRelJCC(targetOverride, *this);
     }
     else {
        pcr_jcc = new pcRelJCC(orig_target, *this);
     }
     assert(pcr_jcc);
     gen.addPCRelRegion(pcr_jcc);
     goto done;
   }

   if (insnType & IS_CALL) {
     // Create pcRelRegion that eventually will call pcRelCall::apply
     pcRelCall *pcr_call;
     if (targetOverride) {
        pcr_call = new pcRelCall(targetOverride, *this);
     }
     else {
        pcr_call = new pcRelCall(orig_target, *this);
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
       pcRelJump *ft_jump = new pcRelJump(fallthroughOverride, *this, false);
       assert(ft_jump);
       gen.addPCRelRegion(ft_jump);
   }

   return true;
}

#if defined(arch_x86_64)
unsigned instruction::jumpSize(long disp, unsigned addr_width) 
{
   if (addr_width == 8 && !is_disp32(disp))
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::jumpSize(long /*disp*/, unsigned /*addr_width*/) 
{
   return JUMP_SZ;
}
#endif

unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) 
{
    long disp = to - (from + JUMP_SZ);
    return jumpSize(disp, addr_width);
}

#if defined(arch_x86_64)
unsigned instruction::maxJumpSize(unsigned addr_width) 
{
   if (addr_width == 8)
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::maxJumpSize(unsigned /*addr_width*/) 
{
   return JUMP_SZ;
}
#endif

bool instruction::isCmp() const {
    if(*op_ptr_ == CMP_EB_GB || *op_ptr_ == CMP_EV_GV ||
       *op_ptr_ == CMP_GB_EB || *op_ptr_ == CMP_GV_EV ||
       *op_ptr_ == CMP_AL_LB || *op_ptr_ == CMP_RAX_LZ)
    {
        return true;
    }

    if(*op_ptr_ == 0x80 || *op_ptr_ == 0x81 || *op_ptr_ == 0x83) 
    {
        // group 1 opcodes -- operation depends on reg (bits 3-5) of
        // modr/m byte
        const unsigned char *p = op_ptr_+1;
        if( (*p & (7<<3)) == (7<<3))
            return true;
    }

    return false;
}

#define SIB_SET_REG(x, y) ((x) |= ((y) & 7))
#define SIB_SET_INDEX(x, y) ((x) |= (((y) & 7) << 3))
#define SIB_SET_SS(x, y) ((x) | (((y) & 3) << 6))

typedef struct parsed_instr_t {
   int num_prefixes;
   int opcode_size;
   int disp_position;
   int disp_size;
   int imm_position;
   int imm_size;

   unsigned char sib_byte;
   unsigned char modrm_byte;
   int sib_position; 
   int modrm_position;

   int address_size;
   int modrm_operand;

   int rex_position;
   int rex_byte;
   int rex_w;
   int rex_r;
   int rex_x;
   int rex_b;

   ia32_instruction orig_instr;
   ia32_entry *entry;
} parse_instr_t;

bool instruction::getUsedRegs(pdvector<int> &regs) {
   const unsigned char *insn_ptr = ptr();

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
      return false;
   
   if (loc.modrm_position == -1)
      //Only supporting MOD/RM instructions now
      return false; 

   if (loc.address_size == 1)
      //Don't support 16-bit instructions yet
      return false;

   if (loc.modrm_reg == 4 && !loc.rex_r)
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      return false;

   if (loc.modrm_mod == 3)
      //This instruction doesn't use the MOD/RM to access memory
      return false;

   for (unsigned i=0; i<3; i++) {
      const ia32_operand& op = entry->operands[i];
      if (op.admet == am_O) {
         //The MOD/RM specifies a register that's used
         int regused = loc.modrm_reg;
         if (loc.address_size == 4) {
            regused |= loc.rex_r << 4;
         }
         regs.push_back(regused);
      }
      else if (op.admet == am_reg) {
	using namespace Dyninst::InstructionAPI;
         //The instruction implicitely references a memory instruction
         switch (op.optype) {
             case x86::iah:   
             case x86::ial:
             case x86::iax:   
             case x86::ieax:
               regs.push_back(REGNUM_RAX);
               if (loc.rex_byte) regs.push_back(REGNUM_R8);
               break;
             case x86::ibh:
             case x86::ibl:
             case x86::ibx:
             case x86::iebx:
               regs.push_back(REGNUM_RBX);
               if (loc.rex_byte) regs.push_back(REGNUM_R11);
               break;
             case x86::ich:
             case x86::icl:
             case x86::icx:
             case x86::iecx:
                 regs.push_back(REGNUM_RCX);
               if (loc.rex_byte) regs.push_back(REGNUM_R9);
               break;
             case x86::idh:
             case x86::idl:
             case x86::idx:
             case x86::iedx:
                 regs.push_back(REGNUM_RDX);
               if (loc.rex_byte) regs.push_back(REGNUM_R10);
               break;
             case x86::isp:
             case x86::iesp:
                regs.push_back(REGNUM_RSP);
               if (loc.rex_byte) regs.push_back(REGNUM_R12);
               break;
             case x86::ibp:
             case x86::iebp:
               regs.push_back(REGNUM_RBP);
               if (loc.rex_byte) regs.push_back(REGNUM_R13);
               break;
             case x86::isi:
             case x86::iesi:
               regs.push_back(REGNUM_RSI);
               if (loc.rex_byte) regs.push_back(REGNUM_R14);
               break;
             case x86::idi:
             case x86::iedi:
               regs.push_back(REGNUM_RDI);
               if (loc.rex_byte) regs.push_back(REGNUM_R15);
               break;
            case op_edxeax:
               regs.push_back(REGNUM_RAX);
               regs.push_back(REGNUM_RDX);
               break;
            case op_ecxebx:
               regs.push_back(REGNUM_RBX);
               regs.push_back(REGNUM_RCX);
               break;
         }
      }
   }
   return true;
}

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
bool instruction::generateMem(codeGen &gen,
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
     cerr << "error 2" << endl;
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
   const unsigned char *insn_ptr = ptr();

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
      return false;
   
   if (loc.modrm_position == -1)
      //Only supporting MOD/RM instructions now
      return false; 

   if (loc.address_size == 1)
      //Don't support 16-bit instructions yet
      return false;

   if (loc.modrm_reg == 4 && !loc.rex_r) {
      //The non-memory access register is %rsp/%esp, we can't work with
      // this register due to our register saving techniques.
      return false;
   }

   if (loc.modrm_mod == 3) {
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
   unsigned char new_rex = 0;
   
   //Emit all prefixes except for rex
   for (int i=0; i<loc.num_prefixes; i++) {
      if (i != loc.rex_position)
         *walker++ = insn_ptr[i];
   }
   
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
     //*walker++ = insn_ptr[i];
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

int instruction::getStackDelta()
{
   ia32_instruction instruc;
   const unsigned char *p = ptr();
   ia32_decode(0, ptr(), instruc);

   if (instruc.getEntry()->id == e_push)
      return -4;
   if (instruc.getEntry()->id == e_pop)
      return 4;
   if (instruc.getEntry()->id == e_pusha_d)
      return (-4 * 9);
   if (instruc.getEntry()->id == e_popa_d)
      return (4 * 9);

   if (p[0] == 0x83 && p[1] == 0xec) {
      //Subtract byte
      return -1 * (signed char) p[2];
   }

   if (p[0] == 0x83 && p[1] == 0xc4) {
      //Add byte
      return (signed char) p[2];
   }
   
   return 0;
}

bool instruction::isNop() const
{ 

   int displacement_location = 0;
   int displacement_size = 0;
   if (!(type_ & IS_NOP)) //NOP or LEA
      return false;

   if (*op_ptr_ == NOP) {
      return true;
   }

   ia32_memacc mac[3];
   ia32_condition cnd;
   ia32_locations loc;

   ia32_instruction instruc(mac, &cnd, &loc);

   ia32_decode(IA32_FULL_DECODER, ptr(), instruc);


   if (instruc.getEntry()->id == e_nop) {
      return true;
   }

   if (loc.modrm_mod == 3) {
      return false;
   }
   if (loc.modrm_mod == 0 && loc.modrm_rm == 5) {
      return false;
   }

   if (loc.rex_x) {
      return false;
   }
   if (loc.rex_r != loc.rex_b) {
      return false;
   }

   if (loc.disp_position != -1) {
      for (unsigned i=0; i<loc.disp_size; i++) {
         if (ptr_[i + loc.disp_position] != 0) {
            return false;
         }
      }
      displacement_location = loc.disp_position;
      displacement_size = loc.disp_size;
   }
   
   if (loc.modrm_rm == 4) {
      unsigned scale;
      Register index, base;
      decode_SIB(loc.sib_byte, scale, index, base);
      
      if (index != 4) {
         return false;
      }

      if (base != loc.modrm_reg) {
         return false;
      }
   }
   else if (loc.modrm_reg != loc.modrm_rm) {
      return false;
   }


   return true;
}

#if defined(os_linux) || defined(os_freebsd)
/*
 * Static binary rewriting support
 *
 * Some of the following functions replace the standard ctor and dtor handlers
 * in a binary. Currently, these operations only work with binaries linked with
 * the GNU toolchain. However, it should be straightforward to extend these
 * operations to other toolchains.
 */
static const std::string LIBC_CTOR_HANDLER("__do_global_ctors_aux");
static const std::string LIBC_DTOR_HANDLER("__do_global_dtors_aux");
static const std::string DYNINST_CTOR_HANDLER("DYNINSTglobal_ctors_handler");
static const std::string DYNINST_CTOR_LIST("DYNINSTctors_addr");
static const std::string DYNINST_DTOR_HANDLER("DYNINSTglobal_dtors_handler");
static const std::string DYNINST_DTOR_LIST("DYNINSTdtors_addr");
static const std::string SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
static const std::string SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");

static bool replaceHandler(int_function *origHandler, int_function *newHandler, 
        int_symbol *newList, const std::string &listRelName)
{
    // Add instrumentation to replace the function
    const pdvector<instPoint *> &entries = origHandler->funcEntries();
    AstNodePtr funcJump = AstNode::funcReplacementNode(const_cast<int_function *>(newHandler));
    for(unsigned j = 0; j < entries.size(); ++j) {
        miniTramp *mini = entries[j]->addInst(funcJump,
                callPreInsn, orderFirstAtPoint, true, false);
        if( !mini ) {
            return false;
        }

        // XXX the func jumps are not being generated properly if this is set
        mini->baseT->setCreateFrame(false);

        pdvector<instPoint *> ignored;
        entries[j]->func()->performInstrumentation(false, ignored);
    }

    /* create the special relocation for the new list -- search the RT library for
     * the symbol
     */
    Symbol *newListSym = const_cast<Symbol *>(newList->sym());
    
    std::vector<Region *> allRegions;
    if( !newListSym->getSymtab()->getAllRegions(allRegions) ) {
        return false;
    }

    bool success = false;
    std::vector<Region *>::iterator reg_it;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
        vector<relocationEntry>::iterator rel_it;
        for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
            if( rel_it->getDynSym() == newListSym ) {
                relocationEntry *rel = &(*rel_it);
                rel->setName(listRelName);
                success = true;
            }
        }
    }

    return success;
}

bool BinaryEdit::doStaticBinarySpecialCases() {
    Symtab *origBinary = mobj->parse_img()->getObject();

    /* Special Case 1: Handling global constructor and destructor Regions
     *
     * Replace global ctors function with special ctors function,
     * and create a special relocation for the ctors list used by the special
     * ctors function
     *
     * Replace global dtors function with special dtors function,
     * and create a special relocation for the dtors list used by the special
     * dtors function
     */
    if( !mobj->parse_img()->findGlobalConstructorFunc(LIBC_CTOR_HANDLER) ) {
        return false;
    }

    if( !mobj->parse_img()->findGlobalDestructorFunc(LIBC_DTOR_HANDLER) ) {
        return false;
    }

    // First, find all the necessary symbol info.
    int_function *globalCtorHandler = findOnlyOneFunction(LIBC_CTOR_HANDLER);
    if( !globalCtorHandler ) {
        logLine("failed to find libc constructor handler\n");
        return false;
    }

    int_function *dyninstCtorHandler = findOnlyOneFunction(DYNINST_CTOR_HANDLER);
    if( !dyninstCtorHandler ) {
        logLine("failed to find Dyninst constructor handler\n");
        return false;
    }

    int_function *globalDtorHandler = findOnlyOneFunction(LIBC_DTOR_HANDLER);
    if( !globalDtorHandler ) {
        logLine("failed to find libc destructor handler\n");
        return false;
    }

    int_function *dyninstDtorHandler = findOnlyOneFunction(DYNINST_DTOR_HANDLER);
    if( !dyninstDtorHandler ) {
        logLine("failed to find Dyninst destructor handler\n");
        return false;
    }

    int_symbol ctorsListInt;
    int_symbol dtorsListInt;
    bool ctorFound = false, dtorFound = false; 
    std::vector<BinaryEdit *>::iterator rtlib_it;
    for(rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
        if( (*rtlib_it)->getSymbolInfo(DYNINST_CTOR_LIST, ctorsListInt) ) {
            ctorFound = true;
            if( dtorFound ) break;
        }

        if( (*rtlib_it)->getSymbolInfo(DYNINST_DTOR_LIST, dtorsListInt) ) {
            dtorFound = true;
            if( ctorFound ) break;
        }
    }

    if( !ctorFound ) {
         logLine("failed to find ctors list symbol\n");
         return false;
    }

    if( !dtorFound ) {
        logLine("failed to find dtors list symbol\n");
        return false;
    }

    /*
     * Replace the libc ctor and dtor handlers with our special handlers
     */
    if( !replaceHandler(globalCtorHandler, dyninstCtorHandler,
                &ctorsListInt, SYMTAB_CTOR_LIST_REL) ) {
        logLine("Failed to replace libc ctor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced ctor function %s with %s\n",
                FILE__, __LINE__, LIBC_CTOR_HANDLER.c_str(),
                DYNINST_CTOR_HANDLER.c_str());
    }

    if( !replaceHandler(globalDtorHandler, dyninstDtorHandler,
                &dtorsListInt, SYMTAB_DTOR_LIST_REL) ) {
        logLine("Failed to replace libc dtor handler with special handler");
        return false;
    }else{
        inst_printf("%s[%d]: replaced dtor function %s with %s\n",
                FILE__, __LINE__, LIBC_DTOR_HANDLER.c_str(),
                DYNINST_DTOR_HANDLER.c_str());
    }

    /*
     * Special Case 2: Issue a warning if attempting to link pthreads into a binary
     * that originally did not support it or into a binary that is stripped. This
     * scenario is not supported with the initial release of the binary rewriter for
     * static binaries.
     *
     * The other side of the coin, if working with a binary that does have pthreads
     * support, pthreads needs to be loaded.
     */
    bool isMTCapable = isMultiThreadCapable();
    bool foundPthreads = false;

    vector<Archive *> libs;
    vector<Archive *>::iterator libIter;
    if( origBinary->getLinkingResources(libs) ) {
        for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
            if( (*libIter)->name().find("libpthread") != std::string::npos ) {
                foundPthreads = true;
                break;
            }
        }
    }

    if( foundPthreads && (!isMTCapable || origBinary->isStripped()) ) {
        fprintf(stderr,
            "\nWARNING: the pthreads library has been loaded and\n"
            "the original binary is not multithread-capable or\n"
            "it is stripped. Currently, the combination of these two\n"
            "scenarios is unsupported and unexpected behavior may occur.\n");
    }else if( !foundPthreads && isMTCapable ) {
        fprintf(stderr,
            "\nWARNING: the pthreads library has not been loaded and\n"
            "the original binary is multithread-capable. Unexpected\n"
            "behavior may occur because some pthreads routines are\n"
            "unavailable in the original binary\n");
    }

    /* 
     * Special Case 3:
     * The RT library has some dependencies -- Symtab always needs to know
     * about these dependencies. So if the dependencies haven't already been
     * loaded, load them.
     */
    bool loadLibc = true;

    for(libIter = libs.begin(); libIter != libs.end(); ++libIter) {
        if( (*libIter)->name().find("libc.a") != std::string::npos ) {
            loadLibc = false;
        }
    }

    if( loadLibc ) {
        std::map<std::string, BinaryEdit *> res = openResolvedLibraryName("libc.a");
        std::map<std::string, BinaryEdit *>::iterator bedit_it;
        for(bedit_it = res.begin(); bedit_it != res.end(); ++bedit_it) {
            if( bedit_it->second == NULL ) {
                logLine("Failed to load DyninstAPI_RT library dependency (libc.a)");
                return false;
            }
        }
    }

    return true;
}

#endif
