/*
 * Copyright (c) 1996-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#if !defined(dyninst_comp_h_)
#define dyninst_comp_h

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
  // FIXME This field (appImage) probably isn't necessary.  It looks looks like
  // appImage is easily derivable from appThread.
  BPatch_image *appImage;

  DyninstMutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual ~DyninstMutator();
};
extern "C" {
TestMutator *TestMutator_factory();
}

COMPLIB_DLL_EXPORT int waitUntilStopped(BPatch *, BPatch_thread *appThread, 
                      int testnum, const char *testname);
COMPLIB_DLL_EXPORT bool signalAttached(BPatch_thread *appThread, BPatch_image *appImage);

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
COMPLIB_DLL_EXPORT int replaceFunctionCalls(BPatch_thread *appThread, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected);
//
//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
COMPLIB_DLL_EXPORT BPatchSnippetHandle *insertSnippetAt(BPatch_thread *appThread,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName);

//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
COMPLIB_DLL_EXPORT int insertCallSnippetAt(BPatch_thread *appThread,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName);


COMPLIB_DLL_EXPORT BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail);



// Function to preload some libraries for test1_21 and test1_22
COMPLIB_DLL_EXPORT int readyTest21or22(BPatch_thread *appThread, 
      char *libNameA, char *libNameB, int mutateeFortran);

COMPLIB_DLL_EXPORT int strcmpcase(char *s1, char *s2);

COMPLIB_DLL_EXPORT void instrument_entry_points( BPatch_thread * app_thread,
			      BPatch_image * ,
			      BPatch_function * func,
			      BPatch_snippet * code );


COMPLIB_DLL_EXPORT void instrument_exit_points( BPatch_thread * app_thread,
			     BPatch_image * ,
			     BPatch_function * func,
			     BPatch_snippet * code );

// Tests to see if the mutatee has defined the mutateeCplusplus flag
COMPLIB_DLL_EXPORT int isMutateeCxx(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeFortran flag
COMPLIB_DLL_EXPORT int isMutateeFortran(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeF77 flag
int isMutateeF77(BPatch_image *appImage);

COMPLIB_DLL_EXPORT void MopUpMutatees(const unsigned int mutatees, BPatch_thread *appThread[]);

COMPLIB_DLL_EXPORT void contAndWaitForAllThreads(BPatch *bpatch, BPatch_thread *appThread, 
      BPatch_thread **mythreads, int *threadCount);

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
COMPLIB_DLL_EXPORT bool verifyChildMemory(BPatch_thread *appThread, 
                       const char *name, int expectedVal);


COMPLIB_DLL_EXPORT void dumpvect(BPatch_Vector<BPatch_point*>* res, const char* msg);

COMPLIB_DLL_EXPORT bool validate(BPatch_Vector<BPatch_point*>* res,
                            BPatch_memoryAccess* acc[], const char* msg);

COMPLIB_DLL_EXPORT BPatch_callWhen instrumentWhere(  const BPatch_memoryAccess* memAccess);

COMPLIB_DLL_EXPORT int instCall(BPatch_thread* bpthr, const char* fname,
				const BPatch_Vector<BPatch_point*>* res);


COMPLIB_DLL_EXPORT int instEffAddr(BPatch_thread* bpthr, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional);

COMPLIB_DLL_EXPORT int instByteCnt(BPatch_thread* bpthr, const char* fname,
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


COMPLIB_DLL_EXPORT bool createNewProcess(BPatch *bpatch, BPatch_thread *&appThread, 
                                         BPatch_image *&appImage, char *pathname, 
                                         const char** child_argv);


COMPLIB_DLL_EXPORT int instrumentToCallZeroArg(BPatch_thread *appThread, 
                                               BPatch_image *appImage, 
                                               char *instrumentee, char*patch, 
                                               int testNo, char *testName);


COMPLIB_DLL_EXPORT char* saveWorld(BPatch_thread *appThread);

COMPLIB_DLL_EXPORT int letOriginalMutateeFinish(BPatch_thread *appThread);

COMPLIB_DLL_EXPORT BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname);
COMPLIB_DLL_EXPORT BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname);

COMPLIB_DLL_EXPORT bool setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);
COMPLIB_DLL_EXPORT bool getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);

// Functions in test_lib_soExecution.C below
//           or test_lib_dllExecution.C

// Function in MutateeStart.C
COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, const char *pathname, const char **child_argv, bool useAttach);

COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, const char *mutatee, const char *testname, bool useAttach, char *logfilename, char *humanlogname);

COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group, char *logfilename, char *humanlogname, bool verboseFormat, bool printLabels, int debugPrint, char *pidfilename);

COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group, char *logfilename, char *humanlogname, bool verboseFormat, bool printLabels, int debugPrint, char *pidfilename);

// COMPLIB_DLL_EXPORT BPatch_thread *startMutateeTestSet(BPatch *bpatch, char *pathname, 
// 				   test_data_t tests[],
// 				   int first_test, int last_test,
// 				   bool useAttach, ProcessList &procList,
// 				   char *logfilename, bool runAllTests,
// 				   std::vector<char *> test_list);

// COMPLIB_DLL_EXPORT BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool useAttach, test_data_t tests[], unsigned int num_tests, int oldtest, char *logfilename);

COMPLIB_DLL_EXPORT void killMutatee(BPatch_thread *appThread);

#endif


