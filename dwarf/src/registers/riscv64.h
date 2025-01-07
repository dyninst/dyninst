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
#include "registers/riscv64_regs.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  /*
   * DWARF Encodings
   *
   * riscv64:
   *  DWARF for the RISC-V Architecture
   *  https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-dwarf.adoc
   */

  Dyninst::MachRegister riscv64_from_dwarf(int encoding) {
    switch(encoding) {
      // 64-bit general-purpose registers
      case 0: return Dyninst::riscv64::x0;
      case 1: return Dyninst::riscv64::x1;
      case 2: return Dyninst::riscv64::x2;
      case 3: return Dyninst::riscv64::x3;
      case 4: return Dyninst::riscv64::x4;
      case 5: return Dyninst::riscv64::x5;
      case 6: return Dyninst::riscv64::x6;
      case 7: return Dyninst::riscv64::x7;
      case 8: return Dyninst::riscv64::x8;
      case 9: return Dyninst::riscv64::x9;
      case 10: return Dyninst::riscv64::x10;
      case 11: return Dyninst::riscv64::x11;
      case 12: return Dyninst::riscv64::x12;
      case 13: return Dyninst::riscv64::x13;
      case 14: return Dyninst::riscv64::x14;
      case 15: return Dyninst::riscv64::x15;
      case 16: return Dyninst::riscv64::x16;
      case 17: return Dyninst::riscv64::x17;
      case 18: return Dyninst::riscv64::x18;
      case 19: return Dyninst::riscv64::x19;
      case 20: return Dyninst::riscv64::x20;
      case 21: return Dyninst::riscv64::x21;
      case 22: return Dyninst::riscv64::x22;
      case 23: return Dyninst::riscv64::x23;
      case 24: return Dyninst::riscv64::x24;
      case 25: return Dyninst::riscv64::x25;
      case 26: return Dyninst::riscv64::x26;
      case 27: return Dyninst::riscv64::x27;
      case 28: return Dyninst::riscv64::x28;
      case 29: return Dyninst::riscv64::x29;
      case 30: return Dyninst::riscv64::x30;
      case 31: return Dyninst::riscv64::x31;

      // Floating point registers
      case 32: return Dyninst::riscv64::f0;
      case 33: return Dyninst::riscv64::f1;
      case 34: return Dyninst::riscv64::f2;
      case 35: return Dyninst::riscv64::f3;
      case 36: return Dyninst::riscv64::f4;
      case 37: return Dyninst::riscv64::f5;
      case 38: return Dyninst::riscv64::f6;
      case 39: return Dyninst::riscv64::f7;
      case 40: return Dyninst::riscv64::f8;
      case 41: return Dyninst::riscv64::f9;
      case 42: return Dyninst::riscv64::f10;
      case 43: return Dyninst::riscv64::f11;
      case 44: return Dyninst::riscv64::f12;
      case 45: return Dyninst::riscv64::f13;
      case 46: return Dyninst::riscv64::f14;
      case 47: return Dyninst::riscv64::f15;
      case 48: return Dyninst::riscv64::f16;
      case 49: return Dyninst::riscv64::f17;
      case 50: return Dyninst::riscv64::f18;
      case 51: return Dyninst::riscv64::f19;
      case 52: return Dyninst::riscv64::f20;
      case 53: return Dyninst::riscv64::f21;
      case 54: return Dyninst::riscv64::f22;
      case 55: return Dyninst::riscv64::f23;
      case 56: return Dyninst::riscv64::f24;
      case 57: return Dyninst::riscv64::f25;
      case 58: return Dyninst::riscv64::f26;
      case 59: return Dyninst::riscv64::f27;
      case 60: return Dyninst::riscv64::f28;
      case 61: return Dyninst::riscv64::f29;
      case 62: return Dyninst::riscv64::f30;
      case 63: return Dyninst::riscv64::f31;
      default: return Dyninst::InvalidReg; break;
    }
  }

  int riscv64_to_dwarf(Dyninst::MachRegister reg) {
    switch(reg.val()) {

      // 64-bit general-purpose registers
      case Dyninst::riscv64::ix0: return 0;
      case Dyninst::riscv64::ix1: return 1;
      case Dyninst::riscv64::ix2: return 2;
      case Dyninst::riscv64::ix3: return 3;
      case Dyninst::riscv64::ix4: return 4;
      case Dyninst::riscv64::ix5: return 5;
      case Dyninst::riscv64::ix6: return 6;
      case Dyninst::riscv64::ix7: return 7;
      case Dyninst::riscv64::ix8: return 8;
      case Dyninst::riscv64::ix9: return 9;
      case Dyninst::riscv64::ix10: return 10;
      case Dyninst::riscv64::ix11: return 11;
      case Dyninst::riscv64::ix12: return 12;
      case Dyninst::riscv64::ix13: return 13;
      case Dyninst::riscv64::ix14: return 14;
      case Dyninst::riscv64::ix15: return 15;
      case Dyninst::riscv64::ix16: return 16;
      case Dyninst::riscv64::ix17: return 17;
      case Dyninst::riscv64::ix18: return 18;
      case Dyninst::riscv64::ix19: return 19;
      case Dyninst::riscv64::ix20: return 20;
      case Dyninst::riscv64::ix21: return 21;
      case Dyninst::riscv64::ix22: return 22;
      case Dyninst::riscv64::ix23: return 23;
      case Dyninst::riscv64::ix24: return 24;
      case Dyninst::riscv64::ix25: return 25;
      case Dyninst::riscv64::ix26: return 26;
      case Dyninst::riscv64::ix27: return 27;
      case Dyninst::riscv64::ix28: return 28;
      case Dyninst::riscv64::ix29: return 29;
      case Dyninst::riscv64::ix30: return 30;
      case Dyninst::riscv64::ix31: return 31;

      // Floating point registers
      case Dyninst::riscv64::if0: return 32;
      case Dyninst::riscv64::if1: return 33;
      case Dyninst::riscv64::if2: return 34;
      case Dyninst::riscv64::if3: return 35;
      case Dyninst::riscv64::if4: return 36;
      case Dyninst::riscv64::if5: return 37;
      case Dyninst::riscv64::if6: return 38;
      case Dyninst::riscv64::if7: return 39;
      case Dyninst::riscv64::if8: return 40;
      case Dyninst::riscv64::if9: return 41;
      case Dyninst::riscv64::if10: return 42;
      case Dyninst::riscv64::if11: return 43;
      case Dyninst::riscv64::if12: return 44;
      case Dyninst::riscv64::if13: return 45;
      case Dyninst::riscv64::if14: return 46;
      case Dyninst::riscv64::if15: return 47;
      case Dyninst::riscv64::if16: return 48;
      case Dyninst::riscv64::if17: return 49;
      case Dyninst::riscv64::if18: return 50;
      case Dyninst::riscv64::if19: return 51;
      case Dyninst::riscv64::if20: return 52;
      case Dyninst::riscv64::if21: return 53;
      case Dyninst::riscv64::if22: return 54;
      case Dyninst::riscv64::if23: return 55;
      case Dyninst::riscv64::if24: return 56;
      case Dyninst::riscv64::if25: return 57;
      case Dyninst::riscv64::if26: return 58;
      case Dyninst::riscv64::if27: return 59;
      case Dyninst::riscv64::if28: return 60;
      case Dyninst::riscv64::if29: return 61;
      case Dyninst::riscv64::if30: return 62;
      case Dyninst::riscv64::if31: return 63;

      default: return -1;
    }
  }

}}
