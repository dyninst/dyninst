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

#include "mmapalloc.h"
#include <sys/mman.h>
#include <string.h>
#include "unaligned_memory_access.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/aarch64_regs.h"

static const unsigned int linux_x86_64_mmap_flags_position = 26;
static const unsigned int linux_x86_64_mmap_size_position = 43;
static const unsigned int linux_x86_64_mmap_addr_position = 49;
static const unsigned int linux_x86_64_mmap_start_position = 4;
static const unsigned char linux_x86_64_call_mmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop,nop,nop,nop
   0x48, 0x8d, 0x64, 0x24, 0x80,                   //lea    -128(%rsp),%rsp
   0x49, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r8
   0x49, 0xc7, 0xc1, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r9
   0x49, 0xc7, 0xc2, 0x22, 0x00, 0x00, 0x00,       //mov    $0x22,%r10
   0x48, 0xc7, 0xc2, 0x07, 0x00, 0x00, 0x00,       //mov    $0x7,%rdx
   0x48, 0x31, 0xf6,                               //xor    %rsi,%rsi
   0x48, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00,       //mov    $<size>,%rsi
   0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $<addr>,%rdi
   0x00, 0x00, 0x00,                               //
   0x48, 0xc7, 0xc0, 0x09, 0x00, 0x00, 0x00,       //mov    $0x9,%rax
   0x0f, 0x05,                                     //syscall
   0x48, 0x8d, 0xa4, 0x24, 0x80, 0x00, 0x00, 0x00, //lea    128(%rsp),%rsp
   0xcc,                                           //Trap
   0x90                                            //nop
};
static const unsigned int linux_x86_64_call_mmap_size = sizeof(linux_x86_64_call_mmap);

static const unsigned int linux_x86_64_munmap_size_position = 15;
static const unsigned int linux_x86_64_munmap_addr_position = 21;
static const unsigned int linux_x86_64_munmap_start_position = 4;
static const unsigned char linux_x86_64_call_munmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop,nop,nop,nop
   0x48, 0x8d, 0x64, 0x24, 0x80,                   //lea    -128(%rsp),%rsp
   0x48, 0x31, 0xf6,                               //xor    %rsi,%rsi
   0x48, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00,       //mov    $<size>,%rsi
   0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $<addr>,%rdi
   0x00, 0x00, 0x00,                               //
   0x48, 0xc7, 0xc0, 0x0b, 0x00, 0x00, 0x00,       //mov    $0xb,%rax
   0x0f, 0x05,                                     //syscall
   0x48, 0x8d, 0xa4, 0x24, 0x80, 0x00, 0x00, 0x00, //lea    128(%rsp),%rsp
   0xcc,                                           //Trap
   0x90                                            //nop
};
static const unsigned int linux_x86_64_call_munmap_size = sizeof(linux_x86_64_call_munmap);

static const unsigned int linux_x86_mmap_flags_position = 20;
static const unsigned int linux_x86_mmap_size_position = 10;
static const unsigned int linux_x86_mmap_addr_position = 5;
static const unsigned int linux_x86_mmap_start_position = 4;
static const unsigned char linux_x86_call_mmap[] = {
   0x90, 0x90, 0x90, 0x90,                //nop; nop; nop; nop
   0xbb, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebx  (addr)
   0xb9, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ecx  (size)
   0xba, 0x07, 0x00, 0x00, 0x00,          //mov    $0x7,%edx  (perms)
   0xbe, 0x22, 0x00, 0x00, 0x00,          //mov    $0x22,%esi (flags)
   0xbf, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%edi  (fd)
   0xbd, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebp  (offset)
   0xb8, 0xc0, 0x00, 0x00, 0x00,          //mov    $0xc0,%eax (syscall)
   0xcd, 0x80,                            //int    $0x80
   0xcc,                                  //Trap
   0x90                                   //nop
};
static const unsigned int linux_x86_call_mmap_size = sizeof(linux_x86_call_mmap);

static const unsigned int linux_x86_munmap_size_position = 10;
static const unsigned int linux_x86_munmap_addr_position = 5;
static const unsigned int linux_x86_munmap_start_position = 4;
static const unsigned char linux_x86_call_munmap[] = {
   0x90, 0x90, 0x90, 0x90,                //nop; nop; nop; nop
   0xbb, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebx  (addr)
   0xb9, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ecx  (size)
   0xb8, 0xc0, 0x00, 0x00, 0x00,          //mov    $0x5b,%eax (syscall)
   0xcd, 0x80,                            //int    $0x80
   0xcc,                                  //Trap
   0x90                                   //nop
};
static const unsigned int linux_x86_call_munmap_size = sizeof(linux_x86_call_munmap);

