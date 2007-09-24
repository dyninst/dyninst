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

/* Test application (Mutatee) */

/* $Id: test1_31F_mutatee.c,v 1.1 2007/09/24 16:37:59 cooksey Exp $ */

#include <stdlib.h>
#include "mutatee_util.h"

// **********************************************************************
// The following structure (struct block_) is made to correspond with the
// Fortran common block (globals) defined in test1_common.h.  Be sure all
// changes to this structure are reflected in the other.
// **********************************************************************
struct block_ {
  int globalVariable31_1, globalVariable31_2_, globalVariable31_3_, globalVariable31_4_;
  int passedTest_;
};

#if !defined(XLF)
#define test1_31_func1 test1_31_func1_
#define test1_31f_init_globals test1_31f_init_globals_
#define test1_31f_globals test1_31f_globals_
#endif

extern struct block_ test1_31f_globals;

extern void test1_31f_init_globals ();
extern void test1_31_func1 ();

int mutateeFortran = 1;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

int test1_31F_mutatee() {
  test1_31f_globals.passedTest_ = FALSE;

  if (setupFortranOutput()) {
    logerror("Error redirecting Fortran component output to log file\n");
  }

  test1_31f_init_globals();

  test1_31_func1();

  if (cleanupFortranOutput()) {
    logerror("Error restoring output to stdout\n");
  }

  /* Combine fortran passedTest with C passedTest */
  if (test1_31f_globals.passedTest_) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}
