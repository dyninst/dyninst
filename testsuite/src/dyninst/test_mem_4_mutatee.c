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
#include "test_mem_util.h"
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

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

int passed = 0;

/* Function definitions follow */

static void check()
{
  if (accessCnt == accessExp) {
    logerror("Passed test_mem_4 (access instrumentation)\n");
    passed = 1;
  } else {
    logerror("**Failed** test_mem_4 (access instrumentation)\n");
    logerror("    access counter seems wrong.\n");
  }
  dprintf("accessCnt = %d    accessExp = %d\n", accessCnt, accessExp);
}

/* skeleton test doesn't do anything besides say that it passed */
int test_mem_4_mutatee() {
  loadCnt = 0;
  storeCnt = 0;
  prefeCnt = 0;
  accessCnt = 0;

  if (setupFortranOutput()) {
    logstatus("Error redirecting assembly component output to log file\n");
  }

  result_of_loadsnstores = loadsnstores(2,3,4);
  dprintf("\nresult=0x%x loads=%d stores=%d prefetches=%d accesses=%d\n",
          result_of_loadsnstores, loadCnt, storeCnt, prefeCnt, accessCnt);

  init_test_data();

  check();

  if (cleanupFortranOutput()) {
    logstatus("Error restoring output to stdout\n");
  }

  if (passed) {
    test_passes(testname);
    return 0; /* No error */
  } else {
    return -1; /* Error */
  }
}
