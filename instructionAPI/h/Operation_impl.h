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

#ifndef DYNINST_INSTRUCTIONAPI_OPERATION_IMPL_H
#define DYNINST_INSTRUCTIONAPI_OPERATION_IMPL_H

#include "Architecture.h"
#include "entryIDs.h"
#include "dyninst_visibility.h"

#include <string>

namespace Dyninst { namespace InstructionAPI {

  class DYNINST_EXPORT Operation {
  public:
    friend class InstructionDecoder_power; // for editing mnemonics after creation
    friend class InstructionDecoder_aarch64;
    friend class InstructionDecoder_amdgpu_gfx908;
    friend class InstructionDecoder_amdgpu_gfx90a;
    friend class InstructionDecoder_amdgpu_gfx940;
    friend class InstructionDecoder_riscv64;
    friend class InstructionDecoder_x86;

  public:
    Operation() = default;
    Operation(entryID id, std::string m, Architecture arch)
      : operationID(id), archDecodedFrom(arch), mnemonic{std::move(m)} {}

    std::string format() const;

    entryID getID() const;
    prefixEntryID getPrefixID() const;

    void updateMnemonic(std::string new_mnemonic) { mnemonic = std::move(new_mnemonic); }

    bool operator==(Operation const&) const;

    bool isVectorInsn{};

  private:
    mutable entryID operationID{};
    Architecture archDecodedFrom{};
    prefixEntryID prefixID{};
    mutable std::string mnemonic;
  };
}}

#endif
