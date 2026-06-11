#include "codegen/codegen-power.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Dyn.h"
#include "codegen/emitters/PowerPC/ppc64/generators.h"
#include "inst-power.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterPowerPC64Dyn::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
    // This code is complicated by the need to set the new TOC and restore it
    // post-(call/branch). That means we can't use a branch if asked, since we won't
    // regain control. Fun.
    //
    // 1. Move down the stack and save R12 and LR
    // 2. R2 := TOC of callee
    // 3. Load callee into R12 (V2 ABI requires the callee address should be in R12)
    // 4. LR := R12
    // 5. Call LR
    // 6. Restore R12 and LR
    // 7. R2 := TOC of caller
    // 8. Move up the stack
    // IF (!call)
    //   Return

    const unsigned TOCreg = 2;
    const unsigned wordsize = gen.width();
    assert(wordsize == 8);
    Address dest = block->start();

    // We need the callee TOC, which we find by function, not by block.
    std::vector<func_instance *> funcs;
    block->getFuncs(std::back_inserter(funcs));
    Address callee_toc = gen.addrSpace()->getTOCoffsetInfo(funcs[0]);

    Address caller_toc = 0;
    if(gen.func()) {
      caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.func());
    } else if(gen.point()) {
      assert(gen.point()->func());
      caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.point()->func());
    } else {
      // Don't need it, and this might be an iRPC
    }
    unsigned r12 = 12;

    // Move down the stack to create space for saving registers
    ppc64::pushStack(gen);

    // Save R12 and LR
    insnCodeGen::generateMoveFromLR(gen, TOCreg);
    insnCodeGen::generateMemAccess64(gen, STDop, STDxop, TOCreg, REG_SP, 3 * wordsize);
    insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r12, REG_SP, 4 * wordsize);

    // Use the R12 to generate the destination address
    insnCodeGen::loadImmIntoReg(gen, r12, dest);
    insnCodeGen::generateMoveToLR(gen, r12);

    // Load the callee TOC
    insnCodeGen::loadImmIntoReg(gen, TOCreg, callee_toc);

    instruction branch_insn(BRLraw);
    insnCodeGen::generate(gen, branch_insn);

    // Restore R12 and LR
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, TOCreg, REG_SP, 3 * wordsize);
    insnCodeGen::generateMoveToLR(gen, TOCreg);
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r12, REG_SP, 4 * wordsize);

    // Load caller TOC
    insnCodeGen::loadImmIntoReg(gen, TOCreg, caller_toc);

    // Move up the stack
    ppc64::popStack(gen);

    if(!call) {
      instruction ret(BRraw);
      insnCodeGen::generate(gen, ret);
    }

    return true;
  }

}}
