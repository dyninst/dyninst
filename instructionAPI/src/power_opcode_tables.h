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

#ifndef DYNINST_INSTRUCTIONAPI_POWER_OPCODE_TABLES_H
#define DYNINST_INSTRUCTIONAPI_POWER_OPCODE_TABLES_H

#include "dyninst_visibility.h"
#include "InstructionDecoder-power.h"

#include <map>
#include <vector>

namespace Dyninst { namespace InstructionAPI {

  typedef void (InstructionDecoder_power::*operandFactory)();
  typedef std::vector<operandFactory> operandSpec;
  typedef const power_entry &(InstructionDecoder_power::*nextTableFunc)();
  typedef std::map<unsigned int, power_entry> power_table;

  struct DYNINST_EXPORT power_entry {
    power_entry(entryID o, const char *m, nextTableFunc next, operandSpec ops)
        : op(o), mnemonic(m), next_table(next), operands(ops) {}

    power_entry() : op(power_op_INVALID), mnemonic("INVALID"), next_table(NULL) {
      operands.reserve(5);
    }

    power_entry(const power_entry &o)
        : op(o.op), mnemonic(o.mnemonic), next_table(o.next_table), operands(o.operands) {}

    const power_entry &operator=(const power_entry &rhs) {
      operands.reserve(rhs.operands.size());
      op = rhs.op;
      mnemonic = rhs.mnemonic;
      next_table = rhs.next_table;
      operands = rhs.operands;
      return *this;
    }

    entryID op;
    const char *mnemonic;
    nextTableFunc next_table;
    operandSpec operands;
    static void buildTables();
    static std::vector<power_entry> main_opcode_table;
    static power_table extended_op_0;
    static power_table extended_op_4;
    static power_table extended_op_4_1409;
    static power_table extended_op_4_1538;
    static power_table extended_op_4_1921;
    static power_table extended_op_19;
    static power_table extended_op_30;
    static power_table extended_op_31;
    static power_table extended_op_57;
    static power_table extended_op_58;
    static power_table extended_op_59;
    static power_table extended_op_60;
    static power_table extended_op_60_specials;
    static power_table extended_op_60_347;
    static power_table extended_op_60_475;
    static power_table extended_op_61;
    static power_table extended_op_63;
    static power_table extended_op_63_583;
    static power_table extended_op_63_804;
    static power_table extended_op_63_836;
  };

  extern power_entry invalid_entry;
}}

#endif
