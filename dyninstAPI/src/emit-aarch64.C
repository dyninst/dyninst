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

/*
 * emit-aarch64.C - ARMv8 code generators (emitters)
 */

/*
#include <assert.h>
#include <stdio.h>
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include "dyninstAPI/src/dynProcess.h"

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
// get_index...
#include "dyninstAPI/src/dynThread.h"
#include "ABI.h"
#include "liveness.h"
#include "RegisterConversion.h"
*/

#include "dyninstAPI/src/inst-aarch64.h"
#include "dyninstAPI/src/emit-aarch64.h"
#include "dyninstAPI/src/registerSpace.h"

codeBufIndex_t EmitterAARCH64::emitIf(
        Register expr_reg, Register target, RegControl /*rc*/, codeGen &gen)
{
    instruction insn;
    insn.clear();

    // compare to 0 and branch
    // register number, its value is compared to 0.
    INSN_SET(insn, 0, 4, expr_reg);
    INSN_SET(insn, 5, 23, (target+4)/4);
    INSN_SET(insn, 25, 30, 0x1a); // CBZ
    INSN_SET(insn, 31, 31, 1);

    insnCodeGen::generate(gen,insn);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();
    return retval;
}

void EmitterAARCH64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    insnCodeGen::loadImmIntoReg(gen, dest, imm);
}


void EmitterAARCH64::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);

    insnCodeGen::loadImmIntoReg(gen, scratch, addr);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest,
            scratch, 0, size, insnCodeGen::Post);

    gen.rs()->freeRegister(scratch);
    gen.markRegDefined(dest);
}


void EmitterAARCH64::emitStore(Address addr, Register src, int size, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);

    insnCodeGen::loadImmIntoReg(gen, scratch, addr);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, src,
            scratch, 0, size, insnCodeGen::Pre);

    gen.rs()->freeRegister(scratch);
    gen.markRegDefined(src);
}


void EmitterAARCH64::emitOp(
        unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
    // dest = src1 + src2
    if( opcode == plusOp )
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, 0, src1, src2, dest, true);

    // dest = src1 - src2
    else if( opcode == minusOp )
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);
    
    // dest = src1 * src2
    else if( opcode == timesOp )
        insnCodeGen::generateMul(gen, src1, src2, dest, true);

    // dest = src1 & src2
    else if( opcode == andOp )
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::And, 0, src1, 0, src2, dest, true);  

    // dest = src1 | src2
    else if( opcode == orOp )
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, src1, 0, src2, dest, true);  

    // dest = src1 ^ src2
    else if( opcode == xorOp )
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Eor, 0, src1, 0, src2, dest, true);

    else assert(0);
}


void EmitterAARCH64::emitRelOp(
        unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
    // CMP is an alias to SUBS;
    // dest here has src1-src2, which it's not important because the flags are
    // used for the comparison, not the subtration value.
    // Besides that dest must contain 1 for true or 0 for false, and the content
    // of dest is gonna be changed as follow.
    insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);

    // make dest = 1, meaning true
    insnCodeGen::loadImmIntoReg(gen, dest, 0x1);

    // insert conditional jump to skip dest=0 in case the comparison resulted true
    // therefore keeping dest=1
    insnCodeGen::generateConditionalBranch(gen, 8, opcode, s);

    // make dest = 0, in case it fails the branch
    insnCodeGen::loadImmIntoReg(gen, dest, 0x0);
}


//#sasha Fix parameters number
void EmitterAARCH64::emitGetParam(
        Register, Register param_num,
        instPoint::Type, opCode op,
        bool, codeGen &gen)
{
    registerSlot *regSlot = NULL;
    switch (op) {
        case getParamOp:
            if(param_num <= 3) {
                // param_num is 0..8 - it's a parameter number, not a register
                regSlot = (*(gen.rs()))[registerSpace::r0 + param_num];
                break;

            } else {
                assert(0);
            }
            break;
        default:
            assert(0);
            break;
    } // end of swich(op)

    assert(regSlot);
    //Register reg = regSlot->number;

    //return reg;
}


void EmitterAARCH64::emitRelOpImm(
        unsigned opcode, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
{
    //Register src2 = gen.rs()->allocateRegister(gen, true);
    Register src2 = gen.rs()->getScratchRegister(gen);
    emitLoadConst(src2, src2imm, gen);

    // CMP is an alias to SUBS;
    // dest here has src1-src2, which it's not important because the flags are
    // used for the comparison, not the subtration value.
    // Besides that dest must contain 1 for true or 0 for false, and the content
    // of dest is gonna be changed as follow.
    insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);

    // make dest = 1, meaning true
    insnCodeGen::loadImmIntoReg(gen, dest, 0x1);

    // insert conditional jump to skip dest=0 in case the comparison resulted true
    // therefore keeping dest=1
    insnCodeGen::generateConditionalBranch(gen, 8, opcode, s);

    // make dest = 0, in case it fails the branch
    insnCodeGen::loadImmIntoReg(gen, dest, 0x0);

    gen.rs()->freeRegister(src2);
    gen.markRegDefined(dest);
}


void EmitterAARCH64::emitLoadIndir(Register dest, Register addr_src, int size, codeGen &gen)
{
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest,
            addr_src, 0, size, insnCodeGen::Post);

    gen.markRegDefined(dest);
}
void EmitterAARCH64::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen)
{
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, src,
            addr_reg, 0, size, insnCodeGen::Pre);

    gen.markRegDefined(addr_reg);
}

void EmitterAARCH64::emitLoadOrigRegRelative(
        Register dest, Address offset, Register base, codeGen &gen, bool deref)
{

    gen.markRegDefined(dest);
    Register scratch = gen.rs()->getScratchRegister(gen);
    assert(scratch);
    gen.markRegDefined(scratch);

    // either load the address or the contents at that address
    if(deref)
    {
        // load the stored register 'base' into scratch
        emitLoadOrigRegister(base, scratch, gen);
        // move offset(%scratch), %dest
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest,
            scratch, offset, /*size==8?true:false*/4, insnCodeGen::Offset);
    }
    else
    {
        // load the stored register 'base' into dest
	emitLoadOrigRegister(base, scratch, gen);
	insnCodeGen::loadImmIntoReg(gen, dest, offset);
	insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, 0, dest, scratch, dest, true);
    }
}


void EmitterAARCH64::emitLoadOrigRegister(Address register_num, Register destination, codeGen &gen)
{

   registerSlot *src = (*gen.rs())[register_num];
   assert(src);
   registerSlot *dest = (*gen.rs())[destination];
   assert(dest);

   if (src->name == "sp") {
      insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0,
             TRAMP_FRAME_SIZE_64, REG_SP, destination, true);

      return;
   }

   if (src->spilledState == registerSlot::unspilled)
   {
      // not on the stack. Directly move the value
      insnCodeGen::generateMove(gen, destination, (Register) register_num, true);
      return;
   }


    int offset = TRAMP_GPR_OFFSET(gen.width());
    // its on the stack so load it.
    insnCodeGen::restoreRegister(gen, destination, offset + (register_num * gen.width()),
            insnCodeGen::Offset);
}


