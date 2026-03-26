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

#ifndef DYNINST_COMMON_IA32_ENTRY_H
#define DYNINST_COMMON_IA32_ENTRY_H

#include "concurrent.h"
#include "entryIDs.h"
#include "ia32_locations.h"

#include <string>

namespace NS_x86 {

  DYNINST_EXPORT extern dyn_hash_map<entryID, std::string> entryNames_IAPI;
  DYNINST_EXPORT extern dyn_hash_map<prefixEntryID, std::string> prefixEntryNames_IAPI;

  struct ia32_condition {
    bool is;
    // TODO: add a field/hack for ECX [not needed for CMOVcc, but for Jcc]
    int tttn;

    ia32_condition() : is(false), tttn(-1) {}

    void set(int _tttn) {
      is = true;
      tttn = _tttn;
    }
  };

  struct ia32_operand {  // operand as given in Intel book tables
    unsigned int admet;  // addressing method
    unsigned int optype; // operand type;
  };

  // An instruction table entry
  struct ia32_entry {
    const char *name(ia32_locations *locs = NULL);
    entryID getID(ia32_locations *locs = NULL) const;
    entryID id;
    unsigned int otable;      // which opcode table is next; if t_done it is the current one
    unsigned char tabidx;     // at what index to look, 0 if it easy to deduce from opcode
    bool hasModRM;            // true if the instruction has a MOD/RM byte
    ia32_operand operands[4]; // operand descriptors
    unsigned int legacyType;  // legacy type of the instruction (e.g. (IS_CALL | REL_W))
    // code to decode memory access - this field should be seen as two 16 bit fields
    // the lower half gives operand semantics, e.g. s1RW2R, the upper half is a fXXX hack if needed
    // before hating me for this: it takes a LOT less time to add ONE field to ~2000 table lines!
    // The upper 3 bits of this field (bits 29, 30, 31) are specifiers for implicit operands.
    unsigned int opsema;

    unsigned int impl_dec; /* Implicit operands and decoration descriptions */
  };

}

#endif
