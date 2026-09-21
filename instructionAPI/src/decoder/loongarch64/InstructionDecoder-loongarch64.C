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

#include "InstructionDecoder-loongarch64.h"
#include "Instruction.h"
#include "common/src/arch-loongarch64.h"

using namespace NS_loongarch64;

namespace Dyninst {
namespace InstructionAPI {

InstructionDecoder_loongarch64::InstructionDecoder_loongarch64(Architecture a)
    : InstructionDecoderImpl(a), is64BitMode(true) {
}

InstructionDecoder_loongarch64::~InstructionDecoder_loongarch64() {
}

Instruction InstructionDecoder_loongarch64::decode(InstructionDecoder::buffer &b) {
    assert(0 && "InstructionDecoder_loongarch64::decode not implemented");
    return Instruction();
}

void InstructionDecoder_loongarch64::setMode(bool is64) {
    is64BitMode = is64;
}

void InstructionDecoder_loongarch64::decodeOpcode() {
    assert(0 && "decodeOpcode not implemented");
}

void InstructionDecoder_loongarch64::decodeOperands() {
    assert(0 && "decodeOperands not implemented");
}

void InstructionDecoder_loongarch64::decodeALU() {
    assert(0 && "decodeALU not implemented");
}

void InstructionDecoder_loongarch64::decodeBranch() {
    assert(0 && "decodeBranch not implemented");
}

void InstructionDecoder_loongarch64::decodeLoadStore() {
    assert(0 && "decodeLoadStore not implemented");
}

void InstructionDecoder_loongarch64::decodeSpecial() {
    assert(0 && "decodeSpecial not implemented");
}

} // namespace InstructionAPI
} // namespace Dyninst