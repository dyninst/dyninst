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

void test1_37_call1();
void test1_37_call2();
void test1_37_call3();
void test1_37_inc1();
void test1_37_inc2();
void test1_37_inc3();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

volatile int globalVariable37_1 = 0;
volatile int globalVariable37_2 = 0;
volatile int globalVariable37_3 = 0;

/* Function definitions follow */

/* Test #37 (loop instrumentation) */

int test1_37_mutatee() {
  int failed = FALSE;

    const int ANSWER37_1 = 11002;
    const int ANSWER37_2 = 26;
    const int ANSWER37_3 = 752;

    test1_37_call1();
    test1_37_call2();
    test1_37_call3();

    if (globalVariable37_1 != ANSWER37_1) {
      failed = TRUE;
	logerror( "**Failed** test #37 (instrument loops)\n");
	logerror( "  globalVariable37_1 is %d, should have been %d.\n",
		globalVariable37_1, ANSWER37_1);
    }
    if (globalVariable37_2 != ANSWER37_2) {
      failed = TRUE;
	logerror( "**Failed** test #37 (instrument loops)\n");
	logerror( "  globalVariable37_2 is %d, should have been %d.\n",
		globalVariable37_2, ANSWER37_2);
    } 
    if (globalVariable37_3 != ANSWER37_3) {
      failed = TRUE;
	logerror( "**Failed** test #37 (instrument loops)\n");
	logerror( "  globalVariable37_3 is %d, should have been %d.\n",
		globalVariable37_3, ANSWER37_3);
    } 
    
    if (FALSE == failed) {
	logerror( "Passed test #37 (instrument loops)\n" );
	test_passes(testname);
	return 0; /* Test passed */
    } else {
      return -1; /* Test failed */
    }
}

void test1_37_inc1() { globalVariable37_1++; }
        
/* At the end of normal execution, globalVariable37_1 should
   hold 100 + ( 100 * 5 ) + ( 100 * 10 ) + ( 100 * 10 * 7 ) = 8600
    
   If we instrument the entry and exit edge of each loop with a call
   to test1_37_inc1, globalVariable37_1 should be increased by
   2 + ( 2 * 100 ) + ( 2 * 100 * 10 ) + ( 2 * 100 ) = 2402
        
   Successful loop edge instrumentation will give us a value of 11002.
*/  
void test1_37_call1() {
    int i, j, k, m;

    for (i = 0; i < 100; i++) {
      globalVariable37_1++;

        for (j = 0; j < 10; j++) {
            globalVariable37_1++;
            
            for (k = 0; k < 7; k++) {
                globalVariable37_1++;
            }
        }
        
        m = 0;
        do {
            globalVariable37_1++;
            m++;
        } while (m < 5);
    }
}

void test1_37_inc2() { globalVariable37_2++; }

/* At the end of normal execution, globalVariable37_2 should
   hold 20.
   If we instrument the entry and exit edge of each loop with a call
   to test1_37_inc2, globalVariable37_2 should be increased by
   2 + 2 + 2 = 6

   Successful loop edge instrumentation will give us a value of 26.
*/

/* The comment below is no longer relevant, but has been left in as an
 explanation of how this test arose. The original mechanism for testing
 loops (instrumenting some arbitrary block in the body that was expected
 to execute a certain number of times was fundamentally flawed, for
 reasons that will become apparent with a little thought. We're leaving
 these tests in, though, in the hopes that the compiler will produce
 different loop idioms and thus stress-test our loop detection code.
*/

/* test with small loop bodies. since there are no statements right after the
   start of the outer two loops there isn't much space to instrument. */
void test1_37_call2() {                
    volatile int i = 0;
    volatile int j = 0;
    volatile int k = 0;
    
    while (i < 5) {
        while (j < 10) {
            do {
	      globalVariable37_2++;
                i++; j++; k++;
            } while (k < 20);
        }
    }
}


void test1_37_inc3() { globalVariable37_3++; }

/* At the end of normal execution, globalVariable37_3 should
   hold 100 / 2 + (100 / 2 ) * 10 = 550
   
   If we instrument the entry and exit edge of each loop with a call
   to test1_37_inc3, globalVariable37_3 should be increased by
   2 * 100 + 2 = 202

   Successful loop edge instrumentation will give us a value of 752.
*/

/* test with if statements as the only statements in a loop body. */
void test1_37_call3() {
    volatile int i, j;

    for (i = 0; i < 100; i++) {
        if (0 == (i % 2)) {
            globalVariable37_3++;
        }
        for (j = 0; j < 10; j++) {
            if (0 == (i % 2)) {
                globalVariable37_3++;
            }
        }
    }
}
