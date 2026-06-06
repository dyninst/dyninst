#include "arch-aarch64.h"
#include "codegen/codegen-aarch64.h"
#include "EmitterAarch64Stat.h"
#include "registerSpace/registerSpace.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterAarch64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    Address dest = getInterModuleFuncAddr(callee, gen);
    long varOffset = dest - gen.currAddr();

    Register baseReg = gen.rs()->getScratchRegister(gen);
    assert(baseReg != Null_Register && "cannot get a scratch register");
    emitMovePCToReg(baseReg, gen);

    std::vector<Register> exclude;
    exclude.push_back(baseReg);
    // mov offset to a reg
    auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
    // add/sub offset to baseReg
    insnCodeGen::generateAddSubShifted(
        gen, (signed long long)varOffset > 0 ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0, addReg,
        baseReg, baseReg, true);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, baseReg, baseReg, 0, 8,
                                   insnCodeGen::Offset);

    // call instruction
    instruction branchInsn;
    branchInsn.clear();
    // Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);
    // Set register
    INSN_SET(branchInsn, 5, 9, baseReg);
    // Set other bits. Basically, these are the opcode bits.
    // The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    INSN_SET(branchInsn, 21, 21, 1);
    insnCodeGen::generate(gen, branchInsn);

    return true;
  }

  bool EmitterAarch64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    Address dest = getInterModuleFuncAddr(callee, gen);
    long varOffset = dest - gen.currAddr();

    Register baseReg = gen.rs()->getScratchRegister(gen);
    assert(baseReg != Null_Register && "cannot get a scratch register");
    emitMovePCToReg(baseReg, gen);

    std::vector<Register> exclude;
    exclude.push_back(baseReg);
    // mov offset to a reg
    auto addReg = insnCodeGen::moveValueToReg(gen, labs(varOffset), &exclude);
    // add/sub offset to baseReg
    insnCodeGen::generateAddSubShifted(
        gen, (signed long long)varOffset > 0 ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0, addReg,
        baseReg, baseReg, true);
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, baseReg, baseReg, 0, 8,
                                   insnCodeGen::Offset);

    // jump instruction
    instruction branchInsn;
    branchInsn.clear();
    // Set bits which are 0 for both BR and BLR
    INSN_SET(branchInsn, 0, 4, 0);
    INSN_SET(branchInsn, 10, 15, 0);
    // Set register
    INSN_SET(branchInsn, 5, 9, baseReg);
    // Set other bits. Basically, these are the opcode bits.
    // The only difference between BR and BLR is that bit 21 is 1 for BLR.
    INSN_SET(branchInsn, 16, 31, BRegOp);
    insnCodeGen::generate(gen, branchInsn);

    return true;
  }

}}
