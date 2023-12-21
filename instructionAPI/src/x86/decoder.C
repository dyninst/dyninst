#include "x86/decoder.h"
#include "x86/register-xlat.h"
#include "mnemonic-xlat.h"
#include "debug.h"

#include "entryIDs.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/MachRegister.h"

#include <cstdint>
#include <map>
#include <vector>

namespace di = Dyninst::InstructionAPI;

static di::Result_Type size_to_type(uint8_t);

static bool is_call(di::InsnCategory const c) { return c == di::c_CallInsn; }
static bool is_cft(di::InsnCategory const c) { return c == di::c_BranchInsn || is_call(c); }
static bool is_conditional(di::InsnCategory const c, entryID const id) { return c == di::c_BranchInsn && id != e_jmp; }

struct implicit_state final { bool read, written; };
static std::map<x86_reg,implicit_state> implicit_registers(cs_detail const*);

struct eflags_t final { Dyninst::MachRegister reg; implicit_state state; };
static std::vector<eflags_t> expand_eflags(x86_reg, cs_mode);

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
     * There are three types:
     *
     *   add r1, r2       ; r1, r2 are both X86_OP_REG
     *   jmp -64          ; -64 is X86_OP_IMM
     *   mov r1, [0x33]   ; r1 is X86_OP_REG, 0x33 is X86_OP_MEM
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
    for(auto r : implicit_registers(d)) {
      constexpr bool is_implicit = true;
      x86_reg reg = r.first;
      if(reg == X86_REG_EFLAGS) {
        for(auto fr : expand_eflags(reg, this->mode)) {
          if(!fr.state.read && !fr.state.written) continue; // don't report untouched registers
          auto regAST = makeRegisterExpression(fr.reg);
          insn->appendOperand(regAST, fr.state.read, fr.state.written, is_implicit);
        }
      } else {
        MachRegister mreg = x86::translate_register(reg, this->mode);
        auto regAST = makeRegisterExpression(mreg);
        implicit_state s = r.second;
        insn->appendOperand(regAST, s.read, s.written, is_implicit);
      }
    }
  }

  void x86_decoder::decode_reg(Instruction const* insn, cs_x86_op const& operand) {
    auto regAST = makeRegisterExpression(x86::translate_register(operand.reg, mode));

    if(is_cft(insn->getCategory())) {
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
    // Capstone internally converts all immediate values to int64_t
    auto immAST = Immediate::makeImmediate(Result(s64, operand.imm));

    if(!is_cft(insn->getCategory())) {
      insn->appendOperand(immAST, false, false, false);
      return;
    }

    auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

    auto const& dis = dis_with_detail;
    auto const isCall = is_call(insn->getCategory());
    auto const isConditional = is_conditional(insn->getCategory(), insn->getOperation().getID());
    auto const usesRelativeAddressing = cs_insn_group(dis.handle, dis.insn, CS_GRP_BRANCH_RELATIVE);

    if(usesRelativeAddressing) {
      // Capstone adjusts the offset to account for the current instruction's length, so we can
      // just create an addition AST expression here.
      auto target(makeAddExpression(IP, immAST, s64));
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

    // TODO: handle segment registers
    if(operand.mem.base != X86_REG_INVALID) {
      effectiveAddr = makeRegisterExpression(x86::translate_register(operand.mem.base, this->mode));
    }
    if(operand.mem.index != X86_REG_INVALID) {
      Expression::Ptr indexAST = makeRegisterExpression(x86::translate_register(operand.mem.index, this->mode));
      indexAST = makeMultiplyExpression(indexAST, Immediate::makeImmediate(Result(u8, operand.mem.scale)), s64);
      if(effectiveAddr) {
        effectiveAddr = makeAddExpression(effectiveAddr, indexAST, s64);
      } else {
        effectiveAddr = indexAST;
      }
    }

    auto immAST = Immediate::makeImmediate(Result(s32, operand.mem.disp));
    if(effectiveAddr) {
      effectiveAddr = makeAddExpression(effectiveAddr, immAST, s64);
    } else {
      effectiveAddr = immAST;
    }

    Expression::Ptr memAST;
    if(insn->getOperation().getID() == e_lea) {
      memAST = effectiveAddr;
    } else {
      Result_Type type = size_to_type(operand.size);
      memAST = makeDereferenceExpression(effectiveAddr, type);
    }

    if(is_cft(insn->getCategory())) {
      auto const isCall = is_call(insn->getCategory());
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

std::map<x86_reg,implicit_state> implicit_registers(cs_detail const* d) {
  std::map<x86_reg, implicit_state> regs;
  for(int i = 0; i < d->regs_read_count; ++i) {
    regs.emplace(static_cast<x86_reg>(d->regs_read[i]), implicit_state{true, false});
  }
  for(int i = 0; i < d->regs_write_count; ++i) {
    auto res = regs.emplace(static_cast<x86_reg>(d->regs_write[i]), implicit_state{false, true});
    if(!res.second) {
      // Register already existed, so was read. Mark it written.
      res.first->second.written = true;
    }
  }
  return regs;
}

/* clang-format  off */
std::vector<eflags_t> expand_eflags(x86_reg r, cs_mode m) {
  /*
   *  There are six possible types of access modeled by Capstone:
   *
   *    MODIFY    - written to
   *    PRIOR     - this is never used in Capstone
   *    RESET     - set to zero
   *    SET       - written to
   *    TEST      - read from
   *    UNDEFINED - no guarantees about the state of the flag
   *
   */
  std::vector<eflags_t> regs;
  if(m == CS_MODE_64) {
    regs.push_back({Dyninst::x86_64::af,  {!!(r & X86_EFLAGS_TEST_AF), !!((r & X86_EFLAGS_SET_AF) || (r & X86_EFLAGS_RESET_AF) || (r & X86_EFLAGS_MODIFY_AF))}});
    regs.push_back({Dyninst::x86_64::cf,  {!!(r & X86_EFLAGS_TEST_CF), !!((r & X86_EFLAGS_SET_CF) || (r & X86_EFLAGS_RESET_CF) || (r & X86_EFLAGS_MODIFY_CF))}});
    regs.push_back({Dyninst::x86_64::sf,  {!!(r & X86_EFLAGS_TEST_SF), !!((r & X86_EFLAGS_SET_SF) || (r & X86_EFLAGS_RESET_SF) || (r & X86_EFLAGS_MODIFY_SF))}});
    regs.push_back({Dyninst::x86_64::zf,  {!!(r & X86_EFLAGS_TEST_ZF), !!((r & X86_EFLAGS_SET_ZF) || (r & X86_EFLAGS_RESET_ZF) || (r & X86_EFLAGS_MODIFY_ZF))}});
    regs.push_back({Dyninst::x86_64::pf,  {!!(r & X86_EFLAGS_TEST_PF), !!((r & X86_EFLAGS_SET_PF) || (r & X86_EFLAGS_RESET_PF) || (r & X86_EFLAGS_MODIFY_PF))}});
    regs.push_back({Dyninst::x86_64::of,  {!!(r & X86_EFLAGS_TEST_OF), !!((r & X86_EFLAGS_SET_OF) || (r & X86_EFLAGS_RESET_OF) || (r & X86_EFLAGS_MODIFY_OF))}});
    regs.push_back({Dyninst::x86_64::tf,  {!!(r & X86_EFLAGS_TEST_TF), !!(                           (r & X86_EFLAGS_RESET_TF) || (r & X86_EFLAGS_MODIFY_TF))}});
    regs.push_back({Dyninst::x86_64::if_, {!!(r & X86_EFLAGS_TEST_IF), !!((r & X86_EFLAGS_SET_IF) || (r & X86_EFLAGS_RESET_IF) || (r & X86_EFLAGS_MODIFY_IF))}});
    regs.push_back({Dyninst::x86_64::df,  {!!(r & X86_EFLAGS_TEST_DF), !!((r & X86_EFLAGS_SET_DF) || (r & X86_EFLAGS_RESET_DF) || (r & X86_EFLAGS_MODIFY_DF))}});
    regs.push_back({Dyninst::x86_64::nt_, {!!(r & X86_EFLAGS_TEST_NT), !!(                           (r & X86_EFLAGS_RESET_NT) || (r & X86_EFLAGS_MODIFY_NT))}});
    regs.push_back({Dyninst::x86_64::rf,  {!!(r & X86_EFLAGS_TEST_RF), !!(                           (r & X86_EFLAGS_RESET_RF) || (r & X86_EFLAGS_MODIFY_RF))}});
    return regs;
  }
  if(m == CS_MODE_32) {
    regs.push_back({Dyninst::x86::af,  {!!(r & X86_EFLAGS_TEST_AF), !!((r & X86_EFLAGS_SET_AF) || (r & X86_EFLAGS_RESET_AF) || (r & X86_EFLAGS_MODIFY_AF))}});
    regs.push_back({Dyninst::x86::cf,  {!!(r & X86_EFLAGS_TEST_CF), !!((r & X86_EFLAGS_SET_CF) || (r & X86_EFLAGS_RESET_CF) || (r & X86_EFLAGS_MODIFY_CF))}});
    regs.push_back({Dyninst::x86::sf,  {!!(r & X86_EFLAGS_TEST_SF), !!((r & X86_EFLAGS_SET_SF) || (r & X86_EFLAGS_RESET_SF) || (r & X86_EFLAGS_MODIFY_SF))}});
    regs.push_back({Dyninst::x86::zf,  {!!(r & X86_EFLAGS_TEST_ZF), !!((r & X86_EFLAGS_SET_ZF) || (r & X86_EFLAGS_RESET_ZF) || (r & X86_EFLAGS_MODIFY_ZF))}});
    regs.push_back({Dyninst::x86::pf,  {!!(r & X86_EFLAGS_TEST_PF), !!((r & X86_EFLAGS_SET_PF) || (r & X86_EFLAGS_RESET_PF) || (r & X86_EFLAGS_MODIFY_PF))}});
    regs.push_back({Dyninst::x86::of,  {!!(r & X86_EFLAGS_TEST_OF), !!((r & X86_EFLAGS_SET_OF) || (r & X86_EFLAGS_RESET_OF) || (r & X86_EFLAGS_MODIFY_OF))}});
    regs.push_back({Dyninst::x86::tf,  {!!(r & X86_EFLAGS_TEST_TF), !!(                           (r & X86_EFLAGS_RESET_TF) || (r & X86_EFLAGS_MODIFY_TF))}});
    regs.push_back({Dyninst::x86::if_, {!!(r & X86_EFLAGS_TEST_IF), !!((r & X86_EFLAGS_SET_IF) || (r & X86_EFLAGS_RESET_IF) || (r & X86_EFLAGS_MODIFY_IF))}});
    regs.push_back({Dyninst::x86::df,  {!!(r & X86_EFLAGS_TEST_DF), !!((r & X86_EFLAGS_SET_DF) || (r & X86_EFLAGS_RESET_DF) || (r & X86_EFLAGS_MODIFY_DF))}});
    regs.push_back({Dyninst::x86::nt_, {!!(r & X86_EFLAGS_TEST_NT), !!(                           (r & X86_EFLAGS_RESET_NT) || (r & X86_EFLAGS_MODIFY_NT))}});
    regs.push_back({Dyninst::x86::rf,  {!!(r & X86_EFLAGS_TEST_RF), !!(                           (r & X86_EFLAGS_RESET_RF) || (r & X86_EFLAGS_MODIFY_RF))}});
    return regs;
  }

  return regs;
}
/* clang-format  on */