static const unsigned int linux_ppc32_mmap_flags_hi_position = 34;
static const unsigned int linux_ppc32_mmap_flags_lo_position = 38;
static const unsigned int linux_ppc32_mmap_size_hi_position = 18;
static const unsigned int linux_ppc32_mmap_size_lo_position = 22;
static const unsigned int linux_ppc32_mmap_addr_hi_position = 10;
static const unsigned int linux_ppc32_mmap_addr_lo_position = 14;
static const unsigned int linux_ppc32_mmap_start_position = 4;
static const unsigned char linux_ppc32_call_mmap[] = {
   0x60, 0x00, 0x00, 0x00,            // nop
   0x38, 0x00, 0x00, 0x5a,            // li      r0,<syscall>
   0x3c, 0x60, 0x00, 0x00,            // lis     r3,<addr_hi>
   0x60, 0x63, 0x00, 0x00,            // ori     r3,r3,<addr_lo>
   0x3c, 0x80, 0x00, 0x00,            // lis     r4,<size_hi>
   0x60, 0x84, 0x00, 0x00,            // ori     r4,r4,<size_lo>
   0x3c, 0xa0, 0x00, 0x00,            // lis     r5,<perms_hi>
   0x60, 0xa5, 0x00, 0x07,            // ori     r5,r5,<perms_lo>
   0x3c, 0xc0, 0x00, 0x00,            // lis     r6,<flags_hi>
   0x60, 0xc6, 0x00, 0x00,            // ori     r6,r6,<flags_lo>
   0x3c, 0xe0, 0xff, 0xff,            // lis     r7,<fd_hi>
   0x60, 0xe7, 0xff, 0xff,            // ori     r7,r7,<fd_lo>
   0x3d, 0x00, 0x00, 0x00,            // lis     r8,<offset>
   0x44, 0x00, 0x00, 0x02,            // sc
   0x7d, 0x82, 0x10, 0x08,            // trap
   0x60, 0x00, 0x00, 0x00             // nop
};
static const unsigned int linux_ppc32_call_mmap_size = sizeof(linux_ppc32_call_mmap);

static const unsigned int linux_ppc32_munmap_size_hi_position = 18;
static const unsigned int linux_ppc32_munmap_size_lo_position = 22;
static const unsigned int linux_ppc32_munmap_addr_hi_position = 10;
static const unsigned int linux_ppc32_munmap_addr_lo_position = 14;
static const unsigned int linux_ppc32_munmap_start_position = 4;
static const unsigned char linux_ppc32_call_munmap[] = {
   0x60, 0x60, 0x60, 0x60,              // nop
   0x38, 0x00, 0x00, 0x5b,              // li      r0,<syscall>
   0x3c, 0x60, 0x00, 0x00,              // lis     r3,<addr_hi>
   0x60, 0x63, 0x00, 0x00,              // ori     r3,r3,<addr_lo>
   0x3c, 0x80, 0x00, 0x00,              // lis     r4,<size_hi>
   0x60, 0x84, 0x00, 0x00,              // ori     r4,r4,<size_lo>
   0x44, 0x00, 0x00, 0x02,              // sc
   0x7d, 0x82, 0x10, 0x08,              // trap
   0x60, 0x00, 0x00, 0x00               // nop
};
static const unsigned int linux_ppc32_call_munmap_size = sizeof(linux_ppc32_call_munmap);

static const unsigned int linux_ppc64_mmap_start_position = 4;
static const unsigned int linux_ppc64_mmap_addr_highest_position =    2;
static const unsigned int linux_ppc64_mmap_addr_higher_position =     3;
static const unsigned int linux_ppc64_mmap_addr_hi_position =         5;
static const unsigned int linux_ppc64_mmap_addr_lo_position =         6;
static const unsigned int linux_ppc64_mmap_size_highest_position =    7;
static const unsigned int linux_ppc64_mmap_size_higher_position =     8;
static const unsigned int linux_ppc64_mmap_size_hi_position =        10;
static const unsigned int linux_ppc64_mmap_size_lo_position =        11;
static const unsigned int linux_ppc64_mmap_flags_highest_position =  17;
static const unsigned int linux_ppc64_mmap_flags_higher_position =   18;
static const unsigned int linux_ppc64_mmap_flags_hi_position =       20;
static const unsigned int linux_ppc64_mmap_flags_lo_position =       21;
static const uint32_t linux_ppc64_call_mmap[] = {
   /*  0 */ 0x60000000,              // nop
   /*  1 */ 0x3800005a,              // li      r0,<syscall>
   /*  2 */ 0x3c600000,              // lis     r3,<addr_highest>
   /*  3 */ 0x60630000,              // ori     r3,r3,<addr_higher>
   /*  4 */ 0x786307c6,              // rldicr  r3,r3,0x32,0x31,
   /*  5 */ 0x64630000,              // oris    r3,r3,<addr_hi>
   /*  6 */ 0x60630000,              // ori     r3,r3,<addr_lo>
   /*  7 */ 0x3c800000,              // lis     r4,<size_highest>
   /*  8 */ 0x60840000,              // ori     r4,r4,<size_higher>
   /*  9 */ 0x788407c6,              // rldicr  r4,r4,0x32,0x31,
   /* 10 */ 0x64840000,              // oris    r4,r4,<size_hi>
   /* 11 */ 0x60840000,              // ori     r4,r4,<size_lo>
   /* 12 */ 0x3ca00000,              // lis     r5,<perms_highest>
   /* 13 */ 0x60a50000,              // ori     r5,r5,<perms_higher>
   /* 14 */ 0x78a507c6,              // rldicr  r5,r5,0x32,0x31,
   /* 15 */ 0x64a50000,              // oris    r5,r5,<perms_hi>
   /* 16 */ 0x60a50007,              // ori     r5,r5,<perms_lo>
   /* 17 */ 0x3cc00000,              // lis     r6,<flags_highest>
   /* 18 */ 0x60c60000,              // ori     r6,r6,<flags_higher>
   /* 19 */ 0x78c607c6,              // rldicr  r6,r6,0x32,0x31,
   /* 20 */ 0x64c60000,              // oris    r6,r6,<flags_hi>
   /* 21 */ 0x60c60000,              // ori     r6,r6,<flags_lo>
   /* 22 */ 0x3ce00000,              // lis     r7,<fd=-1>
   /* 23 */ 0x7ce73bb8,              // nand    r7,r7,r7
   /* 24 */ 0x3d000000,              // lis     r8,<offset>
   /* 25 */ 0x44000002,              // sc
   /* 26 */ 0x7d821008,              // trap
   /* 27 */ 0x60000000               // nop
};
static const unsigned int linux_ppc64_call_mmap_size = sizeof(linux_ppc64_call_mmap);

