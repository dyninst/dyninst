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
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test1_38_call1();
/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_38_globalVariable2 = -1;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void funCall38_1();
static void funCall38_2();
static void funCall38_3();
static void funCall38_4();
static void funCall38_5();
static void funCall38_6();
static void funCall38_7();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int globalVariable38_1 = 0;

/* Function definitions follow */

/* Test #38 (basic block addresses) */

int test1_38_mutatee() {
    /* The only possible failures occur in the mutator. */
  if (1 == test1_38_globalVariable2) {
    logerror( "Passed test #38 (basic block addresses)\n" );
    test_passes(testname);
    return 0; /* Test passed */
  } else {
    return -1; /* Test failed */
  }
}

/* the mutator checks if the addresses of these call* functions are within 
   the address ranges of the basic blocks in the flowgraph */
void test1_38_call1() {
    int i, j, k, m;

    funCall38_1();

    for (i = 0; i < 50; i++) {
	m = i;
	funCall38_2();

	for (j = 0; i < 100; i++) {
	    for (k = 0; k < i ; i++) {
		funCall38_3();
	    }
	}

	funCall38_4();

	while (m < 100) {
	    funCall38_5();
	    m++;
	}

	funCall38_6();
    }

    funCall38_7();
}

void funCall38_1() { globalVariable38_1++; }
void funCall38_2() { globalVariable38_1++; }
void funCall38_3() { globalVariable38_1++; }
void funCall38_4() { globalVariable38_1++; }
void funCall38_5() { globalVariable38_1++; }
void funCall38_6() { globalVariable38_1++; }
void funCall38_7() { globalVariable38_1++; }
