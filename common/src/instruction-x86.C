#include "ia32_prefixes.h"
#include "instruction-x86.h"

#include <cstdint>

namespace NS_x86 {

  instruction *instruction::copy() const {
    // Or should we copy? I guess it depends on who allocated
    // the memory...
    return new instruction(*this);
  }

  unsigned instruction::spaceToRelocate() const {
    // List of instructions that might be painful:
    // jumps (displacement will change)
    // call (displacement will change)
    // PC-relative ops

    // TODO: pc-relative ops

    // longJumpSize == size of code needed to get
    // anywhere in memory space.
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
    const int longJumpSize = JUMP_ABS64_SZ;
#else
    const int longJumpSize = JUMP_ABS32_SZ;
#endif

    // Assumption: rewriting calls into immediates will
    // not be larger than rewriting a call into a call...

    if(!((type() & REL_B) || (type() & REL_W) || (type() & REL_D) || (type() & REL_D_DATA))) {
      return size();
    }

    // Now that the easy case is out of the way...

    if(type() & IS_JUMP) {
      // Worst-case: prefixes + max-displacement branch
      return count_prefixes(type()) + longJumpSize;
    }
    if(type() & IS_JCC) {
      // Jump conditional; jump past jump; long jump
      return count_prefixes(type()) + 2 + 2 + longJumpSize;
    }
    if(type() & IS_CALL) {
      // Worst case is approximated by two long jumps (AMD64) or a REL32 (x86)
      unsigned size;
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
      size = 2 * JUMP_ABS64_SZ + count_prefixes(type());
#else
      size = JUMP_SZ + count_prefixes(type());
#endif
      size = (size > CALL_RELOC_THUNK) ? size : CALL_RELOC_THUNK;
      return size;
    }
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
    if(type() & REL_D_DATA) {
      // Worst-case: replace RIP with push of IP, use, and cleanup
      // 8: unknown; previous constant
      return count_prefixes(type()) + size() + 8;
    }
#endif

    assert(0);
    return 0;
  }

#if defined(DYNINST_CODEGEN_ARCH_X86_64)

  static bool is_disp32(long disp) {
    return (disp <= INT32_MAX && disp >= INT32_MIN);
  }

  unsigned instruction::jumpSize(long disp, unsigned addr_width) {
    if(addr_width == 8 && !is_disp32(disp)) {
      return JUMP_ABS64_SZ;
    }
    return JUMP_SZ;
  }
#else
  unsigned instruction::jumpSize(long /*disp*/, unsigned /*addr_width*/) {
    return JUMP_SZ;
  }
#endif

  unsigned instruction::jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width) {
    long disp = to - (from + JUMP_SZ);
    return jumpSize(disp, addr_width);
  }

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
  unsigned instruction::maxJumpSize(unsigned addr_width) {
    if(addr_width == 8) {
      return JUMP_ABS64_SZ;
    }
    return JUMP_SZ;
  }
#else
  unsigned instruction::maxJumpSize(unsigned /*addr_width*/) {
    return JUMP_SZ;
  }
#endif
}