static const unsigned int linux_ppc64_munmap_start_position = 4;
static const unsigned int linux_ppc64_munmap_addr_highest_position =  2;
static const unsigned int linux_ppc64_munmap_addr_higher_position =   3;
static const unsigned int linux_ppc64_munmap_addr_hi_position =       5;
static const unsigned int linux_ppc64_munmap_addr_lo_position =       6;
static const unsigned int linux_ppc64_munmap_size_highest_position =  7;
static const unsigned int linux_ppc64_munmap_size_higher_position =   8;
static const unsigned int linux_ppc64_munmap_size_hi_position =      10;
static const unsigned int linux_ppc64_munmap_size_lo_position =      11;
static const uint32_t linux_ppc64_call_munmap[] = {
   /*  0 */ 0x60000000,              // nop
   /*  1 */ 0x3800005b,              // li      r0,<syscall>
   /*  2 */ 0x3c600000,              // lis     r3,<addr_highest>
   /*  3 */ 0x60630000,              // ori     r3,r3,<addr_higher>
   /*  4 */ 0x786307c6,              // rldicr  r3,r3,0x32,0x31,
   /*  5 */ 0x64630000,              // oris    r3,r3,<addr_hi>
   /*  6 */ 0x60630000,              // ori     r3,r3,<addr_lo>
   /*  7 */ 0x3c800000,              // lis     r4,<size_highest>
   /*  8 */ 0x60840000,              // ori     r4,r4,<size_higher>
   /*  9 */ 0x788407c6,              // rldicr  r4,r4,0x32,0x31,
   /* 10 */ 0x64840000,              // oris    r4,r4,<size_hi>
   /* 11 */ 0x60840000,              // ori     r4,r4,<size_lo>
   /* 12 */ 0x44000002,              // sc
   /* 13 */ 0x7d821008,              // trap
   /* 14 */ 0x60000000               // nop
};
static const unsigned int linux_ppc64_call_munmap_size = sizeof(linux_ppc64_call_munmap);

//aarch64
//mov
//31-21 | 20 - 5 | 4 - 0
//      | imm    | reg
static const unsigned int linux_aarch64_mmap_start_position = 4;
static const unsigned int linux_aarch64_mmap_addr_position =  8;
static const unsigned int linux_aarch64_mmap_size_position =  linux_aarch64_mmap_addr_position +16;
static const unsigned int linux_aarch64_mmap_flags_position = linux_aarch64_mmap_size_position + 16;
static const unsigned char linux_aarch64_call_mmap[] = {
    // _NR_mmap 1058
    0xd5, 0x03, 0x20, 0x1f,         // nop              ;mmap(void *addr, size_t size, int _prot,
                                    //                  ;   int _flags, int _fd, _off_t offset)
    //0xd2, 0x80, 0x84, 0x48,       // mov x8, #d1058   ;pass sys call number
    0xd2, 0x80, 0x1b, 0xc8,         // mov x8, #222   ;pass sys call number,
                                    //222 is correct, 1058 can lead to invalid return val

    0xd2, 0x80, 0x00, 0x00,         // mov  x0, #0           ;<addr>
    0xf2, 0xa0, 0x00, 0x00,         // movk x0, #0, lsl #16     ;<addr>
    0xf2, 0xc0, 0x00, 0x00,         // movk x0, #0, lsl #32     ;<addr>
    0xf2, 0xe0, 0x00, 0x00,         // movk x0, #0, lsl #48     ;<addr>

    0xd2, 0x80, 0x00, 0x01,         // mov  x1, #0               ;<size>
    0xf2, 0xa0, 0x00, 0x01,         // movk x1, #0, lsl #16      ;<size>
    0xf2, 0xc0, 0x00, 0x01,         // movk x1, #0, lsl #32      ;<size>
    0xf2, 0xe0, 0x00, 0x01,         // movk x1, #0, lsl #48      ;<size>

    0xd2, 0x80, 0x00, 0x03,         // mov x3, #0x00            ;<flags>
    0xf2, 0xa0, 0x00, 0x03,         // movk x3, #0x00, lsl #16      ;<flags>
    //0xf2, 0xc0, 0x00, 0x03,         // movk x3, #0x00, lsl #32      ;<flags>
    //0xf2, 0xe0, 0x00, 0x03,         // movk x3, #0x00, lsl #48      ;<flags>

    0xd2, 0x80, 0x00, 0xe2,             // mov x2, #0x7     ;<prot>
    0xd2, 0x80, 0x00, 0x04,             // mov x4  #0       ;fd
    0xd2, 0x80, 0x00, 0x05,             // mov x5, #0       ;offset

    0xd4, 0x00, 0x00, 0x01,             // svc #0           ;system call
    0xd4, 0x20, 0x00, 0x00,             // brk #0           ;trap?
    0xd5, 0x03, 0x20, 0x1f              // nop
};
static const unsigned int linux_aarch64_call_mmap_size = sizeof(linux_aarch64_call_mmap);

