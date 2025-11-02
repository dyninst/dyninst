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

#ifndef INSTRUCTIONAPI_X86_DECODER_H
#define INSTRUCTIONAPI_X86_DECODER_H

#include "capstone/capstone.h"
#include "InstructionDecoderImpl.h"
#include "Result.h"

namespace Dyninst { namespace InstructionAPI {

  class x86_decoder final : public InstructionDecoderImpl {
  public:
    struct disassem final {
      csh handle{};
      cs_insn *insn{};
    };

  private:
    cs_mode mode{};
    disassem disassembler{};

  public:
    x86_decoder(Dyninst::Architecture a);
    x86_decoder() = delete;
    x86_decoder(x86_decoder const &) = delete;
    x86_decoder &operator=(x86_decoder const &) = delete;
    x86_decoder(x86_decoder &&) = delete;
    x86_decoder &operator=(x86_decoder &&) = delete;
    ~x86_decoder();

    Instruction decode(InstructionDecoder::buffer&) override;

  private:
    void decode_operands(Instruction&);
  };

}}

#endif
