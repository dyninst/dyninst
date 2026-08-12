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

#ifndef DYNINST_CUDA_REGS_H
#define DYNINST_CUDA_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace cuda {

  /**
   * For interpreting constants:
   *  Lowest 16 bits (0x000000ff) is base register ID
   *  Next 16 bits (0x0000ff00) is the aliasing and subrange ID-
   *    used on x86/x86_64 to distinguish between things like EAX and AH
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
   *  Top 16 bits (0xff000000) are the architecture.
   *
   *  These values/layout are not guaranteed to remain the same as part of the
   *  public interface, and may change.
   **/

  const signed int GPR = 0x00000000;
  const signed int PR  = 0x00010000;
  const signed int BR  = 0x00020000;
  const signed int UR  = 0x00040000;
  const signed int UPR = 0x00080000;

  // General purpose registers
  //          (  name,  ID | cat |      arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_CUDA_REG_LIST(X) \
      X(r0, 0 | GPR | Arch_cuda) \
      X(r1, 1 | GPR | Arch_cuda) \
      X(r2, 2 | GPR | Arch_cuda) \
      X(r3, 3 | GPR | Arch_cuda) \
      X(r4, 4 | GPR | Arch_cuda) \
      X(r5, 5 | GPR | Arch_cuda) \
      X(r6, 6 | GPR | Arch_cuda) \
      X(r7, 7 | GPR | Arch_cuda) \
      X(r8, 8 | GPR | Arch_cuda) \
      X(r9, 9 | GPR | Arch_cuda) \
      X(r10, 10 | GPR | Arch_cuda) \
      X(r11, 11 | GPR | Arch_cuda) \
      X(r12, 12 | GPR | Arch_cuda) \
      X(r13, 13 | GPR | Arch_cuda) \
      X(r14, 14 | GPR | Arch_cuda) \
      X(r15, 15 | GPR | Arch_cuda) \
      X(r16, 16 | GPR | Arch_cuda) \
      X(r17, 17 | GPR | Arch_cuda) \
      X(r18, 18 | GPR | Arch_cuda) \
      X(r19, 19 | GPR | Arch_cuda) \
      X(r20, 20 | GPR | Arch_cuda) \
      X(r21, 21 | GPR | Arch_cuda) \
      X(r22, 22 | GPR | Arch_cuda) \
      X(r23, 23 | GPR | Arch_cuda) \
      X(r24, 24 | GPR | Arch_cuda) \
      X(r25, 25 | GPR | Arch_cuda) \
      X(r26, 26 | GPR | Arch_cuda) \
      X(r27, 27 | GPR | Arch_cuda) \
      X(r28, 28 | GPR | Arch_cuda) \
      X(r29, 29 | GPR | Arch_cuda) \
      X(r30, 30 | GPR | Arch_cuda) \
      X(r31, 31 | GPR | Arch_cuda) \
      X(r32, 32 | GPR | Arch_cuda) \
      X(r33, 33 | GPR | Arch_cuda) \
      X(r34, 34 | GPR | Arch_cuda) \
      X(r35, 35 | GPR | Arch_cuda) \
      X(r36, 36 | GPR | Arch_cuda) \
      X(r37, 37 | GPR | Arch_cuda) \
      X(r38, 38 | GPR | Arch_cuda) \
      X(r39, 39 | GPR | Arch_cuda) \
      X(r40, 40 | GPR | Arch_cuda) \
      X(r41, 41 | GPR | Arch_cuda) \
      X(r42, 42 | GPR | Arch_cuda) \
      X(r43, 43 | GPR | Arch_cuda) \
      X(r44, 44 | GPR | Arch_cuda) \
      X(r45, 45 | GPR | Arch_cuda) \
      X(r46, 46 | GPR | Arch_cuda) \
      X(r47, 47 | GPR | Arch_cuda) \
      X(r48, 48 | GPR | Arch_cuda) \
      X(r49, 49 | GPR | Arch_cuda) \
      X(r50, 50 | GPR | Arch_cuda) \
      X(r51, 51 | GPR | Arch_cuda) \
      X(r52, 52 | GPR | Arch_cuda) \
      X(r53, 53 | GPR | Arch_cuda) \
      X(r54, 54 | GPR | Arch_cuda) \
      X(r55, 55 | GPR | Arch_cuda) \
      X(r56, 56 | GPR | Arch_cuda) \
      X(r57, 57 | GPR | Arch_cuda) \
      X(r58, 58 | GPR | Arch_cuda) \
      X(r59, 59 | GPR | Arch_cuda) \
      X(r60, 60 | GPR | Arch_cuda) \
      X(r61, 61 | GPR | Arch_cuda) \
      X(r62, 62 | GPR | Arch_cuda) \
      X(r63, 63 | GPR | Arch_cuda) \
      X(r64, 64 | GPR | Arch_cuda) \
      X(r65, 65 | GPR | Arch_cuda) \
      X(r66, 66 | GPR | Arch_cuda) \
      X(r67, 67 | GPR | Arch_cuda) \
      X(r68, 68 | GPR | Arch_cuda) \
      X(r69, 69 | GPR | Arch_cuda) \
      X(r70, 70 | GPR | Arch_cuda) \
      X(r71, 71 | GPR | Arch_cuda) \
      X(r72, 72 | GPR | Arch_cuda) \
      X(r73, 73 | GPR | Arch_cuda) \
      X(r74, 74 | GPR | Arch_cuda) \
      X(r75, 75 | GPR | Arch_cuda) \
      X(r76, 76 | GPR | Arch_cuda) \
      X(r77, 77 | GPR | Arch_cuda) \
      X(r78, 78 | GPR | Arch_cuda) \
      X(r79, 79 | GPR | Arch_cuda) \
      X(r80, 80 | GPR | Arch_cuda) \
      X(r81, 81 | GPR | Arch_cuda) \
      X(r82, 82 | GPR | Arch_cuda) \
      X(r83, 83 | GPR | Arch_cuda) \
      X(r84, 84 | GPR | Arch_cuda) \
      X(r85, 85 | GPR | Arch_cuda) \
      X(r86, 86 | GPR | Arch_cuda) \
      X(r87, 87 | GPR | Arch_cuda) \
      X(r88, 88 | GPR | Arch_cuda) \
      X(r89, 89 | GPR | Arch_cuda) \
      X(r90, 90 | GPR | Arch_cuda) \
      X(r91, 91 | GPR | Arch_cuda) \
      X(r92, 92 | GPR | Arch_cuda) \
      X(r93, 93 | GPR | Arch_cuda) \
      X(r94, 94 | GPR | Arch_cuda) \
      X(r95, 95 | GPR | Arch_cuda) \
      X(r96, 96 | GPR | Arch_cuda) \
      X(r97, 97 | GPR | Arch_cuda) \
      X(r98, 98 | GPR | Arch_cuda) \
      X(r99, 99 | GPR | Arch_cuda) \
      X(r100, 100 | GPR | Arch_cuda) \
      X(r101, 101 | GPR | Arch_cuda) \
      X(r102, 102 | GPR | Arch_cuda) \
      X(r103, 103 | GPR | Arch_cuda) \
      X(r104, 104 | GPR | Arch_cuda) \
      X(r105, 105 | GPR | Arch_cuda) \
      X(r106, 106 | GPR | Arch_cuda) \
      X(r107, 107 | GPR | Arch_cuda) \
      X(r108, 108 | GPR | Arch_cuda) \
      X(r109, 109 | GPR | Arch_cuda) \
      X(r110, 110 | GPR | Arch_cuda) \
      X(r111, 111 | GPR | Arch_cuda) \
      X(r112, 112 | GPR | Arch_cuda) \
      X(r113, 113 | GPR | Arch_cuda) \
      X(r114, 114 | GPR | Arch_cuda) \
      X(r115, 115 | GPR | Arch_cuda) \
      X(r116, 116 | GPR | Arch_cuda) \
      X(r117, 117 | GPR | Arch_cuda) \
      X(r118, 118 | GPR | Arch_cuda) \
      X(r119, 119 | GPR | Arch_cuda) \
      X(r120, 120 | GPR | Arch_cuda) \
      X(r121, 121 | GPR | Arch_cuda) \
      X(r122, 122 | GPR | Arch_cuda) \
      X(r123, 123 | GPR | Arch_cuda) \
      X(r124, 124 | GPR | Arch_cuda) \
      X(r125, 125 | GPR | Arch_cuda) \
      X(r126, 126 | GPR | Arch_cuda) \
      X(r127, 127 | GPR | Arch_cuda) \
      X(r128, 128 | GPR | Arch_cuda) \
      X(r129, 129 | GPR | Arch_cuda) \
      X(r130, 130 | GPR | Arch_cuda) \
      X(r131, 131 | GPR | Arch_cuda) \
      X(r132, 132 | GPR | Arch_cuda) \
      X(r133, 133 | GPR | Arch_cuda) \
      X(r134, 134 | GPR | Arch_cuda) \
      X(r135, 135 | GPR | Arch_cuda) \
      X(r136, 136 | GPR | Arch_cuda) \
      X(r137, 137 | GPR | Arch_cuda) \
      X(r138, 138 | GPR | Arch_cuda) \
      X(r139, 139 | GPR | Arch_cuda) \
      X(r140, 140 | GPR | Arch_cuda) \
      X(r141, 141 | GPR | Arch_cuda) \
      X(r142, 142 | GPR | Arch_cuda) \
      X(r143, 143 | GPR | Arch_cuda) \
      X(r144, 144 | GPR | Arch_cuda) \
      X(r145, 145 | GPR | Arch_cuda) \
      X(r146, 146 | GPR | Arch_cuda) \
      X(r147, 147 | GPR | Arch_cuda) \
      X(r148, 148 | GPR | Arch_cuda) \
      X(r149, 149 | GPR | Arch_cuda) \
      X(r150, 150 | GPR | Arch_cuda) \
      X(r151, 151 | GPR | Arch_cuda) \
      X(r152, 152 | GPR | Arch_cuda) \
      X(r153, 153 | GPR | Arch_cuda) \
      X(r154, 154 | GPR | Arch_cuda) \
      X(r155, 155 | GPR | Arch_cuda) \
      X(r156, 156 | GPR | Arch_cuda) \
      X(r157, 157 | GPR | Arch_cuda) \
      X(r158, 158 | GPR | Arch_cuda) \
      X(r159, 159 | GPR | Arch_cuda) \
      X(r160, 160 | GPR | Arch_cuda) \
      X(r161, 161 | GPR | Arch_cuda) \
      X(r162, 162 | GPR | Arch_cuda) \
      X(r163, 163 | GPR | Arch_cuda) \
      X(r164, 164 | GPR | Arch_cuda) \
      X(r165, 165 | GPR | Arch_cuda) \
      X(r166, 166 | GPR | Arch_cuda) \
      X(r167, 167 | GPR | Arch_cuda) \
      X(r168, 168 | GPR | Arch_cuda) \
      X(r169, 169 | GPR | Arch_cuda) \
      X(r170, 170 | GPR | Arch_cuda) \
      X(r171, 171 | GPR | Arch_cuda) \
      X(r172, 172 | GPR | Arch_cuda) \
      X(r173, 173 | GPR | Arch_cuda) \
      X(r174, 174 | GPR | Arch_cuda) \
      X(r175, 175 | GPR | Arch_cuda) \
      X(r176, 176 | GPR | Arch_cuda) \
      X(r177, 177 | GPR | Arch_cuda) \
      X(r178, 178 | GPR | Arch_cuda) \
      X(r179, 179 | GPR | Arch_cuda) \
      X(r180, 180 | GPR | Arch_cuda) \
      X(r181, 181 | GPR | Arch_cuda) \
      X(r182, 182 | GPR | Arch_cuda) \
      X(r183, 183 | GPR | Arch_cuda) \
      X(r184, 184 | GPR | Arch_cuda) \
      X(r185, 185 | GPR | Arch_cuda) \
      X(r186, 186 | GPR | Arch_cuda) \
      X(r187, 187 | GPR | Arch_cuda) \
      X(r188, 188 | GPR | Arch_cuda) \
      X(r189, 189 | GPR | Arch_cuda) \
      X(r190, 190 | GPR | Arch_cuda) \
      X(r191, 191 | GPR | Arch_cuda) \
      X(r192, 192 | GPR | Arch_cuda) \
      X(r193, 193 | GPR | Arch_cuda) \
      X(r194, 194 | GPR | Arch_cuda) \
      X(r195, 195 | GPR | Arch_cuda) \
      X(r196, 196 | GPR | Arch_cuda) \
      X(r197, 197 | GPR | Arch_cuda) \
      X(r198, 198 | GPR | Arch_cuda) \
      X(r199, 199 | GPR | Arch_cuda) \
      X(r200, 200 | GPR | Arch_cuda) \
      X(r201, 201 | GPR | Arch_cuda) \
      X(r202, 202 | GPR | Arch_cuda) \
      X(r203, 203 | GPR | Arch_cuda) \
      X(r204, 204 | GPR | Arch_cuda) \
      X(r205, 205 | GPR | Arch_cuda) \
      X(r206, 206 | GPR | Arch_cuda) \
      X(r207, 207 | GPR | Arch_cuda) \
      X(r208, 208 | GPR | Arch_cuda) \
      X(r209, 209 | GPR | Arch_cuda) \
      X(r210, 210 | GPR | Arch_cuda) \
      X(r211, 211 | GPR | Arch_cuda) \
      X(r212, 212 | GPR | Arch_cuda) \
      X(r213, 213 | GPR | Arch_cuda) \
      X(r214, 214 | GPR | Arch_cuda) \
      X(r215, 215 | GPR | Arch_cuda) \
      X(r216, 216 | GPR | Arch_cuda) \
      X(r217, 217 | GPR | Arch_cuda) \
      X(r218, 218 | GPR | Arch_cuda) \
      X(r219, 219 | GPR | Arch_cuda) \
      X(r220, 220 | GPR | Arch_cuda) \
      X(r221, 221 | GPR | Arch_cuda) \
      X(r222, 222 | GPR | Arch_cuda) \
      X(r223, 223 | GPR | Arch_cuda) \
      X(r224, 224 | GPR | Arch_cuda) \
      X(r225, 225 | GPR | Arch_cuda) \
      X(r226, 226 | GPR | Arch_cuda) \
      X(r227, 227 | GPR | Arch_cuda) \
      X(r228, 228 | GPR | Arch_cuda) \
      X(r229, 229 | GPR | Arch_cuda) \
      X(r230, 230 | GPR | Arch_cuda) \
      X(r231, 231 | GPR | Arch_cuda) \
      X(r232, 232 | GPR | Arch_cuda) \
      X(r233, 233 | GPR | Arch_cuda) \
      X(r234, 234 | GPR | Arch_cuda) \
      X(r235, 235 | GPR | Arch_cuda) \
      X(r236, 236 | GPR | Arch_cuda) \
      X(r237, 237 | GPR | Arch_cuda) \
      X(r238, 238 | GPR | Arch_cuda) \
      X(r239, 239 | GPR | Arch_cuda) \
      X(r240, 240 | GPR | Arch_cuda) \
      X(r241, 241 | GPR | Arch_cuda) \
      X(r242, 242 | GPR | Arch_cuda) \
      X(r243, 243 | GPR | Arch_cuda) \
      X(r244, 244 | GPR | Arch_cuda) \
      X(r245, 245 | GPR | Arch_cuda) \
      X(r246, 246 | GPR | Arch_cuda) \
      X(r247, 247 | GPR | Arch_cuda) \
      X(r248, 248 | GPR | Arch_cuda) \
      X(r249, 249 | GPR | Arch_cuda) \
      X(r250, 250 | GPR | Arch_cuda) \
      X(r251, 251 | GPR | Arch_cuda) \
      X(r252, 252 | GPR | Arch_cuda) \
      X(r253, 253 | GPR | Arch_cuda) \
      X(r254, 254 | GPR | Arch_cuda) \
      X(r255, 255 | GPR | Arch_cuda) \
      X(ur0, 0 |  UR | Arch_cuda) \
      X(ur1, 1 |  UR | Arch_cuda) \
      X(ur2, 2 |  UR | Arch_cuda) \
      X(ur3, 3 |  UR | Arch_cuda) \
      X(ur4, 4 |  UR | Arch_cuda) \
      X(ur5, 5 |  UR | Arch_cuda) \
      X(ur6, 6 |  UR | Arch_cuda) \
      X(ur7, 7 |  UR | Arch_cuda) \
      X(ur8, 8 |  UR | Arch_cuda) \
      X(ur9, 9 |  UR | Arch_cuda) \
      X(ur10, 10 |  UR | Arch_cuda) \
      X(ur11, 11 |  UR | Arch_cuda) \
      X(ur12, 12 |  UR | Arch_cuda) \
      X(ur13, 13 |  UR | Arch_cuda) \
      X(ur14, 14 |  UR | Arch_cuda) \
      X(ur15, 15 |  UR | Arch_cuda) \
      X(ur16, 16 |  UR | Arch_cuda) \
      X(ur17, 17 |  UR | Arch_cuda) \
      X(ur18, 18 |  UR | Arch_cuda) \
      X(ur19, 19 |  UR | Arch_cuda) \
      X(ur20, 20 |  UR | Arch_cuda) \
      X(ur21, 21 |  UR | Arch_cuda) \
      X(ur22, 22 |  UR | Arch_cuda) \
      X(ur23, 23 |  UR | Arch_cuda) \
      X(ur24, 24 |  UR | Arch_cuda) \
      X(ur25, 25 |  UR | Arch_cuda) \
      X(ur26, 26 |  UR | Arch_cuda) \
      X(ur27, 27 |  UR | Arch_cuda) \
      X(ur28, 28 |  UR | Arch_cuda) \
      X(ur29, 29 |  UR | Arch_cuda) \
      X(ur30, 30 |  UR | Arch_cuda) \
      X(ur31, 31 |  UR | Arch_cuda) \
      X(ur32, 32 |  UR | Arch_cuda) \
      X(ur33, 33 |  UR | Arch_cuda) \
      X(ur34, 34 |  UR | Arch_cuda) \
      X(ur35, 35 |  UR | Arch_cuda) \
      X(ur36, 36 |  UR | Arch_cuda) \
      X(ur37, 37 |  UR | Arch_cuda) \
      X(ur38, 38 |  UR | Arch_cuda) \
      X(ur39, 39 |  UR | Arch_cuda) \
      X(ur40, 40 |  UR | Arch_cuda) \
      X(ur41, 41 |  UR | Arch_cuda) \
      X(ur42, 42 |  UR | Arch_cuda) \
      X(ur43, 43 |  UR | Arch_cuda) \
      X(ur44, 44 |  UR | Arch_cuda) \
      X(ur45, 45 |  UR | Arch_cuda) \
      X(ur46, 46 |  UR | Arch_cuda) \
      X(ur47, 47 |  UR | Arch_cuda) \
      X(ur48, 48 |  UR | Arch_cuda) \
      X(ur49, 49 |  UR | Arch_cuda) \
      X(ur50, 50 |  UR | Arch_cuda) \
      X(ur51, 51 |  UR | Arch_cuda) \
      X(ur52, 52 |  UR | Arch_cuda) \
      X(ur53, 53 |  UR | Arch_cuda) \
      X(ur54, 54 |  UR | Arch_cuda) \
      X(ur55, 55 |  UR | Arch_cuda) \
      X(ur56, 56 |  UR | Arch_cuda) \
      X(ur57, 57 |  UR | Arch_cuda) \
      X(ur58, 58 |  UR | Arch_cuda) \
      X(ur59, 59 |  UR | Arch_cuda) \
      X(ur60, 60 |  UR | Arch_cuda) \
      X(ur61, 61 |  UR | Arch_cuda) \
      X(ur62, 62 |  UR | Arch_cuda) \
      X(ur63, 63 |  UR | Arch_cuda) \
      X(pc, 256 | GPR | Arch_cuda) \
      X(p0, 0 |  PR | Arch_cuda) \
      X(p1, 1 |  PR | Arch_cuda) \
      X(p2, 2 |  PR | Arch_cuda) \
      X(p3, 3 |  PR | Arch_cuda) \
      X(p4, 4 |  PR | Arch_cuda) \
      X(p5, 5 |  PR | Arch_cuda) \
      X(p6, 6 |  PR | Arch_cuda) \
      X(b1, 1 |  BR | Arch_cuda) \
      X(b2, 2 |  BR | Arch_cuda) \
      X(b3, 3 |  BR | Arch_cuda) \
      X(b4, 4 |  BR | Arch_cuda) \
      X(b5, 5 |  BR | Arch_cuda) \
      X(b6, 6 |  BR | Arch_cuda) \
      X(up0, 0 | UPR | Arch_cuda) \
      X(up1, 1 | UPR | Arch_cuda) \
      X(up2, 2 | UPR | Arch_cuda) \
      X(up3, 3 | UPR | Arch_cuda) \
      X(up4, 4 | UPR | Arch_cuda) \
      X(up5, 5 | UPR | Arch_cuda) \
      X(up6, 6 | UPR | Arch_cuda) \
      X(up7, 7 | UPR | Arch_cuda) \
      X(up8, 8 | UPR | Arch_cuda) \
      X(up9, 9 | UPR | Arch_cuda) \
      X(up10, 10 | UPR | Arch_cuda) \
      X(up11, 11 | UPR | Arch_cuda) \
      X(up12, 12 | UPR | Arch_cuda) \
      X(up13, 13 | UPR | Arch_cuda) \
      X(up14, 14 | UPR | Arch_cuda) \
      X(up15, 15 | UPR | Arch_cuda)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_CUDA_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_CUDA_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}}

#endif
