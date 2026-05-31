/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "InstructionDecoderImpl.h"
#include "entryIDs.h"

#include <algorithm>
#include <array>

#if defined(DYNINST_ENABLE_ZYDIS)
#include <Zydis/Zydis.h>
#endif

namespace {
  namespace ia = Dyninst::InstructionAPI;
  using ui = ia::InstructionDecoder::unknown_instruction;
  ui::callback_t callback{};

#if defined(DYNINST_ENABLE_ZYDIS)
  // Built-in "unknown instruction" callback for x86-64. InstructionAPI calls
  // this when it cannot decode a byte sequence; we only need the length of the
  // instruction so the decoder can skip over it, so we hand back a no-op of the
  // recovered length (its bytes and control flow are never inspected).
  ia::Instruction default_x86_callback(ia::InstructionDecoder::buffer seqn) {
    using namespace Dyninst;
    auto const buf_len = static_cast<size_t>(seqn.end - seqn.start);

    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZydisDecodedInstruction insn;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    ZyanStatus status =
        ZydisDecoderDecodeFull(&decoder, seqn.start, buf_len, &insn, operands);
    if(ZYAN_SUCCESS(status) && insn.length > 0) {
      return ia::Instruction{ia::Operation{e_nop, "nop", Arch_x86_64}, insn.length,
                             seqn.start, Arch_x86_64};
    }
    return ia::Instruction{};
  }
#endif
}

using namespace std;

namespace Dyninst { namespace InstructionAPI {
  DYNINST_EXPORT InstructionDecoder::InstructionDecoder(const unsigned char* buffer_, size_t size,
                                                        Architecture arch)
      : m_buf(buffer_, size) {
    m_Impl = InstructionDecoderImpl::makeDecoderImpl(arch);
  }

  DYNINST_EXPORT InstructionDecoder::InstructionDecoder(const void* buffer_, size_t size,
                                                        Architecture arch)
      : m_buf(reinterpret_cast<const unsigned char*>(buffer_), size) {
    m_Impl = InstructionDecoderImpl::makeDecoderImpl(arch);
  }

  DYNINST_EXPORT Instruction InstructionDecoder::decode() {
    if(m_buf.start >= m_buf.end)
      return Instruction();
    Instruction const& ins = m_Impl->decode(m_buf);

    if(!ins.isLegalInsn() && ::callback) {
      auto const buf_len = static_cast<unsigned int>(m_buf.end - m_buf.start);
      auto const size = (maxInstructionLength < buf_len) ? maxInstructionLength : buf_len;

      // Don't let the user modify the real byte stream
      std::array<unsigned char, maxInstructionLength> buf{};
      std::copy_n(m_buf.start, size, buf.data());
      buffer user_buf{buf.data(), buf.data() + size};

      auto user_ins = ::callback(user_buf);
      m_buf.start += user_ins.size();
      return user_ins;
    }
    return ins;
  }

  DYNINST_EXPORT Instruction InstructionDecoder::decode(const unsigned char* b) {
    buffer tmp(b, b + maxInstructionLength);
    return m_Impl->decode(tmp);
  }

  using cbt = InstructionDecoder::unknown_instruction::callback_t;

  void InstructionDecoder::unknown_instruction::register_callback(cbt cb) { ::callback = cb; }

  cbt InstructionDecoder::unknown_instruction::unregister_callback() {
    auto c = ::callback;
    ::callback = nullptr;
    return c;
  }

  bool InstructionDecoder::unknown_instruction::register_default_callback() {
#if defined(DYNINST_ENABLE_ZYDIS)
    ::callback = &default_x86_callback;
    return true;
#else
    return false;
#endif
  }
}}
