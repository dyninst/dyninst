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

static void createBuffer(Process::ptr proc,
                         Dyninst::Address calltarg,
                         Dyninst::Address tocval,
                         unsigned char* &buffer, unsigned &buffer_size,
                         unsigned long &start_offset)
{
   switch (proc->getArchitecture()) {
      case Dyninst::Arch_x86_64: {
         buffer = (unsigned char *) malloc(17);
         //nop, nop, nop, nop
         buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0x90;
         //mov %rax, $imm
         buffer[4] = 0x48; buffer[5] = 0xb8;
         memcpy(buffer+6, &calltarg, 8);
         //call %rax
         buffer[14] = 0xff; buffer[15] = 0xd0;
         //Trap
         buffer[16] = 0xcc;
         buffer_size = 17;
         start_offset = 4;
         break;
      }
      case Dyninst::Arch_x86: {
         buffer = (unsigned char *) malloc(12);
         uint32_t addr32 = (uint32_t) calltarg;
         //nop, nop, nop, nop
         buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0x90;
         //mov %eax, $imm
         buffer[4] = 0xb8;
         memcpy(buffer+5, &addr32, 4);
         //call %eax
         buffer[9] = 0xff; buffer[10] = 0xd0;
         //Trap
         buffer[11] = 0xcc;
         buffer_size = 12;
         start_offset = 4;
         break;
      }
      case Dyninst::Arch_ppc32: {
         buffer_size = 6*4;
         buffer = (unsigned char *)malloc(buffer_size);
         uint32_t addr32 = (uint32_t) calltarg;
         // nop
         buffer[0] = 0x60; buffer[1] = 0x00; buffer[2] = 0x00; buffer[3] = 0x00;
         // lis r0, 0
         buffer[4] = 0x3c; buffer[5] = 0x00; buffer[6] = 0x00; buffer[7] = 0x00;
         // ori r0, r0, 0
         buffer[8] = 0x60; buffer[9] = 0x00; buffer[10] = 0x00; buffer[11] = 0x00;
         // mtctr r0
         buffer[12] = 0x7c; buffer[13] = 0x09; buffer[14] = 0x03; buffer[15] = 0xa6;
         // bctrl
         buffer[16] = 0x4e; buffer[17] = 0x80; buffer[18] = 0x04; buffer[19] = 0x21;
         // trap
         buffer[20] = 0x7d; buffer[21] = 0x82; buffer[22] = 0x10; buffer[23] = 0x08;
         start_offset = 4;

         // copy address into buffer
         *((uint16_t *) (buffer + 6)) = (uint16_t)(addr32 >> 16);
         *((uint16_t *) (buffer + 10)) = (uint16_t)addr32;
         break;
      }
      case Dyninst::Arch_ppc64: {
         buffer_size = 15*4;
         buffer = (unsigned char *)malloc(buffer_size);
         // nop
         buffer[0] = 0x60; buffer[1] = 0x00; buffer[2] = 0x00; buffer[3] = 0x00;
         // lis r0, 0
         buffer[4] = 0x3c; buffer[5] = 0x00; buffer[6] = 0x00; buffer[7] = 0x00;
         // ori r0, r0, 0
         buffer[8] = 0x60; buffer[9] = 0x00; buffer[10] = 0x00; buffer[11] = 0x00;
         // rldicr r0, r0, 32, 31
         buffer[12] = 0x78; buffer[13] = 0x00; buffer[14] = 0x07; buffer[15] = 0xc6;
         // oris r0, r0, 0
         buffer[16] = 0x64; buffer[17] = 0x00; buffer[18] = 0x00; buffer[19] = 0x00;
         // ori r0, r0, 0
         buffer[20] = 0x60; buffer[21] = 0x00; buffer[22] = 0x00; buffer[23] = 0x00;
         // mtctr
         buffer[24] = 0x7c; buffer[25] = 0x09; buffer[26] = 0x03; buffer[27] = 0xa6;
         // lis r2, 0
         buffer[28] = 0x3c; buffer[29] = 0x40; buffer[30] = 0x00; buffer[31] = 0x00;
         // ori     r2,r2,0
         buffer[32] = 0x60; buffer[33] = 0x42; buffer[34] = 0x00; buffer[35] = 0x00;
         // rldicr  r2,r2,32,31
         buffer[36] = 0x78; buffer[37] = 0x42; buffer[38] = 0x07; buffer[39] = 0xc6;
         // oris    r2,r2,0
         buffer[40] = 0x64; buffer[41] = 0x42; buffer[42] = 0x00; buffer[43] = 0x00;
         // ori     r2,r2,0
         buffer[44] = 0x60; buffer[45] = 0x42; buffer[46] = 0x00; buffer[47] = 0x00;
         // li      r11,0
         buffer[48] = 0x39; buffer[49] = 0x60; buffer[50] = 0x00; buffer[51] = 0x00;
         // bctrl
         buffer[52] = 0x4e; buffer[53] = 0x80; buffer[54] = 0x04; buffer[55] = 0x21;
         // trap
         buffer[56] = 0x7d; buffer[57] = 0x82; buffer[58] = 0x10; buffer[59] = 0x08;
         start_offset = 4;

         // copy address to buffer
         *((uint16_t *) (buffer + 6)) = (uint16_t)((uint64_t)calltarg >> 48);
         *((uint16_t *) (buffer + 10)) = (uint16_t)((uint64_t)calltarg >> 32);
         *((uint16_t *) (buffer + 18)) = (uint16_t)(calltarg >> 16);
         *((uint16_t *) (buffer + 22)) = (uint16_t)(calltarg);

         // copy TOC value into buffer
         *((uint16_t *) (buffer + 30)) = (uint16_t)((uint64_t)tocval >> 48);
         *((uint16_t *) (buffer + 34)) = (uint16_t)((uint64_t)tocval >> 32);
         *((uint16_t *) (buffer + 42)) = (uint16_t)(tocval >> 16);
         *((uint16_t *) (buffer + 46)) = (uint16_t)(tocval);
         break;
      }
        case(Dyninst::Arch_aarch64): {
            //nop
            //mov x0, <addr>
            //blr x0
            //brk #0
            unsigned int addr_pos = 4;
            char tmp_buf[] = {
                0xd5, 0x03, 0x20, 0x1f,     // nop
                0xd2, 0x80, 0x00, 0x00,     // mov  x0, #0           ;<addr>
                0xf2, 0xa0, 0x00, 0x00,     // movk x0, #0, lsl #16     ;<addr>
                0xf2, 0xc0, 0x00, 0x00,     // movk x0, #0, lsl #32     ;<addr>
                0xf2, 0xe0, 0x00, 0x00,     // movk x0, #0, lsl #48     ;<addr>
                0xd6, 0x3f, 0x00, 0x00,     // blr x0
                0xd4, 0x20, 0x00, 0x00      // brk #0
            };
            buffer_size = sizeof(tmp_buf);
            buffer = (unsigned char *)malloc(buffer_size);
            start_offset = 4;

            memcpy(buffer, tmp_buf, buffer_size);

#define BYTE_ASSGN(POS, VAL)\
            (*(((char *) buffer) + POS + 1)) |= ((VAL>>11)&0x1f);\
            (*(((char *) buffer) + POS + 2)) |= ((VAL>> 3)&0xff);\
            (*(((char *) buffer) + POS + 3)) |= ((VAL<< 5)&0xf0);

            BYTE_ASSGN(addr_pos,    (uint16_t)calltarg )
            BYTE_ASSGN(addr_pos+4,  (uint16_t)(calltarg>>16) )
            BYTE_ASSGN(addr_pos+8,  (uint16_t)(calltarg>>32) )
            BYTE_ASSGN(addr_pos+12, (uint16_t)(calltarg>>48) )

#define SWAP4BYTE(POS) \
            buffer[POS+3]^= buffer[POS]; \
            buffer[POS]  ^= buffer[POS+3]; \
            buffer[POS+3]^= buffer[POS]; \
            buffer[POS+2]^= buffer[POS+1]; \
            buffer[POS+1]^= buffer[POS+2]; \
            buffer[POS+2]^= buffer[POS+1];

            for(unsigned int i = 0; i < buffer_size; i+=4){
                SWAP4BYTE(i)
                pthrd_printf("0x%8x\n", *((unsigned int*)((char *)buffer+i)) );
            }

            break;
        }
        default:
            perr_printf("Unknown architecture!");
            assert(0);
   }
}

