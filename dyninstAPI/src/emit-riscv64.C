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

#include "dyninstAPI/src/inst-riscv64.h"
#include "dyninstAPI/src/emit-riscv64.h"
#include "dyninstAPI/src/registerSpace.h"

codeBufIndex_t EmitterRISCV64::emitIf(
        Register expr_reg, Register target, RegControl /*rc*/, codeGen &gen)
{
    // TODO
    return 0;
}

void EmitterRISCV64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
    insnCodeGen::loadImmIntoReg(gen, dest, imm);
}


void EmitterRISCV64::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);

    insnCodeGen::loadImmIntoReg(gen, scratch, addr);
    /* As we want to zero extend the dest register, we should set isUnsigned to true */
    insnCodeGen::generateMemLoad(gen, dest, scratch, 0, size, true);

    gen.rs()->freeRegister(scratch);
    gen.markRegDefined(dest);
}


void EmitterRISCV64::emitStore(Address addr, Register src, int size, codeGen &gen)
{
    Register scratch = gen.rs()->getScratchRegister(gen);

    insnCodeGen::loadImmIntoReg(gen, scratch, addr);
    insnCodeGen::generateMemStore(gen, scratch, dest, 0, size);

    gen.rs()->freeRegister(scratch);
    gen.markRegDefined(src);
}


void EmitterRISCV64::emitOp(
        unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
    // dest = src1 + src2
    if (opcode == plusOp) {
        insnCodeGen::generateAdd(gen, src1, src2, dest);
    }

    // dest = src1 - src2
    else if(opcode == minusOp) {
        insnCodeGen::generateSub(gen, src1, src2, dest);
    }
    
    // dest = src1 * src2
    else if (opcode == timesOp) {
        insnCodeGen::generateMul(gen, src1, src2, dest);
    }

    // dest = src1 & src2
    else if (opcode == andOp) {
        insnCodeGen::generateAnd(gen, src1, src2, dest);
    }

    // dest = src1 | src2
    else if (opcode == orOp) {
        insnCodeGen::generateOr(gen, src1, src2, dest);
    }

    // dest = src1 ^ src2
    else if (opcode == xorOp) {
        insnCodeGen::generateXor(gen, src1, src2, dest);
    }
    else assert(0);
}


void EmitterRISCV64::emitRelOp(
        unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
    // TODO
}


//#sasha Fix parameters number
void EmitterRISCV64::emitGetParam(
        Register, Register param_num,
        instPoint::Type, opCode op,
        bool, codeGen &gen)
{
    // TODO
}


void EmitterRISCV64::emitRelOpImm(
        unsigned opcode, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
{
    // TODO
}


void EmitterRISCV64::emitLoadIndir(Register dest, Register addr_src, int size, codeGen &gen)
{
    // TODO
}
void EmitterRISCV64::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen)
{
    // TODO
}

void EmitterRISCV64::emitLoadOrigRegRelative(
        Register dest, Address offset, Register base, codeGen &gen, bool deref)
{
    // TODO
}


void EmitterRISCV64::emitLoadOrigRegister(Address register_num, Register destination, codeGen &gen)
{
    // TODO
}


