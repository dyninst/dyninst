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
   DYNINSTsigill:
	
   DYNINSTsigill executes an illegal instruction, causing SIGILL to be
   sent to the calling process.  It is only needed for
   detach-on-the-fly, where detached mutatees stop to wait for the
   mutator or paradynd to reattach by sending themselves SIGILL.

   A detached mutatee that stops itself with SIGSTOP will not be
   noticed by the mutator or paradynd.  It must send SIGILL to be
   noticed.
    
   There are two ways to send SIGILL to yourself: call kill() or
   execute an illegal instruction.

   We chose to use illegal instructions. The SIGILL handler has a
   simpler implementation if SIGILL is caused by an illegal
   instruction.  We do not use kill() to send SIGILL.

   The test suite calls this function through the dynamic loader to
   avoid duplication of code in the test suite and additional makefile
   complexity.
    
   3000 bytes of source code for 3 bytes of object code.
*/

    .text
    .globl DYNINSTsigill

DYNINSTsigill:
    ud2
    ret

.section .note.GNU-stack,"",@progbits
