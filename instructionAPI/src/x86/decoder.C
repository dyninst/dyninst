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

static di::Result_Type size_to_type(uint8_t);

static bool is_cft(di::InsnCategory const c) { return c == di::c_BranchInsn || c == di::c_CallInsn; }
static bool is_call(di::InsnCategory const c) { return is_cft(c) && c == di::c_CallInsn; }
static bool is_conditional(di::InsnCategory const c, entryID const id) { return c == di::c_BranchInsn && id != e_jmp; }

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
      auto ret_addr = insn->makeReturnExpression();
      insn->addSuccessor(ret_addr, false, true, false, false);
      return;
    }

    /* Decode _explicit_ operands
     *
     * There are three types of these for x86:
     *
     *   add r1, r2       ; r1, r2 are both X86_OP_REG
     *   jmp -64          ; -64 is X86_OP_IMM
     *   mov r1, 0x33     ; r1 is X86_OP_REG, 0x33 is X86_OP_MEM
     */
    auto* d = dis.insn->detail;
    for(uint8_t i = 0; i < d->x86.op_count; ++i) {
      cs_x86_op const& operand = d->x86.operands[i];
      switch(operand.type) {
        case X86_OP_REG:
          decode_reg(insn, operand);
          break;
        case X86_OP_IMM:
          decode_imm(insn, operand);
          break;
        case X86_OP_MEM:
          decode_mem(insn, operand);
          break;
        case X86_OP_INVALID:
          decode_printf("[0x%lx %s %s] has an invalid operand.\n", dis.insn->address,
                        dis.insn->mnemonic, dis.insn->op_str);
          break;
      }
    }

    /* Decode _implicit_ operands
     *
     * These are operands which are not part of the opcode. Some opcodes
     * have both explicit and implicit operands. For example,
     *
     *   add r1, r2       ; {e,r}flags is written to implicitly
     *   jmp -64          ; PC/IP is written to implicitly
     *
     * Some have only implicit:
     *
     *   pop  ; modifies stack pointer {e,r}sp
     */
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
  }

  void x86_decoder::decode_reg(Instruction const* insn, cs_x86_op const& operand) {
    // TODO: correctly mark implicit registers
    auto const isCFT = is_cft(insn->getCategory());
    auto regAST = makeRegisterExpression(di::x86::translate_register(operand.reg, mode));

    if(isCFT) {
      auto const isCall = is_call(insn->getCategory());
      insn->addSuccessor(regAST, isCall, true, false, false);
      return;
    }

    // It's an error if an operand is neither read nor written.
    // In this case, we mark it as both read and written to be conservative.
    bool isRead = ((operand.access & CS_AC_READ) != 0);
    bool isWritten = ((operand.access & CS_AC_WRITE) != 0);
    if(!isRead && !isWritten) {
      isRead = isWritten = true;
    }
    insn->appendOperand(regAST, isRead, isWritten, false);
  }

  void x86_decoder::decode_imm(Instruction const* insn, cs_x86_op const& operand) {
    auto immAST = Immediate::makeImmediate(Result(s32, operand.imm));
    auto const isCFT = is_cft(insn->getCategory());

    if(!isCFT) {
      insn->appendOperand(immAST, false, false, false);
      return;
    }

    // It looks like that Capstone automatically adjust the offset with the instruction length
    auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

    auto const& dis = dis_with_detail;
    auto const isCall = is_call(insn->getCategory());
    auto const isConditional = is_conditional(insn->getCategory(), insn->getOperation().getID());
    auto const usesRelativeAddressing = cs_insn_group(dis.handle, dis.insn, CS_GRP_BRANCH_RELATIVE);

    if(usesRelativeAddressing) {
      auto target(makeAddExpression(IP, immAST, u64));
      insn->addSuccessor(target, isCall, false, isConditional, false);
    } else {
      insn->addSuccessor(immAST, isCall, false, isConditional, false);
    }
    if(isConditional) {
      insn->addSuccessor(IP, false, false, true, true);
    }
  }

  void x86_decoder::decode_mem(Instruction const* insn, cs_x86_op const& operand) {
    Expression::Ptr effectiveAddr;
    auto const isCFT = is_cft(insn->getCategory());
    auto const isCall = is_call(insn->getCategory());
    auto const isConditional = is_conditional(insn->getCategory(), insn->getOperation().getID());
    // TODO: handle segment registers
    if(operand.mem.base != X86_REG_INVALID) {
      effectiveAddr = makeRegisterExpression(x86::translate_register(operand.mem.base, this->mode));
    }
    if(operand.mem.index != X86_REG_INVALID) {
      Expression::Ptr indexAST = makeRegisterExpression(x86::translate_register(operand.mem.index, this->mode));
      indexAST = makeMultiplyExpression(indexAST, Immediate::makeImmediate(Result(u8, operand.mem.scale)), u64);
      if(effectiveAddr)
        effectiveAddr = makeAddExpression(effectiveAddr, indexAST, u64);
      else
        effectiveAddr = indexAST;
    }
    // Displacement for addressing memory. So it is unsigned
    auto immAST = Immediate::makeImmediate(Result(u32, operand.mem.disp));
    if(effectiveAddr)
      effectiveAddr = makeAddExpression(effectiveAddr, immAST, u64);
    else
      effectiveAddr = immAST;
    Result_Type type = size_to_type(operand.size);
    if(type == invalid_type) {
//      err = true;
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
      bool isRead = ((operand.access & CS_AC_READ) != 0);
      bool isWritten = ((operand.access & CS_AC_WRITE) != 0);
      if(!isRead && !isWritten) {
        isRead = isWritten = true;
      }
      insn->appendOperand(memAST, isRead, isWritten, false);
    }
  }
}}

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