static const unsigned int linux_aarch64_munmap_start_position = 4;
static const unsigned int linux_aarch64_munmap_addr_position =  8;
static const unsigned int linux_aarch64_munmap_size_position = linux_aarch64_munmap_addr_position + 16;
static const unsigned char linux_aarch64_call_munmap[] = {
    0xd5, 0x03, 0x20, 0x1f,             // nop              ;munmap(void *addr, int size)
    0xd2, 0x80, 0x1a, 0xe8,         // mov x8, #d215    ;pass sys call number

    0xd2, 0x80, 0x00, 0x00,         // mov x0, #0       ;&addr
    0xf2, 0xa0, 0x00, 0x00,         // mov x0, #0, lsl #16     ;<addr>
    0xf2, 0xc0, 0x00, 0x00,         // mov x0, #0, lsl #32     ;<addr>
    0xf2, 0xe0, 0x00, 0x00,         // mov x0, #0, lsl #48     ;<addr>

    0xd2, 0x80, 0x00, 0x01,         // mov x1, #0               ;size
    0xf2, 0xa0, 0x00, 0x01,         // mov x1, #0, lsl #16      ;<size>
    0xf2, 0xc0, 0x00, 0x01,         // mov x1, #0, lsl #32      ;<size>
    0xf2, 0xe0, 0x00, 0x01,         // mov x1, #0, lsl #48      ;<size>

    0xd4, 0x00, 0x00, 0x01,             // svc #0
    0xd4, 0x20, 0x00, 0x00,             // brk #0
    0xd5, 0x03, 0x20, 0x1f              // nop
};
static const unsigned int linux_aarch64_call_munmap_size = sizeof(linux_aarch64_call_munmap);

static const unsigned int freebsd_x86_64_mmap_flags_position = 21;
static const unsigned int freebsd_x86_64_mmap_size_position = 34;
static const unsigned int freebsd_x86_64_mmap_addr_position = 44;
static const unsigned int freebsd_x86_64_mmap_start_position = 4;
static const unsigned char freebsd_x86_64_call_mmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop sled
   0x49, 0xc7, 0xc1, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r9 (offset)
   0x49, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff,       //mov    $0xffffffffffffffff,%r8 (fd)
   0x49, 0xc7, 0xc2, 0x12, 0x10, 0x00, 0x00,       //mov    $0x1012,%r10 (flags)
   0x48, 0xc7, 0xc2, 0x07, 0x00, 0x00, 0x00,       //mov    $0x7,%rdx (perms)
   0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rsi (size)
   0x00, 0x00, 0x00,                               //
   0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rdi (addr)
   0x00, 0x00, 0x00,                               //
   0xb8, 0xdd, 0x01, 0x00, 0x00,                   //mov    $0x1dd,%eax (SYS_mmap)
   0x0f, 0x05,                                     //syscall
   0xcc,                                           //trap
   0x90                                            //nop
};
static const unsigned int freebsd_x86_64_call_mmap_size = sizeof(freebsd_x86_64_call_mmap);

static const unsigned int freebsd_x86_64_munmap_size_position = 6;
static const unsigned int freebsd_x86_64_munmap_addr_position = 16;
static const unsigned int freebsd_x86_64_munmap_start_position = 4;
static const unsigned char freebsd_x86_64_call_munmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop sled
   0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rsi
   0x00, 0x00, 0x00,                               //
   0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rdi
   0x00, 0x00, 0x00,                               //
   0xb8, 0x49, 0x00, 0x00, 0x00,                   //mov    $0x49,%eax
   0x0f, 0x05,                                     //syscall
   0xcc,                                           //trap
   0x90                                            //nop
};
static const unsigned int freebsd_x86_64_call_munmap_size = sizeof(freebsd_x86_64_call_munmap);

static const unsigned int freebsd_x86_mmap_flags_position = 9;
static const unsigned int freebsd_x86_mmap_size_position = 16;
static const unsigned int freebsd_x86_mmap_addr_position = 21;
static const unsigned int freebsd_x86_mmap_start_position = 4;
static const unsigned char freebsd_x86_call_mmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop sled
   0x6a, 0x00,                                     //push   $0x0 (offset)
   0x6a, 0xff,                                     //push   $0xffffffff (fd)
   0x68, 0x12, 0x10, 0x00, 0x00,                   //push   $0x1012 (flags)
   0x6a, 0x07,                                     //push   $0x7 (perms)
   0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (size)
   0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (addr)
   0xb8, 0xdd, 0x01, 0x00, 0x00,                   //mov    $0x1dd,%eax (SYS_mmap)
   0x50,                                           //push   %eax (required by calling convention)
   0xcd, 0x80,                                     //int    $0x80
   0x8d, 0x64, 0x24, 0x1c,                         //lea    0x1c(%esp),%esp
   0xcc,                                           //trap
   0x90                                            //nop
};
static const unsigned int freebsd_x86_call_mmap_size = sizeof(freebsd_x86_call_mmap);

static const unsigned int freebsd_x86_munmap_size_position = 5;
static const unsigned int freebsd_x86_munmap_addr_position = 10;
static const unsigned int freebsd_x86_munmap_start_position = 4;
static const unsigned char freebsd_x86_call_munmap[] = {
   0x90, 0x90, 0x90, 0x90,                         //nop sled
   0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (size)
   0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (addr)
   0xb8, 0x49, 0x00, 0x00, 0x00,                   //mov    $0x49,%eax (SYS_munmap)
   0x50,                                           //push   %eax (required by calling convention)
   0xcd, 0x80,                                     //int    $0x80
   0x8d, 0x64, 0x24, 0x0c,                         //lea    0xc(%esp),%esp
   0xcc,                                           //trap
   0x90                                            //nop
};
static const unsigned int freebsd_x86_call_munmap_size = sizeof(freebsd_x86_call_munmap);

mmap_alloc_process::mmap_alloc_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                       std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f)
{
}

mmap_alloc_process::mmap_alloc_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

mmap_alloc_process::~mmap_alloc_process()
{
}

// For compatibility
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

