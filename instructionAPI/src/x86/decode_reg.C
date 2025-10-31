#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "decoder.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "x86/register_xlat.h"


namespace Dyninst { namespace InstructionAPI {

  void x86_decoder::decode_reg(Instruction &insn, cs_x86_op const &operand) {
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;
    constexpr bool is_conditional = true;
    constexpr bool is_fallthrough = true;

    auto regAST = makeRegisterExpression(x86::translate_register(operand.reg, mode));

    const bool isCall = insn.isCall();
    if(insn.isBranch() || isCall) {
      insn.addSuccessor(regAST, isCall, is_indirect, !is_conditional, !is_fallthrough);
      return;
    }

    // It's an error if an operand is neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool is_read = ((operand.access & CS_AC_READ) != 0);
    bool is_written = ((operand.access & CS_AC_WRITE) != 0);

    if(!is_read && !is_written) {
      is_read = is_written = true;
    }

    // Capstone read/write isn't populated for 'dx' for the I/O input string instructions
    switch(insn.getOperation().getID()) {
      case e_insb:
      case e_insw:
      case e_insd:
      case e_outsb:
      case e_outsw:
      case e_outsd:
        is_read = true;
        is_written = false;
        break;
      default:
        break;
    }

    insn.appendOperand(regAST, is_read, is_written, !is_implicit);
  }

}}
