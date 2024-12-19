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
#include "registers/ppc64_regs.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  Dyninst::MachRegister ppc64_from_dwarf(int encoding) {
    switch(encoding) {
      case 0: return Dyninst::ppc64::r0;
      case 1: return Dyninst::ppc64::r1;
      case 2: return Dyninst::ppc64::r2;
      case 3: return Dyninst::ppc64::r3;
      case 4: return Dyninst::ppc64::r4;
      case 5: return Dyninst::ppc64::r5;
      case 6: return Dyninst::ppc64::r6;
      case 7: return Dyninst::ppc64::r7;
      case 8: return Dyninst::ppc64::r8;
      case 9: return Dyninst::ppc64::r9;
      case 10: return Dyninst::ppc64::r10;
      case 11: return Dyninst::ppc64::r11;
      case 12: return Dyninst::ppc64::r12;
      case 13: return Dyninst::ppc64::r13;
      case 14: return Dyninst::ppc64::r14;
      case 15: return Dyninst::ppc64::r15;
      case 16: return Dyninst::ppc64::r16;
      case 17: return Dyninst::ppc64::r17;
      case 18: return Dyninst::ppc64::r18;
      case 19: return Dyninst::ppc64::r19;
      case 20: return Dyninst::ppc64::r20;
      case 21: return Dyninst::ppc64::r21;
      case 22: return Dyninst::ppc64::r22;
      case 23: return Dyninst::ppc64::r23;
      case 24: return Dyninst::ppc64::r24;
      case 25: return Dyninst::ppc64::r25;
      case 26: return Dyninst::ppc64::r26;
      case 27: return Dyninst::ppc64::r27;
      case 28: return Dyninst::ppc64::r28;
      case 29: return Dyninst::ppc64::r29;
      case 30: return Dyninst::ppc64::r30;
      case 31: return Dyninst::ppc64::r31;
      case 32: return Dyninst::ppc64::fpr0;
      case 33: return Dyninst::ppc64::fpr1;
      case 34: return Dyninst::ppc64::fpr2;
      case 35: return Dyninst::ppc64::fpr3;
      case 36: return Dyninst::ppc64::fpr4;
      case 37: return Dyninst::ppc64::fpr5;
      case 38: return Dyninst::ppc64::fpr6;
      case 39: return Dyninst::ppc64::fpr7;
      case 40: return Dyninst::ppc64::fpr8;
      case 41: return Dyninst::ppc64::fpr9;
      case 42: return Dyninst::ppc64::fpr10;
      case 43: return Dyninst::ppc64::fpr11;
      case 44: return Dyninst::ppc64::fpr12;
      case 45: return Dyninst::ppc64::fpr13;
      case 46: return Dyninst::ppc64::fpr14;
      case 47: return Dyninst::ppc64::fpr15;
      case 48: return Dyninst::ppc64::fpr16;
      case 49: return Dyninst::ppc64::fpr17;
      case 50: return Dyninst::ppc64::fpr18;
      case 51: return Dyninst::ppc64::fpr19;
      case 52: return Dyninst::ppc64::fpr20;
      case 53: return Dyninst::ppc64::fpr21;
      case 54: return Dyninst::ppc64::fpr22;
      case 55: return Dyninst::ppc64::fpr23;
      case 56: return Dyninst::ppc64::fpr24;
      case 57: return Dyninst::ppc64::fpr25;
      case 58: return Dyninst::ppc64::fpr26;
      case 59: return Dyninst::ppc64::fpr27;
      case 60: return Dyninst::ppc64::fpr28;
      case 61: return Dyninst::ppc64::fpr29;
      case 62: return Dyninst::ppc64::fpr30;
      case 63: return Dyninst::ppc64::fpr31;
      case 64: return Dyninst::ppc64::cr;
      case 65: return Dyninst::InvalidReg; // FPSCR
      case 100: return Dyninst::ppc64::mq;
      case 101: return Dyninst::ppc64::xer;
      case 102: return Dyninst::InvalidReg;
      case 103: return Dyninst::InvalidReg;
      case 104: return Dyninst::InvalidReg; // RTCU
      case 105: return Dyninst::InvalidReg; // RTCL
      case 106: return Dyninst::InvalidReg;
      case 107: return Dyninst::InvalidReg;
      case 108: return Dyninst::ppc64::lr;
      case 109: return Dyninst::ppc64::ctr;
      default: return Dyninst::InvalidReg;
    }
  }

  int ppc64_to_dwarf(Dyninst::MachRegister reg) {
    switch(reg.val()) {
      case Dyninst::ppc64::ir0: return 0;
      case Dyninst::ppc64::ir1: return 1;
      case Dyninst::ppc64::ir2: return 2;
      case Dyninst::ppc64::ir3: return 3;
      case Dyninst::ppc64::ir4: return 4;
      case Dyninst::ppc64::ir5: return 5;
      case Dyninst::ppc64::ir6: return 6;
      case Dyninst::ppc64::ir7: return 7;
      case Dyninst::ppc64::ir8: return 8;
      case Dyninst::ppc64::ir9: return 9;
      case Dyninst::ppc64::ir10: return 10;
      case Dyninst::ppc64::ir11: return 11;
      case Dyninst::ppc64::ir12: return 12;
      case Dyninst::ppc64::ir13: return 13;
      case Dyninst::ppc64::ir14: return 14;
      case Dyninst::ppc64::ir15: return 15;
      case Dyninst::ppc64::ir16: return 16;
      case Dyninst::ppc64::ir17: return 17;
      case Dyninst::ppc64::ir18: return 18;
      case Dyninst::ppc64::ir19: return 19;
      case Dyninst::ppc64::ir20: return 20;
      case Dyninst::ppc64::ir21: return 21;
      case Dyninst::ppc64::ir22: return 22;
      case Dyninst::ppc64::ir23: return 23;
      case Dyninst::ppc64::ir24: return 24;
      case Dyninst::ppc64::ir25: return 25;
      case Dyninst::ppc64::ir26: return 26;
      case Dyninst::ppc64::ir27: return 27;
      case Dyninst::ppc64::ir28: return 28;
      case Dyninst::ppc64::ir29: return 29;
      case Dyninst::ppc64::ir30: return 30;
      case Dyninst::ppc64::ir31: return 31;
      case Dyninst::ppc64::ifpr0: return 32;
      case Dyninst::ppc64::ifpr1: return 33;
      case Dyninst::ppc64::ifpr2: return 34;
      case Dyninst::ppc64::ifpr3: return 35;
      case Dyninst::ppc64::ifpr4: return 36;
      case Dyninst::ppc64::ifpr5: return 37;
      case Dyninst::ppc64::ifpr6: return 38;
      case Dyninst::ppc64::ifpr7: return 39;
      case Dyninst::ppc64::ifpr8: return 40;
      case Dyninst::ppc64::ifpr9: return 41;
      case Dyninst::ppc64::ifpr10: return 42;
      case Dyninst::ppc64::ifpr11: return 43;
      case Dyninst::ppc64::ifpr12: return 44;
      case Dyninst::ppc64::ifpr13: return 45;
      case Dyninst::ppc64::ifpr14: return 46;
      case Dyninst::ppc64::ifpr15: return 47;
      case Dyninst::ppc64::ifpr16: return 48;
      case Dyninst::ppc64::ifpr17: return 49;
      case Dyninst::ppc64::ifpr18: return 50;
      case Dyninst::ppc64::ifpr19: return 51;
      case Dyninst::ppc64::ifpr20: return 52;
      case Dyninst::ppc64::ifpr21: return 53;
      case Dyninst::ppc64::ifpr22: return 54;
      case Dyninst::ppc64::ifpr23: return 55;
      case Dyninst::ppc64::ifpr24: return 56;
      case Dyninst::ppc64::ifpr25: return 57;
      case Dyninst::ppc64::ifpr26: return 58;
      case Dyninst::ppc64::ifpr27: return 59;
      case Dyninst::ppc64::ifpr28: return 60;
      case Dyninst::ppc64::ifpr29: return 61;
      case Dyninst::ppc64::ifpr30: return 62;
      case Dyninst::ppc64::ifpr31: return 63;
      case Dyninst::ppc64::icr: return 64;
      case Dyninst::ppc64::imq: return 100;
      case Dyninst::ppc64::ixer: return 101;
      case Dyninst::ppc64::ilr: return 108;
      case Dyninst::ppc64::ictr: return 109;
      default: return -1;
    }
  }

}}
