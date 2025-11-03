#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "decoder.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "x86/register_xlat.h"
#include "type_conversion.h"

namespace Dyninst { namespace InstructionAPI {

  void x86_decoder::decode_imm(Instruction &insn, cs_x86_op const &operand) {
    auto const type = size_to_type_signed(operand.size);
    auto imm = Immediate::makeImmediate(Result(type, operand.imm));

    constexpr bool is_read = true;
    constexpr bool is_written = true;
    constexpr bool is_implicit = true;
    constexpr bool is_indirect = true;

    // If it's not a control flow instruction, then this is a normal operand
    // e.g., 'mov rax, 0x1234'
    {
      // 'ret imm16' does a near/far return to calling procedure and pops imm16
      // bytes from stack. The immediate is _not_ the return target.
      bool const is_ret = insn.isReturn();

      if(!this->is_cft(insn) || is_ret) {
        insn.appendOperand(std::move(imm), !is_read, !is_written, !is_implicit);
        return;
      }
    }

    auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

    auto const is_call_insn = insn.isCall();
    auto const is_conditional_insn = insn.isConditional();
    auto const usesRelativeAddressing = cs_insn_group(disassembler.handle, disassembler.insn, CS_GRP_BRANCH_RELATIVE);
    bool const is_fallthrough_insn = insn.allowsFallThrough();

    if(usesRelativeAddressing) {
      insn.appendOperand(imm, is_read, !is_written, !is_implicit);
      insn.appendOperand(IP, is_read, is_written, is_implicit);
      // Capstone adjusts the offset to account for the current instruction's length, so we can
      // just create an addition AST expression here.
      auto target(makeAddExpression(IP, imm, s64));
      insn.addSuccessor(std::move(target), is_call_insn, !is_indirect, is_conditional_insn, is_fallthrough_insn);
    } else {
      insn.addSuccessor(std::move(imm), is_call_insn, !is_indirect, is_conditional_insn, is_fallthrough_insn);
    }
    if(is_conditional_insn) {
      constexpr bool is_call = true;
      constexpr bool is_conditional = true;
      insn.addSuccessor(std::move(IP), !is_call, !is_indirect, is_conditional, is_fallthrough_insn);
    }
  }

}}
