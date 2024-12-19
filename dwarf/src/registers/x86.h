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
#include "registers/x86_regs.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  /*
   * DWARF Encodings
   *
   * x86:
   *  System V Application Binary Interface
   *  Intel386 Architecture Processor Supplement
   *  Version 1.0 February 3, 2015
   *  Table 2.14: DWARF Register Number Mapping
   *  https://gitlab.com/x86-psABIs/i386-ABI
   */

  Dyninst::MachRegister x86_from_dwarf(int encoding) {
    switch(encoding) {
      case 0: return Dyninst::x86::eax;
      case 1: return Dyninst::x86::ecx;
      case 2: return Dyninst::x86::edx;
      case 3: return Dyninst::x86::ebx;
      case 4: return Dyninst::x86::esp;
      case 5: return Dyninst::x86::ebp;
      case 6: return Dyninst::x86::esi;
      case 7: return Dyninst::x86::edi;
      case 8: return Dyninst::x86::eip;
      case 9: return Dyninst::x86::flags;
      case 10: return Dyninst::InvalidReg;
      case 11: return Dyninst::x86::st0;
      case 12: return Dyninst::x86::st1;
      case 13: return Dyninst::x86::st2;
      case 14: return Dyninst::x86::st3;
      case 15: return Dyninst::x86::st4;
      case 16: return Dyninst::x86::st5;
      case 17: return Dyninst::x86::st6;
      case 18: return Dyninst::x86::st7;
      case 19: return Dyninst::InvalidReg;
      case 20: return Dyninst::InvalidReg;
      case 21: return Dyninst::x86::xmm0;
      case 22: return Dyninst::x86::xmm1;
      case 23: return Dyninst::x86::xmm2;
      case 24: return Dyninst::x86::xmm3;
      case 25: return Dyninst::x86::xmm4;
      case 26: return Dyninst::x86::xmm5;
      case 27: return Dyninst::x86::xmm6;
      case 28: return Dyninst::x86::xmm7;
      case 29: return Dyninst::x86::mm0;
      case 30: return Dyninst::x86::mm1;
      case 31: return Dyninst::x86::mm2;
      case 32: return Dyninst::x86::mm3;
      case 33: return Dyninst::x86::mm4;
      case 34: return Dyninst::x86::mm5;
      case 35: return Dyninst::x86::mm6;
      case 36: return Dyninst::x86::mm7;
      case 37: return Dyninst::InvalidReg;
      case 38: return Dyninst::InvalidReg;
      case 39: return Dyninst::x86::mxcsr;
      case 40: return Dyninst::x86::es;
      case 41: return Dyninst::x86::cs;
      case 42: return Dyninst::x86::ss;
      case 43: return Dyninst::x86::ds;
      case 44: return Dyninst::x86::fs;
      case 45: return Dyninst::x86::gs;
      case 46: return Dyninst::InvalidReg;
      case 47: return Dyninst::InvalidReg;
      case 48: return Dyninst::x86::tr;
      case 49: return Dyninst::x86::ldtr;
      case 50: return Dyninst::InvalidReg;
      case 51: return Dyninst::InvalidReg;
      case 52: return Dyninst::InvalidReg;
      case 53: return Dyninst::InvalidReg;
      case 54: return Dyninst::InvalidReg;
      case 55: return Dyninst::InvalidReg;
      case 56: return Dyninst::InvalidReg;
      case 57: return Dyninst::InvalidReg;
      case 58: return Dyninst::InvalidReg;
      case 59: return Dyninst::InvalidReg;
      case 60: return Dyninst::InvalidReg;
      case 61: return Dyninst::InvalidReg;
      case 62: return Dyninst::InvalidReg;
      case 63: return Dyninst::InvalidReg;
      case 64: return Dyninst::InvalidReg;
      case 65: return Dyninst::InvalidReg;
      case 66: return Dyninst::InvalidReg;
      case 67: return Dyninst::InvalidReg;
      case 68: return Dyninst::InvalidReg;
      case 69: return Dyninst::InvalidReg;
      case 70: return Dyninst::InvalidReg;
      case 71: return Dyninst::InvalidReg;
      case 72: return Dyninst::InvalidReg;
      case 73: return Dyninst::InvalidReg;
      case 74: return Dyninst::InvalidReg;
      case 75: return Dyninst::InvalidReg;
      case 76: return Dyninst::InvalidReg;
      case 77: return Dyninst::InvalidReg;
      case 78: return Dyninst::InvalidReg;
      case 79: return Dyninst::InvalidReg;
      case 80: return Dyninst::InvalidReg;
      case 81: return Dyninst::InvalidReg;
      case 82: return Dyninst::InvalidReg;
      case 83: return Dyninst::InvalidReg;
      case 84: return Dyninst::InvalidReg;
      case 85: return Dyninst::InvalidReg;
      case 86: return Dyninst::InvalidReg;
      case 87: return Dyninst::InvalidReg;
      case 88: return Dyninst::InvalidReg;
      case 89: return Dyninst::InvalidReg;
      case 90: return Dyninst::InvalidReg;
      case 91: return Dyninst::InvalidReg;
      case 92: return Dyninst::InvalidReg;

      /* End of documented registers */
      /* The rest of these are assigned arbitrary values for internal Dyninst use. */
      case 1024: return Dyninst::x86::ax;
      case 1025: return Dyninst::x86::ah;
      case 1026: return Dyninst::x86::al;
      case 1027: return Dyninst::x86::cx;
      case 1028: return Dyninst::x86::ch;
      case 1029: return Dyninst::x86::cl;
      case 1030: return Dyninst::x86::dx;
      case 1031: return Dyninst::x86::dh;
      case 1032: return Dyninst::x86::dl;
      case 1033: return Dyninst::x86::bx;
      case 1034: return Dyninst::x86::bh;
      case 1035: return Dyninst::x86::bl;
      case 1036: return Dyninst::x86::sp;
      case 1037: return Dyninst::x86::bp;
      case 1038: return Dyninst::x86::si;
      case 1039: return Dyninst::x86::di;
      case 1040: return Dyninst::x86::gdtr;
      case 1041: return Dyninst::x86::idtr;
      case 1042: return Dyninst::x86::cf;
      case 1043: return Dyninst::x86::flag1;
      case 1044: return Dyninst::x86::pf;
      case 1045: return Dyninst::x86::flag3;
      case 1046: return Dyninst::x86::af;
      case 1047: return Dyninst::x86::flag5;
      case 1048: return Dyninst::x86::zf;
      case 1049: return Dyninst::x86::sf;
      case 1050: return Dyninst::x86::tf;
      case 1051: return Dyninst::x86::if_;
      case 1052: return Dyninst::x86::df;
      case 1053: return Dyninst::x86::of;
      case 1054: return Dyninst::x86::flagc;
      case 1055: return Dyninst::x86::flagd;
      case 1056: return Dyninst::x86::nt_;
      case 1057: return Dyninst::x86::flagf;
      case 1058: return Dyninst::x86::rf;
      case 1059: return Dyninst::x86::vm;
      case 1060: return Dyninst::x86::ac;
      case 1061: return Dyninst::x86::vif;
      case 1062: return Dyninst::x86::vip;
      case 1063: return Dyninst::x86::id;
      case 1064: return Dyninst::x86::cr0;
      case 1065: return Dyninst::x86::cr1;
      case 1066: return Dyninst::x86::cr2;
      case 1067: return Dyninst::x86::cr3;
      case 1068: return Dyninst::x86::cr4;
      case 1069: return Dyninst::x86::cr5;
      case 1070: return Dyninst::x86::cr6;
      case 1071: return Dyninst::x86::cr7;
      case 1072: return Dyninst::x86::dr0;
      case 1073: return Dyninst::x86::dr1;
      case 1074: return Dyninst::x86::dr2;
      case 1075: return Dyninst::x86::dr3;
      case 1076: return Dyninst::x86::dr4;
      case 1077: return Dyninst::x86::dr5;
      case 1078: return Dyninst::x86::dr6;
      case 1079: return Dyninst::x86::dr7;
      case 1080: return Dyninst::x86::fcw;
      case 1081: return Dyninst::x86::fsw;
      case 1082: return Dyninst::x86::ymm0;
      case 1083: return Dyninst::x86::ymm1;
      case 1084: return Dyninst::x86::ymm2;
      case 1085: return Dyninst::x86::ymm3;
      case 1086: return Dyninst::x86::ymm4;
      case 1087: return Dyninst::x86::ymm5;
      case 1088: return Dyninst::x86::ymm6;
      case 1089: return Dyninst::x86::ymm7;
      case 1090: return Dyninst::x86::zmm0;
      case 1091: return Dyninst::x86::zmm1;
      case 1092: return Dyninst::x86::zmm2;
      case 1093: return Dyninst::x86::zmm3;
      case 1094: return Dyninst::x86::zmm4;
      case 1095: return Dyninst::x86::zmm5;
      case 1096: return Dyninst::x86::zmm6;
      case 1097: return Dyninst::x86::zmm7;
      case 1098: return Dyninst::x86::k0;
      case 1099: return Dyninst::x86::k1;
      case 1100: return Dyninst::x86::k2;
      case 1101: return Dyninst::x86::k3;
      case 1102: return Dyninst::x86::k4;
      case 1103: return Dyninst::x86::k5;
      case 1104: return Dyninst::x86::k6;
      case 1105: return Dyninst::x86::k7;
      case 1106: return Dyninst::x86::oeax;
      case 1107: return Dyninst::x86::fsbase;
      case 1108: return Dyninst::x86::gsbase;
      case 1109: return Dyninst::x86::tr0;
      case 1110: return Dyninst::x86::tr1;
      case 1111: return Dyninst::x86::tr2;
      case 1112: return Dyninst::x86::tr3;
      case 1113: return Dyninst::x86::tr4;
      case 1114: return Dyninst::x86::tr5;
      case 1115: return Dyninst::x86::tr6;
      case 1116: return Dyninst::x86::tr7;
      default: return Dyninst::InvalidReg;
    }
  }

  int x86_to_dwarf(Dyninst::MachRegister reg) {
    switch(reg.val()) {
      case Dyninst::x86::ieax: return 0;
      case Dyninst::x86::iecx: return 1;
      case Dyninst::x86::iedx: return 2;
      case Dyninst::x86::iebx: return 3;
      case Dyninst::x86::iesp: return 4;
      case Dyninst::x86::iebp: return 5;
      case Dyninst::x86::iesi: return 6;
      case Dyninst::x86::iedi: return 7;
      case Dyninst::x86::ieip: return 8;
      case Dyninst::x86::iflags: return 9;
      /*[10] Reserved */
      case Dyninst::x86::ist0: return 11;
      case Dyninst::x86::ist1: return 12;
      case Dyninst::x86::ist2: return 13;
      case Dyninst::x86::ist3: return 14;
      case Dyninst::x86::ist4: return 15;
      case Dyninst::x86::ist5: return 16;
      case Dyninst::x86::ist6: return 17;
      case Dyninst::x86::ist7: return 18;
      /*[19] Reserved */
      /*[20] Reserved */
      case Dyninst::x86::ixmm0: return 21;
      case Dyninst::x86::ixmm1: return 22;
      case Dyninst::x86::ixmm2: return 23;
      case Dyninst::x86::ixmm3: return 24;
      case Dyninst::x86::ixmm4: return 25;
      case Dyninst::x86::ixmm5: return 26;
      case Dyninst::x86::ixmm6: return 27;
      case Dyninst::x86::ixmm7: return 28;
      case Dyninst::x86::imm0: return 29;
      case Dyninst::x86::imm1: return 30;
      case Dyninst::x86::imm2: return 31;
      case Dyninst::x86::imm3: return 32;
      case Dyninst::x86::imm4: return 33;
      case Dyninst::x86::imm5: return 34;
      case Dyninst::x86::imm6: return 35;
      case Dyninst::x86::imm7: return 36;
      /*[37] Reserved */
      /*[38] Reserved */
      case Dyninst::x86::imxcsr: return 39;
      case Dyninst::x86::ies: return 40;
      case Dyninst::x86::ics: return 41;
      case Dyninst::x86::iss: return 42;
      case Dyninst::x86::ids: return 43;
      case Dyninst::x86::ifs: return 44;
      case Dyninst::x86::igs: return 45;
      /*[46] Reserved */
      /*[47] Reserved */
      case Dyninst::x86::itr: return 48;
      case Dyninst::x86::ildtr: return 49;
      /*[50] Reserved */
      /*[51] Reserved */
      /*[52] Reserved */
      /*[53] Reserved */
      /*[54] Reserved */
      /*[55] Reserved */
      /*[56] Reserved */
      /*[57] Reserved */
      /*[58] Reserved */
      /*[59] Reserved */
      /*[60] Reserved */
      /*[61] Reserved */
      /*[62] Reserved */
      /*[63] Reserved */
      /*[64] Reserved */
      /*[65] Reserved */
      /*[66] Reserved */
      /*[67] Reserved */
      /*[68] Reserved */
      /*[69] Reserved */
      /*[70] Reserved */
      /*[71] Reserved */
      /*[72] Reserved */
      /*[73] Reserved */
      /*[74] Reserved */
      /*[75] Reserved */
      /*[76] Reserved */
      /*[77] Reserved */
      /*[78] Reserved */
      /*[79] Reserved */
      /*[80] Reserved */
      /*[81] Reserved */
      /*[82] Reserved */
      /*[83] Reserved */
      /*[84] Reserved */
      /*[85] Reserved */
      /*[86] Reserved */
      /*[87] Reserved */
      /*[88] Reserved */
      /*[89] Reserved */
      /*[90] Reserved */
      /*[91] Reserved */
      /*[92] Reserved */

      /* End of documented registers */
      /* The rest of these are assigned arbitrary values for internal Dyninst use. */
      case Dyninst::x86::iax: return 1024;
      case Dyninst::x86::iah: return 1025;
      case Dyninst::x86::ial: return 1026;
      case Dyninst::x86::icx: return 1027;
      case Dyninst::x86::ich: return 1028;
      case Dyninst::x86::icl: return 1029;
      case Dyninst::x86::idx: return 1030;
      case Dyninst::x86::idh: return 1031;
      case Dyninst::x86::idl: return 1032;
      case Dyninst::x86::ibx: return 1033;
      case Dyninst::x86::ibh: return 1034;
      case Dyninst::x86::ibl: return 1035;
      case Dyninst::x86::isp: return 1036;
      case Dyninst::x86::ibp: return 1037;
      case Dyninst::x86::isi: return 1038;
      case Dyninst::x86::idi: return 1039;
      case Dyninst::x86::igdtr: return 1040;
      case Dyninst::x86::iidtr: return 1041;
      case Dyninst::x86::icf: return 1042;
      case Dyninst::x86::iflag1: return 1043;
      case Dyninst::x86::ipf: return 1044;
      case Dyninst::x86::iflag3: return 1045;
      case Dyninst::x86::iaf: return 1046;
      case Dyninst::x86::iflag5: return 1047;
      case Dyninst::x86::izf: return 1048;
      case Dyninst::x86::isf: return 1049;
      case Dyninst::x86::itf: return 1050;
      case Dyninst::x86::iif_: return 1051;
      case Dyninst::x86::idf: return 1052;
      case Dyninst::x86::iof: return 1053;
      case Dyninst::x86::iflagc: return 1054;
      case Dyninst::x86::iflagd: return 1055;
      case Dyninst::x86::int_: return 1056;
      case Dyninst::x86::iflagf: return 1057;
      case Dyninst::x86::irf: return 1058;
      case Dyninst::x86::ivm: return 1059;
      case Dyninst::x86::iac: return 1060;
      case Dyninst::x86::ivif: return 1061;
      case Dyninst::x86::ivip: return 1062;
      case Dyninst::x86::iid: return 1063;
      case Dyninst::x86::icr0: return 1064;
      case Dyninst::x86::icr1: return 1065;
      case Dyninst::x86::icr2: return 1066;
      case Dyninst::x86::icr3: return 1067;
      case Dyninst::x86::icr4: return 1068;
      case Dyninst::x86::icr5: return 1069;
      case Dyninst::x86::icr6: return 1070;
      case Dyninst::x86::icr7: return 1071;
      case Dyninst::x86::idr0: return 1072;
      case Dyninst::x86::idr1: return 1073;
      case Dyninst::x86::idr2: return 1074;
      case Dyninst::x86::idr3: return 1075;
      case Dyninst::x86::idr4: return 1076;
      case Dyninst::x86::idr5: return 1077;
      case Dyninst::x86::idr6: return 1078;
      case Dyninst::x86::idr7: return 1079;
      case Dyninst::x86::ifcw: return 1080;
      case Dyninst::x86::ifsw: return 1081;
      case Dyninst::x86::iymm0: return 1082;
      case Dyninst::x86::iymm1: return 1083;
      case Dyninst::x86::iymm2: return 1084;
      case Dyninst::x86::iymm3: return 1085;
      case Dyninst::x86::iymm4: return 1086;
      case Dyninst::x86::iymm5: return 1087;
      case Dyninst::x86::iymm6: return 1088;
      case Dyninst::x86::iymm7: return 1089;
      case Dyninst::x86::izmm0: return 1090;
      case Dyninst::x86::izmm1: return 1091;
      case Dyninst::x86::izmm2: return 1092;
      case Dyninst::x86::izmm3: return 1093;
      case Dyninst::x86::izmm4: return 1094;
      case Dyninst::x86::izmm5: return 1095;
      case Dyninst::x86::izmm6: return 1096;
      case Dyninst::x86::izmm7: return 1097;
      case Dyninst::x86::ik0: return 1098;
      case Dyninst::x86::ik1: return 1099;
      case Dyninst::x86::ik2: return 1100;
      case Dyninst::x86::ik3: return 1101;
      case Dyninst::x86::ik4: return 1102;
      case Dyninst::x86::ik5: return 1103;
      case Dyninst::x86::ik6: return 1104;
      case Dyninst::x86::ik7: return 1105;
      case Dyninst::x86::ioeax: return 1106;
      case Dyninst::x86::ifsbase: return 1107;
      case Dyninst::x86::igsbase: return 1108;
      case Dyninst::x86::itr0: return 1109;
      case Dyninst::x86::itr1: return 1110;
      case Dyninst::x86::itr2: return 1111;
      case Dyninst::x86::itr3: return 1112;
      case Dyninst::x86::itr4: return 1113;
      case Dyninst::x86::itr5: return 1114;
      case Dyninst::x86::itr6: return 1115;
      case Dyninst::x86::itr7: return 1116;
      default: return -1;
    }

  }
}}
