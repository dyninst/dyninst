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

// $Id: test2_9.C,v 1.1 2008/10/30 19:20:31 legendre Exp $
/*
 * #Name: test2_9
 * #Desc: dump core but do not terminate
 * #Dep: 
 * #Arch: (sparc_sun_sunos4_1_3_test,sparc_sun_solaris2_4_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

#include "dyninst_comp.h"
class test2_9_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_9_factory() {
  return new test2_9_Mutator();
}

//
// Test #9 - dump core but do not terminate
//	This test dumps the core file from the mutatee process, without having
//	the process terminate exuection.  It looks for the creation of the file
//	"mycore" in the current directory.
//      
// static int mutatorTest(BPatch_thread *appThread, BPatch_image * /*appImage*/)
test_results_t test2_9_Mutator::executeTest() {
    // dump core, but do not terminate.
    // this doesn't seem to do anything - jkh 7/12/97
    if (access("mycore", F_OK) == 0) {
        dprintf("File \"mycore\" exists.  Deleting it.\n");
	if (unlink("mycore") != 0) {
	    fprintf(stderr, "Couldn't delete the file \"mycore\".  Exiting.\n");
	    // Get rid of this exit() call and replace it with something
	    // that makes more sense
	    return FAILED;
	}
    }

    clearError();
    appThread->dumpCore("mycore", true); // FIXME deprecated function; also, true should terminate the process(?)
    bool coreExists = (access("mycore", F_OK) == 0);
    int gotError = getError();
    if (gotError || !coreExists) {
	logerror("**Failed** test #9 (dump core but do not terminate)\n");
	if (gotError)
	    logerror("    error reported by dumpCore\n");
	if (!coreExists)
	    logerror("    the core file wasn't written\n");
        return FAILED;
    } else {
	unlink("mycore");
    	logerror("Passed test #9 (dump core but do not terminate)\n");
        return PASSED;
    }
}
