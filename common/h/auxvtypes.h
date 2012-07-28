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
#ifndef AUXV_TYPES_H
#define AUXV_TYPES_H

// These are not defined on some systems, so this header should help.  This 
// also defines a method to convert auxv types to strings, for more 
// readable debug output.

// --- Standard auxv values from elf.h ---
#ifndef AT_NULL
#define AT_NULL 0
#endif  // AT_NULL

#ifndef AT_IGNORE
#define AT_IGNORE 1
#endif  // AT_IGNORE

#ifndef AT_EXECFD
#define AT_EXECFD 2
#endif  // AT_EXECFD

#ifndef AT_PHDR
#define AT_PHDR 3
#endif  // AT_PHDR

#ifndef AT_PHENT
#define AT_PHENT 4
#endif  // AT_PHENT

#ifndef AT_PHNUM
#define AT_PHNUM 5
#endif  // AT_PHNUM

#ifndef AT_PAGESZ
#define AT_PAGESZ 6
#endif  // AT_PAGESZ

#ifndef AT_BASE
#define AT_BASE 7
#endif  // AT_BASE

#ifndef AT_FLAGS
#define AT_FLAGS 8
#endif  // AT_FLAGS

#ifndef AT_ENTRY
#define AT_ENTRY 9
#endif  // AT_ENTRY

#ifndef AT_NOTELF
#define AT_NOTELF 10
#endif  // AT_NOTELF

#ifndef AT_UID
#define AT_UID	11
#endif  // AT_UID

#ifndef AT_EUID
#define AT_EUID 12
#endif  // AT_EUID

#ifndef AT_GID
#define AT_GID	13
#endif  // AT_GID

#ifndef AT_EGID
#define AT_EGID 14
#endif  // AT_EGID

#ifndef AT_CLKTCK
#define AT_CLKTCK 17
#endif  // AT_CLKTCK

#ifndef AT_PLATFORM
#define AT_PLATFORM 15
#endif  // AT_PLATFORM

#ifndef AT_HWCAP
#define AT_HWCAP 16	
#endif  // AT_HWCAP

#ifndef AT_FPUCW
#define AT_FPUCW 18
#endif  // AT_FPUCW

#ifndef AT_DCACHEBSIZE
#define AT_DCACHEBSIZE 19
#endif  // AT_DCACHEBSIZE

#ifndef AT_ICACHEBSIZE
#define AT_ICACHEBSIZE 20
#endif  // AT_ICACHEBSIZE

#ifndef AT_UCACHEBSIZE
#define AT_UCACHEBSIZE 21
#endif  // AT_UCACHEBSIZE

#ifndef AT_IGNOREPPC
#define AT_IGNOREPPC 22
#endif  // AT_IGNOREPPC

#ifndef AT_SECURE
#define	AT_SECURE 23
#endif  // AT_SECURE

#ifndef AT_SYSINFO
#define AT_SYSINFO 32
#endif  // AT_SYSINFO

#ifndef AT_SYSINFO
#define AT_SYSINFO_EHDR	33
#endif  // AT_SYSINFO

#ifndef AT_L1I
#define AT_L1I_CACHESHAPE 34
#endif  // AT_L1I

#ifndef AT_L1D
#define AT_L1D_CACHESHAPE 35
#endif  // AT_L1D

#ifndef AT_L2
#define AT_L2_CACHESHAPE 36
#endif  // AT_L2

#ifndef AT_L3
#define AT_L3_CACHESHAPE 37
#endif  // AT_L3



/// Converts an auxv type into a const string.  Useful for debugging.
const char *auxv_type_to_string(int auxv_type);

#endif // AUXV_TYPES_H


