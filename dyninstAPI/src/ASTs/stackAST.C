#include "Architecture.h"
#include "ast_helpers.h"
#include "codegen.h"
#include "debug.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "stackAST.h"

namespace Dyninst { namespace DyninstAPI {

bool stackAST::allocateCanaryRegister(codeGen &gen, bool noCost, Dyninst::Register &reg,
                                          bool &needSaveAndRestore) {
  // Let's see if we can find a dead register to use!
  instPoint *point = gen.point();

  // Try to get a scratch register from the register space
  registerSpace *regSpace = registerSpace::actualRegSpace(point);
  bool realReg = true;
  Dyninst::Register tmpReg = regSpace->getScratchRegister(gen, noCost, realReg);
  if(tmpReg != Dyninst::Null_Register) {
    reg = tmpReg;
    needSaveAndRestore = false;
    if(gen.getArch() == Dyninst::Arch_x86) {
      gen.rs()->noteVirtualInReal(reg, RealRegister(reg));
    }
    return true;
  }

  // Couldn't find a dead register to use :-(
  registerSpace *deadRegSpace = registerSpace::optimisticRegSpace(gen.addrSpace());
  reg = deadRegSpace->getScratchRegister(gen, noCost, realReg);
  if(reg == Dyninst::Null_Register) {
    ast_printf("WARNING: using default allocateAndKeep in allocateCanaryRegister\n");
    reg = allocateAndKeep(gen, noCost);
  }
  needSaveAndRestore = true;
  ast_printf("allocateCanaryRegister will require save&restore at 0x%lx\n", gen.point()->addr());

  return true;
}

}}
