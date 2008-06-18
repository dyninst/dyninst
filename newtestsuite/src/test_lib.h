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
#include "test_info_new.h"
#include "test_lib_dll.h"

#if !defined(P_sleep)
#if defined(os_windows)
#define P_sleep(sec) Sleep(1000*(sec))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define P_sleep(sec) sleep(sec)
#include <unistd.h>
#endif
#endif

#define DYNINST_NO_ERROR -1

#include "test_results.h"
#include "TestMutator.h"
#include "TestOutputDriver.h"

#define RETURNONFAIL(x) if ( x < 0 ) return FAILED;
#define RETURNONNULL(x) if ( x == NULL ) return FAILED;
#define PASS 0
#define FAIL -1

// Functions in test_lib.C

TESTLIB_DLL_EXPORT int waitUntilStopped(BPatch *, BPatch_thread *appThread, 
                      int testnum, const char *testname);
TESTLIB_DLL_EXPORT bool signalAttached(BPatch_thread *appThread, BPatch_image *appImage);
TESTLIB_DLL_EXPORT int startNewProcessForAttach(const char *pathname, const char *argv[], FILE *outlog, FILE *errlog);


TESTLIB_DLL_EXPORT void dprintf(const char *fmt, ...);

TESTLIB_DLL_EXPORT void checkCost(BPatch_snippet snippet);

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
TESTLIB_DLL_EXPORT BPatch_variableExpr *findVariable(BPatch_image *appImage, 
                                  const char* var,
                                  BPatch_Vector <BPatch_point *> *point);


TESTLIB_DLL_EXPORT void setDebugPrint(int debug);

// New logging system
extern TESTLIB_DLL_EXPORT TestOutputDriver * getOutput();
extern TESTLIB_DLL_EXPORT void setOutput(TestOutputDriver * new_output);

// loadOutputDriver loads an output driver plug-in and returns a pointer to
// the output driver implemented by it.
TESTLIB_DLL_EXPORT TestOutputDriver *loadOutputDriver(char *odname, void * data);

// Set up the log files for test library output
TESTLIB_DLL_EXPORT void setOutputLog(FILE *log_fp);
TESTLIB_DLL_EXPORT void setErrorLog(FILE *log_fp);
TESTLIB_DLL_EXPORT FILE *getOutputLog();
TESTLIB_DLL_EXPORT FILE *getErrorLog();
TESTLIB_DLL_EXPORT void setOutputLogFilename(char *log_fn);
TESTLIB_DLL_EXPORT void setErrorLogFilename(char *log_fn);
TESTLIB_DLL_EXPORT char *getOutputLogFilename();
TESTLIB_DLL_EXPORT char *getErrorLogFilename();
// Functions to print messages to the log files
TESTLIB_DLL_EXPORT void logstatus(const char *fmt, ...);
TESTLIB_DLL_EXPORT void logerror(const char *fmt, ...);
TESTLIB_DLL_EXPORT void flushOutputLog();
TESTLIB_DLL_EXPORT void flushErrorLog();

// Functions used for redirecting output e.g. to a temp file for entering into
// the database after running a test
TESTLIB_DLL_EXPORT int printout(char *fmt, ...);
TESTLIB_DLL_EXPORT int printerr(char *fmt, ...);
TESTLIB_DLL_EXPORT int printhuman(char *fmt, ...);

// Functions related to database output
TESTLIB_DLL_EXPORT void enableDBLog(TestInfo *test, RunGroup *runGroup);
TESTLIB_DLL_EXPORT void clearDBLog();


// Mutatee PID registration, for cleaning up hung mutatees
// TODO Check if these make any sense on Windows.  I suspect I'll need to
// change them.
TESTLIB_DLL_EXPORT void setPIDFilename(char *pfn);
TESTLIB_DLL_EXPORT char *getPIDFilename();
TESTLIB_DLL_EXPORT void registerPID(int pid);


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



TESTLIB_DLL_EXPORT void addLibArchExt(char *dest, unsigned int dest_max_len, int psize);

// Function to preload some libraries for test1_21 and test1_22
TESTLIB_DLL_EXPORT int readyTest21or22(BPatch_thread *appThread, 
      char *libNameA, char *libNameB, int mutateeFortran);

TESTLIB_DLL_EXPORT int strcmpcase(char *s1, char *s2);

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

TEST_DLL_EXPORT void contAndWaitForAllThreads(BPatch *bpatch, BPatch_thread *appThread, 
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

TESTLIB_DLL_EXPORT int pointerSize(BPatch_image *img);

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


bool createNewProcess(BPatch *bpatch, BPatch_thread *&appThread, BPatch_image *&appImage, 
      char *pathname, const char** child_argv);


int instrumentToCallZeroArg(BPatch_thread *appThread, BPatch_image *appImage, char *instrumentee, char*patch, int testNo, char *testName);


char* saveWorld(BPatch_thread *appThread);

int letOriginalMutateeFinish(BPatch_thread *appThread);

BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname);
BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname);

bool setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);
bool getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname);

char *searchPath(const char *path, const char *file);

// Functions in test_lib_soExecution.C below
//           or test_lib_dllExecution.C

TESTLIB_DLL_EXPORT bool inTestList(test_data_t &test, std::vector<char *> &test_list);

// TODO Implement this function for Windows   
TESTLIB_DLL_EXPORT bool getMutatorsForRunGroup (RunGroup *group, std::vector<TestInfo *> &group_tests);

TESTLIB_DLL_EXPORT int loadLibRunTest(test_data_t &testLib, ParameterDict &param);

// Function in MutateeStart.C
TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, char *pathname, const char **child_argv, bool useAttach);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTestAll(BPatch *bpatch, char *pathname, bool useAttach, ProcessList &procList, char *logfilename);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, char *mutatee, char *testname, bool useAttach, char *logfilename, char *humanlogname);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group, char *logfilename, char *humanlogname, bool verboseFormat, bool printLabels, int debugPrint, char *pidfilename);

TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group, ProcessList &procList, char *logfilename, char *humanlogname, bool verboseFormat, bool printLabels, int debugPrint, char *pidfilename);

// TESTLIB_DLL_EXPORT BPatch_thread *startMutateeTestSet(BPatch *bpatch, char *pathname, 
// 				   test_data_t tests[],
// 				   int first_test, int last_test,
// 				   bool useAttach, ProcessList &procList,
// 				   char *logfilename, bool runAllTests,
// 				   std::vector<char *> test_list);

// TESTLIB_DLL_EXPORT BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool useAttach, test_data_t tests[], unsigned int num_tests, int oldtest, char *logfilename);

TESTLIB_DLL_EXPORT void killMutatee(BPatch_thread *appThread);

#endif


