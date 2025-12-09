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

#ifndef DYNINST_TESTS_INSTRUCTIONAPI_OPCODE_TESTS_H
#define DYNINST_TESTS_INSTRUCTIONAPI_OPCODE_TESTS_H

#include "Instruction.h"

namespace Dyninst {
namespace InstructionAPI {

struct opcode_test {
  entryID opcode;
  entryID encoded_opcode;
  std::string opcode_mnemonic;
  std::string encoded_opcode_mnemonic;
  opcode_test(entryID op, entryID enc_op, const std::string &op_mnem,
              const std::string &enc_op_mnem)
      : opcode(op), encoded_opcode(enc_op), opcode_mnemonic(op_mnem),
        encoded_opcode_mnemonic(enc_op_mnem) {}
  // Constructor for the case where the opcode is the same as the encoded opcode
  opcode_test(entryID op, const std::string &op_mnem)
      : opcode_test(op, op, op_mnem, op_mnem) {}
};

bool verify(Instruction const &, opcode_test const &);

} // namespace InstructionAPI
} // namespace Dyninst

#endif
