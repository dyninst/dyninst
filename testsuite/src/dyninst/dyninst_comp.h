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

#ifndef dyninst_comp_h_
#define dyninst_comp_h_

#include "test_lib.h"
#include "TestMutator.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_process.h"
#include "BPatch_function.h"

// Base class for the mutator part of a test
class COMPLIB_DLL_EXPORT DyninstMutator : public TestMutator {
public:
  BPatch_thread *appThread;
  BPatch_addressSpace *appAddrSpace;
  BPatch_binaryEdit *appBinEdit;
  BPatch_process *appProc;
  // FIXME This field (appImage) probably isn't necessary.  It looks looks like
  // appImage is easily derivable from appThread.
  BPatch_image *appImage;

  create_mode_t runmode;

  DyninstMutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual ~DyninstMutator();
};
extern "C" {
	TEST_DLL_EXPORT TestMutator *TestMutator_factory();
}

COMPLIB_DLL_EXPORT int waitUntilStopped(BPatch *, BPatch_process *appThread,
                      int testnum, const char *testname);
COMPLIB_DLL_EXPORT bool signalAttached(BPatch_image *appImage);

COMPLIB_DLL_EXPORT void checkCost(BPatch_snippet snippet);

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
COMPLIB_DLL_EXPORT BPatch_variableExpr *findVariable(BPatch_image *appImage, 
                                  const char* var,
                                  BPatch_Vector <BPatch_point *> *point);


//
//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
COMPLIB_DLL_EXPORT int replaceFunctionCalls(BPatch_addressSpace *appAddrSpace, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected);
//
//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
COMPLIB_DLL_EXPORT BPatchSnippetHandle *insertSnippetAt(BPatch_addressSpace *appThread,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName);

//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
COMPLIB_DLL_EXPORT int insertCallSnippetAt(BPatch_addressSpace *appAddrSpace,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName);


COMPLIB_DLL_EXPORT BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail);



// Function to preload some libraries for test1_21 and test1_22
COMPLIB_DLL_EXPORT int readyTest21or22(BPatch_thread *appThread, 
      char *libNameA, char *libNameB, int mutateeFortran);

COMPLIB_DLL_EXPORT int strcmpcase(char *s1, char *s2);

COMPLIB_DLL_EXPORT void instrument_entry_points( BPatch_addressSpace * app_thread,
			      BPatch_image * ,
			      BPatch_function * func,
			      BPatch_snippet * code );


COMPLIB_DLL_EXPORT void instrument_exit_points( BPatch_addressSpace * app_thread,
			     BPatch_image * ,
			     BPatch_function * func,
			     BPatch_snippet * code );

// Tests to see if the mutatee has defined the mutateeCplusplus flag
COMPLIB_DLL_EXPORT int isMutateeCxx(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeFortran flag
COMPLIB_DLL_EXPORT int isMutateeFortran(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeF77 flag
int isMutateeF77(BPatch_image *appImage);

COMPLIB_DLL_EXPORT void MopUpMutatees(const int mutatees, BPatch_process *appProc[]);

COMPLIB_DLL_EXPORT void contAndWaitForAllProcs(BPatch *bpatch, BPatch_process *appThread,
        BPatch_process **mythreads, int *threadCount);

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
COMPLIB_DLL_EXPORT bool verifyChildMemory(BPatch_process *appThread,
                       const char *name, int expectedVal);


COMPLIB_DLL_EXPORT void dumpvect(BPatch_Vector<BPatch_point*>* res, const char* msg);

COMPLIB_DLL_EXPORT bool validate(BPatch_Vector<BPatch_point*>* res,
                            BPatch_memoryAccess* acc[], const char* msg);

COMPLIB_DLL_EXPORT BPatch_callWhen instrumentWhere(  const BPatch_memoryAccess* memAccess);

COMPLIB_DLL_EXPORT int instCall(BPatch_addressSpace* as, const char* fname,
				const BPatch_Vector<BPatch_point*>* res);


COMPLIB_DLL_EXPORT int instEffAddr(BPatch_addressSpace* as, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional);

COMPLIB_DLL_EXPORT int instByteCnt(BPatch_addressSpace* as, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional);

COMPLIB_DLL_EXPORT int pointerSize(BPatch_image *img);

typedef struct {
    bool             valid;
    bool             optional;
    BPatch_frameType type;
    const char      *function_name;
} frameInfo_t;

COMPLIB_DLL_EXPORT bool checkStack(BPatch_thread *appThread,
		const frameInfo_t correct_frame_info[],
		unsigned num_correct_names,
		int test_num, const char *test_name);

COMPLIB_DLL_EXPORT void buildArgs(const char** child_argv, char *pathname, int testNo);


COMPLIB_DLL_EXPORT int instrumentToCallZeroArg(BPatch_thread *appThread, 
                                               BPatch_image *appImage, 
                                               char *instrumentee, char*patch, 
                                               int testNo, char *testName);


COMPLIB_DLL_EXPORT int letOriginalMutateeFinish(BPatch_thread *appThread);

COMPLIB_DLL_EXPORT BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname);
COMPLIB_DLL_EXPORT BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname);

COMPLIB_DLL_EXPORT bool setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);
COMPLIB_DLL_EXPORT bool getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);

// Functions in test_lib_soExecution.C below
//           or test_lib_dllExecution.C

COMPLIB_DLL_EXPORT BPatch_binaryEdit *startBinaryTest(BPatch *bpatch, RunGroup *group);

COMPLIB_DLL_EXPORT bool runBinaryTest(RunGroup *group, ParameterDict &params, test_results_t &test_result);

// COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTestSet(BPatch *bpatch, char *pathname, 
// 				   test_data_t tests[],
// 				   int first_test, int last_test,
// 				   bool createmode, ProcessList &procList,
// 				   char *logfilename, bool runAllTests,
// 				   std::vector<char *> test_list);

// COMPLIB_DLL_EXPORT BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool createmode, test_data_t tests[], unsigned int num_tests, int oldtest, char *logfilename);

COMPLIB_DLL_EXPORT void killMutatee(BPatch_process *appThread);

#endif


