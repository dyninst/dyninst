#include "x86/decoder.h"
#include "x86/register-xlat.h"

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "mnemonic-xlat.h"

namespace Dyninst { namespace InstructionAPI {

  x86_decoder::x86_decoder(Dyninst::Architecture a): InstructionDecoderImpl(a),
      mode{a == Dyninst::Arch_x86_64 ? CS_MODE_64 : CS_MODE_32} {}

  x86_decoder::~x86_decoder() {}

  void x86_decoder::doDelayedDecode(Instruction const*) {}
  void x86_decoder::decodeOpcode(InstructionDecoder::buffer&) {}
  bool x86_decoder::decodeOperands(Instruction const*) { return true; }

}}
