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

/* $Id: test1_26F_mutatee.c,v 1.1 2008/10/30 19:18:39 legendre Exp $ */

#include <stdlib.h>
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

struct struct26_1 {
    int field1_;
    int field2_;
};

struct struct26_2_ {
    int field1_;
    int field2_;
    int field3_[10];
    struct struct26_1 field4_;
};

// **********************************************************************
// The following structure (struct block_) is made to correspond with the
// Fortran common block (globals) defined in test1_common.h.  Be sure all
// changes to this structure are reflected in the other.
// **********************************************************************
struct block_ {
/*	struct struct26_2_ globalVariable26_1; */
  int globalVariable26_2_;
  int globalVariable26_3_, globalVariable26_4_, globalVariable26_5_, globalVariable26_6_,
    globalVariable26_7_, globalVariable26_8_, globalVariable26_9_, globalVariable26_10_, globalVariable26_11_,
    globalVariable26_12_, globalVariable26_13_;
  int passedTest_;
};

#if !defined(XLF)
#define func26_1 func26_1_
#define test1_26f_init_globals test1_26f_init_globals_
#define test1_26f_globals test1_26f_globals_
#endif

extern struct block_ test1_26f_globals;

extern void test1_26f_init_globals ();
extern void func26_1 ();

int mutateeFortran = 1;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

int test1_26F_mutatee() {
  test1_26f_globals.passedTest_ = FALSE;

  if (setupFortranOutput()) {
    logerror("Error redirecting Fortran component output to log file\n");
  }

  test1_26f_init_globals();

  func26_1();

  if (cleanupFortranOutput()) {
    logerror("Error restoring output to stdout\n");
  }

    /* Combine fortran passedTest with C passedTest */
  if (test1_26f_globals.passedTest_) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}
