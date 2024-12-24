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
#include "registers/x86_64_regs.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  /*
   * DWARF Encodings
   *
   * x86_64:
   *   System V Application Binary Interface
   *   AMD64 Architecture Processor Supplement
   *   Version 1.0 June 21, 2022
   *   Table 3.36: DWARF Register Number Mapping
   *   https://gitlab.com/x86-psABIs/x86-64-ABI
   */

  Dyninst::MachRegister x8664_from_dwarf(int encoding) {
    switch(encoding) {
      case 0: return Dyninst::x86_64::rax;
      case 1: return Dyninst::x86_64::rdx;
      case 2: return Dyninst::x86_64::rcx;
      case 3: return Dyninst::x86_64::rbx;
      case 4: return Dyninst::x86_64::rsi;
      case 5: return Dyninst::x86_64::rdi;
      case 6: return Dyninst::x86_64::rbp;
      case 7: return Dyninst::x86_64::rsp;
      case 8: return Dyninst::x86_64::r8;
      case 9: return Dyninst::x86_64::r9;
      case 10: return Dyninst::x86_64::r10;
      case 11: return Dyninst::x86_64::r11;
      case 12: return Dyninst::x86_64::r12;
      case 13: return Dyninst::x86_64::r13;
      case 14: return Dyninst::x86_64::r14;
      case 15: return Dyninst::x86_64::r15;
      case 16: return Dyninst::x86_64::rip;
      case 17: return Dyninst::x86_64::xmm0;
      case 18: return Dyninst::x86_64::xmm1;
      case 19: return Dyninst::x86_64::xmm2;
      case 20: return Dyninst::x86_64::xmm3;
      case 21: return Dyninst::x86_64::xmm4;
      case 22: return Dyninst::x86_64::xmm5;
      case 23: return Dyninst::x86_64::xmm6;
      case 24: return Dyninst::x86_64::xmm7;
      case 25: return Dyninst::x86_64::xmm8;
      case 26: return Dyninst::x86_64::xmm9;
      case 27: return Dyninst::x86_64::xmm10;
      case 28: return Dyninst::x86_64::xmm11;
      case 29: return Dyninst::x86_64::xmm12;
      case 30: return Dyninst::x86_64::xmm13;
      case 31: return Dyninst::x86_64::xmm14;
      case 32: return Dyninst::x86_64::xmm15;
      case 33: return Dyninst::x86_64::st0;
      case 34: return Dyninst::x86_64::st1;
      case 35: return Dyninst::x86_64::st2;
      case 36: return Dyninst::x86_64::st3;
      case 37: return Dyninst::x86_64::st4;
      case 38: return Dyninst::x86_64::st5;
      case 39: return Dyninst::x86_64::st6;
      case 40: return Dyninst::x86_64::st7;
      case 41: return Dyninst::x86_64::mm0;
      case 42: return Dyninst::x86_64::mm1;
      case 43: return Dyninst::x86_64::mm2;
      case 44: return Dyninst::x86_64::mm3;
      case 45: return Dyninst::x86_64::mm4;
      case 46: return Dyninst::x86_64::mm5;
      case 47: return Dyninst::x86_64::mm6;
      case 48: return Dyninst::x86_64::mm7;
      case 49: return Dyninst::x86_64::flags;
      case 50: return Dyninst::x86_64::es;
      case 51: return Dyninst::x86_64::cs;
      case 52: return Dyninst::x86_64::ss;
      case 53: return Dyninst::x86_64::ds;
      case 54: return Dyninst::x86_64::fs;
      case 55: return Dyninst::x86_64::gs;
      case 56: return Dyninst::InvalidReg;
      case 57: return Dyninst::InvalidReg;
      case 58: return Dyninst::x86_64::fsbase;
      case 59: return Dyninst::x86_64::gsbase;
      case 60: return Dyninst::InvalidReg;
      case 61: return Dyninst::InvalidReg;
      case 62: return Dyninst::x86_64::tr;
      case 63: return Dyninst::x86_64::ldtr;
      case 64: return Dyninst::x86_64::mxcsr;
      case 65: return Dyninst::x86_64::fcw;
      case 66: return Dyninst::x86_64::fsw;
      case 67: return Dyninst::x86_64::xmm16;
      case 68: return Dyninst::x86_64::xmm17;
      case 69: return Dyninst::x86_64::xmm18;
      case 70: return Dyninst::x86_64::xmm19;
      case 71: return Dyninst::x86_64::xmm20;
      case 72: return Dyninst::x86_64::xmm21;
      case 73: return Dyninst::x86_64::xmm22;
      case 74: return Dyninst::x86_64::xmm23;
      case 75: return Dyninst::x86_64::xmm24;
      case 76: return Dyninst::x86_64::xmm25;
      case 77: return Dyninst::x86_64::xmm26;
      case 78: return Dyninst::x86_64::xmm27;
      case 79: return Dyninst::x86_64::xmm28;
      case 80: return Dyninst::x86_64::xmm29;
      case 81: return Dyninst::x86_64::xmm30;
      case 82: return Dyninst::x86_64::xmm31;
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
      case 93: return Dyninst::InvalidReg;
      case 94: return Dyninst::InvalidReg;
      case 95: return Dyninst::InvalidReg;
      case 96: return Dyninst::InvalidReg;
      case 97: return Dyninst::InvalidReg;
      case 98: return Dyninst::InvalidReg;
      case 99: return Dyninst::InvalidReg;
      case 100: return Dyninst::InvalidReg;
      case 101: return Dyninst::InvalidReg;
      case 102: return Dyninst::InvalidReg;
      case 103: return Dyninst::InvalidReg;
      case 104: return Dyninst::InvalidReg;
      case 105: return Dyninst::InvalidReg;
      case 106: return Dyninst::InvalidReg;
      case 107: return Dyninst::InvalidReg;
      case 108: return Dyninst::InvalidReg;
      case 109: return Dyninst::InvalidReg;
      case 110: return Dyninst::InvalidReg;
      case 111: return Dyninst::InvalidReg;
      case 112: return Dyninst::InvalidReg;
      case 113: return Dyninst::InvalidReg;
      case 114: return Dyninst::InvalidReg;
      case 115: return Dyninst::InvalidReg;
      case 116: return Dyninst::InvalidReg;
      case 117: return Dyninst::InvalidReg;
      case 118: return Dyninst::x86_64::k0;
      case 119: return Dyninst::x86_64::k1;
      case 120: return Dyninst::x86_64::k2;
      case 121: return Dyninst::x86_64::k3;
      case 122: return Dyninst::x86_64::k4;
      case 123: return Dyninst::x86_64::k5;
      case 124: return Dyninst::x86_64::k6;
      case 125: return Dyninst::x86_64::k7;
      case 126: return Dyninst::InvalidReg;
      case 127: return Dyninst::InvalidReg;
      case 128: return Dyninst::InvalidReg;
      case 129: return Dyninst::InvalidReg;

      /* End of documented registers */
      /* The rest of these are assigned arbitrary values for internal Dyninst use. */
      case 1024: return Dyninst::x86_64::eax;
      case 1025: return Dyninst::x86_64::ax;
      case 1026: return Dyninst::x86_64::ah;
      case 1027: return Dyninst::x86_64::al;
      case 1028: return Dyninst::x86_64::ecx;
      case 1029: return Dyninst::x86_64::cx;
      case 1030: return Dyninst::x86_64::ch;
      case 1031: return Dyninst::x86_64::cl;
      case 1032: return Dyninst::x86_64::edx;
      case 1033: return Dyninst::x86_64::dx;
      case 1034: return Dyninst::x86_64::dh;
      case 1035: return Dyninst::x86_64::dl;
      case 1036: return Dyninst::x86_64::ebx;
      case 1037: return Dyninst::x86_64::bx;
      case 1038: return Dyninst::x86_64::bh;
      case 1039: return Dyninst::x86_64::bl;
      case 1040: return Dyninst::x86_64::esp;
      case 1041: return Dyninst::x86_64::sp;
      case 1042: return Dyninst::x86_64::spl;
      case 1043: return Dyninst::x86_64::ebp;
      case 1044: return Dyninst::x86_64::bp;
      case 1045: return Dyninst::x86_64::bpl;
      case 1046: return Dyninst::x86_64::esi;
      case 1047: return Dyninst::x86_64::si;
      case 1048: return Dyninst::x86_64::sil;
      case 1049: return Dyninst::x86_64::edi;
      case 1050: return Dyninst::x86_64::di;
      case 1051: return Dyninst::x86_64::dil;
      case 1052: return Dyninst::x86_64::r8b;
      case 1053: return Dyninst::x86_64::r8w;
      case 1054: return Dyninst::x86_64::r8d;
      case 1055: return Dyninst::x86_64::r9b;
      case 1056: return Dyninst::x86_64::r9w;
      case 1057: return Dyninst::x86_64::r9d;
      case 1058: return Dyninst::x86_64::r10b;
      case 1059: return Dyninst::x86_64::r10w;
      case 1060: return Dyninst::x86_64::r10d;
      case 1061: return Dyninst::x86_64::r11b;
      case 1062: return Dyninst::x86_64::r11w;
      case 1063: return Dyninst::x86_64::r11d;
      case 1064: return Dyninst::x86_64::r12b;
      case 1065: return Dyninst::x86_64::r12w;
      case 1066: return Dyninst::x86_64::r12d;
      case 1067: return Dyninst::x86_64::r13b;
      case 1068: return Dyninst::x86_64::r13w;
      case 1069: return Dyninst::x86_64::r13d;
      case 1070: return Dyninst::x86_64::r14b;
      case 1071: return Dyninst::x86_64::r14w;
      case 1072: return Dyninst::x86_64::r14d;
      case 1073: return Dyninst::x86_64::r15b;
      case 1074: return Dyninst::x86_64::r15w;
      case 1075: return Dyninst::x86_64::r15d;
      case 1076: return Dyninst::x86_64::eip;
      case 1077: return Dyninst::x86_64::cf;
      case 1078: return Dyninst::x86_64::flag1;
      case 1079: return Dyninst::x86_64::pf;
      case 1080: return Dyninst::x86_64::flag3;
      case 1081: return Dyninst::x86_64::af;
      case 1082: return Dyninst::x86_64::flag5;
      case 1083: return Dyninst::x86_64::zf;
      case 1084: return Dyninst::x86_64::sf;
      case 1085: return Dyninst::x86_64::tf;
      case 1086: return Dyninst::x86_64::if_;
      case 1087: return Dyninst::x86_64::df;
      case 1088: return Dyninst::x86_64::of;
      case 1089: return Dyninst::x86_64::flagc;
      case 1090: return Dyninst::x86_64::flagd;
      case 1091: return Dyninst::x86_64::nt_;
      case 1092: return Dyninst::x86_64::flagf;
      case 1093: return Dyninst::x86_64::rf;
      case 1094: return Dyninst::x86_64::vm;
      case 1095: return Dyninst::x86_64::ac;
      case 1096: return Dyninst::x86_64::vif;
      case 1097: return Dyninst::x86_64::vip;
      case 1098: return Dyninst::x86_64::id;
      case 1099: return Dyninst::x86_64::gdtr;
      case 1100: return Dyninst::x86_64::idtr;
      case 1101: return Dyninst::x86_64::cr0;
      case 1102: return Dyninst::x86_64::cr1;
      case 1103: return Dyninst::x86_64::cr2;
      case 1104: return Dyninst::x86_64::cr3;
      case 1105: return Dyninst::x86_64::cr4;
      case 1106: return Dyninst::x86_64::cr5;
      case 1107: return Dyninst::x86_64::cr6;
      case 1108: return Dyninst::x86_64::cr7;
      case 1109: return Dyninst::x86_64::cr8;
      case 1110: return Dyninst::x86_64::cr9;
      case 1111: return Dyninst::x86_64::cr10;
      case 1112: return Dyninst::x86_64::cr11;
      case 1113: return Dyninst::x86_64::cr12;
      case 1114: return Dyninst::x86_64::cr13;
      case 1115: return Dyninst::x86_64::cr14;
      case 1116: return Dyninst::x86_64::cr15;
      case 1117: return Dyninst::x86_64::dr0;
      case 1118: return Dyninst::x86_64::dr1;
      case 1119: return Dyninst::x86_64::dr2;
      case 1120: return Dyninst::x86_64::dr3;
      case 1121: return Dyninst::x86_64::dr4;
      case 1122: return Dyninst::x86_64::dr5;
      case 1123: return Dyninst::x86_64::dr6;
      case 1124: return Dyninst::x86_64::dr7;
      case 1125: return Dyninst::x86_64::dr8;
      case 1126: return Dyninst::x86_64::dr9;
      case 1127: return Dyninst::x86_64::dr10;
      case 1128: return Dyninst::x86_64::dr11;
      case 1129: return Dyninst::x86_64::dr12;
      case 1130: return Dyninst::x86_64::dr13;
      case 1131: return Dyninst::x86_64::dr14;
      case 1132: return Dyninst::x86_64::dr15;
      case 1133: return Dyninst::x86_64::ymm0;
      case 1134: return Dyninst::x86_64::ymm1;
      case 1135: return Dyninst::x86_64::ymm2;
      case 1136: return Dyninst::x86_64::ymm3;
      case 1137: return Dyninst::x86_64::ymm4;
      case 1138: return Dyninst::x86_64::ymm5;
      case 1139: return Dyninst::x86_64::ymm6;
      case 1140: return Dyninst::x86_64::ymm7;
      case 1141: return Dyninst::x86_64::ymm8;
      case 1142: return Dyninst::x86_64::ymm9;
      case 1143: return Dyninst::x86_64::ymm10;
      case 1144: return Dyninst::x86_64::ymm11;
      case 1145: return Dyninst::x86_64::ymm12;
      case 1146: return Dyninst::x86_64::ymm13;
      case 1147: return Dyninst::x86_64::ymm14;
      case 1148: return Dyninst::x86_64::ymm15;
      case 1149: return Dyninst::x86_64::ymm16;
      case 1150: return Dyninst::x86_64::ymm17;
      case 1151: return Dyninst::x86_64::ymm18;
      case 1152: return Dyninst::x86_64::ymm19;
      case 1153: return Dyninst::x86_64::ymm20;
      case 1154: return Dyninst::x86_64::ymm21;
      case 1155: return Dyninst::x86_64::ymm22;
      case 1156: return Dyninst::x86_64::ymm23;
      case 1157: return Dyninst::x86_64::ymm24;
      case 1158: return Dyninst::x86_64::ymm25;
      case 1159: return Dyninst::x86_64::ymm26;
      case 1160: return Dyninst::x86_64::ymm27;
      case 1161: return Dyninst::x86_64::ymm28;
      case 1162: return Dyninst::x86_64::ymm29;
      case 1163: return Dyninst::x86_64::ymm30;
      case 1164: return Dyninst::x86_64::ymm31;
      case 1165: return Dyninst::x86_64::zmm0;
      case 1166: return Dyninst::x86_64::zmm1;
      case 1167: return Dyninst::x86_64::zmm2;
      case 1168: return Dyninst::x86_64::zmm3;
      case 1169: return Dyninst::x86_64::zmm4;
      case 1170: return Dyninst::x86_64::zmm5;
      case 1171: return Dyninst::x86_64::zmm6;
      case 1172: return Dyninst::x86_64::zmm7;
      case 1173: return Dyninst::x86_64::zmm8;
      case 1174: return Dyninst::x86_64::zmm9;
      case 1175: return Dyninst::x86_64::zmm10;
      case 1176: return Dyninst::x86_64::zmm11;
      case 1177: return Dyninst::x86_64::zmm12;
      case 1178: return Dyninst::x86_64::zmm13;
      case 1179: return Dyninst::x86_64::zmm14;
      case 1180: return Dyninst::x86_64::zmm15;
      case 1181: return Dyninst::x86_64::zmm16;
      case 1182: return Dyninst::x86_64::zmm17;
      case 1183: return Dyninst::x86_64::zmm18;
      case 1184: return Dyninst::x86_64::zmm19;
      case 1185: return Dyninst::x86_64::zmm20;
      case 1186: return Dyninst::x86_64::zmm21;
      case 1187: return Dyninst::x86_64::zmm22;
      case 1188: return Dyninst::x86_64::zmm23;
      case 1189: return Dyninst::x86_64::zmm24;
      case 1190: return Dyninst::x86_64::zmm25;
      case 1191: return Dyninst::x86_64::zmm26;
      case 1192: return Dyninst::x86_64::zmm27;
      case 1193: return Dyninst::x86_64::zmm28;
      case 1194: return Dyninst::x86_64::zmm29;
      case 1195: return Dyninst::x86_64::zmm30;
      case 1196: return Dyninst::x86_64::zmm31;
      case 1197: return Dyninst::x86_64::orax;
      case 1198: return Dyninst::x86_64::tr0;
      case 1199: return Dyninst::x86_64::tr1;
      case 1200: return Dyninst::x86_64::tr2;
      case 1201: return Dyninst::x86_64::tr3;
      case 1202: return Dyninst::x86_64::tr4;
      case 1203: return Dyninst::x86_64::tr5;
      case 1204: return Dyninst::x86_64::tr6;
      case 1205: return Dyninst::x86_64::tr7;
      default: return Dyninst::InvalidReg;
    }
  }

  int x8664_to_dwarf(Dyninst::MachRegister reg) {
    switch(reg.val()) {
      case Dyninst::x86_64::irax: return 0;
      case Dyninst::x86_64::irdx: return 1;
      case Dyninst::x86_64::ircx: return 2;
      case Dyninst::x86_64::irbx: return 3;
      case Dyninst::x86_64::irsi: return 4;
      case Dyninst::x86_64::irdi: return 5;
      case Dyninst::x86_64::irbp: return 6;
      case Dyninst::x86_64::irsp: return 7;
      case Dyninst::x86_64::ir8: return 8;
      case Dyninst::x86_64::ir9: return 9;
      case Dyninst::x86_64::ir10: return 10;
      case Dyninst::x86_64::ir11: return 11;
      case Dyninst::x86_64::ir12: return 12;
      case Dyninst::x86_64::ir13: return 13;
      case Dyninst::x86_64::ir14: return 14;
      case Dyninst::x86_64::ir15: return 15;
      case Dyninst::x86_64::irip: return 16;
      case Dyninst::x86_64::ixmm0: return 17;
      case Dyninst::x86_64::ixmm1: return 18;
      case Dyninst::x86_64::ixmm2: return 19;
      case Dyninst::x86_64::ixmm3: return 20;
      case Dyninst::x86_64::ixmm4: return 21;
      case Dyninst::x86_64::ixmm5: return 22;
      case Dyninst::x86_64::ixmm6: return 23;
      case Dyninst::x86_64::ixmm7: return 24;
      case Dyninst::x86_64::ixmm8: return 25;
      case Dyninst::x86_64::ixmm9: return 26;
      case Dyninst::x86_64::ixmm10: return 27;
      case Dyninst::x86_64::ixmm11: return 28;
      case Dyninst::x86_64::ixmm12: return 29;
      case Dyninst::x86_64::ixmm13: return 30;
      case Dyninst::x86_64::ixmm14: return 31;
      case Dyninst::x86_64::ixmm15: return 32;
      case Dyninst::x86_64::ist0: return 33;
      case Dyninst::x86_64::ist1: return 34;
      case Dyninst::x86_64::ist2: return 35;
      case Dyninst::x86_64::ist3: return 36;
      case Dyninst::x86_64::ist4: return 37;
      case Dyninst::x86_64::ist5: return 38;
      case Dyninst::x86_64::ist6: return 39;
      case Dyninst::x86_64::ist7: return 40;
      case Dyninst::x86_64::imm0: return 41;
      case Dyninst::x86_64::imm1: return 42;
      case Dyninst::x86_64::imm2: return 43;
      case Dyninst::x86_64::imm3: return 44;
      case Dyninst::x86_64::imm4: return 45;
      case Dyninst::x86_64::imm5: return 46;
      case Dyninst::x86_64::imm6: return 47;
      case Dyninst::x86_64::imm7: return 48;
      case Dyninst::x86_64::iflags: return 49;
      case Dyninst::x86_64::ies: return 50;
      case Dyninst::x86_64::ics: return 51;
      case Dyninst::x86_64::iss: return 52;
      case Dyninst::x86_64::ids: return 53;
      case Dyninst::x86_64::ifs: return 54;
      case Dyninst::x86_64::igs: return 55;
      /*[56] Reserved */
      /*[57] Reserved */
      case Dyninst::x86_64::ifsbase: return 58;
      case Dyninst::x86_64::igsbase: return 59;
      /*[60] Reserved */
      /*[61] Reserved */
      case Dyninst::x86_64::itr: return 62;
      case Dyninst::x86_64::ildtr: return 63;
      case Dyninst::x86_64::imxcsr: return 64;
      case Dyninst::x86_64::ifcw: return 65;
      case Dyninst::x86_64::ifsw: return 66;
      case Dyninst::x86_64::ixmm16: return 67;
      case Dyninst::x86_64::ixmm17: return 68;
      case Dyninst::x86_64::ixmm18: return 69;
      case Dyninst::x86_64::ixmm19: return 70;
      case Dyninst::x86_64::ixmm20: return 71;
      case Dyninst::x86_64::ixmm21: return 72;
      case Dyninst::x86_64::ixmm22: return 73;
      case Dyninst::x86_64::ixmm23: return 74;
      case Dyninst::x86_64::ixmm24: return 75;
      case Dyninst::x86_64::ixmm25: return 76;
      case Dyninst::x86_64::ixmm26: return 77;
      case Dyninst::x86_64::ixmm27: return 78;
      case Dyninst::x86_64::ixmm28: return 79;
      case Dyninst::x86_64::ixmm29: return 80;
      case Dyninst::x86_64::ixmm30: return 81;
      case Dyninst::x86_64::ixmm31: return 82;
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
      /*[93] Reserved */
      /*[94] Reserved */
      /*[95] Reserved */
      /*[96] Reserved */
      /*[97] Reserved */
      /*[98] Reserved */
      /*[99] Reserved */
      /*[100] Reserved */
      /*[101] Reserved */
      /*[102] Reserved */
      /*[103] Reserved */
      /*[104] Reserved */
      /*[105] Reserved */
      /*[106] Reserved */
      /*[107] Reserved */
      /*[108] Reserved */
      /*[109] Reserved */
      /*[110] Reserved */
      /*[111] Reserved */
      /*[112] Reserved */
      /*[113] Reserved */
      /*[114] Reserved */
      /*[115] Reserved */
      /*[116] Reserved */
      /*[117] Reserved */
      case Dyninst::x86_64::ik0: return 118;
      case Dyninst::x86_64::ik1: return 119;
      case Dyninst::x86_64::ik2: return 120;
      case Dyninst::x86_64::ik3: return 121;
      case Dyninst::x86_64::ik4: return 122;
      case Dyninst::x86_64::ik5: return 123;
      case Dyninst::x86_64::ik6: return 124;
      case Dyninst::x86_64::ik7: return 125;
      /*[126] Reserved */
      /*[127] Reserved */
      /*[128] Reserved */
      /*[129] Reserved */

      /* End of documented registers */
      /* The rest of these are assigned arbitrary values for internal Dyninst use. */
      case Dyninst::x86_64::ieax: return 1024;
      case Dyninst::x86_64::iax: return 1025;
      case Dyninst::x86_64::iah: return 1026;
      case Dyninst::x86_64::ial: return 1027;
      case Dyninst::x86_64::iecx: return 1028;
      case Dyninst::x86_64::icx: return 1029;
      case Dyninst::x86_64::ich: return 1030;
      case Dyninst::x86_64::icl: return 1031;
      case Dyninst::x86_64::iedx: return 1032;
      case Dyninst::x86_64::idx: return 1033;
      case Dyninst::x86_64::idh: return 1034;
      case Dyninst::x86_64::idl: return 1035;
      case Dyninst::x86_64::iebx: return 1036;
      case Dyninst::x86_64::ibx: return 1037;
      case Dyninst::x86_64::ibh: return 1038;
      case Dyninst::x86_64::ibl: return 1039;
      case Dyninst::x86_64::iesp: return 1040;
      case Dyninst::x86_64::isp: return 1041;
      case Dyninst::x86_64::ispl: return 1042;
      case Dyninst::x86_64::iebp: return 1043;
      case Dyninst::x86_64::ibp: return 1044;
      case Dyninst::x86_64::ibpl: return 1045;
      case Dyninst::x86_64::iesi: return 1046;
      case Dyninst::x86_64::isi: return 1047;
      case Dyninst::x86_64::isil: return 1048;
      case Dyninst::x86_64::iedi: return 1049;
      case Dyninst::x86_64::idi: return 1050;
      case Dyninst::x86_64::idil: return 1051;
      case Dyninst::x86_64::ir8b: return 1052;
      case Dyninst::x86_64::ir8w: return 1053;
      case Dyninst::x86_64::ir8d: return 1054;
      case Dyninst::x86_64::ir9b: return 1055;
      case Dyninst::x86_64::ir9w: return 1056;
      case Dyninst::x86_64::ir9d: return 1057;
      case Dyninst::x86_64::ir10b: return 1058;
      case Dyninst::x86_64::ir10w: return 1059;
      case Dyninst::x86_64::ir10d: return 1060;
      case Dyninst::x86_64::ir11b: return 1061;
      case Dyninst::x86_64::ir11w: return 1062;
      case Dyninst::x86_64::ir11d: return 1063;
      case Dyninst::x86_64::ir12b: return 1064;
      case Dyninst::x86_64::ir12w: return 1065;
      case Dyninst::x86_64::ir12d: return 1066;
      case Dyninst::x86_64::ir13b: return 1067;
      case Dyninst::x86_64::ir13w: return 1068;
      case Dyninst::x86_64::ir13d: return 1069;
      case Dyninst::x86_64::ir14b: return 1070;
      case Dyninst::x86_64::ir14w: return 1071;
      case Dyninst::x86_64::ir14d: return 1072;
      case Dyninst::x86_64::ir15b: return 1073;
      case Dyninst::x86_64::ir15w: return 1074;
      case Dyninst::x86_64::ir15d: return 1075;
      case Dyninst::x86_64::ieip: return 1076;
      case Dyninst::x86_64::icf: return 1077;
      case Dyninst::x86_64::iflag1: return 1078;
      case Dyninst::x86_64::ipf: return 1079;
      case Dyninst::x86_64::iflag3: return 1080;
      case Dyninst::x86_64::iaf: return 1081;
      case Dyninst::x86_64::iflag5: return 1082;
      case Dyninst::x86_64::izf: return 1083;
      case Dyninst::x86_64::isf: return 1084;
      case Dyninst::x86_64::itf: return 1085;
      case Dyninst::x86_64::iif_: return 1086;
      case Dyninst::x86_64::idf: return 1087;
      case Dyninst::x86_64::iof: return 1088;
      case Dyninst::x86_64::iflagc: return 1089;
      case Dyninst::x86_64::iflagd: return 1090;
      case Dyninst::x86_64::int_: return 1091;
      case Dyninst::x86_64::iflagf: return 1092;
      case Dyninst::x86_64::irf: return 1093;
      case Dyninst::x86_64::ivm: return 1094;
      case Dyninst::x86_64::iac: return 1095;
      case Dyninst::x86_64::ivif: return 1096;
      case Dyninst::x86_64::ivip: return 1097;
      case Dyninst::x86_64::iid: return 1098;
      case Dyninst::x86_64::igdtr: return 1099;
      case Dyninst::x86_64::iidtr: return 1100;
      case Dyninst::x86_64::icr0: return 1101;
      case Dyninst::x86_64::icr1: return 1102;
      case Dyninst::x86_64::icr2: return 1103;
      case Dyninst::x86_64::icr3: return 1104;
      case Dyninst::x86_64::icr4: return 1105;
      case Dyninst::x86_64::icr5: return 1106;
      case Dyninst::x86_64::icr6: return 1107;
      case Dyninst::x86_64::icr7: return 1108;
      case Dyninst::x86_64::icr8: return 1109;
      case Dyninst::x86_64::icr9: return 1110;
      case Dyninst::x86_64::icr10: return 1111;
      case Dyninst::x86_64::icr11: return 1112;
      case Dyninst::x86_64::icr12: return 1113;
      case Dyninst::x86_64::icr13: return 1114;
      case Dyninst::x86_64::icr14: return 1115;
      case Dyninst::x86_64::icr15: return 1116;
      case Dyninst::x86_64::idr0: return 1117;
      case Dyninst::x86_64::idr1: return 1118;
      case Dyninst::x86_64::idr2: return 1119;
      case Dyninst::x86_64::idr3: return 1120;
      case Dyninst::x86_64::idr4: return 1121;
      case Dyninst::x86_64::idr5: return 1122;
      case Dyninst::x86_64::idr6: return 1123;
      case Dyninst::x86_64::idr7: return 1124;
      case Dyninst::x86_64::idr8: return 1125;
      case Dyninst::x86_64::idr9: return 1126;
      case Dyninst::x86_64::idr10: return 1127;
      case Dyninst::x86_64::idr11: return 1128;
      case Dyninst::x86_64::idr12: return 1129;
      case Dyninst::x86_64::idr13: return 1130;
      case Dyninst::x86_64::idr14: return 1131;
      case Dyninst::x86_64::idr15: return 1132;
      case Dyninst::x86_64::iymm0: return 1133;
      case Dyninst::x86_64::iymm1: return 1134;
      case Dyninst::x86_64::iymm2: return 1135;
      case Dyninst::x86_64::iymm3: return 1136;
      case Dyninst::x86_64::iymm4: return 1137;
      case Dyninst::x86_64::iymm5: return 1138;
      case Dyninst::x86_64::iymm6: return 1139;
      case Dyninst::x86_64::iymm7: return 1140;
      case Dyninst::x86_64::iymm8: return 1141;
      case Dyninst::x86_64::iymm9: return 1142;
      case Dyninst::x86_64::iymm10: return 1143;
      case Dyninst::x86_64::iymm11: return 1144;
      case Dyninst::x86_64::iymm12: return 1145;
      case Dyninst::x86_64::iymm13: return 1146;
      case Dyninst::x86_64::iymm14: return 1147;
      case Dyninst::x86_64::iymm15: return 1148;
      case Dyninst::x86_64::iymm16: return 1149;
      case Dyninst::x86_64::iymm17: return 1150;
      case Dyninst::x86_64::iymm18: return 1151;
      case Dyninst::x86_64::iymm19: return 1152;
      case Dyninst::x86_64::iymm20: return 1153;
      case Dyninst::x86_64::iymm21: return 1154;
      case Dyninst::x86_64::iymm22: return 1155;
      case Dyninst::x86_64::iymm23: return 1156;
      case Dyninst::x86_64::iymm24: return 1157;
      case Dyninst::x86_64::iymm25: return 1158;
      case Dyninst::x86_64::iymm26: return 1159;
      case Dyninst::x86_64::iymm27: return 1160;
      case Dyninst::x86_64::iymm28: return 1161;
      case Dyninst::x86_64::iymm29: return 1162;
      case Dyninst::x86_64::iymm30: return 1163;
      case Dyninst::x86_64::iymm31: return 1164;
      case Dyninst::x86_64::izmm0: return 1165;
      case Dyninst::x86_64::izmm1: return 1166;
      case Dyninst::x86_64::izmm2: return 1167;
      case Dyninst::x86_64::izmm3: return 1168;
      case Dyninst::x86_64::izmm4: return 1169;
      case Dyninst::x86_64::izmm5: return 1170;
      case Dyninst::x86_64::izmm6: return 1171;
      case Dyninst::x86_64::izmm7: return 1172;
      case Dyninst::x86_64::izmm8: return 1173;
      case Dyninst::x86_64::izmm9: return 1174;
      case Dyninst::x86_64::izmm10: return 1175;
      case Dyninst::x86_64::izmm11: return 1176;
      case Dyninst::x86_64::izmm12: return 1177;
      case Dyninst::x86_64::izmm13: return 1178;
      case Dyninst::x86_64::izmm14: return 1179;
      case Dyninst::x86_64::izmm15: return 1180;
      case Dyninst::x86_64::izmm16: return 1181;
      case Dyninst::x86_64::izmm17: return 1182;
      case Dyninst::x86_64::izmm18: return 1183;
      case Dyninst::x86_64::izmm19: return 1184;
      case Dyninst::x86_64::izmm20: return 1185;
      case Dyninst::x86_64::izmm21: return 1186;
      case Dyninst::x86_64::izmm22: return 1187;
      case Dyninst::x86_64::izmm23: return 1188;
      case Dyninst::x86_64::izmm24: return 1189;
      case Dyninst::x86_64::izmm25: return 1190;
      case Dyninst::x86_64::izmm26: return 1191;
      case Dyninst::x86_64::izmm27: return 1192;
      case Dyninst::x86_64::izmm28: return 1193;
      case Dyninst::x86_64::izmm29: return 1194;
      case Dyninst::x86_64::izmm30: return 1195;
      case Dyninst::x86_64::izmm31: return 1196;
      case Dyninst::x86_64::iorax: return 1197;
      case Dyninst::x86_64::itr0: return 1198;
      case Dyninst::x86_64::itr1: return 1199;
      case Dyninst::x86_64::itr2: return 1200;
      case Dyninst::x86_64::itr3: return 1201;
      case Dyninst::x86_64::itr4: return 1202;
      case Dyninst::x86_64::itr5: return 1203;
      case Dyninst::x86_64::itr6: return 1204;
      case Dyninst::x86_64::itr7: return 1205;
      default: return -1;
    }
  }

}}
