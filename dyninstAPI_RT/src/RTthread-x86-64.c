/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "dyninstAPI_RT/src/RTthread.h"

long atomic_set(volatile int *val)
{
   long result = 0;
#if defined(MUTATEE_32)
   __asm("movl $1,%%eax\n"
         "movl %1,%%ecx\n"
         "lock\n"
         "xchgl %%eax, (%%ecx)\n"
         "movl %%eax, %0\n"
         : "=r" (result)
         : "r" (val)
         : "%eax",
           "%ecx");
#else
   __asm("mov $1,%%rax\n"
         "mov %1,%%rcx\n"
         "lock\n"
         "xchg %%rax, (%%rcx)\n"
         "mov %%rax, %0\n"
         : "=r" (result)
         : "r" (val)
         : "%rax",
           "%rcx");
#endif
   return !result;
}
/*
#if 1
   __asm(
         "movl $0,%%eax\n"
         "movl $1,%%ebx\n"
         "movl %1,%%ecx\n"
         "lock\n"
         "cmpxchgl %%ebx,(%%ecx)\n"
         "setz %%al\n"
         "movl %%eax,%0\n"
         : "=r" (result)
         : "r" (val)
         : "%eax", "%ebx", "%ecx");
#else
      __asm(
            "mov $0,%%rax\n"
            "mov $1,%%rbx\n"
            "mov %1,%%rcx\n"
            "lock\n"
            "cmpxchg %%rbx,(%%rcx)\n"
            "setz %%al\n"
            "mov %%rax,%0\n"
            : "=r" (result)
            : "r" (val)
            : "%rax", "%rbx", "%rcx");
#endif
      return result;
*/

int tc_lock_lock(tc_lock_t *tc)
{
   dyntid_t me;

   me = dyn_pthread_self();
   if (me == tc->tid)
      return DYNINST_DEAD_LOCK;

   while (1) {
      int wasNotLocked = atomic_set(&tc->mutex);
      if( wasNotLocked ) {
          tc->tid = me;
          break;
      }
   }
   return 0;
}

unsigned DYNINSTthreadIndexFAST() {
   return 0;
}
