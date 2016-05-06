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
         //
         // Since PPC opcodes are all 32 bits, work with them in their native
         // size and don't assume any endianness.  When the final copy into 
         // the byte buffer occurs, make the assumption that the target
         // architecture matches that of the host.
#if defined(arch_ppc_little_endian_test)
         uint32_t ins[] = {
            /*  0 */ 0x60000000, // nop
            /*  1 */ 0x3d8c0000, // lis r12, 0              # Add Immediate Shifted (calltarg[15-8]) to r12
            /*  2 */ 0x618c0000, // ori r12, r12, 0         # Or Immediate (calltarg[7-0]) to r12
            /*  3 */ 0x780007c6, // rldicr r12, r12, 32, 31 # Rotate Left Doubleword Immediate then Clear Right
            /*  4 */ 0x658c0000, // oris r12, r12, 0        # Or Immediate Shifted (calltarg[31-24]) to r12
            /*  5 */ 0x618c0000, // ori r12, r12, 0         # Or Immediate (calltarg[23-16]) to r12
            /*  6 */ 0x7d8903a6, // mtctr r12               # Move r12 to Count Register
            /*  7 */ 0x4e800421, // bctrl                   # Branch to Count Register
            /*  8 */ 0x7d821008  // trap                    # Trap to return from RPC
         };
#else
         uint32_t ins[] = {
            /*  0 */ 0x60000000, // nop
            /*  1 */ 0x3c000000, // lis r0, 0               # Add Immediate Shifted (calltarg[15-8]) to r0
            /*  2 */ 0x60000000, // ori r0, r0, 0           # Or Immediate (calltarg[7-0]) to r0
            /*  3 */ 0x780007c6, // rldicr r0, r0, 32, 31   # Rotate Left Doubleword Immediate then Clear Right
            /*  4 */ 0x64000000, // oris r0, r0, 0          # Or Immediate Shifted (calltarg[31-24]) to r0
            /*  5 */ 0x60000000, // ori r0, r0, 0           # Or Immediate (calltarg[23-16]) to r0
            /*  6 */ 0x7c0903a6, // mtctr r0                # Move r0 to Count Register
            /*  7 */ 0x3c400000, // lis r2, 0               # Add Immediate Shifted (calltarg[15-8]) to TOC(aka r2)
            /*  8 */ 0x60420000, // ori r2,r2,0             # Or Immediate (calltarg[7-0]) to TOC(aka r2)
            /*  9 */ 0x784207c6, // rldicr  r2,r2,32,31     # Rotate Left Doubleword Immediate then Clear Right
            /* 10 */ 0x64420000, // oris    r2,r2,0         # Or Immediate Shifted (calltarg[31-24]) to TOC(aka r2)
            /* 11 */ 0x60420000, // ori     r2,r2,0         # Or Immediate (calltarg[23-16]) to TOC(aka r2)
            /* 12 */ 0x4e800421, // bctrl                   # Branch to Count Register
            /* 13 */ 0x7d821008  // trap                    # Trap to return from RPC
         };
#endif // defined(arch_ppc_little_endian_test)

         // copy address to buffer
         ins[1] |= (uint32_t)(((uint64_t)calltarg >> 48) & 0xffff);
         ins[2] |= (uint32_t)(((uint64_t)calltarg >> 32) & 0xffff);
         ins[4] |= (uint32_t)((calltarg >> 16) & 0xffff);
         ins[5] |= (uint32_t)(calltarg & 0xffff);

#if !defined(arch_ppc_little_endian_test)
         // copy TOC value into buffer
         ins[7] |= (uint32_t)(((uint64_t)tocval >> 48) & 0xffff);
         ins[8] |= (uint32_t)(((uint64_t)tocval >> 32) & 0xffff);
         ins[10] |= (uint32_t)((tocval >> 16) & 0xffff);
         ins[11] |= (uint32_t)(tocval & 0xffff);
#endif // !defined(arch_ppc_little_endian_test_test)

         buffer_size = sizeof(ins);
         buffer = (unsigned char *)malloc(buffer_size);
         memcpy(buffer, ins, buffer_size);
         start_offset = 4;
         break;
      }
      case(Dyninst::Arch_aarch64): {
         //nop
         //mov x0, <addr>
         //blr x0
         //brk #0
         unsigned int addr_pos = 4;
         unsigned char tmp_buf[] = {
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
         (*(((unsigned char *) buffer) + POS + 1)) |= ((VAL>>11)&0x1f);\
         (*(((unsigned char *) buffer) + POS + 2)) |= ((VAL>> 3)&0xff);\
         (*(((unsigned char *) buffer) + POS + 3)) |= ((VAL<< 5)&0xf0);

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
            pthrd_printf("0x%8x\n", *((unsigned int*)((unsigned char *)buffer+i)) );
         }

         break;
      }
      default:
         perr_printf("Unknown architecture!");
         assert(0);
   }
}

