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
  //          (  name,  ID | cat |      arch,   arch)
  DEF_REGISTER(    r0,   0 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r1,   1 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r2,   2 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r3,   3 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r4,   4 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r5,   5 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r6,   6 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r7,   7 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r8,   8 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(    r9,   9 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r10,  10 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r11,  11 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r12,  12 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r13,  13 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r14,  14 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r15,  15 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r16,  16 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r17,  17 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r18,  18 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r19,  19 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r20,  20 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r21,  21 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r22,  22 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r23,  23 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r24,  24 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r25,  25 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r26,  26 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r27,  27 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r28,  28 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r29,  29 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r30,  30 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r31,  31 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r32,  32 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r33,  33 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r34,  34 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r35,  35 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r36,  36 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r37,  37 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r38,  38 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r39,  39 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r40,  40 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r41,  41 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r42,  42 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r43,  43 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r44,  44 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r45,  45 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r46,  46 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r47,  47 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r48,  48 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r49,  49 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r50,  50 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r51,  51 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r52,  52 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r53,  53 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r54,  54 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r55,  55 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r56,  56 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r57,  57 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r58,  58 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r59,  59 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r60,  60 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r61,  61 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r62,  62 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r63,  63 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r64,  64 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r65,  65 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r66,  66 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r67,  67 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r68,  68 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r69,  69 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r70,  70 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r71,  71 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r72,  72 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r73,  73 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r74,  74 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r75,  75 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r76,  76 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r77,  77 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r78,  78 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r79,  79 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r80,  80 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r81,  81 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r82,  82 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r83,  83 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r84,  84 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r85,  85 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r86,  86 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r87,  87 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r88,  88 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r89,  89 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r90,  90 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r91,  91 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r92,  92 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r93,  93 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r94,  94 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r95,  95 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r96,  96 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r97,  97 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r98,  98 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(   r99,  99 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r100, 100 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r101, 101 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r102, 102 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r103, 103 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r104, 104 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r105, 105 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r106, 106 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r107, 107 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r108, 108 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r109, 109 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r110, 110 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r111, 111 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r112, 112 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r113, 113 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r114, 114 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r115, 115 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r116, 116 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r117, 117 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r118, 118 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r119, 119 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r120, 120 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r121, 121 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r122, 122 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r123, 123 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r124, 124 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r125, 125 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r126, 126 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r127, 127 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r128, 128 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r129, 129 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r130, 130 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r131, 131 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r132, 132 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r133, 133 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r134, 134 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r135, 135 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r136, 136 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r137, 137 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r138, 138 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r139, 139 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r140, 140 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r141, 141 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r142, 142 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r143, 143 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r144, 144 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r145, 145 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r146, 146 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r147, 147 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r148, 148 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r149, 149 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r150, 150 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r151, 151 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r152, 152 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r153, 153 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r154, 154 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r155, 155 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r156, 156 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r157, 157 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r158, 158 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r159, 159 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r160, 160 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r161, 161 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r162, 162 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r163, 163 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r164, 164 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r165, 165 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r166, 166 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r167, 167 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r168, 168 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r169, 169 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r170, 170 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r171, 171 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r172, 172 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r173, 173 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r174, 174 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r175, 175 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r176, 176 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r177, 177 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r178, 178 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r179, 179 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r180, 180 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r181, 181 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r182, 182 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r183, 183 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r184, 184 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r185, 185 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r186, 186 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r187, 187 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r188, 188 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r189, 189 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r190, 190 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r191, 191 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r192, 192 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r193, 193 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r194, 194 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r195, 195 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r196, 196 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r197, 197 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r198, 198 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r199, 199 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r200, 200 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r201, 201 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r202, 202 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r203, 203 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r204, 204 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r205, 205 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r206, 206 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r207, 207 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r208, 208 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r209, 209 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r210, 210 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r211, 211 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r212, 212 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r213, 213 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r214, 214 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r215, 215 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r216, 216 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r217, 217 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r218, 218 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r219, 219 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r220, 220 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r221, 221 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r222, 222 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r223, 223 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r224, 224 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r225, 225 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r226, 226 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r227, 227 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r228, 228 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r229, 229 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r230, 230 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r231, 231 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r232, 232 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r233, 233 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r234, 234 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r235, 235 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r236, 236 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r237, 237 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r238, 238 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r239, 239 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r240, 240 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r241, 241 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r242, 242 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r243, 243 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r244, 244 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r245, 245 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r246, 246 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r247, 247 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r248, 248 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r249, 249 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r250, 250 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r251, 251 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r252, 252 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r253, 253 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r254, 254 | GPR | Arch_cuda, "cuda");
  DEF_REGISTER(  r255, 255 | GPR | Arch_cuda, "cuda");

  // uniform registers
  DEF_REGISTER(   ur0,   0 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur1,   1 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur2,   2 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur3,   3 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur4,   4 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur5,   5 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur6,   6 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur7,   7 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur8,   8 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(   ur9,   9 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur10,  10 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur11,  11 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur12,  12 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur13,  13 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur14,  14 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur15,  15 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur16,  16 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur17,  17 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur18,  18 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur19,  19 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur20,  20 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur21,  21 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur22,  22 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur23,  23 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur24,  24 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur25,  25 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur26,  26 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur27,  27 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur28,  28 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur29,  29 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur30,  30 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur31,  31 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur32,  32 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur33,  33 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur34,  34 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur35,  35 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur36,  36 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur37,  37 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur38,  38 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur39,  39 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur40,  40 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur41,  41 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur42,  42 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur43,  43 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur44,  44 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur45,  45 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur46,  46 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur47,  47 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur48,  48 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur49,  49 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur50,  50 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur51,  51 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur52,  52 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur53,  53 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur54,  54 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur55,  55 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur56,  56 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur57,  57 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur58,  58 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur59,  59 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur60,  60 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur61,  61 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur62,  62 |  UR | Arch_cuda, "cuda");
  DEF_REGISTER(  ur63,  63 |  UR | Arch_cuda, "cuda");

  // Placeholder for a pc register, so that we don't assert
  DEF_REGISTER(    pc, 256 | GPR | Arch_cuda, "cuda");

  // Predicate registers used as source or dest operands
  // Different from a predicate register used as instruction predicate,
  // which is handle by operand::isTruePredicate and operand::isFalsePredicate
  DEF_REGISTER(    p0,   0 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p1,   1 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p2,   2 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p3,   3 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p4,   4 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p5,   5 |  PR | Arch_cuda, "cuda");
  DEF_REGISTER(    p6,   6 |  PR | Arch_cuda, "cuda");

  DEF_REGISTER(    b1,   1 |  BR | Arch_cuda, "cuda");
  DEF_REGISTER(    b2,   2 |  BR | Arch_cuda, "cuda");
  DEF_REGISTER(    b3,   3 |  BR | Arch_cuda, "cuda");
  DEF_REGISTER(    b4,   4 |  BR | Arch_cuda, "cuda");
  DEF_REGISTER(    b5,   5 |  BR | Arch_cuda, "cuda");
  DEF_REGISTER(    b6,   6 |  BR | Arch_cuda, "cuda");

  // XXX(Keren): not sure how many uprs, use 16 for safety
  DEF_REGISTER(   up0,   0 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up1,   1 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up2,   2 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up3,   3 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up4,   4 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up5,   5 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up6,   6 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up7,   7 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up8,   8 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(   up9,   9 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up10,  10 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up11,  11 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up12,  12 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up13,  13 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up14,  14 | UPR | Arch_cuda, "cuda");
  DEF_REGISTER(  up15,  15 | UPR | Arch_cuda, "cuda");

}}

#endif
