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
#include <stdlib.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

#if defined(os_freebsd_test)
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test4_2_func2();
void test4_2_func3();
void test4_2_func4();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

unsigned int test4_2_global1 = 0xdeadbeef;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* should be called by parent */
void test4_2_func3() {
    dprintf("pid %d in test4_2_func3\n", (int) getpid());
    test4_2_global1 = 2000002;
}

/* should be called by child */
void test4_2_func4() {
    dprintf("pid %d in test4_2_func4\n", (int) getpid());
    test4_2_global1 = 2000003;
}

void test4_2_func2() {
    /* if we get this value at exit, no function was called */
    test4_2_global1 = 2000001;
}

int test4_2_mutatee() {
#ifndef os_windows_test
    int pid;

    pid = fork();
    dprintf("fork result: %d\n", pid);

    if (pid >= 0) {
       /* both parent and child exit here */
       test4_2_func2();
       dprintf("at exit of %d, test4_2_global1 = %d\n", (int) getpid(),
               test4_2_global1);
       
#if defined(rs6000_ibm_aix4_1_test)
       if( pid > 0){
          dprintf("%d waiting for child\n", getpid());

          /* On AIX the child dies when the parent exits, so wait */
          /* apparently the parent needs to wake up occasionally to keep Dyninst happy */
          dprintf("%d SLEEPING\n",getpid());
          sleep(5);
          dprintf("%d SLEEP MORE\n",getpid());
          sleep(1);
          dprintf("%d SLEEP MORE\n",getpid());
          sleep(5);
          dprintf("%d DONE SLEEPING\n",getpid());
       }
#endif

       dprintf("Mutatee %d exiting...\n", getpid());
       exit(getpid());
    } else if (pid < 0) {
       /* error case */
       exit(pid);
    }
#endif
    return 0; /* No error, technically.  Shouldn't get here */
}
