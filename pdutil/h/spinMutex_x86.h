/* spinMutex_x86.h */
/* C and C++ interface */

/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef _SPINMUTEX_X86_H_
#define _SPINMUTEX_X86_H_

//inline int spinMutex_tryToGrab(volatile unsigned char *theMutex) {
//   /* Returns true iff the mutex lock was grabbed.  Doesn't wait.
//      Very fast. */
////   register unsigned char oldValue = (unsigned char)1;
//
////   asm volatile ("xchgb ( %0 ), %1" :
////		 "=r" (theMutex)
////		 : "r" (oldValue));
//
////   asm volatile ("xchgb ( %0 ), %1" :
////		 "=r" (theMutex),
////		 "=r" (oldValue));
//
////   return (!oldValue);
//
///* WARNING: Not yet implemented properly, since I can't get gdb to output
//   correct assembly language for the xchgb instruction */
////   if (*theMutex)
////      return 0; /* failure */
////
////   *theMutex=1;
////   return 1; /* success */
//}
//
////int spinMutex_tryToGrab(volatile unsigned char *theMutex) {
////   /* assumes: input arg (address to a byte) is in eax and
////               result as usual goes to eax.  Assumes no inlining */
////   asm volatile ("movb bl, 1");
////   asm volatile ("xchgb (eax), bl");
////
////   /* Now, if the contents of reg bl are 0 then success else failure */
////   xxx;
////}

inline int spinMutex_tryToGrab(volatile unsigned char *theMutex) {
   /* Not yet implemented */
   return 1;
}

inline void spinMutex_release(volatile unsigned char *theMutex) {
   *theMutex = (unsigned char)0;
}

#endif
