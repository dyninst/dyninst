/* spinMutex_sparc.h */
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

#ifndef _SPINMUTEX_SPARC_H_
#define _SPINMUTEX_SPARC_H_

inline int spinMutex_tryToGrab(volatile unsigned char *theMutex) {
   // returns true iff the mutex lock was grabbed.  Doesn't wait.  Very fast.
   // Assumes SPARC architecture.

//   register unsigned prevValue;
//   asm volatile ("ldstub [ %1 ], %0" : "=r" (prevValue) : "r" (theMutex));
//   return (prevValue == 0);
return 1;
}

inline void spinMutex_release(volatile unsigned char *theMutex) {
   // Assumes SPARC architecture.

// Don't ask me why, but g++ 2.7.2 for sunos barfs while compiling the "stbar"
// instr, saying it's an unknown instruction.
//#ifndef sparc_sun_sunos4_1_3
//   asm volatile ("stbar");
//#endif

//   *theMutex = (unsigned char)0;
}

#endif
