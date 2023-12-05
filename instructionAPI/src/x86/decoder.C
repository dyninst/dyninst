#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "x86/decoder.h"
#include "x86/register-xlat.h"
#include "mnemonic-xlat.h"
#include "debug.h"

#include "entryIDs.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/MachRegister.h"

#include <cstdint>
#include <utility>

namespace di = Dyninst::InstructionAPI;

static bool is_in_group(cs_detail*, cs_group_type);
static di::Result_Type size_to_type(uint8_t);

namespace Dyninst { namespace InstructionAPI {

  x86_decoder::x86_decoder(Dyninst::Architecture a): InstructionDecoderImpl(a) {

    mode = (a == Dyninst::Arch_x86_64) ? CS_MODE_64 : CS_MODE_32;

    auto create = [this](disassem &d, cs_opt_value v) {
      cs_open(CS_ARCH_X86, this->mode, &d.handle);
      cs_option(d.handle, CS_OPT_DETAIL, v);
      d.insn = cs_malloc(d.handle);
    };

    /*
     *  With details enabled, a Capstone instruction object has complete information.
     *
     *  This is used in 'decodeOperands' because all of the details are needed.
    */
    create(dis_with_detail, CS_OPT_ON);

    /*
     *  Without details, a Capstone instruction object has fewer populated fields
     *  (e.g., no operand details) so takes up less space and time. Capstone instruction
     *  objects _always_ populate the mnemonic and a string representation of the operands.
     *
     *  This is used in 'decodeOpcode' to quickly create an Instruction object.
    */
    create(dis_without_detail, CS_OPT_OFF);
  }

  x86_decoder::~x86_decoder() {
    cs_free(dis_with_detail.insn, 1);
    cs_close(&dis_with_detail.handle);

    cs_free(dis_without_detail.insn, 1);
    cs_close(&dis_without_detail.handle);
  }

  void x86_decoder::doDelayedDecode(Instruction const* insn) {
    auto* code = static_cast<unsigned char const*>(insn->ptr());
    size_t codeSize = insn->size();
    uint64_t cap_addr = 0;

    // We need all of the instruction details in order to unpack the operands
    auto &dis = dis_with_detail;

    // The iterator form of disassembly allows reuse of the instruction object, reducing
    // the number of memory allocations.
    if(!cs_disasm_iter(dis.handle, &code, &codeSize, &cap_addr, dis.insn)) {
      decode_printf("Failed to disassemble instruction at %p: %s\n", code, cs_strerror(cs_errno(dis.handle)));
      return;
    }
    decode_operands(insn, dis);
  }

  void x86_decoder::decodeOpcode(InstructionDecoder::buffer& buf) {
    auto* code = buf.start;
    size_t codeSize = buf.end - buf.start;
    uint64_t cap_addr = 0;

    // We want this to be as fast as possible, so don't have Capstone provide all details.
    auto &dis = dis_without_detail;

    // The iterator form of disassembly allows reuse of the instruction object, reducing
    // the number of memory allocations.
    if(!cs_disasm_iter(dis.handle, &code, &codeSize, &cap_addr, dis.insn)) {
      decode_printf("Failed to disassemble instruction at %p: %s\n", code, cs_strerror(cs_errno(dis.handle)));
      m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
      return;
    }

    entryID e = x86::translate_opcode(static_cast<x86_insn>(dis.insn->id));
    m_Operation = Operation(e, dis.insn->mnemonic, m_Arch);
    buf.start += dis.insn->size;
  }

  bool x86_decoder::decodeOperands(Instruction const*) {
    // Capstone does this for us.
    return true;
  }

