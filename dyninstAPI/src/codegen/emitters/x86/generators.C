#include "arch-regs-x86.h"
#include "codegen/emitters/x86/generators.h"
#include "inst-x86.h"
#include "registerSpace/registerSpace.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <limits>

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  void emitAddMem(Address addr, int imm, codeGen &gen) {
    // This add needs to encode "special" due to an exception
    //  to the normal encoding rules and issues caused by AMD64's
    //  pc-relative data addressing mode.  Our helper functions will
    //  not correctly emit what we want, and we want this very specific
    //  mode for the add instruction.  So I'm just writing raw bytes.

    GET_PTR(insn, gen);
    if(imm < 128 && imm > -127) {
      if(gen.rs()->getAddressWidth() == 8) {
        append_memory_as_byte(insn, 0x48); // REX byte for a quad-add
      }
      append_memory_as_byte(insn, 0x83);
      append_memory_as_byte(insn, 0x04);
      append_memory_as_byte(insn, 0x25);

      assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32-bits");
      append_memory_as(insn, static_cast<uint32_t>(addr)); // Write address

      append_memory_as(insn, static_cast<int8_t>(imm));
      SET_PTR(insn, gen);
      return;
    }

    if(imm == 1) {
      if(gen.rs()->getAddressWidth() == 4) {
        append_memory_as_byte(insn, 0xFF); // incl
        append_memory_as_byte(insn, 0x05);
      } else {
        assert(gen.rs()->getAddressWidth() == 8);
        append_memory_as_byte(insn, 0xFF); // inlc with SIB
        append_memory_as_byte(insn, 0x04);
        append_memory_as_byte(insn, 0x25);
      }
    } else {
      append_memory_as_byte(insn, 0x81); // addl
      append_memory_as_byte(insn, 0x4);
      append_memory_as_byte(insn, 0x25);
    }

    assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32-bits");
    append_memory_as(insn, static_cast<uint32_t>(addr)); // Write address

    if(imm != 1) {
      append_memory_as(insn, int32_t{imm}); // Write immediate value to add
    }

    SET_PTR(insn, gen);
  }

  void emitSegPrefix(Register segReg, codeGen &gen) {
    switch(segReg) {
      case REGNUM_FS:
        emitSimpleInsn(PREFIX_SEGFS, gen);
        return;
      case REGNUM_GS:
        emitSimpleInsn(PREFIX_SEGGS, gen);
        return;
      default:
        assert(0 && "Segment register not handled");
        return;
    }
  }

}}}
