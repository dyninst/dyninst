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
#if !defined(SNIPPETS_H_)
#define SNIPPETS_H_

/* definitions for iRPC snippets */

extern const unsigned int x86_64_mmap_flags_position;
extern const unsigned int x86_64_mmap_size_position;
extern const unsigned int x86_64_mmap_addr_position;
extern const unsigned int x86_64_mmap_start_position;
extern const unsigned char x86_64_call_mmap[];
extern const unsigned int x86_64_call_mmap_size;

extern const unsigned int x86_64_munmap_size_position;
extern const unsigned int x86_64_munmap_addr_position;
extern const unsigned int x86_64_munmap_start_position;
extern const unsigned char x86_64_call_munmap[];
extern const unsigned int x86_64_call_munmap_size;

extern const unsigned int x86_mmap_flags_position;
extern const unsigned int x86_mmap_size_position;
extern const unsigned int x86_mmap_addr_position;
extern const unsigned int x86_mmap_start_position;
extern const unsigned char x86_call_mmap[];
extern const unsigned int x86_call_mmap_size;

extern const unsigned int x86_munmap_size_position;
extern const unsigned int x86_munmap_addr_position;
extern const unsigned int x86_munmap_start_position;
extern const unsigned char x86_call_munmap[];
extern const unsigned int x86_call_munmap_size;

extern const unsigned int ppc32_mmap_flags_hi_position;
extern const unsigned int ppc32_mmap_flags_lo_position;
extern const unsigned int ppc32_mmap_size_hi_position;
extern const unsigned int ppc32_mmap_size_lo_position;
extern const unsigned int ppc32_mmap_addr_hi_position;
extern const unsigned int ppc32_mmap_addr_lo_position;
extern const unsigned int ppc32_mmap_start_position;
extern const unsigned char ppc32_call_mmap[];
extern const unsigned int ppc32_call_mmap_size;

extern const unsigned int ppc32_munmap_size_hi_position;
extern const unsigned int ppc32_munmap_size_lo_position;
extern const unsigned int ppc32_munmap_addr_hi_position;
extern const unsigned int ppc32_munmap_addr_lo_position;
extern const unsigned int ppc32_munmap_start_position;
extern const unsigned char ppc32_call_munmap[];
extern const unsigned int ppc32_call_munmap_size;

extern const unsigned int ppc64_mmap_flags_highest_position;
extern const unsigned int ppc64_mmap_flags_higher_position;
extern const unsigned int ppc64_mmap_flags_hi_position;
extern const unsigned int ppc64_mmap_flags_lo_position;
extern const unsigned int ppc64_mmap_size_highest_position;
extern const unsigned int ppc64_mmap_size_higher_position;
extern const unsigned int ppc64_mmap_size_hi_position;
extern const unsigned int ppc64_mmap_size_lo_position;
extern const unsigned int ppc64_mmap_addr_highest_position;
extern const unsigned int ppc64_mmap_addr_higher_position;
extern const unsigned int ppc64_mmap_addr_hi_position;
extern const unsigned int ppc64_mmap_addr_lo_position;
extern const unsigned int ppc64_mmap_start_position;
extern const unsigned char ppc64_call_mmap[];
extern const unsigned int ppc64_call_mmap_size;

extern const unsigned int ppc64_munmap_size_highest_position;
extern const unsigned int ppc64_munmap_size_higher_position;
extern const unsigned int ppc64_munmap_size_hi_position;
extern const unsigned int ppc64_munmap_size_lo_position;
extern const unsigned int ppc64_munmap_addr_highest_position;
extern const unsigned int ppc64_munmap_addr_higher_position;
extern const unsigned int ppc64_munmap_addr_hi_position;
extern const unsigned int ppc64_munmap_addr_lo_position;
extern const unsigned int ppc64_munmap_start_position;
extern const unsigned char ppc64_call_munmap[];
extern const unsigned int ppc64_call_munmap_size;

extern const unsigned int aarch64_mmap_flags_highest_position;
extern const unsigned int aarch64_mmap_flags_higher_position;
extern const unsigned int aarch64_mmap_flags_hi_position;
extern const unsigned int aarch64_mmap_flags_lo_position;
extern const unsigned int aarch64_mmap_size_highest_position;
extern const unsigned int aarch64_mmap_size_higher_position;
extern const unsigned int aarch64_mmap_size_hi_position;
extern const unsigned int aarch64_mmap_size_lo_position;
extern const unsigned int aarch64_mmap_addr_highest_position;
extern const unsigned int aarch64_mmap_addr_higher_position;
extern const unsigned int aarch64_mmap_addr_hi_position;
extern const unsigned int aarch64_mmap_addr_lo_position;
extern const unsigned int aarch64_mmap_start_position;
extern const unsigned char aarch64_call_mmap[];
extern const unsigned int aarch64_call_mmap_size;

extern const unsigned int aarch64_munmap_size_highest_position;
extern const unsigned int aarch64_munmap_size_higher_position;
extern const unsigned int aarch64_munmap_size_hi_position;
extern const unsigned int aarch64_munmap_size_lo_position;
extern const unsigned int aarch64_munmap_addr_highest_position;
extern const unsigned int aarch64_munmap_addr_higher_position;
extern const unsigned int aarch64_munmap_addr_hi_position;
extern const unsigned int aarch64_munmap_addr_lo_position;
extern const unsigned int aarch64_munmap_start_position;
extern const unsigned char aarch64_call_munmap[];
extern const unsigned int aarch64_call_munmap_size;

#endif
