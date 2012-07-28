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

#include <pthread.h>
#include "RTthread.h"

unsigned DYNINSTthreadIndexFAST()
{
   return 0;
}

int tc_lock_lock(tc_lock_t *lock)
{
   /* If we've already got the lock, fail. */
   dyntid_t myid = dyn_pthread_self();
   if( myid == lock->tid ) {
      return DYNINST_DEAD_LOCK;
   }
   
   /* Loop until we acquire the lock. */
   
   /* Assumes tc->mutex is a 4-byte value. */
   __asm__ __volatile__ (
         "mov ar.ccv = r0\n"
         "mov r22 = 1;;\n"
         "1: cmpxchg4.acq r21 = [%0], r22, ar.ccv;;\n"
         "cmp.eq p6, p7 = r21, r0;;\n"
         /* if we read a zero from memory, we've acquired the lock, 
            so p7 -> we did not acquire the lock */
         "(p7) br.cond.spnt.few 1b;;\n"
         : /* no output */
         : "r" (& lock->mutex)
         : "p6", "p7", "r21", "r22", "ar.ccv", "memory"
         );      
   /* Lay claim to it. */
   lock->tid = myid;
   return 0;
} /* end tc_lock_lock() */

