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

// $Id: timing.C,v 1.15 2000/07/28 17:22:35 pcroth Exp $

#include "common/h/Timer.h"

#if defined(rs6000_ibm_aix4_1)
#define NOPS_4  asm("oril 0,0,0"); asm("oril 0,0,0"); asm("oril 0,0,0"); asm("oril 0,0,0")
#elif defined(i386_unknown_nt4_0)
#define NOPS_4 { __asm nop __asm nop __asm nop __asm nop }
#elif defined(mips_sgi_irix6_4)
#  ifndef USES_NATIVE_CC
#define NOPS_4  __asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop")
#  else
#define NOPS_4  ; ; ; 
#  endif
#else
#define NOPS_4  asm("nop"); asm("nop"); asm("nop"); asm("nop")
#endif

#define NOPS_16 NOPS_4; NOPS_4; NOPS_4; NOPS_4

double timing_loop(const unsigned TRIES, const unsigned LOOP_LIMIT) {
  const double MILLION = 1.0e6;
  unsigned       i, j;
  timer stopwatch;
  double speed=0, max_speed=0;

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    if (stopwatch.usecs() > 0)
        speed   = ((double)(256*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    if (stopwatch.usecs() > 0)
        speed   = ((double)(512*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }

  for (j=0; j<TRIES; j++) {
    stopwatch.start();
    for (i = 0; i < LOOP_LIMIT; i++) {
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
      NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    stopwatch.stop();
    if (stopwatch.usecs() > 0)
        speed   = ((double)(1024*LOOP_LIMIT)/stopwatch.usecs())/MILLION;
    stopwatch.clear();
    if (speed > max_speed)
      max_speed = speed;
  }

#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0) \
 || defined(i386_unknown_linux2_0)
  // the speed of the pentium is being overestimated by a factor of 2
  max_speed /= 2;
#elif defined(mips_sgi_irix6_4)
  max_speed /= 4;
#endif

  return max_speed;
}

int cyclesPerSecond_default(unsigned &cps)
{
  double raw = timing_loop(1, 100000) * 1000000.0;
  cps = (unsigned)raw;
  return 0; // never fails
}

