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
#include <sys/types.h>
#include <unistd.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_fork_13_func1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

int test_fork_13_global1 = 1;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int dummyVal = 0;

/* Function definitions follow */

void test_fork_13_func1() { 
  dummyVal += 10;
}

int test_fork_13_mutatee() {
#if defined(i386_unknown_nt4_0_test)
  return 0;
#endif
  int pid;
  /* dprintf("mutatee:  starting fork\n"); */
  pid = fork();
  /* dprintf("mutatee:  stopping fork\n"); */

  /* mutatee will get paused here, temporarily, when the mutator receives
     the postForkCallback */

  if (pid == 0) {   /* child */
    dprintf("Child: starting tests\n");
    test_fork_13_func1();
    dprintf("Child: done with tests, exiting\n");
  } else if(pid > 0) {
    dprintf("Parent: starting tests\n");
    test_fork_13_func1();
    dprintf("Parent: done with tests, exiting\n");
  } else if(pid < 0) {
    logerror("error on fork\n");
    return -1;  /* error case */
  }

  return 0; /* This mutatee's return code is not checked */
}
