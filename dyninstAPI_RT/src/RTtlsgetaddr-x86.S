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

/*
 * This function is used to help support TLS when rewriting static binaries.
 * It is originally provided by the dynamic linker (which is unavailable to
 * static binaries).  One TLS model is to call the function with a module
 * index in order to retrieve a TLS variable (see general dynamic). This model
 * is unnecessarily general for static binaries but does exist in some non-PIC
 * relocatable files. The GNU ld approach is to perform code transformations to
 * relax the TLS model. In order to keep the rewriter for static binaries
 * simple, the approach of emulating a call to ___tls_get_addr was used instead
 * of implementing these link-time code transformations.
 */

#if defined(DYNINST_RT_STATIC_LIB)
.text 
.globl ___tls_get_addr

___tls_get_addr:
    mov (%eax), %eax /* Gets the TLS offset of the variable */
    add %gs:0, %eax /* Gets the absolute address of the variable */
    ret

#endif

.section .note.GNU-stack,"",@progbits
