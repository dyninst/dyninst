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

/* $Id: test1_36F_mutatee.c,v 1.1 2008/10/30 19:19:22 legendre Exp $ */

#include <stdlib.h>
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

// **********************************************************************
// The following structure (struct block_) is made to correspond with the
// Fortran common block (globals) defined in test1_common.h.  Be sure all
// changes to this structure are reflected in the other.
// **********************************************************************
struct block_ {
  int test1_36_globalVariable1_, test1_36_globalVariable2_, test1_36_globalVariable3_,
    test1_36_globalVariable4_, test1_36_globalVariable5_, test1_36_globalVariable6_,
    test1_36_globalVariable7_, test1_36_globalVariable8_, test1_36_globalVariable9_,
    test1_36_globalVariable10_;
   int passedTest_;
};

#if !defined(XLF)
#define test1_36_func1 test1_36_func1_
#define test1_36f_init_globals test1_36f_init_globals_
#define test1_36f_globals test1_36f_globals_
#endif

extern struct block_ test1_36f_globals;

extern void test1_36f_init_globals ();
extern void test1_36_func1 ();

int mutateeFortran = 1;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

int test1_36_globalVariable1;
int test1_36_globalVariable2;
int test1_36_globalVariable3;
int test1_36_globalVariable4;
int test1_36_globalVariable5;
int test1_36_globalVariable6;
int test1_36_globalVariable7;
int test1_36_globalVariable8;
int test1_36_globalVariable9;
int test1_36_globalVariable10;

int test1_36F_mutatee() {
  test1_36f_globals.passedTest_ = FALSE;

  if (setupFortranOutput()) {
    logerror("Error redirecting Fortran component output to log file\n");
  }

  test1_36f_init_globals();

  test1_36_func1();

  if (cleanupFortranOutput()) {
    logerror("Error restoring output to stdout\n");
  }

  /* Combine fortran passedTest with C passedTest */
  if (test1_36f_globals.passedTest_) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}

void xlf90_41_hack()
{
    test1_36f_globals.test1_36_globalVariable1_ = test1_36_globalVariable1;
    test1_36f_globals.test1_36_globalVariable2_ = test1_36_globalVariable2;
    test1_36f_globals.test1_36_globalVariable3_ = test1_36_globalVariable3;
    test1_36f_globals.test1_36_globalVariable4_ = test1_36_globalVariable4;
    test1_36f_globals.test1_36_globalVariable5_ = test1_36_globalVariable5;
    test1_36f_globals.test1_36_globalVariable6_ = test1_36_globalVariable6;
    test1_36f_globals.test1_36_globalVariable7_ = test1_36_globalVariable7;
    test1_36f_globals.test1_36_globalVariable8_ = test1_36_globalVariable8;
    test1_36f_globals.test1_36_globalVariable9_ = test1_36_globalVariable9;
    test1_36f_globals.test1_36_globalVariable10_ = test1_36_globalVariable10;
}

