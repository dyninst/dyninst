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

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "debug.h"
#include "x86/decoder.h"

/***************************************************************************
 * The work here is based on
 *
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  June 2025
 *
 *  Intel Architecture Instruction Set Extensions and Future Features (IISE)
 *  May 2021
 *
 *  AMD64 Architecture Programmer’s Manual (AMDAPM)
 *  Revision 3.33
 *  November 2021
 *
 ***************************************************************************/

namespace Dyninst { namespace InstructionAPI {

  x86_decoder::x86_decoder(Dyninst::Architecture a) : InstructionDecoderImpl(a) {

    mode = (a == Dyninst::Arch_x86_64) ? CS_MODE_64 : CS_MODE_32;

    cs_open(CS_ARCH_X86, this->mode, &disassembler.handle);
    cs_option(disassembler.handle, CS_OPT_DETAIL, CS_OPT_ON);
    disassembler.insn = cs_malloc(disassembler.handle);
  }

  x86_decoder::~x86_decoder() {
    cs_free(disassembler.insn, 1);
    cs_close(&disassembler.handle);
  }

  Instruction x86_decoder::decode(InstructionDecoder::buffer &buf) {
    return {};
  }

  void x86_decoder::decode_operands(Instruction&) {
  }

}}
