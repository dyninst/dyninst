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


#if !defined(IA32_LOCS_H)
#define IA32_LOCS_H

class ia32_locations {
    public:
   ia32_locations() : num_prefixes(0), opcode_size(0), opcode_position(-1),
   disp_size(0), disp_position(-1), imm_cnt(0),
   modrm_position(-1), modrm_operand(-1), modrm_byte(0), modrm_mod(0),
   modrm_rm(0), modrm_reg(0), sib_byte(0), sib_position(-1),
   rex_position(-1), rex_byte(0), rex_w(0), rex_r(0), rex_x(0), rex_b(0),
   address_size(0) {
       imm_position[0] = -1;
       imm_size[0] = 0;
       imm_position[1] = -1;
       imm_size[1] = 0;
   }
   void reinit() {
       num_prefixes = 0;
       opcode_size = 0;
       opcode_position = -1;
       disp_size = 0;
       disp_position = -1;
       imm_cnt = 0;
       imm_position[0] = -1;
       imm_size[0] = -1;
       imm_position[1] = -1;
       imm_size[1] = -1;
       modrm_position = -1;
       modrm_operand = -1;
       modrm_byte = 0;
       modrm_mod = 0;
       modrm_rm = 0;
       modrm_reg = 0;
       sib_byte = 0;
       sib_position = -1;
       rex_position = -1;
       rex_byte = 0;
       rex_w = 0;
       rex_r = 0;
       rex_x = 0;
       rex_b = 0;
       address_size = 0;
   }
   int num_prefixes;
   unsigned int opcode_size;
   int opcode_position;
   
   unsigned disp_size;
   int disp_position;

   int imm_cnt;
   
   int modrm_position;
   int modrm_operand;
   unsigned char modrm_byte;
   unsigned char modrm_mod;
   unsigned char modrm_rm;
   unsigned char modrm_reg;

   unsigned char sib_byte;
   int sib_position;
   
   int rex_position;
   unsigned char rex_byte;
   unsigned char rex_w;
   unsigned char rex_r;
   unsigned char rex_x;
   unsigned char rex_b;

   int address_size;
   int imm_position[2];
   unsigned int imm_size[2];
};

#endif //!defined(IA32_LOCS_H)