bool mmap_alloc_process::plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp)
{
    switch (getTargetArch())
    {
        case Arch_x86_64: {
            bool result = thr->getRegister(x86_64::rax, resp);
            assert(result);
	    if(!result) return false;
            break;
        }
        case Arch_x86: {
            bool result = thr->getRegister(x86::eax, resp);
            assert(result);
	    if(!result) return false;
            break;
        }
        case Arch_ppc32: {
            bool result = thr->getRegister(ppc32::r3, resp);
            assert(result);
	    if(!result) return false;
            break;
        }
        case Arch_ppc64: {
            bool result = thr->getRegister(ppc64::r3, resp);
            assert(result);
	        if(!result) return false;
            break;
        }
        case Arch_aarch64: {
            bool result = thr->getRegister(aarch64::x0, resp);
            assert(result);
	            if(!result) return false;
            break;
        }
        default:
            assert(0);
            break;
    }
    return true;
}

bool mmap_alloc_process::plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size,
                                                      void* &buffer, unsigned long &buffer_size,
                                                      unsigned long &start_offset)
{
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    if (use_addr)
        flags |= MAP_FIXED;
    else
        addr = 0x0;

    if (getTargetArch() == Arch_x86_64 || getTargetArch() == Arch_x86) {
        const void *buf_tmp = NULL;
        unsigned addr_size = 0;
        unsigned addr_pos = 0;
        unsigned flags_pos = 0;
        unsigned size_pos = 0;

        bool use_linux = (getOS() == Dyninst::Linux);
        bool use_bsd = (getOS() == Dyninst::FreeBSD);
        bool use_64 = (getTargetArch() == Arch_x86_64);

        if (use_linux && use_64) {
           buf_tmp = linux_x86_64_call_mmap;
           buffer_size = linux_x86_64_call_mmap_size;
           start_offset = linux_x86_64_mmap_start_position;
           addr_pos = linux_x86_64_mmap_addr_position;
           flags_pos = linux_x86_64_mmap_flags_position;
           size_pos = linux_x86_64_mmap_size_position;
           addr_size = 8;
        }
        else if (use_bsd && use_64) {
           buf_tmp = freebsd_x86_64_call_mmap;
           buffer_size = freebsd_x86_64_call_mmap_size;
           start_offset = freebsd_x86_64_mmap_start_position;
           addr_pos = freebsd_x86_64_mmap_addr_position;
           flags_pos = freebsd_x86_64_mmap_flags_position;
           size_pos = freebsd_x86_64_mmap_size_position;
           addr_size = 8;
        }
        else if (use_linux && !use_64) {
           buf_tmp = linux_x86_call_mmap;
           buffer_size = linux_x86_call_mmap_size;
           start_offset = linux_x86_mmap_start_position;
           addr_pos = linux_x86_mmap_addr_position;
           flags_pos = linux_x86_mmap_flags_position;
           size_pos = linux_x86_mmap_size_position;
           addr_size = 4;
        }
        else if (use_bsd && !use_64) {
           buf_tmp = freebsd_x86_call_mmap;
           buffer_size = freebsd_x86_call_mmap_size;
           start_offset = freebsd_x86_mmap_start_position;
           addr_pos = freebsd_x86_mmap_addr_position;
           flags_pos = freebsd_x86_mmap_flags_position;
           size_pos = freebsd_x86_mmap_size_position;
           addr_size = 4;
        }
        else {
           assert(0); //Fill in the entry in mmapalloc.h for this system
        }

        buffer = malloc(buffer_size);
        memcpy(buffer, buf_tmp, buffer_size);

        //Assuming endianess of debugger and debugee match.
        assert(size <= std::numeric_limits<uint32_t>::max() && "size more than 32 bits");
        write_memory_as(static_cast<char *>(buffer)+size_pos, static_cast<uint32_t>(size));
        write_memory_as(static_cast<char *>(buffer)+flags_pos, static_cast<uint32_t>(flags));
        if (addr_size == 8)  {
            write_memory_as(static_cast<char *>(buffer)+addr_pos, uint64_t{addr});
        }  else if (addr_size == 4)  {
            assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32 bits");
            write_memory_as(static_cast<char *>(buffer)+addr_pos, static_cast<uint32_t>(addr));
        }  else  {
            assert(0);
        }
   }
    else  if (getTargetArch() == Arch_ppc32) {
        unsigned int flags_hi_position;
        unsigned int flags_lo_position;
        unsigned int size_hi_position;
        unsigned int size_lo_position;
        unsigned int addr_hi_position;
        unsigned int addr_lo_position;
        const void *buf_tmp;

        bool use_linux = (getOS() == Linux);

        if (use_linux) {
           flags_hi_position = linux_ppc32_mmap_flags_hi_position;
           flags_lo_position = linux_ppc32_mmap_flags_lo_position;
           size_hi_position = linux_ppc32_mmap_size_hi_position;
           size_lo_position = linux_ppc32_mmap_size_lo_position;
           addr_hi_position = linux_ppc32_mmap_addr_hi_position;
           addr_lo_position = linux_ppc32_mmap_addr_lo_position;
           start_offset = linux_ppc32_mmap_start_position;
           buffer_size = linux_ppc32_call_mmap_size;
           buf_tmp = linux_ppc32_call_mmap;
        }
        else {
           assert(0); //Fill in the entry in mmapalloc.h for this system
        }

        buffer = malloc(buffer_size);
        memcpy(buffer, buf_tmp, buffer_size);

        // Assuming endianess of debugger and debuggee match
        write_memory_as(static_cast<char *>(buffer)+size_hi_position, static_cast<uint16_t>(size >> 16));
        write_memory_as(static_cast<char *>(buffer)+size_lo_position, static_cast<uint16_t>(size));
        write_memory_as(static_cast<char *>(buffer)+flags_hi_position, static_cast<uint16_t>(flags >> 16));
        write_memory_as(static_cast<char *>(buffer)+flags_lo_position, static_cast<uint16_t>(flags));
        write_memory_as(static_cast<char *>(buffer)+addr_hi_position, static_cast<uint16_t>(addr >> 16));
        write_memory_as(static_cast<char *>(buffer)+addr_lo_position, static_cast<uint16_t>(addr));
   }
   else if (getTargetArch() == Arch_ppc64) {
      unsigned int flags_highest_position;
      unsigned int flags_higher_position;
      unsigned int flags_hi_position;
      unsigned int flags_lo_position;
      unsigned int size_highest_position;
      unsigned int size_higher_position;
      unsigned int size_hi_position;
      unsigned int size_lo_position;
      unsigned int addr_highest_position;
      unsigned int addr_higher_position;
      unsigned int addr_hi_position;
      unsigned int addr_lo_position;
      const void *buf_tmp;

      bool use_linux = (getOS() == Linux);

      if (use_linux) {
         flags_highest_position = linux_ppc64_mmap_flags_highest_position;
         flags_higher_position = linux_ppc64_mmap_flags_higher_position;
         flags_hi_position = linux_ppc64_mmap_flags_hi_position;
         flags_lo_position = linux_ppc64_mmap_flags_lo_position;
         size_highest_position = linux_ppc64_mmap_size_highest_position;
         size_higher_position = linux_ppc64_mmap_size_higher_position;
         size_hi_position = linux_ppc64_mmap_size_hi_position;
         size_lo_position = linux_ppc64_mmap_size_lo_position;
         addr_highest_position = linux_ppc64_mmap_addr_highest_position;
         addr_higher_position = linux_ppc64_mmap_addr_higher_position;
         addr_hi_position = linux_ppc64_mmap_addr_hi_position;
         addr_lo_position = linux_ppc64_mmap_addr_lo_position;
         start_offset = linux_ppc64_mmap_start_position;
         buffer_size = linux_ppc64_call_mmap_size;
         buf_tmp = linux_ppc64_call_mmap;
      }
      else {
         assert(0); //Fill in the entry in mmapalloc.h for this system
      }
      buffer = malloc(buffer_size);
      memcpy(buffer, buf_tmp, buffer_size);

      uint32_t *pwords = (uint32_t *)buffer;

      // MJMTODO - Assumes endianess of debugger and debuggee match
      pwords[size_highest_position]  |= static_cast<uint32_t>((uint64_t{size} >> 48) & 0x0000ffff);
      pwords[size_higher_position]   |= static_cast<uint32_t>((uint64_t{size} >> 32) & 0x0000ffff);
      pwords[size_hi_position]       |= static_cast<uint32_t>((uint64_t{size} >> 16) & 0x0000ffff);
      pwords[size_lo_position]       |= static_cast<uint32_t>(uint64_t{size} & 0x0000ffff);
      pwords[flags_highest_position] |= static_cast<uint32_t>((static_cast<uint64_t>(flags) >> 48) & 0x0000ffff);
      pwords[flags_higher_position]  |= static_cast<uint32_t>((static_cast<uint64_t>(flags) >> 32) & 0x0000ffff);
      pwords[flags_hi_position]      |= static_cast<uint32_t>((static_cast<uint64_t>(flags) >> 16) & 0x0000ffff);
      pwords[flags_lo_position]      |= static_cast<uint32_t>(static_cast<uint64_t>(flags) & 0x0000ffff);
      pwords[addr_highest_position]  |= static_cast<uint32_t>((uint64_t{addr} >> 48) & 0x0000ffff);
      pwords[addr_higher_position]   |= static_cast<uint32_t>((uint64_t{addr} >> 32) & 0x0000ffff);
      pwords[addr_hi_position]       |= static_cast<uint32_t>((uint64_t{addr} >> 16) & 0x0000ffff);
      pwords[addr_lo_position]       |= static_cast<uint32_t>(uint64_t{addr} & 0x0000ffff);
    } else if( getTargetArch() == Arch_aarch64 ){
        const void *buf_tmp;
        unsigned int addr_pos, size_pos, flags_pos;

        bool use_linux = ( getOS() == Linux );

        if (use_linux) {
           start_offset             = linux_aarch64_mmap_start_position;
           buffer_size              = linux_aarch64_call_mmap_size;
           buf_tmp                  = linux_aarch64_call_mmap;
           addr_pos                 = linux_aarch64_mmap_addr_position;
           size_pos                 = linux_aarch64_mmap_size_position;
           flags_pos                = linux_aarch64_mmap_flags_position;
        }
        else {
           assert(0); //Fill in the entry in mmapalloc.h for this system
        }
        buffer = malloc(buffer_size);
        memcpy(buffer, buf_tmp, buffer_size);

        // To avoid the matter of endianness, I decided to operate on byte.
        pthrd_printf("ARM-info: create alloc snippet...\n");
#define BYTE_ASSGN(POS, VAL) \
            (*(static_cast<char *>(buffer) + POS + 1)) |= ((VAL>>11)&0x1f);\
            (*(static_cast<char *>(buffer) + POS + 2)) |= ((VAL>> 3)&0xff);\
            (*(static_cast<char *>(buffer) + POS + 3)) |= ((VAL<< 5)&0xf0);

        BYTE_ASSGN(addr_pos,    static_cast<uint16_t>(addr)     )
        BYTE_ASSGN(addr_pos+4,  static_cast<uint16_t>(addr>>16) )
        BYTE_ASSGN(addr_pos+8,  static_cast<uint16_t>(addr>>32) )
        BYTE_ASSGN(addr_pos+12, static_cast<uint16_t>(addr>>48) )

        BYTE_ASSGN(size_pos,    static_cast<uint16_t>(size) )
        BYTE_ASSGN(size_pos+4,  static_cast<uint16_t>(size>>16) )
        BYTE_ASSGN(size_pos+8,  static_cast<uint16_t>(size>>32) )
        BYTE_ASSGN(size_pos+12, static_cast<uint16_t>(size>>48) )

        BYTE_ASSGN(flags_pos,    static_cast<uint16_t>(flags) )
        BYTE_ASSGN(flags_pos+4,  static_cast<uint16_t>(flags>>16) )
        //BYTE_ASSGN(flags_pos+8,  static_cast<uint16_t>(flags>>32) )
        //BYTE_ASSGN(flags_pos+12, static_cast<uint16_t>(flags>>48) )

        //according to experiments, aarch64 is little-endian
        //the byte order with a word should be re-arranged
#define SWAP4BYTE(POS) \
            static_cast<char *>(buffer)[POS+3]^= static_cast<char*>(buffer)[POS]; \
            static_cast<char *>(buffer)[POS]  ^= static_cast<char*>(buffer)[POS+3]; \
            static_cast<char *>(buffer)[POS+3]^= static_cast<char*>(buffer)[POS]; \
            static_cast<char *>(buffer)[POS+2]^= static_cast<char*>(buffer)[POS+1]; \
            static_cast<char *>(buffer)[POS+1]^= static_cast<char*>(buffer)[POS+2]; \
            static_cast<char *>(buffer)[POS+2]^= static_cast<char*>(buffer)[POS+1];

        for(unsigned int i=0; i < buffer_size ; i+=4){
            SWAP4BYTE(i)
        }

//debug
#if 1
        pthrd_printf("ARM-info: dump alloc snippet...\n");
        pthrd_printf("addr %lu, 0x%lx\n", addr, addr);
        pthrd_printf("size %lu, 0x%lx\n", size, size);
        pthrd_printf("flags 0x%x:\n", (unsigned int)flags);

        for(unsigned int i = 0; i< buffer_size ; i+=4){
            pthrd_printf("0x%8x\n", Dyninst::read_memory_as<uint32_t>(static_cast<char *>(buffer)+i)) ;
        }

#endif

    }else{
        assert(0);
    }

    return true;
}

