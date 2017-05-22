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

#define do_dyninst_breakpoint() stop_process_()

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_stack_3_func1();
void test_stack_3_func2();
void test_stack_3_func3();
void test_stack_3_func4();

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

/* Function definitions follow */

void test_stack_3_func4() {
  /* Do nothing */ ;
}

void test_stack_3_func3() {
  do_dyninst_breakpoint();
} /* end test_stack_3_func3() */

void test_stack_3_func2() {
  /* This function will be instrumented to call test_stack_3_func3, which
     stops the mutatee and allows the mutator to walk the stack. */
	   
  /* This is to give us a third place to instrument. */
  test_stack_3_func4();
} /* end test_stack_3_func2() */
	
void test_stack_3_func1() {
  /* Stop myself.  The mutator will instrument test_stack_3_func2() at this point. */
  do_dyninst_breakpoint();
	
  /* This function will be instrumented. */
  test_stack_3_func2();
} /* end test_stack_3_func1() */

/* skeleton test doesn't do anything besides say that it passed */
int test_stack_3_mutatee() {
  test_stack_3_func1();
  return 0; /* Return code is not checked */
}
