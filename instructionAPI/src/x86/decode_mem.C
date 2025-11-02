#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "decoder.h"
#include "entryIDs.h"
#include "x86/register_xlat.h"
#include "type_conversion.h"


namespace Dyninst { namespace InstructionAPI {

  void x86_decoder::decode_mem(Instruction &insn, cs_x86_op const &operand) {
    /*
     *  SDM 3.7.5 Specifying an Offset
     *
     *  The offset part of a memory address can be specified directly as a static value (called
     *  a displacement) or through an address computation made up of one or more of the following
     *  components:
     *
     *  - Displacement: An 8-, 16-, or 32-bit value.
     *  - Base:         The value in a general-purpose register.
     *  - Index:        The value in a general-purpose register.
     *  - Scale factor: A value of 2, 4, or 8 that is multiplied by the index value.
     *
     *  Offset = Base + (Index * Scale) + Displacement
     *
     *  The offset which results from adding these components is called an effective address. Each
     *  of these components can have either a positive or negative (2s complement) value, with the
     *  exception of the scaling factor.
     */

    auto disp = [&]() -> Expression::Ptr {
      // Capstone: Displacement value, valid if encoding.disp_offset != 0
      if(disassembler.insn->detail->x86.encoding.disp_offset == 0) {
        return {};
      }
      auto const size = disassembler.insn->detail->x86.encoding.disp_size;
      auto const type = size_to_type_signed(size);
      return Immediate::makeImmediate(Result(type, operand.mem.disp));
    }();

    auto base = [&]() -> Expression::Ptr {
      if(operand.mem.base == X86_REG_INVALID) {
        return {};
      }
      auto const basereg = x86::translate_register(operand.mem.base, this->mode);
      return makeRegisterExpression(basereg);
    }();

    auto index = [&]() -> Expression::Ptr {
      if(operand.mem.index == X86_REG_INVALID) {
        return {};
      }
      if(operand.mem.index == X86_REG_EIZ || operand.mem.index == X86_REG_RIZ) {
        // {E,R}IZ is a pseudo register used when the index register is explicitly
        // set to zero in the opcode. It's used by decoders, but has no meaning
        // for the ISA.
        return {};
      }
      auto reg = x86::translate_register(operand.mem.index, this->mode);
      return makeRegisterExpression(reg);
    }();

    auto scaled_index = [&]() -> Expression::Ptr {
      // SDM: A scale factor may be used only when an index also is used
      if(!index) {
        return {};
      }
      auto scale = Immediate::makeImmediate(Result(u8, operand.mem.scale));

      // The size of the Index register *should* be the same as the size
      // of the operand, but I can't find a requirement for that in the
      // SDM so don't assume any sizes here.
      auto reg = x86::translate_register(operand.mem.index, this->mode);
      auto const type = size_to_type_signed(reg.size());
      return makeMultiplyExpression(std::move(index), std::move(scale), type);
    }();

    // Calculate `(Index * Scale) + Displacement`
    auto rhs = [&]() -> Expression::Ptr {
      if(!scaled_index) {
        return disp;
      }
      if(!disp) {
        return scaled_index;
      }

      // As with scaled_index, this shouldn't be necessary but be careful.
      auto const index_reg = x86::translate_register(operand.mem.index, this->mode);
      auto const disp_size = disassembler.insn->detail->x86.encoding.disp_size;
      auto const size = std::max(index_reg.size(), static_cast<std::uint32_t>(disp_size));
      auto const type = size_to_type_signed(size);
      return makeAddExpression(std::move(scaled_index), std::move(disp), type);
    }();

    auto effectiveAddr = [&]() -> Expression::Ptr {
      if(!rhs) {
        return base;
      }
      if(!base) {
        return rhs;
      }
      auto const type = size_to_type_signed(operand.size);
      return makeAddExpression(std::move(base), std::move(rhs), type);
    }();

    auto finalAddress = [&]() -> Expression::Ptr {
      if(operand.mem.segment == X86_REG_INVALID) {
        return effectiveAddr;
      }

      // Real-Address Mode `Seg + effectiveAddr`
      auto const reg = x86::translate_register(operand.mem.segment, this->mode);
      auto segReg = makeRegisterExpression(reg);

      // It's an absolute address, so it's unsigned
      auto const type = size_to_type_unsigned(operand.size);
      return makeAddExpression(std::move(segReg), std::move(effectiveAddr), type);
    }();

    constexpr bool is_implicit = true;

    // Special cases that read and write, but Capstone only has one access specifier
    {
      constexpr bool is_read = true;
      constexpr bool is_written = true;

      switch(insn.getOperation().getID()) {
        case e_cmpxchg:
        case e_cmpxchg16b:
        case e_cmpxchg8b:
        case e_xchg: {
          // exchange instructions read and write their memory operand
          auto const type = size_to_type_signed(operand.size);
          auto deref = makeDereferenceExpression(std::move(finalAddress), type);
          insn.appendOperand(std::move(deref), is_read, is_written, !is_implicit);
          return;
        }
        case e_lea: {
          // LEA (Load Effective Address) does not dereference its memory operand
          insn.appendOperand(std::move(finalAddress), !is_read, !is_written, !is_implicit);
          return;
        }
        default:
          break;
      }
    }

    // Capstone may report register operands as neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool is_read = ((operand.access & CS_AC_READ) != 0);
    bool is_written = ((operand.access & CS_AC_WRITE) != 0);
    if(!is_read && !is_written) {
      is_read = is_written = true;
    }

    // has a control flow target (e.g., `call [rbx + rax * 2 + 0xff]`)
    if(is_cft(insn)) {
      auto const is_call = insn.isCall();
      auto const is_fallthrough = insn.allowsFallThrough();
      auto const type = size_to_type_signed(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.addSuccessor(std::move(expr), is_call, true, false, is_fallthrough, !is_implicit);
      return;
    }

    // x87 instruction
    if(cs_insn_group(disassembler.handle, disassembler.insn, X86_GRP_FPU)) {
      auto const type = size_to_type_float(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
      return;
    }

    // vector instruction
    if(insn.isVector()) {
      auto const type = size_to_type_memory(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
      return;
    }

    is_written = [is_written, &insn]() {
      switch(insn.getOperation().getID()) {
        // String comparisons write to their destination registers, but not to memory
        case e_cmpsb:
        case e_cmpsw:
        case e_cmpsd:
        case e_cmpsq:
        case e_lodsb:
        case e_lodsw:
        case e_lodsd:
        case e_lodsq:
        case e_outsb:
        case e_outsw:
        case e_outsd:
          return false;
        default:
          return is_written;
      }
    }();

    is_read = [is_read, &insn]() {
      switch(insn.getOperation().getID()) {
        // 'Input from Port to String' instructions read their source registers, but not from memory
        case e_insb:
        case e_insw:
        case e_insd:
          return false;
        default:
          return is_read;
      }
    }();

    // basic instruction (e.g., `mov rax, ds:[rbx + rax * 2 + 0xff]`)
    auto const type = size_to_type_signed(operand.size);
    auto expr = makeDereferenceExpression(std::move(finalAddress), type);
    insn.appendOperand(std::move(expr), is_read, is_written, !is_implicit);
  }

}}