bool mmap_alloc_process::plat_createDeallocationSnippet(Dyninst::Address addr,
                                                        unsigned long size, void* &buffer,
                                                        unsigned long &buffer_size,
                                                        unsigned long &start_offset)
{
   if (getTargetArch() == Arch_x86_64 || getTargetArch() == Arch_x86) {
       const void *buf_tmp = NULL;
       unsigned addr_size = 0;
       unsigned addr_pos = 0;
       unsigned size_pos = 0;

       bool use_linux = (getOS() == Dyninst::Linux);
       bool use_bsd = (getOS() == Dyninst::FreeBSD);
       bool use_64 = (getTargetArch() == Arch_x86_64);

       if (use_linux && use_64) {
          buf_tmp = linux_x86_64_call_munmap;
          buffer_size = linux_x86_64_call_munmap_size;
          start_offset = linux_x86_64_munmap_start_position;
          addr_pos = linux_x86_64_munmap_addr_position;
          size_pos = linux_x86_64_munmap_size_position;
          addr_size = 8;
       }
       else if (use_bsd && use_64) {
          buf_tmp = freebsd_x86_64_call_munmap;
          buffer_size = freebsd_x86_64_call_munmap_size;
          start_offset = freebsd_x86_64_munmap_start_position;
          addr_pos = freebsd_x86_64_munmap_addr_position;
          size_pos = freebsd_x86_64_munmap_size_position;
          addr_size = 8;
       }
       else if (use_linux && !use_64) {
          buf_tmp = linux_x86_call_munmap;
          buffer_size = linux_x86_call_munmap_size;
          start_offset = linux_x86_munmap_start_position;
          addr_pos = linux_x86_munmap_addr_position;
          size_pos = linux_x86_munmap_size_position;
          addr_size = 4;
       }
       else if (use_bsd && !use_64) {
          buf_tmp = freebsd_x86_call_munmap;
          buffer_size = freebsd_x86_call_munmap_size;
          start_offset = freebsd_x86_munmap_start_position;
          addr_pos = freebsd_x86_munmap_addr_position;
          size_pos = freebsd_x86_munmap_size_position;
          addr_size = 4;
       }
       else {
          assert(0);
       }

       buffer = malloc(buffer_size);
       memcpy(buffer, buf_tmp, buffer_size);

       //Assuming endianess of debugger and debugee match.
        assert(size <= std::numeric_limits<uint32_t>::max() && "size more than 32 bits");
       write_memory_as(static_cast<char *>(buffer)+size_pos, static_cast<uint32_t>(size));
       if (addr_size == 8)  {
          write_memory_as(static_cast<char *>(buffer)+addr_pos, uint64_t{addr});
       }  else if (addr_size == 4)  {
          assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32 bits");
          write_memory_as(static_cast<char *>(buffer)+addr_pos, static_cast<uint32_t>(addr));
       }  else  {
          assert(0);
       }
   }
   else if (getTargetArch() == Arch_ppc32) {
      unsigned int size_hi_position;
      unsigned int size_lo_position;
      unsigned int addr_hi_position;
      unsigned int addr_lo_position;
      const void *buf_tmp = NULL;

      bool use_linux = (getOS() == Linux);
      if (use_linux) {
         buf_tmp = linux_ppc32_call_munmap;
         size_hi_position = linux_ppc32_munmap_size_hi_position;
         size_lo_position = linux_ppc32_munmap_size_lo_position;
         addr_hi_position = linux_ppc32_munmap_addr_hi_position;
         addr_lo_position = linux_ppc32_munmap_addr_lo_position;
         buffer_size = linux_ppc32_call_munmap_size;
         start_offset = linux_ppc32_munmap_start_position;
      }
      else {
         assert(0);
      }

       buffer = malloc(buffer_size);
       memcpy(buffer, buf_tmp, buffer_size);

       // Assuming endianess of debugger and debuggee match
       write_memory_as(static_cast<char *>(buffer)+size_hi_position, static_cast<uint16_t>(size >> 16));
       write_memory_as(static_cast<char *>(buffer)+size_lo_position, static_cast<uint16_t>(size));
       write_memory_as(static_cast<char *>(buffer)+addr_hi_position, static_cast<uint16_t>(addr >> 16));
       write_memory_as(static_cast<char *>(buffer)+addr_lo_position, static_cast<uint16_t>(addr));
   }
   else if( getTargetArch() == Arch_ppc64 ) {
      unsigned int size_highest_position;
      unsigned int size_higher_position;
      unsigned int size_hi_position;
      unsigned int size_lo_position;
      unsigned int addr_highest_position;
      unsigned int addr_higher_position;
      unsigned int addr_hi_position;
      unsigned int addr_lo_position;
      const void *buf_tmp = NULL;

      bool use_linux = (getOS() == Linux);
      if (use_linux) {
         buf_tmp = linux_ppc64_call_munmap;
         size_highest_position = linux_ppc64_munmap_size_highest_position;
         size_higher_position = linux_ppc64_munmap_size_higher_position;
         size_hi_position = linux_ppc64_munmap_size_hi_position;
         size_lo_position = linux_ppc64_munmap_size_lo_position;
         addr_highest_position = linux_ppc64_munmap_addr_highest_position;
         addr_higher_position = linux_ppc64_munmap_addr_higher_position;
         addr_hi_position = linux_ppc64_munmap_addr_hi_position;
         addr_lo_position = linux_ppc64_munmap_addr_lo_position;
         buffer_size = linux_ppc64_call_munmap_size;
         start_offset = linux_ppc64_munmap_start_position;
      }
      else {
         assert(0);
      }

      buffer = malloc(buffer_size);
      memcpy(buffer, buf_tmp, buffer_size);

      uint32_t *pwords = static_cast<uint32_t *>(buffer);

      // MJMTODO - Assumes endianess of debugger and debuggee match
      pwords[size_highest_position] |= static_cast<uint32_t>((uint64_t{size} >> 48) & 0x0000ffff);
      pwords[size_higher_position]  |= static_cast<uint32_t>((uint64_t{size} >> 32) & 0x0000ffff);
      pwords[size_hi_position]      |= static_cast<uint32_t>((uint64_t{size} >> 16) & 0x0000ffff);
      pwords[size_lo_position]      |= static_cast<uint32_t>(uint64_t{size} & 0x0000ffff);
      pwords[addr_highest_position] |= static_cast<uint32_t>((uint64_t{addr} >> 48) & 0x0000ffff);
      pwords[addr_higher_position]  |= static_cast<uint32_t>((uint64_t{addr} >> 32) & 0x0000ffff);
      pwords[addr_hi_position]      |= static_cast<uint32_t>((uint64_t{addr} >> 16) & 0x0000ffff);
      pwords[addr_lo_position]      |= static_cast<uint32_t>(uint64_t{addr} & 0x0000ffff);
   }
   else if( getTargetArch() == Arch_aarch64 ) {
        const void *buf_tmp = NULL;
        unsigned int addr_size;
        unsigned int addr_pos;
        unsigned int size_pos;

        bool use_linux = (getOS() == Linux );
        if (use_linux) {
            buf_tmp             = linux_aarch64_call_munmap;
            buffer_size         = linux_aarch64_call_munmap_size;
            start_offset        = linux_aarch64_munmap_start_position;
            addr_pos            = linux_aarch64_munmap_addr_position;
            size_pos            = linux_aarch64_munmap_size_position;
            addr_size           = 8;
        }
        else {
           assert(0);
        }

        buffer = malloc(buffer_size);
        memcpy(buffer, buf_tmp, buffer_size);

        pthrd_printf("ARM-info: create de-alloc snippet...\n");

        BYTE_ASSGN(addr_pos,    static_cast<uint16_t>(addr)     )
        BYTE_ASSGN(addr_pos+4,  static_cast<uint16_t>(addr>>16) )
        BYTE_ASSGN(addr_pos+8,  static_cast<uint16_t>(addr>>32) )
        BYTE_ASSGN(addr_pos+12, static_cast<uint16_t>(addr>>48) )

        BYTE_ASSGN(size_pos,    static_cast<uint16_t>(size) )
        BYTE_ASSGN(size_pos+4,  static_cast<uint16_t>(size>>16) )
        BYTE_ASSGN(size_pos+8,  static_cast<uint16_t>(size>>32) )
        BYTE_ASSGN(size_pos+12, static_cast<uint16_t>(size>>48) )

        //swap 4bytes
        for(unsigned int i=0; i<buffer_size; i+=4){
            SWAP4BYTE(i)
        }

        // Assuming endianess of debugger and debuggee match
        assert(addr_size == 8);
   }
   else {
      assert(0);
   }

   return true;
}
