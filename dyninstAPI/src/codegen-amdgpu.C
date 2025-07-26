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

#include <stdlib.h>
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/inst-amdgpu.h"
#include "dyninstAPI/src/emit-amdgpu.h"
#include "dyninstAPI/src/function.h"

// "Casting" methods. We use a "base + offset" model, but often need to
// turn that into "current instruction pointer".
codeBuf_t *insnCodeGen::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
#if defined(endian_mismatch)
  // Writing an instruction.  Convert byte order if necessary.
  unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
  unsigned raw = insn.asInt();
#endif

  gen.copy(&raw, sizeof(unsigned));
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
#if defined(endian_mismatch)
    // Writing an instruction.  Convert byte order if necessary.
    unsigned raw = swapBytesIfNeeded(insn.asInt());
#else
    unsigned raw = insn.asInt();
#endif

    gen.insert(&raw, sizeof(unsigned), position);
}

void insnCodeGen::generateIllegal(codeGen & /* gen */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateTrap(codeGen & /* gen */) {
  // BinaryEdit::addTrap calls this.
  // We currently only do static binary rewriting for AMDGPU and don't need to generate trap instructions.
}

void insnCodeGen::generateBranch(codeGen & /* gen */, long /* disp */, bool /* link */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool /* link */) {
    long disp = (to - from);
    long wordOffset = disp/4;

    Emitter *emitter = gen.emitter();

    if (wordOffset >= INT16_MIN && wordOffset <= INT16_MAX) {
      emitter->emitShortJump(wordOffset, gen);
    } else {
      // TODO: Right now hardcoding s90. But this needs to use 2 pairs of dead registers.
      emitter->emitLongJump(90, from, to, gen);
    }
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
    generateBranch(gen, from, to, true);
}

void insnCodeGen::generateLongBranch(codeGen & /* gen */, Dyninst::Address /* from */, Dyninst::Address /* to */, bool /* isCall */){
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateBranchViaTrap(codeGen &/* gen */, Dyninst::Address /* from */, Dyninst::Address /* to */, bool /* isCall */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateConditionalBranch(codeGen& /* gen */, Dyninst::Address /* to */, unsigned /* opcode */, bool /* s */){
    assert(false && "Not implemented for AMDGPU");
}


void insnCodeGen::generateAddSubShifted(codeGen & /* gen */, insnCodeGen::ArithOp /* op */, int /* shift */, int /* imm6 */, Dyninst::Register /* rm */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */){
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateAddSubImmediate(codeGen & /* gen */, insnCodeGen::ArithOp /* op */, int /* shift */, int /* imm12 */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */)
{
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMul(codeGen & /* gen */, Dyninst::Register /* rm */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateDiv(codeGen & /* gen */, Dyninst::Register /* rm */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */ , bool /* s */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateBitwiseOpShifted(codeGen & /* gen */, insnCodeGen::BitwiseOp /* op */, int /* shift */, Dyninst::Register /* rm */, int /* imm6 */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateLoadReg(codeGen &, Dyninst::Register, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateStoreReg(codeGen &, Dyninst::Register, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateLoadReg64(codeGen &, Dyninst::Register, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateStoreReg64(codeGen &, Dyninst::Register, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMove(codeGen & /* gen */, int /* imm16 */, int /* shift */, Dyninst::Register /* rd */, MoveOp /* movOp */) {

    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMove(codeGen & /* gen */, Dyninst::Register /* rd */, Dyninst::Register /* rm */, bool /* is64bit */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMoveSP(codeGen & /* gen */, Dyninst::Register /* rn */, Dyninst::Register /* rd */, bool /* is64bit */) {
    assert(false && "Not implemented for AMDGPU");
}


Dyninst::Register insnCodeGen::moveValueToReg(codeGen & /* gen */, long int /* val */, std::vector<Dyninst::Register> * /* exclude */) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMemAccess(codeGen & /* gen */, LoadStore /* accType */, Dyninst::Register /* r1 */, Dyninst::Register /* r2 */, int /* immd */, unsigned /* size */, IndexMode /* im */) {
    assert(false && "Not implemented for AMDGPU");
}

// This is for generating STR/LDR (SIMD&FP) (immediate) for indexing modes of Post, Pre and Offset
void insnCodeGen::generateMemAccessFP(codeGen &/* gen */, LoadStore /* accType */, Dyninst::Register /* rt */, Dyninst::Register /* rn */, int /* immd */, int /* size */, bool /* is128bit */, IndexMode /* im */){
    assert(false && "Not implemented for AMDGPU");
}

// rlwinm ra,rs,n,0,31-n
void insnCodeGen::generateLShift(codeGen &, Dyninst::Register, int, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateRShift(codeGen &, Dyninst::Register, int, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

// sld ra, rs, rb
void insnCodeGen::generateLShift64(codeGen &, Dyninst::Register, int, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

// srd ra, rs, rb
void insnCodeGen::generateRShift64(codeGen &, Dyninst::Register, int, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
void insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
    assert((size % instruction::size()) == 0);

    Emitter *emitter = gen.emitter();

    while (size) {
        emitter->emitNops(1, gen);
        size -= instruction::size();
    }
}

void insnCodeGen::generateRelOp(codeGen &, int, int, Dyninst::Register, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}


void insnCodeGen::saveRegister(codeGen & /* gen */, Dyninst::Register /* r */, int /* sp_offset */, IndexMode /* im */) {
    assert(false && "Not implemented for AMDGPU");
}


void insnCodeGen::restoreRegister(codeGen & /* gen */, Dyninst::Register /* r */, int /* sp_offset */, IndexMode /* im */) {
    assert(false && "Not implemented for AMDGPU");
}


// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void insnCodeGen::loadPartialImmIntoReg(codeGen &, Dyninst::Register, long) {
}

int insnCodeGen::createStackFrame(codeGen &, int, std::vector<Dyninst::Register>& , std::vector<Dyninst::Register>&) {
    assert(false && "Not implemented for AMDGPU");
  return 0;
}

void insnCodeGen::removeStackFrame(codeGen &) {
    assert(false && "Not implemented for AMDGPU");
}

bool insnCodeGen::generateMem(codeGen &, instruction&, Dyninst::Address, Dyninst::Address, Dyninst::Register, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMoveFromLR(codeGen &, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMoveToLR(codeGen &, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

void insnCodeGen::generateMoveToCR(codeGen &, Dyninst::Register) {
    assert(false && "Not implemented for AMDGPU");
}

// There's nothing to modify here. Generate an emit a jump to target.
bool insnCodeGen::modifyJump(Dyninst::Address target, NS_amdgpu::instruction & /* insn */, codeGen &gen) {
    long disp = target - gen.currAddr();
    Emitter *emitter = gen.emitter();
    int16_t wordOffset = disp/4;

    assert(wordOffset > MIN_IMM16 && wordOffset < MAX_IMM16 && "wordOffset must fit in a 16-bit signed value");
    emitter->emitShortJump(wordOffset, gen);

    return true;
}

/* TODO and/or FIXME
 * The logic used by this function is common across architectures but is replicated 
 * in architecture-specific manner in all codegen-* files.
 * This means that the logic itself needs to be refactored into the (platform 
 * independent) codegen.C file. Appropriate architecture-specific,
 * bit-twiddling functions can then be defined if necessary in the codegen-* files 
 * and called as necessary by the common, refactored logic.
*/

/*
 * <Addr X> conditionalJump A
 * <Addr C> nextInstruction  // C = X+4
 *
 * The above is transformed to:
 *
 * <Addr X>   conditionalJump <B>
 * <Addr X+4> jump C
 * <Addr B>   jump A           // B = X+8
 * <Addr C>   nextInstruction  // C = X+12
 *
 */
bool insnCodeGen::modifyJcc(Dyninst::Address target, NS_amdgpu::instruction &insn, codeGen &gen) {
    // We start by modifying the current conditional jump instruction in-place:
    //  <Addr X> conditionalJump A  ---> <Addr X> conditionalJump B
    assert(insn.size() == 4 && "Conditional branch on AMDGPU must be 4 bytes long");
    uint32_t rawInst = insn.asInt();

    // This is a SOPP instruction, so simply set the the SIMM16 field appropriately.
    Vega::setSImm16SopP(1, rawInst); // CPU does (1)*4 + 4 and computes the target = X+8 i.e B

    // Now copy this at the end of the codegen buffer
    gen.copy((void *)&rawInst, sizeof rawInst);

    // We are at offset X+4, we want to jump to X+12, i.e C
    // Now emit jump C
    Emitter *emitter = gen.emitter();
    emitter->emitShortJump(1, gen); // CPU does (1)*4 + 4 and computes the target = <X+4> + 8 = X+12 i.e C

    // Now emit jump A, the original target
	  long from = gen.currAddr();
    long disp = target - from;
    int16_t wordOffset = disp/4;

    assert(wordOffset > MIN_IMM16 && wordOffset < MAX_IMM16 && "wordOffset must fit in a 16-bit signed value");
    emitter->emitShortJump(wordOffset, gen);

    return true;
}

bool insnCodeGen::modifyCall(Dyninst::Address target, NS_amdgpu::instruction &insn, codeGen &gen) {
    if (insn.isUncondBranch())
        return modifyJump(target, insn, gen);
    else
        return modifyJcc(target, insn, gen);
}

bool insnCodeGen::modifyData(Dyninst::Address /* target */, NS_amdgpu::instruction & /* insn */, codeGen & /* gen */) {
    assert(false && "Not implemented for AMDGPU");
    return false;
}
