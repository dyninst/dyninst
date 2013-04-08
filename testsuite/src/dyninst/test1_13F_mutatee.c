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

/* $Id: test1_13F_mutatee.c,v 1.1 2008/10/30 19:17:41 legendre Exp $ */

#include <stdlib.h>
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

// **********************************************************************
// The following structure (struct block_) is made to correspond with the
// Fortran common block (globals) defined in test1_common.h.  Be sure all
// changes to this structure are reflected in the other.
// **********************************************************************
struct block_ {
  int globalVariable13_1;
  int passedTest_;
};

#if !defined(XLF)
#define test1_13_func1 test1_13_func1_
#define test1_13f_init_globals test1_13f_init_globals_
#define test1_13f_globals test1_13f_globals_
#endif

extern struct block_ test1_13f_globals;

extern void test1_13f_init_globals ();
extern void test1_13_func1 ();

int mutateeFortran = 1;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

int test1_13F_mutatee() {
  int retval;
  int *pp1, *pp2, *pp3, *pp4, *pp5;

  test1_13f_globals.passedTest_ = FALSE;

  pp1 = (int*) malloc (sizeof (int));
  pp2 = (int*) malloc (sizeof (int));
  pp3 = (int*) malloc (sizeof (int));
  pp4 = (int*) malloc (sizeof (int));
  pp5 = (int*) malloc (sizeof (int));

  if (pp1 && pp2 && pp3 && pp4 && pp5) {
    *pp1 = 1; *pp2 = 2; *pp3 = 3; *pp4 = 4; *pp5 = 5;

    if (setupFortranOutput()) {
      logerror("Error redirecting Fortran component output to log file\n");
    }

    test1_13f_init_globals();

    *pp1 = 131; *pp2 = 132; *pp3 = 133; *pp4 = 134; *pp5 = 135;

    test1_13_func1(pp1, pp2, pp3, pp4, pp5);

    if (cleanupFortranOutput()) {
      logerror("Error restoring output to stdout\n");
    }

    /* Combine fortran passedTest with C passedTest */
    if (test1_13f_globals.passedTest_) {
      test_passes(testname);
      retval = 0; /* Passed test */
    } else {
      retval = -1; /* Failed test */
    }
  } else {
    /* Error allocating memory */
    logerror("**Failed** test1_13F ()\n");
    logerror("    Error allocating memory\n");
    retval = -1; /* Failed test */
  }

  /* Clean up memory */
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

  return retval;
}
