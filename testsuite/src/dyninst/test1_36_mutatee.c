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

int test1_36_call1(int i1, int i2, int i3, int i4, int i5, int i6, int i7,
		   int i8, int i9, int i10);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test1_36_globalVariable1 = 0;
int test1_36_globalVariable2 = 0;
int test1_36_globalVariable3 = 0;
int test1_36_globalVariable4 = 0;
int test1_36_globalVariable5 = 0;
int test1_36_globalVariable6 = 0;
int test1_36_globalVariable7 = 0;
int test1_36_globalVariable8 = 0;
int test1_36_globalVariable9 = 0;
int test1_36_globalVariable10 = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* Test #36 (callsite parameter referencing) */

int test1_36_func1() {
   int failure = 0;

   int result = test1_36_call1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

   if(result != 55) {
      logerror("  expecting a value of 55 from test1_36_call1, got %d\n",
              result);
      failure = 1;
   }

   if(test1_36_globalVariable1 != 1) {
      logerror("   for test 36, expecting arg1 value of 1, got %d\n",
              test1_36_globalVariable1);
      failure = 1;      
   }
   if(test1_36_globalVariable2 != 2) {
      logerror("   for test 36, expecting arg2 value of 2, got %d\n",
              test1_36_globalVariable2);
      failure = 1;      
   }
   if(test1_36_globalVariable3 != 3) {
      logerror("   for test 36, expecting arg3 value of 3, got %d\n",
              test1_36_globalVariable3);
      failure = 1;      
   }
   if(test1_36_globalVariable4 != 4) {
      logerror("   for test 36, expecting arg4 value of 4, got %d\n",
              test1_36_globalVariable4);
      failure = 1;
   }
   if(test1_36_globalVariable5 != 5) {
      logerror("   for test 36, expecting arg5 value of 5, got %d\n",
              test1_36_globalVariable5);
      failure = 1;
   }
   if(test1_36_globalVariable6 != 6) {
      logerror("   for test 36, expecting arg6 value of 6, got %d\n",
              test1_36_globalVariable6);
      failure = 1;
   }
   if(test1_36_globalVariable7 != 7) {
      logerror("   for test 36, expecting arg7 value of 7, got %d\n",
              test1_36_globalVariable7);
      failure = 1;
   }
   if(test1_36_globalVariable8 != 8) {
      logerror("   for test 36, expecting arg8 value of 8, got %d\n",
              test1_36_globalVariable8);
      failure = 1;
   }
   if(test1_36_globalVariable9 != 9) {
      logerror("   for test 36, expecting arg9 value of 9, got %d\n",
              test1_36_globalVariable9);
      failure = 1;
   }
   if(test1_36_globalVariable10 != 10) {
      logerror("   for test 36, expecting arg10 value of 10, got %d\n",
              test1_36_globalVariable10);
      failure = 1;
   }

   if(failure == 0) {
      logerror( "Passed test #36 (callsite parameter referencing)\n" );    
      return 0; /* Test passed */
   } else {
      logerror( "**Failed** test #36 (callsite parameter referencing)\n");
      return -1; /* Test failed */
   }
}

int test1_36_mutatee() {
  if (test1_36_func1() == 0) {
    test_passes(testname);
    return 0;
  } else {
    return -1;
  }
}

int test1_36_call1(int i1, int i2, int i3, int i4, int i5, int i6, int i7,
		   int i8, int i9, int i10) {
   return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
}
