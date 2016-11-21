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

void test1_32_func2();
void test1_32_func3();
void test1_32_func4( int value );

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

static int globalVariable32_1 = 0;
static int globalVariable32_2 = 0;
static int globalVariable32_3 = 0;
static int globalVariable32_4 = 0;

/* Function definitions follow */

int test1_32_mutatee() {
  int passed;

  globalVariable32_1 = 0;
  globalVariable32_2 = 0;
  globalVariable32_3 = 0;
  globalVariable32_4 = 0;

  test1_32_func2();

  passed = ( globalVariable32_3 == 1 );
  if( ! passed )
    {
      logerror( "**Failed** test #32 (non-recursive base tramp guard)\n" );
      logerror( "    globalVariable32_3 = %d, should be 1 (no instrumentation got executed?).\n",
	      globalVariable32_3 );
      return -1; /* Test failed */
    }

  passed = ( globalVariable32_4 == 3 );
  if( ! passed )
    {
      logerror( "**Failed** test #32 (non-recursive base tramp guard)\n" );
      logerror( "    globalVariable32_4 = %d, should be 3.\n",
	      globalVariable32_4 );
      switch( globalVariable32_4 )
	{
	case 0: logerror( "    Recursive guard works fine.\n" ); break;
	case 1: logerror( "    Pre-instr recursive guard does not work.\n" ); break;
	case 2: logerror( "    Post-instr recursive guard does not work.\n" ); break;
	case 3: logerror( "    None of the recursive guards work.\n" ); break;
	default: logerror( "    Something is really wrong.\n" ); break;
	}
      return -1; /* Test failed */
    }

  test_passes(testname);
  logerror( "Passed test #32 (recursive base tramp guard)\n" );

  return 0; /* Test passed */
}

void test1_32_func2() {
  globalVariable32_2 = 1;
}

void test1_32_func3() {
  globalVariable32_3 = 1;
}

void test1_32_func4( int value ) {
  if( value == 0 )
    {
      logerror( "func_32_4 called with value = 0 !\n" );
    }

  globalVariable32_4 += value;
}
