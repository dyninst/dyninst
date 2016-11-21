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
#include <signal.h>
#include <unistd.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

#define do_dyninst_breakpoint() stop_process_()

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_stack_4_func1();
void test_stack_4_func2();
void test_stack_4_func3();
void test_stack_4_func4();
void test_stack_4_func5();
void test_stack_4_sigalrm_handler(int signum);

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

static int globalVariable4_1 = 0;
static volatile int globalVariable4_2 = 0;

/* Function definitions follow */

#if !defined(i386_unknown_nt4_0_test)
void test_stack_4_func5() {
    globalVariable4_1++;
}

void test_stack_4_func4() {
    /* Breakpoint and allow the mutator to walk the stack */
    do_dyninst_breakpoint();
}

void test_stack_4_sigalrm_handler(int signum)
{
    /* this handler will be instrumented */
    globalVariable4_1++;
    globalVariable4_2 = 1;
    test_stack_4_func5();
}

void test_stack_4_func3() {
    globalVariable4_1++;

    /* Cause a SIGALRM */
    alarm(1);
    while (0 == globalVariable4_2) ;
}

void test_stack_4_func2() {	   
    globalVariable4_1++;
    test_stack_4_func3();
}
#endif /* i386_unknown_nt4_0_test */

void test_stack_4_func1() {
#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) \
 || defined(amd64_unknown_freebsd7_0_test) \
 || defined(i386_unknown_freebsd7_0_test) /* Blind duplication */ \
 || defined(sparc_sun_solaris2_4_test) \
 || defined(ia64_unknown_linux2_4_test)

    /* Breakpoint to allow instrumenting signal handler */
    do_dyninst_breakpoint();

    void (*old_handler)(int) = signal(SIGALRM, test_stack_4_sigalrm_handler);

    globalVariable4_1++;
    test_stack_4_func2();

    signal(SIGALRM, old_handler);
#endif
}

/* skeleton test doesn't do anything besides say that it passed */
int test_stack_4_mutatee() {
#ifndef os_windows_test
  /* Windows doesn't support signals */
  test_stack_4_func1();
#endif
  return 0; /* Return code is not checked */
}
