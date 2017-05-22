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

// $Id: test2_13.C,v 1.1 2008/10/30 19:20:16 legendre Exp $
/*
 * #Name: test2_13
 * #Desc: loadLibrary failure test
 * #Dep: 
 * #Arch: !(i386_unknown_nt4_0_test,alpha_dec_osf4_0_test)
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

#include "dyninst_comp.h"

static BPatch *bpatch;
static char loadLibErrStr[256] = "no error";
static void llErrorFunc(BPatchErrorLevel level, int num,
			const char * const *params);

class test2_13_Mutator : public DyninstMutator {
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT  TestMutator *test2_13_factory() {
  return new test2_13_Mutator();
}

static void llErrorFunc(BPatchErrorLevel level, int num, const char * const *params)
{

  char line[256];
  const char *msg = bpatch->getEnglishErrorString(num);
  bpatch->formatErrorString(line, sizeof(line), msg, params);

  if (num == 124) {
    strcpy(loadLibErrStr, line);
  }
  else {
    logerror("Unexpected Error #%d (level %d): %s\n", num, level, line);
  }

}

// Start Test Case #13 - (loadLibrary failure test)
// static int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage )
test_results_t test2_13_Mutator::executeTest() {
  test_results_t retval;
  if (appProc->isTerminated()) {
    logerror( "**Failed** test #13 (dlopen failure reporting test)\n" );
    logerror("%s[%d]: mutatee in unexpected (terminated) state\n", __FILE__, __LINE__);
    return FAILED;
  }

  BPatchErrorCallback oldErrorFunc = bpatch->registerErrorCallback(llErrorFunc);
  
  if (appProc->loadLibrary("noSuchLibrary.Ever")) {
    logerror("**Failed** test #13 (failure reporting for loadLibrary)\n");
    retval = FAILED;
  }
  else {
    if (!strcmp(loadLibErrStr, "no error") || !strcmp(loadLibErrStr, "")) {
      logerror( "**Failed** test #13 (dlopen failure reporting test)\n" );
      logerror( "\tno error string produced\n" );
      retval = FAILED;
    }
    else {
      // Set the variable test2_12_passed in the mutatee
      BPatch_variableExpr *passed_expr =
	appImage->findVariable("test2_13_passed");
      if (passed_expr == NULL) {
	logerror("**Failed** test #13 (dlopen failure reporting test)\n");
	logerror("    Unable to locate test2_13_passed\n");
	retval = FAILED;
      } else {
	int pvalue = 1;
	passed_expr->writeValue(&pvalue);
	logerror( "Passed test #13 (dlopen failure test: %s)\n",
		  loadLibErrStr);
	retval = PASSED;
      }
    }
  }
  bpatch->registerErrorCallback(oldErrorFunc);
  return retval;
}

// extern "C" TEST_DLL_EXPORT int test2_13_mutatorMAIN(ParameterDict &param)
test_results_t test2_13_Mutator::setup(ParameterDict &param) {
    int createmode = param["createmode"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());

    appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    appProc = appThread->getProcess();

    // Read the program's image and get an associated image object
    appImage = appProc->getImage();

    // Signal the child that we've attached
    if (createmode == USEATTACH) {
       signalAttached(appImage);
    }

    return PASSED;
}
