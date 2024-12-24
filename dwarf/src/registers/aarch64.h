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

#include "registers/MachRegister.h"
#include "registers/aarch64_regs.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  /*
   * DWARF Encodings
   *
   * Aarch64:
   *  DWARF for the Arm 64-bit Architecture
   *  6th October 2023
   *  4.1 DWARF register names
   *  https://github.com/ARM-software/abi-aa/releases/download/2023Q3/aadwarf64.pdf
   */

  Dyninst::MachRegister aarch64_from_dwarf(int encoding) {
    switch(encoding) {
      // 64-bit general-purpose registers
      case 0: return Dyninst::aarch64::x0;
      case 1: return Dyninst::aarch64::x1;
      case 2: return Dyninst::aarch64::x2;
      case 3: return Dyninst::aarch64::x3;
      case 4: return Dyninst::aarch64::x4;
      case 5: return Dyninst::aarch64::x5;
      case 6: return Dyninst::aarch64::x6;
      case 7: return Dyninst::aarch64::x7;
      case 8: return Dyninst::aarch64::x8;
      case 9: return Dyninst::aarch64::x9;
      case 10: return Dyninst::aarch64::x10;
      case 11: return Dyninst::aarch64::x11;
      case 12: return Dyninst::aarch64::x12;
      case 13: return Dyninst::aarch64::x13;
      case 14: return Dyninst::aarch64::x14;
      case 15: return Dyninst::aarch64::x15;
      case 16: return Dyninst::aarch64::x16;
      case 17: return Dyninst::aarch64::x17;
      case 18: return Dyninst::aarch64::x18;
      case 19: return Dyninst::aarch64::x19;
      case 20: return Dyninst::aarch64::x20;
      case 21: return Dyninst::aarch64::x21;
      case 22: return Dyninst::aarch64::x22;
      case 23: return Dyninst::aarch64::x23;
      case 24: return Dyninst::aarch64::x24;
      case 25: return Dyninst::aarch64::x25;
      case 26: return Dyninst::aarch64::x26;
      case 27: return Dyninst::aarch64::x27;
      case 28: return Dyninst::aarch64::x28;
      case 29: return Dyninst::aarch64::x29;
      case 30: return Dyninst::aarch64::x30;
      case 31: return Dyninst::aarch64::sp;  // 64-bit stack pointer
      case 32: return Dyninst::aarch64::pc;  // 64-bit program counter
      case 33: return Dyninst::aarch64::elr_el1;  // Current mode exception link register
      case 34: return Dyninst::InvalidReg;  // RA_SIGN_STATE Return address signed state pseudo-register (beta)
      case 35: return Dyninst::aarch64::tpidrro_el0;  // EL0 Read-Only Software Thread ID register
      case 36: return Dyninst::aarch64::tpidr_el0;  // EL0 Read/Write Software Thread ID register
      case 37: return Dyninst::aarch64::tpidr_el1;  // EL1 Software Thread ID register
      case 38: return Dyninst::aarch64::tpidr_el2;  // EL2 Software Thread ID register
      case 39: return Dyninst::aarch64::tpidr_el3;  // EL3 Software Thread ID register

      // Reserved
      case 40:
      case 41:
      case 42:
      case 43:
      case 44:
      case 45: return Dyninst::InvalidReg;

      case 46: return Dyninst::aarch64::vg;  // 64-bit SVE vector granule pseudo-register (beta state)
      case 47: return Dyninst::aarch64::ffr;  // VG × 8-bit SVE first fault register

      // VG × 8-bit SVE predicate registers
      case 48: return Dyninst::aarch64::p0;
      case 49: return Dyninst::aarch64::p1;
      case 50: return Dyninst::aarch64::p2;
      case 51: return Dyninst::aarch64::p3;
      case 52: return Dyninst::aarch64::p4;
      case 53: return Dyninst::aarch64::p5;
      case 54: return Dyninst::aarch64::p6;
      case 55: return Dyninst::aarch64::p7;
      case 56: return Dyninst::aarch64::p8;
      case 57: return Dyninst::aarch64::p9;
      case 58: return Dyninst::aarch64::p10;
      case 59: return Dyninst::aarch64::p11;
      case 60: return Dyninst::aarch64::p12;
      case 61: return Dyninst::aarch64::p13;
      case 62: return Dyninst::aarch64::p14;
      case 63: return Dyninst::aarch64::p15;

      // 128-bit FP/Advanced SIMD registers
      case 64: return Dyninst::aarch64::q0;
      case 65: return Dyninst::aarch64::q1;
      case 66: return Dyninst::aarch64::q2;
      case 67: return Dyninst::aarch64::q3;
      case 68: return Dyninst::aarch64::q4;
      case 69: return Dyninst::aarch64::q5;
      case 70: return Dyninst::aarch64::q6;
      case 71: return Dyninst::aarch64::q7;
      case 72: return Dyninst::aarch64::q8;
      case 73: return Dyninst::aarch64::q9;
      case 74: return Dyninst::aarch64::q10;
      case 75: return Dyninst::aarch64::q11;
      case 76: return Dyninst::aarch64::q12;
      case 77: return Dyninst::aarch64::q13;
      case 78: return Dyninst::aarch64::q14;
      case 79: return Dyninst::aarch64::q15;
      case 80: return Dyninst::aarch64::q16;
      case 81: return Dyninst::aarch64::q17;
      case 82: return Dyninst::aarch64::q18;
      case 83: return Dyninst::aarch64::q19;
      case 84: return Dyninst::aarch64::q20;
      case 85: return Dyninst::aarch64::q21;
      case 86: return Dyninst::aarch64::q22;
      case 87: return Dyninst::aarch64::q23;
      case 88: return Dyninst::aarch64::q24;
      case 89: return Dyninst::aarch64::q25;
      case 90: return Dyninst::aarch64::q26;
      case 91: return Dyninst::aarch64::q27;
      case 92: return Dyninst::aarch64::q28;
      case 93: return Dyninst::aarch64::q29;
      case 94: return Dyninst::aarch64::q30;
      case 95: return Dyninst::aarch64::q31;

      // VG × 64-bit SVE vector registers (beta state)
      case 96: return Dyninst::aarch64::z0;
      case 97: return Dyninst::aarch64::z1;
      case 98: return Dyninst::aarch64::z2;
      case 99: return Dyninst::aarch64::z3;
      case 100: return Dyninst::aarch64::z4;
      case 101: return Dyninst::aarch64::z5;
      case 102: return Dyninst::aarch64::z6;
      case 103: return Dyninst::aarch64::z7;
      case 104: return Dyninst::aarch64::z8;
      case 105: return Dyninst::aarch64::z9;
      case 106: return Dyninst::aarch64::z10;
      case 107: return Dyninst::aarch64::z11;
      case 108: return Dyninst::aarch64::z12;
      case 109: return Dyninst::aarch64::z13;
      case 110: return Dyninst::aarch64::z14;
      case 111: return Dyninst::aarch64::z15;
      case 112: return Dyninst::aarch64::z16;
      case 113: return Dyninst::aarch64::z17;
      case 114: return Dyninst::aarch64::z18;
      case 115: return Dyninst::aarch64::z19;
      case 116: return Dyninst::aarch64::z20;
      case 117: return Dyninst::aarch64::z21;
      case 118: return Dyninst::aarch64::z22;
      case 119: return Dyninst::aarch64::z23;
      case 120: return Dyninst::aarch64::z24;
      case 121: return Dyninst::aarch64::z25;
      case 122: return Dyninst::aarch64::z26;
      case 123: return Dyninst::aarch64::z27;
      case 124: return Dyninst::aarch64::z28;
      case 125: return Dyninst::aarch64::z29;
      case 126: return Dyninst::aarch64::z30;
      case 127: return Dyninst::aarch64::z31;
      default: return Dyninst::InvalidReg; break;
    }
  }

  int aarch64_to_dwarf(Dyninst::MachRegister reg) {
    switch(reg.val()) {
      case Dyninst::aarch64::ix0: return 0;
      case Dyninst::aarch64::ix1: return 1;
      case Dyninst::aarch64::ix2: return 2;
      case Dyninst::aarch64::ix3: return 3;
      case Dyninst::aarch64::ix4: return 4;
      case Dyninst::aarch64::ix5: return 5;
      case Dyninst::aarch64::ix6: return 6;
      case Dyninst::aarch64::ix7: return 7;
      case Dyninst::aarch64::ix8: return 8;
      case Dyninst::aarch64::ix9: return 9;
      case Dyninst::aarch64::ix10: return 10;
      case Dyninst::aarch64::ix11: return 11;
      case Dyninst::aarch64::ix12: return 12;
      case Dyninst::aarch64::ix13: return 13;
      case Dyninst::aarch64::ix14: return 14;
      case Dyninst::aarch64::ix15: return 15;
      case Dyninst::aarch64::ix16: return 16;
      case Dyninst::aarch64::ix17: return 17;
      case Dyninst::aarch64::ix18: return 18;
      case Dyninst::aarch64::ix19: return 19;
      case Dyninst::aarch64::ix20: return 20;
      case Dyninst::aarch64::ix21: return 21;
      case Dyninst::aarch64::ix22: return 22;
      case Dyninst::aarch64::ix23: return 23;
      case Dyninst::aarch64::ix24: return 24;
      case Dyninst::aarch64::ix25: return 25;
      case Dyninst::aarch64::ix26: return 26;
      case Dyninst::aarch64::ix27: return 27;
      case Dyninst::aarch64::ix28: return 28;
      case Dyninst::aarch64::ix29: return 29;
      case Dyninst::aarch64::ix30: return 30;
      case Dyninst::aarch64::isp: return 31;  // 64-bit stack pointer
      case Dyninst::aarch64::ipc: return 32;  // 64-bit program counter
      case Dyninst::aarch64::ielr_el1: return 33;  // Current mode exception link register

      // 34 is invalid -- RA_SIGN_STATE Return address signed state pseudo-register (beta)

      case Dyninst::aarch64::itpidrro_el0: return 35;  // EL0 Read-Only Software Thread ID register
      case Dyninst::aarch64::itpidr_el0: return 36;  // EL0 Read/Write Software Thread ID register
      case Dyninst::aarch64::itpidr_el1: return 37;  // EL1 Software Thread ID register
      case Dyninst::aarch64::itpidr_el2: return 38;  // EL2 Software Thread ID register
      case Dyninst::aarch64::itpidr_el3: return 39;  // EL3 Software Thread ID register

      // 40 to 45 are reserved

      case Dyninst::aarch64::ivg: return 46;  // 64-bit SVE vector granule pseudo-register (beta state)
      case Dyninst::aarch64::iffr: return 47;  // VG × 8-bit SVE first fault register

      // VG × 8-bit SVE predicate registers
      case Dyninst::aarch64::ip0: return 48;
      case Dyninst::aarch64::ip1: return 49;
      case Dyninst::aarch64::ip2: return 50;
      case Dyninst::aarch64::ip3: return 51;
      case Dyninst::aarch64::ip4: return 52;
      case Dyninst::aarch64::ip5: return 53;
      case Dyninst::aarch64::ip6: return 54;
      case Dyninst::aarch64::ip7: return 55;
      case Dyninst::aarch64::ip8: return 56;
      case Dyninst::aarch64::ip9: return 57;
      case Dyninst::aarch64::ip10: return 58;
      case Dyninst::aarch64::ip11: return 59;
      case Dyninst::aarch64::ip12: return 60;
      case Dyninst::aarch64::ip13: return 61;
      case Dyninst::aarch64::ip14: return 62;
      case Dyninst::aarch64::ip15: return 63;

      // 128-bit FP/Advanced SIMD registers
      case Dyninst::aarch64::iq0: return 64;
      case Dyninst::aarch64::iq1: return 65;
      case Dyninst::aarch64::iq2: return 66;
      case Dyninst::aarch64::iq3: return 67;
      case Dyninst::aarch64::iq4: return 68;
      case Dyninst::aarch64::iq5: return 69;
      case Dyninst::aarch64::iq6: return 70;
      case Dyninst::aarch64::iq7: return 71;
      case Dyninst::aarch64::iq8: return 72;
      case Dyninst::aarch64::iq9: return 73;
      case Dyninst::aarch64::iq10: return 74;
      case Dyninst::aarch64::iq11: return 75;
      case Dyninst::aarch64::iq12: return 76;
      case Dyninst::aarch64::iq13: return 77;
      case Dyninst::aarch64::iq14: return 78;
      case Dyninst::aarch64::iq15: return 79;
      case Dyninst::aarch64::iq16: return 80;
      case Dyninst::aarch64::iq17: return 81;
      case Dyninst::aarch64::iq18: return 82;
      case Dyninst::aarch64::iq19: return 83;
      case Dyninst::aarch64::iq20: return 84;
      case Dyninst::aarch64::iq21: return 85;
      case Dyninst::aarch64::iq22: return 86;
      case Dyninst::aarch64::iq23: return 87;
      case Dyninst::aarch64::iq24: return 88;
      case Dyninst::aarch64::iq25: return 89;
      case Dyninst::aarch64::iq26: return 90;
      case Dyninst::aarch64::iq27: return 91;
      case Dyninst::aarch64::iq28: return 92;
      case Dyninst::aarch64::iq29: return 93;
      case Dyninst::aarch64::iq30: return 94;
      case Dyninst::aarch64::iq31: return 95;

      // VG × 64-bit SVE vector registers (beta state)
      case Dyninst::aarch64::iz0: return 96;
      case Dyninst::aarch64::iz1: return 97;
      case Dyninst::aarch64::iz2: return 98;
      case Dyninst::aarch64::iz3: return 99;
      case Dyninst::aarch64::iz4: return 100;
      case Dyninst::aarch64::iz5: return 101;
      case Dyninst::aarch64::iz6: return 102;
      case Dyninst::aarch64::iz7: return 103;
      case Dyninst::aarch64::iz8: return 104;
      case Dyninst::aarch64::iz9: return 105;
      case Dyninst::aarch64::iz10: return 106;
      case Dyninst::aarch64::iz11: return 107;
      case Dyninst::aarch64::iz12: return 108;
      case Dyninst::aarch64::iz13: return 109;
      case Dyninst::aarch64::iz14: return 110;
      case Dyninst::aarch64::iz15: return 111;
      case Dyninst::aarch64::iz16: return 112;
      case Dyninst::aarch64::iz17: return 113;
      case Dyninst::aarch64::iz18: return 114;
      case Dyninst::aarch64::iz19: return 115;
      case Dyninst::aarch64::iz20: return 116;
      case Dyninst::aarch64::iz21: return 117;
      case Dyninst::aarch64::iz22: return 118;
      case Dyninst::aarch64::iz23: return 119;
      case Dyninst::aarch64::iz24: return 120;
      case Dyninst::aarch64::iz25: return 121;
      case Dyninst::aarch64::iz26: return 122;
      case Dyninst::aarch64::iz27: return 123;
      case Dyninst::aarch64::iz28: return 124;
      case Dyninst::aarch64::iz29: return 125;
      case Dyninst::aarch64::iz30: return 126;
      case Dyninst::aarch64::iz31: return 127;
      default: return -1;
    }
  }

}}
