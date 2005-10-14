/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
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
   tid_t myid = dyn_pthread_self();
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

