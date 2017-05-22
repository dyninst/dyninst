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
#include <assert.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#if defined(os_windows_test)
#include <windows.h>
#endif

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test3_3_call1(int arg1, int arg2);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

volatile int test3_3_ret = (int)0xdeadbeef;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

volatile int dummy = 1;

/* Function definitions follow */

void test3_3_call1(int arg1, int arg2)
{
     dprintf("test3_3_call1() called with arg1=%d,arg2=%d\n", arg1, arg2);
}

/*
 * Test #3 - call a function which should be instrumented to set the 
 *     global variable test3ret to a value (by the mutator).
 */
int test3_3_mutatee() {
     FILE *fp;
     char filename[80];
#if defined(os_windows_test)
     sprintf(filename, "test3.out.%d", GetCurrentProcessId());
#else
	 sprintf(filename, "test3.out.%d", getpid());
#endif
	 fp = fopen(filename, "w");

     assert(fp);
     fprintf(fp, "%d\n", test3_3_ret);
     fclose(fp);
     return 0; /* No error on this end */
}
