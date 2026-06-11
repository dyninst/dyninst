#include "arch-power.h"
#include "codegen/codegen-power.h"
#include "codegen/codegen.h"
#include "codegen/emitters/PowerPC/ppc64/generators.h"
#include "inst-power.h"

namespace Dyninst { namespace DyninstAPI { namespace ppc64 {

  void popStack(codeGen &gen) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE_32);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateImm(gen, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE_64);
    }
  }

  /*
   * Emit code to push down the stack, AST-generate style
   */
  void pushStack(codeGen &gen) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE_32);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, STDop, STDUxop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE_64);
    }
  }

}}}