  void x86_decoder::decode_operands(Instruction const* insn, disassem dis) {
    auto const category = insn->getCategory();
    if(category == c_ReturnInsn) {
      auto ret_addr = makeDereferenceExpression(makeRegisterExpression(MachRegister::getStackPointer(m_Arch)), u64);
      insn->addSuccessor(ret_addr, false, true, false, false);
      return;
    }

    const bool isCFT = (category == c_BranchInsn || category == c_CallInsn);
    const bool isCall = isCFT && c_CallInsn;
    const bool isConditional = (category == c_BranchInsn && insn->getOperation().getID() != e_jmp);
    const bool isRela = is_in_group(d, CS_GRP_BRANCH_RELATIVE);

    bool err = false;
    cs_x86* detail = &(d->x86);
    for(uint8_t i = 0; i < detail->op_count; ++i) {
      cs_x86_op* operand = &(detail->operands[i]);
      if(operand->type == X86_OP_REG) {
        auto regAST = makeRegisterExpression(x86::translate_register(operand->reg, mode));
        if(isCFT) {
          // if a call or a jump has a register as an operand,
          // it should not be a conditional jump
          assert(!isConditional);
          insn->addSuccessor(regAST, isCall, true, false, false);
        } else {
          // Capstone may report register operands as neither read nor written.
          // In this case, we mark it as both read and written to be conservative.
          bool isRead = ((operand->access & CS_AC_READ) != 0);
          bool isWritten = ((operand->access & CS_AC_WRITE) != 0);
          if(!isRead && !isWritten) {
            isRead = isWritten = true;
          }
          insn->appendOperand(regAST, isRead, isWritten, false);
        }
        // TODO: correctly mark implicit registers
      } else if(operand->type == X86_OP_IMM) {
        auto immAST = Immediate::makeImmediate(Result(s32, operand->imm));
        if(isCFT) {
          // It looks like that Capstone automatically adjust the offset with the instruction length
          auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
          if(isRela) {
            auto target(makeAddExpression(IP, immAST, u64));
            insn->addSuccessor(target, isCall, false, isConditional, false);
          } else {
            insn->addSuccessor(immAST, isCall, false, isConditional, false);
          }
          if(isConditional)
            insn->addSuccessor(IP, false, false, true, true);
        } else {
          insn->appendOperand(immAST, false, false, false);
        }
      } else if(operand->type == X86_OP_MEM) {
        Expression::Ptr effectiveAddr;
        x86_op_mem* mem = &(operand->mem);
        // TODO: handle segment registers
        if(mem->base != X86_REG_INVALID) {
          effectiveAddr = makeRegisterExpression(x86::translate_register(mem->base, this->mode));
        }
        if(mem->index != X86_REG_INVALID) {
          Expression::Ptr indexAST = makeRegisterExpression(x86::translate_register(mem->index, this->mode));
          indexAST = makeMultiplyExpression(indexAST, Immediate::makeImmediate(Result(u8, mem->scale)), u64);
          if(effectiveAddr)
            effectiveAddr = makeAddExpression(effectiveAddr, indexAST, u64);
          else
            effectiveAddr = indexAST;
        }
        // Displacement for addressing memory. So it is unsigned
        auto immAST = Immediate::makeImmediate(Result(u32, mem->disp));
        if(effectiveAddr)
          effectiveAddr = makeAddExpression(effectiveAddr, immAST, u64);
        else
          effectiveAddr = immAST;
        Result_Type type = size_to_type(operand->size);
        if(type == invalid_type) {
          err = true;
        }
        Expression::Ptr memAST;
        if(insn->getOperation().getID() == e_lea)
          memAST = effectiveAddr;
        else
          memAST = makeDereferenceExpression(effectiveAddr, type);
        if(isCFT) {
          assert(!isConditional);
          insn->addSuccessor(memAST, isCall, true, false, false);
        } else {
          // Capstone may report register operands as neither read nor written.
          // In this case, we mark it as both read and written to be conservative.
          bool isRead = ((operand->access & CS_AC_READ) != 0);
          bool isWritten = ((operand->access & CS_AC_WRITE) != 0);
          if(!isRead && !isWritten) {
            isRead = isWritten = true;
          }
          insn->appendOperand(memAST, isRead, isWritten, false);
        }
      } else {
        fprintf(stderr, "Unhandled capstone operand type %d\n", operand->type);
      }
    }

    // The key is a Capstone register enum
    // The value is a pair of boolean, where the first represnet whether read or not
    // and the second one represents whether written or not
    std::map<uint16_t, std::pair<bool, bool>> implicitRegs;
    for(int i = 0; i < d->regs_read_count; ++i) {
      implicitRegs.insert(std::make_pair(d->regs_read[i], std::make_pair(true, false)));
    }
    for(int i = 0; i < d->regs_write_count; ++i) {
      auto it = implicitRegs.find(d->regs_write[i]);
      if(it == implicitRegs.end()) {
        implicitRegs.insert(std::make_pair(d->regs_write[i], std::make_pair(false, true)));
      } else {
        it->second.second = true;
      }
    }

    for(auto rit = implicitRegs.begin(); rit != implicitRegs.end(); ++rit) {
      MachRegister reg = x86::translate_register((x86_reg)rit->first, this->mode);
      // Traditionally, instructionAPI only present individual flag fields,
      // not the whole flag register
      if(reg == Dyninst::x86::flags || reg == Dyninst::x86_64::flags)
        continue;
      auto regAST = makeRegisterExpression(reg);
      insn->appendOperand(regAST, rit->second.first, rit->second.second, true);
    }
    if(err)
      fprintf(stderr, "\tinstruction %s\n", insn->format().c_str());
  }

}}

bool is_in_group(cs_detail *d, cs_group_type g) {
  for (uint8_t i = 0; i < d->groups_count; ++i)
    if (d->groups[i] == g)
      return true;
  return false;
}

di::Result_Type size_to_type(uint8_t cap_size) {
  switch (cap_size) {
    case 1:  return di::u8;
    case 2:  return di::u16;
    case 3:  return di::u24;
    case 4:  return di::u32;   // Could also be m32 or sp_float
    case 6:  return di::u48;
    case 8:  return di::u64;   // Could also be sp_double or m64
    case 10: return di::m80;
    case 12: return di::m96;
    case 14: return di::m14;
    case 16: return di::m128;  // Could also be dbl128
    case 20: return di::m160;
    case 24: return di::m192;
    case 28: return di::m224;
    case 32: return di::m256;
    case 36: return di::m288;
    case 40: return di::m320;
    case 44: return di::m352;
    case 48: return di::m384;
    case 52: return di::m416;
    case 56: return di::m448;
    case 60: return di::m480;
    case 64: return di::m512;
    default: return di::invalid_type;
  }
}
