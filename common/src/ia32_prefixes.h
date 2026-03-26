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

#ifndef DYNINST_COMMON_IA32_PREFIXES_H
#define DYNINST_COMMON_IA32_PREFIXES_H

#include "dyninst_visibility.h"
#include "encoding-x86.h"
#include "ia32_locations.h"

#include <cassert>

namespace NS_x86 {

  /** Returns the immediate operand of an instruction **/
  DYNINST_EXPORT int count_prefixes(unsigned insnType);

  // Copy all prefixes but the Operand-Size and Address-Size prefixes (0x66 and 0x67)
  DYNINST_EXPORT unsigned copy_prefixes_nosize(const unsigned char *&origInsn,
                                               unsigned char *&newInsn, unsigned insnType);

  DYNINST_EXPORT unsigned copy_prefixes(const unsigned char *&origInsn, unsigned char *&newInsn,
                                        unsigned insnType);

  class ia32_instruction;

  class ia32_prefixes {
    friend bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn,
                                     bool mode_64);
    friend bool ia32_decode_rex(const unsigned char *addr, ia32_prefixes &, ia32_locations *loc);

  private:
    unsigned int count;
    // At most 4 prefixes are allowed for Intel 32-bit CPUs
    // There also 4 groups, so this array is 0 if no prefix
    // from that group is present, otherwise it contains the
    // prefix opcode
    // For 64-bit CPUs, an additional REX prefix is possible,
    // so this array is extended to 5 elements
    unsigned char prfx[5];
    unsigned char opcode_prefix;

  public:
    unsigned int getCount() const {
      return count;
    }

    unsigned char getPrefix(unsigned char group) const {
      assert(group <= 4);
      return prfx[group];
    }

    bool rexW() const {
      return prfx[4] & 0x8;
    }

    bool rexR() const {
      return prfx[4] & 0x4;
    }

    bool rexX() const {
      return prfx[4] & 0x2;
    }

    bool rexB() const {
      return prfx[4] & 0x1;
    }

    unsigned char getOpcodePrefix() const {
      return opcode_prefix;
    }

    unsigned char getAddrSzPrefix() const {
      return prfx[3];
    }

    unsigned char getOperSzPrefix() const {
      return prfx[2];
    }

    /* Because VEX fields are based on the VEX type, they are decoded immediately. */
    bool vex_present;            /* Does this instruction have a vex prefix?  */
    bool XOP;                    /* whether this instrucxtion is an XOP instrucxtion */
    VEX_TYPE vex_type;           /* If there is a vex prefix present, what type is it? */
    unsigned char vex_prefix[5]; /* Support up to EVEX (VEX-512) */
    int vex_sse_mult;            /* index for sse multiplexer table */
    int vex_vvvv_reg;            /* The register specified by this prefix. */
    int vex_ll;                  /* l bit for VEX2, VEX3 or ll for EVEX */
    int vex_pp;                  /* pp bits for VEX2, VEX3 or EVEX */
    int vex_m_mmmm;              /* m-mmmm bits for VEX2, VEX3 or EVEX */
    int vex_w;                   /* w bit for VEX2, VEX3 or EVEX */
    int vex_V;                   /* V' modifier for EVEX */
    int vex_r;                   /* The VEX REXR bit for VEX2, VEX3 or EVEX*/
    int vex_R;                   /* The VEX REXR' bit for EVEX */
    int vex_x;                   /* The VEX REXX bit for VEX2, VEX3 or EVEX */
    int vex_b;                   /* The VEX REXB bit for VEX2, VEX3 or EVEX */
    int vex_aaa;                 /* Selects the vector mask register for EVEX */
  };
}

#endif
