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

/* Test application (Mutatee) */

/* $Id: test1_9F_mutatee.c,v 1.1 2008/10/30 19:20:06 legendre Exp $ */

#include <stdlib.h>
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

// **********************************************************************
// The following structure (struct block_) is made to correspond with the
// Fortran common block (globals) defined in test1_common.h.  Be sure all
// changes to this structure are reflected in the other.
// **********************************************************************
struct block_ {
  int passedTest_;
};

#if !defined(XLF)
#define test1_9_func1 test1_9_func1_
#define test1_9f_globals test1_9f_globals_
#endif

extern struct block_ test1_9f_globals;

/* FIXME This is the wrong prototype for this function */
extern void test1_9_func1 (int *, int *, int *, int *, int *,
			   int *, int *, int *, int *, int *);

int mutateeFortran = 1;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

int test1_9F_mutatee() {
  int retval;
  int *pp1, *pp2, *pp3, *pp4, *pp5, *pp6, *pp7, *pp8, *pp9, *pp10;

  test1_9f_globals.passedTest_ = FALSE;

  pp1 = (int*) malloc (sizeof (int));
  pp2 = (int*) malloc (sizeof (int));
  pp3 = (int*) malloc (sizeof (int));
  pp4 = (int*) malloc (sizeof (int));
  pp5 = (int*) malloc (sizeof (int));
  pp6 = (int*) malloc (sizeof (int));
  pp7 = (int*) malloc (sizeof (int));
  pp8 = (int*) malloc (sizeof (int));
  pp9 = (int*) malloc (sizeof (int));
  pp10 = (int*) malloc (sizeof (int));

  if (pp1 && pp2 && pp3 && pp4 && pp5 && pp6 && pp7 && pp8 && pp9 && pp10) {
    /* Got the memory we asked for; continue the test */
    *pp1 = 1; *pp2 = 2; *pp3 = 3; *pp4 = 4; *pp5 = 5;
    *pp6 = 6; *pp7 = 7; *pp8 = 8; *pp9 = 9; *pp10 = 10;

    if (setupFortranOutput()) {
      logerror("Error redirecting Fortran component output to log file\n");
    }
    
    test1_9_func1(pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10);

    if (cleanupFortranOutput()) {
      logerror("Error restoring output to stdout\n");
    }

    /* Combine fortran passedTest with C passedTest */
    if (test1_9f_globals.passedTest_) {
      test_passes(testname);
      retval = 0;
    } else {
      retval = -1;
    }
  } else {
    logerror("**Failed** test1_9F: error allocating memory\n");
    retval = -1;
  }

  /* Clean up the allocated memory */
  if (pp1) {
    free(pp1);
    pp1 = NULL;
  }
  if (pp2) {
    free(pp2);
    pp2 = NULL;
  }
  if (pp3) {
    free(pp3);
    pp3 = NULL;
  }
  if (pp4) {
    free(pp4);
    pp4 = NULL;
  }
  if (pp5) {
    free(pp5);
    pp5 = NULL;
  }
  if (pp6) {
    free(pp6);
    pp6 = NULL;
  }
  if (pp7) {
    free(pp7);
    pp7 = NULL;
  }
  if (pp8) {
    free(pp8);
    pp8 = NULL;
  }
  if (pp9) {
    free(pp9);
    pp9 = NULL;
  }
  if (pp10) {
    free(pp10);
    pp10 = NULL;
  }

  return retval;
}
