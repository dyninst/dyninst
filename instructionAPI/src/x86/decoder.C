#include "x86/decoder.h"
#include "x86/register-xlat.h"

#include "capstone/capstone.h"
#include "capstone/x86.h"

#include "x86/decoder.h"
#include "x86/register-xlat.h"
#include "mnemonic-xlat.h"
#include "debug.h"

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
  
  void x86_decoder::doDelayedDecode(Instruction const*) {}
  
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

}}
