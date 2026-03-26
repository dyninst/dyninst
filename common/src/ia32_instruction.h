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

#ifndef COMMON_IA32_INSTRUCTION_H
#define COMMON_IA32_INSTRUCTION_H

#include "ia32_entry.h"
#include "ia32_memacc.h"
#include "ia32_prefixes.h"

namespace NS_x86 {

  class ia32_instruction {
    friend unsigned int ia32_decode_operands(const ia32_prefixes &pref, const ia32_entry &gotit,
                                             const char *addr, ia32_instruction &instruct);
    friend bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn,
                                     bool mode_64);
    friend ia32_instruction &ia32_decode(unsigned int capa, const unsigned char *addr,
                                         ia32_instruction &instruct, bool mode_64);
    friend int ia32_decode_opcode(unsigned int capa, const unsigned char *addr,
                                  ia32_instruction &instruct, ia32_entry **gotit_ret, bool mode_64);
    friend unsigned int ia32_decode_operands(const ia32_prefixes &pref, const ia32_entry &gotit,
                                             const unsigned char *addr, ia32_instruction &instruct,
                                             ia32_memacc *mac, bool mode_64);
    friend ia32_instruction &ia32_decode_FP(const ia32_prefixes &pref, const unsigned char *addr,
                                            ia32_instruction &instruct);
    friend unsigned int ia32_emulate_old_type(ia32_instruction &instruct, bool mode_64);
    friend ia32_instruction &ia32_decode_FP(unsigned int opcode, const ia32_prefixes &pref,
                                            const unsigned char *addr, ia32_instruction &instruct,
                                            ia32_entry *entry, ia32_memacc *mac, bool mode_64);

    unsigned int size;
    ia32_prefixes prf;
    ia32_memacc *mac;
    ia32_condition *cond;
    ia32_entry *entry;
    ia32_locations *loc;
    unsigned int legacy_type;
    bool rip_relative_data;

  public:
    ia32_instruction(ia32_memacc *_mac = NULL, ia32_condition *_cnd = NULL,
                     ia32_locations *loc_ = NULL)
        : size(0), prf(), mac(_mac), cond(_cnd), entry(NULL), loc(loc_), legacy_type(0),
          rip_relative_data(false) {}

    ia32_entry *getEntry() {
      return entry;
    }

    unsigned int getSize() const {
      return size;
    }

    unsigned int getPrefixCount() const {
      return prf.getCount();
    }

    ia32_prefixes *getPrefix() {
      return &prf;
    }

    unsigned int getLegacyType() const {
      return legacy_type;
    }

    bool hasRipRelativeData() const {
      return rip_relative_data;
    }

    const ia32_memacc &getMac(int which) const {
      return mac[which];
    }

    const ia32_condition &getCond() const {
      return *cond;
    }

    const ia32_locations &getLocationInfo() const {
      return *loc;
    }
  };

}

#endif
