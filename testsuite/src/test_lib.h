/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

#ifndef _test_lib_h_
#define _test_lib_h_

#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "Process_data.h"
#include "ParameterDict.h"
#include "TestData.h"
#include "test_lib_dll.h"


#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 10 apr 2001 
#define P_sleep(sec) Sleep(1000*(sec))
#include <windows.h>
#else
#define P_sleep(sec) sleep(sec)
#include <unistd.h>
#endif

#define DYNINST_NO_ERROR -1

#define RETURNONFAIL(x) if ( x < 0 ) return -1;
#define RETURNONNULL(x) if ( x == NULL ) return -1;
#define PASS 0
#define FAIL -1

typedef enum {
   TEST1,
   TEST2,
   TEST2B,
   TEST3,
   TEST4,
   TEST12,
} test_style_t;


// Functions in test_lib.C

TESTLIB_DLL_EXPORT int waitUntilStopped(BPatch *, BPatch_thread *appThread, 
                      int testnum, const char *testname);
TESTLIB_DLL_EXPORT void signalAttached(BPatch_thread *appThread, BPatch_image *appImage);
int startNewProcessForAttach(const char *pathname, const char *argv[]);


TESTLIB_DLL_EXPORT void dprintf(const char *fmt, ...);

TESTLIB_DLL_EXPORT void checkCost(BPatch_snippet snippet);

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
TESTLIB_DLL_EXPORT BPatch_variableExpr *findVariable(BPatch_image *appImage, const char* var,
                                  BPatch_Vector <BPatch_point *> *point);

void setMutateeFortran(int mutFor);
TESTLIB_DLL_EXPORT void setDebugPrint(int debug);

//
//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
TESTLIB_DLL_EXPORT int replaceFunctionCalls(BPatch_thread *appThread, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected);
//
//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
TESTLIB_DLL_EXPORT BPatchSnippetHandle *insertSnippetAt(BPatch_thread *appThread,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName);

//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
TESTLIB_DLL_EXPORT int insertCallSnippetAt(BPatch_thread *appThread,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName);


TESTLIB_DLL_EXPORT BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail);



TESTLIB_DLL_EXPORT void addLibArchExt(char *dest, unsigned int dest_max_len);

// Function to preload some libraries for test1_21 and test1_22
TESTLIB_DLL_EXPORT int readyTest21or22(BPatch_thread *appThread, char *libNameA, char *libNameB);


TESTLIB_DLL_EXPORT void instrument_entry_points( BPatch_thread * app_thread,
			      BPatch_image * ,
			      BPatch_function * func,
			      BPatch_snippet * code );


TESTLIB_DLL_EXPORT void instrument_exit_points( BPatch_thread * app_thread,
			     BPatch_image * ,
			     BPatch_function * func,
			     BPatch_snippet * code );

// Tests to see if the mutatee has defined the mutateeCplusplus flag
int isMutateeCxx(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeFortran flag
TESTLIB_DLL_EXPORT int isMutateeFortran(BPatch_image *appImage);
// Tests to see if the mutatee has defined the mutateeF77 flag
int isMutateeF77(BPatch_image *appImage);

TESTLIB_DLL_EXPORT void MopUpMutatees(const unsigned int mutatees, BPatch_thread *appThread[]);

void contAndWaitForAllThreads(BPatch *bpatch, BPatch_thread *appThread, 
      BPatch_thread **mythreads, int *threadCount);

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
TESTLIB_DLL_EXPORT bool verifyChildMemory(BPatch_thread *appThread, 
                       const char *name, int expectedVal);


TESTLIB_DLL_EXPORT void dumpvect(BPatch_Vector<BPatch_point*>* res, const char* msg);

TESTLIB_DLL_EXPORT bool validate(BPatch_Vector<BPatch_point*>* res,
                            BPatch_memoryAccess* acc[], const char* msg);

BPatch_callWhen instrumentWhere(  const BPatch_memoryAccess* memAccess);

TESTLIB_DLL_EXPORT int instCall(BPatch_thread* bpthr, const char* fname,
              const BPatch_Vector<BPatch_point*>* res);


TESTLIB_DLL_EXPORT int instEffAddr(BPatch_thread* bpthr, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional);

TESTLIB_DLL_EXPORT int instByteCnt(BPatch_thread* bpthr, const char* fname,
		 const BPatch_Vector<BPatch_point*>* res,
                 bool conditional);

typedef struct {
    bool             valid;
    bool             optional;
    BPatch_frameType type;
    const char      *function_name;
} frameInfo_t;

TESTLIB_DLL_EXPORT bool checkStack(BPatch_thread *appThread,
		const frameInfo_t correct_frame_info[],
		unsigned num_correct_names,
		int test_num, const char *test_name);

void buildArgs(const char** child_argv, char *pathname, int testNo);


void createNewProcess(BPatch *bpatch, BPatch_thread *&appThread, BPatch_image *&appImage, 
      char *pathname, const char** child_argv);


int instrumentToCallZeroArg(BPatch_thread *appThread, BPatch_image *appImage, char *instrumentee, char*patch, int testNo, char *testName);


char* saveWorld(BPatch_thread *appThread);

int letOriginalMutateeFinish(BPatch_thread *appThread);

BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname);
BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname);

void setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);
void getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);


// Functions in test_lib_soExecution.C below
//           or test_lib_dllExecution.C

   
TESTLIB_DLL_EXPORT int loadLibRunTest(test_data_t &testLib, ParameterDict &param);

// Function in MutateeStart.C
BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, char *pathname, const char **child_argv, bool useAttach);

BPatch_thread *startMutateeTestAll(BPatch *bpatch, char *pathname, bool useAttach, ProcessList &procList);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, 
      bool useAttach);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, 
      bool useAttach, ProcessList &procList);

BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool useAttach, test_data_t tests[], unsigned int num_tests, int oldtest);

TESTLIB_DLL_EXPORT void killMutatee(BPatch_thread *appThread);


#endif


