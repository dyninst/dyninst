#include "arch-aarch64.h"
#include "codegen/codegen-aarch64.h"
#include "generators.h"
#include "inst-aarch64.h"

#include <cassert>

namespace Dyninst { namespace DyninstAPI { namespace aarch64 {

  /*
   * Emit code to push down the stack
   */
  void pushStack(codeGen &gen) {
    if(gen.width() == 8) {
      insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Sub, 0, TRAMP_FRAME_SIZE_64, REG_SP,
                                           REG_SP, true);
    } else {
      assert(0); // 32 bit not implemented
    }
  }

  void popStack(codeGen &gen) {
    if(gen.width() == 8) {
      insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, TRAMP_FRAME_SIZE_64, REG_SP,
                                           REG_SP, true);
    } else {
      assert(0); // 32 bit not implemented
    }
  }

}}}
